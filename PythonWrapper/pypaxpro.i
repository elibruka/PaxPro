/* PAXPRO PYTHON INTERFACE */

%include <stl.i>
%include "exception.i"
%module pypaxpro
%{
  #include "paraxialSimulation.hpp"
  #include "genericScattering.hpp"
  #include "materialFunction.hpp"
  #include "shapes.hpp"
  #include "geometry.hpp"
%}

%include "paraxialSimulation.hpp"
%include "genericScattering.hpp"
%include "shapes.hpp"
%include "geometry.hpp"
%include "materialFunction.hpp"
