shell := bash
.oneshell:
.shellflags := -eu -o pipefail -c
.delete_on_error:
makeflags += --warn-undefined-variables
makeflags += --no-builtin-rules

ifeq ($(origin .RECIPEPREFIX), undefined)
  $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
endif
.RECIPEPREFIX = >


objt = parameters.cpp patch.cpp landscape.cpp writer.cpp simulation.cpp main.cpp
cc = g++
ws = -Wall -Wextra -Weffc++ -DDEBUG
ncap = -DNOCAP
nmig = -DNOMIGR
cl = -std=c++17 -march=native
o0 = -O2
ll = -lboost_program_options
nimi = sim

# Default - debug printing (i.e. verbose) build
debug : $(objt)
> $(cc) $(objt) $(cl) $(ws) $(ll) -o $(nimi)

# Carrying capacity off debug build
dnocap: $(objt)
> $(cc) $(objt) $(cl) $(ws) $(ncap) $(ll) -o $(nimi)

# Migration off debug build
dnomigr: $(objt)
> $(cc) $(objt) $(cl) $(ws) $(nmig) $(ll) -o $(nimi)

# Migr and cc off debug build
dnn: $(objt)
> $(cc) $(objt) $(cl) $(ws) $(ncap) $(nmig) $(ll) -o $(nimi)

# optimised build for simulations
opt : $(objt)
> $(cc) $(objt) $(cl) $(o0) $(ll) -o $(nimi)
