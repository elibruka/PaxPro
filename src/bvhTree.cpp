#include "bvhTree.hpp"
#include "tetraGeometry.hpp"
#include <cassert>
#define DEBUG_BVH

using namespace std;

BVHTreeNode::~BVHTreeNode()
{
  delete left; left=nullptr;
  delete right; right=nullptr;
}

void BVHTreeNode::build( const TetraGeometry &tgeo )
{
  clog << "Building Bounded Volume Hierachy...\n";
  geo = &tgeo;
  for ( unsigned int i=0;i<geo->getTetras().size();i++ )
  {
    tetraID.push_back(geo->tetra(i).id);
  }

  // Set bounding box
  geo->boundingBox( crn1, crn2 );
  split();
  clog << "Finished...\n";
}

double BVHTreeNode::newVolume( array<double,3> &newcrn1, array<double,3> &newcrn2 ) const
{
  double xmin = newcrn1[0] < crn1[0] ? newcrn1[0]:crn1[0];
  double xmax = newcrn2[0] > crn2[0] ? newcrn2[0]:crn2[0];
  double ymin = newcrn1[1] < crn1[1] ? newcrn1[1]:crn1[1];
  double ymax = newcrn2[1] > crn2[1] ? newcrn2[1]:crn2[1];
  double zmin = newcrn1[2] < crn1[2] ? newcrn1[2]:crn1[2];
  double zmax = newcrn2[2] > crn2[2] ? newcrn2[2]:crn2[2];
  return (xmax-xmin)*(ymax-ymin)*(zmax-zmin);
}

double BVHTreeNode::volume() const
{
  double vol = 1.0;
  for ( unsigned int i=0;i<3;i++ )
  {
    vol *= (crn2[i]-crn1[i]);
  }
  return vol;
}

void BVHTreeNode::split()
{
  left = new BVHTreeNode();
  right = new BVHTreeNode();
  left->parent = this;
  right->parent = this;
  left->geo = geo;
  right->geo = geo;
  left->level = level+1;
  right->level = level+1;

  left->crn1 = crn1;
  left->crn2 = crn2;
  right->crn1 = crn1;
  right->crn2 = crn2;
  left->splitDirection = (splitDirection+1)%3;
  right->splitDirection = (splitDirection+1)%3;

  left->crn2[splitDirection] = (crn1[splitDirection] + crn2[splitDirection])/2.0;
  right->crn1[splitDirection] = left->crn2[splitDirection];

  unsigned int nInsertedLeft = 0;
  unsigned int nInsertedRight = 0;
  for ( unsigned int i=0;i<tetraID.size();i++ )
  {
    array<double,3> tetraCrn1;
    array<double,3> tetraCrn2;
    geo->tetraBound(tetraID[i],tetraCrn1,tetraCrn2);
    double newVolLeft = left->newVolume(tetraCrn1,tetraCrn2);
    double newVolRight = right->newVolume(tetraCrn1,tetraCrn2);

    if ( abs(newVolLeft-left->volume()) < abs(newVolRight-right->volume()) )
    {
      left->insert(tetraID[i],tetraCrn1,tetraCrn2);
      nInsertedLeft++;
    }
    else
    {
      right->insert(tetraID[i],tetraCrn1,tetraCrn2);
      nInsertedRight++;
    }
  }

  if ( (( nInsertedLeft == 0 ) && ( nInsertedRight > 1 )) || ((nInsertedRight==0) && (nInsertedLeft > 1)) )
  {
    balance(nInsertedLeft,nInsertedRight);
  }

  //left->printBoundingBox();
  //right->printBoundingBox();

  assert( (nInsertedLeft+nInsertedRight) == tetraID.size() );
  if ( nInsertedLeft > 1 )
  {
    assert( left != nullptr );
    left->split();
  }
  else if ( nInsertedLeft == 0 )
  {
    delete left; left=nullptr;
  }

  if ( nInsertedRight > 1 )
  {
    assert( right != nullptr );
    right->split();
  }
  else if ( nInsertedRight == 0 )
  {
    delete right; right=nullptr;
  }
}

void BVHTreeNode::insert( unsigned int id, array<double,3> &newcrn1, array<double,3> &newcrn2 )
{
  tetraID.push_back(id);
  for ( unsigned int i=0;i<3;i++ )
  {
    crn1[i] = newcrn1[i] < crn1[i] ? newcrn1[i]:crn1[i];
    crn2[i] = newcrn2[i] > crn2[i] ? newcrn2[i]:crn2[i];
  }
}

