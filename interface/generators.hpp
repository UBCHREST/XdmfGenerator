#ifndef PETSCXDMFGENERATOR_GENERATORS_HPP
#define PETSCXDMFGENERATOR_GENERATORS_HPP
#include <filesystem>

namespace petscXdmfGenerator {
void Generate(std::filesystem::path, std::filesystem::path = {});
void Generate(std::filesystem::path inputFilePath, std::ostream& stream);
}

#endif  // PETSCXDMFGENERATOR_CONVERTERS_HPP
