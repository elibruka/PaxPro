#include "tetraGeometry.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <armadillo>
#include <cassert>
//#define TETRA_DEBUG

using namespace std;

TetraGeometry::~TetraGeometry()
{
  delete bvh; bvh=nullptr;
}

TetraGeometry::TetraGeometry( const TetraGeometry &other )
{
  delta = other.delta;
  beta = other.beta;
  elements = other.elements;
  nodes = other.nodes;
  if ( other.bvh != nullptr )
  {
    bvh = new BVHTreeNode(*other.bvh);
    #ifdef TETRA_DEBUG
      bvh->statistics();
    #endif
  }
  previousID = other.previousID;
  physicalEntityNumber = other.physicalEntityNumber;
  boundingBoxComputed = other.boundingBoxComputed;
  bboxCrn1 = other.bboxCrn1;
  bboxCrn2 = other.bboxCrn2;
}

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
    if ( line.find("$PhysicalNames") != string::npos )
    {
      readPhysicalEntities( infile );
    }
    if ( line.find("$Nodes") != string::npos )
    {
      clog << "Reading nodes...\n";
      readNodes( infile );

      #ifdef TETRA_DEBUG
        cout << "Read " << nodes.size() << " nodes...\n";
      #endif
    }

    if ( line.find("$Elements") != string::npos )
    {
      clog << "Reading elements...\n";
      readElements( infile );
      #ifdef TETRA_DEBUG
        cout << "Read " << elements.size() << " elements\n";
      #endif
    }
  }
  infile.close();
  checkImportedMesh();
  boundingBox();

  delete bvh;
  bvh = new BVHTreeNode();
  bvh->build( *this );
  bvh->statistics();
}

