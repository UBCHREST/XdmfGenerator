#ifndef PETSCXDMFGENERATOR_GENERATORS_HPP
#define PETSCXDMFGENERATOR_GENERATORS_HPP
#include <filesystem>

namespace petscXdmfGenerator {
void Generate(std::filesystem::path, std::filesystem::path = {});
}

#endif  // PETSCXDMFGENERATOR_CONVERTERS_HPP
