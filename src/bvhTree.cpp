#include "bvhTree.hpp"
#include "tetraGeometry.hpp"

using namespace std;

BVHTreeNode::~BVHTreeNode()
{
  delete left; left=nullptr;
  delete right; right=nullptr;
}

void BVHTreeNode::build( const TetraGeometry &tgeo )
{
  geo = &tgeo;
  for ( unsigned int i=0;i<geo->getTetras().size();i++ )
  {
    tetraID.push_back(i);
  }
  split();
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

void BVHTreeNode::split()
{
  left = new BVHTreeNode();
  right = new BVHTreeNode();
  left->crn1 = crn1;
  left->crn2 = crn2;
  right->crn1 = crn1;
  right->crn2 = crn2;
  left->splitDirection = (splitDirection+1)%3;
  right->splitDirection = (splitDirection+1)%3;

  left->crn2[splitDirection] = crn2[splitDirection]/2.0;
  right->crn1[splitDirection] = crn2[splitDirection]/2.0;

  unsigned int nInsertedLeft = 0;
  unsigned int nInsertedRight = 0;
  for ( unsigned int i=0;i<tetraID.size();i++ )
  {
    array<double,3> tetraCrn1;
    array<double,3> tetraCrn2;
    geo->tetraBound(i,tetraCrn1,tetraCrn2);
    double newVolLeft = left->newVolume(tetraCrn1,tetraCrn2);
    double newVolRight = right->newVolume(tetraCrn1,tetraCrn2);
    if ( newVolLeft < newVolRight )
    {
      left->insert(i,tetraCrn1,tetraCrn2);
      nInsertedLeft++;
    }
    else
    {
      right->insert(i,tetraCrn1,tetraCrn2);
      nInsertedRight++;
    }
  }

  if ( nInsertedLeft > 1 )
  {
    left->split();
  }
  else if ( nInsertedLeft == 0 )
  {
    delete left; left=nullptr;
  }

  if ( nInsertedRight > 1 )
  {
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
