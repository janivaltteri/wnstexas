#ifndef LANDSCAPE_H_
#define LANDSCAPE_H_

#include <iostream>
#include <vector>
#include <string>

#include "parameters.h"
#include "patch.h"

class Landscape
{

 public:

  std::vector<Patch> patches;
  
  // int size; // this should not be here

  bool initialised;  

  Landscape()
    : patches(std::vector<Patch>()), initialised(false)
  {
#ifdef DEBUG
    std::cout << "Landscape constructor called" << std::endl;
#endif
  };

  ~Landscape()
  {
#ifdef DEBUG
    std::cout << "Landscape destructor called" << std::endl;
#endif
  }

  // copy constructor will be used ?

  // returns new time
  double step(const Setup &stp,
	      const Parameters &par,
	      const double time);

  // note: only _b versions are used now
  //bool read_patch_csv(const std::string filename); // not used
  bool read_patch_csv_b(const std::string filename);
  //bool read_migration_csv(const std::string filename); // not used
  bool read_migration_csv_b(const std::string filename);
  
  void set_init_state();
  bool set_init_infection(const Parameters &p);
  void set_migration_proportions(const double gamma);
  void print(const int idx) const;
  
};

#endif