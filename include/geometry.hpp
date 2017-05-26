#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "shapes.hpp"
#include <string>
#include <vector>

namespace geom
{
  enum class Operation_t{UNION, DIFFERENCE};
  class Part
  {
  public:
    Part( const char* name ): name(name){};

    /** Make union with the new shape */
    void add( const Shape &shape );

    /** Make difference with the new shape */
    void difference( const Shape &shape );

    /** Returns true if is inside */
    bool isInside( double x, double y, double z ) const;

    /** Simply dump all the objects in order to an openSCAD file  */
    void dump( const char* fname ) const;

    /** Saves the object including differences and unions */
    void save( const char* fname ) const;

    /** Real part of the refractive index */
    double delta{0.0};

    /** Imaginary part of the refractive index */
    double beta{0.0};
  private:
    std::string name;
    std::vector<const Shape*> shapes;
    std::vector<Operation_t> operations;
  };
};
#endif
