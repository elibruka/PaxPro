import numpy as np
import matplotlib as mpl
from matplotlib import pyplot as plt
import h5py as h5

DATA_FILE = "data/sphere871436.h5"

def formFactorSphere( q, r ):
    res = 3.0*( np.sin(q*r) - q*r*np.cos(q*r) )/(q*r)**3
    res[np.isnan(res)] = 1.0
    return res

def main():
    cmap = "bone"
    with h5.File(DATA_FILE, 'r') as hf:
        basegroup = hf.get("/data")
        farFieldDS = hf.get("/data/farField")
        qmin = farFieldDS.attrs["qmin"]
        qmax = farFieldDS.attrs["qmax"]
        ff = np.array( farFieldDS )
        intensity = np.array( hf.get("/data/exitIntensity") )
        phase = np.array( hf.get("/data/exitPhase") )
        xmin = basegroup.attrs["xmin"]
        xmax = basegroup.attrs["xmax"]
        ymin = basegroup.attrs["ymin"]
        ymax = basegroup.attrs["ymax"]

    fig = plt.figure()

    # Plot intensity
    ax = fig.add_subplot(2,2,1)
    im = ax.imshow( intensity, aspect="auto", extent=[xmin,xmax,ymin,ymax], cmap=cmap )
    ax.set_xlabel("$x$ (nm)")
    ax.set_ylabel("$y$ (nm)" )
    fig.colorbar( im )
    ax.set_title( "Exit Intensity" )

    # Plot phase
    axP = fig.add_subplot(2,2,2)
    im = axP.imshow( phase, aspect="auto", extent=[xmin,xmax,ymin,ymax], cmap=cmap )
    axP.set_title("Exit Phase")
    axP.set_xlabel("$x$ (nm)")
    axP.set_ylabel("$y$ (nm)" )
    fig.colorbar(im)

    # Plot far field
    axF = fig.add_subplot(2,2,3)
    im = axF.imshow( ff, aspect="auto", extent=[qmin,qmax,qmin,qmax], norm=mpl.colors.LogNorm(), cmap=cmap )
    axF.set_xlabel("$q_x$ (nm$^{-1}$)")
    axF.set_ylabel("$q_y$ (nm$^{-1}$)")
    fig.colorbar(im)
    axF.set_title("Far field")

    axFE = fig.add_subplot(2,2,4)
    q = np.linspace( qmin, qmax, ff.shape[1] )
    axFE.plot( q, ff[int(ff.shape[0]/2),:], color="#fc8d62", label="Num", lw=3)
    axFE.set_xlabel( "$q_x$ (nm$^{-1}$)" )
    axFE.set_ylabel("Intensity (a.u.)")

    # Normalize the amplitude
    form = formFactorSphere( q, 500.0 )**2
    normalization = np.sum( ff[ff.shape[0]/2,:] )*len(form)/( np.sum(form)*ff.shape[1] )
    form *= normalization
    axFE.plot(q, form, color="#377eb8", label="$F(q)$")
    axFE.legend( loc="upper right", frameon=False )
    axFE.set_yscale("log")
    axFE.set_title("Comparison with form factor")
    plt.show()

if __name__ == "__main__":
    main()
