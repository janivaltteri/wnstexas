/*
  white nose syndrome simulator
  simpler texas version
  Jani V Anttila
  2018 -- 2020
*/

// todo: return code nonzero if errors

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <time.h>
//#include <filesystem>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "simulation.h"
#include "parameters.h"
#include "landscape.h"

//namespace fs = std::filesystem;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{

  // check build flags
#ifdef DEBUG
  std::cout << "build: DEBUG set" << std::endl;
#endif

#ifdef NOMIGR
  std::cout << "build: NOMIGR set" << std::endl;
#endif

#ifdef NOCAP
  std::cout << "build: NOCAP set" << std::endl;
#endif
  
  // set defaults for file names
  std::string setup_infile{ "test_sim.json" };
  std::string parfilelist_infile{ "parfilelist.txt" };
  std::string outfileprefix{ "out" };
  std::string locations_infile{ "hibernacula.csv" };
  std::string distances_infile{ "distances.csv" };
  //std::string directoryname{ "" };
  //bool directory_supplied{ false };

  // commandline argument parsing -> file names
  try{

    // todo: add verbose option
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "print help message")
      ("setup,s",       po::value<std::string>(), "setup json file")
      ("parfilelist,p", po::value<std::string>(), "file with parameter file list")
      ("outfile,o",     po::value<std::string>(), "outfile prefix")
      ("locations,l",   po::value<std::string>(), "patch locations csv")
      ("distances,d",   po::value<std::string>(), "patch distances csv")
      //("directory,w",   po::value<std::string>(), "write directory")
      ;

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")){
      std::cout << desc << "\n";
      return 0;
    }

    if(vm.count("setup")){
      setup_infile = vm["setup"].as<std::string>();
      std::cout << "Reading setup from " << setup_infile << std::endl;
    }else{
      std::cout << "Setup not given       - default: " << setup_infile
		<< std::endl;
    }

    if(vm.count("parfilelist")){
      parfilelist_infile = vm["parfilelist"].as<std::string>();
      std::cout << "Reading parameter files listed in " << parfilelist_infile
		<< std::endl;
    }else{
      std::cout << "Parfilelist not given - default: " << parfilelist_infile
		<< std::endl;
    }

    if(vm.count("outfile")){
      outfileprefix = vm["outfile"].as<std::string>();
      std::cout << "Using outfileprefix " << outfileprefix << std::endl;
    }else{
      std::cout << "Outfile not given     - default: " << outfileprefix
		<< std::endl;
    }

    if(vm.count("locations")){
      locations_infile = vm["locations"].as<std::string>();
      std::cout << "Reading patch description from " << locations_infile
		<< std::endl;
    }else{
      std::cout << "Locations not given   - default: " << locations_infile
		<< std::endl;
    }

    if(vm.count("distances")){
      distances_infile = vm["distances"].as<std::string>();
      std::cout << "Reading patch distances from " << distances_infile
		<< std::endl;
    }else{
      std::cout << "Distances not given   - default: " << distances_infile
		<< std::endl;
    }

    /*
    if(vm.count("directory")){
      directoryname = vm["directory"].as<std::string>();
      std::cout << "Using directory " << directoryname << std::endl;
      directory_supplied = true;
    }else{
      std::cout << "Directory not given" << std::endl;
    }
    */

  }catch(std::exception& e){

    std::cout << "cmd arguments error: " << e.what() << std::endl;
    return 1;

  }catch(...){
    
    std::cout << "cmd arguments: exception of unknown type" << std::endl;
    return 1;
    
  }

  // read vector of parameter files from parameter list file
  std::vector<std::string> parfilelist;
  std::ifstream pfs(parfilelist_infile);
  std::string read;
  while(pfs >> read){ parfilelist.push_back(read); }
#ifdef DEBUG
  std::cout << "Parameter file list contains " << parfilelist.size()
	    << " entries" << std::endl;
