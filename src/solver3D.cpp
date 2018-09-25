#include "solver3D.hpp"
#include <cassert>
#include "paraxialSimulation.hpp"
#include <omp.h>
#include <ctime>
#include <chrono>
#include <iostream>

using namespace std;

Solver3D::~Solver3D()
{
  delete plots; plots=NULL;
  delete solution; solution=NULL;
  delete currentSolution; currentSolution=NULL;
  delete prevSolution; prevSolution=NULL;
  delete refractiveIndex; refractiveIndex=NULL;
  delete refractiveIndexSlice; refractiveIndexSlice = NULL;
}

void Solver3D::setSimulator( ParaxialSimulation &sim )
{
  Solver::setSimulator( sim );
  Nx = guide->nodeNumberTransverse();
  Nz = guide->nodeNumberLongitudinal();
  Ny = Nx;

  unsigned int downSampledX = Nx/guide->transverseDiscretization().downsamplingRatio;
  unsigned int downSampledZ = Nz/guide->longitudinalDiscretization().downsamplingRatio;

  // Deallocate if already allocated
  delete solution; solution=NULL;
  delete prevSolution; prevSolution=NULL;
  delete currentSolution; currentSolution=NULL;
  delete refractiveIndex; refractiveIndex=NULL;
  delete refractiveIndexSlice; refractiveIndexSlice=NULL;

  prevSolution = new arma::cx_mat(Ny,Nx);
  currentSolution = new arma::cx_mat(Ny,Nx);
  solution = new arma::cx_cube( downSampledX, downSampledX, downSampledZ+1 );
  refractiveIndex = new arma::cube(downSampledX, downSampledX, downSampledZ+1 );
  refractiveIndexSlice = new arma::mat(Nx,Ny);

  if ( downSampledX != Nx )
  {
    filter.setSourceSize( Nx );
    filter.setTargetSize( downSampledX );
    filter.computeFilterCoefficients( kernel );
  }
}

void Solver3D::filterTransverse( arma::cx_mat &mat )
{
  assert( Nx == Ny ); // If this is the case, the filter coefficients does not need to be recomputed

  visa::ArmaGetter<cdouble, visa::ArmaMatrix_t::COL> colGetter;

  #pragma omp parallel for
  for ( unsigned int i=0;i<mat.n_rows;i++ )
  {
    colGetter.fixedIndx = i;
    filter.filterArray( mat, colGetter );
  }

  unsigned int maxThreads = omp_get_num_threads();
  vector< visa::ArmaGetter<cdouble, visa::ArmaMatrix_t::ROW> > rowGetters;
  for ( unsigned int i=0;i<maxThreads;i++ )
  {
    rowGetters.push_back( visa::ArmaGetter<cdouble, visa::ArmaMatrix_t::ROW>() );
  }

  #pragma omp parallel
  {
    unsigned int id = omp_get_thread_num();
    #pragma omp for
    for ( unsigned int i=0;i<mat.n_cols;i++ )
    {
      rowGetters[id].fixedIndx = i;
      filter.filterArray( mat, rowGetters[id] );
    }
  }
}

void Solver3D::copyCurrentSolution( unsigned int step )
{
  assert( currentSolution->n_rows == prevSolution->n_rows );
  assert( currentSolution->n_cols == prevSolution->n_cols );

  *prevSolution = *currentSolution;
  

  if (step%guide->longitudinalDiscretization().downsamplingRatio == 0 )
  {
    unsigned int currZ = step/guide->longitudinalDiscretization().downsamplingRatio;
    if (( currentSolution->n_rows != solution->n_rows ) || ( currentSolution->n_cols != solution->n_cols ))
    {
      // TODO: Old implementation of downsampling, remove in future
      //filterTransverse( *currentSolution );
    }

    double deltaX = static_cast<double>( currentSolution->n_rows )/static_cast<double>( solution->n_rows );
    double deltaY = static_cast<double>( currentSolution->n_cols )/static_cast<double>( solution->n_cols );

    unsigned int maxX = deltaX*( solution->n_rows - 1 );
    unsigned int maxY = deltaY*( solution->n_cols - 1 );
    assert( maxX < currentSolution->n_rows );
    assert( maxY < currentSolution->n_cols );

    // Downsample the array
    if ( currZ < solution->n_slices )
    {
	  evaluateRefractiveIndex(*refractiveIndexSlice,currZ);
      #pragma omp parallel for
      for ( unsigned int i=0;i<solution->n_rows*solution->n_cols;i++ )
      {
        unsigned int row = i%solution->n_rows;
        unsigned int col = i/solution->n_rows;
        //(*solution)(row,col,currZ) = (*currentSolution)( row*deltaX, col*deltaY );
        (*solution)(row,col,currZ) = arma::sum( arma::sum(
          currentSolution->submat( row*deltaX, col*deltaY, row*deltaX+deltaX-1, col*deltaY+deltaY-1) ))/(deltaX*deltaY);
          
          (*refractiveIndex)(row,col,currZ) = arma::sum( arma::sum(
          refractiveIndexSlice->submat( row*deltaX, col*deltaY, row*deltaX+deltaX-1, col*deltaY+deltaY-1) ))/(deltaX*deltaY);
      }
    }
    else
    {
      static bool printWarning = true;
      if ( printWarning )
      {
        cout << "Warning! The z-index is out of bounds. Stops saving fields...\n";
        printWarning = false;
      }
    }
  }
}

