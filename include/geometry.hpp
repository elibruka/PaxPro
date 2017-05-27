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
    ~Part();
    Part( const Part &other );
    Part& operator =( const Part &rhs );

    /** Make union with the new shape */
    void add( Shape &shape );

    /** Make difference with the new shape */
    void difference( Shape &shape );

    /** Translate by (x,y,z) */
    void translate( double x, double y, double z );

    /** Rotate the object */
    void rotate( double angle, Axis_t axis );

    /** Returns true if is inside */
    bool isInside( double x, double y, double z ) const;

    /** Simply dump all the objects to an openSCAD file  */
    void dump( const char* fname ) const;

    /** Saves the object including differences and unions */
    void save( const char* fname ) const;

    /** Returns the name of the part */
    const std::string& getName() const { return name; };

    /** Real part of the refractive index */
    double delta{0.0};

    /** Imaginary part of the refractive index */
    double beta{0.0};
  private:
    std::string name;
    std::vector<Shape*> shapes;
    std::vector<Operation_t> operations;
    bool ownShapeObjects{false};

    /** Swaps member variables */
    void swap( const Part &other );
  };

  /** A module is a collection of parts */
  class Module
  {
  public:
    Module( const char* name ): name(name){};
    ~Module();
    Module( const Module &other );
    Module& operator =( const Module &rhs );

    /** Adds a new part */
    void add( Part &newpart );

    /** Make difference between two parts */
    void difference( Part &newpart );

    /** Returns the X-ray material properties at position (x,y,z). Return true if the point belongs to the module */
    bool getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const;

    /** Translate the entire module */
    void translate( double x, double y, double z );

    /** Rotate the entire module */
    void rotate( double angleDeg, Axis_t axis );

    /** Saves the individual parts to openSCAD files */
    void saveIndividualParts( const char* prefix );
  private:
    std::vector<Part*> parts;
    std::vector<Operation_t> operations;
    std::string name;
    bool ownPartObjects{false};

    void swap( const Module &other );
  };
};
#endif
