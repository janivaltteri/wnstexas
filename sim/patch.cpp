#include "patch.h"

#include <cmath>

const double pi{ 3.1415926535 };

double fungalgrowth(const double i_temp);

void Patch::update_state_k(const Setup &s,
			   const Parameters &p,
			   const double time)
{
  double wakepop = sw + ew + iw;
  bool hibe = hibernating(time,p);
  double grow_r;
  double wake_r;
  double sleep_r;
  if(hibe){
    grow_r = 0.0;
    wake_r = 0.0;
    sleep_r = p.sleep_rate;
  }else{
    grow_r = p.rh;
    wake_r = p.wake_rate;
    sleep_r = 0.0;
  }
  // store infection values for printing
  if(s.env_sigm){
    env_inf = p.env_max*sw*(pow((f/capacity)/p.env_half,p.env_steep) /
			    (1.0 + pow((f/capacity)/p.env_half,p.env_steep)));
  }else{
    env_inf = p.beta_e*sw*f/capacity;
  }
  dir_inf = p.beta_d*sh*ih/capacity;
  // get new state
  dsw = p.delta*ew -
    env_inf +
    grow_r*(sw + ew) -
    grow_r*sw*wakepop/capacity +
    wake_r*sh - sleep_r*sw;
  dew = env_inf +
    p.ny*iw -
    p.delta*ew -
    grow_r*ew*wakepop/capacity +
    wake_r*eh - sleep_r*ew;
  diw = -p.ny*iw -
    grow_r*iw*wakepop/capacity +
    wake_r*ih - sleep_r*iw;
  dsh = -dir_inf -
    p.myy_h*sh +
    sleep_r*sw - wake_r*sh;
  deh = dir_inf -
    p.khi*eh -
    p.myy_h*eh +
    sleep_r*ew - wake_r*eh;
  dih = p.khi*eh -
    p.myy_f*ih -
    p.myy_h*ih +
    sleep_r*iw - wake_r*ih;
  df = p.rf*f*fungalgrowth(intemp(time)) -
    p.rf*f*f/capacity +
    p.lambda*(iw + ih);
  // write back
  sw = sw + s.dt*dsw;
  ew = ew + s.dt*dew;
  iw = iw + s.dt*diw;
  sh = sh + s.dt*dsh;
  eh = eh + s.dt*deh;
  ih = ih + s.dt*dih;
  f =  f  + s.dt*df;
}

void Patch::update_state(const Setup &s,
			 const Parameters &p,
			 const double time)
{
  double wakepop = sw + ew + iw;
  bool hibe = hibernating(time,p);
  double grow_r;
  double wake_r;
  double sleep_r;
  if(hibe){
    grow_r = 0.0;
    wake_r = 0.0;
    sleep_r = p.sleep_rate;
  }else{
    grow_r = p.rh;
    wake_r = p.wake_rate;
    sleep_r = 0.0;
  }
  // get new state
  dsw = p.delta*ew -
    p.beta_e*sw*f +
    grow_r*(sw + ew) -
    p.rh*sw*wakepop +
    wake_r*sh - sleep_r*sw;
  dew = p.beta_e*sw*f + p.ny*iw -
    p.delta*ew -
    p.rh*ew*wakepop +
    wake_r*eh - sleep_r*ew;
  diw = -p.ny*iw -
    p.rh*iw*wakepop +
    wake_r*ih - sleep_r*iw;
  dsh = -p.beta_d*sh*ih - p.myy_h*sh +
    sleep_r*sw - wake_r*sh;
  deh = p.beta_d*sh*ih - p.khi*eh - p.myy_h*eh +
    sleep_r*ew - wake_r*eh;
  dih = p.khi*eh - p.myy_f*ih - p.myy_h*ih +
    sleep_r*iw - wake_r*ih;
  df = p.rf*f*fungalgrowth(intemp(time)) -
    p.rf*f*f + p.lambda*(iw+ih);
  // write back
  sw = sw + s.dt*dsw;
  ew = ew + s.dt*dew;
  iw = iw + s.dt*diw;
  sh = sh + s.dt*dsh;
  eh = eh + s.dt*deh;
  ih = ih + s.dt*dih;
  f =  f  + s.dt*df;
}

double Patch::intemp(const double time) const
{
  double v = sin(2.0*pi*time/365.0);
  if(v > 0.0){
    return intemp_hi*v + intemp_base;
  }else{
    return intemp_lo*v + intemp_base;
  }
}

double Patch::outemp(const double time) const
{
  return outemp_ampl*sin(2.0*pi*time/365.0) + outemp_base;
}

double fungalgrowth(const double i_temp)
{
  const double maksimi{ 50.0 };
  const double b3{ 0.0377 };
  const double c3{ 0.25 };
  const double tmin{ 7.0 };
  const double tmax{ 15.0 };
  if(i_temp < tmin){
    return 0.0;
  }else if(i_temp > tmax){
    return 0.0;
  }else{
    return maksimi*(b3*(i_temp-tmin))*(b3*(i_temp-tmin))*(1.0 - exp(c3*(i_temp-tmax)));
  }
}

bool Patch::hibernating(const double time, const Parameters &p) const
{
  double o_temp = outemp(time);
  if(o_temp < p.wake_temp){
    return true;
  }else{
    double i_temp = intemp(time);
    if((o_temp < p.wake_temp_up) && (i_temp < p.sleep_temp)){
      return true;
    }else{
      return false;
    }
  }
}

