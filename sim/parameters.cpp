#include "parameters.h"

#include <fstream>

#include "utils.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

bool Setup::read_setup_json(const std::string filename)
{
  bool all_found = true;
  std::ifstream i(filename);
  json j;
  i >> j;
  start_inf = get_from_json<double>(j,"start_inf",all_found);
  num_sims =  get_from_json<int>(j,"num_sims",all_found);
  res =       get_from_json<int>(j,"res",all_found);
  max_time =  get_from_json<int>(j,"max_time",all_found);
  printmode = get_from_json<int>(j,"printmode",all_found);
  stride    = get_from_json<int>(j,"stride",all_found);
  env_sigm =  get_from_json<bool>(j,"env_sigm",all_found);
  if(all_found){
#ifdef DEBUG
    std::cout << "setup read ok" << std::endl;
#endif
    dt = 1.0/static_cast<double>(res);
    initialised = true;
  }else{
    std::cout << "errors in reading setup" << std::endl;
    initialised = false;
  }
  return all_found;
}

void Setup::print() const
{
  std::cout << "setup is: " << std::endl
	    << " dt " << dt << std::endl
	    << " num_sims " << num_sims << std::endl
	    << " res " << res << std::endl
	    << " max_time " << max_time << std::endl
	    << " printmode " << printmode << std::endl
	    << " stride " << stride << std::endl
	    << " start_inf " << start_inf << std::endl
	    << " env_sigm " << env_sigm << std::endl;
}


bool Parameters::read_parameters_json(std::string filename)
{
  bool all_found = true;
  std::ifstream i(filename);
  json j;
  i >> j;
  rh =     get_from_json<double>(j,"rh",all_found);
  rf =     get_from_json<double>(j,"rf",all_found);
  beta_e = get_from_json<double>(j,"beta_e",all_found);
  beta_d = get_from_json<double>(j,"beta_d",all_found);
  env_max =   get_from_json<double>(j,"env_max",all_found);
  env_half =  get_from_json<double>(j,"env_half",all_found);
  env_steep = get_from_json<double>(j,"env_steep",all_found);
  myy_f =  get_from_json<double>(j,"myy_f",all_found);
  myy_h =  get_from_json<double>(j,"myy_h",all_found);
  lambda = get_from_json<double>(j,"lambda",all_found);
  delta =  get_from_json<double>(j,"delta",all_found);
  wake_temp =  get_from_json<double>(j,"wake_temp",all_found);
  wake_temp_up = get_from_json<double>(j,"wake_temp_up",all_found);
  wake_rate =  get_from_json<double>(j,"wake_rate",all_found);
  sleep_temp = get_from_json<double>(j,"sleep_temp",all_found);
  sleep_rate = get_from_json<double>(j,"sleep_rate",all_found);
  khi =    get_from_json<double>(j,"khi",all_found);
  ny =     get_from_json<double>(j,"ny",all_found);
  gamma =  get_from_json<double>(j,"gamma",all_found);
  mprop =  get_from_json<double>(j,"mprop",all_found);
  initsw = get_from_json<double>(j,"initsw",all_found);
  initew = get_from_json<double>(j,"initew",all_found);
  initiw = get_from_json<double>(j,"initiw",all_found);
  initf  = get_from_json<double>(j,"initf",all_found);
  if(all_found){
#ifdef DEBUG
    std::cout << "parameters read ok" << std::endl;
#endif
    initialised = true;
  }else{
    std::cout << "errors in reading parameters" << std::endl;
    initialised = false;
  }
  return all_found;
}

void Parameters::print() const
{
  std::cout << "parameters are: " << std::endl
	    << " rh " << rh << " rf " << rf << std::endl
	    << " beta_e " << beta_e << " beta_d " << beta_d << std::endl
	    << " env_max " << env_max << " env_half " << env_half
	    << " env_steep " << env_steep << std::endl
	    << " myy_f " << myy_f << " myy_h " << myy_h << std::endl
	    << " lambda " << lambda << " delta " << delta << std::endl
	    << " wake_temp " << wake_temp << " wake_rate " << wake_rate
	    << " wake_temp_up " << wake_temp_up << std::endl
	    << " sleep_temp " << sleep_temp << " sleep_rate " << sleep_rate << std::endl
	    << " khi " << khi << " ny " << ny << std::endl
	    << " gamma " << gamma << " mprop " << mprop << std::endl
	    << " initsw " << initsw
	    << " initew " << initew
	    << " initiw " << initiw
	    << " initf " << initf << std::endl;
}
