# Create swig module

swig -I../include -c++ -python pypaxpro.i
g++ -fPIC -c pypaxpro_wrap.cxx -I/usr/include/python3.5m/ -I../include
g++ -shared pypaxpro_wrap.o ../src/libpaxpro.a -L/usr/local/lib $(python3-config --ldflags) -ljsoncpp -lhdf5_hl_cpp \
 -llapack -lgsl -lgslcblas -lvisa -lsfml-window -lsfml-graphics -lfftw3 -larmadillo -o _pypaxpro.so
