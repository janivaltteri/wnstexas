#include "simulation.h"

#include <iostream>

bool Simulation::simulate()
{
  if(!ls.initialised){
    
    std::cout << "error: simulate called before landscape initialisation" << std::endl;
    return false;
    
  }else if(!par.initialised){
    
    std::cout << "error: simulate called before parameter initialisation" << std::endl;
    return false;

  }else if(!setup.initialised){
    
    std::cout << "error: simulate called before setup initialisation" << std::endl;
    return false;

  }else if(!wr.check_stream()){

    std::cout << "error: writer stream not open" << std::endl;
    return false;
    
  }else{

    double new_time{ 0.0 };
    bool infection_set{ false };
    bool infection_set_ok;

    // start simulation
    int counter = 0;
    while(time < static_cast<double>(setup.max_time)){
      if(!infection_set && (time >= setup.start_inf)){
	infection_set_ok = ls.set_init_infection(par);
	if(!infection_set_ok){
	  std::cout << "warning: set_init_infection error" << std::endl;
	}
	infection_set = true;
      }
      new_time = ls.step(setup,par,time);
      time = new_time;
      if(infection_set){
	if((counter++ % setup.stride) == 0){
	  wr.write_state(ls,time);
	}
      }
    }
    
    return true;
  }
}
