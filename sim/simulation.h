#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <string>

#ifdef DEBUG
#include <iostream>
#endif

#include "parameters.h"
#include "landscape.h"
#include "writer.h"

class Simulation
{
 public:

  Setup setup;
  Parameters par;
  Landscape ls;
  Writer wr;

  double time;

  Simulation(const std::string outfilename)
    : setup(Setup()),
      par(Parameters()),
      ls(Landscape()),
      wr(Writer(outfilename)), // should be () and open_streams()
      time(0.0)
  {
#ifdef DEBUG
    std::cout << "Simulation constructor called" << std::endl;
#endif
  };

  ~Simulation()
  {
#ifdef DEBUG
    std::cout << "Simulation destructor called" << std::endl;
#endif
  }
  
  /*
  Simulation(Setup &in_setup)
    : setup(in_setup) { };
  */

  bool simulate();

  // private:
  // static Setup read_setup_from_file(std::string &setupfilename);

};

#endif
