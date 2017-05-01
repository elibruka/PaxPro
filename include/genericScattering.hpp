#ifndef GENERIC_SCATTERING_H
#define GENERIC_SCATTERING_H
#include "paraxialSimulation.hpp"
#include "postProcessMod.hpp"
#include "fftSolver3D.hpp"
#include "gaussianBeam.hpp"
#include "materialFunction.hpp"
#include "alternatingDirectionSolver.hpp"

class TetraGeometry;
class GenericScattering: public ParaxialSimulation
{
public:
  GenericScattering( const char* name );
  virtual ~GenericScattering();

  /** Set function that specifies the material */
  void setMaterial( MaterialFunction &mat ){ material = &mat; };

  /** Set material function based on a tetrahedron mesh */
  void setMaterial( TetraGeometry &mat );

  /** Set the waist of the Gaussian beam */
  void setBeamWaist( double waist ){ gbeam.setWaist(waist); };

  /** Sets the maximum scattering angle that will be stored */
  void setMaxScatteringAngle( double angMax );

  /** Solve scattering */
  virtual void solve() override;

  /** Gets the X-ray material properties */
  virtual void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const override;

  /** Prints all the attributes */
  void printInfo() const;

  /** Set the number of discretization points */
  void setNumberOfSteps( unsigned int Nx, unsigned int Ny, unsigned int Nz );

  // Attributes
  unsigned int exportNx{512};
  unsigned int exportNy{512};
  unsigned int FFTPadLength{1024};
  double xmin{-1.0};
  double xmax{1.0};
  double ymin{-1.0};
  double ymax{1.0};
  double zmin{-1.0};
  double zmax{1.0};
  double dx{0.1};
  double dy{0.1};
  double dz{0.1};
  unsigned int downSampleX{1};
  unsigned int downSampleY{1};
  unsigned int downSampleZ{1};
  double wavelength{0.1569};
  double intensityMin{0.0};
  double intensityMax{1.0};
  double phaseMin{-3.14159};
  double phaseMax{3.14159};
  std::string imgname{""};
  bool realTimeVisualization{false};
  bool useFFTSolver{true};
private:
  MaterialFunction *material{nullptr};
  post::ExitField ef;
  post::ExitIntensity ei;
  post::ExitPhase ep;
  post::FarField ff;

  GaussianBeam gbeam;
  FFTSolver3D fft3Dsolver;
  ADI adisolver;

  arma::cx_mat *reference{NULL};
  bool isReferenceRun{true};

  /** Has to be called before simulation is solved */
  void init();
};
#endif
