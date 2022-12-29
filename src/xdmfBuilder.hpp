#ifndef XDMFGENERATOR_XDMF_HPP
#define XDMFGENERATOR_XDMF_HPP

#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include "hdfObject.hpp"
#include "xdmfSpecification.hpp"
#include "xmlElement.hpp"

namespace xdmfGenerator {
class XdmfBuilder {
   private:
    const std::shared_ptr<XdmfSpecification> specification;

    // internal helper  write functions
    void WriteCells(xdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription);
    void WriteVertices(XmlElement& element, const XdmfSpecification::FieldDescription& geometryDescription);
    XmlElement& WriteData(xdmfGenerator::XmlElement& element, const xdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription);
    void WriteField(xdmfGenerator::XmlElement& element, xdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription);
    static XmlElement& GenerateTimeGrid(XmlElement& element, const std::vector<double>& time);
    static XmlElement& GenerateHybridSpaceGrid(XmlElement& element, const std::string& domainName);
    XmlElement& GenerateSpaceGrid(XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription, const XdmfSpecification::FieldDescription& geometryDescription,
                                  const std::string& domainName);

    template <typename T>
    inline static std::string toString(T value) {
        std::stringstream asString;
        asString << value;
        return asString.str();
    }

    template <typename T>
    inline static std::string JoinVector(const std::vector<T>& vector, const std::string&& delim = " ") {
        std::string joined = std::accumulate(begin(vector), end(vector), std::string{}, [&delim](std::string r, T p) { return r.empty() ? toString(p) : std::move(r) + delim + toString(p); });
        return joined;
    }

    template <typename... Types>
    inline static std::string Join(Types&&... args) {
        std::stringstream joined;
        ((joined << args << ' '), ...);

        auto joinedString = joined.str();
        if (!joinedString.empty()) {
            joinedString.pop_back();
        }
        return joinedString;
    }

    std::string Hdf5PathToName(std::string hdf5Path);

   public:
    explicit XdmfBuilder(std::shared_ptr<XdmfSpecification> specification);
    std::unique_ptr<XmlElement> Build();
};
}  // namespace xdmfGenerator

#endif  // XDMFGENERATOR_XDMF_HPP
