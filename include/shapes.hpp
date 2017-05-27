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

    /** Return a clone of this object */
    virtual Shape* clone() = 0;

    /** Returns the code segment that represents the object */
    void openSCADExport( std::string &code ) const;

    /** Translate the object in the x, y and z direction respectively */
    void translate( double x, double y, double z );

    /** Rotates the object */
    void rotate( double angleDeg, Axis_t axis );

    /** Scale an object along one of its axes */
    void scale( double factor, Axis_t axis );

    /** Transform the coordinate according to the orientation of the shape */
    void transform( double &x, double &y, double  &z ) const;

    /** Computes the inverse geometrical transform */
    void inverseTransform( double &x, double &y, double &z ) const;

    /** Get the transformation matrix */
    const arma::mat& getTransformation() const { return transformation; };

    /** Computes the inverse transformation matrix */
    void getInverseTransformation( arma::mat &inverse ) const;

    /** Get the name of the part */
    const std::string& getName() const { return name; };
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

    /** Returns a pointer to a clone of this class */
    virtual Shape* clone() override { return new Sphere(*this); };
  protected:
    double radius{0.0};
  };

  /** A class that implements a box */
  class Box: public Shape
  {
  public:
    Box( double Lx, double Ly, double Lz ): Shape("Cube"), Lx(Lx), Ly(Ly), Lz(Lz){};

    /** Override */
    virtual bool isInside( double x, double y, double z ) const override final;

    /** Overide */
    virtual void openSCADDescription( std::string &description) const override final;

    /** Returns a pointer to a clone of this box */
    virtual Shape* clone() override { return new Box(*this); };
  protected:
    double Lx;
    double Ly;
    double Lz;
  };

  class Cylinder: public Shape
  {
  public:
    Cylinder( double radius, double height ): Shape("Cylindger"), r1(radius), r2(radius), height(height){};
    Cylinder( double r1, double r2, double height ): Shape("Cylinder"), r1(r1), r2(r2), height(height){};

    /** Override */
    virtual bool isInside( double x, double y, double z ) const override final;

    /** Overide */
    virtual void openSCADDescription( std::string &description) const override final;

    /** Returns a pointer to a clone of this object */
    virtual Shape* clone() override { return new Cylinder(*this); };
  protected:
    double r1;
    double r2;
    double height;
  };
};

#endif
