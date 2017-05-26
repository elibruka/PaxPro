#include "geometry.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;

int main()
{
  try
  {
    geom::Sphere sphere(10.0);
    geom::Part part( "testpart" );
    part.add( sphere );

    geom::Box box(20.0,20.0,20.0);
    box.rotate( 45.0, geom::Axis_t::Z );
    box.translate( 5.0, 0.0, 0.0 );
    part.add( box );

    geom::Cylinder cylinder( 10.0, 10.0 );
    cylinder.translate( 15.0, 0.0, 0.0 );
    part.difference( cylinder );

    geom::Sphere ellipsoid( 10.0 );

    // Note that the translations are also affected by the scale
    // Thus a translation of x=-2.5 corresponds in this case to x = -4.0*2.5 = -10.0
    ellipsoid.scale( 4.0, geom::Axis_t::X );
    ellipsoid.translate( -2.5, 0.0, 0.0 );
    part.add( ellipsoid );
    part.save( "data/testpart.scad" );
  }
  catch( exception &exc )
  {
    cout << exc.what() << endl;
    return 1;
  }
  return 0;
}
