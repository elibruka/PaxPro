#ifndef BVH_TREE_H
#define BVH_TREE_H
#include <array>
#include <vector>

class TetraGeometry;
class BVHTreeNode
{
public:
  BVHTreeNode(){};
  BVHTreeNode( const BVHTreeNode &other );
  ~BVHTreeNode();

  /** Builds the BVH tree */
  void build( const TetraGeometry &geo );

  /** Returns the ID of the tetrahedron. Returns -1 if it did not find a tetrahedron */
  int getID( double x, double y, double z );

  /** Computes the new volume of the bounding box if a box defined by crn1 and crn2 is added */
  double newVolume( std::array<double,3> &crn1, std::array<double,3> &crn2 ) const;

  /** Get volume of current */
  double volume() const;

  /** Insert new ID */
  void insert( unsigned int id, std::array<double,3> &crn1, std::array<double,3> &crn2 );

  /** Counts the number of leaf nodes */
  void statistics();

  /** Balance the tree if skew */
  void balance( unsigned int &nInsertedLeft, unsigned int &nInsertedRight );
private:
  std::array<double,3> crn1;
  std::array<double,3> crn2;
  BVHTreeNode *left{nullptr};
  BVHTreeNode *right{nullptr};
  BVHTreeNode *parent{nullptr};
  std::vector<unsigned int> tetraID;
  unsigned int splitDirection{2};
  const TetraGeometry *geo{nullptr};
  bool hasCheckedLeft{false};
  bool hasCheckedRight{false};
  unsigned int level{0};

  /** Void split */
  void split();

  /** Return true if it is inside the bounding box */
  bool isInside( double x, double y, double z ) const;

  /** Prints the bounding box coordinates to the console */
  void printBoundingBox() const;
};
#endif
