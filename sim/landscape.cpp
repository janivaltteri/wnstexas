#include "landscape.h"
#include "utils.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <cmath>

#include <boost/tokenizer.hpp>

double Landscape::step(const Setup &stp,
		       const Parameters &par,
		       const double time)
{
  double current_time = time;

#ifndef NOMIGR
  double swm;
  double ewm;
  int to_idx;
#endif
  
  for(auto t = 0; t < stp.res; t++){
    for(Patch &p : patches){
#ifdef NOCAP
      p.update_state(stp,par,current_time);
#else
      p.update_state_k(stp,par,current_time);
#endif
      p.s_immigrants = 0.0;
      p.e_immigrants = 0.0;
    }
    current_time += stp.dt;
#ifndef NOMIGR
    swm = 0.0;
    ewm = 0.0;
    for(Patch &p : patches){
      for(int i = 0; i < static_cast<int>(p.neighbours.size()); i++){
	if(!p.hibernating(current_time,par)){
	  to_idx = p.neighbours.at(i).patch_idx;
	  if(!patches.at(to_idx).hibernating(current_time,par)){
	    swm = stp.dt * par.mprop * p.sw * p.neighbours.at(i).proportion;
	    ewm = stp.dt * par.mprop * p.ew * p.neighbours.at(i).proportion;
	    patches.at(to_idx).s_immigrants += swm;
	    patches.at(to_idx).e_immigrants += ewm;
	    p.sw -= swm;
	    p.ew -= ewm;
	  }
	}
      }
    }
    for(Patch &p : patches){
      p.sw += p.s_immigrants;
      p.ew += p.e_immigrants;
    }
#endif
  }
  return current_time;
}

/**
 *  This function reads in the hibernacula/landscape file
 *  the landscape file format is:
 *  patch, county, init_inf, intemp_hi, intemp_lo, intemp_base, 
 *    outemp_ampl, outemp_base, K, wdist
 *  types are:
 *  int, int, int, double, double, double, double, double, double, double
 */
bool Landscape::read_patch_csv_b(const std::string filename)
{
  bool all_ok{ true };
  std::ifstream in{ filename };

  if(!in.is_open()){
    
    all_ok = false;
    initialised = false;
    std::cout << "could not open " << filename << std::endl;
    
  }else{

    typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokeniser;

    std::vector<std::string> vec;
    std::string line;

    int patch_count{ 0 };

    // skip first line
    getline(in,line);

    // read file line by line
    while(getline(in,line)){
      
      Tokeniser tok(line);
      vec.assign(tok.begin(),tok.end());
      
      if(vec.size() != 10){
	all_ok = false;
	std::cout << "warning: != 10 tokens on line "
		  << patch_count << std::endl;
	continue;
      }

      // vec.at(0) is patch idx, should be same as loop index
      if(patch_count != string_to_num<int>(vec.at(0))){
	all_ok = false;
	std::cout << "warning: patch indices not uniformly increasing "
		  << patch_count << std::endl;
	break;
      }

      patches.push_back(Patch(string_to_num<int>(vec.at(1)),
			      string_to_num<int>(vec.at(2)),
			      string_to_num<double>(vec.at(3)),
			      string_to_num<double>(vec.at(4)),
			      string_to_num<double>(vec.at(5)),
			      string_to_num<double>(vec.at(6)),
			      string_to_num<double>(vec.at(7)),
			      string_to_num<double>(vec.at(8)),
			      string_to_num<double>(vec.at(9))));

      patch_count++;
    }

    initialised = true;
#ifdef DEBUG
    std::cout << "set " << patch_count << " patches" << std::endl;
#endif
  }

  return all_ok;
}

/**
 *  This function reads in the distances file
 *  the migration file format is:
 *  from patch, to patch, within, distance
 *  types are:
 *  int, int, int, double
 */
