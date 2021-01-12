#include "writer.h"

#include <iostream>
#include <fstream>

void Writer::open_stream()
{
  
#ifdef DEBUG
  std::cout << "writing to stream " << filename << std::endl;
#endif
  
  os.open(filename);
  
  // separate write init
  os << "time,patch,sw,ew,iw,sh,eh,ih,f,itemp,otemp,sm,em,env,dir"
     << std::endl;
}

bool Writer::check_stream()
{
  if(os.is_open()){
    return true;
  }else{
    return false;
  }
}

void Writer::write_state(const Landscape &l, const double time)
{
  for(int i = 0; i < static_cast<int>(l.patches.size()); i++){
    os << time << "," << i
       << "," << l.patches.at(i).sw
       << "," << l.patches.at(i).ew
       << "," << l.patches.at(i).iw
       << "," << l.patches.at(i).sh
       << "," << l.patches.at(i).eh
       << "," << l.patches.at(i).ih
       << "," << l.patches.at(i).f
       << "," << l.patches.at(i).intemp(time)
       << "," << l.patches.at(i).outemp(time)
       << "," << l.patches.at(i).s_immigrants
       << "," << l.patches.at(i).e_immigrants
       << "," << l.patches.at(i).env_inf
       << "," << l.patches.at(i).dir_inf
       << std::endl;
  }
}

void Writer::close_stream()
{
  os.close();
}

