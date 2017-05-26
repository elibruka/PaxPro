#ifndef SHAPES_H
#define SHAPES_H

#include <string>
#include <armadillo>

namespace geom
{
  enum class Axis_t { X, Y, Z };
  /** Base class for all shapes */
  class Shape
  {
  public:
    Shape( const char* name );

    /** Returns true if a point is inside the object */
    virtual bool isInside( double x, double y, double z ) const = 0;

    /** A string that should returns the corresponding openSCAD description of the object */
    virtual void openSCADDescription( std::string &description ) const = 0;

    /** Returns the code segment that represents the object */
    void openSCADExport( std::string &code ) const;

    /** Translate the object in the x, y and z direction respectively */
    void translate( double x, double y, double z );

    /** Rotates the object */
    void rotate( double angleDeg, Axis_t axis );

    /** Transform the coordinate according to the orientation of the shape */
    void transform( double &x, double &y, double  &z ) const;
  protected:
    std::string name;
    arma::mat transformation;
  };

  /** A class that implements a sphere */
  class Sphere: public Shape
  {
  public:
    Sphere( double radius ): Shape("sphere"), radius(radius){};

    /** Override the isInside function */
    virtual bool isInside( double x, double y, double z ) const override final;

    /** Override openSCADExport */
    virtual void openSCADDescription( std::string &description ) const override final;
  protected:
    double radius{0.0};
  };
};

#endif
