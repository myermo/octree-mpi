# ---  B U I L D I N G  --- #
# ------------------------- #

# Executable
add_executable(${PROJECT_NAME} ${sources})

# Linking libraries
if (TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            OpenMP::OpenMP_CXX)
endif ()

if (TARGET armadillo::armadillo)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            armadillo::armadillo)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${ARMADILLO_LIBRARIES})
endif ()

if (TARGET Eigen3::Eigen)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            Eigen3::Eigen)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${EIGEN_LIBRARIES})
endif ()

if (TARGET MPI::MPI_CXX)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            MPI::MPI_CXX)
endif ()

if (TARGET PETSc::PETSc)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            PETSc::PETSc)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${PETSC_LIBRARIES})
endif ()

if (TARGET LibXml2::LibXml2)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            LibXml2::LibXml2)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${LIBXML2_LIBRARIES})
endif ()

if (TARGET LAPACK::LAPACK)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            LAPACK::LAPACK -llapacke)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${LAPACK_LIBRARIES} -llapacke)
endif ()

if (TARGET BITPIT::BITPIT)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC
            BITPIT::BITPIT)
else ()
    target_link_libraries(${PROJECT_NAME}
            PUBLIC
            ${BITPIT_LIBRARIES})
endif ()

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${LASLIB_LIBRARIES})

# target_link_libraries(${PROJECT_NAME}
#         PUBLIC
#         ${CMAKE_SOURCE_DIR}/lib/bitpit-bitpit-1.9.0/build/lib/libbitpit_MPI_D.so)
