import numpy as np
from matplotlib import pyplot as plt
import h5py as h5

def main():
    fname = "data/waveguide2DExample.h5"

    with h5.File(fname, 'r') as hf:
        intensity = np.array( hf.get("data/amplitude") )
        group = hf.get("data")
        xmin = group.attrs["xmin"]
        xmax = group.attrs["xmax"]
        zmin = group.attrs["zmin"]/1000.0
        zmax = group.attrs["zmax"]/1000.0

    fig = plt.figure()
    ax = fig.add_subplot(1,1,1)
    ax.imshow( intensity.T, cmap="inferno", aspect="auto", extent=[zmin,zmax,xmin,xmax])
    ax.set_xlabel("$z$ (um)")
    ax.set_ylabel("$x$ (nm)")
    plt.savefig("Fig/waveguide2DIntensity.png")
    plt.show()

if __name__ == "__main__":
    main()
