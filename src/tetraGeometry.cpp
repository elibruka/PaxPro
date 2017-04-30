#include "tetraGeometry.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <armadillo>
#define TETRA_DEBUG

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

      #ifdef TETRA_DEBUG
        cout << "Read " << nodes.size() << " nodes...\n";
      #endif
    }

    if ( line.find("$Elements") )
    {
      clog << "Reading elements...\n";
      readElements( infile );
      #ifdef TETRA_DEBUG
        cout << "Read " << elements.size() << " elements\n";
      #endif
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
  elements.resize(0);
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
    elements.push_back(tetra);
  }
}

void TetraGeometry::barycentric( double x, double y, double z, const Tetrahedron &tetra, arma::vec &bary ) const
{
  // TODO: Optimize this by using diretly the inverse matrix, rather than solving
  arma::mat matrix(3,3);
  bary.set_size(3);
  for ( unsigned int i=0;i<3;i++ )
  {
    matrix(0,i) = nodes[tetra.nodes[i]].x - nodes[tetra.nodes[3]].x;
    matrix(1,i) = nodes[tetra.nodes[i]].y - nodes[tetra.nodes[3]].y;
    matrix(2,i) = nodes[tetra.nodes[i]].z - nodes[tetra.nodes[3]].z;
  }
  bary[0] = x-nodes[tetra.nodes[3]].x;
  bary[1] = y-nodes[tetra.nodes[3]].y;
  bary[2] = z-nodes[tetra.nodes[3]].z;
  bary = arma::solve(matrix,bary);
}

bool TetraGeometry::isInside( double x, double y, double z, const Tetrahedron &tetra ) const
{
  arma::vec bary;
  barycentric(x,y,z,tetra,bary);
  for ( unsigned int i=0;i<3;i++ )
  {
    if (( bary[i] < 0.0 ) || (bary[i] > 1.0 )) return false;
  }
  return true;
}

void TetraGeometry::boundingBox( double crn1[3], double crn2[3] ) const
{
  for ( unsigned int i=0;i<3;i++ )
  {
    crn1[i] = 1E30;
    crn2[i] = -1E30;
  }
  for ( unsigned int i=0;i<nodes.size(); i++ )
  {
    if ( nodes[i].x < crn1[0] ) crn1[0] = nodes[i].x;
    if ( nodes[i].y < crn1[1] ) crn1[1] = nodes[i].y;
    if ( nodes[i].z < crn1[2] ) crn1[2] = nodes[i].z;
    if ( nodes[i].x > crn2[0] ) crn2[0] = nodes[i].x;
    if ( nodes[i].y > crn2[1] ) crn2[1] = nodes[i].y;
    if ( nodes[i].z > crn2[2] ) crn2[2] = nodes[i].z;
  }
}

void TetraGeometry::centerOfMass( unsigned int id, double com[3] ) const
{
  com[0] = 0.0;
  com[1] = 0.0;
  com[2] = 0.0;
  for ( unsigned int i=0;i<4;i++ )
  {
    com[0] += nodes[elements[id].nodes[i]].x;
    com[1] += nodes[elements[id].nodes[i]].y;
    com[2] += nodes[elements[id].nodes[i]].z;
  }

  com[0] /= 4.0;
  com[1] /= 4.0;
  com[2] /= 4.0;
}
