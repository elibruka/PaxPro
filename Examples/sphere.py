import sys
sys.path.append("/home/dkleiven/Documents/PaxPro/PythonWrapper")
import pypaxpro as pypax

class Sphere(pypax.MaterialFunction):
    def __init__(self, radius):
        pypax.MaterialFunction.__init__(self)
        self.r = radius

    def getXrayMatProp( self, x, y, z, delta, beta ):
        '''
        Override the get X-ray material properties function
        '''
        rSq = x**2 + y**2 + z**2
        if ( rSq < self.r**2 ):
            delta = 8.9E-6
            beta = 7E-7
        else:
            delta = 0.0
            beta = 0.0

def main():
    r = 500.0
    xmin = -1.5*r
    xmax = 1.5*r
    zmin = -1.05*r
    zmax = 1.05*r

    dx = (xmax-xmin)/1024
    dz = (zmax-zmin)/1024

    simulator = pypax.GenericScattering("SphereScattering")
    simulator.description = "X-ray scattering from a sphere using the Python interface!"
    simulator.setBeamWaist(400.0*r) # Set very large to mimic a plane wave
    simulator.setMaxScatteringAngle(0.05) # Maximum scattering angle that will be stored

    # Set the simulation domain
    simulator.xmin = xmin
    simulator.xmax = xmax
    simulator.ymin = xmin
    simulator.ymax = xmax
    simulator.zmin = zmin
    simulator.zmax = zmax
    simulator.dx = dx
    simulator.dy = dx
    simulator.dz = dz

    # If the profile is not needed it is recommended to downscale the 3D matrix stored to reduce
    # the memory requirements
    # NOTE: This does not affect the resolution of the exit fields computed, only the resolution
    # of the 3D matrix stored
    simulator.downSampleX = 64
    simulator.downSampleY = 64
    simulator.downSampleZ = 64

    simulator.wavelength = 0.1569
    simulator.FFTPadLength = 32768

    scatterer = Sphere(r)
    simulator.setMaterial( scatterer )

    simulator.solve()

    ctlfile = pypax.ControlFile("data/spherePython")
    simulator.save(ctlfile)

if __name__ == "__main__":
    main()
