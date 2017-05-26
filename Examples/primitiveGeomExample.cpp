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
    part.save( "data/testpart.scad" );
  }
  catch( exception &exc )
  {
    cout << exc.what() << endl;
    return 1;
  }
  return 0;
}
