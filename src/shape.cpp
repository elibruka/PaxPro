#include "shapes.hpp"
#include <cmath>
#include <sstream>
#include <cassert>

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
  mat(0,3) = -x;
  mat(1,3) = -y;
  mat(2,3) = -z;
  transformation = mat*transformation;
}

void geom::Shape::rotate( double angleDeg, geom::Axis_t axis )
{
  double PI = acos(-1.0);
  double angle = angleDeg*PI/180.0;
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
      mat(0,2) = -sin(angle);
      mat(2,0) = sin(angle);
      mat(2,2) = cos(angle);
      break;
    case geom::Axis_t::Z:
      mat(0,0) = cos(angle);
      mat(0,1) = sin(angle);
      mat(1,0) = -sin(angle);
      mat(1,1) = cos(angle);
      break;
  }
  transformation = mat*transformation;
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
  stringstream ss;
  arma::mat reverseTrans;
  getInverseTransformation(reverseTrans);
  ss << "multmatrix( m=[";
  for ( unsigned int i=0;i<4;i++ )
  {
    ss << "[" << reverseTrans(i,0) << "," << reverseTrans(i,1) << "," << reverseTrans(i,2) << ",";
    ss << reverseTrans(i,3) << "]";
    if ( i != 3 ) ss << ",\n";
    else ss << "])";
  }
  string openscadCode;
  openSCADDescription(openscadCode);
  ss << "{" <<  openscadCode << "}\n";
  code = ss.str();
}

void geom::Shape::getInverseTransformation( arma::mat &inverse ) const
{
  inverse = arma::inv(transformation);
  /*
  inverse.set_size(4,4);
  inverse.eye();
  for ( unsigned int i=0;i<3;i++ )
  {
    for ( unsigned int j=i+1;j<3;j++ )
    {
      inverse(j,i) = transformation(i,j);
      inverse(i,j) = transformation(j,i);
    }
    inverse(i,3) = -transformation(i,3);
  }*/
}

void geom::Shape::inverseTransform( double &x, double &y, double &z ) const
{
  arma::mat inverse;
  getInverseTransformation( inverse );
  arma::vec vec(4);
  vec(0) = x;
  vec(1) = y;
  vec(2) = z;
  vec(3) = 1.0;
  vec = inverse*vec;
  x = vec(0);
  y = vec(1);
  z = vec(2);
}

////////////////////////////////////////////////////////////////////////////////
bool geom::Sphere::isInside( double x, double y, double z ) const
{
  transform(x,y,z);
  double r = sqrt( x*x + y*y + z*z );
  return r < radius;
}

void geom::Sphere::openSCADDescription( std::string &description ) const
{
  stringstream ss;
  ss << "sphere(" << radius << ",true);";
  description = ss.str();
}

////////////////////////////////////////////////////////////////////////////////
bool geom::Box::isInside( double x, double y, double z ) const
{
  transform(x,y,z);
  return ( x > -Lx/2.0 ) && ( x < Lx/2.0 ) && ( y > -Ly/2.0 ) && ( y < Ly/2.0 ) && ( z > -Lz/2.0 ) && ( z > Lz/2.0 );
}

void geom::Box::openSCADDescription( std::string &description ) const
{
  stringstream ss ;
  ss << "cube([" << Lx << "," << Ly << "," << Lz << "],true);";
  description = ss.str();
}


////////////////////////////////////////////////////////////////////////////////
bool geom::Cylinder::isInside( double x, double y, double z ) const
{
  transform(x,y,z);
  bool zIsInside = ( z < height/2.0 ) && ( z > -height/2.0 );
  if ( !zIsInside ) return false;

  double slope = ( r2-r1 )/height;
  double radius = slope*(z+height/2.0) + r1;
  assert( radius >= 0.0 );
  double r = sqrt(x*x+y*y);
  return r < radius;
}

void geom::Cylinder::openSCADDescription( string &description ) const
{
  stringstream ss;
  ss << "cylinder(" << height << "," << r1 << "," << r2 << ",true);";
  description = ss.str();
}
