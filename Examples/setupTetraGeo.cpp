#include "tetraGeometry.hpp"
#include <stdexcept>
#include <iostream>
#include <map>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

int main( int argc, char** argv )
{
  srand(time(0));
  try
  {
    TetraGeometry geo;
    geo.load( "Geometries/sphere.msh" );
    map<string,XrayMatProperty> matprop;

    XrayMatProperty sphere;
    sphere.delta = 1.0;
    sphere.beta = 1.0;
    matprop["Sphere"] = sphere;

    XrayMatProperty surroundings;
    //surroundings.delta = 0.0;
    //surroundings.beta = 0.0;
    //matprop["void"] = surroundings;
    geo.setMatProp( matprop );

    // Run some tests
    double delta, beta;

    array<double,3> crn1, crn2;
    geo.boundingBox(crn1,crn2);

    array<double,3> center;
    for ( unsigned int i=0;i<3;i++ )
    {
      center[i] = 0.5*(crn1[i]+crn2[i]);
    }
    double R = (crn2[0]-crn1[0])/2.0;
    unsigned int numberOfCorrect = 0;
    unsigned int maxIter = 1000;
    for ( unsigned int i=0;i<maxIter;i++ )
    {
      double x = (static_cast<double>(rand())/RAND_MAX)*(crn2[0]-crn1[0]) + crn1[0];
      double y = (static_cast<double>(rand())/RAND_MAX)*(crn2[1]-crn1[1]) + crn1[1];
      double z = (static_cast<double>(rand())/RAND_MAX)*(crn2[2]-crn1[2]) + crn1[2];
      geo.getXrayMatProp(x,y,z,delta,beta);

      // Test if the points are inside the sphere
      if ( sqrt( pow(x-center[0],2) + pow(y-center[1],2) + pow(z-center[2],2) ) < R )
      {
        // Is inside
        if ( delta > 0.9 ) numberOfCorrect++;
      }
      else
      {
        if ( delta < 0.9 ) numberOfCorrect++;
      }
    }
    cout << "NOTE: there can be some discrepency as the sphere is represented by flat faces\n";
    cout << "Tree search returns the correct ID " << numberOfCorrect << " of " << maxIter << " times.\n";

  }
  catch( exception &exc )
  {
    cout << exc.what() << endl;
  }
  catch(...)
  {
    cout << "Unrecognized exception!\n";
  }
  return 0;
}
