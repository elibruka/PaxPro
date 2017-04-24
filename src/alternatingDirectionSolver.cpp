#include "alternatingDirectionSolver.hpp"
#include "paraxialSimulation.hpp"
#include <cassert>
#include <omp.h>
#define ADI_DEBUG

using namespace std;
void ADI::xImplicit( unsigned int step )
{
  assert( guide != NULL );
  cdouble *diag = new cdouble[Nx*Ny];
  cdouble *subdiag = new cdouble[Nx*Ny-1];
  cdouble *rhs = new cdouble[Nx*Ny];
  cdouble im(0.0,1.0);

  dx = guide->transverseDiscretization().step;
  dy = guide->verticalDiscretization().step;
  double dz = guide->longitudinalDiscretization().step/2.0;
  k = guide->getWavenumber();

  double z = guide->getZ(step);
  double delta, beta;

  #ifdef ADI_DEBUG
    clog << "Building matrix...\n";
  #endif

  // Build matrix system
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
      unsigned int j = indx%Nx;
      unsigned int i = indx/Nx;
      double x = guide->getX(j);
      double y = guide->getY(i);
      guide->getXrayMatProp(x,y,z,delta,beta);
      //unsigned int indx = i*Nx+j;
      diag[indx] = 1.0/dz + 0.5*im/(k*dx*dx) + 0.5*beta*k + 0.5*im*delta*k;
      rhs[indx] = (-0.5*im/(k*dx*dx) - im/(k*dy*dy) - 0.5*beta*k - 0.5*im*delta*k + 1.0/dz)*(*prevSolution)(j,i);
      if ( j>0 )
      {
        subdiag[indx-1] = -0.25*im/(k*dx*dx);
        rhs[indx] += 0.25*im*(*prevSolution)(j-1,i)/(k*dx*dx);
      }
      if ( j<Nx-1 )
      {
        rhs[indx] += 0.25*im*(*prevSolution)(j+1,i)/(k*dx*dx);
      }
      if ( i > 0 )
      {
        rhs[indx] += 0.5*im*(*prevSolution)(j,i-1)/(k*dy*dy);
      }
      if ( i<Ny-1 )
      {
        rhs[indx] += 0.5*im*(*prevSolution)(j,i+1)/(k*dy*dy);
      }
  }
  if ( useTBC ) applyTBC( diag, rhs, ImplicitDirection_t::X );

  #ifdef ADI_DEBUG
    clog << "Solving matrix...\n";
  #endif
  // Solve the system
  matrixSolver.solve( diag, subdiag, rhs, Nx*Ny );

  #ifdef ADI_DEBUG
    clog << "Transferring solution to currentSolution array...\n";
  #endif
  
  // Copy the solution to the current solution
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
    unsigned int j = indx%Nx;
    unsigned int i = indx/Nx;
    (*currentSolution)(j,i) = diag[indx];
  }
  delete [] diag;
  delete [] subdiag;
  delete [] rhs;
}

void ADI::yImplicit( unsigned int step )
{
  assert( guide != NULL );
  cdouble *diag = new cdouble[Nx*Ny];
  cdouble *subdiag = new cdouble[Nx*Ny-1];
  cdouble *rhs = new cdouble[Nx*Ny];
  cdouble im(0.0,1.0);

  dx = guide->transverseDiscretization().step;
  dy = guide->verticalDiscretization().step;
  double dz = guide->longitudinalDiscretization().step/2.0;
  k = guide->getWavenumber();

  double z = guide->getZ(step) + dz;
  double delta, beta;

  // Build matrix system
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
      unsigned int i = indx/Ny;
      unsigned int j = indx%Ny;
      double x = guide->getX(i);
      double y = guide->getY(j);
      guide->getXrayMatProp(x,y,z,delta,beta);
      diag[indx] = 1.0/dz + 0.5*im/(k*dy*dy) + 0.5*beta*k + 0.5*im*delta*k;
      rhs[indx] = (-0.5*im/(k*dy*dy) - im/(k*dx*dx) - 0.5*beta*k - 0.5*im*delta*k + 1.0/dz)*(*prevSolution)(i,j);
      if ( j>0 )
      {
        subdiag[indx-1] = -0.25*im/(k*dy*dy);
        rhs[indx] += 0.25*im*(*prevSolution)(i,j-1)/(k*dy*dy);
      }
      if ( j<Ny-1 )
      {
        rhs[indx] += 0.25*im*(*prevSolution)(i,j+1)/(k*dy*dy);
      }
      if ( i > 0 )
      {
        rhs[indx] += 0.5*im*(*prevSolution)(i-1,j)/(k*dx*dx);
      }
      if ( i<Ny-1 )
      {
        rhs[indx] += 0.5*im*(*prevSolution)(i+1,j)/(k*dx*dx);
      }
  }
  if ( useTBC ) applyTBC( diag, rhs, ImplicitDirection_t::Y );

  // Solve the system
  matrixSolver.solve( diag, subdiag, rhs, Nx*Ny );

  // Copy the solution to the current solution
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
    unsigned int i = indx/Ny;
    unsigned int j = indx%Ny;
    (*currentSolution)(i,j) = diag[indx];
  }
  delete [] diag;
  delete [] subdiag;
  delete [] rhs;
}

