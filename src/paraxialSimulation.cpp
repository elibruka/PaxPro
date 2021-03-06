#include "paraxialSimulation.hpp"
#include "H5Cpp.h"
#include "solver2D.hpp"
#include "crankNicholson.hpp"
#include "waveGuide.hpp"
#include "solver1D.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>
#include "paraxialSource.hpp"
#include "arraySource.hpp"
#include "h5Attribute.hpp"
#include "hdf5DataspaceCreator.hpp"
#include <limits>
#include <stdexcept>
#include <utility>
#include <sstream>

using namespace std;
const double PI = acos(-1.0);
ParaxialSimulation::ParaxialSimulation(const char* name):xDisc(new Disctretization), zDisc(new Disctretization),
yDisc(new Disctretization), name(name){};

ParaxialSimulation::~ParaxialSimulation()
{
  delete xDisc; xDisc = NULL;
  delete zDisc; zDisc = NULL;
  delete yDisc; yDisc = NULL;

  if ( solverInitializedViaInit )
  {
    delete solver; solver = NULL;
  }
  delete farFieldModulus; farFieldModulus = NULL;
  delete file; file = NULL;
  delete maingroup; maingroup = NULL;
}

void ParaxialSimulation::solve()
{
  solver->solve();
}

void ParaxialSimulation::step()
{
  solver->step();
}

void ParaxialSimulation::reset()
{
  if ( solver != nullptr ) solver->reset();
}

void ParaxialSimulation::verifySolverReady() const
{
  if ( solver == NULL )
  {
    throw ( runtime_error("No solver specified!"));
  }

  if ( src == NULL )
  {
    throw ( runtime_error("No source specified!"));
  }
}

void ParaxialSimulation::setWaveLength( double lambda )
{
  wavenumber = 2.0*PI/lambda;
}

void ParaxialSimulation::setTransverseDiscretization( double xmin, double xmax, double step )
{
  setTransverseDiscretization( xmin, xmax, step, 1 );
}

void ParaxialSimulation::setTransverseDiscretization( double xmin, double xmax, double step, unsigned int downsamplingRatio )
{
  xDisc->min = xmin;
  xDisc->max = xmax;
  xDisc->step = step;
  xDisc->downsamplingRatio = downsamplingRatio;
  yDisc->downsamplingRatio = xDisc->downsamplingRatio;
}

void ParaxialSimulation::setVerticalDiscretization( double ymin, double ymax, double step )
{
  yDisc->min = ymin;
  yDisc->max = ymax;
  yDisc->step = step;
}

void ParaxialSimulation::setLongitudinalDiscretization( double zmin, double zmax, double step )
{
  setLongitudinalDiscretization( zmin, zmax, step, 1 );
}

void ParaxialSimulation::setLongitudinalDiscretization( double zmin, double zmax, double step, unsigned int downsamplingRatio )
{
  zDisc->min = zmin;
  zDisc->max = zmax;
  zDisc->step = step;
  zDisc->downsamplingRatio = downsamplingRatio;
}

unsigned int ParaxialSimulation::nodeNumberTransverse() const
{
  return (xDisc->max - xDisc->min)/xDisc->step + 1.0;
}

unsigned int ParaxialSimulation::nodeNumberVertical() const
{
  return (yDisc->max - yDisc->min)/yDisc->step + 1.0;
}

unsigned int ParaxialSimulation::nodeNumberLongitudinal() const
{
  return ( zDisc->max - zDisc->min)/zDisc->step + 1.0;
}

void ParaxialSimulation::setSolver( Solver &solv )
{
  if ( solverInitializedViaInit )
  {
    throw ("You cannot set a new solver when it has been initialized via the init function");
  }

  solver = &solv;
  solver->setSimulator( *this );
}

