target_sources(xdmfGeneratorLibrary
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

target_include_directories(xdmfGeneratorLibrary PRIVATE ${CMAKE_CURRENT_LIST_DIR})
