# Description
PaXPro (Paraxial X-ray Propagator) is a C++ library for solving the paraxial
wave equation.

# Installation
First install all dependencies.
There are two scripts that should install the dependencies that are available
via *apt-get* (debian systems) or *dnf* (fedora).
[VISA](https://github.com/davidkleiven/VISA) needs to be downloaded and
compiled as this is not available on the package systems.
Hence, on Debian systems run
```bash
  su
  bash installDependenciesDebian.sh
```
and on Fedora run
```bash
  su
  bash installDependenciesFedora.sh
```
Then run
```bash
cmake .
make
su
make install
```

# Dependencies
The library depends on the following libraries:
* [Lapack](http://www.netlib.org/lapack/])
* [BLAS](http://www.netlib.org/blas/])
* [GSL](https://www.gnu.org/software/gsl/])
* [JSONCPP](https://github.com/open-source-parsers/jsoncpp])
* [ZLIB](http://www.zlib.net/])
* [HDF5 Libraries](https://support.hdfgroup.org/HDF5/])
* [Armadillo](http://arma.sourceforge.net/])
* [SFML](https://www.sfml-dev.org/documentation/2.4.2/annotated.php])
* [VISA](https://github.com/davidkleiven/VISA])
* [FFTW3](http://www.fftw.org/])
