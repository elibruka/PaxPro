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
  //arma::cube tempRes = arma::abs( solver.getSolution3D() );
  const arma::cx_cube& sol = solver.getSolution3D();
  res.set_size(sol.n_rows, sol.n_cols, sol.n_slices );
  double maxval = abs( sol.max() ); // Armadillo uses max

  for ( unsigned int i=0;i<sol.n_slices;i++ )
  for ( unsigned int j=0;j<sol.n_cols;j++ )
  for ( unsigned int k=0;k<sol.n_rows;k++ )
  {
    res(k,j,i) = 255.0*abs( sol(k,j,i) )/maxval;
  }
}

void post::LogIntensityUint8::result( const Solver &solver, arma::Cube<unsigned char> &res )
{
  const arma::cx_cube& sol = solver.getSolution3D();
  res.set_size( sol.n_rows,  sol.n_cols,  sol.n_slices );
  double maxvalInSolution = log( abs(sol.max()) ); // Armadillo used absolute value
  double minvalInSolution = log( abs(sol.min()) ); // Armadillo used absolute value

  // Set upper range of the solution
  double max, min;
  if ( maxvalSet )
  {
    max = maxvalInSolution > maxval ? maxval:maxvalInSolution;
  }
  else
  {
    max = maxvalInSolution;
  }

  if ( minvalSet )
  {
    min = minvalInSolution < minval ? minval:minvalInSolution;
  }
  else
  {
    min = minvalInSolution;
  }

  for ( unsigned int i=0;i<sol.n_slices;i++ )
  for ( unsigned int j=0;j<sol.n_cols;j++ )
  for ( unsigned int k=0;k<sol.n_rows;k++ )
  {
    if ( log(abs(sol(k,j,i))) > max )
    {
      res(k,j,i) = 255;
    }
    else if ( log(abs(sol(k,j,i))) < min )
    {
      res(k,j,i) = 0;
    }
    else
    {
      res(k,j,i) = 255*( log(abs(sol(k,j,i)))-min )/(max-min);
    }
  }
}

void post::LogIntensityUint8::setMinValue( double min )
{
  minval = log(min);
  minvalSet = true;
}

void post::LogIntensityUint8::setMaxValue( double max )
{
  maxval = log(max);
  maxvalSet = true;
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
  if ( resizeMatrices )
  {
    arma::mat copy(res);
    resizeMatrix( copy, res );
  }
}
