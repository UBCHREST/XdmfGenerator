static char help[] = "Command line executable to generate xdmf file hdf5 visualization. \n";

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include "xdmfBuilder.hpp"

int main(int argc, char **args) {
    // Get the file path


//    if (!fileSpecified) {
//        throw std::invalid_argument("the hdf5 file must be specified using the --input flag");
//    }

    std::filesystem::path filePath(filename);
    if (!std::filesystem::exists(filePath)) {
        throw std::invalid_argument("unable to locate input file: " + filePath.string());
    }

    // prepare
    auto root = std::make_shared<petscXdmfGenerator::HdfObject>(filePath);
    auto specification = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(root);
    auto builder = petscXdmfGenerator::XdmfBuilder(specification);
    auto xml = builder.Build();

    // build the path to the output file
    std::filesystem::path outputFile = filePath.parent_path();
    outputFile /= (filePath.stem().string() + ".xmf");

    // write to the file
    std::ofstream xmlFile;
    xmlFile.open(outputFile);
    xml->PrettyPrint(xmlFile);
    xmlFile.close();

    std::cout << "XDMF file written to " << outputFile << std::endl;
}
