#ifndef XDMFGENERATOR_XDMFSPECIFICATION_H
#define XDMFGENERATOR_XDMFSPECIFICATION_H

#include <functional>
#include <memory>
#include <string>
#include <utility>
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
        hsize_t number = 0;
        hsize_t numberCorners = 0;
        hsize_t dimension = 0;
    };

    struct FieldDescription {
        std::string name;
        DataLocation location;
        std::vector<hsize_t> shape;
        hsize_t timeOffset = 0;
        hsize_t componentOffset = 0;
        hsize_t componentStride = 1;
        hsize_t componentDimension;
        FieldLocation fieldLocation;
        FieldType fieldType;
        bool hasTimeDimension = false;

       public:
        [[nodiscard]] bool HasTimeDimension() const { return hasTimeDimension; }

        [[nodiscard]] hsize_t GetDof() const { return shape.size() > 2 ? shape[1] : shape[0]; }
        [[nodiscard]] hsize_t GetDimension() const { return componentDimension; }
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
        std::map<std::size_t, std::vector<GridDescription>> grids;
    };

    // store the list of grids
    std::vector<GridCollectionDescription> gridsCollections;

    // helper functions
    static void GenerateFieldsFromPetsc(std::vector<FieldDescription>& fields, const std::vector<std::shared_ptr<xdmfGenerator::HdfObject>>& hdfFields, FieldLocation location,
                                        const std::string& fileName, hsize_t timeOffset);
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
    explicit XdmfSpecification(std::string identifier = "") : identifier(std::move(identifier)) {}

    // provide generator functions
    static std::shared_ptr<XdmfSpecification> FromPetscHdf(std::shared_ptr<xdmfGenerator::HdfObject>);

    //! multiple file xdmfs
    static std::shared_ptr<XdmfSpecification> FromPetscHdf(const std::function<std::shared_ptr<xdmfGenerator::HdfObject>()>&);

    [[nodiscard]] const std::string& GetIdentifier() const { return identifier; }
};

}  // namespace xdmfGenerator
#endif  // XDMFGENERATOR_XDMFSPECIFICATION_H
