#pragma once
#include <stdint.h>
#include <cmath>
namespace tankguard {
struct Settings {
  float low = 30, high = 85, rise = 2;
  uint32_t confirm = 5000, minOff = 60000, stale = 2500;
  uint32_t maxFill = 20*60*1000, riseWindow = 3*60*1000;
};
enum class Fault { None, Sensor, Stale, MaxFill, NoRise, Config };
class Controller {
 public:
  Settings s;
  bool armed = false, demand = false;
  Fault fault = Fault::None;
  float level = 0;
  explicit Controller(Settings settings = {}) : s(settings) {}
  void begin(uint32_t now) { armed=false; demand=false; fault=Fault::None;
    seen=false; lowPending=false; lastOff=now; }
  void stop(uint32_t now) { if(demand) lastOff=now; demand=false; lowPending=false; }
  void disarm(uint32_t now) { stop(now); armed=false; }
  bool acknowledge(uint32_t now) {
    stop(now); armed=false;
    if(!validConfig()) {fault=Fault::Config; return false;}
    if(!seen || uint32_t(now-lastSample)>s.stale) return false;
    fault=Fault::None; lastOff=now; return true;
  }
  bool arm(uint32_t now) {
    if(!validConfig()) { trip(Fault::Config,now); return false; }
    if(fault!=Fault::None || !seen || uint32_t(now-lastSample)>s.stale) return false;
    armed=true; return true;
  }
  void sample(float value, bool valid, uint32_t now, Fault invalidFault = Fault::Sensor) {
    if(!valid || !std::isfinite(value) || value<0 || value>100) {
      if(invalidFault==Fault::None) invalidFault=Fault::Sensor;
      seen=false; trip(invalidFault,now); return;
    }
    level=value; lastSample=now; seen=true;
    if(fault!=Fault::None || !armed) return;
    if(demand) {
      if(level>=s.high) { stop(now); return; }
    } else if(level<=s.low) {
      if(!lowPending) { lowPending=true; lowSince=now; }
      if(uint32_t(now-lowSince)>=s.confirm && uint32_t(now-lastOff)>=s.minOff) {
        demand=true; fillSince=riseSince=now; riseBase=level; lowPending=false;
      }
    } else lowPending=false;
    tick(now);
  }
  void tick(uint32_t now) {
    if(armed && (!seen || uint32_t(now-lastSample)>s.stale)) { trip(Fault::Stale,now); return; }
    if(!demand) return;
    if(uint32_t(now-fillSince)>=s.maxFill) { trip(Fault::MaxFill,now); return; }
    if(uint32_t(now-riseSince)>=s.riseWindow) {
      if(level-riseBase<s.rise) { trip(Fault::NoRise,now); return; }
      riseBase=level; riseSince=now;
    }
  }
 private:
  bool seen=false, lowPending=false;
  uint32_t lastSample=0,lastOff=0,lowSince=0,fillSince=0,riseSince=0;
  float riseBase=0;
  bool validConfig() const { return std::isfinite(s.low) && std::isfinite(s.high) &&
    std::isfinite(s.rise) && s.low>=0 && s.high<=100 && s.low<s.high && s.rise>0 &&
    s.confirm>0 && s.minOff>0 && s.stale>0 && s.maxFill>0 && s.riseWindow>0; }
  void trip(Fault f,uint32_t now) { stop(now); armed=false; if(fault==Fault::None) fault=f; }
};
inline const char* faultName(Fault f) {
 switch(f) { case Fault::None:return "none"; case Fault::Sensor:return "sensor";
 case Fault::Stale:return "stale"; case Fault::MaxFill:return "max_fill";
 case Fault::NoRise:return "no_rise"; default:return "config"; }
}
}
