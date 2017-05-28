#include "materialFunction.hpp"

void OpenSCADMaterial::getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const
{
  delta = 0.0;
  beta = 0.0;
  if ( isReferenceRun ) return;

  for ( unsigned int i=0;i<modules.size();i++ )
  {
    if ( modules[i]->getXrayMatProp( x, y, z, delta, beta) ) return;
  }
}

void OpenSCADMaterial::addModule( const geom::Module &newmodule )
{
  modules.push_back( &newmodule );
}
