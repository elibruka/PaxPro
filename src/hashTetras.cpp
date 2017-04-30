#include "hashTetras.hpp"
#include "tetraGeometry.hpp"
#include <iostream>
#include <cmath>
//#define DEBUG_HASHTETRAS

using namespace std;

unsigned int HashedTetras::bucketIndex( double x, double y, double z ) const
{
  unsigned int row = (x-xmin)*N/(xmax-xmin);
  unsigned int col = (y-ymin)*N/(ymax-ymin);
  unsigned int slice = (z-zmin)*N/(zmax-zmin);
  return slice*N*N + col*N + row;
}

void HashedTetras::setupTable()
{
  buckets.clear();
  buckets.resize(N*N*N);
  double crn1[3], crn2[3];
  geo->boundingBox(crn1,crn2);
  xmin = crn1[0];
  ymin = crn1[1];
  zmin = crn1[2];
  xmax = crn2[0];
  ymax = crn2[1];
  zmax = crn2[2];

  for ( unsigned int i=0;i<geo->getTetras().size();i++ )
  {
    double com[3];
    geo->centerOfMass(i,com);
    #ifdef DEBUG_HASHTETRAS
      clog << "COM: " << com[0] << " " << com[1] << " " << com[2] << endl;
    #endif
    insertID(i,com);
  }
}

void HashedTetras::build()
{
  Nupper = 2*pow( geo->getTetras().size(), 1.0/3.0 );
  Nlower = 1;
  unsigned int maxiter = 100;
  for ( unsigned int i=0;i<maxiter;i++ )
  {
    clog << "Optimizing LUT: Nupper=" << Nupper << " Nlower=" << Nlower << endl;
    N = (Nupper+Nlower)/2;
    setupTable();
    if (N==Nlower )
    {
      cout << "Cannot optimize LUT further...\n";
      break;
    }

    bool atLeastOneBucketHasOnlyOneEntry = false;
    bool emptyBucketsExist = false;
    for ( unsigned int i=0;i<buckets.size();i++ )
    {
      if ( buckets[i].size() == 0 )
      {
        emptyBucketsExist = true;
        break;
      }
      else if ( buckets[i].size() == 1 )
      {
        atLeastOneBucketHasOnlyOneEntry = true;
      }
    }

    if ( emptyBucketsExist )
    {
      // Reduce the number of buckets
      Nupper = N;
    }
    else if ( !atLeastOneBucketHasOnlyOneEntry )
    {
      // Increase number of buckets
      Nlower = N;
    }
    else
    {
      break;
    }

    if ( i == (maxiter-1) )
    {
      cout << "Maximum number of iterations reached while building LUT table for tetrahedrons...\n";
    }
  }
  //N = Nlower;
  printStatistics();
}

void HashedTetras::insertID( unsigned int id, double com[3] )
{
  unsigned int bucket = bucketIndex( com[0], com[1], com[2] );
  buckets[bucket].push_back(id);
}

void HashedTetras::printStatistics() const
{
  unsigned int maxTetra = 0;
  int minTetra = 100000000;
  double average = 0.0;
  for ( unsigned int i=0;i<buckets.size();i++ )
  {
    if ( buckets[i].size() < minTetra ) minTetra = buckets[i].size();
    if ( buckets[i].size() > maxTetra ) maxTetra = buckets[i].size();
    average += buckets[i].size();
  }
  average /= buckets.size();
  cout << "========== LUT TABLE FOR TETRAHEDRONS ===========================\n";
  cout << "Number of buckets: " << buckets.size() << endl;
  cout << "Maximum number of tetrahedrons in bucket: " << maxTetra << endl;
  cout << "Minimum number of tetrahedrons in bucket: " << minTetra << endl;
  cout << "Average number of tetrahedrons in bucket: " << average << endl;
  cout << "=================================================================\n";
}

const vector<unsigned int>& HashedTetras::getBucket( double x, double y, double z ) const
{
  return buckets[bucketIndex(x,y,z)];
}