void ParaxialSimulation::save( const string &fname )
{
  save( fname.c_str() );
}
void ParaxialSimulation::save( const char* h5fname )
{
  if ( solver == NULL )
  {
    throw ( runtime_error("No solver specified!\n"));
  }

  if ( src == NULL )
  {
    throw ( runtime_error("No source specified!\n") );
  }
  //string fname = ctl.getFnameTemplate();
  //string h5fname = fname+".h5";
  //string jsonfname = fname+".json";
  vector<string> dsets;
  if ( file != NULL ) delete file;
  file = new H5::H5File( h5fname, H5F_ACC_TRUNC );
  maingroup = new H5::Group( file->createGroup(groupname+"/") );
  setGroupAttributes();

  // Save all results from all the post processing modules
  for ( unsigned int i=0;i<postProcess.size();i++ )
  {
    vector<H5Attr> attrib;
    arma::vec res1D;
    arma::mat res2D;
    arma::cube res3D;
    arma::Cube<unsigned char> resUint8_3D;

    // Get the result. No two of these functions will always be empty
    postProcess[i]->addAttrib( attrib );
    switch ( postProcess[i]->getReturnType( *solver ) )
    {
      case ( post::PostProcessingModule::ReturnType_t::vector1D ):
        postProcess[i]->result( *solver, res1D );
        saveArray( res1D, postProcess[i]->getName().c_str(), attrib );
        break;
      case ( post::PostProcessingModule::ReturnType_t::matrix2D ):
        postProcess[i]->result( *solver, res2D );
        saveArray( res2D, postProcess[i]->getName().c_str(), attrib );
        break;
      case ( post::PostProcessingModule::ReturnType_t::cube3D ):
        if ( postProcess[i]->isUint8 )
        {
          postProcess[i]->result( *solver, resUint8_3D );
          saveArray( resUint8_3D, postProcess[i]->getName().c_str(), H5::PredType::NATIVE_UINT8 );
        }
        else
        {
          postProcess[i]->result( *solver, res3D );
          saveArray( res3D, postProcess[i]->getName().c_str(), attrib );
        }
        break;
    }
    clog << "Dataset " << postProcess[i]->getName() << " added to HDF5 file\n";
  }
}

void ParaxialSimulation::getExitField( arma::vec &vec ) const
{
  vec.set_size( solver->getSolution().n_rows );
  unsigned int extractCol = solver->getSolution().n_cols-1;
  for ( unsigned int i=0;i<vec.n_elem; i++ )
  {
    vec(i) = solver->getSolution()(i,extractCol).real();
  }
}

void ParaxialSimulation::getExitField( arma::cx_vec &vec ) const
{
  vec.set_size( solver->getSolution().n_rows );
  unsigned int extractCol = solver->getSolution().n_cols-1;
  for ( unsigned int i=0;i<vec.n_elem; i++ )
  {
    vec(i) = solver->getSolution()(i,extractCol);
  }
}

void ParaxialSimulation::closestIndex( double x, double z, unsigned int &ix, unsigned int &iz ) const
{
  ix = (x - xDisc->min)/xDisc->step;
  iz = (z - zDisc->min)/zDisc->step;
}

double ParaxialSimulation::getIntensity( double x, double z ) const
{
  // TODO: The interpolation in this routine does not work
  // Some assertions for debugging
  assert( x < xDisc->max );
  assert( x >= xDisc->min );
  assert( z < zDisc->max );
  assert( z >= zDisc->min );

  unsigned int ix, iz;
  closestIndex( x, z, ix, iz );

  double x1 = xDisc->min + ix*xDisc->step;
  double x2 = x1 + xDisc->step;
  double z1 = zDisc->min + iz*zDisc->step;
  double z2 = z1 + iz*zDisc->step;

  double intensity = pow( abs(solver->getSolution()(ix,iz)),2 )*(x2 - x)*(z2-z);
  intensity += pow( abs(solver->getSolution()(ix+1,iz)), 2 )*(x-x1)*(z2-z);
  intensity += pow( abs(solver->getSolution()(ix,iz+1)), 2 )*(z-z1)*(x2-x);
  intensity += pow( abs(solver->getSolution()(ix+1,iz+1)), 2 )*(z2-z)*(x-x1);
  return intensity/( (x2-x1)*(z2-z1) );
}

double ParaxialSimulation::getIntensity( unsigned int ix, unsigned int iz ) const
{
  return pow( abs( solver->getSolution()(ix,iz) ), 2 );
}

double ParaxialSimulation::getZ( unsigned int iz ) const
{
  return zDisc->min + iz*zDisc->step;
}

double ParaxialSimulation::getX( int ix ) const
{
  return xDisc->min + ix*xDisc->step;
}

double ParaxialSimulation::getY( int iy ) const
{
  return yDisc->min + iy*yDisc->step;
}

template <class arrayType>
void ParaxialSimulation::saveArray( arrayType &matrix, const char* dsetname )
{
  vector<H5Attr> dummy;
  saveArray( matrix, dsetname, dummy, H5::PredType::NATIVE_DOUBLE );
}

template <class arrayType>
void ParaxialSimulation::saveArray( arrayType &matrix, const char* dsetname, H5::PredType dtype )
{
  vector<H5Attr> dummy;
  saveArray( matrix, dsetname, dummy, dtype );
}

template <class arrayType>
void ParaxialSimulation::saveArray( arrayType &matrix, const char* dsetname, const vector<H5Attr> &attrs )
{
  saveArray( matrix, dsetname, attrs, H5::PredType::NATIVE_DOUBLE );
}

