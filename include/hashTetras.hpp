#ifndef HASH_TETRAS_H
#define HASH_TETRAS_H
#include <vector>

class TetraGeometry;
class HashedTetras
{
public:
  HashedTetras( const TetraGeometry &geo ): geo(&geo){};

  /** Builds a hash table to for easier lookup of the elements */
  void build();

  /** Print info about the table */
  void printStatistics() const;

  /** Return the bucket where x,y,z belongs */
  const std::vector<unsigned int>& getBucket( double x, double y, double z ) const;
private:
  const TetraGeometry* geo{nullptr};
  std::vector< std::vector<unsigned int> > buckets;
  double xmin{0.0};
  double xmax{0.0};
  double ymin{0.0};
  double ymax{0.0};
  double zmin{0.0};
  double zmax{0.0};
  unsigned int N{1}; // Equally many buckets in all dimensions
  unsigned int Nlower{1};
  unsigned int Nupper{1};

  /** Returns the bucket of which the point x,y,z belongs */
  unsigned int bucketIndex( double x, double y, double z ) const;

  /** Inserts the ID into the correct bucket */
  void insertID( unsigned int id, double com[3] );

  /** Builds the LUT with N buckets */
  void setupTable();
};
#endif