#endif

  // construct vector of outfilenames
  std::vector<std::string> outfilenames;
  for(int i = 0; i < static_cast<int>(parfilelist.size()); i++){
    std::vector<std::string> pfstrings;
    boost::split(pfstrings, parfilelist.at(i), boost::is_any_of("."));
    std::stringstream ss;
    ss <<  outfileprefix << "-" << pfstrings.at(0) << ".csv";
    outfilenames.push_back(ss.str());
  }

  std::vector<Simulation> sims;
  
  // these use copy constructors
  sims.reserve(outfilenames.size());
  for(auto i = 0; i < static_cast<int>(outfilenames.size()); i++){
    sims.push_back(Simulation(outfilenames.at(i)));
  }

#ifdef DEBUG
  //std::cout << "Current path is " << fs::current_path() << std::endl;
#endif

  bool simulation_init_ok{ true };

  // setup
  if(!sims.at(0).setup.read_setup_json(setup_infile)){
    simulation_init_ok = false;
    std::cout << "error reading setup" << std::endl;
  }else{
    // locations
    if(!sims.at(0).ls.read_patch_csv_b(locations_infile)){
      simulation_init_ok = false;
      std::cout << "error reading landscape file" << std::endl;
    }else{
      // distances
      if(!sims.at(0).ls.read_migration_csv_b(distances_infile)){
	simulation_init_ok = false;
	std::cout << "error reading migration file" << std::endl;
      }else{

	// if there are multiple sims, copy setup and landscape
	if(outfilenames.size() > 1){
	  for(auto i = 1; i < static_cast<int>(outfilenames.size()); i++){
	    sims.at(i).setup = sims.at(0).setup; // copy setup
	    sims.at(i).ls = sims.at(0).ls; // copy landscape
	  }
	}

	// parameters
	for(auto i = 0; i < static_cast<int>(outfilenames.size()); i++){
	  if(!sims.at(i).par.read_parameters_json(parfilelist.at(i))){
	    simulation_init_ok = false;
	    std::cout << "error reading parameter file " << parfilelist.at(i) << std::endl;
	  }else{
	    sims.at(i).ls.set_init_state();
	    //sims.at(i).ls.set_init_state(sims.at(i).par);
	    sims.at(i).ls.set_migration_proportions(sims.at(i).par.gamma);
	  }
	}

#ifdef DEBUG
	sims.at(0).setup.print();
	sims.at(0).par.print();
#endif

      }
    }
  }

  // if there are multiple simulations to perform then copy the
  // setup from sims[0] and read parameters from files
  // todo: make simulation_init_ok false if any errors occur
  /*
  if(outfilenames.size() > 1){
    for(auto i = 1; i < static_cast<int>(outfilenames.size()); i++){
      sims.at(i).setup = sims.at(0).setup; // copy setup
      // todo: test that this returns true
      sims.at(i).par.read_parameters_json(parfilelist.at(i));
      sims.at(i).ls = sims.at(0).ls; // copy landscape
    }
  }
  */

  time_t time0;
  time(&time0);

  // do simulations
  // todo: simulate returns true/false
  // std::vector<bool> sim_ok; // set length
  if(simulation_init_ok){
    std::cout << "simulating " << std::endl << std::flush;
    // this is the part that should be parallel
    for(auto i = 0; i < static_cast<int>(outfilenames.size()); i++){
      std::cout << i << " " << std::flush;
      sims.at(i).wr.open_stream();
      sims.at(i).wr.write_state(sims.at(i).ls, sims.at(i).time);
      // todo: abort if returns false
      sims.at(i).simulate();
      sims.at(i).wr.close_stream();
    }
    std::cout << std::endl;
  }else{
    std::cout << "errors in initialising simulations"
	      << std::endl;
  }

  time_t time1;
  time(&time1);

  double seconds{ difftime(time1,time0) };

  std::cout << "reached end in " << seconds << " seconds" << std::endl;

  /*
    std::vector<std::unique_ptr<simulation>> sims;
    for(auto i = 0; i < p.num_sims; i++){
      sims.push_back(std::make_unique<simulation>());
    }
    
    for(auto i = 0; i < p.num_sims; i++){
    // todo: check how to pass landscape and dispersal objects
      sims.at(i)->initialise(std::make_shared<par>(p),
                             lands,
                             i);
    }

    #pragma omp parallel for
    for(auto i = 0; i < p.num_sims; i++){
      sims.at(i)->simulate(disp);
    }
  */

#ifdef DEBUG
  std::cout << "exiting" << std::endl;
#endif

  return 0;
}