bool Landscape::read_migration_csv_b(const std::string filename)
{
  bool all_ok{ true };
  std::ifstream in{ filename };

  if(!in.is_open()){
    
    all_ok = false;
    initialised = false;
    std::cout << "could not open " << filename << std::endl;
    
  }else{
    
    typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokeniser;

    std::vector<std::string> vec;
    std::string line;

    int idx_from;
    int idx_to;
    int within;
    double distance;
    int county_from;
    int county_to;

    int read_count{ 0 };
    
    while(getline(in,line)){
      
      Tokeniser tok(line);
      vec.assign(tok.begin(),tok.end());
      
      if(vec.size() != 4){
	all_ok = false;
	std::cout << "warning: != 4 tokens on line "
		  << read_count << std::endl;
	continue;
      }

      idx_from = string_to_num<int>(vec.at(0));
      idx_to = string_to_num<int>(vec.at(1));
      within = string_to_num<int>(vec.at(2));
      distance = string_to_num<double>(vec.at(3));

      county_from = patches.at(idx_from).county;
      county_to = patches.at(idx_to).county;
      
      if(within == 1){
	if(county_to == county_from){
	  all_ok = false;
	  std::cout << "warning: migr to from same "
		    << read_count << std::endl;
	}
      }

      if(within == 0){
	if(county_to != county_from){
	  all_ok = false;
	  std::cout << "warning: migr to from not same "
		    << read_count << std::endl;
	}
      }
      
      patches.at(idx_from).neighbours.push_back(Neighbour(idx_to,distance));
      read_count++;
    }

#ifdef DEBUG
    std::cout << "set " << read_count << " connections" << std::endl;
#endif
  }

  if(!all_ok){
    initialised = false;
  }
  
  return all_ok;
}

/**
 * called from main (l. 208)
 * sets initial state
 */
void Landscape::set_init_state()
{
  for(auto &p : patches){
    p.sw = p.capacity;
    p.ew = 0.0;
    p.iw = 0.0;
    p.f  = 0.0;
    p.sh = 0.0;
    p.eh = 0.0;
    p.ih = 0.0;
  }
}

/**
 * called from Simulation::simulate() (l. 36)
 */
bool Landscape::set_init_infection(const Parameters &par)
{
  bool all_ok{ true };
  
  for(auto &p : patches){

    bool patch_ok{ true };
    
    double wake_sum = p.sw;
    double sleep_sum = p.sh;
    if(p.ew > 0.0) all_ok = false;
    if(p.iw > 0.0) all_ok = false;
    if(p.eh > 0.0) all_ok = false;
    if(p.ih > 0.0) all_ok = false;
    if(p.f > 0.0) all_ok = false;

    if(p.init_infection){
      p.ew = par.initew * wake_sum;
      p.iw = par.initiw * wake_sum;
      p.eh = par.initew * sleep_sum;
      p.ih = par.initiw * sleep_sum;
      p.f = par.initf * p.capacity;
    }

    if(!patch_ok) all_ok = false;
  }

  return all_ok;
}

/**
 * sets the neighbour list migration proportions
 * called from main (l.179)
 */
void Landscape::set_migration_proportions(const double gamma)
{
  double prop{ 0.0 };
  double target_ccap{ 0.0 };
  for(auto &p : patches){
    int num_neighbours = p.neighbours.size();
    double msum{ 0.0 };
    for(auto i = 0; i < num_neighbours; i++){
      target_ccap = patches.at(p.neighbours.at(i).patch_idx).capacity;
      prop = target_ccap * exp(-gamma * p.neighbours.at(i).distance);
      p.neighbours.at(i).proportion = prop;
      msum += prop;
    }
    msum += (p.capacity - 1.0) * exp(-gamma * p.within_distance);
    for(auto &n : p.neighbours){
      n.proportion /= msum;
    }
  }
}

void Landscape::print(const int idx) const
{
  std::cout << " patch " << idx
	    << " sw " << patches.at(idx).sw
	    << " ew " << patches.at(idx).ew
	    << " iw " << patches.at(idx).iw
	    << " sh " << patches.at(idx).sh
	    << " eh " << patches.at(idx).eh
	    << " ih " << patches.at(idx).ih
	    << " f "  << patches.at(idx).f
	    << std::endl;
}

