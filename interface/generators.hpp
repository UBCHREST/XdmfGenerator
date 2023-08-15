#ifndef XDMFGENERATOR_GENERATORS_HPP
#define XDMFGENERATOR_GENERATORS_HPP
#include <filesystem>
#include <functional>
#include <vector>

namespace xdmfGenerator {
std::vector<std::filesystem::path> Generate(const std::filesystem::path&, std::filesystem::path = {});
void Generate(const std::filesystem::path& inputFilePath, std::ostream& stream);
std::vector<std::filesystem::path> Generate(const std::vector<std::filesystem::path>&, const std::filesystem::path&,
                                            const std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)>& monitor = {});
void Generate(const std::vector<std::filesystem::path>& inputFilePaths, std::ostream& stream);
}  // namespace xdmfGenerator

#endif  // XDMFGENERATOR_GENERATORS_HPP
