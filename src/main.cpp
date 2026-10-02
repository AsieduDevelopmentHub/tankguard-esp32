#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "controller.h"
#ifndef TG_SIMULATED
#define TG_SIMULATED 1
#endif
static_assert(!config::OUTPUT_ENABLED, "Starter cannot drive pump; commission hardware first.");
tankguard::Controller controller;
float simulated = 50;
bool simulatedValid = true;
uint32_t lastRead = 0;
char command[40]; size_t commandSize = 0;
bool overflow = false;

void runCommand() {
  command[commandSize]=0;
  uint32_t now=millis();
  if(strcmp(command,"arm")==0) Serial.println(controller.arm(now)?"armed (DRY RUN)":"arm rejected");
  else if(strcmp(command,"stop")==0) controller.disarm(now);
  else if(strcmp(command,"ack")==0) Serial.println(controller.acknowledge(now)?"fault cleared; arm separately":"ack rejected");
#if TG_SIMULATED
  else if(strcmp(command,"invalid")==0) simulatedValid=false;
  else if(strncmp(command,"level ",6)==0) {
    char* end; float v=strtof(command+6,&end);
    if(end!=command+6 && *end==0 && isfinite(v) && v>=0 && v<=100) {
      simulated=v; simulatedValid=true;
    } else Serial.println("level requires 0..100");
  }
#endif
  else Serial.println("Commands: arm | stop | ack | level 0..100 (bench) | invalid (bench)");
}

bool readLevel(float& level) {
#if TG_SIMULATED
  level=simulated; return simulatedValid;
#else
  if(!config::CALIBRATED || config::EMPTY_CM<=config::FULL_CM || config::FULL_CM<2) return false;
  float cm[3];
  for(int i=0;i<3;i++) {
    digitalWrite(config::TRIG,LOW); delayMicroseconds(3);
    digitalWrite(config::TRIG,HIGH); delayMicroseconds(10); digitalWrite(config::TRIG,LOW);
    unsigned long us=pulseIn(config::ECHO,HIGH,25000UL);
    cm[i]=us*0.0343f/2;
    // Reject entire batch on any invalid pulse; do not mask sensor faults with a median.
    if(us==0 || cm[i]<2 || cm[i]>400 || cm[i]>config::EMPTY_CM+5) return false;
    if(i<2) delay(65);
  }
  for(int i=0;i<2;i++) for(int j=i+1;j<3;j++) if(cm[j]<cm[i]) {float v=cm[i];cm[i]=cm[j];cm[j]=v;}
  // Use nearest reading for prompt high-stop; median for ordinary level control.
  float nearest=100*(config::EMPTY_CM-cm[0])/(config::EMPTY_CM-config::FULL_CM);
  float distance=nearest>=controller.s.high?cm[0]:cm[1];
  level=constrain(100*(config::EMPTY_CM-distance)/(config::EMPTY_CM-config::FULL_CM),0.0f,100.0f);
  return true;
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
    float level=0; bool valid=readLevel(level); now=millis(); lastRead=now;
    controller.sample(level,valid,now);
    Serial.printf("valid=%d level=%.1f armed=%d demand=%d output=OFF fault=%s\n",
      valid,valid?level:-1.0f,controller.armed,controller.demand,tankguard::faultName(controller.fault));
  }
  digitalWrite(config::RELAY,LOW); // Unconditionally locked OFF in both environments.
  delay(1);
}
