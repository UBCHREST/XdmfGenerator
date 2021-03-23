cmake_minimum_required(VERSION 3.14)

# Create the new project
project(PetscXdmf VERSION 0.0.4)

# Set the standards
set(CMAKE_CXX_STANDARD 17)

add_library(petscXdmfGeneratorLibrary)

# Include access to HDF
find_package(HDF5 REQUIRED)
target_include_directories(petscXdmfGeneratorLibrary PRIVATE ${HDF5_INCLUDE_DIRS})
#NOTE: Any use of this library is expected to link in the HDF5_LIBRARIES

# Load in the lib
add_subdirectory(src)
target_include_directories(petscXdmfGeneratorLibrary PUBLIC ${CMAKE_CURRENT_LIST_DIR}/interface)

# FetContent will allow downloading of other libraries
include(FetchContent)

# Build a stand alone executable
add_executable(petscXdmfGenerator main.cpp)
target_link_libraries(petscXdmfGenerator PRIVATE petscXdmfGeneratorLibrary ${HDF5_LIBRARIES})

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