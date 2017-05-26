#include "shapes.hpp"
#include <cmath>
#include <sstream>

using namespace std;

using namespace std;

geom::Shape::Shape( const char* name ): name(name)
{
  transformation.set_size(4,4);
  transformation.eye();
}

void geom::Shape::translate( double x, double y, double z )
{
  arma::mat mat(4,4);
  mat.eye();
  mat(3,0) = x;
  mat(3,1) = y;
  mat(3,2) = z;
  transformation = transformation*mat;
}

void geom::Shape::rotate( double angleDeg, geom::Axis_t axis )
{
  double PI = acos(-1.0);
  double angle = angle*PI/180.0;
  arma::mat mat(4,4);
  mat.eye();

  switch( axis )
  {
    case geom::Axis_t::X:
      mat(1,1) = cos(angle);
      mat(1,2) = sin(angle);
      mat(2,1) = -sin(angle);
      mat(2,2) = cos(angle);
      break;
    case geom::Axis_t::Y:
      mat(0,0) = cos(angle);
      mat(0,2) = sin(angle);
      mat(2,0) = -sin(angle);
      mat(2,2) = cos(angle);
      break;
    case geom::Axis_t::Z:
      mat(0,0) = cos(angle);
      mat(0,1) = sin(angle);
      mat(1,0) = -sin(angle);
      mat(1,1) = cos(angle);
      break;
  }
  transformation = transformation*mat;
}

void geom::Shape::transform( double &x, double &y, double &z ) const
{
  arma::vec vec(4);
  vec(0) = x;
  vec(1) = y;
  vec(2) = z;
  vec(3) = 1.0;
  vec = transformation*vec;
  x = vec(0);
  y = vec(1);
  z = vec(2);
}

void geom::Shape::openSCADExport( string &code ) const
{
  // Not implemented
}

////////////////////////////////////////////////////////////////////////////////
bool geom::Sphere::isInside( double x, double y, double z ) const
{
  double r = sqrt( x*x + y*y + z*z );
  return r < radius;
}

void geom::Sphere::openSCADDescription( std::string &description ) const
{
  stringstream ss;
  ss << "sphere(" << radius << ");";
  description = ss.str();
}
