
#include "xdmfBuilder.hpp"
#include <map>
#include <set>
#include <string>

static const auto DataItem = "DataItem";
static const auto Grid = "Grid";

static std::map<unsigned long long, std::map<unsigned long long, std::string>> cellMap = {
    {1, {{1, "Polyvertex"}, {2, "Polyline"}}}, {2, {{3, "Triangle"}, {4, "Quadrilateral"}}}, {3, {{4, "Tetrahedron"}, {6, "Wedge"}, {8, "Hexahedron"}}}};

static std::map<std::string, std::string> typeMap = {{"scalar", "Scalar"}, {"vector", "Vector"}, {"tensor", "Tensor6"}, {"matrix", "Matrix"}};

static std::map<unsigned long long, std::map<std::string, std::vector<std::string>>> typeExt = {{2, {{"vector", {"x", "y"}}, {"tensor", {"xx", "yy", "xy"}}}},
                                                                                                {3, {{"vector", {"x", "y", "z"}}, {"tensor", {"xx", "yy", "zz", "xy", "yz", "xz"}}}}};

std::shared_ptr<petscXdmfGenerator::XdmfBuilder> petscXdmfGenerator::XdmfBuilder::FromPetscHdf(std::shared_ptr<petscXdmfGenerator::HdfObject> rootObject) {
    auto builder = std::make_shared<XdmfBuilder>();

    // store the file name
    builder->hdf5File = rootObject->Name();

    // determine the geometry object and path
    std::shared_ptr<petscXdmfGenerator::HdfObject> geometryObject = FindHdfChild(rootObject, "geometry");
    if (geometryObject) {
        builder->geometry.path = geometryObject->Path();
        builder->geometry.number = geometryObject->Get("vertices")->Shape()[0];
        builder->geometry.dimension = geometryObject->Get("vertices")->Shape()[1];
    }

    // topology
    std::shared_ptr<petscXdmfGenerator::HdfObject> topologyObject = FindHdfChild(rootObject, "topology");
    if (topologyObject) {
        builder->topology.path = topologyObject->Path();
        builder->topology.number = topologyObject->Get("cells")->Shape()[0];
        builder->topology.numberCorners = topologyObject->Get("cells")->Shape()[1];
        builder->topology.dimension = topologyObject->Get("cells")->Attribute<unsigned long long>("cell_dim");
    }

    // hybrid topology
    std::shared_ptr<petscXdmfGenerator::HdfObject> hybridTopologyObject = FindHdfChild(rootObject, "hybrid_topology");
    if (hybridTopologyObject) {
        builder->hybridTopology.path = hybridTopologyObject->Path();
        builder->hybridTopology.number = hybridTopologyObject->Get("cells")->Shape()[0];
        builder->hybridTopology.numberCorners = hybridTopologyObject->Get("cells")->Shape()[1];
    }

    // get the time
    builder->time = rootObject->Get("time")->RawData<double>();

    // get the vertex fields and map into a vertex map
    if (rootObject->Contains("vertex_fields")) {
        for (auto& vertexField : rootObject->Get("vertex_fields")->Items()) {
            FieldDescription description{.name = vertexField->Name(), .path = vertexField->Path(), .shape = vertexField->Shape(), .vectorFieldType = vertexField->AttributeString("vector_field_type")};
            builder->vertexFields.push_back(description);
        }
    }
    if (rootObject->Contains("cell_fields")) {
        for (auto& cellField : rootObject->Get("cell_fields")->Items()) {
            FieldDescription description{.name = cellField->Name(), .path = cellField->Path(), .shape = cellField->Shape(), .vectorFieldType = cellField->AttributeString("vector_field_type")};
            builder->cellFields.push_back(description);
        }
    }

    // Get the time history
    return builder;
}

std::shared_ptr<petscXdmfGenerator::HdfObject> petscXdmfGenerator::XdmfBuilder::FindHdfChild(std::shared_ptr<petscXdmfGenerator::HdfObject>& root, std::string name) {
    if (root->Contains("viz") && root->Get("viz")->Contains(name)) {
        return root->Get("viz")->Get(name);
    } else if (root->Contains(name)) {
        return root->Get(name);
    }

    return nullptr;
}

