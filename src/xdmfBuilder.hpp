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
    void WriteVertices(XmlElement& element, const XdmfSpecification::FieldDescription& geometryDescription, unsigned long long timeStep = TimeInvariant);
    XmlElement& WriteData(petscXdmfGenerator::XmlElement& element, const petscXdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription, unsigned long long timeStep );
    void WriteField(petscXdmfGenerator::XmlElement& element, petscXdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription, unsigned long long timeStep);
    static XmlElement& GenerateTimeGrid(XmlElement& element, const std::vector<double>& time);
    static XmlElement& GenerateHybridSpaceGrid(XmlElement& element);
    XmlElement& GenerateSpaceGrid(XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription , const XdmfSpecification::FieldDescription& geometryDescription, unsigned long long timeStep);

    template <typename T>
    inline static std::string toString(T value) {
        std::stringstream asString;
        asString << value;
        return asString.str();
    }

    template <typename T>
    inline static std::string JoinVector(const std::vector<T>& vector, const std::string&& delim = " ") {
        std::string joined =
            std::accumulate(begin(vector), end(vector), std::string{}, [&delim](std::string r, T p) { return r.empty() ? toString(p) : std::move(r) + delim + toString(p); });
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

    inline void AddReference(const std::string& hdf5Path, const std::string& xmlPath){
        xmlReferences[hdf5Path] = xmlPath + "[@Name=\"" + Hdf5PathToName(hdf5Path)+ "\"]";
    }

    inline bool HasReference(const std::string& hdf5Path){
        return xmlReferences.count(hdf5Path) != 0;
    }


    void UseReference(XmlElement& element, std::string id);

   public:
    explicit XdmfBuilder(std::shared_ptr<XdmfSpecification> specification);
    std::unique_ptr<XmlElement> Build();
};
}  // namespace petscXdmfGenerator

#endif  // PETSCXDMFGENERATOR_XDMF_HPP
