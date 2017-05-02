from distutils.core import setup, Extension

cppModule = Extension("_pypaxpro", include_dirs=["include"],
swig_opts=["-c++","-py3","-I./include"],
sources=["src/absorber.cpp", "src/cladding.cpp", "src/controlFile.cpp", "src/crancNicholson.cpp",
"src/exitFieldSource.cpp", "src/farFieldPost.cpp", "src/fftSolver2D.cpp", "src/fftSolver3D.cpp", "src/fixedValuesSource.cpp",
"src/gaussianBeam.cpp", "src/genericScattering.cpp", "src/h5Attribute.cpp", "src/linearMap1D.cpp", "src/paraxialEquation.cpp",
"src/paraxialSimulation.cpp", "src/paraxialSource.cpp", "src/planeWave.cpp", "src/postProcessing.cpp", "src/postProcessMod.cpp",
"src/refractiveIndex.cpp", "src/solver.cpp", "src/solver1D.cpp", "src/solver2D.cpp", "src/solver3D.cpp", "src/stdFDsolver.cpp",
"src/transmittivity.cpp", "src/waveGuideFDSimulation.cpp", "src/borderTracker.cpp", "src/curvedWaveGuide2D.cpp",
"src/alternatingDirectionSolver.cpp", "src/tetraGeometry.cpp", "src/bvhTree.cpp", "pypaxpro/pypaxpro.i"],
libraries=["armadillo","jsoncpp", "hdf5_cpp","hdf5_hl_cpp", "lapack","gsl","gslcblas","visa","sfml-window","sfml-graphics",
"fftw3_omp"],
extra_compile_args=["-std=c++11","-fopenmp"],
library_dirs=["/usr/local/lib","/usr/lib", "/usr/lib64"]
)

setup(
    name="pypaxpro",
    version="1.0",
    description="PyPaxPro Python interface",
    author="David Kleiven",
    author_email="davidkleiven446@gmail.com",
    packages=["pypaxpro"],
    ext_modules=[cppModule]
    )
