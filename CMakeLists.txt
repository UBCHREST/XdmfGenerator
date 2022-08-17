cmake_minimum_required(VERSION 3.18.4)

# Create the new project
project(xdmfGenerator VERSION 0.1.6)

# Set the standards
set(CMAKE_CXX_STANDARD 17)

# check for and download yaml-cpp
find_package(HDF5 REQUIRED)

# make cache variables for install destinations
include(GNUInstallDirs)

# create a library to be used by this exe or other programs
add_library(xdmfGeneratorLibrary)
target_include_directories(xdmfGeneratorLibrary PRIVATE ${HDF5_INCLUDE_DIRS})

# Load in the lib
add_subdirectory(src)
target_include_directories(xdmfGeneratorLibrary
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# FetContent will allow downloading of other libraries
include(FetchContent)

# Build a stand alone executable
add_executable(xdmfGenerator main.cpp)
target_link_libraries(xdmfGenerator PRIVATE xdmfGeneratorLibrary ${HDF5_LIBRARIES})

# Check if we should enable testing options
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
    include(CTest)

    # Include the format command only if we are the main project
    include(clangFormatter.cmake)

    # Setup and configure testing
    if (BUILD_TESTING)
        enable_testing()
        include(GoogleTest)
        add_subdirectory(tests)
    endif ()
endif ()

# load in the build/compiler standards
FetchContent_Declare(
        chrestCompilerFlags
        GIT_REPOSITORY https://github.com/UBCHREST/chrestCompilerFlags.git
        GIT_TAG main
)
FetchContent_MakeAvailable(chrestCompilerFlags)
target_link_libraries(xdmfGenerator PRIVATE chrestCompilerFlags)
target_link_libraries(xdmfGeneratorLibrary PRIVATE chrestCompilerFlags)

# install the xdmfGeneratorLibrary (and others) target and create export-set
install(TARGETS xdmfGeneratorLibrary xdmfGenerator chrestCompilerFlags
        EXPORT xdmfGeneratorLibraryTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

# install header file(s)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/interface/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.hpp")

# generate and install export file
install(EXPORT xdmfGeneratorLibraryTargets
        FILE xdmfGeneratorLibraryTargets.cmake
        NAMESPACE CHREST::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xdmfGeneratorLibrary
        )

# include CMakePackageConfigHelpers macro
include(CMakePackageConfigHelpers)

# generate the version file for the config file
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/xdmfGeneratorLibraryConfigVersion.cmake"
        VERSION "${CMAKE_PROJECT_VERSION}"
        COMPATIBILITY AnyNewerVersion
)

# create config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/xdmfGeneratorLibraryConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xdmfGeneratorLibrary
        )

# install config files
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/xdmfGeneratorLibraryConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/xdmfGeneratorLibraryConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xdmfGeneratorLibrary
        )

# generate the export targets for the build tree
export(EXPORT xdmfGeneratorLibraryTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/xdmfGeneratorLibraryTargets.cmake"
        NAMESPACE CHREST::
        )
