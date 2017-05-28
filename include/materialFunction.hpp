#ifndef MATERIAL_FUNCTION_H
#define MATERIAL_FUNCTION_H
#include "geometry.hpp"
#include <vector>

class MaterialFunction
{
public:
  virtual void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const = 0;
};

class OpenSCADMaterial: public MaterialFunction
{
public:
  OpenSCADMaterial(){};

  virtual void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const override;

  /** Adds a new part */
  void addModule( const geom::Module &newmodule );

  double deltaSurrounding{0.0};
  double betaSurrounding{0.0};

  /** Boolean that disables the material search. The refracive index of the surroundings is returned for all x,y,z */
  bool isReferenceRun{false};
protected:
  std::vector<const geom::Module*> modules;
};
#endif
