cmake_minimum_required (VERSION 2.6)

project( ParaxialSimulationLibrary )
set( CMAKE_CXX_STANDARD 11 )
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#add_definitions( -DARMA_NO_DEBUG )
add_definitions( -O3 )
#add_definitions( -DCURRENT_DIR=${CMAKE_CURRENT_SOURCE_DIR})
set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fopenmp")
list(APPEND CMAKE_MODULE_PATH "/usr/share/SFML/cmake/Modules")
list(APPEND CMAKE_MODULE_PATH ${SFML_DIR})
MESSAGE(STATUS "Reading from command line: ${JSON_DIR}")

# Add libraries
include_directories( include )
include_directories(/usr/include/jsoncpp)
include_directories( /usr/local/include )
include_directories( /usr/lib/x86_64-linux-gnu/hdf5/serial/include )

set( INSTALL_INC_DIR /home/elisabbk/.local/include )
set( INSTALL_LIB_DIR /home/elisabbk/.local/lib )
set( INSTALL_EXEC_DIR /home/elisabbk/.local/bin )
set( INSTALL_PYTHON_DIR /home/elisabbk/.local/lib/python/paxpro )

# ========================== FIND ALL LIBRARIES ================================
link_directories( /usr/local/lib )
link_directories( /usr/lib/x86_64-linux-gnu )
set( CMAKE_LIBRARY_PATH /usr/lib/x86_64-linux-gnu )

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
MESSAGE(STATUS "PYTHON: ${PYTHON_INCLUDE_DIRS}")

find_package( LAPACK REQUIRED )
include_directories(${LAPACK_INCLUDE_DIR})
set(LIB ${LAPACK_LIBRARIES})

find_package( BLAS REQUIRED )
include_directories(${BLAS_INCLUDE_DIR})
set(LIB ${LIB} ${BLAS_LIBRARIES})

find_package( ZLIB REQUIRED )
include_directories(${ZLIB_INCLUDE_DIR})
set(LIB ${LIB} ${ZLIB_LIBRARIES})

#set(CMAKE_INCLUDE_PATH "/usr/include")
#find_package( Armadillo REQUIRED )
#include_directories(${ARMADILLO_INCLUDE_DIRS})
set(LIB ${LIB} /home/davidkl/.local/lib/libarmadillo.so)
include_directories(/home/davidkl/.local/include)

find_package( HDF5 REQUIRED COMPONENTS C CXX HL)
include_directories( ${HDF5_INCLUDE_DIRS} )

MESSAGE( STATUS ${HDF5_INCLUDE_DIRS} )
set(LIB ${LIB} ${HDF5_CXX_LIBRARIES})

# Check for the VISA library
find_library( VISA_LIB visa HINTS /usr/local/lib )
if ( NOT VISA_LIB )
  MESSAGE (FATAL_ERROR "Did not find the VISA library")
endif ( NOT VISA_LIB )
set( LIB ${LIB} ${VISA_LIB} )

find_package( SFML REQUIRED system window graphics network audio)
include_directories(${SFML_INCLUDE_DIR})
set(LIB ${LIB} ${SFML_LIBRARIES})
if (NOT SFML_LIBRARIES)
  set(LIB ${LIB} libsfml-system.so libsfml-window.so libsfml-graphics.so libsfml-network.so libsfml-audio.so)
endif()

find_path( VISA_HEADER visa/ )
if ( NOT VISA_HEADER )
  MESSAGE(FATAL_ERROR "Did not find the VISA header files!")
endif( NOT VISA_HEADER )

# Find pthreads library
find_package( OpenMP )
if ( OpenMP_found )
  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
endif()

find_package( OpenGL REQUIRED )

include_directories( ${FLTK_INCLUDE_DIR} )
set( LIB ${LIB} ${FLTK_BASE_LIBRARY} )
set( LIB ${LIB} ${FLTK_GL_LIBRARY} )
set( LIB ${LIB} ${FLTK_FORMS_LIBRARY} )

# Find the FFTW3 library with threads
find_library( FFTW_OMP_LIB fftw3_omp )
if ( NOT FFTW_OMP_LIB )
  MESSAGE( FATAL_ERROR "Could not find the FFTW3 openMP library!")
