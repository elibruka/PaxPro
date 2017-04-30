#ifndef GEOMETRY_SIMPLICIES_H
#define GEOMETRY_SIMPLICIES_H
#include <array>

struct Node
{
  double x;
  double y;
  double z;
};

struct Tetrahedron
{
  unsigned int id;
  std::array<unsigned int,4> nodes;
};
#endif
