#include "fftSolver3D.hpp"
#include <cmath>
#include <visa/visa.hpp>
#include "paraxialSimulation.hpp"
#include <iostream>
#include <sstream>
#include <omp.h>
#include <ctime>
//#define PRINT_TIMING_INFO
#define UPDATE_MESSAGE_FREQUENCY 10
using namespace std;

const double PI = acos(-1.0);
typedef visa::Colormaps::Colormap_t cmap_t;
FFTSolver3D::~FFTSolver3D()
{
  if ( planInitialized )
  {
    fftw_destroy_plan( ftforw );
    fftw_destroy_plan( ftback );
  }
}

cdouble FFTSolver3D::kernel( double kx, double ky ) const
{
  double dz = guide->longitudinalDiscretization().step;
  double k = guide->getWavenumber();
  cdouble A(0.0, 0.5*dz/k);
  return exp(-A*(kx*kx + ky*ky) );
}

double FFTSolver3D::spatialFreqX( unsigned int indx, unsigned int size ) const
{
  if ( indx > size/2 )
  {
    indx = size-indx;
  }
  double stepX = guide->transverseDiscretization().step;
  return 2.0*PI*indx/(stepX*size);
}

double FFTSolver3D::spatialFreqY( unsigned int indx, unsigned int size ) const
{
  if ( indx > size/2 )
  {
    indx = size-indx;
  }
  double stepY = guide->verticalDiscretization().step;
  return 2.0*PI*indx/(stepY*size);
}

void FFTSolver3D::solveStep( unsigned int step )
{
  if ( !planInitialized )
  {
    assert( prevSolution->is_square() );
    assert( currentSolution->is_square() );

    prev = reinterpret_cast<fftw_complex*>( prevSolution->memptr() );
    curr = reinterpret_cast<fftw_complex*>( currentSolution->memptr() );
    ftforw = fftw_plan_dft_2d( prevSolution->n_rows, prevSolution->n_cols, prev, curr, FFTW_FORWARD, FFTW_ESTIMATE );
    ftback = fftw_plan_dft_2d( prevSolution->n_rows, prevSolution->n_cols, curr, prev, FFTW_BACKWARD, FFTW_ESTIMATE );
    planInitialized = true;
    clog << endl; // To better for the step update
  }

  #ifdef PRINT_TIMING_INFO
    clock_t start = clock();
  #endif

  propagate();

  #ifdef PRINT_TIMING_INFO
    clog << "Propagation step took: " << static_cast<double>( clock()-start )/CLOCKS_PER_SEC << "sec\n";
    start = clock();
  #endif

  refraction( step );

  #ifdef PRINT_TIMING_INFO
    clog << "Refraction step took: " << static_cast<double>( clock()-start )/CLOCKS_PER_SEC << "sec\n";
  #endif

  applyAbsorbingBC();
}

void FFTSolver3D::propagate()
{
  // NOTE: FFTW assumes row-major ordering, while Armadillo uses column major

  #ifdef PRINT_TIMING_INFO
    clock_t start = clock();
  #endif

  fftw_execute( ftforw ); // prev --> current

  #ifdef PRINT_TIMING_INFO
    clog << "FFT forward took: " << static_cast<double>(clock()-start)/CLOCKS_PER_SEC << " sec\n";
  #endif

  if ( visFourierSpace )
  {
    assert( plots != NULL ); // Just in case, should never happen at this stage
    arma::mat fourierIntensity = arma::abs(*currentSolution );
    plots->get("FourierIntensity").fillVertexArray( fourierIntensity );
    plots->show();
  }

  #pragma omp parallel for
  for ( unsigned int i=0;i<currentSolution->n_cols*currentSolution->n_rows;i++ )
  {
    unsigned int row = i%currentSolution->n_rows;
    unsigned int col = i/currentSolution->n_rows;
    double kx = spatialFreqX( col, currentSolution->n_cols );
    double ky = spatialFreqY( row, currentSolution->n_rows );
    (*currentSolution)(row,col) *= kernel( kx, ky );
  }

  fftw_execute( ftback ); // current --> prev
}

