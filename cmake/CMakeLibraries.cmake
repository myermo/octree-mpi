# ---  L I B R A R I E S  --- #
# --------------------------- #

# Add module directory to the include path.
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/modules")

# Add lib/ folder to the list of folder where CMake looks for packages
set(LIB_FOLDER "${CMAKE_SOURCE_DIR}/lib")
list(APPEND CMAKE_MODULE_PATH ${LIB_FOLDER})

# OpenMP
find_package(OpenMP REQUIRED)
if (OPENMP_CXX_FOUND)
    message(STATUS "OpenMP found and to be linked")
else ()
    message(SEND_ERROR "Could not find OpenMP")
endif ()

# Armadillo
find_package(Armadillo REQUIRED)
if (TARGET armadillo::armadillo)
    message(STATUS "Dependency armadillo::armadillo found")
elseif (${ARMADILLO_FOUND})
    include_directories(${ARMADILLO_INCLUDE_DIR})
    message(STATUS "Armadillo include: " ${ARMADILLO_INCLUDE_DIR})
    message(STATUS "Armadillo libraries: " ${ARMADILLO_LIBRARIES})
else ()
    message(SEND_ERROR "Could not find armadillo::armadillo")
endif ()

# Eigen3
find_package(Eigen3 REQUIRED)
if (TARGET Eigen3::Eigen)
    message(STATUS "Dependency Eigen3::Eigen found")
elseif (${EIGEN3_FOUND})
    include_directories(${EIGEN3_INCLUDE_DIR})
    message(STATUS "Eigen include: ${EIGEN3_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find Eigen3")
endif ()

# LASlib
find_package(LASLIB REQUIRED)
if (${LASLIB_FOUND})
    include_directories(${LASLIB_INCLUDE_DIR} ${LASZIP_INCLUDE_DIR})
    message(STATUS "LASlib include: ${LASLIB_INCLUDE_DIR} ${LASZIP_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find LASLIB")
endif ()

# MPI
find_package(MPI REQUIRED)
if (MPI_CXX_FOUND)
    message(STATUS "MPI found and to be linked")
else ()
    message(SEND_ERROR "Could not find MPI")
endif ()

# TODO: add PETSc, lapacke, and whatever else is needed
find_package(PETSc REQUIRED)
if (TARGET PETSc::PETSc)
    message(STATUS "Dependency PETSC::PETSC found")
elseif (${PETSC_FOUND})
    include_directories(${PETSC_INCLUDE_DIR})
    message(STATUS "PETSc include: " ${PETSC_INCLUDE_DIR})
    message(STATUS "PETSc libraries: " ${PETSC_LIBRARIES})
else ()
    message(SEND_ERROR "Could not find PETSC::PETSC")
endif ()

find_package(LibXml2 REQUIRED)
if (TARGET LibXml2::LibXml2)
    message(STATUS "Dependency LibXml2::LibXml2 found")
elseif (${LIBXML2_FOUND})
    include_directories(${LIBXML2_INCLUDE_DIR})
    message(STATUS "LibXml2 include: ${LIBXML2_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find LibXml2")
endif ()

find_package(LAPACK REQUIRED)
if (TARGET LAPACK::LAPACK)
    message(STATUS "Dependency lapacke::lapacke found")
elseif (${LAPACK_FOUND})
    include_directories(${LAPACK_INCLUDE_DIR})
    message (STATUS "LibLapacke include: ${LAPACK_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find LibLapacke")
endif ()

# set(BITPIT_DIR ${CMAKE_SOURCE_DIR}/lib/bitpit-bitpit-1.9.0/build/cmake/)
# find_library(BITPIT_LIBRARY
#     libbitpit_MPI_D.so
#     HINTS ${CMAKE_SOURCE_DIR}/lib/bitpit-bitpit-1.9.0/build/lib/
# )
# set(BITPIT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/bitpit-bitpit-1.9.0/build/include/)
# find_package(BITPIT REQUIRED)
# include(${BITPIT_USE_FILE})

find_package(BITPIT REQUIRED)
if (TARGET BITPIT::BITPIT)
    message(STATUS "Dependency BITPIT::BITPIT found")
elseif (${BITPIT_FOUND})
    include_directories(${BITPIT_INCLUDE_DIR})
    message(STATUS "bitpit include: ${BITPIT_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find bitpit")
endif ()
