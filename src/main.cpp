#include <Arduino.h>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "controller.h"
#include "level.h"
#ifndef TG_SIMULATED
#define TG_SIMULATED 1
#endif
static_assert(!config::OUTPUT_ENABLED, "Starter cannot drive pump; commission hardware first.");
tankguard::Controller controller;
#if TG_SIMULATED
float simulated = 50;
bool simulatedValid = true;
#endif
uint32_t lastRead = 0;
char command[40]; size_t commandSize = 0;
bool overflow = false;

enum class Acquire { Ok, Uncalibrated, BadSpan, Invalid };

const char* acquireName(Acquire a) {
  switch(a) {
    case Acquire::Ok: return "ok";
    case Acquire::Uncalibrated: return "uncalibrated";
    case Acquire::BadSpan: return "bad_span";
    default: return "invalid";
  }
}

void printCommands() {
  Serial.println("Commands: help | arm | stop | ack | level 0..100 (bench) | invalid (bench)");
}

void runCommand() {
  command[commandSize]=0;
  char* text=command;
  while(*text==' ' || *text=='\t') text++;
  size_t n=strlen(text);
  while(n>0 && (text[n-1]==' ' || text[n-1]=='\t')) text[--n]=0;
  if(n==0) return;
  uint32_t now=millis();
  if(strcmp(text,"help")==0) printCommands();
  else if(strcmp(text,"arm")==0) Serial.println(controller.arm(now)?"armed (DRY RUN)":"arm rejected");
  else if(strcmp(text,"stop")==0) { controller.disarm(now); Serial.println("disarmed; output locked off"); }
  else if(strcmp(text,"ack")==0) {
    bool hadFault=controller.fault!=tankguard::Fault::None;
    if(!controller.acknowledge(now)) Serial.println("ack rejected");
    else if(hadFault) Serial.println("fault cleared; disarmed; arm separately");
    else Serial.println("disarmed; arm separately");
  }
#if TG_SIMULATED
  else if(strcmp(text,"invalid")==0) simulatedValid=false;
  else if(strncmp(text,"level ",6)==0) {
    char* end; float v=strtof(text+6,&end);
    if(end!=text+6 && *end==0 && std::isfinite(v) && v>=0 && v<=100) {
      simulated=v; simulatedValid=true;
    } else Serial.println("level requires 0..100");
  }
#endif
  else printCommands();
}

Acquire readLevel(float& level) {
#if TG_SIMULATED
  level=simulated; return simulatedValid?Acquire::Ok:Acquire::Invalid;
#else
  if(!config::CALIBRATED) return Acquire::Uncalibrated;
  if(!tankguard::calibrationSpanOk(config::EMPTY_CM,config::FULL_CM,config::MIN_SPAN_CM))
    return Acquire::BadSpan;
  float cm[3];
  for(int i=0;i<3;i++) {
    digitalWrite(config::TRIG,LOW); delayMicroseconds(3);
    digitalWrite(config::TRIG,HIGH); delayMicroseconds(10); digitalWrite(config::TRIG,LOW);
    unsigned long us=pulseIn(config::ECHO,HIGH,25000UL);
    cm[i]=us*0.0343f/2.0f;
    // Reject the batch on any dead or out-of-range pulse. Do not median over a fault.
    if(us==0 || !tankguard::echoInRange(cm[i],config::EMPTY_CM)) return Acquire::Invalid;
    if(i<2) delay(65);
  }
  // Keep the last accepted percent so a stuck false echo cannot become the new baseline.
  static bool haveLevel=false;
  static float lastPercent=0;
  tankguard::LevelEval ev=tankguard::levelFromEchoes(cm[0],cm[1],cm[2],config::EMPTY_CM,
    config::FULL_CM,controller.s.high,config::MAX_ECHO_SPREAD_CM,haveLevel,lastPercent,
    config::MAX_LEVEL_STEP,config::MIN_SPAN_CM);
  if(!ev.ok) return Acquire::Invalid;
  haveLevel=true; lastPercent=ev.percent; level=ev.percent;
  return Acquire::Ok;
#endif
}
void setup() {
  // OFF before serial startup; external pull-down remains necessary during reset.
  digitalWrite(config::RELAY,LOW); pinMode(config::RELAY,OUTPUT);
  pinMode(config::TRIG,OUTPUT); digitalWrite(config::TRIG,LOW); pinMode(config::ECHO,INPUT);
  Serial.begin(115200); controller.begin(millis());
  Serial.println("TankGuard starter: OUTPUT LOCKED OFF. Type help. Starts disarmed.");
}
void loop() {
  // Bounded parser: serial traffic cannot starve the control loop.
  for(int budget=0;budget<64 && Serial.available();budget++) {
    char c=Serial.read();
    if(c=='\r') continue;
    if(c=='\n') { if(!overflow && commandSize) runCommand(); commandSize=0; overflow=false; }
    else if(commandSize<sizeof(command)-1 && !overflow) command[commandSize++]=c;
    else overflow=true;
  }
  uint32_t now=millis();
  controller.tick(now);
  if(uint32_t(now-lastRead)>=config::SAMPLE_MS) {
    float level=0; Acquire aq=readLevel(level); now=millis(); lastRead=now;
    tankguard::Fault invalidAs=(aq==Acquire::Uncalibrated || aq==Acquire::BadSpan)
      ? tankguard::Fault::Config : tankguard::Fault::Sensor;
    controller.sample(level,aq==Acquire::Ok,now,invalidAs);
    Serial.printf("valid=%d reason=%s level=%.1f armed=%d demand=%d output=OFF fault=%s\n",
      aq==Acquire::Ok,acquireName(aq),aq==Acquire::Ok?level:-1.0f,controller.armed,controller.demand,
      tankguard::faultName(controller.fault));
  }
  digitalWrite(config::RELAY,LOW); // Unconditionally locked OFF in both environments.
  delay(1);
}
