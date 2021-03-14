target_sources(petscXdmfGeneratorLibrary
        PUBLIC
        hdfObject.hpp
        hdfObject.cpp
        xmlElement.cpp
        xmlElement.hpp
        xdmfSpecification.hpp
        xdmfSpecification.cpp
        xdmfBuilder.hpp
        xdmfBuilder.cpp
        generators.hpp
        generators.cpp
        )

target_include_directories(petscXdmfGeneratorLibrary PUBLIC ${CMAKE_CURRENT_LIST_DIR})
