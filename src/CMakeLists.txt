target_sources(petscXdmfGeneratorLibrary
        PUBLIC
        library.hpp
        library.cpp
        object.hpp
        object.cpp
        )

target_include_directories(petscXdmfGeneratorLibrary PUBLIC ${CMAKE_CURRENT_LIST_DIR})
