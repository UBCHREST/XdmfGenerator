#ifndef XDMFGENERATOR_GENERATORS_HPP
#define XDMFGENERATOR_GENERATORS_HPP
#include <filesystem>
#include <functional>
#include <vector>

namespace xdmfGenerator {
void Generate(const std::filesystem::path& inputFilePath, std::ostream& stream);
void Generate(const std::vector<std::filesystem::path>&, std::ostream& stream, const std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)>& monitor = {});
}  // namespace xdmfGenerator

#endif  // XDMFGENERATOR_GENERATORS_HPP
