#ifndef GENERIC_SCATTERING_H
#define GENERIC_SCATTERING_H
#include "paraxialSimulation.hpp"
#include "postProcessMod.hpp"
#include "fftSolver3D.hpp"
#include "gaussianBeam.hpp"
#include "materialFunction.hpp"
#include "alternatingDirectionSolver.hpp"
#include "projectionSolver.hpp"

class GenericScattering: public ParaxialSimulation
{
public:
  enum class SolverType_t {ADI,FFT,PROJ};
  GenericScattering( const char* name );
  virtual ~GenericScattering();

  void setMaterial(  const MaterialFunction &mat ){ material = &mat; };

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

  /** Returns the far field (squared) */
  void getFarField( arma::mat &farf );

  /** Add additional post processing modules. The standard ones are far field, exit field and exit phase */
  void addPostProcessing( post::PostProcessingModule &ppm );

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
  bool subtract_reference{true};
  std::string imgname{""};
  bool realTimeVisualization{false};
  SolverType_t propagator{SolverType_t::FFT};
  bool supressMessages{false};
private:
  const MaterialFunction *material{NULL};
  post::ExitField ef;
  post::ExitIntensity ei;
  post::ExitPhase ep;
  post::FarField ff;
  std::vector<post::PostProcessingModule*> userDefinedPPM;

  GaussianBeam gbeam;
  FFTSolver3D fft3Dsolver;
  ADI adisolver;
  ProjectionSolver projSolver;

  arma::cx_mat *reference{NULL};
  bool isReferenceRun{true};

  bool isFirstTime{true};

  /** Has to be called before simulation is solved */
  void init();
};
#endif
