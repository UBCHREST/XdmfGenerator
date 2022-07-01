#ifndef PETSCXDMFGENERATOR_GENERATORS_HPP
#define PETSCXDMFGENERATOR_GENERATORS_HPP
#include <filesystem>
#include <vector>

namespace xdmfGenerator {
std::vector<std::filesystem::path> Generate(std::filesystem::path, std::filesystem::path = {});
void Generate(std::filesystem::path inputFilePath, std::ostream& stream);
std::vector<std::filesystem::path> Generate(std::vector<std::filesystem::path>, std::filesystem::path);
void Generate(std::vector<std::filesystem::path> inputFilePaths, std::ostream& stream);
}  // namespace xdmfGenerator

#endif  // PETSCXDMFGENERATOR_CONVERTERS_HPP
