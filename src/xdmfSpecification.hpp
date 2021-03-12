#ifndef PETSCXDMFGENERATOR_XDMFSPECIFICATION_H
#define PETSCXDMFGENERATOR_XDMFSPECIFICATION_H

#include<string>
#include<vector>
#include<memory>
#include "hdfObject.hpp"

namespace petscXdmfGenerator {
enum FieldLocation {NODE, CELL};
enum FieldType {SCALAR, VECTOR, TENSOR, MATRIX};

class XdmfSpecification {
   private:
    struct TopologyDescription {
        std::string path = "";
        unsigned long long number = 0;
        unsigned long long numberCorners = 0;
        unsigned long long dimension = 0;
    };

    struct GeometryDescription {
        std::string path = "";
        unsigned long long number = 0;
        unsigned long long dimension = 0;
    };

    struct FieldDescription {
        std::string name = "";
        std::string path = "";
        std::vector<unsigned long long> shape;
        FieldLocation fieldLocation;
        FieldType fieldType;
    };

    struct GridDescription{
        std::string name = "domain";
        // store each type of geometry/topology data
        TopologyDescription topology;
        TopologyDescription hybridTopology;
        GeometryDescription geometry;

        // store the field data
        std::vector<FieldDescription> fields;

        // This is empty for steady state problems
        std::vector<double> time;

        // determine if the gridTimeInvariant
        bool gridTimeInvariant = true;
    };

    // Store the path to the file
    std::string hdf5File;

    // store the list of grids
    std::vector<GridDescription> grids;

    // helper functions
    static void GenerateFieldsFromPetsc(std::vector<FieldDescription>& fields, const std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>>& hdfFields,FieldLocation location);
    static std::shared_ptr<petscXdmfGenerator::HdfObject> FindPetscHdfChild(std::shared_ptr<petscXdmfGenerator::HdfObject>& root, std::string name);
    // Allow the builder to access
    friend class XdmfBuilder;

   public:
    // provide generator functions
    static std::shared_ptr<XdmfSpecification> FromPetscHdf(std::shared_ptr<petscXdmfGenerator::HdfObject>);


};

}
#endif  // PETSCXDMFGENERATOR_XDMFSPECIFICATION_H
