#include "genericScattering.hpp"
#include <stdexcept>
//#define PRINT_DEBUG

using namespace std;
GenericScattering::GenericScattering( const char* name ): ParaxialSimulation(name)
{
  gbeam.setDim( ParaxialSource::Dim_t::THREE_D );
}

GenericScattering::~GenericScattering()
{
  delete reference; reference=NULL;
}

void GenericScattering::setMaxScatteringAngle( double anglemax )
{
  ff.setAngleRange( -anglemax, anglemax );
}

void GenericScattering::init()
{
  reset();
  isReferenceRun = true;
  #ifdef PRINT_DEBUG
    clog << "Initialization started...\n";
    clog << "Set export dimensions and padlength...\n";
  #endif

  ef.setExportDimensions( exportNx, exportNy );
  ei.setExportDimensions( exportNx, exportNy );
  ep.setExportDimensions( exportNx, exportNy );
  ff.setPadLength( FFTPadLength );
  ff.setExportDimensions( exportNx, exportNy );

  #ifdef PRINT_DEBUG
    clog << "Set reference solution array...\n";
  #endif

  #ifdef PRINT_DEBUG
    clog << "Set discretization...\n";
  #endif

  setTransverseDiscretization( xmin, xmax, dx, downSampleX );
  setVerticalDiscretization( ymin, ymax, dy );
  setLongitudinalDiscretization( zmin, zmax, (zmax-zmin)/3.0, 1 ); // Settings for reference run

  #ifdef PRINT_DEBUG
    clog << "Set wavelength...\n";
  #endif

  setWaveLength( wavelength );
  gbeam.setWavelength( wavelength );

  #ifdef PRINT_DEBUG
    clog << "Set visualization phase min/max and intensity min/max...\n";
  #endif

  if ( realTimeVisualization )
  {
    switch ( propagator )
    {
      case SolverType_t::ADI:
        adisolver.realTimeVisualization();
        adisolver.useTBC = true;
        break;
      case SolverType_t::FFT:
        fft3Dsolver.visualizeRealSpace();
        fft3Dsolver.setIntensityMinMax( intensityMin, intensityMax );
        fft3Dsolver.setPhaseMinMax( phaseMin, phaseMax );
        // fft3Dsolver.overlayGeometry();
        break;
    }
    /*
    if ( useFFTSolver )
    {
      fft3Dsolver.visualizeRealSpace();
      fft3Dsolver.setIntensityMinMax( intensityMin, intensityMax );
      fft3Dsolver.setPhaseMinMax( phaseMin, phaseMax );
    }
    else
    {
      adisolver.realTimeVisualization();
      adisolver.useTBC = true;
    }*/
  }

  if ( imgname != "" )
  {
    fft3Dsolver.storeImages( imgname.c_str() );
  }

  #ifdef PRINT_DEBUG
    clog << "Set solver and post processing modules...\n";
  #endif

  switch ( propagator )
  {
    case SolverType_t::ADI:
      setSolver( adisolver );
      break;
    case SolverType_t::FFT:
      setSolver( fft3Dsolver );
      break;
    case SolverType_t::PROJ:
      setSolver( projSolver );
      break;
  }
  /*
  if ( useFFTSolver ) setSolver( fft3Dsolver );
  else setSolver( adisolver );*/

  #ifdef PRINT_DEBUG
    clog << "Set boundary conditions from Gaussian beam...\n";
  #endif

  setBoundaryConditions( gbeam );

  if ( isFirstTime )
  {
    *this << ef << ei << ep << ff;
    isFirstTime = false;

    for ( unsigned int i=0;i<userDefinedPPM.size();i++ )
    {
      *this << *userDefinedPPM[i];
    }
  }

  #ifdef PRINT_DEBUG
    clog << "Initialization finished...\n";
  #endif

  ff.verifyConsistentAngles( dx, dy, wavenumber );
}

void GenericScattering::getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const
{
  if ( isReferenceRun )
  {
    delta = 0.0;
    beta = 0.0;
    return;
  }
  material->getXrayMatProp( x, y, z, delta, beta );
}

void GenericScattering::solve()
{
  if ( material == NULL )
  {
    throw( runtime_error("No material set!") );
  }
  init();
  printInfo();

  // Reference run
  ParaxialSimulation::solve();

  delete reference;
  reference = new arma::cx_mat;

  switch( propagator )
  {
    case SolverType_t::ADI:
      *reference = adisolver.getLastSolution3D();
      break;
    case SolverType_t::FFT:
      *reference = fft3Dsolver.getLastSolution3D();
      break;
    case SolverType_t::PROJ:
      *reference = projSolver.getLastSolution3D();
      break;
  }
  // if ( useFFTSolver ) *reference = fft3Dsolver.getLastSolution3D(); Why is this commented out??
  // else *reference = adisolver.getLastSolution3D();

  if (subtract_reference) ff.setReference( *reference );
  clog << "Reference solution computed\n";

  reset();
  isReferenceRun = false;
  if ( realTimeVisualization )
  {
    switch( propagator )
    {
      case SolverType_t::ADI:
        adisolver.realTimeVisualization();
        break;
      case SolverType_t::FFT:
        fft3Dsolver.visualizeRealSpace();
        break;
    }
    //if ( useFFTSolver ) fft3Dsolver.visualizeRealSpace();
    //else adisolver.realTimeVisualization();
  }
  // Set resolution for higher
  setLongitudinalDiscretization( zmin, zmax, dz, downSampleZ );
  switch( propagator )
  {
    case SolverType_t::ADI:
      adisolver.updateDimensionsOfArrays();
      break;
    case SolverType_t::FFT:
      fft3Dsolver.updateDimensionsOfArrays();
      break;
    case SolverType_t::PROJ:
      projSolver.updateDimensionsOfArrays();
      break;
  }
  setBoundaryConditions( gbeam );
  ParaxialSimulation::solve();
}

void GenericScattering::printInfo() const
{
  if ( !supressMessages )
  {
    cout << "===================== SIMULATION INFO ===========================\n";
    cout << "Maximum size of matrices for export: Nx= " << exportNx << " Ny=" << exportNy << endl;
    cout << "FFT pad length: " << FFTPadLength << endl;
    cout << "Domain size: x=[" << xmin << "," << xmax << "], y=[" << ymin << "," << ymax << "], z=["<<zmin<<","<<zmax<<"]\n";
    cout << "Stepsize: dx=" << dx << ", dy="<<dy<<", dz="<<dz<<endl;
    cout << "Downsample 3D solution by factor: downX=" << downSampleX << " downY=" << downSampleY << " downZ="<<downSampleZ << endl;
    cout << "Wavelength: " << wavelength << endl;
    cout << "Intensity limit for realtime visualization: Imin=" << intensityMin << " Imax=" << intensityMax << endl;
    cout << "Phase limit for realtime visualization: Pmin=" << phaseMin << " Pmax=" << phaseMax << endl;
    cout << "Image are saved to: " << imgname << endl;
    cout << "Gaussian beam waist: " << gbeam.getWaist() << endl;
    cout << "Scattering angle: [" << ff.getMinScatteringAngle() << "," << ff.getMaxScatteringAngle() << "]\n";
    cout << "==================================================================\n";
  }
}

void GenericScattering::getFarField( arma::mat &farF )
{
  ff.result( *solver, farF );
}

void GenericScattering::addPostProcessing( post::PostProcessingModule &ppm )
{
  userDefinedPPM.push_back( &ppm );
}