void FFTSolver3D::refraction( unsigned int step )
{
  assert ( step > 0 );
  double stepZ = guide->longitudinalDiscretization().step;
  double wavenumber = guide->getWavenumber();
  double z1 = guide->getZ( step );
  double z0 = guide->getZ( step-1 );
  cdouble im(0.0,1.0);
  const double ZERO = 1E-10;

  // FFTW3: Divide by length to normalize
  double normalization = prevSolution->n_rows*prevSolution->n_cols;

  #pragma omp parallel for
  for ( unsigned int i=0;i<prevSolution->n_cols*prevSolution->n_rows; i++ )
  {
    unsigned int row = i%prevSolution->n_rows;
    unsigned int col = i/prevSolution->n_rows;

    double x = guide->getX(col);
    double y = guide->getY(row);
    double delta, beta, deltaPrev, betaPrev;
    guide->getXrayMatProp( x, y, z1, delta, beta );
    guide->getXrayMatProp( x, y, z0, deltaPrev, betaPrev );

    if (( abs(delta-deltaPrev) > ZERO ) || ( abs(beta-betaPrev) > ZERO ))
    {
        // Wave has crossed a border
        refractionIntegral( x, y, z0, z1, delta, beta );
    }

    //normalization = 1.0;
    (*currentSolution)(row,col) = (*prevSolution)(row,col)*exp( -wavenumber*(beta+im*delta)*stepZ )/normalization;
  }


  if ( visRealSpace )
  {
    assert( plots != NULL ); // Just in case, should never happen at this stage
    arma::mat values = arma::flipud( arma::abs( *currentSolution ) );
    plots->get("Intensity").setCmap( cmap_t::NIPY_SPECTRAL );
    plots->get("Intensity").setOpacity(1.0);
    plots->get("Intensity").setColorLim( intensityMin, intensityMax );
    plots->get("Intensity").setImg( values );
    plots->draw();

    if ( overlayRefractiveIndex )
    {
      plots->get("Intensity").setCmap( cmap_t::GREYSCALE );
      plots->get("Intensity").setOpacity(0.5);
      arma::mat refr(values);
      evaluateRefractiveIndex( refr, z1 );
      double refrMin = refr.min();
      double refrMax = refr.max();
      cout << refrMin << " " << refrMax << endl;
      
		
      plots->get("Intensity").setColorLim( refrMin, refrMax );
      plots->get("Intensity").setImg( refr );
      plots->draw();
    }

    values = arma::flipud( -arma::arg( *currentSolution ) );
    plots->get("Phase").setImg( values );
    plots->draw();
    plots->show();

    if ( createAnimation )
    {
      stringstream ss;
      ss << imageName << imgCounter++ << ".png";
      plots->saveImg( ss.str().c_str() );
    }
  }
}

void FFTSolver3D::visualizeRealSpace()
{
  if ( plots == NULL )
  {
    plots = new visa::WindowHandler;
  }

  visRealSpace = true;
  plots->setLayout(2,1);
  plots->useSeparateDrawing();
  plots->addPlot("Intensity");
  plots->addPlot("Phase");
  plots->get("Intensity").setCmap( cmap_t::NIPY_SPECTRAL );
  plots->get("Phase").setCmap( cmap_t::NIPY_SPECTRAL );
}

void FFTSolver3D::visualizeFourierSpace()
{
  if ( plots == NULL )
  {
    plots = new visa::WindowHandler;
  }
  visFourierSpace = true;
  plots->addPlot("FourierIntensity");
}

void FFTSolver3D::setIntensityMinMax( double min, double max )
{
  intensityMin = min;
  intensityMax = max;

  if ( visRealSpace )
  {
    assert( plots != NULL );
    plots->get("Intensity").setColorLim( min, max );
  }
  else
  {
    clog << "Warning! visualizeRealSpace not called. setIntensityMinMax will have no effet\n";
  }
}

void FFTSolver3D::setPhaseMinMax( double min, double max )
{
  if ( visRealSpace )
  {
    assert( plots != NULL );
    plots->get("Phase").setColorLim( min, max );
  }
  else
  {
    clog << "Warning! visualizeRealSpace not called. setPhaseMinMax will have no effect\n";
  }
}

