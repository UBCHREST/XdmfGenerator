#ifndef XDMFGENERATOR_GENERATORS_HPP
#define XDMFGENERATOR_GENERATORS_HPP
#include <filesystem>
#include <functional>
#include <vector>

namespace xdmfGenerator {
/**
 * Generate an xdmf file contents for a single hdf5 file.
 * @param inputFilePath the path to the hdf5 file
 * @param stream the stream to which the xdfm xml
 */
void Generate(const std::filesystem::path& inputFilePath, std::ostream& stream);

/**
 * Generate an xdmf file for a single hdf5 file.
 * @param inputFilePath the path to the hdf5 file
 * @param outputFilePath optional path to the xdmf file.  If not specified a default location is used.
 */
void Generate(const std::filesystem::path& inputFilePath, std::filesystem::path outputFilePath = {});

/**
 * Parse and create a time history xdmf file for multiple hdf5 files
 * @param paths vector of paths to the hdf5 files
 * @param stream the stream to which the xdfm xml
 * @param monitor optional monitor that reports the current file being parsed.  Useful to monitor large jobs
 */
void Generate(const std::vector<std::filesystem::path>& paths, std::ostream& stream, const std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)>& monitor = {});

/**
 * Parse and create a time history xdmf file for multiple hdf5 files.
 * @param paths vector of paths to the hdf5 file
 * @param outputFilePath path to the xdmf file
 * @param monitor optional monitor that reports the current file being parsed.  Useful to monitor large jobs
 */
void Generate(const std::vector<std::filesystem::path>& paths, const std::filesystem::path& outputFilePath,
              const std::function<void(const std::filesystem::path& path, std::size_t i, std::size_t count)>& monitor = {});

}  // namespace xdmfGenerator

#endif  // XDMFGENERATOR_GENERATORS_HPP
