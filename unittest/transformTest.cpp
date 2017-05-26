#include <gtest/gtest.h>
#include "shapes.hpp"

TEST( geometry, rotateZ )
{
  // Create a sphere with radius 10.0
  geom::Sphere sphere(10.0);

  // Rotate 90 degree about the z axis
  sphere.rotate( 90.0, geom::Axis_t::Z );

  double x = 1.0;
  double y = 0.0;
  double z = 0.0;
  double xExp = 0.0;
  double yExp = -1.0;
  double zExp = 0.0;

  sphere.transform( x, y, z );

  EXPECT_NEAR( x, xExp, 1E-6 );
  EXPECT_NEAR( y, yExp, 1E-6 );
  EXPECT_NEAR( z, zExp, 1E-6 );
};

TEST( geometry, rotateY )
{
  geom::Sphere sphere(10.0);
  sphere.rotate(90.0, geom::Axis_t::Y );
  double x = 1.0;
  double y = 0.0;
  double z = 0.0;

  sphere.transform( x, y, z );
  EXPECT_NEAR( x, 0.0, 1E-6 );
  EXPECT_NEAR( y, 0.0, 1E-6 );
  EXPECT_NEAR( z, 1.0, 1E-6 );
}

TEST( geometry, rotateX )
{
  geom::Sphere sphere(10.0);
  sphere.rotate(90.0, geom::Axis_t::X );
  double x = 0.0;
  double y = 1.0;
  double z = 0.0;

  sphere.transform( x, y, z );
  EXPECT_NEAR( x, 0.0, 1E-6 );
  EXPECT_NEAR( y, 0.0, 1E-6 );
  EXPECT_NEAR( z, -1.0, 1E-6 );
}

TEST( geometry, translation )
{
  geom::Sphere sphere(10.0);
  sphere.translate( 10.0, -10.0, -3.0 );
  double x = 1.0;
  double y = -1.0;
  double z = 10.0;
  sphere.transform( x, y, z );
  EXPECT_NEAR( x, -9.0, 1E-6 );
  EXPECT_NEAR( y, 9.0, 1E-6 );
  EXPECT_NEAR( z, 13.0, 1E-6 );
}
