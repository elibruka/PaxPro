#include <PaxPro/paxpro.hpp>

// #include "geometry.hpp"
// #include <Paxpro/CSGMaterial>
// #include <PaxPro/genericScattering.hpp>

#include <armadillo>


int main ()
{

geom::Part slabSphere("slabSphere");
double r = 5.0;
geom::Box box(250, 50,5000);
geom::Sphere sphere(r);
slabSphere.add(box);

slabSphere.translate(0.0, -(25+r),0.0); // -36.5
slabSphere.add(sphere);
slabSphere.translate(0.0, -r,0.0);
slabSphere.delta=1E-4; //-4
slabSphere.beta=1E-6; // -6
slabSphere.save("slabSphere.scad");


geom::Module slab_mod("slabSphere_module");
slab_mod.add(slabSphere);

CSGMaterial material;
material.addModule(slab_mod);


double xmin = -140;
double xmax = 140;
double ymin = -140; // -60
double ymax = 140; // 190
double zmin = 0;
double zmax = 5000;

GenericScattering simulation("slabSphere");

simulation.setBeamWaist(5); // 20
simulation.setBeamAngleX(0.0);
simulation.setBeamAngleY(0.23);
simulation.setMaxScatteringAngle(1.0);

double dx = (xmax-xmin)/512; //512
double dy = dx;
double dz = (zmax-zmin)/256; // 256
  
simulation.xmin = xmin;
simulation.xmax = xmax;
simulation.ymin = ymin;
simulation.ymax = ymax;
simulation.zmin = zmin;
simulation.zmax = zmax;
simulation.dx = dx;
simulation.dy = dy;
simulation.dz = dz;
simulation.subtract_reference = false;

simulation.downSampleX = 8; //8
simulation.downSampleY = 8; //8
simulation.downSampleZ = 8; //8

simulation.wavelength = 0.1569;

simulation.realTimeVisualization = true;
simulation.propagator = GenericScattering::SolverType_t::FFT;

simulation.FFTPadLength = 32768;

simulation.setMaterial( material );
post::Intensity intensity;
simulation.addPostProcessing(intensity);
post::RefractiveIndex refractiveIndex;
simulation.addPostProcessing(refractiveIndex);

simulation.solve();

simulation.save("slabSphere.h5");

return 0;
}
