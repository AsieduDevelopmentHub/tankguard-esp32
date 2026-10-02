#include "controller.h"
#include <cassert>
#include <cstdio>
using namespace tankguard;
Settings fast(){Settings s;s.confirm=10;s.minOff=20;s.stale=100;s.maxFill=1000;s.riseWindow=500;return s;}
void start(Controller& c,uint32_t t=0){c.begin(t);c.sample(20,true,t);assert(c.arm(t));c.sample(20,true,t);c.sample(20,true,t+20);assert(c.demand);}
int main(){
 {Controller c(fast());c.begin(0);c.sample(20,true,100);assert(!c.demand);assert(!c.armed);}
 {Controller c(fast());start(c);c.sample(60,true,25);assert(c.demand);c.sample(85,true,30);assert(!c.demand);c.sample(20,true,31);c.sample(20,true,42);assert(!c.demand);c.sample(20,true,50);assert(c.demand);}
 {Controller c(fast());start(c);c.sample(0,false,25);assert(!c.demand&&c.fault==Fault::Sensor);assert(!c.arm(26));c.sample(20,true,27);assert(c.acknowledge(28));assert(!c.armed);assert(c.arm(29));}
 {Controller c(fast());start(c);c.tick(121);assert(!c.demand&&c.fault==Fault::Stale);}
 {auto s=fast();s.riseWindow=50;Controller c(s);start(c);c.sample(20,true,70);assert(c.fault==Fault::NoRise&&!c.demand);}
 {auto s=fast();s.maxFill=50;Controller c(s);start(c);c.sample(30,true,70);assert(c.fault==Fault::MaxFill&&!c.demand);}
 {Controller c(fast());start(c,UINT32_MAX-10);c.sample(85,true,15);assert(!c.demand);}
 {Controller c(fast());start(c);c.disarm(21);assert(!c.armed&&!c.demand);c.begin(22);assert(!c.arm(22));}
 {auto s=fast();s.high=s.low;Controller c(s);c.begin(0);c.sample(20,true,0);assert(!c.arm(0)&&c.fault==Fault::Config);}
 {Controller c(fast());c.begin(0);c.sample(NAN,true,1);assert(c.fault==Fault::Sensor);}
 puts("10 controller scenarios passed");
}
