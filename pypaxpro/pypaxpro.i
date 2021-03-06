/* PAXPRO PYTHON INTERFACE */

%include <stl.i>
%include "exception.i"
%module pypaxpro
%{
  #define SWIG_FILE_WITH_INIT
  #include "paraxialSimulation.hpp"
  #include "genericScattering.hpp"
  #include "materialFunction.hpp"
  #include "shapes.hpp"
  #include "geometry.hpp"
  #include "paraxialSource.hpp"
  #include "gaussianBeam.hpp"
  #include "postProcessing.hpp"
  #include "postProcessMod.hpp"
  #include "solver.hpp"
  #include "solver2D.hpp"
  #include "solver3D.hpp"
  #include "crankNicholson.hpp"
  #include "fftSolver2D.hpp"
  #include "fftSolver3D.hpp"
  #include "alternatingDirectionSolver.hpp"
  #include "planeWave.hpp"
%}

%include "paraxialSimulation.hpp"
%include "genericScattering.hpp"
%include "shapes.hpp"
%include "geometry.hpp"
%include "materialFunction.hpp"
%include "paraxialSource.hpp"
%include "gaussianBeam.hpp"
%include "postProcessing.hpp"
%include "postProcessMod.hpp"
%include "solver.hpp"
%include "solver2D.hpp"
%include "solver3D.hpp"
%include "crankNicholson.hpp"
%include "fftSolver2D.hpp"
%include "fftSolver3D.hpp"
%include "alternatingDirectionSolver.hpp"
%include "planeWave.hpp"
