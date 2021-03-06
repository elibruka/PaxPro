#include "gaussianBeam.hpp"
#include <cmath>

using namespace std;
const double PI = acos(-1.0);

double GaussianBeam::rayleighRange() const
{
  return PI*waist*waist/getWavelength();
}

double GaussianBeam::guoyPhase( double z ) const
{
  return atan( z/rayleighRange() );
}

double GaussianBeam::inverseRadiusOfCurvature( double z ) const
{
  double zR = rayleighRange();
  return z/( z*z + zR*zR);
}

double GaussianBeam::spotSize( double z ) const
{
  double zR = rayleighRange();
  return waist*sqrt( 1.0 + pow(z/zR,2));
}

cdouble GaussianBeam::get( double x, double z ) const
{
  x -= centerX;
  cdouble im(0.0,1.0);
  double waistRatio = waist/spotSize( z-z0 );
  double gaussianFactor = exp(-pow(x/spotSize(z-z0),2) );
  cdouble phaseFactor = exp(-im*wavenumber*z0 + 0.5*im*wavenumber*x*x*inverseRadiusOfCurvature(z-z0) \
                          -im*guoyPhase(z-z0));
  cdouble ang_phaseX = angle_factorX(x);
  return amplitude*waistRatio*gaussianFactor*phaseFactor*ang_phaseX;
}

cdouble GaussianBeam::angle_factorX(double x) const
{
  double ang_rad = angleX * PI / 180.0;
  double k = getWavenumber();
  cdouble im(0.0,1.0);
  return exp(im*k*sin(ang_rad)*x);
}

cdouble GaussianBeam::angle_factorY(double y) const
{
  double ang_rad = angleY * PI / 180.0;
  double k = getWavenumber();
  cdouble im(0.0,1.0);
  return exp(im*k*sin(ang_rad)*y);
}

cdouble GaussianBeam::get( double x, double y, double z ) const
{
  y -= centerY;
  double gaussY = exp( -pow(y/spotSize(z-z0),2) );
  cdouble angY = angle_factorY(y);
  return get(x,z)*gaussY*angY;
}

void GaussianBeam::setCenter( double xc, double yc )
{
  centerX = xc;
  centerY = yc;
}

double GaussianBeam::beamDivergence() const
{
  return getWavelength()/(PI*waist);
}

void GaussianBeam::info( Json::Value &obj ) const
{
  ParaxialSource::info( obj );
  obj["waist"] = waist;
  obj["divergenceDeg"] = beamDivergence()*180.0/PI;
}
