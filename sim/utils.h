#ifndef UTILS_H_
#define UTILS_H_

//#include <stdexcept>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

template<typename T>
T get_from_json(json &jobj, std::string fieldname, bool &check)
{
  if(jobj.contains(fieldname)){
    return static_cast<T>(jobj[fieldname]);
  }else{
    std::cout << "parameter file does not contain "
	      << fieldname << std::endl;
    check = false;

    // this might not work for all types T (e.g. std::string??)
    return static_cast<T>(0);
  }
}

// todo: this should throw if conversion not possible
template<typename T>
T string_to_num(const std::string &s)
{
  std::istringstream i(s);
  T x;
  if(!(i >> x)){
    //throw std::invalid_argument( "non-numeric token" );
    return static_cast<T>(0);
  }
  return x;
}

#endif
