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
	a,b=np.shape(ff)
	ax = fig.add_subplot(1,1,1)
	ax.imshow(ff.T,cmap="nipy_spectral",norm=LogNorm())
	ax.plot(np.array([0,a]),np.array([b/2,b/2]),"k--")
	ax.plot(np.array([a/2,a/2]),np.array([0,b]),"k--")
	plt.show()
	
	fig=plt.figure()
	a,b=np.shape(ei)
	ax = fig.add_subplot(1,1,1)
	ax.imshow(ei.T,cmap="nipy_spectral",norm=LogNorm())
	ax.plot([0,a],[b/2,b/2],"k--")
	ax.plot([a/2,a/2],[0,b],"k--")
	plt.show()
	
	fig=plt.figure()
	ax = fig.add_subplot(1,1,1)
	ax.imshow(ep.T,cmap="nipy_spectral")
	plt.show()
	
	

if __name__ == "__main__":
	main()
