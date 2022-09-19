#ifndef PETSCXDMFGENERATOR_GENERATORS_HPP
#define PETSCXDMFGENERATOR_GENERATORS_HPP
#include <filesystem>
#include <vector>
#include <functional>


namespace xdmfGenerator {
std::vector<std::filesystem::path> Generate(std::filesystem::path, std::filesystem::path = {});
void Generate(std::filesystem::path inputFilePath, std::ostream& stream);
std::vector<std::filesystem::path> Generate(std::vector<std::filesystem::path>, std::filesystem::path, std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)> monitor = {});
void Generate(std::vector<std::filesystem::path> inputFilePaths, std::ostream& stream);
}  // namespace xdmfGenerator

#endif  // PETSCXDMFGENERATOR_CONVERTERS_HPP
