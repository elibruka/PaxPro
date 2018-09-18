import h5py as h5
from matplotlib import pyplot as plt
from matplotlib.colors import LogNorm
import numpy as np

def main():
	with h5.File("slab.h5","r") as infile:
		ff = np.array(infile["data/farField"])
		ei = np.array(infile["data/exitIntensity"])
		ep = np.array(infile["data/exitPhase"])
	fig=plt.figure()
	ax = fig.add_subplot(1,1,1)
	ax.imshow(ff.T,cmap="nipy_spectral",norm=LogNorm())
	plt.show()
	
	fig=plt.figure()
	ax = fig.add_subplot(1,1,1)
	ax.imshow(ei.T,cmap="nipy_spectral",norm=LogNorm())
	plt.show()
	
	fig=plt.figure()
	ax = fig.add_subplot(1,1,1)
	ax.imshow(ep.T,cmap="nipy_spectral")
	plt.show()

if __name__ == "__main__":
	main()
