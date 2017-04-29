#include "tetraGeometry.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>

using namespace std;

void TetraGeometry::load( const char *fname )
{
  ifstream infile;
  infile.open( fname );
  if ( !infile.good() )
  {
    string strfname(fname);
    string msg("Could not open file ");
    msg += strfname;
    throw( runtime_error(msg) );
  }

  string line;
  while( getline(infile, line) )
  {
    if ( line.find("$Nodes") != string::npos )
    {
      clog << "Reading nodes...\n";
      readNodes( infile );
    }

    if ( line.find("$Elements") )
    {
      clog << "Reading elements...\n";
      readElements( infile );
    }
  }
  infile.close();
}

void TetraGeometry::readNodes( ifstream &infile )
{
  int numberOfNodes;
  infile >> numberOfNodes;
  nodes.resize(numberOfNodes);
  string line;
  while( getline(infile, line) )
  {
    if ( line.find("$EndNodes") != string::npos ) return;
    stringstream ss(line);
    int indx;
    Node newnode;
    ss >> indx >> newnode.x >> newnode.y >> newnode.z;
    if ( indx > nodes.size() )
    {
      stringstream msg;
      msg << "Too many nodes. Current node " << indx << ". Expected maximum " << numberOfNodes << " nodes.";
      throw( runtime_error(msg.str()) );
    }
    nodes[indx-1] = newnode;
  }
}

void TetraGeometry::readElements( ifstream &infile )
{
  int numberOfElem;
  infile >> numberOfElem;
  string line;
  while( getline(infile, line) )
  {
    if ( line.find("$EndElements") != string::npos ) return;
    int elemNumber;
    int elemType;
    int numberOfTags;
    stringstream ss(line);
    ss >> elemNumber >> elemType >> numberOfTags;
    if ( elemType != 4 ) continue;
    int dummy;
    for ( unsigned int i=0;i<numberOfTags;i++ )
    {
      ss >> dummy;
    }
    Tetrahedron tetra;
    tetra.id = elemNumber;
    ss >> tetra.nodes[0] >> tetra.nodes[1] >> tetra.nodes[2] >> tetra.nodes[3];
  }
}
