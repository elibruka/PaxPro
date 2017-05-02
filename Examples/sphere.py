import sys
sys.path.append("/home/dkleiven/Documents/PaxPro/PythonWrapper")
sys.path.append("/usr/local/lib")
import pypaxpro as pypax

def main():
    r = 100.0
    simulator = pypax.GenericScattering("SphereScattering")
    simulator.description = "X-ray scattering from a sphere using the Python interface!"
    simulator.setBeamWaist(400.0*r) # Set very large to mimic a plane wave
    simulator.setMaxScatteringAngle(0.05) # Maximum scattering angle that will be stored

    # Define the material properties belonging to different regions in the msh file
    xrayMatProp = pypax.XrayMatProperty()
    xrayMatProp.delta = 0.0
    xrayMatProp.beta = 0.0

    regionMat = pypax.region()
    xrayMatProp.delta = 4.9E-5
    xrayMatProp.beta = 8.9E-6
    regionMat["Sphere"] = xrayMatProp

    material = pypax.TetraGeometry()
    material.lengthScale = 100.0 # Set the length scale
    material.load( "Geometries/sphere.msh" )
    material.setMatProp( regionMat )

    # If the profile is not needed it is recommended to downscale the 3D matrix stored to reduce
    # the memory requirements
    # NOTE: This does not affect the resolution of the exit fields computed, only the resolution
    # of the 3D matrix stored
    simulator.downSampleX = 64
    simulator.downSampleY = 64
    simulator.downSampleZ = 64

    simulator.wavelength = 0.1569
    simulator.FFTPadLength = 32768

    simulator.setMaterial( material )
    simulator.setNumberOfSteps( 1024, 1024, 256 )
    simulator.useFFTSolver = False # USE ADI

    simulator.solve()

    ctlfile = pypax.ControlFile("data/spherePython")
    simulator.save(ctlfile)


if __name__ == "__main__":
    main()
