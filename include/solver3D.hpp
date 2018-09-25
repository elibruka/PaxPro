#ifndef SOLVER_3D_H
#define SOLVER_3D_H
#include <armadillo>
#include <climits>
#include "solver.hpp"
#include <visa/visa.hpp>
#include "config.h"

class ParaxialSimulation;

class Solver3D: public Solver
{
public:
  Solver3D( const char* name ):Solver(name, Dimension_t::THREE_D){};
  virtual ~Solver3D();

  /** Set the simulation routine */
  void setSimulator( ParaxialSimulation &sim ) override;

  /** Set the initial conditions */
  void setInitialConditions( const arma::cx_mat &values ) override;

  /** Sets the time in seconds between status messages */
  void setTimeBetweenStatus( unsigned int sec ){ secBetweenStatusMessage = sec; };

  /** Propagate one step */
  virtual void step() override;

  /** Solve the entire system */
  virtual void solve() override;

  /** Returns the complex solution */
  virtual const arma::cx_cube& getSolution3D() const override;
  
  virtual const arma::cube& getRefractiveIndex3D() const override { return *refractiveIndex; };

  /** Returns the last solution */
  virtual const arma::cx_mat& getLastSolution3D() const override{ return *prevSolution; };

  /** Use real time visualization */
  void realTimeVisualization();

  /** Set plot limits */
  void setPlotLimits( double intensityMin, double intensityMax, double phaseMin, double phaseMax );

  /** Set the plot limits */
  void setPlotLimits( double intensityMin, double intensityMax, double phaseMin, double phaseMax, bool intensityLogScale );

  /** Updates the dimensions of the arrays */
  virtual void updateDimensionsOfArrays() override;
protected:
  arma::cube *refractiveIndex{NULL};
  arma::mat *refractiveIndexSlice{NULL};
  arma::cx_cube *solution{NULL};
  arma::cx_mat *currentSolution{NULL};
  arma::cx_mat *prevSolution{NULL};
  bool logScaleIntensity{false};

  // Some parameters
  unsigned int Nx, Ny, Nz;

  /** Filter the transverse signal */
  void filterTransverse( arma::cx_mat &mat );

  /** Copy the current solution to the previous array */
  void copyCurrentSolution( unsigned int step );

  /** Solver specific function to propagate one step */
  virtual void solveStep( unsigned int step ) = 0;

  unsigned int secBetweenStatusMessage{15};

  /** Real-time visualization */
  bool realTimeVis{false};
  
    /** Evaluates the refractive index for the purpose of overlay */
  void evaluateRefractiveIndex( arma::mat &refr, double z ) const;

  visa::WindowHandler *plots{NULL};
};
#endif
