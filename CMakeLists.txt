cmake_minimum_required(VERSION 3.14)

# Set the compilers based upon the PETSc package if we are not a sub-project
if(NOT CMAKE_CXX_COMPILER)
message("Setting custom compilers")
find_package(PkgConfig REQUIRED)
pkg_get_variable(PETSC_CXX_COMPILER PETSc cxxcompiler)
set(CMAKE_CXX_COMPILER ${PETSC_CXX_COMPILER})
endif()

# Create the new project
project(PetscXdmfGenerator VERSION 0.0.1)

# Set the standards
set(CMAKE_CXX_STANDARD 17)

# Load in the lib
add_library(petscXdmfGeneratorLibrary)
#target_compile_definitions(petscXdmfGeneratorLibrary PRIVATE H5_USE_18_API_DEFAULT)
add_subdirectory(src)

# load in petsc
# Load the Required 3rd Party Libaries
pkg_check_modules(PETSc REQUIRED PETSc)

# FetContent will allow downloading of other libraries
include(FetchContent)

# Include PETSc and MPI
target_include_directories(petscXdmfGeneratorLibrary PUBLIC ${PETSc_INCLUDE_DIRS})
target_link_libraries(petscXdmfGeneratorLibrary PUBLIC ${PETSc_LIBRARIES} hdf5_hl hdf5)
target_link_directories (petscXdmfGeneratorLibrary PUBLIC ${PETSc_LIBRARY_DIRS})

# Build a stand alone executable
add_executable(petscXdmfGenerator main.cpp)
target_link_libraries(petscXdmfGenerator PRIVATE petscXdmfGeneratorLibrary)

# Check if we should enable testing options
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
    include(CTest)

    # Include the format command only if we are the main project
    include(clangFormatter.cmake)
endif()

# Setup and configure testing
if(BUILD_TESTING)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()