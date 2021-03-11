#include "generators.hpp"
#include <fstream>
#include "xdmfBuilder.hpp"

namespace petscXdmfGenerator {
void Generate(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath) {
    // prepare the builder
    auto hdfObject = std::make_shared<petscXdmfGenerator::HdfObject>(inputFilePath);
    auto builder = petscXdmfGenerator::XdmfBuilder::FromPetscHdf(hdfObject);
    auto xml = builder->Build();

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
}  // namespace petscXdmfGenerator
