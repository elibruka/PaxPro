#ifndef TETRAGEOMETRY_H
#define TETRAGEOMETRY_H
#include "materialFunction.hpp"
#include "geometrySimplices.hpp"
#include <vector>
#include <fstream>
#include <armadillo>

class TetraGeometry: public MaterialFunction
{
public:
  TetraGeometry(){};

  /** Load GMSH mesh file */
  void load( const char* fname );

  /** Returns the X-ray material functions */
  void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const override{};

  /** Returns the array of tetrahedrons */
  const std::vector<Tetrahedron>& getTetras() const{ return elements; };

  /** Return the tetrahedron with with given id */
  const Tetrahedron& tetra( unsigned int id ){ return elements[id]; };

  /** Return the center of mass of tetrahedon id */
  void centerOfMass( unsigned int id, double com[3] ) const;

  /** Get the bounding box */
  void boundingBox( double crn1[3], double crn2[3] ) const;
private:
  std::vector<double> delta;
  std::vector<double> beta;
  std::vector<Tetrahedron> elements;
  std::vector<Node> nodes;

  /** Reads the nodes from the GMSH mesh file */
  void readNodes( std::ifstream &infile );

  /** Reads the elements from the gmsh mesh file */
  void readElements( std::ifstream &infile );

  /** Computes the barycentric coordinate wrt tetra*/
  void barycentric( double x, double y, double z, const Tetrahedron &tetra, arma::vec &bary ) const;

  /** Returns true if the point is inside the tetrahedron */
  bool isInside( double x, double y, double z, const Tetrahedron &tetra ) const;
};
#endif
