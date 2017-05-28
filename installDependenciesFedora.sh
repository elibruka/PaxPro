# Bash script that will install the dependencies available fedora

dnf install cmake
dnf install lapack-devel lapack-static
dnf install openblas openblas-openmp
dnf install gsl gsl-devel
dnf install jsoncpp jsoncpp-devel
dnf install hdf5 hdf5-devel hdf5-static hdf5-openmpi-devel hdf5-openmpi-static
dnf install armadillo
dnf install SFML-devel SFML
dnf install fltk fltk-devel
dnf install fftw-devel fftw-static fftw-openmpi-devel
dnf install gtest gtest-devel