void Solver3D::setInitialConditions( const arma::cx_mat &values )
{

  if (( currentSolution == NULL ) || ( prevSolution == NULL ) || ( solution == NULL ))
  {
    throw ( runtime_error("The function setSimulator needs to be called before setInitialConditions!") );
  }

  if (( values.n_rows != currentSolution->n_rows ) || ( values.n_cols != currentSolution->n_cols ))
  {
    stringstream ss;
    ss << "Dimension of matrices does not match!\n";
    ss << "Given: Nrows: " << values.n_rows << " Ncols: " << values.n_cols;
    ss << "\nRequired: Nrows: " << currentSolution->n_rows << " Ncols: " << currentSolution->n_cols;
    throw( runtime_error( ss.str() ) );
  }

  (*currentSolution) = values;
  copyCurrentSolution( 0 );
}

void Solver3D::step()
{
  if ( realTimeVis )
  {
    arma::mat values = arma::flipud( arma::abs( *currentSolution ) );

    if ( logScaleIntensity )
    {
      values = arma::log10(values);
    }
    plots->get("Intensity").setImg(values);
    values = arma::flipud( -arma::arg( *currentSolution ) );
    plots->get("Phase").setImg( values );
    plots->show();
  }

  solveStep( currentStep );
  copyCurrentSolution( currentStep++ );
}

void Solver3D::solve()
{
  auto lastTime = chrono::steady_clock::now();
  auto now = lastTime;
  bool printEstimatedTime = true;
  for ( unsigned int i=1;i<guide->nodeNumberLongitudinal();i++ )
  {
    step();

    now = chrono::steady_clock::now();
    chrono::duration<double> elapsedSec = now - lastTime;
    if ( elapsedSec > chrono::duration<double>(secBetweenStatusMessage) )
    {
      clog << "Propagation step: " << i << " of " << guide->nodeNumberLongitudinal() << endl;
      lastTime = chrono::steady_clock::now();
      if ( printEstimatedTime )
      {
        double timePerStep = elapsedSec.count()/i;
        int totalSec = timePerStep*guide->nodeNumberLongitudinal();
        int hours = totalSec/3600;
        int min = (totalSec-3600*hours)/60;
        int sec = totalSec-3600*hours-60*min;
        clog << "Estimated time: " << hours << "h " << min << "min " << sec << "s\n";
        printEstimatedTime = false;
      }
    }
  }

  // TODO: Should one support downsampling in 3D. This will require an extra copy
  //filterInLongitudinalDirection();
  //downSampleLongitudinalDirection();
}

const arma::cx_cube& Solver3D::getSolution3D() const
{
  return *solution;
}

void Solver3D::realTimeVisualization()
{
  realTimeVis = true;
  delete plots;
  plots = new visa::WindowHandler;
  plots->setLayout(2,1);
  plots->addPlot("Intensity");
  plots->addPlot("Phase");
  typedef visa::Colormaps::Colormap_t cmap_t;
  plots->get("Intensity").setCmap( cmap_t::VIRIDIS );
  plots->get("Phase").setCmap( cmap_t::NIPY_SPECTRAL );
  plots->get("Intensity").setColorLim(0.0,1.1);
  plots->get("Phase").setColorLim(-3.14159,3.14159);
}

void Solver3D::setPlotLimits( double intensityMin, double intensityMax, double phaseMin, double phaseMax )
{
  setPlotLimits( intensityMin, intensityMax, phaseMin, phaseMax, false );
}

void Solver3D::setPlotLimits( double intensityMin, double intensityMax, double phaseMin, double phaseMax, bool intensityLogScale )
{
  if ( plots == NULL ) return;
  logScaleIntensity = intensityLogScale;
  if ( intensityLogScale )
  {
    plots->get("Intensity").setColorLim( log10(intensityMin), log10(intensityMax) );
  }
  else
  {
    plots->get("Intensity").setColorLim( intensityMin, intensityMax );
  }
  plots->get("Phase").setColorLim( phaseMin, phaseMax );
}

void Solver3D::updateDimensionsOfArrays()
{
  // The only thing that is done when the simulator is set in this class is to
  // Assign a pointer to the simulation and set the correct size of the solution arrays
  // Hence, this function only needs to call the setSimulator function
  this->setSimulator( *guide );
}


void Solver3D::evaluateRefractiveIndex( arma::mat &refr, double z ) const
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
      refr(j,i) = delta;
    }
  }
  refr = arma::flipud( refr );
}
