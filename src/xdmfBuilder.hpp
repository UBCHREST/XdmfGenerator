#ifndef PETSCXDMFGENERATOR_XDMF_HPP
#define PETSCXDMFGENERATOR_XDMF_HPP

#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include "hdfObject.hpp"
#include "xmlElement.hpp"
#include "xdmfSpecification.hpp"

namespace petscXdmfGenerator {
class XdmfBuilder {
   private:
    const std::shared_ptr<XdmfSpecification> specification;
    std::map<std::string, std::string> xmlReferences;

    // store constant values
    inline const static unsigned long long TimeInvariant = -1;

    // internal helper  write functions
    void WriteCells(petscXdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription, unsigned long long timeStep = TimeInvariant);
    void WriteVertices(XmlElement& element, const XdmfSpecification::GeometryDescription& geometryDescription, unsigned long long timeStep = TimeInvariant);
    static void WriteField(XmlElement& element, XdmfSpecification::FieldDescription& fieldDescription, unsigned long long timeStep, unsigned long long numSteps, unsigned long long cellDimension,
                           unsigned long long spaceDimension);
    static XmlElement& GenerateTimeGrid(XmlElement& element, const std::vector<double>& time);
    static XmlElement& GenerateHybridSpaceGrid(XmlElement& element);
    XmlElement& GenerateSpaceGrid(XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription , const XdmfSpecification::GeometryDescription& geometryDescription, unsigned long long timeStep);

    template <typename T>
    inline static std::string Join(const std::vector<T>& vector, const std::string&& delim = " ") {
        std::string joined =
            std::accumulate(begin(vector), end(vector), std::string{}, [&delim](std::string r, T p) { return r.empty() ? std::to_string(p) : std::move(r) + delim + std::to_string(p); });
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

    inline void AddReference(std::string id, std::string xmlPath, std::string name){
        xmlReferences[id] = xmlPath + "[@Name=\"" + name+ "\"]";
    }

    inline bool HasReference(std::string id){
        return xmlReferences.count(id) != 0;
    }

    void AddReference(XmlElement& element, std::string id);

   public:
    explicit XdmfBuilder(std::shared_ptr<XdmfSpecification> specification);
    std::unique_ptr<XmlElement> Build();
};
}  // namespace petscXdmfGenerator

#endif  // PETSCXDMFGENERATOR_XDMF_HPP
