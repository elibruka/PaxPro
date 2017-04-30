#ifndef BVH_TREE_H
#define BVH_TREE_H
#include <array>
#include <vector>

class TetraGeometry;
class BVHTreeNode
{
public:
  BVHTreeNode();
  ~BVHTreeNode();

  /** Builds the BVH tree */
  void build( const TetraGeometry &geo );

  /** Returns the ID of the tetrahedron */
  unsigned int getID( double x, double y, double z ) const;

  /** Computes the new volume of the bounding box if a box defined by crn1 and crn2 is added */
  double newVolume( std::array<double,3> &crn1, std::array<double,3> &crn2 ) const;

  /** Insert new ID */
  void insert( unsigned int id, std::array<double,3> &crn1, std::array<double,3> &crn2 );
private:
  std::array<double,3> crn1;
  std::array<double,3> crn2;
  BVHTreeNode *left{nullptr};
  BVHTreeNode *right{nullptr};
  std::vector<unsigned int> tetraID;
  unsigned int splitDirection{2};
  const TetraGeometry *geo{nullptr};

  /** Void split */
  void split();
};
#endif