template <class arrayType>
void ParaxialSimulation::saveArray( arrayType &matrix, const char* dsetname, const vector<H5Attr> &attrs, H5::PredType dtype )
{
  // Create dataspace
  hsize_t fdim[3];
  DataspaceCreator<arrayType> dsinfo;
  dsinfo.setDims( matrix, fdim );

  H5::DataSpace dataspace( dsinfo.rank, fdim );

  string name(groupname);
  name += dsetname;
  // Create dataset
  H5::DataSet ds( file->createDataSet(name, dtype, dataspace) );
  H5::DataSpace attribSpace(H5S_SCALAR);
  for ( unsigned int i=0;i<attrs.size();i++ )
  {
    H5::Attribute att = ds.createAttribute( attrs[i].name.c_str(), attrs[i].dtype, attribSpace );
    if ( attrs[i].dtype == H5::PredType::NATIVE_INT )
    {
      int value = attrs[i].value;
      att.write( H5::PredType::NATIVE_INT, &value );
    }
    else
    {
      double value = attrs[i].value;
      att.write( H5::PredType::NATIVE_DOUBLE, &value );
    }
  }

  // Write to file
  ds.write( matrix.memptr(), dtype );
}

// Pre-fine allowed template types
template void ParaxialSimulation::saveArray<arma::vec>( arma::vec &matrix, const char* dsetname, const vector<H5Attr> &attrs );
template void ParaxialSimulation::saveArray<arma::mat>( arma::mat &matrix, const char* dsetname, const vector<H5Attr> &attrs );
template void ParaxialSimulation::saveArray<arma::cube>( arma::cube &matrix, const char* dsetname, const vector<H5Attr> &attrs );
template void ParaxialSimulation::saveArray<arma::Cube<unsigned char> >( arma::Cube<unsigned char> &matrix, const char* dsetname, const vector<H5Attr> &attrs );

template void ParaxialSimulation::saveArray<arma::vec>( arma::vec &matrix, const char* dsetname, H5::PredType dtype );
template void ParaxialSimulation::saveArray<arma::mat>( arma::mat &matrix, const char* dsetname, H5::PredType dtype );
template void ParaxialSimulation::saveArray<arma::cube>( arma::cube &matrix, const char* dsetname, H5::PredType dtype );
template void ParaxialSimulation::saveArray<arma::Cube<unsigned char> >( arma::Cube<unsigned char> &matrix, const char* dsetname, H5::PredType dtype );

template void ParaxialSimulation::saveArray<arma::vec>( arma::vec &matrix, const char* dsetname );
template void ParaxialSimulation::saveArray<arma::mat>( arma::mat &matrix, const char* dsetname );
template void ParaxialSimulation::saveArray<arma::cube>( arma::cube &m, const char* dsetname );
template void ParaxialSimulation::saveArray<arma::Cube<unsigned char> >( arma::Cube<unsigned char> &m, const char* dsetname );

template void ParaxialSimulation::saveArray<arma::vec>( arma::vec &matrix, const char* dsetname, const vector<H5Attr> &attrs, H5::PredType dtype );
template void ParaxialSimulation::saveArray<arma::mat>( arma::mat &matrix, const char* dsetname, const vector<H5Attr> &attrs, H5::PredType dtype );
template void ParaxialSimulation::saveArray<arma::cube>( arma::cube &matrix, const char* dsetname, const vector<H5Attr> &attrs, H5::PredType dtype );
template void ParaxialSimulation::saveArray<arma::Cube<unsigned char> >( arma::Cube<unsigned char> &matrix, const char* dsetname, const vector<H5Attr> &attrs, H5::PredType dtype );



void ParaxialSimulation::addAttribute( H5::DataSet &ds, const char* name, double value )
{
  H5::DataSpace attribSpace(H5S_SCALAR);
  H5::Attribute att = ds.createAttribute( name, H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &value );
}

void ParaxialSimulation::addAttribute( H5::DataSet &ds, const char* name, int value )
{
  H5::DataSpace attribSpace(H5S_SCALAR);
  H5::Attribute att = ds.createAttribute( name, H5::PredType::NATIVE_INT, attribSpace );
  att.write( H5::PredType::NATIVE_INT, &value );
}

