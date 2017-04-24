#include "postProcessMod.hpp"
#include "solver.hpp"
#include <iostream>

using namespace std;

void post::Intensity::result( const Solver &solver, arma::mat &res )
{
  res = arma::abs( solver.getSolution() );
}

void post::Intensity::result( const Solver &solver, arma::cube &res )
{
  res = arma::abs( solver.getSolution3D() );
}

void post::IntensityUint8::result( const Solver &solver, arma::Cube<unsigned char> &res )
{
  arma::cube tempRes = arma::abs( solver.getSolution3D() );
  res.set_size(tempRes.n_rows, tempRes.n_cols, tempRes.n_slices );
  double maxval = tempRes.max();
  
  for ( unsigned int i=0;i<tempRes.n_slices;i++ )
  for ( unsigned int j=0;j<tempRes.n_cols;j++ )
  for ( unsigned int k=0;k<tempRes.n_rows;k++ )
  {
    res(k,j,i) = 256.0*tempRes(k,j,i)/maxval;
  }
}

void post::Phase::result( const Solver &solver, arma::mat &res )
{
  res = arma::arg( solver.getSolution() );

  if ( resizeMatrices )
  {
    arma::mat copy(res);
    resizeMatrix( copy, res );
  }
}

void post::Phase::result( const Solver &solver, arma::cube &res )
{
  res = arma::arg( solver.getSolution3D() );
}

void post::ExitField::result( const Solver &solver, arma::vec &res )
{
  res = arma::real( solver.getLastSolution() );
}

void post::ExitField::result( const Solver &solver, arma::mat&res )
{
  res = arma::real( solver.getLastSolution3D() );

  if ( resizeMatrices )
  {
    arma::mat copy(res);
    resizeMatrix( copy, res );
  }
}

void post::ExitIntensity::result( const Solver &solver, arma::vec &res )
{
  res = arma::pow( arma::abs( solver.getLastSolution() ), 2 );
}

void post::ExitIntensity::result( const Solver &solver, arma::mat &res )
{
  res = arma::pow( arma::abs( solver.getLastSolution3D() ), 2 );

  if ( resizeMatrices )
  {
    arma::mat copy(res);
    resizeMatrix( copy, res );
  }
}

void post::ExitPhase::result( const Solver &solver, arma::vec &res )
{
  res = arma::arg( solver.getLastSolution() );
}

void post::ExitPhase::result( const Solver &solver, arma::mat &res )
{
  res = arma::arg( solver.getLastSolution3D() );
}
