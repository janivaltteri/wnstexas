#ifndef PATCH_H_
#define PATCH_H_

#include <vector>
#include "parameters.h"

struct Neighbour
{
public:

  int patch_idx;
  double distance;   // km
  double proportion; // proportion of population migrating there
  
Neighbour(const int in_patch_idx,
	  const double in_distance)
  : patch_idx(in_patch_idx),
    distance(in_distance),
    proportion(0.0)
  { };
};

struct Patch
{

public:

  int county;
  int init_infection;

  double sw; double sh; double ew; double eh; double iw; double ih; double f;
  
  double dsw; double dsh; double dew; double deh; double diw; double dih; double df;

  double capacity;
  double within_distance; // within county distance, used for self migration prop

  double intemp_hi; /// init from landscape::read_from_csv
  double intemp_lo; /// init from landscape::read_from_csv
  double intemp_base; /// init from landscape::read_from_csv
  double outemp_ampl; /// init from landscape::read_from_csv
  double outemp_base; /// init from landscape::read_from_csv

  double s_immigrants; double e_immigrants;
  double env_inf; double dir_inf;

  std::vector<Neighbour> neighbours;

  // this default constructor should not be used
Patch()
  : county(0), init_infection(0),
    sw(0.0), sh(0.0), ew(0.0), eh(0.0), iw(0.0), ih(0.0), f(0.0),
    dsw(0.0), dsh(0.0), dew(0.0), deh(0.0), diw(0.0), dih(0.0), df(0.0),
    capacity(1.0), within_distance(0.0),
    intemp_hi(0.0), intemp_lo(0.0), intemp_base(0.0),
    outemp_ampl(0.0), outemp_base(0.0),
    s_immigrants(0.0), e_immigrants(0.0), env_inf(0.0), dir_inf(0.0),
    neighbours(std::vector<Neighbour>())
  { };

  // this constructor is called from landscape::read_patch_csv_b
Patch(const int in_county_idx,
      const int in_init_infection,
      const double in_intemp_hi,
      const double in_intemp_lo,
      const double in_intemp_base,
      const double in_outemp_ampl,
      const double in_outemp_base,
      const double in_capacity,
      const double in_wdist)
  : county(in_county_idx), init_infection(in_init_infection),
    sw(0.0), sh(0.0), ew(0.0), eh(0.0), iw(0.0), ih(0.0), f(0.0),
    dsw(0.0), dsh(0.0), dew(0.0), deh(0.0), diw(0.0), dih(0.0), df(0.0),
    capacity(in_capacity), within_distance(in_wdist),
    intemp_hi(in_intemp_hi), intemp_lo(in_intemp_lo), intemp_base(in_intemp_base),
    outemp_ampl(in_outemp_ampl), outemp_base(in_outemp_base),
    s_immigrants(0.0), e_immigrants(0.0), env_inf(0.0), dir_inf(0.0),
    neighbours(std::vector<Neighbour>())
  { };
  
  // no need for copy constructor

  // called if NOCAP defined
  void update_state(const Setup &s,
		    const Parameters &p,
		    const double time);

  // default ccap version
  void update_state_k(const Setup &s,
		      const Parameters &p,
		      const double time);

  bool hibernating(const double time, const Parameters &p) const;
  //double wakerate(const double o_temp, const Parameters &p);
  //double sleeprate(const double o_temp, const Parameters &p);
  
  double intemp(const double time) const;

  double outemp(const double time) const;

};

#endif
