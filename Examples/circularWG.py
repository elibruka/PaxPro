import sys
sys.path.append("../")
sys.path.append("./")
import pyconfig
sys.path.insert(0,pyconfig.pythonpath)
import pypaxpro as pypax
import numpy as np

def main():
    length = 1E5 # Length of the waveguide in nm
    radius = 100.0  # Radius of the waveguide in nm
    Lx = 4.0*radius
    Ly = 4.0*radius

    outerbox = pypax.Box( Lx, Ly, length )

    # Make the inner cylinder a bit longer and translate it a little
    # This makes the difference better
    innerCyl = pypax.Cylinder( radius, length+2.0 )
    innerCyl.translate(0.0,0.0,-0.5)
    wg = pypax.Part("Waveguide")
    wg.add( outerbox )
    wg.difference( innerCyl )
    wg.delta = 4.14E-5
    wg.beta = 8.9E-6
    wgMod = pypax.Module("WaveguideModule")
    wgMod.add( wg )
    wgMod.translate( 0.0, 0.0, length/2.0 )
    wgMod.save( "data/circularWaveguide.scad" )

    # Setup the simulation
    material = pypax.CSGMaterial()
    material.addModule( wgMod )

    simulator = pypax.ParaxialSimulation( "CircularWaveguide" )
    simulator.description = "Simulation of a 3D circular straight waveguide"
    xmin = -Lx/2.0
    xmax = Lx/2.0
    ymin = -Ly/2.0
    ymax = Ly/2.0
    zmin = 0.0
    zmax = length
    Nx = Ny = 1024
    Nz = 2048
    dx = (xmax-xmin)/Nx
    dy = (ymax-ymin)/Ny
    dz = (zmax-zmin)/Nz
    simulator.setTransverseDiscretization( xmin, xmax, dx, 2 )
    simulator.setLongitudinalDiscretization( zmin, zmax, dz, 16 )
    simulator.setVerticalDiscretization( ymin, ymax, dy )

    source = pypax.GaussianBeam()
    source.setWaist( 2.0*radius )
    source.setWavelength( 0.1569 )

    # Tell PaXPro to use the 3D version of a Gaussian source (2D is default)
    source.setDim( source.Dim_t_THREE_D )

    # Define post processing modules
    intensity = pypax.Intensity()
    exitPhase = pypax.ExitPhase()
    exitAmp = pypax.ExitIntensity()

    solver = pypax.ADI()
    solver.realTimeVisualization()
    solver.setPlotLimits( 1E-4, 2.0, -np.pi, np.pi, True )
    simulator.setSolver( solver )
    simulator.material = material
    simulator.setBoundaryConditions( source )

    simulator.addPostProcessingModule( intensity )
    simulator.addPostProcessingModule( exitPhase )
    simulator.addPostProcessingModule( exitAmp )
    simulator.solve()
    simulator.save( "data/circularWG.h5" )

if __name__ == "__main__":
    main()
