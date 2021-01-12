#ifndef WRITER_H_
#define WRITER_H_

#include <string>
#include <fstream>

#ifdef DEBUG
#include <iostream>
#endif

#include "landscape.h"

class Writer
{
  
 public:

  std::string filename;
  std::ofstream os;

  Writer(const std::string in_filename)
    : filename(in_filename),
      os(std::ofstream()) { };

  // copy constructor
  Writer(const Writer& other)
    : filename(other.filename),
      os(std::ofstream())
  {
#ifdef DEBUG
    std::cout << "Writer copy constructor called" << std::endl;
#endif
  };
    
  void open_stream();

  bool check_stream(); // check if is open

  void write_state(const Landscape &l, const double time);

  void close_stream();    

};

#endif
