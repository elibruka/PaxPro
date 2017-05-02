/* PAXPRO PYTHON INTERFACE */

%include <stl.i>
%include "exception.i"
%include "std_string.i"
%include "std_map.i"
%module(directors="1") pypaxpro
%{
  #include "paraxialSimulation.hpp"
  #include "genericScattering.hpp"
  #include "tetraGeometry.hpp"
  #include "controlFile.hpp"
  #include "materialFunction.hpp"

%}

namespace std{
%template(region) map<string,XrayMatProperty>;
}

%include "paraxialSimulation.hpp"
%include "genericScattering.hpp"
%include "controlFile.hpp"
%include "materialFunction.hpp"
%include "tetraGeometry.hpp"
