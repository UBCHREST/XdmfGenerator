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
    struct TopologyDescription {
        std::string path = "";
        unsigned long long number = 0;
        unsigned long long numberCorners = 0;
        unsigned long long dimension = 0;
    };

    struct FieldDescription {
        std::string name = "";
        std::string path = "";
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

    struct GridDescription {
        std::string name = "domain";
        // store each type of geometry/topology data
        TopologyDescription topology;
        TopologyDescription hybridTopology;
        FieldDescription geometry;

        // store the field data
        std::vector<FieldDescription> fields;

        // This is empty for steady state problems
        std::vector<double> time;
    };

    // Store the path to the file
    std::string hdf5File;

    // store the list of grids
    std::vector<GridDescription> grids;

    // helper functions
    static void GenerateFieldsFromPetsc(std::vector<FieldDescription>& fields, const std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>>& hdfFields, FieldLocation location);
    static std::shared_ptr<petscXdmfGenerator::HdfObject> FindPetscHdfChild(std::shared_ptr<petscXdmfGenerator::HdfObject>& root, std::string name);
    // Allow the builder to access
    friend class XdmfBuilder;

   public:
    // provide generator functions
    static std::shared_ptr<XdmfSpecification> FromPetscHdf(std::shared_ptr<petscXdmfGenerator::HdfObject>);
};

}  // namespace petscXdmfGenerator
#endif  // PETSCXDMFGENERATOR_XDMFSPECIFICATION_H
