#include <PaxPro/genericScattering.hpp>
#include <PaxPro/controlFile.hpp>
#include <iostream>

using namespace std;

/** Sphere centered at (0,0,0) */
class Sphere: public MaterialFunction
{
public:
  Sphere( double rad ): radius(rad){};
  void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const override
  {
    double rSq = x*x + y*y + z*z;
    if ( rSq < radius*radius )
    {
      // Is inside sphere
      delta = 8.9E-6;
      beta = 7E-7;
      return;
    }
    delta = 0.0;
    beta = 0.0;
  }
private:
  double radius{0.0};
};

/** Main function for running the program */
int main( int argc, char **argv )
{

  // Define some lengths in nano meter
  double r = 500.0;
  double xmin = -1.5*r;
  double xmax = 1.5*r;
  double zmin = -1.05*r;
  double zmax = 1.05*r;

  // Calculate the stepsizes in x and z direction (the step size in y-direction is here assumed to be equal to the one in x-direction)
  double dx = (xmax-xmin)/512;
  double dz = (zmax-zmin)/256;

  // Initialize the GenericScattering class
  GenericScattering simulation("sphere");
  simulation.description = "Simualtion of a sphere with radius 500 nm. \
  Delta=8.9E-6 and beta=7E-7.";

  // Set the waist of the Gaussian beam
  simulation.setBeamWaist( 400.0*r );

  // Set the maximum scattering angle to store
  // The maximum scattering angle that is possible to calculate depend on the discretization
  // and is given by q_max = pi/dx --> alpha_max = q_max/k
  simulation.setMaxScatteringAngle( 0.05 );

  // Set the domain size and stepsizes
  simulation.xmin = xmin;
  simulation.xmax = xmax;
  simulation.ymin = xmin;
  simulation.ymax = xmax;
  simulation.zmin = zmin;
  simulation.zmax = zmax;
  simulation.dx = dx;
  simulation.dy = dx;
  simulation.dz = dz;

  // To save memor the results can be downsampled by the factors given below
  // These values yields a very coarse sampling
  // This does not affect the resolution only the 3D matrix with field values that is stored
  // Thus, if the only interesting quantities is quantities that can be calculated from the
  // exit fields (like total accumulated phase, far field, exit intensity etc. )
  // these values can be set rather high (like below)
  simulation.downSampleX = 512;
  simulation.downSampleY = 512;
  simulation.downSampleZ = 512;

  // Set the wavelength in nm
  simulation.wavelength = 0.1569;

  simulation.realTimeVisualization = true;
  simulation.useFFTSolver = false;

  // Set how long the padded signal should be when computing the far fields
  simulation.FFTPadLength = 32768;

  // Initialize the geometry
  Sphere sphere( r );
  simulation.setMaterial( sphere );

  try
  {
    // Solve the paraxial waveequation
    simulation.solve();

    // Save results
    ControlFile ctl("data/sphere");
    simulation.save( ctl );
    ctl.save();
  }
  catch ( exception &exc )
  {
    cout << exc.what() << endl;
    return 1;
  }
  catch (...)
  {
    cout << "Unrecognized exception!\n";
    return 1;
  }
  return 0;
}
