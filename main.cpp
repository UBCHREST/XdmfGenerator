#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>
#include "generators.hpp"

void PrintHelp() {
    std::cout << "Xdmf Converter for ABLATE/PETSc" << std::endl;
    std::cout << "Required Input Options: " << std::endl;
    std::cout << "- Single HDF5 file: This option is for a single hdf5 file that contains one or more time steps." << std::endl;
    std::cout << "- Directory: This option is for a multiple hdf5 files inside a directory.  Only hdf5/hdf files will be used." << std::endl;
    std::cout << "      Optional Arguments:" << std::endl;
    std::cout << "          +n : Starts processing directory hdf5 files at item n" << std::endl;
    std::cout << "          -n : Stop processing directory hdf5 files at total items - n" << std::endl;
    std::cout << "          ~n : Only process every n files" << std::endl;
}

/**
 * helper function to parse  command line arguments
 * @param prefix
 * @param argc
 * @param args
 * @return
 */
static std::optional<int> findNumber(char prefix, int argc, char** args) {
    for (int a = 0; a < argc; ++a) {
        auto arg = std::string(args[a]);
        if (arg.rfind(prefix, 0) == 0) {
            return std::stoi(arg.substr(1));
        }
    }
    return {};
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

        // check for +/- limits
        if (auto plusLimit = findNumber('+', argc, args)) {
            inputFilePaths.erase(inputFilePaths.begin(), inputFilePaths.begin() + plusLimit.value());
        }

        if (auto minusLimit = findNumber('-', argc, args)) {
            int newSize = inputFilePaths.size() - minusLimit.value();
            newSize = std::max(newSize, 0);
            inputFilePaths.resize(newSize);
        }

        if (auto extractNumberOptional = findNumber('~', argc, args)) {
            std::size_t extractNumber = extractNumberOptional.value();
            std::vector<std::filesystem::path> inputFilePathsExtract;

            for (std::size_t i = 0; i < inputFilePaths.size(); i += extractNumber) {
                inputFilePathsExtract.push_back(inputFilePaths[i]);
            }

            inputFilePaths = inputFilePathsExtract;
        }

        // write to the file
        std::ofstream xmlFile;
        xmlFile.open(outputFile);
        xdmfGenerator::Generate(
            inputFilePaths, xmlFile, [](const std::filesystem::path& monitorPath, std::size_t i, std::size_t count) { std::cout << i << "/" << count << ": " << monitorPath.stem() << std::endl; });
        std::cout << "XDMF file written to " << outputFile << std::endl;

        xmlFile.close();

    } else {
        // build the path to the output file
        std::filesystem::path outputFile = filePath.parent_path();
        outputFile /= (filePath.stem().string() + ".xmf");

        // write to the file
        std::ofstream xmlFile;
        xmlFile.open(outputFile);
        xdmfGenerator::Generate(filePath, xmlFile);
        xmlFile.close();

        std::cout << "XDMF file written to " << outputFile << std::endl;
    }
}
