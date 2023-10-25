#include "generators.hpp"
#include <fstream>
#include <iostream>
#include "xdmfBuilder.hpp"

namespace xdmfGenerator {

void Generate(const std::filesystem::path& inputFilePath, std::ostream& stream) {
    // prepare the builder
    auto hdfObject = std::make_shared<xdmfGenerator::HdfObject>(inputFilePath);
    auto specification = xdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);

    auto builder = xdmfGenerator::XdmfBuilder(specification);
    auto xml = builder.Build();

    // write to the stream
    xml->PrettyPrint(stream);
}

void Generate(const std::filesystem::path& inputFilePath, std::filesystem::path outputFilePath) {
    // build the path to the output file
    if (outputFilePath.empty()) {
        outputFilePath = inputFilePath.parent_path();
        outputFilePath /= (inputFilePath.stem().string() + ".xmf");
    }

    // write to the file
    std::ofstream xmlFile;
    xmlFile.open(outputFilePath);

    // reuse the existing function
    Generate(inputFilePath, xmlFile);

    xmlFile.close();
}

void Generate(const std::vector<std::filesystem::path>& inputFilePaths, std::ostream& stream, const std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)>& monitor) {
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

    auto specification = xdmfGenerator::XdmfSpecification::FromPetscHdf(consumer);

    auto builder = xdmfGenerator::XdmfBuilder(specification);
    auto xml = builder.Build();

    // write to the stream
    xml->PrettyPrint(stream);
}

void Generate(const std::vector<std::filesystem::path>& inputFilePaths, const std::filesystem::path& outputFilePath,
              const std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)>& monitor) {
    // write to the file
    std::ofstream xmlFile;
    xmlFile.open(outputFilePath);

    // reuse the existing function
    Generate(inputFilePaths, xmlFile, monitor);

    xmlFile.close();
}

}  // namespace xdmfGenerator
