#include "landscape.h"
#include "utils.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <cmath>

#include <boost/tokenizer.hpp>

// todo: have separate patch initialised and migr initialised bools

// todo: migration needs update to patch and county idxs
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
  
  // setup.res -> number of substeps
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
	if(!p.hibernating(current_time,par)){ // oli: from_temp > par.wake_temp
	  to_idx = p.neighbours.at(i).patch_idx;
	  if(!patches.at(to_idx).hibernating(current_time,par)){ // to_temp > par.wake_temp
	    // todo: check this logic
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

      // todo: check that conversions work and all not ok if no
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

    // todo: check increasing order in from and to!!

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

      // these should throw if input not good
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

// todo: check that patches and migrations are read before

/**
 * called from main (l. 208)
 * sets initial state
 */
void Landscape::set_init_state()
{
  for(auto &p : patches){
    /*
    if(p.init_infection > 0){
      p.sw = par.initsw * p.capacity;
      p.ew = par.initew * p.capacity;
      p.iw = par.initiw * p.capacity;
      p.f  = par.initf * p.capacity;
    }else{
    */
    p.sw = p.capacity;
    p.ew = 0.0;
    p.iw = 0.0;
    p.f  = 0.0;
    //}
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

// todo: check that patches and migrations are read before

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
      // neighbour idx: p.neighbours.at(i).patch_idx
      // neighbour ccap: patches[idx].capacity
      // total: patches.at(p.neighbours.at(i).patch_idx).capacity
      target_ccap = patches.at(p.neighbours.at(i).patch_idx).capacity;
      prop = target_ccap * exp(-gamma * p.neighbours.at(i).distance);
      p.neighbours.at(i).proportion = prop;
      msum += prop;
    }
    // migration to self
    // (p.capacity - 1) * exp(-gamma * p.self_distance)
    // msum += self_migration
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

// old version for reference - not used
/*
bool Landscape::read_patch_csv(const std::string filename)
{ 
  // first count rows, then read and populate

  std::ifstream cavefile_count(filename);
  if(cavefile_count){
    // put this to temp var and place to member later
    int size = std::count(std::istreambuf_iterator<char>(cavefile_count), 
			  std::istreambuf_iterator<char>(), '\n');
    cavefile_count.close(); // miksi kahdesti? virhe?
    std::cout << "landscape::read_patch_csv counts " << size << " lines" << std::endl;
    patches.resize(size);
    cavefile_count.close(); // miksi kahdesti? virhe?
  }else{
    std::cout << "error: could not read " << filename << std::endl;
    return false;
  }
  // cavefile_count.close() ?

  std::ifstream cavefile_read(filename);
  if(cavefile_read){
    std::string line_value;
    std::string field_string;
    std::stringstream line_stream;
  
    int county_idx;
    int init_infection;
    double in_ampl;
    double in_base;
    double ou_ampl;
    double ou_base;

    // the landscape file format is:
    // county, init_infection, intemp_ampl, intemp_base, outemp_ampl, outemp_base
    // types are:
    // int, int, double, double, double, double
    for(auto i = 0; i < size; i++){ // proceed line by line, one for each patch
      // read the full line and place to stream
      getline(cavefile_read,line_value,'\n');
      line_stream.clear();
      line_stream << line_value;
      // read county index
      getline(line_stream,field_string,',');
      county_idx = string_to_num<int>(field_string);
      // read init_infection
      getline(line_stream,field_string,',');
      init_infection = string_to_num<int>(field_string);
      // read intemp_amplitude
      getline(line_stream,field_string,',');
      in_ampl = string_to_num<double>(field_string);
      // read intemp_base
      getline(line_stream,field_string,',');
      in_base = string_to_num<double>(field_string);
      // read outemp_ampl
      getline(line_stream,field_string,',');
      ou_ampl = string_to_num<double>(field_string);
      // read outemp_base
      getline(line_stream,field_string,',');
      ou_base = string_to_num<double>(field_string);
      // set the read values to patch i
      patches.at(i).county = county_idx;
      patches.at(i).init_infection = init_infection;
      patches.at(i).intemp_ampl = in_ampl; // 6.0;
      patches.at(i).intemp_base = in_base; // 3.0;
      patches.at(i).outemp_ampl = ou_ampl; // 14.0;
      patches.at(i).outemp_base = ou_base; // 10.0;
      // call to empty state, diff, immigrants
      if(i == 0){
	std::cout << "patch 0 reads: " << county_idx
		  << " " << init_infection << " " << in_ampl << " " << in_base
		  << " " << ou_ampl << " " << ou_base << std::endl;
      }
    }
    cavefile_read.close();
    initialised = true;
  }else{
    std::cout << "error: could not read " << filename;
    initialised = false;
    return false;
  }
  // cavefile_read.close(); ?
  
  return true;
}
*/

// old version for reference - not used
/*
bool Landscape::read_migration_csv(const std::string filename)
{
  // first count rows, then read and populate
  
  std::ifstream migrfile_count(filename);
  int read_count;
  if(migrfile_count){
    read_count = std::count(std::istreambuf_iterator<char>(migrfile_count), 
			    std::istreambuf_iterator<char>(), '\n');
    migrfile_count.close(); // miksi kahdesti? virhe?
    std::cout << "landscape::read_migration_csv counts "
	      << read_count << " lines" << std::endl;
    migrfile_count.close(); // miksi kahdesti? virhe?
  }else{
    std::cout << "error: could not read " << filename << std::endl;
    return false;
  }
  // migrfile_count.close() ?

  std::ifstream migrfile_read(filename);
  if(migrfile_read){
    std::string line_value;
    std::string field_string;
    std::stringstream line_stream;

    // these should be idx_from and idx_to
    int county_from;
    int county_to;
    double distance;

    // todo: check increasing order in from and to!!
    // the migration file format is:
    // from county, to county, distance
    // types are:
    // int, int, double
    for(auto i = 0; i < read_count; i++){ // proceed line by line, one for each patch
      // read the full line and place to stream
      getline(migrfile_read,line_value,'\n');
      line_stream.clear();
      line_stream << line_value;
      // read from county
      getline(line_stream,field_string,',');
      county_from = string_to_num<int>(field_string);
      // read to county
      getline(line_stream,field_string,',');
      county_to = string_to_num<int>(field_string);
      // read distance
      getline(line_stream,field_string,',');
      distance = string_to_num<double>(field_string);
      // set the read values to patch i
      patches.at(county_from).neighbours.push_back(Neighbour(distance,county_to));
    }
    migrfile_read.close();
    //initialised = true;
  }else{
    std::cout << "error: could not read " << filename;
    initialised = false;
    return false;
  }
  // migrfile_read.close(); ?
  
  return true;
}
*/

