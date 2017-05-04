

# Compile the library
set( SOURCES absorber.cpp cladding.cpp crancNicholson.cpp
exitFieldSource.cpp farFieldPost.cpp fftSolver2D.cpp fftSolver3D.cpp fixedValuesSource.cpp
gaussianBeam.cpp genericScattering.cpp h5Attribute.cpp linearMap1D.cpp paraxialEquation.cpp
paraxialSimulation.cpp paraxialSource.cpp planeWave.cpp postProcessing.cpp postProcessMod.cpp
refractiveIndex.cpp solver.cpp solver1D.cpp solver2D.cpp solver3D.cpp stdFDsolver.cpp
transmittivity.cpp waveGuideFDSimulation.cpp borderTracker.cpp curvedWaveGuide2D.cpp
alternatingDirectionSolver.cpp )

add_library( paxpro STATIC ${SOURCES} )

# Install the library
install( FILES libpaxpro.a DESTINATION ${INSTALL_LIB_DIR})
