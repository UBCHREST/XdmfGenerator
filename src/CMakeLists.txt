
target_sources(petscXdmfGeneratorLibrary
        PRIVATE
        hdfObject.hpp
        hdfObject.cpp
        xmlElement.cpp
        xmlElement.hpp
        xdmfSpecification.hpp
        xdmfSpecification.cpp
        xdmfBuilder.hpp
        xdmfBuilder.cpp
        generators.cpp
        )

target_include_directories(petscXdmfGeneratorLibrary PRIVATE ${CMAKE_CURRENT_LIST_DIR})
