#ifndef TETRAGEOMETRY_H
#define TETRAGEOMETRY_H
#include "materialFunction.hpp"
#include <vector>
#include <fstream>

struct Node
{
  double x;
  double y;
  double z;
};

struct Tetrahedron
{
  unsigned int id;
  unsigned int nodes[4];
};

class TetraGeometry: public MaterialFunction
{
public:
  TetraGeometry(){};

  /** Load GMSH mesh file */
  void load( const char* fname );
private:
  std::vector<double> delta;
  std::vector<double> beta;
  std::vector<Tetrahedron> tetras;
  std::vector<Node> nodes;

  /** Reads the nodes from the GMSH mesh file */
  void readNodes( std::ifstream &infile );

  /** Reads the elements from the gmsh mesh file */
  void readElements( std::ifstream &infile );
};
#endif