void ADI::solveStep( unsigned int step )
{
  #ifdef ADI_DEBUG
    clog << "Solving X implicitly...\n";
  #endif
  xImplicit(step);

  #ifdef ADI_DEBUG
    clog << "Copy solution to previous array...\n";
  #endif
  copyCurrentSolution(step);

  #ifdef ADI_DEBUG
    clog << "Solving Y implicitly...\n";
  #endif
  yImplicit(step);
}

void ADI::applyTBC( cdouble diag[], cdouble rhs[], ImplicitDirection_t dir )
{
  cdouble im(0.0,1.0);
  switch( dir )
  {
    case ImplicitDirection_t::X:
      // X-direction is implicit
      #pragma omp parallel for
      for ( unsigned int i=0;i<Ny;i++ )
      {
        cdouble kdx = -im*log((*prevSolution)(Nx-1,i)/(*prevSolution)(Nx-2,i));
        if ( kdx.real() < 0.0 ) kdx = 0.0;
        unsigned int indx = i*Nx+Nx-1;
        diag[indx] -= 0.25*im*exp(im*kdx)/(k*dx*dx);
        rhs[indx] += 0.25*im*exp(im*kdx)*(*prevSolution)(Nx-1,i)/(k*dx*dx);

        // Other side
        indx = i*Nx;
        kdx = im*log((*prevSolution)(0,i)/(*prevSolution)(1,i));
        if ( kdx.real() < 0.0 ) kdx = 0.0;
        diag[indx] -= 0.25*im*exp(-im*kdx)/(k*dx*dx);
        rhs[indx] += 0.25*im*exp(-im*kdx)*(*prevSolution)(0,i)/(k*dx*dx);
      }

      // In y-direction
      #pragma omp parallel for
      for ( unsigned int i=0;i<Nx;i++ )
      {
        // y=Ny-1
        cdouble kdy = -im*log((*prevSolution)(i,Ny-1)/(*prevSolution)(i,Ny-2));
        if ( kdy.real() < 0.0 ) kdy = 0.0;
        unsigned int indx = (Ny-1)*Nx + i;
        rhs[indx] += 0.5*im*exp(im*kdy)*(*prevSolution)(i,Ny-1)/(k*dy*dy);

        // Other side y = 0
        indx = i;
        kdy = im*log((*prevSolution)(i,0)/(*prevSolution)(i,1));
        if ( kdy.real() < 0.0 ) kdy = 0.0;
        rhs[indx] +=  0.5*im*exp(-im*kdy)*(*prevSolution)(i,0)/(k*dy*dy);
      }
      break;
    case ImplicitDirection_t::Y:
    // Y-direction is implicit
    #pragma omp parallel for
    for ( unsigned int i=0;i<Nx;i++ )
    {
      cdouble kdy = -im*log((*prevSolution)(i,Ny-1)/(*prevSolution)(i,Ny-2));
      if ( kdy.real() < 0.0 ) kdy = 0.0;
      unsigned int indx = i*Ny+Ny-1;
      diag[indx] -= 0.25*im*exp(im*kdy)/(k*dy*dy);
      rhs[indx] += 0.25*im*exp(im*kdy)*(*prevSolution)(i,Ny-1)/(k*dy*dy);

      // Other side
      indx = i*Ny;
      kdy = im*log((*prevSolution)(i,0)/(*prevSolution)(i,1));
      if ( kdy.real() < 0.0 ) kdy = 0.0;
      diag[indx] -= 0.25*im*exp(-im*kdy)/(k*dy*dy);
      rhs[indx] += 0.25*im*exp(-im*kdy)*(*prevSolution)(i,0)/(k*dy*dy);
    }

    // In x-direction
    #pragma omp parallel for
    for ( unsigned int i=0;i<Ny;i++ )
    {
      // y=Ny-1
      cdouble kdx = -im*log((*prevSolution)(Nx-1,i)/(*prevSolution)(Nx-2,i));
      if ( kdx.real() < 0.0 ) kdx = 0.0;
      unsigned int indx = (Nx-1)*Ny + i;
      rhs[indx] += 0.5*im*exp(im*kdx)*(*prevSolution)(Nx-1,i)/(k*dx*dx);

      // Other side y = 0
      indx = i;
      kdx = im*log((*prevSolution)(0,i)/(*prevSolution)(1,i));
      if ( kdx.real() < 0.0 ) kdx = 0.0;
      rhs[indx] +=  0.5*im*exp(-im*kdx)*(*prevSolution)(0,i)/(k*dx*dx);
    }
    break;

  }
}
