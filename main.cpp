#include <filesystem>
#include <iostream>
#include <memory>
#include "generators.hpp"

int main(int argc, char **args) {
    // Get the file path
    if (argc < 2) {
        throw std::invalid_argument("the hdf5 file must be specified as the first argument");
    }

    std::filesystem::path filePath(args[1]);
    if (!std::filesystem::exists(filePath)) {
        throw std::invalid_argument("unable to locate input file: " + filePath.string());
    }

    // build the path to the output file
    std::filesystem::path outputFile = filePath.parent_path();
    outputFile /= (filePath.stem().string() + ".xmf");

    // write to the file
    petscXdmfGenerator::Generate(filePath, outputFile);
    std::cout << "XDMF file written to " << outputFile << std::endl;
}
