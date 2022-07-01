#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>
#include "generators.hpp"

void PrintHelp() {
    std::cout << "Xdmf Converter for ABLATE/PETSc" << std::endl;
    std::cout << "Required Input Options: " << std::endl;
    std::cout << "- Single HDF5 file: This option is for a single hdf5 file that contains one or more time steps." << std::endl;
    std::cout << "- Directory: This option is for a multiple hdf5 files inside a directory.  Only hdf5/hdf files will be used." << std::endl;
}

int main(int argc, char** args) {
    // Get the file path
    if (argc < 2) {
        PrintHelp();
        throw std::invalid_argument("the hdf5 file must be specified as the first argument");
    }

    if ((std::string(args[1]) == "--help") || (std::string(args[1]) == "-h")) {
        PrintHelp();
        exit(0);
    }

    // There is a single input
    std::filesystem::path filePath(args[1]);
    if (!std::filesystem::exists(filePath)) {
        throw std::invalid_argument("unable to locate input file/directory: " + filePath.string());
    }

    // check if it is a directory
    if (std::filesystem::is_directory(filePath)) {
        filePath = std::filesystem::canonical(filePath);
        std::vector<std::filesystem::path> inputFilePaths;
        std::filesystem::path outputFile = filePath / (filePath.stem().string() + ".xmf");

        for (const auto& file : std::filesystem::directory_iterator(filePath)) {
            if (file.path().extension() == ".hdf5" || file.path().extension() == ".hdf") {
                inputFilePaths.push_back(file.path());
            }
        }

        // sort the paths
        std::sort(inputFilePaths.begin(), inputFilePaths.end());

        auto paths = xdmfGenerator::Generate(inputFilePaths, outputFile);
        for (const auto& path : paths) {
            std::cout << "XDMF file written to " << path << std::endl;
        }
    } else {
        // build the path to the output file
        std::filesystem::path outputFile = filePath.parent_path();
        outputFile /= (filePath.stem().string() + ".xmf");

        // write to the file
        auto paths = xdmfGenerator::Generate(filePath, outputFile);
        for (const auto& path : paths) {
            std::cout << "XDMF file written to " << path << std::endl;
        }
    }
}
