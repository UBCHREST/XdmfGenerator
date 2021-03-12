
#include "xdmfBuilder.hpp"
#include <map>
#include <set>
#include <string>
#include <algorithm>

static const auto DataItem = "DataItem";
static const auto Grid = "Grid";

using namespace petscXdmfGenerator;
static std::map<unsigned long long, std::map<unsigned long long, std::string>> cellMap = {
    {1, {{0, "Polyvertex"}, {1, "Polyvertex"}, {2, "Polyline"}}}, {2, {{0, "Polyvertex"}, {3, "Triangle"}, {4, "Quadrilateral"}}}, {3, {{0, "Polyvertex"}, {4, "Tetrahedron"}, {6, "Wedge"}, {8, "Hexahedron"}}}};

static std::map<FieldType, std::string> typeMap = {{SCALAR, "Scalar"}, {VECTOR, "Vector"}, {TENSOR, "Tensor6"}, {MATRIX, "Matrix"}};

static std::map<FieldLocation, std::string> locationMap = {{NODE, "Node"}, {CELL, "Cell"}};

static std::map<unsigned long long, std::map<FieldType, std::vector<std::string>>> typeExt = {{2, {{VECTOR, {"x", "y"}}, {TENSOR, {"xx", "yy", "xy"}}}},
                                                                                                {3, {{VECTOR, {"x", "y", "z"}}, {TENSOR, {"xx", "yy", "zz", "xy", "yz", "xz"}}}}};

XdmfBuilder::XdmfBuilder(std::shared_ptr<XdmfSpecification> specification) : specification(specification) {
}


