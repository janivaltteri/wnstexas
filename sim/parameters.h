#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <string>

#include <iostream>


class Setup
{ 
public:

  double dt;     //derived
  double start_inf; // time when the infection is initiated

  int num_sims;
  int res;       // sets dt
  int max_time;  // could be double
  int printmode; // could be enum
  int stride;    // write to file every stride interval

  bool env_sigm; // false: linear, true: sigmoid

  bool initialised;

  Setup()
    : dt(0.0), start_inf(0.0), num_sims(0), res(0), max_time(0),
      printmode(0), stride(0), env_sigm(false), initialised(false)
  {
#ifdef DEBUG
    std::cout << "Setup constructor called" << std::endl;
#endif
  };

  ~Setup()
  {
#ifdef DEBUG
    std::cout << "Setup destructor called" << std::endl;
#endif
  }

  bool read_setup_json(const std::string filename);
  void print() const;
  
};

struct Parameters
{
public:

  double rh;     double rf;
  double beta_e; double beta_d;
  double env_max; double env_half; double env_steep;
  double myy_f;  double myy_h;
  double lambda;
  double delta;
  double wake_temp;  double wake_temp_up; double wake_rate;
  double sleep_temp; double sleep_rate;
  double khi;
  double ny;
  double gamma;
  double mprop;
  double initsw; double initew; double initiw;
  double initf;

  bool initialised;

  bool read_parameters_json(const std::string filename);
  
  void print() const;
  
};

#endif
