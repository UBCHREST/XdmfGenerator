#include "xdmfSpecification.hpp"
#include <stdexcept>
#include <algorithm>

using namespace petscXdmfGenerator;


const static std::map<std::string, FieldType> petscTypeLookUpFromFieldType = {{"scalar",SCALAR}, {"vector",VECTOR}, {"tensor",TENSOR}, {"matrix",MATRIX}};
const static std::map<int, FieldType> petscTypeLookUpFromNC = {{1 ,SCALAR}, {2,VECTOR}};

void petscXdmfGenerator::XdmfSpecification::GenerateFieldsFromPetsc(std::vector<FieldDescription>& fields, const std::vector<std::shared_ptr<petscXdmfGenerator::HdfObject>>& hdfFields,
                                                                    petscXdmfGenerator::FieldLocation location) {
    for (auto& hdfField : hdfFields) {
        FieldDescription description { .name = hdfField->Name(), .path = hdfField->Path(), .shape = hdfField->Shape(), .fieldLocation = location };

        if(hdfField->HasAttribute("vector_field_type")) {
            description.fieldType = petscTypeLookUpFromFieldType.at(hdfField->AttributeString("vector_field_type"));
        }else if(hdfField->HasAttribute("Nc")){
            description.fieldType = petscTypeLookUpFromNC.at(hdfField->Attribute<int>("Nc"));
        }else{
            throw std::runtime_error("Cannot determine field type for " + description.name);
        }

        // the 1 dimension is left off for scalars, add it back to the shape
        if(description.fieldType == SCALAR){
            description.shape.push_back(1);
        }

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
        mainGrid.geometry.name = verticesObject->Name(),
        mainGrid.geometry.path = verticesObject->Path(),
        mainGrid.geometry.shape = verticesObject->Shape(),
        mainGrid.geometry.fieldLocation = NODE,
        mainGrid.geometry.fieldType = VECTOR;

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

        // add in any other fields
        if (rootObject->Contains("particle_fields")) {
            GenerateFieldsFromPetsc(particleGrid.fields, rootObject->Get("particle_fields")->Items(), NODE);
        }

        if(rootObject->Contains("particles")){
            std::shared_ptr<petscXdmfGenerator::HdfObject> geometryObject = rootObject->Get("particles")->Get("coordinates");
            // store the geometry
            particleGrid.geometry.name = geometryObject->Name(),
            particleGrid.geometry.path = geometryObject->Path(),
            particleGrid.geometry.shape = geometryObject->Shape(),
            particleGrid.geometry.fieldLocation = NODE,
            particleGrid.geometry.fieldType = VECTOR;
        }else{
            // grad the geometry from the particle_fields
            auto gridField = std::find_if(particleGrid.fields.begin(), particleGrid.fields.end(), [] (const auto& f) { return f.name =="DMSwarmPIC_coor"; });
            if(gridField != particleGrid.fields.end()){
                particleGrid.geometry =*gridField;
                particleGrid.fields.erase(gridField);
            }else{
                throw std::runtime_error("Cannot determine geometry for particles");
            }

        }

        // hard code simple topology
        particleGrid.topology.path = "";
        particleGrid.topology.number = particleGrid.geometry.GetDof();
        particleGrid.topology.numberCorners =0;
        particleGrid.topology.dimension = particleGrid.geometry.GetDimension();

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
