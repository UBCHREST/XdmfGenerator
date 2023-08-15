#include <fstream>
#include <iostream>
#include "xdmfBuilder.hpp"

namespace xdmfGenerator {

void Generate(const std::filesystem::path& inputFilePath, std::ostream& stream) {
    // prepare the builder
    auto hdfObject = std::make_shared<xdmfGenerator::HdfObject>(inputFilePath);
    auto specifications = xdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);

    for (const auto& specification : specifications) {
        auto builder = xdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(stream);
    }
}

std::vector<std::filesystem::path> Generate(const std::filesystem::path& inputFilePath, std::filesystem::path outputFilePath) {
    // build the path to the output file
    if (outputFilePath.empty()) {
        outputFilePath = inputFilePath.parent_path();
        outputFilePath /= (inputFilePath.stem().string() + ".xmf");
    }

    // prepare the builder
    auto hdfObject = std::make_shared<xdmfGenerator::HdfObject>(inputFilePath);
    auto specifications = xdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);
    std::vector<std::filesystem::path> outputPaths;

    for (const auto& specification : specifications) {
        auto specificationOutputPath = outputFilePath.parent_path() / (outputFilePath.stem().string() + specification->GetIdentifier() + outputFilePath.extension().string());

        // write to the file
        std::ofstream xmlFile;
        xmlFile.open(specificationOutputPath);

        auto builder = xdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(xmlFile);

        xmlFile.close();
        outputPaths.push_back(specificationOutputPath);
    }
    return outputPaths;
}

void Generate(const std::vector<std::filesystem::path>& inputFilePaths, std::ostream& stream) {
    // prepare the builder
    std::vector<std::shared_ptr<xdmfGenerator::HdfObject>> hdfObjects;
    hdfObjects.reserve(inputFilePaths.size());
    for (const auto& inputFile : inputFilePaths) {
        hdfObjects.push_back(std::make_shared<xdmfGenerator::HdfObject>(inputFile));
    }

    auto specifications = xdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObjects);
    for (const auto& specification : specifications) {
        auto builder = xdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(stream);
    }
}

std::vector<std::filesystem::path> Generate(const std::vector<std::filesystem::path>& inputFilePaths, const std::filesystem::path& outputFilePath,
                                            const std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)>& monitor) {
    // prevent all hdfObjects from being open at once
    auto inputFileStart = inputFilePaths.begin();
    auto inputFileEnd = inputFilePaths.end();

    auto size = inputFilePaths.size();

    std::function<std::shared_ptr<xdmfGenerator::HdfObject>()> consumer;
    if (monitor) {
        consumer = ([&inputFileStart, &inputFileEnd, monitor, size]() {
            if (inputFileStart != inputFileEnd) {
                monitor(*inputFileStart, size - std::distance(inputFileStart, inputFileEnd) + 1, size);
            }
            return inputFileStart == inputFileEnd ? nullptr : std::make_shared<xdmfGenerator::HdfObject>(*(inputFileStart++));
        });
    } else {
        consumer = ([&inputFileStart, &inputFileEnd]() { return inputFileStart == inputFileEnd ? nullptr : std::make_shared<xdmfGenerator::HdfObject>(*(inputFileStart++)); });
    }

    std::vector<std::filesystem::path> outputPaths;

    auto specifications = xdmfGenerator::XdmfSpecification::FromPetscHdf(consumer);
    for (const auto& specification : specifications) {
        auto specificationOutputPath = outputFilePath.parent_path() / (outputFilePath.stem().string() + specification->GetIdentifier() + outputFilePath.extension().string());

        // write to the file
        std::ofstream xmlFile;
        xmlFile.open(specificationOutputPath);

        auto builder = xdmfGenerator::XdmfBuilder(specification);
        auto xml = builder.Build();

        // write to the stream
        xml->PrettyPrint(xmlFile);

        xmlFile.close();
        outputPaths.push_back(specificationOutputPath);
    }
    return outputPaths;
}
}  // namespace xdmfGenerator
