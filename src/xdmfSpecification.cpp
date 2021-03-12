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
    auto builder = std::make_shared<XdmfSpecification>();

    // store the file name
    builder->hdf5File = rootObject->Name();

    // determine the geometry object and path
    std::shared_ptr<petscXdmfGenerator::HdfObject> geometryObject = FindPetscHdfChild(rootObject, "geometry");
    if (geometryObject) {
        builder->geometry.path = geometryObject->Path();
        builder->geometry.number = geometryObject->Get("vertices")->Shape()[0];
        builder->geometry.dimension = geometryObject->Get("vertices")->Shape()[1];
    }

    // topology
    std::shared_ptr<petscXdmfGenerator::HdfObject> topologyObject = FindPetscHdfChild(rootObject, "topology");
    if (topologyObject) {
        builder->topology.path = topologyObject->Path();
        builder->topology.number = topologyObject->Get("cells")->Shape()[0];
        builder->topology.numberCorners = topologyObject->Get("cells")->Shape()[1];
        builder->topology.dimension = topologyObject->Get("cells")->Attribute<unsigned long long>("cell_dim");
    }

    // hybrid topology
    std::shared_ptr<petscXdmfGenerator::HdfObject> hybridTopologyObject = FindPetscHdfChild(rootObject, "hybrid_topology");
    if (hybridTopologyObject) {
        builder->hybridTopology.path = hybridTopologyObject->Path();
        builder->hybridTopology.number = hybridTopologyObject->Get("cells")->Shape()[0];
        builder->hybridTopology.numberCorners = hybridTopologyObject->Get("cells")->Shape()[1];
    }

    // get the time
    builder->time = rootObject->Contains("time") ? rootObject->Get("time")->RawData<double>(): std::vector<double>();

    // get the vertex fields and map into a vertex map
    if (rootObject->Contains("vertex_fields")) {
        GenerateFieldsFromPetsc(builder->fields, rootObject->Get("vertex_fields")->Items(), NODE);
    }
    if (rootObject->Contains("cell_fields")) {
        GenerateFieldsFromPetsc(builder->fields, rootObject->Get("cell_fields")->Items(), CELL);
    }

    // Get the time history
    return builder;
}
std::shared_ptr<petscXdmfGenerator::HdfObject> XdmfSpecification::FindPetscHdfChild(std::shared_ptr<petscXdmfGenerator::HdfObject>& root, std::string name) {
    if (root->Contains("viz") && root->Get("viz")->Contains(name)) {
        return root->Get("viz")->Get(name);
    } else if (root->Contains(name)) {
        return root->Get(name);
    }

    return nullptr;
}
