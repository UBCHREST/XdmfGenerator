#ifndef PETSCXDMFGENERATOR_XDMF_HPP
#define PETSCXDMFGENERATOR_XDMF_HPP

#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include "hdfObject.hpp"
#include "xmlElement.hpp"

namespace petscXdmfGenerator {
class XdmfBuilder {
   private:
    struct Description {
        std::string path = "";
        unsigned long long number = 0;
        unsigned long long numberCorners = 0;
        unsigned long long dimension = 0;
    };

    struct FieldDescription {
        std::string name = "";
        std::string path = "";
        std::vector<unsigned long long> shape;
        std::string vectorFieldType = "";
    };

    // store each type of geometry/topology data
    Description topology;
    Description hybridTopology;
    Description geometry;

    std::string hdf5File;
    std::vector<double> time;

    // store the field data
    std::vector<FieldDescription> vertexFields;
    std::vector<FieldDescription> cellFields;

    // internal helper functions
    static std::shared_ptr<petscXdmfGenerator::HdfObject> FindHdfChild(std::shared_ptr<petscXdmfGenerator::HdfObject>& root, std::string name);

    static void WriteCells(XmlElement& element, std::string path, unsigned long long numberCells, unsigned long long numberCorners, std::string cellName = "cells");
    static void WriteVertices(XmlElement& element, std::string path, unsigned long long number, unsigned long long dimensions);
    static void WriteField(XmlElement& element, FieldDescription& fieldDescription, unsigned long long timeStep, unsigned long long numSteps, unsigned long long cellDimension,
                           unsigned long long spaceDimension, std::string domain);
    static XmlElement& GenerateTimeGrid(XmlElement& element, const std::vector<double>& time);
    static XmlElement& GenerateHybridSpaceGrid(XmlElement& element);
    static XmlElement& GenerateSpaceGrid(XmlElement& element, unsigned long long numberCells, unsigned long long numberCorners, unsigned long long cellDimension, unsigned long long spaceDimension,
                                         std::string cellName = "cells");

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

   public:
    // provide generator functions
    static std::shared_ptr<XdmfBuilder> FromPetscHdf(std::shared_ptr<petscXdmfGenerator::HdfObject>);

    std::unique_ptr<XmlElement> Build();
};
}  // namespace petscXdmfGenerator

#endif  // PETSCXDMFGENERATOR_XDMF_HPP