void TetraGeometry::readNodes( ifstream &infile )
{
  string line;
  getline(infile,line);
  stringstream ss;
  ss << line;
  int numberOfNodes;
  ss >> numberOfNodes;
  nodes.resize(numberOfNodes);
  while( getline(infile, line) )
  {
    if ( line.find("$EndNodes") != string::npos ) return;

    stringstream ss(line);
    int indx;
    Node newnode;
    ss >> indx >> newnode.x >> newnode.y >> newnode.z;

    newnode.x *= lengthScale;
    newnode.y *= lengthScale;
    newnode.z *= lengthScale;

    #ifdef TETRA_DEBUG
      cout << "x="<<newnode.x << " y=" << newnode.y << " z=" << newnode.z << endl;
    #endif
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
  unsigned int id = 0;
  while( getline(infile, line) )
  {
    if ( line.find("$EndElements") != string::npos ) return;
    int elemNumber;
    int elemType;
    int numberOfTags;
    stringstream ss(line);
    ss >> elemNumber >> elemType >> numberOfTags;
    if (( elemType != 4 ) || (numberOfTags == 0 )) continue;
    int physicalEntity;
    ss >> physicalEntity;
    int dummy;
    for ( unsigned int i=0;i<numberOfTags-1;i++ )
    {
      ss >> dummy;
    }
    Tetrahedron tetra;
    tetra.id = id++;
    tetra.physicalEntity = physicalEntity-1;
    ss >> tetra.nodes[0] >> tetra.nodes[1] >> tetra.nodes[2] >> tetra.nodes[3];
    for ( unsigned int i=0;i<4;i++ )
    {
      tetra.nodes[i] -= 1;
    }
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

void TetraGeometry::boundingBox()
{
  for ( unsigned int i=0;i<3;i++ )
  {
    bboxCrn1[i] = 1E30;
    bboxCrn2[i] = -1E30;
  }
  for ( unsigned int i=0;i<nodes.size(); i++ )
  {
    if ( nodes[i].x < bboxCrn1[0] ) bboxCrn1[0] = nodes[i].x;
    if ( nodes[i].y < bboxCrn1[1] ) bboxCrn1[1] = nodes[i].y;
    if ( nodes[i].z < bboxCrn1[2] ) bboxCrn1[2] = nodes[i].z;
    if ( nodes[i].x > bboxCrn2[0] ) bboxCrn2[0] = nodes[i].x;
    if ( nodes[i].y > bboxCrn2[1] ) bboxCrn2[1] = nodes[i].y;
    if ( nodes[i].z > bboxCrn2[2] ) bboxCrn2[2] = nodes[i].z;
  }
  boundingBoxComputed = true;
}

void TetraGeometry::boundingBox( array<double,3> &crn1, array<double,3> &crn2 ) const
{
  assert( boundingBoxComputed );
  crn1 = bboxCrn1;
  crn2 = bboxCrn2;
}

void TetraGeometry::centerOfMass( unsigned int id, double com[3] ) const
{
  assert( id < elements.size() );
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

void TetraGeometry::checkImportedMesh()
{
  unsigned int maxNodeID = nodes.size();
  for ( unsigned int i=0;i<elements.size();i++ )
  {
    int maxNodeIdInElement = 0;
    for ( unsigned int j=0;j<4;j++ )
    {
      if ( elements[i].nodes[j] > maxNodeID )
      {
        stringstream msg;
        msg << "Element requests a node that is out of bounds. Number of nodes: ";
        msg << maxNodeID;
        msg << " Requested node: " << elements[i].nodes[j];
        throw( runtime_error(msg.str()) );
      }
    }
  }
}

void TetraGeometry::readPhysicalEntities( ifstream &infile )
{
  string line;
  getline(infile,line); // Number of physical entities
  while( getline(infile,line) )
  {
    if ( line.find("$EndPhysicalNames") != string::npos ) return;
    stringstream ss;
    ss << line;
    int dim, number;
    string name;
    ss >> dim >> number >> name;

    // Remove quotes
    name = name.substr(1,name.length()-2);
    physicalEntityNumber.insert( pair<string,unsigned int>(name,number-1) );
  }
}

void TetraGeometry::setMatProp( const map<string,XrayMatProperty> &matprops )
{
  if ( matprops.size() != physicalEntityNumber.size() )
  {
    stringstream msg;
    msg << "Material properties list has to be of the same size as the number of physical entities.";
    msg << " Number of physical entities " << physicalEntityNumber.size();
    msg << " Number of material properties " << matprops.size();
    throw( runtime_error(msg.str()) );
  }

  delta.resize( matprops.size() );
  beta.resize( matprops.size() );
  for ( auto iter=matprops.begin(); iter != matprops.end(); ++iter )
  {
    // Check that key exists
    if ( physicalEntityNumber.find(iter->first) == physicalEntityNumber.end() )
    {
      string msg("Key ");
      msg += iter->first;
      msg += " does not exist in the physical entities!";
      throw( runtime_error(msg) );
    }
    delta[physicalEntityNumber.at(iter->first)] = iter->second.delta;
    beta[physicalEntityNumber.at(iter->first)] = iter->second.beta;
  }
}

void TetraGeometry::tetraBound( unsigned int id, array<double,3> &crn1, array<double,3> &crn2 ) const
{
  crn1.fill( 1E30 );
  crn2.fill( -1E30 );
  for ( unsigned int i=0;i<4;i++ )
  {
    if ( nodes[elements[id].nodes[i]].x < crn1[0] ) crn1[0] = nodes[elements[id].nodes[i]].x;
    if ( nodes[elements[id].nodes[i]].y < crn1[1] ) crn1[1] = nodes[elements[id].nodes[i]].y;
    if ( nodes[elements[id].nodes[i]].z < crn1[2] ) crn1[2] = nodes[elements[id].nodes[i]].z;
    if ( nodes[elements[id].nodes[i]].x > crn2[0] ) crn2[0] = nodes[elements[id].nodes[i]].x;
    if ( nodes[elements[id].nodes[i]].y > crn2[1] ) crn2[1] = nodes[elements[id].nodes[i]].y;
    if ( nodes[elements[id].nodes[i]].z > crn2[2] ) crn2[2] = nodes[elements[id].nodes[i]].z;
  }
}

void TetraGeometry::getXrayMatProp( double x, double y, double z, double &matdelta, double &matbeta )
{
  assert( boundingBoxComputed );
  if (( x <= bboxCrn1[0] ) || ( x >= bboxCrn2[0] ) || \
      ( y <= bboxCrn1[1] ) || ( y >= bboxCrn2[1] ) || \
      ( z <= bboxCrn1[2] ) || ( z >= bboxCrn2[2] ))
    {
      //cout << "Warning! Requested coordinate is outside the bounding box!\n";
      matdelta = 0.0;
      matbeta = 0.0;
      return;
    }

  if ( !isInside(x,y,z, elements[previousID]) )
  {
    assert( bvh != nullptr );
    int newID = bvh->getID(x,y,z);
    if ( newID < 0 )
    {
      // Could not find any tetrahedron, treat as vacuum
      matdelta = 0.0;
      matbeta = 0.0;
      return;
    }
    previousID = newID;
  }

  matdelta = delta[elements[previousID].physicalEntity];
  matbeta = beta[elements[previousID].physicalEntity];
}