endif()
set( LIB ${LIB} ${FFTW_OMP_LIB})

# Find the FFTW3 library
find_library( FFTW_LIB fftw3 )
if ( NOT FFTW_LIB )
  MESSAGE( FATAL_ERROR "Could not find the FFTW library")
endif()
find_path( FFTW_INC fftw3.h)
if ( NOT FFTW_INC )
  MESSAGE( FATAL_ERROR "Could not find the FFTW3 headers!")
endif()
set( LIB ${LIB} ${FFTW_LIB} )
set( LIB ${LIB} "m") # Add C-math library
include_directories( ${FFTW_INC} )

find_package( GTest )
set( LIB ${LIB} ${GTEST_LIBRARIES} )
include_directories( ${GTEST_INCLUDE_DIRS} )
MESSAGE( STATUS ${GTEST_LIBRARIES})


set(CMAKE_INCLUDE_PATH "/usr/include/jsoncpp")
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})
set(LIB ${LIB} libjsoncpp.so)
MESSAGE(STATUS "JSONLIBRARY: ${JSONCPP_LIBRARIES}")
if (NOT ${JSONCPP_FOUND})
  MESSAGE(FATAL_ERROR "Could not find JSONCPP!")
endif()

find_package( GSL REQUIRED )
include_directories(${GSL_INCLUDE_DIR})
set(LIB ${LIB} ${GSL_LIBRARIES})

# # TODO: Seem to be something wrong witht the package detection
# find_library( JSONCPP_LIB jsoncpp )
# set( LIB ${LIB} ${JSONCPP_LIB})
# if ( NOT JSONCPP_LIB )
#   MESSAGE( FATAL_ERROR "Did not find the jsoncpp library!")
# endif( NOT JSONCPP_LIB)
# find_path( JSON_HEADER json)
# if ( NOT JSON_HEADER )
#   MESSAGE( FATAL_ERROR "Did not find the json headers!")
# endif( NOT JSON_HEADER )

configure_file( include/config.h.in include/config.h)
configure_file( pyconfig.py.in pyconfig.py )
#=========================== COMPILE LIBRARY ===================================

add_subdirectory(src)
FILE(GLOB HEADERFILES "include/*.hpp" )
FILE( GLOB TPPFILES "include/*.tpp" )
install( FILES ${HEADERFILES} DESTINATION ${INSTALL_INC_DIR}/PaxPro )
install( FILES ${TPPFILES} DESTINATION ${INSTALL_INC_DIR}/PaxPro )
install( FILES "paxpro++" DESTINATION ${INSTALL_EXEC_DIR} )
install( FILES pypaxpro/pypaxpro.py pypaxpro/_pypaxpro.so DESTINATION ${INSTALL_PYTHON_DIR} )

#========================= EXAMPLES ============================================
add_executable( sphere.out EXCLUDE_FROM_ALL Examples/sphere.cpp)
MESSAGE( STATUS ${LIB} )
#target_link_libraries( sphere.out ${LIB} ${CMAKE_CURRENT_SOURCE_DIR}/src/libpaxpro.a )
target_link_libraries( sphere.out ${LIB} paxpro )

add_executable( waveguide2D.out EXCLUDE_FROM_ALL Examples/waveguide2D.cpp)
target_link_libraries( waveguide2D.out ${LIB} paxpro )

add_executable( primitiveGeomExample.out EXCLUDE_FROM_ALL Examples/primitiveGeomExample.cpp )
target_link_libraries( primitiveGeomExample.out ${LIB} paxpro )

add_executable( slab.out EXCLUDE_FROM_ALL Examples/slab.cpp )
target_link_libraries( slab.out ${LIB} paxpro )

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

add_custom_target(prepare_pypaxpro ALL COMMAND python create_config.py ${CMAKE_CURRENT_SOURCE_DIR} ${LIB} ${dirs} DEPENDS paxpro)
add_custom_target( pypaxpro ALL
WORKING_DIRECTORY pypaxpro COMMAND bash createPythonModule.sh ${dirs} ${LIB} DEPENDS prepare_pypaxpro)


if ( GTEST_FOUND )
  include_directories( unittest )
  add_executable( alltests.out unittest/alltests.cpp )
  target_link_libraries( alltests.out ${LIB} paxpro)
endif()
