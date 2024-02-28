# This module defines the following uncached variables:
#  PETSC_FOUND
#  PETSC_INCLUDE_DIR
#  PETSC_LIBRARIES
#  PETSC_LIBRARY_DIRS

 # -- FindPETScLib: Found PETSc header directory, /home/frd/Documents/4o/TFG/octree-mpi/lib/petsc/include, and library, /usr/lib/x86_64-linux-gnu/libpetsc.so.
 # -- PETSc include: /home/frd/Documents/4o/TFG/octree-mpi/lib/petsc/include
 # -- PETSc libraries: /usr/lib/x86_64-linux-gnu/libpetsc.so

find_path(PETSC_INCLUDE_DIR
        petsc
        HINTS ${CMAKE_SOURCE_DIR}/lib/petsc/include/)

if (PETSC_INCLUDE_DIR)
    find_library(PETSC_LIBRARY
            libpetsc.so
            HINTS ${CMAKE_SOURCE_DIR}/lib/petsc/arch-linux-c-debug/lib/
    )
    if (PETSC_LIBRARY)
        set(PETSC_FOUND ON)
        set(PETSC_LIBRARIES ${PETSC_LIBRARY})
        get_filename_component(PETSC_LIBRARY_DIRS ${PETSC_LIBRARY} PATH)
    endif (PETSC_LIBRARY)
endif (PETSC_INCLUDE_DIR)

if (PETSC_FOUND)
    if (NOT PETSC_FIND_QUIETLY)
        message(STATUS "FindPETScLib: Found PETSc header directory, ${PETSC_INCLUDE_DIR}, and library, ${PETSC_LIBRARIES}.")
    endif (NOT PETSC_FIND_QUIETLY)
else (PETSC_FOUND)
    if (PETSC_FIND_REQUIRED)
        message(FATAL_ERROR "FindPETScLib: Could not find PETSc header and/or library.")
    endif (PETSC_FIND_REQUIRED)
endif (PETSC_FOUND)
