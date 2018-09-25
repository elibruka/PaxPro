# Create swig module

INCLUDE_DIR=$1
LIBS=$2
echo "Creating python module..."
source config.sh
export LIBRARY_PATH=${LIBRARY_PATH}:${LIB_PATH}
echo "Library path:"
echo ${LIB}
echo "Include arguments: "
echo ${INC_ARG}
echo "Link arguments: "
echo ${LIB_LINK}
swig -modern -I../include -c++ -python pypaxpro.i
g++ -fPIC -fopenmp -std=c++11 -c pypaxpro_wrap.cxx ${INC_ARG}
g++ -shared pypaxpro_wrap.o ../src/libpaxpro.a -L/home/davidkl/.local/lib $(python-config --ldflags) ${LIB_LINK} -lgomp -o _pypaxpro.so
