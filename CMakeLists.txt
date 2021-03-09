cmake_minimum_required(VERSION 3.17)

# Set the compilers based upon the PETSc package
find_package(PkgConfig REQUIRED)
pkg_get_variable(PETSC_CXX_COMPILER PETSc cxxcompiler)
set(CMAKE_CXX_COMPILER ${PETSC_CXX_COMPILER})

# Create the new project
project(PetscXdmfGenerator)

# Set the standards
set(CMAKE_CXX_STANDARD 14)
add_compile_definitions(H5_USE_18_API_DEFAULT)

# Load in the lib
add_library(petscXdmfGeneratorLibrary)
add_subdirectory(src)

# load in petsc
# Load the Required 3rd Party Libaries
pkg_check_modules(PETSc REQUIRED PETSc)

# Include PETSc and MPI
target_include_directories(petscXdmfGeneratorLibrary PUBLIC ${PETSc_INCLUDE_DIRS})
target_link_libraries(petscXdmfGeneratorLibrary PUBLIC ${PETSc_LIBRARIES} hdf5_hl hdf5)
target_link_directories (petscXdmfGeneratorLibrary PUBLIC ${PETSc_LIBRARY_DIRS})

# Build a stand alone executable
add_executable(petscXdmfGenerator main.cpp)
target_link_libraries(petscXdmfGenerator PRIVATE petscXdmfGeneratorLibrary)