std::unique_ptr<petscXdmfGenerator::XmlElement> petscXdmfGenerator::XdmfBuilder::Build() {
    // build the preamble
    std::string preamble =
        "<?xml version=\"1.0\" ?>\n"
        "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" [\n"
        "<!ENTITY HeavyData \"" +
        hdf5File +
        "\">\n"
        "]>";

    auto documentPointer = std::make_unique<XmlElement>("Xdmf", preamble);
    auto& document = *documentPointer;

    // create the single domain (better for visit)
    auto& domainElement = document["Domain"];
    domainElement("Name") = "domain";

    // add cells for topology
    if (!topology.path.empty() && topology.number > 0) {
        WriteCells(domainElement, topology.path, topology.number, topology.numberCorners);
    }
    if (!hybridTopology.path.empty() && hybridTopology.number > 0) {
        WriteCells(domainElement, hybridTopology.path, hybridTopology.number, hybridTopology.numberCorners, "hcells");
    }
    // and the vertices
    if (!geometry.path.empty() && geometry.number > 0) {
        WriteVertices(domainElement, geometry.path, geometry.number, geometry.dimension);
    }

    // check if we should use time
    auto useTime = !(time.size() < 2 && time[0] == -1);

    // specify if we add each grid to the domain or a timeGridBase
    auto& gridBase = useTime ? GenerateTimeGrid(domainElement, time) : domainElement;

    // march over and add each grid for each time
    for (auto timeIndex = 0; timeIndex < time.size(); timeIndex++) {
        // add in the hybrid header
        auto& timeIndexBase = hybridTopology.number > 0 ? GenerateHybridSpaceGrid(gridBase) : gridBase;
        if (hybridTopology.number > 0) {
            GenerateSpaceGrid(timeIndexBase, hybridTopology.number, hybridTopology.numberCorners, hybridTopology.dimension, geometry.dimension, "hcells");
        }

        // write the space header
        auto& spaceGrid = GenerateSpaceGrid(timeIndexBase, topology.number, topology.numberCorners, topology.dimension, geometry.dimension);

        // add in each field
        for (auto& field : vertexFields) {
            WriteField(spaceGrid, field, timeIndex, time.size(), topology.dimension, geometry.dimension, "Node");
        }

        for (auto& field : cellFields) {
            WriteField(spaceGrid, field, timeIndex, time.size(), topology.dimension, geometry.dimension, "Cell");
        }
    }

    return documentPointer;
}

void petscXdmfGenerator::XdmfBuilder::WriteCells(petscXdmfGenerator::XmlElement& element, std::string path, unsigned long long int numberCells, unsigned long long int numberCorners,
                                                 std::string cellName) {
    auto& dataItem = element[DataItem];
    dataItem("Name") = cellName;
    dataItem("ItemType") = "Uniform";
    dataItem("Format") = "HDF";
    dataItem("Precision") = "8";
    dataItem("NumberType") = "Float";
    dataItem("Dimensions") = std::to_string(numberCells) + " " + std::to_string(numberCorners);
    dataItem() = "&HeavyData;:" + path + "/cells";
}

void petscXdmfGenerator::XdmfBuilder::WriteVertices(petscXdmfGenerator::XmlElement& element, std::string path, unsigned long long int number, unsigned long long int dimensions) {
    auto& dataItem = element[DataItem];
    dataItem("Name") = "vertices";
    dataItem("Format") = "HDF";
    dataItem("Dimensions") = std::to_string(number) + " " + std::to_string(dimensions);
    dataItem() = "&HeavyData;:" + path + "/vertices";
}

petscXdmfGenerator::XmlElement& petscXdmfGenerator::XdmfBuilder::GenerateTimeGrid(petscXdmfGenerator::XmlElement& element, const std::vector<double>& time) {
    auto& gridItem = element[Grid];
    gridItem("Name") = "TimeSeries";
    gridItem("GridType") = "Collection";
    gridItem("CollectionType") = "Temporal";

    auto& timeElement = gridItem["Time"];
    timeElement("TimeType") = "List";

    auto& dataItem = timeElement[DataItem];
    dataItem("Format") = "XML";
    dataItem("NumberType") = "Float";
    dataItem("Dimensions") = std::to_string(time.size());
    dataItem() = Join(time);

    return gridItem;
}

petscXdmfGenerator::XmlElement& petscXdmfGenerator::XdmfBuilder::GenerateHybridSpaceGrid(petscXdmfGenerator::XmlElement& element) {
    auto& hybridGridItem = element[Grid];
    hybridGridItem("Name") = "Domain";
    hybridGridItem("GridType") = "Collection";
    return hybridGridItem;
}

petscXdmfGenerator::XmlElement& petscXdmfGenerator::XdmfBuilder::GenerateSpaceGrid(petscXdmfGenerator::XmlElement& element, unsigned long long int numberCells, unsigned long long int numberCorners,
                                                                                   unsigned long long int cellDimension, unsigned long long int spaceDimension, std::string cellName) {
    auto& gridItem = element[Grid];
    gridItem("Name") = "domain";
    gridItem("GridType") = "Uniform";

    {
        auto& topology = gridItem["Topology"];
        topology("TopologyType") = cellMap[cellDimension][numberCorners];
        topology("NumberOfElements") = std::to_string(numberCells);

        {
            auto& dataItem = topology[DataItem];
            dataItem("Reference") = "XML";
            dataItem() = "/Xdmf/Domain/DataItem[@Name=\"" + cellName + "\"]";
        }
    }

    auto& geometry = gridItem["Geometry"];
    geometry("GeometryType") = spaceDimension > 2 ? "XYZ" : "XY";
    {
        auto& dataItem = geometry[DataItem];
        dataItem("Reference") = "XML";
        dataItem() = "/Xdmf/Domain/DataItem[@Name=\"vertices\"]";
    }

    return gridItem;
}

void petscXdmfGenerator::XdmfBuilder::WriteField(petscXdmfGenerator::XmlElement& element, petscXdmfGenerator::XdmfBuilder::FieldDescription& fieldDescription, unsigned long long timeStep,
                                                 unsigned long long numSteps, unsigned long long cellDimension, unsigned long long spaceDimension, std::string domain) {
    std::set<std::string> componentTypes = {"tensor", "matrix"};
    //    if (spaceDimension == 2 || cellDimension != spaceDimension){
    //        componentTypes.insert("vector");
    //    }
    // check if this is written as a component or single
    if (componentTypes.find(fieldDescription.vectorFieldType) != componentTypes.end()) {
        unsigned long long dof;
        unsigned long long bs;
        std::string cdims;
        std::string dims;
        std::string stride;
        std::string size;

        if (fieldDescription.shape.size() > 2) {
            dof = fieldDescription.shape[1];
            bs = fieldDescription.shape[2];
            cdims = Join(1, dof, 1);
            dims = Join(numSteps, dof, bs);
            stride = Join(1, 1, 1);
            size = Join(1, dof, 1);
        } else {
            dof = fieldDescription.shape[0];
            bs = fieldDescription.shape[1];
            cdims = Join(1, dof);
            dims = Join(dof, bs);
            stride = Join(1, 1);
            size = Join(dof, 1);
        }

        for (auto c = 0; c < bs; c++) {
            auto ext = typeExt[spaceDimension][fieldDescription.vectorFieldType][c];
            auto start = fieldDescription.shape.size() > 2 ? Join(timeStep, 0, c) : Join(0, c);

            auto& attribute = element["Attribute"];
            attribute("Name") = fieldDescription.name + "_" + ext;
            attribute("Type") = "Scalar";
            attribute("Center") = domain;

            auto& dataItem = attribute[DataItem];
            dataItem("ItemType") = "HyperSlab";
            dataItem("Dimensions") = cdims;
            dataItem("Type") = "HyperSlab";

            {
                auto& dataItemItem = dataItem[DataItem];
                dataItemItem("Dimensions") = Join(3, fieldDescription.shape.size());
                dataItemItem("Format") = "XML";
                dataItemItem() = start + " " + stride + " " + size;
            }
            {
                auto& dataItemItem = dataItem[DataItem];
                dataItemItem("DataType") = "Float";
                dataItemItem("Dimensions") = dims;
                dataItemItem("Format") = "HDF";
                dataItemItem("Precision") = "8";
                dataItemItem() = "&HeavyData;:" + fieldDescription.path;
            }
        }
    } else {
        unsigned long long dof;
        unsigned long long bs;
        if (fieldDescription.shape.size() > 2) {
            dof = fieldDescription.shape[1];
            bs = fieldDescription.shape[2];
        } else if (fieldDescription.shape.size() > 1) {
            if (numSteps > 1) {
                dof = fieldDescription.shape[1];
                bs = 1;
            } else {
                dof = fieldDescription.shape[0];
                bs = fieldDescription.shape[1];
            }
        } else {
            dof = fieldDescription.shape[0];
            bs = 1;
        }
        auto& attribute = element["Attribute"];
        attribute("Name") = fieldDescription.name;
        attribute("Type") = typeMap[fieldDescription.vectorFieldType];
        attribute("Center") = domain;

        auto& dataItem = attribute[DataItem];
        dataItem("ItemType") = "HyperSlab";
        dataItem("Dimensions") = Join(1, dof, bs);
        dataItem("Type") = "HyperSlab";

        {
            auto& dataItemItem = dataItem[DataItem];
            dataItemItem("Dimensions") = Join(3, 3);
            dataItemItem("Format") = "XML";
            dataItemItem() = Join(timeStep, 0, 0) + " " + Join(1, 1, 1) + " " + Join(1, dof, bs);
        }
        {
            auto& dataItemItem = dataItem[DataItem];
            dataItemItem("DataType") = "Float";
            dataItemItem("Dimensions") = Join(numSteps, dof, bs);
            dataItemItem("Format") = "HDF";
            dataItemItem("Precision") = "8";
            dataItemItem() = "&HeavyData;:" + fieldDescription.path;
        }
    }
}
