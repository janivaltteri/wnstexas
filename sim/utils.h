#ifndef UTILS_H_
#define UTILS_H_

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

    return static_cast<T>(0);
  }
}

template<typename T>
T string_to_num(const std::string &s)
{
  std::istringstream i(s);
  T x;
  if(!(i >> x)){
    return static_cast<T>(0);
  }
  return x;
}

#endif
