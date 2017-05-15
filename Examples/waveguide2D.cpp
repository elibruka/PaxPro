#include <PaxPro/paxpro.hpp>
#include <PaxPro/crankNicholson.hpp>
#include <stdexcept>
#include <iostream>

using namespace std;

// Define the material function
class Waveguide2D: public MaterialFunction
{
public:
  Waveguide2D(){};

  // In 2D y=0 and the wave is solved in the xz-plane
  virtual void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const override
  {
    if (( x < width ) && ( x > 0.0 ))
    {
      delta = 0.0;
      beta = 0.0;
    }
    else
    {
      delta = 4.9E-5;
      beta = 8.9E-6;
    }
  }

  // Width of the waveguide in nano meters
  double width{100.0};
};

// Min function
int main()
{
  // Create an instance of the material function
  Waveguide2D wg;

  // Initialize the simulation
  ParaxialSimulation sim("Wg2D");
  sim.description = "Simulation of a 2D waveguide of width 100 nm";
  sim.setTransverseDiscretization( -wg.width, 2.0*wg.width, 3.0*wg.width/512 );

  double length = 2E5; // L=200 um
  sim.setLongitudinalDiscretization( 0.0, length, length/1024 );

  // Define a source
  GaussianBeam gbeam;
  gbeam.setWaist( wg.width );
  gbeam.setCenter( wg.width/2.0, 0.0 );
  gbeam.setWavelength( 0.157 );

  // Define post processing modules
  post::Intensity intensity;

  // Define the solver
  CrankNicholson solver;
  solver.setBoundaryCondition( CrankNicholson::BC_t::TRANSPARENT );

  try
  {
    // Pass the solver to the simulation object
    sim.setSolver( solver );

    // Pass a pointer to the material function to the simulation object
    sim.material = &wg;

    // Set the source, (the function should probably be renamed to avoid confusion...)
    sim.setBoundaryConditions( gbeam );

    // Pass the post processing modules to the simulation object
    sim << intensity;

    // Run the simulation
    sim.solve();

    // Save the results
    sim.save( "data/waveguide2DExample.h5" );
  }
  catch( std::exception &exc )
  {
    cout << exc.what() << endl;
  }
  return 0;
}
