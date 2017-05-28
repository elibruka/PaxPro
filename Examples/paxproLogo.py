import sys
sys.path.insert(0,"/home/dkleiven/Documents/PaxPro/PythonWrapper")
import pypaxpro as pypax
import numpy as np
import h5py as h5
import matplotlib as mpl
from matplotlib import pyplot as plt

def main():
    runSimulation = True
    fname = "data/paxproLogoScattering.h5"
    delta_Au = 4.9E-5
    delta_SiO2 = 8.9E-6
    beta_Au = 5.1E-6
    beta_SiO2 = 1.2E-7
    if ( runSimulation ):
        thickness = 200.0
        height = 1000.0

        sphere = pypax.ParaxialSimulation("TestSim")
        paxproLogo = pypax.Module("paxLogo")

        # Create the letter P
        letterP = pypax.Part("letterP");
        line1 = pypax.Box(height, thickness, thickness )
        letterP.add( line1 )

        line2 = pypax.Box( thickness, 2*thickness, thickness )
        line2.translate( height/2.0-thickness/2.0, 1.5*thickness , 0.0 )
        letterP.add( line2 )

        line3 = pypax.Box( thickness, 2*thickness, thickness )
        line3.translate( 0.0, 1.5*thickness , 0.0 )
        letterP.add( line3 )

        line4 = pypax.Box( height/2.0, thickness, thickness )
        line4.translate( height/4.0, 2.0*thickness, 0.0 )
        letterP.add( line4 )
        letterP.delta = delta_Au
        letterP.beta = beta_Au

        # Create the letter A
        letterA = pypax.Part( "letterA" )
        line5 = pypax.Box( height, thickness, thickness )
        line5.translate( 0.0, 4*thickness, 0.0 )
        letterA.add( line5 )

        line6 = pypax.Box( thickness, 2*thickness, thickness )
        line6.translate( height/2.0-thickness/2.0, 5.5*thickness, 0.0 )
        letterA.add( line6 )

        line7 = pypax.Box( height, thickness, thickness )
        #line5.rotate( angle, pypax.Axis_t_Z )
        line7.translate( 0.0, 6*thickness, 0.0 )
        letterA.add( line7 )

        line8 = pypax.Box( thickness, 2*thickness, thickness )
        line8.translate( 0.0, 5.5*thickness , 0.0 )
        letterA.add( line8 )
        letterA.delta = delta_SiO2
        letterA.beta = beta_SiO2

        # Create the letter X
        letterX = pypax.Part( "letterX" )
        line9 = pypax.Box( height, thickness, thickness )
        line9.rotate( 40.0, pypax.Axis_t_Z )
        line9.translate( 0.0, 9.0*thickness, 0.0 )
        letterX.add( line9 )

        line10 = pypax.Box( height, thickness, thickness )
        line10.rotate( -40.0, pypax.Axis_t_Z )
        line10.translate( 0.0, 9.0*thickness, 0.0 )
        letterX.add( line10 )
        letterX.delta = delta_Au
        letterX.beta = beta_Au

        # Create letter P again
        letterP2 = pypax.Part(letterP)
        letterP2.translate( 0.0, 12.0*thickness, 0.0 )
        letterP2.delta = delta_SiO2
        letterP2.beta = beta_SiO2

        # Create letter R
        letterR = pypax.Part( "letterR" )
        line15 = pypax.Box(height, thickness, thickness )
        line15.translate( 0.0, 16.0*thickness, 0.0 )
        letterR.add( line15)

        line16 = pypax.Box( thickness, 2*thickness, thickness )
        line16.translate( height/2.0-thickness/2.0, 17.5*thickness , 0.0 )
        letterR.add( line16 )

        line17 = pypax.Box( thickness, 2*thickness, thickness )
        line17.translate( 0.0, 17.5*thickness , 0.0 )
        letterR.add( line17 )

        line18 = pypax.Box( height/2.0, thickness, thickness )
        line18.translate( height/4.0, 18.0*thickness, 0.0 )
        letterR.add( line18 )

        line19 = pypax.Box( height/2.0, thickness, thickness )
        line19.rotate( -40.0, pypax.Axis_t_Z )
        line19.translate( -height/4.0, 17.5*thickness, 0.0 )
        letterR.add( line19 )
        letterR.delta = delta_Au
        letterR.beta = beta_Au

        # Create letter O
        letterO = pypax.Part( "letterO" )
        line20 = pypax.Box(height, thickness, thickness )
        line20.translate( 0.0, 20.0*thickness, 0.0 )
        letterO.add( line20)

        line21 = pypax.Box(height, thickness, thickness )
        line21.translate( 0.0, 22.0*thickness, 0.0 )
        letterO.add( line21)

        line22 = pypax.Box( thickness, 2*thickness, thickness )
        line22.translate( height/2.0-thickness/2.0, 21.5*thickness, 0.0 )
        letterO.add( line22 )

        line23 = pypax.Box( thickness, 2*thickness, thickness )
        line23.translate( -height/2.0+thickness/2.0, 21.5*thickness, 0.0 )
        letterO.add( line23 )
        letterO.delta = delta_SiO2
        letterO.beta = beta_SiO2

        paxproLogo.add( letterP )
        paxproLogo.add( letterA )
        paxproLogo.add( letterX )
        paxproLogo.add( letterP2 )
        paxproLogo.add( letterR )
        paxproLogo.add( letterO )
        paxproLogo.save( "data/paxproLogo.scad" )

        # Simulate the X-ray diffraction pattern from the PyPaxPro-Logo!
        material = pypax.OpenSCADMaterial()
        material.addModule( paxproLogo )

        simulator = pypax.GenericScattering("LogoScat")
        simulator.description = "Scattering pattern from the paxpro Logo"
        simulator.setBeamWaist(400.0*height) # Set very large to mimic a plane wave
        simulator.setMaxScatteringAngle(0.05)

        simulator.xmin = -height
        simulator.xmax = height
        simulator.ymin = -2.0*thickness
        simulator.ymax = 25.0*thickness
        simulator.zmin = -thickness
        simulator.zmax = thickness
        Nx = 512
        Ny = 512
        Nz = 128
        simulator.dx = ( simulator.xmax-simulator.xmin )/Nx
        simulator.dy = ( simulator.ymax - simulator.ymin )/Ny
        simulator.dz = ( simulator.zmax - simulator.zmin )/Nz

        simulator.wavelength = 0.1569
        simulator.FFTPadLength = 32768
        simulator.setMaterial( material )
        simulator.solve()
        simulator.save( fname )

    # Read the file and plot the results
    with h5.File( fname, 'r' ) as hf:
        group = hf.get("data")
        xmin = group.attrs["xmin"]
        xmax = group.attrs["xmax"]
        ymin = group.attrs["ymin"]
        ymax = group.attrs["ymax"]
        ffdset = hf.get( "data/farField" )
        qmin = ffdset.attrs["qmin"]
        qmax = ffdset.attrs["qmax"]
        ff = np.array( ffdset )
        exitPhase = np.array( hf.get("data/exitPhase") )

    fig = plt.figure()
    ax1 = fig.add_subplot(1,2,1)
    ax1.imshow(ff, extent=[qmin,qmax,qmin,qmax], origin="lower", cmap="nipy_spectral", aspect="auto", norm=mpl.colors.LogNorm())
    ax2 = fig.add_subplot(1,2,2)
    ax2.imshow( exitPhase, cmap="inferno", origin="lower", aspect="auto", \
    extent=[ymin,ymax,xmin,xmax] )
    ax1.set_xlabel("$q_y$ (nm$^{-1}$)")
    ax1.set_ylabel("$q_x$ (nm$^{-1}$)")
    ax2.set_ylabel("$x$ (nm)")
    ax2.set_xlabel("$y$ (nm)")
    plt.show()

if __name__ == "__main__":
    main()
