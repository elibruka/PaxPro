#include "geometry.hpp"
#include <stdexcept>
#include <fstream>

using namespace std;

void geom::Part::add( const Shape &shape )
{
  shapes.push_back( &shape );
  operations.push_back( Operation_t::UNION );
}

void geom::Part::difference( const Shape &shape )
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

  string everything;

  shapes[0]->openSCADExport( everything );
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
    //osccode += "\n";
    everything =  specifier+everything+osccode+"}\n";
  }

  ofstream out;
  out.open(fname);
  if ( !out.good() )
  {
    throw( runtime_error("Could not open openSCAD output file!") );
  }
  out << everything;
  out.close();
}
