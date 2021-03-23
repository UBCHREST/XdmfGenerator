#include <fstream>
#include "xdmfBuilder.hpp"
#include <iostream>

namespace petscXdmfGenerator {
void Generate(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath) {
    // prepare the builder
    auto hdfObject = std::make_shared<petscXdmfGenerator::HdfObject>(inputFilePath);
    auto specification = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);
    auto builder = petscXdmfGenerator::XdmfBuilder(specification);
    auto xml = builder.Build();

    // build the path to the output file
    if (outputFilePath.empty()) {
        outputFilePath = inputFilePath.parent_path();
        outputFilePath /= (inputFilePath.stem().string() + ".xmf");
    }

    // write to the file
    std::ofstream xmlFile;
    xmlFile.open(outputFilePath);
    xml->PrettyPrint(xmlFile);
    xmlFile.close();
}

void Generate(std::filesystem::path inputFilePath, std::ostream& stream) {
    // prepare the builder
    auto hdfObject = std::make_shared<petscXdmfGenerator::HdfObject>(inputFilePath);
    auto specification = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);
    auto builder = petscXdmfGenerator::XdmfBuilder(specification);
    auto xml = builder.Build();

    // write to the stream
    xml->PrettyPrint(stream);
}
}  // namespace petscXdmfGenerator
