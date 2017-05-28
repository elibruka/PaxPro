#include "geometry.hpp"
#include <stdexcept>
#include <fstream>
#include <cassert>

using namespace std;

geom::Part::~Part()
{
  if ( ownShapeObjects )
  {
    for ( unsigned int i=0;i<shapes.size();i++ )
    {
      delete shapes[i];
    }
  }
}

geom::Part::Part( const Part &other )
{
  this->swap( other );
}

void geom::Part::swap( const Part &other )
{
  if ( this == &other ) return;

  name = other.name;
  if ( ownShapeObjects )
  {
    for ( unsigned int i=0;i<shapes.size();i++ )
    {
      delete shapes[i];
    }
  }

  shapes.clear();
  operations.clear();

  // Just to be sure that clear sets the size of the vectors to 0
  assert( shapes.size() == 0 );
  assert( operations.size() == 0 );

  for ( unsigned int i=0;i<other.shapes.size();i++ )
  {
    shapes.push_back( other.shapes[i]->clone() );
    operations.push_back( other.operations[i] );
  }
  delta = other.delta;
  beta = other.beta;
  ownShapeObjects = true;
}

void geom::Part::add( Shape &shape )
{
  shapes.push_back( &shape );
  operations.push_back( Operation_t::UNION );
}

void geom::Part::difference( Shape &shape )
{
  if ( shapes.size() == 0 )
  {
    throw ( runtime_error("There is no shape to make difference with. You have to add at least one shape before...") );
  }

  shapes.push_back( &shape );
  operations.push_back( Operation_t::DIFFERENCE );
}

void geom::Part::dump( const char* fname ) const
{
  ofstream out;
  out.open( fname );
  if ( !out.good() )
  {
    throw( runtime_error("Could open open openSCAD output file!") );
  }

  for ( unsigned int i=0;i<shapes.size();i++ )
  {
    string code;
    shapes[i]->openSCADExport( code );
    out << code << "\n";
  }
  out.close();

  cout << "Geometry written to " << fname << endl;
}

bool geom::Part::isInside( double x, double y, double z ) const
{
  for ( unsigned int i=0;i<shapes.size();i++ )
  {
    bool insideShape = shapes[shapes.size()-1-i]->isInside( x, y, z );
    if ( insideShape )
    {
      switch( operations[operations.size()-1-i] )
      {
        case Operation_t::UNION:
          return true;
        case Operation_t::DIFFERENCE:
          return false;
      }
    }
  }
  return false;
}

void geom::Part::save( const char* fname ) const
{
  if ( shapes.size() <= 1 )
  {
    dump(fname);
    return;
  }

  std::string description;
  openSCADDescription( description );

  ofstream out;
  out.open(fname);
  if ( !out.good() )
  {
    throw( runtime_error("Could not open openSCAD output file!") );
  }
  out << description;
  out.close();

  cout << "Part saved to " << fname << endl;
}

void geom::Part::openSCADDescription( string &description ) const
{
  shapes[0]->openSCADExport( description );
  for ( unsigned int i=1;i<shapes.size();i++ )
  {
    string specifier;
    switch ( operations[i] )
    {
      case Operation_t::UNION:
        specifier = "union(){\n";
        break;
      case Operation_t::DIFFERENCE:
        specifier = "difference(){\n";
        break;
    }
    string osccode;
    shapes[i]->openSCADExport( osccode );
    description =  specifier+description+osccode+"}\n";
  }
}

void geom::Part::translate( double x, double y, double z )
{
  for ( unsigned int i=0;i<shapes.size();i++ )
  {
    shapes[i]->translate(x,y,z);
  }
}

void geom::Part::rotate( double angleDeg, geom::Axis_t axis )
{
  for ( unsigned int i=0;i<shapes.size();i++ )
  {
    shapes[i]->rotate( angleDeg, axis );
  }
}

void geom::Part::scale( double factor, geom::Axis_t axis )
{
  for ( unsigned int i=0;i<shapes.size();i++ )
  {
    shapes[i]->scale( factor, axis );
  }
}

////////////////////////////////////////////////////////////////////////////////
geom::Module::~Module()
{
  if ( ownPartObjects )
  {
    for ( unsigned int i=0;i<parts.size();i++ )
    {
      delete parts[i];
    }
    parts.clear();
  }
}

geom::Module::Module( const geom::Module &other )
{
  this->swap( other );
}

void geom::Module::add( Part &part )
{
  parts.push_back( &part );
  operations.push_back( Operation_t::UNION );
}

void geom::Module::difference( Part &part )
{
  if ( parts.size() <= 1 )
  {
    throw( runtime_error("There have to be at least one part present before making difference!") );
  }
  parts.push_back( &part );
  operations.push_back( Operation_t::DIFFERENCE );
}

bool geom::Module::getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const
{
  delta = 0.0;
  beta = 0.0;
  for ( unsigned int i=0;i<parts.size();i++ )
  {
    if ( parts[parts.size()-i-1]->isInside(x,y,z) )
    {
      switch( operations[parts.size()-i-1] )
      {
        case Operation_t::UNION:
          delta = parts[parts.size()-i-1]->delta;
          beta = parts[parts.size()-i-1]->beta;
          return true;
        case Operation_t::DIFFERENCE:
          delta = 0.0;
          beta = 0.0;
          return true;
      }
    }
  }
  return false;
}

void geom::Module::translate( double x, double y, double z )
{
  for ( unsigned int i=0;i<parts.size();i++ )
  {
    parts[i]->translate(x,y,z);
  }
}

void geom::Module::rotate( double angleDeg, geom::Axis_t axis )
{
  for ( unsigned int i=0;i<parts.size();i++ )
  {
    parts[i]->rotate( angleDeg, axis );
  }
}

void geom::Module::scale( double factor, geom::Axis_t axis )
{
  for ( unsigned int i=0;i<parts.size();i++ )
  {
    parts[i]->scale( factor, axis );
  }
}

void geom::Module::saveIndividualParts( const char* prefix )
{
  for ( unsigned int i=0;i<parts.size();i++ )
  {
    string fname(prefix);
    fname += (name+"_"+parts[i]->getName()+".scad");
    parts[i]->save(fname.c_str());
  }
}

void geom::Module::swap( const geom::Module &other )
{
  if ( ownPartObjects )
  {
    for ( unsigned int i=0;i<parts.size();i++ )
    {
      delete parts[i];
    }
  }

  parts.clear();
  operations.clear();

  assert( parts.size() == 0 );
  assert( operations.size() == 0 );

  for ( unsigned int i=0;i<other.parts.size();i++ )
  {
    parts.push_back( new geom::Part( *other.parts[i]) );
    operations.push_back( other.operations[i] );
  }

  ownPartObjects = true;
}

void geom::Module::save( const char* fname ) const
{
  string description;
  parts[0]->openSCADDescription( description );
  for ( unsigned int i=1;i<parts.size();i++ )
  {
    string specifier;
    switch ( operations[i] )
    {
      case Operation_t::UNION:
        specifier = "union(){\n";
        break;
      case Operation_t::DIFFERENCE:
        specifier = "difference(){\n";
        break;
    }
    string osccode;
    parts[i]->openSCADDescription( osccode );
    description =  specifier+description+osccode+"}\n";
  }

  ofstream out;
  out.open( fname );

  if ( !out.good() )
  {
    cout << "Could not open file " << fname;
    return;
  }

  out << description;
  out.close();
  cout << "Module saved to " << fname << endl;
}
