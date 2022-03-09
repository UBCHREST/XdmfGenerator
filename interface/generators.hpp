#ifndef PETSCXDMFGENERATOR_GENERATORS_HPP
#define PETSCXDMFGENERATOR_GENERATORS_HPP
#include <filesystem>
#include <vector>

namespace petscXdmfGenerator {
void Generate(std::filesystem::path, std::filesystem::path = {});
void Generate(std::filesystem::path inputFilePath, std::ostream& stream);
void Generate(std::vector<std::filesystem::path>, std::filesystem::path);
void Generate(std::vector<std::filesystem::path> inputFilePaths, std::ostream& stream);
}  // namespace petscXdmfGenerator

#endif  // PETSCXDMFGENERATOR_CONVERTERS_HPP
