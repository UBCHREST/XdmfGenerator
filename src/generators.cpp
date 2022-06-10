#include <fstream>
#include <iostream>
#include "xdmfBuilder.hpp"

namespace petscXdmfGenerator {

void Generate(std::filesystem::path inputFilePath, std::ostream& stream) {
    // prepare the builder
    auto hdfObject = std::make_shared<petscXdmfGenerator::HdfObject>(inputFilePath);
    auto specifications = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);

    for (const auto& specification : specifications) {
        auto builder = petscXdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(stream);
    }
}

std::vector<std::filesystem::path> Generate(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath) {
    // build the path to the output file
    if (outputFilePath.empty()) {
        outputFilePath = inputFilePath.parent_path();
        outputFilePath /= (inputFilePath.stem().string() + ".xmf");
    }

    // prepare the builder
    auto hdfObject = std::make_shared<petscXdmfGenerator::HdfObject>(inputFilePath);
    auto specifications = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);
    std::vector<std::filesystem::path> outputPaths;

    for (const auto& specification : specifications) {
        auto specificationOutputPath = outputFilePath.parent_path() / (outputFilePath.stem().string() + specification->GetIdentifier() + outputFilePath.extension().string());

        // write to the file
        std::ofstream xmlFile;
        xmlFile.open(specificationOutputPath);

        auto builder = petscXdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(xmlFile);

        xmlFile.close();
        outputPaths.push_back(specificationOutputPath);
    }
    return outputPaths;
}

void Generate(std::vector<std::filesystem::path> inputFilePaths, std::ostream& stream) {
    // prepare the builder
    std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>> hdfObjects;
    for (const auto& inputFile : inputFilePaths) {
        hdfObjects.push_back(std::make_shared<petscXdmfGenerator::HdfObject>(inputFile));
    }

    auto specifications = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObjects);
    for (const auto& specification : specifications) {
        auto builder = petscXdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(stream);
    }
}

std::vector<std::filesystem::path> Generate(std::vector<std::filesystem::path> inputFilePaths, std::filesystem::path outputFilePath) {
    std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>> hdfObjects;
    for (const auto& inputFile : inputFilePaths) {
        hdfObjects.push_back(std::make_shared<petscXdmfGenerator::HdfObject>(inputFile));
    }

    std::vector<std::filesystem::path> outputPaths;

    auto specifications = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObjects);
    for (const auto& specification : specifications) {
        auto specificationOutputPath = outputFilePath.parent_path() / (outputFilePath.stem().string() + specification->GetIdentifier() + outputFilePath.extension().string());

        // write to the file
        std::ofstream xmlFile;
        xmlFile.open(specificationOutputPath);

        auto builder = petscXdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(xmlFile);

        xmlFile.close();
        outputPaths.push_back(specificationOutputPath);
    }
    return outputPaths;
}
}  // namespace petscXdmfGenerator
