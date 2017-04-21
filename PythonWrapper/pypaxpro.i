/* PAXPRO PYTHON INTERFACE */

%include <stl.i>
%include "exception.i"
%module(directors="1") pypaxpro
%{
  #include "paraxialSimulation.hpp"
  #include "genericScattering.hpp"
  #include "materialFunction.hpp"
  #include "controlFile.hpp"
%}

%include "paraxialSimulation.hpp"
%include "genericScattering.hpp"
%include "controlFile.hpp"

%feature("director") MaterialFunction;
%include "materialFunction.hpp"
