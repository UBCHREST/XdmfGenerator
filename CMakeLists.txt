cmake_minimum_required(VERSION 3.14)

# Create the new project
project(PetscXdmf VERSION 0.0.8)

# Set the standards
set(CMAKE_CXX_STANDARD 17)

add_library(petscXdmfGeneratorLibrary)

# Include access to HDF
find_package(HDF5 REQUIRED)
target_include_directories(petscXdmfGeneratorLibrary PRIVATE ${HDF5_INCLUDE_DIRS})

# Load in the lib
add_subdirectory(src)
target_include_directories(petscXdmfGeneratorLibrary PUBLIC ${CMAKE_CURRENT_LIST_DIR}/interface)

# FetContent will allow downloading of other libraries
include(FetchContent)

# Build a stand alone executable
add_executable(petscXdmfGenerator main.cpp)
target_link_libraries(petscXdmfGenerator PRIVATE petscXdmfGeneratorLibrary ${HDF5_LIBRARIES})

# Add required warnings for projects
function(default_target_compile_options target)
    target_compile_options(${target} PUBLIC $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Werror -Wall -Wwrite-strings -Wno-strict-aliasing -Wno-unknown-pragmas -Wsign-compare -fstack-protector -fno-stack-check >)
endfunction()

default_target_compile_options(petscXdmfGeneratorLibrary)
default_target_compile_options(petscXdmfGenerator)

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