void ParaxialSimulation::setBoundaryConditions( const ParaxialSource &source )
{

  if ( solver == nullptr )
  {
    throw ( runtime_error("Solver needs to be set before boundary conditions!") );
  }

  src = &source;
  unsigned int Nx = nodeNumberTransverse();

  typedef ParaxialSource::Dim_t Dim_t;
  switch ( source.getDim() )
  {
    case Dim_t::TWO_D:
    {
      arma::cx_vec values(Nx, 1.0);
      for ( unsigned int i=0;i<Nx;i++ )
      {
        double x = getX(i);
        values[i] = src->get( x, 0.0 );
      }
      solver->setInitialConditions( values );
      break;
    }
    case Dim_t::THREE_D:
    {
      unsigned int Ny = nodeNumberVertical();
      arma::cx_mat values(Nx, Ny);
      #pragma omp parallel for
      for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
      {
        unsigned int i = indx/Ny;
        unsigned int j = indx%Ny;
        double x = getX(i);
        double y = getY(j);
        values(j,i) = src->get(x,y,0.0);
      }
      solver->setInitialConditions( values );
    }
  }

  this->setWaveLength( source.getWavelength() );
}

void ParaxialSimulation::setBoundaryConditions( const ArraySource &source )
{
  src = &source;
  if ( source.getVec().n_elem != nodeNumberTransverse() )
  {
    stringstream msg;
    msg << "The size of the given array does not match the number of transverse nodes! ";
    msg << "Required size: " << nodeNumberTransverse();
    msg << " Given size: " << source.getVec().n_elem;
    throw ( runtime_error( msg.str() ) );
  }
  solver->setInitialConditions( source.getVec() );
}

double ParaxialSimulation::getEnergy() const
{
  return 12.398*100.0/getWavelength();
}

double ParaxialSimulation::getWavelength() const
{
  return 2.0*PI/wavenumber;
}

void ParaxialSimulation::setFarFieldAngleRange( double phiMin, double phiMax )
{
  farParam.phiMin = phiMin;
  farParam.phiMax = phiMax;
}

ParaxialSimulation& ParaxialSimulation::operator << ( post::PostProcessingModule &module )
{
  postProcess.push_back( &module );
  return *this;
}

ParaxialSimulation& ParaxialSimulation::operator << (post::FarField &ff )
{
  ff.linkParaxialSim( *this );
  postProcess.push_back( &ff );
  return *this;
}

ParaxialSimulation& ParaxialSimulation::addPostProcessingModule( post::PostProcessingModule &module )
{
  return *this << module;
}

ParaxialSimulation& ParaxialSimulation::addPostProcessingModule( post::FarField &farfield )
{
  return *this << farfield;
}

void ParaxialSimulation::setGroupAttributes()
{
  if ( maingroup == NULL ) return;

  // Create a dataspace
  H5::DataSpace attribSpace(H5S_SCALAR);

  // Write first attribute
  H5::Attribute att = maingroup->createAttribute( "xmin", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &xDisc->min );

  att = maingroup->createAttribute( "xmax", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &xDisc->max );

  att = maingroup->createAttribute( "dx", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &xDisc->step );

  att = maingroup->createAttribute( "zmin", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &zDisc->min );

  att = maingroup->createAttribute( "zmax", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &zDisc->max );

  att = maingroup->createAttribute( "dz", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &zDisc->step );

  att = maingroup->createAttribute( "ymin", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &yDisc->min );

  att = maingroup->createAttribute( "ymax", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &yDisc->max );

  att = maingroup->createAttribute( "dy", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &yDisc->step );

  //att = maingroup->createAttribute( "uid", H5::PredType::NATIVE_INT, attribSpace );
  //att.write( H5::PredType::NATIVE_INT, &uid );

  att = maingroup->createAttribute( "wavenumber", H5::PredType::NATIVE_DOUBLE, attribSpace );
  att.write( H5::PredType::NATIVE_DOUBLE, &wavenumber );

  if ( solver != nullptr )
  {
    string solvername = solver->getName();
    H5::StrType strType( H5::PredType::C_S1, solvername.length() );
    H5std_string strBuffer( solvername.c_str() );
    att = maingroup->createAttribute( "solver", strType, attribSpace );
    att.write( strType, strBuffer );
  }

  if ( description != "" )
  {
    H5::StrType strDataType( H5::PredType::C_S1, description.length() );
    H5std_string strBuffer( description.c_str() );
    att = maingroup->createAttribute( "description", strDataType, attribSpace );
    att.write( strDataType, strBuffer );
  }
}

void ParaxialSimulation::getXrayMatProp( double x, double z, double &delta, double &beta ) const
{
  if ( material != nullptr )
  {
    material->getXrayMatProp( x, 0.0, z, delta, beta );
  }
  else
  {
    delta = 0.0;
    beta = 0.0;
  }
}

void ParaxialSimulation::getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const
{
  if ( material != nullptr )
  {
    material->getXrayMatProp( x, y, z, delta, beta );
  }
  else
  {
    delta = 0.0;
    beta = 0.0;
  }
}
