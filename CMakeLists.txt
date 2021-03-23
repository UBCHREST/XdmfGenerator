cmake_minimum_required(VERSION 3.14)

# Create the new project
project(PetscXdmfGenerator VERSION 0.0.4)

# Set the standards
set(CMAKE_CXX_STANDARD 17)

# Load in the lib
add_library(petscXdmfGeneratorLibrary STATIC)
add_subdirectory(src)

# load in petsc
# Load the Required 3rd Party Libaries
find_package(HDF5 REQUIRED)
message("HDF5_INCLUDE_DIRS" ${HDF5_INCLUDE_DIRS})

# FetContent will allow downloading of other libraries
include(FetchContent)

# Include PETSc and MPI
target_include_directories(petscXdmfGeneratorLibrary PUBLIC ${HDF5_INCLUDE_DIRS})
target_link_libraries(petscXdmfGeneratorLibrary PRIVATE ${HDF5_LIBRARIES})

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