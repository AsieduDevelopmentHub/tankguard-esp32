#include "controller.h"
#include "level.h"
#include <cstdio>
#include <cstdint>
using namespace tankguard;
static int checks=0, failures=0;
#define CHECK(cond) do { ++checks; if(!(cond)) { \
  std::fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#cond); ++failures; } } while(0)
Settings fast(){Settings s;s.confirm=10;s.minOff=20;s.stale=100;s.maxFill=1000;s.riseWindow=500;return s;}
void start(Controller& c,uint32_t t=0){c.begin(t);c.sample(20,true,t);CHECK(c.arm(t));c.sample(20,true,t);c.sample(20,true,t+20);CHECK(c.demand);}
LevelEval echoes(float a,float b,float c,bool hasPrev=false,float prev=0){
  return levelFromEchoes(a,b,c,100.f,15.f,85.f,10.f,hasPrev,prev,15.f,10.f);
}
int main(){
 {Controller c(fast());c.begin(0);c.sample(20,true,100);CHECK(!c.demand);CHECK(!c.armed);}
 {Controller c(fast());start(c);c.sample(60,true,25);CHECK(c.demand);c.sample(85,true,30);CHECK(!c.demand);c.sample(20,true,31);c.sample(20,true,42);CHECK(!c.demand);c.sample(20,true,50);CHECK(c.demand);}
 {Controller c(fast());start(c);c.sample(0,false,25);CHECK(!c.demand&&c.fault==Fault::Sensor);CHECK(!c.arm(26));c.sample(20,true,27);CHECK(c.acknowledge(28));CHECK(!c.armed);CHECK(c.arm(29));}
 {Controller c(fast());start(c);c.tick(121);CHECK(!c.demand&&c.fault==Fault::Stale);}
 {auto s=fast();s.riseWindow=50;Controller c(s);start(c);c.sample(20,true,70);CHECK(c.fault==Fault::NoRise&&!c.demand);}
 {auto s=fast();s.riseWindow=50;Controller c(s);start(c);c.sample(23,true,70);CHECK(c.fault==Fault::None&&c.demand);c.sample(25,true,120);CHECK(c.fault==Fault::None&&c.demand);}
 {auto s=fast();s.maxFill=50;Controller c(s);start(c);c.sample(30,true,70);CHECK(c.fault==Fault::MaxFill&&!c.demand);c.sample(0,false,80);CHECK(c.fault==Fault::MaxFill);}
 {Controller c(fast());start(c,UINT32_MAX-10);c.sample(85,true,15);CHECK(!c.demand);}
 {Controller c(fast());start(c);c.disarm(21);CHECK(!c.armed&&!c.demand);c.begin(22);CHECK(!c.arm(22));}
 {auto s=fast();s.high=s.low;Controller c(s);c.begin(0);c.sample(20,true,0);CHECK(!c.arm(0)&&c.fault==Fault::Config);}
 {Controller c(fast());c.begin(0);c.sample(NAN,true,1);CHECK(c.fault==Fault::Sensor);}
 {Controller c(fast());c.begin(0);c.sample(40,true,0);CHECK(!c.arm(150));CHECK(!c.armed);}
 {Controller c(fast());c.begin(0);CHECK(!c.acknowledge(0));CHECK(c.fault==Fault::None);}
 {Controller c(fast());start(c);CHECK(c.acknowledge(25));CHECK(!c.armed&&!c.demand&&c.fault==Fault::None);CHECK(c.arm(25));CHECK(c.armed&&!c.demand);}
 {Controller c(fast());c.begin(0);c.sample(20,true,0);CHECK(c.arm(0));c.sample(20,true,0);c.sample(50,true,5);c.sample(20,true,6);c.sample(20,true,15);CHECK(!c.demand);c.sample(20,true,30);CHECK(c.demand);}
 {Controller c(fast());c.begin(0);c.sample(0,false,1,Fault::Config);CHECK(c.fault==Fault::Config);c.sample(0,false,2);CHECK(c.fault==Fault::Config);}
 {LevelEval hi=echoes(20,22,24);CHECK(hi.ok&&hi.percent>94.f&&hi.percent<94.2f);}
 {LevelEval mid=echoes(50,52,54);CHECK(mid.ok&&mid.percent>56.4f&&mid.percent<56.6f);}
 {CHECK(!echoes(20,90,95).ok);}
 {CHECK(!echoes(0,50,52).ok);}
 {CHECK(!echoes(50,52,106).ok);}
 {CHECK(!calibrationSpanOk(20.f,15.f,10.f));}
 {LevelEval jump=echoes(80,82,84,true,94.f);CHECK(!jump.ok);}
 {LevelEval held=echoes(21,22,23,true,94.f);CHECK(held.ok);}
 if(failures||checks<1){std::printf("%d checks, %d failed\n",checks,failures);return 1;}
 std::printf("%d checks passed\n",checks);
 return 0;
}
