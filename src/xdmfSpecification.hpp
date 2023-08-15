#ifndef XDMFGENERATOR_XDMFSPECIFICATION_H
#define XDMFGENERATOR_XDMFSPECIFICATION_H

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "hdfObject.hpp"

namespace xdmfGenerator {
enum FieldLocation { NODE, CELL };
enum FieldType { SCALAR, VECTOR, TENSOR, MATRIX, NONE };

class XdmfSpecification {
   private:
    std::string identifier;

    struct DataLocation {
        std::string path;
        std::string file;
    };

    struct TopologyDescription {
        DataLocation location;
        unsigned long long number = 0;
        unsigned long long numberCorners = 0;
        unsigned long long dimension = 0;
    };

    struct FieldDescription {
        std::string name;
        DataLocation location;
        std::vector<unsigned long long> shape;
        unsigned long long timeOffset = 0;
        unsigned long long componentOffset = 0;
        unsigned long long componentStride = 1;
        unsigned long long componentDimension;
        FieldLocation fieldLocation;
        FieldType fieldType;

       public:
        bool HasTimeDimension() const { return shape.size() > 2; }

        unsigned long long GetDof() const { return shape.size() > 2 ? shape[1] : shape[0]; }
        unsigned long long GetDimension() const { return componentDimension; }
    };

    /**
     * Grid at a set point in time
     */
    struct GridDescription {
        // store each type of geometry/topology data
        TopologyDescription topology;
        TopologyDescription hybridTopology;
        FieldDescription geometry;

        // store the field data
        std::vector<FieldDescription> fields;

        // store the time at this grid
        double time = -1;
    };

    /**
     * Collection of time varying grids
     */
    struct GridCollectionDescription {
        std::string name = "domain";

        // grids in time order
        std::vector<GridDescription> grids;
    };

    // store the list of grids
    std::vector<GridCollectionDescription> gridsCollections;

    // helper functions
    static void GenerateFieldsFromPetsc(std::vector<FieldDescription>& fields, const std::vector<std::shared_ptr<xdmfGenerator::HdfObject>>& hdfFields, FieldLocation location,
                                        const std::string& fileName, unsigned long long timeOffset);
    static std::shared_ptr<xdmfGenerator::HdfObject> FindPetscHdfChild(std::shared_ptr<xdmfGenerator::HdfObject>& root, const std::string& name);

    static std::string GetTopologyPostfix(int index) {
        if (index > 0) {
            return "_" + std::to_string(index);
        } else {
            return "";
        }
    }

    // Allow the builder to access
    friend class XdmfBuilder;

   public:
    XdmfSpecification(const std::string& identifier = "") : identifier(identifier) {}

    // provide generator functions
    static std::vector<std::shared_ptr<XdmfSpecification>> FromPetscHdf(std::shared_ptr<xdmfGenerator::HdfObject>);

    //! multiple file xdmfs
    static std::vector<std::shared_ptr<XdmfSpecification>> FromPetscHdf(const std::function<std::shared_ptr<xdmfGenerator::HdfObject>()>&);
    static std::vector<std::shared_ptr<XdmfSpecification>> FromPetscHdf(std::vector<std::shared_ptr<xdmfGenerator::HdfObject>>);

    const std::string& GetIdentifier() const { return identifier; }
};

}  // namespace xdmfGenerator
#endif  // XDMFGENERATOR_XDMFSPECIFICATION_H
