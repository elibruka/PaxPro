#ifndef TETRAGEOMETRY_H
#define TETRAGEOMETRY_H
#include "materialFunction.hpp"
#include "geometrySimplices.hpp"
#include "bvhTree.hpp"
#include <vector>
#include <fstream>
#include <armadillo>
#include <map>
#include <array>

struct XrayMatProperty
{
  double delta{0.0};
  double beta{0.0};
};

class TetraGeometry: public MaterialFunction
{
public:
  TetraGeometry(){};
  TetraGeometry( const TetraGeometry &other );
  virtual ~TetraGeometry();

  /** Load GMSH mesh file */
  void load( const char* fname );

  /** Returns the X-ray material functions */
  void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) override;

  /** Returns the array of tetrahedrons */
  const std::vector<Tetrahedron>& getTetras() const{ return elements; };

  /** Return the tetrahedron with with given id */
  const Tetrahedron& tetra( unsigned int id ) const { return elements[id]; };

  /** Return the center of mass of tetrahedon id */
  void centerOfMass( unsigned int id, double com[3] ) const;

  /** Get the bounding box */
  void boundingBox( std::array<double,3> &crn1, std::array<double,3> &crn2 ) const;

  /** Set X-ray material properties */
  void setMatProp( const std::map<std::string,XrayMatProperty> &matprops );

  /** Computes the bounding box of individual elements */
  void tetraBound( unsigned int id, std::array<double,3> &crn1, std::array<double,3> &crn2 ) const;

  /** Returns true if the point is inside the tetrahedron */
  bool isInside( double x, double y, double z, const Tetrahedron &tetra ) const;

  /** Relevant length scale: all coordinates in the .msh file are multiplied with this number */
  double lengthScale{1.0};
private:
  std::vector<double> delta;
  std::vector<double> beta;
  std::vector<Tetrahedron> elements;
  std::vector<Node> nodes;

  BVHTreeNode *bvh{nullptr};

  /** Reads the nodes from the GMSH mesh file */
  void readNodes( std::ifstream &infile );

  /** Reads the elements from the gmsh mesh file */
  void readElements( std::ifstream &infile );

  /** Computes the barycentric coordinate wrt tetra*/
  void barycentric( double x, double y, double z, const Tetrahedron &tetra, arma::vec &bary ) const;

  /** Reads the physical entities */
  void readPhysicalEntities( std::ifstream &infile );

  /** Checks that imported mesh is consistent */
  void checkImportedMesh();

  unsigned int previousID{0};

  std::map<std::string,unsigned int> physicalEntityNumber;

  bool boundingBoxComputed{false};
  std::array<double,3> bboxCrn1;
  std::array<double,3> bboxCrn2;

  /** Update the bounding box */
  void boundingBox();
};
#endif
