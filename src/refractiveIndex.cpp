#include "refractiveIndex.hpp"
#include <stdexcept>
#include <fstream>
#include <cassert>
#include "config.h"

/*
#ifndef CURRENT_DIR
  #error The macro CURRENT_DIR should be defined by the CMake file
#endif

#define MACRO_TO_STRING(name) #name
#define DIR MACRO_TO_STRING(CURRENT_DIR)
*/
using namespace std;
//const std::string dir(DIR);
const std::string dir(SOURCE_DIR);
const map<elementName, filename> RefractiveIndex::knownElements = {{"Ta", dir+"/MatProp/indexRefrTa.txt"},
                                    {"C2H6O", dir+"/MatProp/indexRefrC2H6O.txt"},
                                    {"SiO2", dir+"/MatProp/indexRefrSiO2.txt"},
                                    {"C2H6O2", dir+"/MatProp/indexRefrC2H6O2.txt"},
                                    {"Pb", dir+"/MatProp/indexRefrPb.txt"},
                                    {"Au", dir+"/MatProp/indexRefrAu.txt"},
                                    {"Vacuum", ""}};

void RefractiveIndex::load( const char* element )
{
  if ( knownElements.find(element) == knownElements.end() )
  {
    string msg("Unknown element ");
    msg += element;
    msg += "!";
    throw ( runtime_error(msg) );
  }
  string lookUp(element);
  if ( lookUp == "Vacuum")
  {
    isVacuum = true;
    return;
  }
  string fname(knownElements.at(lookUp));
  readFromFile( fname );
}

void RefractiveIndex::loadUserDefinedFile( const char* fname )
{
  string filename(fname);
  readFromFile( filename );
}

void RefractiveIndex::readFromFile( const string &fname )
{
    ifstream infile;
    infile.open( fname.c_str() );
    if ( !infile.good() )
    {
      string msg("Could not open refractive index file ");
      msg += fname;
      msg += "!";
      throw (runtime_error(msg));
    }

    // Read the first two lines
    string line;
    getline( infile, line);
    getline( infile, line);

    double newEnergy, newDelta, newBeta;
    while( infile >> newEnergy >> newDelta >> newBeta )
    {
      energy.push_back( newEnergy );
      delta.push_back( newDelta );
      beta.push_back( newBeta );
    }
    infile.close();
}

unsigned int RefractiveIndex::closestAbove( double energyInEv ) const
{
  for ( unsigned int i=0;i<energy.size();i++ )
  {
    if ( energyInEv < energy[i] )
    {
      return i;
    }
  }
  return energy.size();
}

double RefractiveIndex::getDelta( double energyInEv ) const
{
  if ( isVacuum ) return 0.0;

  unsigned int closest = closestAbove( energyInEv );
  if ( closest == energy.size() )
  {
    return delta.back();
  }
  else if ( closest == 0 )
  {
    return delta[0];
  }
  double weight = energyWeight( energyInEv, closest );
  return weight*delta[closest] + (1.0-weight)*delta[closest-1];
}

double RefractiveIndex::getBeta( double energyInEv ) const
{
  if ( isVacuum ) return 0.0;

  unsigned int closest = closestAbove( energyInEv );
  if ( closest == energy.size() )
  {
    return beta.back();
  }
  else if ( closest == 0 )
  {
    return beta[0];
  }
  double weight = energyWeight( energyInEv, closest);
  return weight*beta[closest] + (1.0-weight)*beta[closest-1];
}

double RefractiveIndex::energyWeight( double energyInEv, unsigned int closestIndxAbove  ) const
{
  double weight = (energyInEv-energy[closestIndxAbove-1])/( energy[closestIndxAbove]-energy[closestIndxAbove-1]);
  // DEBUG
  assert ( weight > 0.0 );
  assert ( weight < 1.0 );
  return weight;
}
