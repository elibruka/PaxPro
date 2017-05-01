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
    geo.load( "Geometries/all.msh" );
    map<string,XrayMatProperty> matprop;

    XrayMatProperty sphere;
    sphere.delta = 4.9E-5;
    sphere.beta = 8.9E-6;
    matprop["sphere"] = sphere;

    XrayMatProperty substrate;
    substrate.delta = 1E-5;
    substrate.beta = 8E-7;
    matprop["substrate"]  = substrate;

    XrayMatProperty surroundings;
    //surroundings.delta = 0.0;
    //surroundings.beta = 0.0;
    matprop["void"] = surroundings;
    geo.setMatProp( matprop );

    // Run some tests
    double delta, beta;

    array<double,3> crn1, crn2;
    geo.boundingBox(crn1,crn2);
    for ( unsigned int i=0;i<100;i++ )
    {
      double x = (static_cast<double>(rand())/RAND_MAX)*(crn2[0]-crn1[0]) + crn1[0];
      double y = (static_cast<double>(rand())/RAND_MAX)*(crn2[1]-crn1[1]) + crn1[1];
      double z = (static_cast<double>(rand())/RAND_MAX)*(crn2[2]-crn1[2]) + crn1[2];
      geo.getXrayMatProp(x,y,z,delta,beta);
      cout << "Delta=" << delta << " Beta=" << beta << endl;
    }

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
