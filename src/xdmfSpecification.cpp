#include "xdmfSpecification.hpp"
using namespace petscXdmfGenerator;

void petscXdmfGenerator::XdmfSpecification::GenerateFieldsFromPetsc(std::vector<FieldDescription>& fields, const std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>>& hdfFields,
                                                                    petscXdmfGenerator::FieldLocation location) {
    const static std::map<std::string, FieldType> petscTypeLookUp = {{"scalar",SCALAR}, {"vector",VECTOR}, {"tensor",TENSOR}, {"matrix",MATRIX}};
    for (auto& hdfField : hdfFields) {
        FieldDescription description{
            .name = hdfField->Name(),
            .path = hdfField->Path(),
            .shape = hdfField->Shape(),
            .fieldLocation = location,
            .fieldType = petscTypeLookUp.at(hdfField->AttributeString("vector_field_type"))};
        fields.push_back(description);
    }
}
std::shared_ptr<XdmfSpecification> petscXdmfGenerator::XdmfSpecification::FromPetscHdf(std::shared_ptr<petscXdmfGenerator::HdfObject> rootObject) {
    auto specification = std::make_shared<XdmfSpecification>();

    // store the file name
    specification->hdf5File = rootObject->Name();

    // petsc hdf5 files may have a root domain (this is often a real mesh (FE/FV))
    std::shared_ptr<petscXdmfGenerator::HdfObject> geometryObject = FindPetscHdfChild(rootObject, "geometry");
    if (geometryObject) {
        GridDescription mainGrid;

        // store the geometry
        auto verticesObject = geometryObject->Get("vertices");
        mainGrid.geometry.path = verticesObject->Path();
        mainGrid.geometry.number = verticesObject->Shape()[0];
        mainGrid.geometry.dimension = verticesObject->Shape()[1];

        // check for and get the topology
        std::shared_ptr<petscXdmfGenerator::HdfObject> topologyObject = FindPetscHdfChild(rootObject, "topology");
        if (topologyObject) {
            auto cellObject = topologyObject->Get("cells");
            mainGrid.topology.path = cellObject->Path();
            mainGrid.topology.number = cellObject->Shape()[0];
            mainGrid.topology.numberCorners = cellObject->Shape()[1];
            mainGrid.topology.dimension = cellObject->Attribute<unsigned long long>("cell_dim");
        }

        // hybrid topology
        std::shared_ptr<petscXdmfGenerator::HdfObject> hybridTopologyObject = FindPetscHdfChild(rootObject, "hybrid_topology");
        if (hybridTopologyObject) {
            auto cellObject = topologyObject->Get("hcells");
            mainGrid.hybridTopology.path = cellObject->Path();
            mainGrid.hybridTopology.number = cellObject->Shape()[0];
            mainGrid.hybridTopology.numberCorners = cellObject->Shape()[1];
        }

        // get the time
        mainGrid.time = rootObject->Contains("time") ? rootObject->Get("time")->RawData<double>(): std::vector<double>();

        // get the vertex fields and map into a vertex map
        if (rootObject->Contains("vertex_fields")) {
            GenerateFieldsFromPetsc(mainGrid.fields, rootObject->Get("vertex_fields")->Items(), NODE);
        }
        if (rootObject->Contains("cell_fields")) {
            GenerateFieldsFromPetsc(mainGrid.fields, rootObject->Get("cell_fields")->Items(), CELL);
        }

        // add to the list of grids
        specification->grids.push_back(mainGrid);
    }

    // check for particles
    if(rootObject->Contains("particles") || rootObject->Contains("particle_fields")){
        GridDescription particleGrid;
        particleGrid.name = "particle_domain";

        if(rootObject->Contains("particles")){
            std::shared_ptr<petscXdmfGenerator::HdfObject> geometryObject = FindPetscHdfChild(rootObject, "geometry");
            // store the geometry
            particleGrid.geometry.path = geometryObject->Path();
            particleGrid.geometry.number = geometryObject->Get("vertices")->Shape()[0];
            particleGrid.geometry.dimension = geometryObject->Get("vertices")->Shape()[1];
        }

        // hard code simple topology
        particleGrid.topology.path = "";
        particleGrid.topology.number = particleGrid.geometry.number;
        particleGrid.topology.numberCorners =0;
        particleGrid.topology.dimension = particleGrid.geometry.dimension;

        // get the time
        particleGrid.time = rootObject->Contains("time") ? rootObject->Get("time")->RawData<double>(): std::vector<double>();

        // add to the list of grids
        specification->grids.push_back(particleGrid);

    }


    return specification;
}
std::shared_ptr<petscXdmfGenerator::HdfObject> XdmfSpecification::FindPetscHdfChild(std::shared_ptr<petscXdmfGenerator::HdfObject>& root, std::string name) {
    if (root->Contains("viz") && root->Get("viz")->Contains(name)) {
        return root->Get("viz")->Get(name);
    } else if (root->Contains(name)) {
        return root->Get(name);
    }

    return nullptr;
}
