# https://stackoverflow.com/a/57928919
# macro (HEADER_DIRECTORIES return_list)
#     file(GLOB_RECURSE new_list ${CMAKE_SOURCE_DIR}/lib/bitpit/src/*.hpp)    
#     set(dir_list "")
#     foreach (file_path ${new_list})
#         get_filename_component(dir_path ${file_path} PATH)
#         set(dir_list ${dir_list} ${dir_path})
#     endforeach ()
#     list(REMOVE_DUPLICATES dir_list)
#     set(${return_list} ${dir_list})
# endmacro ()

find_path(BITPIT_INCLUDE_DIR
        bitpit
        HINTS ${CMAKE_SOURCE_DIR}/lib/bitpit/build/include/)

# HEADER_DIRECTORIES(BITPIT_INCLUDE_DIR)

message(STATUS "FindBITPITLib: Found bitpit header directory, ${BITPIT_INCLUDE_DIR}.")
if (BITPIT_INCLUDE_DIR)
    find_library(BITPIT_LIBRARY
            libbitpit_MPI_D.so
            HINTS ${CMAKE_SOURCE_DIR}/lib/bitpit/build/lib/
    )
    if (BITPIT_LIBRARY)
        set(BITPIT_FOUND ON)
        set(BITPIT_LIBRARIES ${BITPIT_LIBRARY})
        get_filename_component(BITPIT_LIBRARY_DIRS ${BITPIT_LIBRARY} PATH)
    endif (BITPIT_LIBRARY)
endif (BITPIT_INCLUDE_DIR)

if (BITPIT_FOUND)
    if (NOT BITPIT_FIND_QUIETLY)
        message(STATUS "FindBITPITLib: Found bitpit header directory, ${BITPIT_INCLUDE_DIR}, and library, ${BITPIT_LIBRARIES}.")
    endif (NOT BITPIT_FIND_QUIETLY)
else (BITPIT_FOUND)
    if (BITPIT_FIND_REQUIRED)
        message(FATAL_ERROR "FindBITPITLib: Could not find bitpit header and/or library.")
    endif (BITPIT_FIND_REQUIRED)
endif (BITPIT_FOUND)