void FFTSolver3D::refractionIntegral( double x, double y, double z1 , double z2, double &delta, double &beta )
{
  double deltaTemp = 0.0;
  double betaTemp = 0.0;
  guide->getXrayMatProp( x, y, z1, delta, beta );
  guide->getXrayMatProp( x, y, z2, deltaTemp, betaTemp );
  delta += deltaTemp;
  beta += betaTemp;
  double z = z1;
  double dz = (z2-z1)/nStepsInRefrIntegral;
  for ( unsigned int i=1;i<nStepsInRefrIntegral-1;i++ )
  {
    z = z1 + i*dz;
    guide->getXrayMatProp( x, y, z, deltaTemp, betaTemp );
    delta += 2.0*deltaTemp;
    beta += 2.0*betaTemp;
  }
  // NOTE: Multiplication with dz is left out as this is done in the refraction function
  delta /= (2.0*nStepsInRefrIntegral);
  beta /= (2.0*nStepsInRefrIntegral);
}

void FFTSolver3D::reset()
{
  imgCounter = 0;
  Solver3D::reset();
  if ( planInitialized )
  {
    fftw_destroy_plan( ftforw );
    fftw_destroy_plan( ftback );
    planInitialized = false;
  }
}

void FFTSolver3D::evaluateRefractiveIndex( arma::mat &refr, double z ) const
{
  // Set size to the default values in VISA
  //unsigned int width = guide->nodeNumberTransverse();
  //unsigned int height = guide->nodeNumberVertical();
  //refr.set_size( height, width );
  //double dx = ( guide->transverseDiscretization().max - guide->transverseDiscretization().min )/width;
  //double dy = ( guide->verticalDiscretization().max - guide->verticalDiscretization().min )/height;

  double delta, beta;
  for ( unsigned int i=0;i<refr.n_cols;i++ )
  {
    //double x = guide->transverseDiscretization().min + i*dx;
    double x = guide->getX(i);
    for ( unsigned int j=0;j<refr.n_rows;j++ )
    {
      //double y = guide->verticalDiscretization().min + j*dy;
      double y = guide->getY(j);
      guide->getXrayMatProp( x, y, z, delta, beta );
      if ( delta > 1E-10 )
      {
        refr(j,i) = 0.0;
      }
      else
      {
        refr(j,i) = 1.0;
      }
      //refr(j,i) = delta;
    }
  }
  refr = arma::flipud( refr );
}

void FFTSolver3D::storeImages( const char* prefix )
{
  imageName = prefix;
  createAnimation = true;
  clog << "Intensity images will be store with prefix: " << imageName << endl;
}

void FFTSolver3D::absorbingBC( double width, double dampingLength )
{
  if ( guide == NULL )
  {
    throw( runtime_error("No waveguide set! The solver needs first to be passed to a waveguide object. Then call this function!") );
  }

  unsigned int nPixX = width/guide->transverseDiscretization().step + 1;
  unsigned int nPixY = width/guide->verticalDiscretization().step + 1;

 clog << "Absorbing BC: " << nPixX << " px in x-direction. " << nPixY << " px in y-direction.\n";

  double invDampingX = guide->transverseDiscretization().step/dampingLength;
  double invDampingY = guide->verticalDiscretization().step/dampingLength;

  clog << "Absorbing BC: Damping length " << static_cast<int>(1.0/invDampingX) << " px in x-direction ";
  clog << static_cast<int>(1.0/invDampingY) << " px in y-direction\n";

  absorbX.setThickness( nPixX );
  absorbY.setThickness( nPixY );
  absorbX.setInverseDampingLength( invDampingX );
  absorbY.setInverseDampingLength( invDampingY );
}

void FFTSolver3D::applyAbsorbingBC()
{
  #pragma omp parallel
  {
    DimGetter<ApplyDim_t::COL> getter;
    #pragma omp for
    for ( unsigned int i=0;i<currentSolution->n_cols;i++ )
    {
      getter.fixed = i;
      absorbY.apply( *currentSolution, getter );
    }
  }

  #pragma omp parallel
  {
    DimGetter<ApplyDim_t::ROW> getter;
    #pragma omp for
    for ( unsigned int i=0;i<currentSolution->n_rows;i++ )
    {
      getter.fixed = i;
      absorbX.apply( *currentSolution, getter );
    }
  }
}
