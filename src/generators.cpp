#include <fstream>
#include <iostream>
#include "xdmfBuilder.hpp"

namespace petscXdmfGenerator {

void Generate(std::filesystem::path inputFilePath, std::ostream& stream) {
    // prepare the builder
    auto hdfObject = std::make_shared<petscXdmfGenerator::HdfObject>(inputFilePath);
    auto specification = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);
    auto builder = petscXdmfGenerator::XdmfBuilder(specification);
    auto xml = builder.Build();

    // write to the stream
    xml->PrettyPrint(stream);
}

void Generate(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath) {
    // build the path to the output file
    if (outputFilePath.empty()) {
        outputFilePath = inputFilePath.parent_path();
        outputFilePath /= (inputFilePath.stem().string() + ".xmf");
    }

    // write to the file
    std::ofstream xmlFile;
    xmlFile.open(outputFilePath);
    Generate(inputFilePath, xmlFile);
    xmlFile.close();
}

void Generate(std::vector<std::filesystem::path> inputFilePaths, std::ostream& stream) {
    // prepare the builder
    std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>> hdfObjects;
    for (const auto& inputFile : inputFilePaths) {
        hdfObjects.push_back(std::make_shared<petscXdmfGenerator::HdfObject>(inputFile));
    }

    auto specification = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObjects);
    auto builder = petscXdmfGenerator::XdmfBuilder(specification);
    auto xml = builder.Build();

    // write to the stream
    xml->PrettyPrint(stream);
}

void Generate(std::vector<std::filesystem::path> inputFilePaths, std::filesystem::path outputFilePath) {
    // write to the file
    std::ofstream xmlFile;
    xmlFile.open(outputFilePath);

    Generate(inputFilePaths, xmlFile);

    xmlFile.close();
}

}  // namespace petscXdmfGenerator
