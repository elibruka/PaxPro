#include "materialFunction.hpp"

void OpenSCADMaterial::getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const
{
  delta = deltaSurrounding;
  beta = betaSurrounding;
  if ( isReferenceRun ) return;

  for ( unsigned int i=0;i<pars.size();i++ )
  {
    if ( parts[i]->isInside( x, y, z) )
    {
      delta = parts[i]->delta;
      beta = parts[i]->beta;
      return;
    }
  }
}

void OpenSCADMaterial::addPart( const geom::Part &newpart )
{
  parts.push_back( &newpart );
}
