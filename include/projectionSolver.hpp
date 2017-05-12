#ifndef PROJECTION_3D_SOLVER_H
#define PROJECTION_3D_SOLVER_H
#include "solver3D.hpp"

class ProjectionSolver: public Solver3D
{
public:
  ProjectionSolver(): Solver3D("ProjectionSolver"){};

protected:
  /** Propagates the solution one step */
  void solveStep( unsigned int step );
};
#endif
