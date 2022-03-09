#ifndef PETSCXDMFGENERATOR_XDMFSPECIFICATION_H
#define PETSCXDMFGENERATOR_XDMFSPECIFICATION_H

#include <memory>
#include <string>
#include <vector>
#include "hdfObject.hpp"

namespace petscXdmfGenerator {
enum FieldLocation { NODE, CELL };
enum FieldType { SCALAR, VECTOR, TENSOR, MATRIX, NONE };

class XdmfSpecification {
   private:
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
    static void GenerateFieldsFromPetsc(std::vector<FieldDescription>& fields, const std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>>& hdfFields, FieldLocation location,
                                        const std::string& fileName);
    static std::shared_ptr<petscXdmfGenerator::HdfObject> FindPetscHdfChild(std::shared_ptr<petscXdmfGenerator::HdfObject>& root, const std::string& name);
    // Allow the builder to access
    friend class XdmfBuilder;

   public:
    // provide generator functions
    static std::shared_ptr<XdmfSpecification> FromPetscHdf(std::shared_ptr<petscXdmfGenerator::HdfObject>);

    //! multiple file xdmfs
    static std::shared_ptr<XdmfSpecification> FromPetscHdf(std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>>);
};

}  // namespace petscXdmfGenerator
#endif  // PETSCXDMFGENERATOR_XDMFSPECIFICATION_H
