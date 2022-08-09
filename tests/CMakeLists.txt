# Download google test
SET(INSTALL_GTEST OFF CACHE BOOL "Don't install gtest" FORCE)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)

FetchContent_MakeAvailable(googletest)

add_executable(tests "")
target_link_libraries(tests PUBLIC gtest gtest_main xdmfGeneratorLibrary ${HDF5_LIBRARIES} chrestCompilerFlags)

target_sources(tests
        PRIVATE
        integrationTests.cpp
        )

gtest_discover_tests(tests
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
        )

# Define a function to allow copying directories.  This function will automatically update files when they are changed
FUNCTION(copy_to_test_directory DIRECTORY)
    FILE(GLOB_RECURSE files ${DIRECTORY}/*)
    FOREACH(absFile ${files})
        file(RELATIVE_PATH relFile ${CMAKE_CURRENT_SOURCE_DIR} ${absFile})
        configure_file(${absFile} ${relFile} COPYONLY)
    ENDFOREACH()
ENDFUNCTION()

copy_to_test_directory(${CMAKE_CURRENT_SOURCE_DIR}/outputs)
copy_to_test_directory(${CMAKE_CURRENT_SOURCE_DIR}/inputs)