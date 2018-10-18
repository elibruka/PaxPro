#include <PaxPro/paxpro.hpp>

// #include "geometry.hpp"
// #include <Paxpro/CSGMaterial>
// #include <PaxPro/genericScattering.hpp>

#include <armadillo>


int main ()
{

geom::Part slab("slab");
geom::Box box(250, 50,5000);
slab.add(box);
slab.delta=1E-4; //-4
slab.beta=1E-6; // -6

slab.translate(0.0, -36.5,0.0);
// slab.rotate(-0.2,geom::Axis_t::X); //0.2
slab.save("slab.scad");

geom::Module slab_mod("slab_module");
slab_mod.add(slab);

CSGMaterial material;
material.addModule(slab_mod);


double xmin = -140;
double xmax = 140;
double ymin = -140; // -60
double ymax = 140; // 190
double zmin = 0;
double zmax = 5000;

GenericScattering simulation("slab");

simulation.setBeamWaist(5); // 20
simulation.setBeamAngleX(0.0);
simulation.setBeamAngleY(0.2);
simulation.setMaxScatteringAngle(1.0);

double dx = (xmax-xmin)/512;
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

simulation.downSampleX = 16; //8
simulation.downSampleY = 16; //8
simulation.downSampleZ = 16; //8

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

simulation.save("slab.h5");


return 0;
}