std::unique_ptr<petscXdmfGenerator::XmlElement> petscXdmfGenerator::XdmfBuilder::Build() {
    // build the preamble
    std::string preamble =
        "<?xml version=\"1.0\" ?>\n"
        "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" [\n"
        "<!ENTITY HeavyData \"" +
        specification->hdf5File +
        "\">\n"
        "]>";

    auto documentPointer = std::make_unique<XmlElement>("Xdmf", preamble);
    auto& document = *documentPointer;

    // create the single domain (better for visit)
    auto& domainElement = document["Domain"];
    domainElement("Name") = "domain";

    // add in each grid
    for(auto& xdmfGrid : specification->grids){
        // store a central reference for time invariant data
        if(xdmfGrid.gridTimeInvariant) {
            if (xdmfGrid.topology.number > 0) {
                WriteCells(domainElement, xdmfGrid.topology);
            }
            if (xdmfGrid.hybridTopology.number > 0) {
                WriteCells(domainElement, xdmfGrid.hybridTopology);
            }
            // and the vertices
            if (xdmfGrid.geometry.number > 0) {
                WriteVertices(domainElement, xdmfGrid.geometry);
            }
        }

        // check if we should use time
        if(xdmfGrid.time.empty()){
            xdmfGrid.time.push_back(-1); // make sure we do at least one time
        }
        auto useTime = !(xdmfGrid.time.size() < 2 && xdmfGrid.time[0] == -1);

        // specify if we add each grid to the domain or a timeGridBase
        auto& gridBase = useTime ? GenerateTimeGrid(domainElement, xdmfGrid.time) : domainElement;

        // march over and add each grid for each time
        for (auto timeIndex = 0; timeIndex < xdmfGrid.time.size(); timeIndex++) {
            auto gridTimeIndex = xdmfGrid.gridTimeInvariant? TimeInvariant : timeIndex;

            // add in the hybrid header
            auto& timeIndexBase = xdmfGrid.hybridTopology.number > 0 ? GenerateHybridSpaceGrid(gridBase) : gridBase;
            if (xdmfGrid.hybridTopology.number > 0) {
                GenerateSpaceGrid(timeIndexBase, xdmfGrid.hybridTopology, xdmfGrid.geometry, gridTimeIndex);
            }

            // write the space header
            auto& spaceGrid = GenerateSpaceGrid(timeIndexBase, xdmfGrid.topology, xdmfGrid.geometry, gridTimeIndex);

            // add in each field
            for (auto& field : xdmfGrid.fields) {
                WriteField(spaceGrid, field, timeIndex, xdmfGrid.time.size(), xdmfGrid.topology.dimension, xdmfGrid.geometry.dimension);
            }
        }
    }

    return documentPointer;
}

void petscXdmfGenerator::XdmfBuilder::WriteCells(petscXdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription, unsigned long long timeStep) {
    // check for an existing reference
    if(HasReference(topologyDescription.path) && timeStep == TimeInvariant){
        UseReference(element, topologyDescription.path);
    }else{
        auto& dataItem = element[DataItem];
        dataItem("Name") = Hdf5PathToName(topologyDescription.path);
        dataItem("ItemType") = "Uniform";
        dataItem("Format") = "HDF";
        dataItem("Precision") = "8";
        dataItem("NumberType") = "Float";
        dataItem("Dimensions") = std::to_string(topologyDescription.number) + " " + std::to_string(topologyDescription.numberCorners);
        dataItem() = "&HeavyData;:" + topologyDescription.path;

        if(timeStep == TimeInvariant ){
            AddReference(topologyDescription.path, dataItem.Path());
        }
    }
}

void petscXdmfGenerator::XdmfBuilder::WriteVertices(petscXdmfGenerator::XmlElement& element, const XdmfSpecification::GeometryDescription& geometryDescription, unsigned long long timeStep) {

    // check for an existing reference
    if(HasReference( geometryDescription.path) && timeStep == TimeInvariant){
        UseReference(element,  geometryDescription.path);
    }else{
        auto& dataItem = element[DataItem];
        dataItem("Name") = "vertices";
        dataItem("Format") = "HDF";
        dataItem("Dimensions") = std::to_string(geometryDescription.number) + " " + std::to_string(geometryDescription.dimension);
        dataItem() = "&HeavyData;:" + geometryDescription.path;

        if(timeStep == TimeInvariant ){
            AddReference( geometryDescription.path, dataItem.Path());
        }
    }

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

petscXdmfGenerator::XmlElement& petscXdmfGenerator::XdmfBuilder::GenerateSpaceGrid(petscXdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription , const XdmfSpecification::GeometryDescription& geometryDescription, unsigned long long timeStep) {
    auto& gridItem = element[Grid];
    gridItem("Name") = "domain";
    gridItem("GridType") = "Uniform";

    {
        auto& topology = gridItem["Topology"];
        topology("TopologyType") = cellMap[topologyDescription.dimension][topologyDescription.numberCorners];
        if(topologyDescription.numberCorners == 0){
            topology("NodesPerElement") = std::to_string(topologyDescription.number);

        }else{
            topology("NumberOfElements") = std::to_string(topologyDescription.number);
            WriteCells(topology, topologyDescription, timeStep);
        }
    }

    auto& geometry = gridItem["Geometry"];
    geometry("GeometryType") = geometryDescription.dimension > 2 ? "XYZ" : "XY";
    WriteVertices(geometry, geometryDescription, timeStep);

    return gridItem;
}

void petscXdmfGenerator::XdmfBuilder::WriteField(petscXdmfGenerator::XmlElement& element, petscXdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription, unsigned long long timeStep,
                                                 unsigned long long numSteps, unsigned long long cellDimension, unsigned long long spaceDimension) {
    std::set<FieldType> componentTypes = {TENSOR, MATRIX};
    if (cellDimension != spaceDimension){
        componentTypes.insert(VECTOR);
    }
    // check if this is written as a component or single
    if (componentTypes.find(fieldDescription.fieldType) != componentTypes.end()) {
        unsigned long long dof;
        unsigned long long nComp;
        std::string cdims;
        std::string dims;
        std::string stride;
        std::string size;

        if (fieldDescription.shape.size() > 2) {
            dof = fieldDescription.shape[1];
            nComp = fieldDescription.shape[2];
            cdims = Join(1, dof, 1);
            dims = Join(numSteps, dof, nComp);
            stride = Join(1, 1, 1);
            size = Join(1, dof, 1);
        } else {
            dof = fieldDescription.shape[0];
            nComp = fieldDescription.shape[1];
            cdims = Join(1, dof);
            dims = Join(dof, nComp);
            stride = Join(1, 1);
            size = Join(dof, 1);
        }

        for (auto c = 0; c < nComp; c++) {
            auto ext = typeExt[spaceDimension][fieldDescription.fieldType][c];
            auto start = fieldDescription.shape.size() > 2 ? Join(timeStep, 0, c) : Join(0, c);

            auto& attribute = element["Attribute"];
            attribute("Name") = fieldDescription.name + "_" + ext;
            attribute("Type") = "Scalar";
            attribute("Center") = locationMap[fieldDescription.fieldLocation];

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
        unsigned long long nComp;
        if (fieldDescription.shape.size() > 2) {
            dof = fieldDescription.shape[1];
            nComp = fieldDescription.shape[2];
        } else if (fieldDescription.shape.size() > 1) {
            if (numSteps > 1) {
                dof = fieldDescription.shape[1];
                nComp = 1;
            } else {
                dof = fieldDescription.shape[0];
                nComp = fieldDescription.shape[1];
            }
        } else {
            dof = fieldDescription.shape[0];
            nComp = 1;
        }
        auto& attribute = element["Attribute"];
        attribute("Name") = fieldDescription.name;
        attribute("Type") = typeMap[fieldDescription.fieldType];
        attribute("Center") = locationMap[fieldDescription.fieldLocation];

        auto& dataItem = attribute[DataItem];
        dataItem("ItemType") = "HyperSlab";
        dataItem("Dimensions") = Join(1, dof, nComp);
        dataItem("Type") = "HyperSlab";

        {
            auto& dataItemItem = dataItem[DataItem];
            dataItemItem("Dimensions") = Join(3, 3);
            dataItemItem("Format") = "XML";
            dataItemItem() = Join(timeStep, 0, 0) + " " + Join(1, 1, 1) + " " + Join(1, dof, nComp);
        }
        {
            auto& dataItemItem = dataItem[DataItem];
            dataItemItem("DataType") = "Float";
            dataItemItem("Dimensions") = Join(numSteps, dof, nComp);
            dataItemItem("Format") = "HDF";
            dataItemItem("Precision") = "8";
            dataItemItem() = "&HeavyData;:" + fieldDescription.path;
        }
    }
}

void XdmfBuilder::UseReference(XmlElement& element, std::string id) {
    auto& reference = element[DataItem];
    reference("Reference") = "XML";
    reference() = xmlReferences.at(id);
}
std::string XdmfBuilder::Hdf5PathToName(std::string hdf5Path) {
    // right now everything we are using has a unique name, this can be relaxed in the future.
    auto pos = hdf5Path.rfind("/");
    if (pos != std::string::npos) {
        hdf5Path = hdf5Path.substr(pos + 1);
    }

    return hdf5Path;
}