unsigned int BVHTreeNode::getID( double x, double y, double z )
{
  BVHTreeNode *current = this;
  unsigned int id = 0;
  while ( true )
  {

    if (( current->left==nullptr ) && ( current->right==nullptr ))
    {
      current->printBoundingBox();
      // Is a leaf node
      if ( geo->isInside( x,y,z, geo->tetra(current->tetraID[0])) )
      {
        id = tetraID[0];
        break;
      }
      current = current->parent;
    }

    if ( ( current->left != nullptr ) && current->left->isInside(x,y,z) && !current->hasCheckedLeft )
    {
      current->hasCheckedLeft = true;
      current = current->left;
    }
    else if ( (current->right!=nullptr) && current->right->isInside(x,y,z) && !current->hasCheckedRight )
    {
      current->hasCheckedRight = true;
      current = current->right;
    }
    else
    {
      // It should never return for the second time to the root node
      assert( !( ( current->parent == nullptr ) && ( current->hasCheckedLeft ) && ( current->hasCheckedRight ) ));
      assert( !((current->parent==nullptr) && current->hasCheckedLeft && !current->right->isInside(x,y,z)) );
      assert( !((current->parent==nullptr) && current->hasCheckedRight && !current->left->isInside(x,y,z)) );

      current->hasCheckedLeft = false;
      current->hasCheckedRight = false;
      current = current->parent;
    }
  }

  // Clean the tree on the path that was use to find the correct leaf node
  while( current != nullptr )
  {
    current->hasCheckedRight = false;
    current->hasCheckedLeft = false;
    current = current->parent;
  }
  return id;
}

bool BVHTreeNode::isInside( double x, double y, double z ) const
{
  return ( x > crn1[0] ) && ( x < crn2[0] ) && \
         ( y > crn1[1] ) && ( y < crn2[1] ) && \
         ( z > crn1[2] ) && ( z < crn2[2] );
}

void BVHTreeNode::printBoundingBox() const
{
  cout << "(" << crn1[0] << "," << crn1[1] << "," << crn1[2] << ") ";
  cout << "(" << crn2[0] << "," << crn2[1] << "," << crn2[2] << ")\n";
}

void BVHTreeNode::statistics()
{
  unsigned int counter = 0;
  unsigned int minDepth = 10000000;
  unsigned int maxDepth = 0;
  BVHTreeNode *current = this;
  while( true )
  {
    if (( current->left==nullptr ) && ( current->right==nullptr ))
    {
      // Is leaf node
      counter++;
      minDepth = current->level < minDepth ? current->level:minDepth;
      maxDepth = current->level > maxDepth ? current->level:maxDepth;
      current = current->parent;
    }

    if ( (current->left!=nullptr) && !current->hasCheckedLeft )
    {
      current->hasCheckedLeft = true;
      current = current->left;
    }
    else if ( (current->right!=nullptr) && !current->hasCheckedRight )
    {
      current->hasCheckedRight = true;
      current = current->right;
    }
    else
    {
      if (( current->parent == nullptr ) && ( current->hasCheckedLeft ) && ( current->hasCheckedRight ))
      {
        // The second time it returns to the root node --> the entire tree has been visited
        current->hasCheckedLeft = false;
        current->hasCheckedRight = false;
        break;
      }
      current->hasCheckedRight = false;
      current->hasCheckedLeft = false;
      current = current->parent;
    }
  }

  cout << "================== BVH STATISTICS ===============================\n";
  cout << "Number of leaf nodes: " << counter << endl;
  cout << "Maximum tree depth: " << maxDepth << endl;
  cout << "Minimum tree depth: " << minDepth << endl;
  cout << "==================================================================\n";
}

void BVHTreeNode::balance( unsigned int &nInsertedLeft, unsigned int &nInsertedRight )
{
  assert( geo != nullptr );
  assert( left != nullptr );
  assert( right != nullptr );

  left->tetraID.resize(0);
  right->tetraID.resize(0);

  // Find the two tetrahedrons that are furthest apart
  unsigned int root1, root2;
  double distance = -1E30;
  for ( unsigned int i=0;i<tetraID.size();i++ )
  for ( unsigned int j=i+1;j<tetraID.size();j++ )
  {
    double com1[3];
    double com2[3];
    geo->centerOfMass( tetraID[i], com1 );
    geo->centerOfMass( tetraID[j], com2 );
    double newdistance = pow( com1[0]-com2[0],2 ) + pow(com1[1]-com2[1],2 ) + pow( com1[2]-com2[2], 2 );
    if ( newdistance > distance )
    {
      root1 = tetraID[i];
      root2 = tetraID[j];
      distance = newdistance;
    }
  }

  geo->tetraBound( root1, left->crn1, left->crn2 );
  left->insert( root1, left->crn1, left->crn2 );
  geo->tetraBound( root2, right->crn1, right->crn2 );
  right->insert( root2, right->crn1, right->crn2 );
  nInsertedLeft = 1;
  nInsertedRight = 1;

  // Insert the rest
  for ( unsigned int i=0;i<tetraID.size();i++ )
  {
    if (( tetraID[i] == root1 ) || ( tetraID[i] == root2 )) continue;
    array<double,3> tetraCrn1;
    array<double,3> tetraCrn2;
    geo->tetraBound(tetraID[i],tetraCrn1,tetraCrn2);
    double newVolLeft = left->newVolume(tetraCrn1,tetraCrn2);
    double newVolRight = right->newVolume(tetraCrn1,tetraCrn2);

    if ( abs(newVolLeft-left->volume()) < abs(newVolRight-right->volume()) )
    {
      left->insert(tetraID[i],tetraCrn1,tetraCrn2);
      nInsertedLeft++;
    }
    else
    {
      right->insert(tetraID[i],tetraCrn1,tetraCrn2);
      nInsertedRight++;
    }
  }
}
