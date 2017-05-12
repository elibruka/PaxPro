#include "projectionSolver.hpp"
#include "paraxialSimulation.hpp"
#include <complex>

using namespace std;
typedef complex<double> cdouble;

void ProjectionSolver::solveStep( unsigned int step )
{
  double stepZ = guide->longitudinalDiscretization().step;
  double wavenumber = guide->getWavenumber();
  double z = guide->getZ( step );
  cdouble im(0.0,1.0);

  #pragma omp parallel for
  for ( unsigned int i=0;i<prevSolution->n_cols*prevSolution->n_rows; i++ )
  {
    unsigned int row = i%prevSolution->n_rows;
    unsigned int col = i/prevSolution->n_rows;

    double x = guide->getX(col);
    double y = guide->getY(row);
    double delta, beta;
    guide->getXrayMatProp( x, y, z, delta, beta );

    (*currentSolution)(row,col) = (*prevSolution)(row,col)*exp( -wavenumber*(beta+im*delta)*stepZ );
  }
}
