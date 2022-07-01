
#include "xdmfBuilder.hpp"
#include <algorithm>
#include <map>
#include <string>

static const auto DataItem = "DataItem";
static const auto Grid = "Grid";

using namespace xdmfGenerator;
static std::map<unsigned long long, std::map<unsigned long long, std::string>> cellMap = {{1, {{0, "Polyvertex"}, {1, "Polyvertex"}, {2, "Polyline"}}},
                                                                                          {2, {{0, "Polyvertex"}, {3, "Triangle"}, {4, "Quadrilateral"}}},
                                                                                          {3, {{0, "Polyvertex"}, {4, "Tetrahedron"}, {6, "Wedge"}, {8, "Hexahedron"}}}};

static std::map<FieldType, std::string> typeMap = {{SCALAR, "Scalar"}, {VECTOR, "Vector"}, {TENSOR, "Tensor6"}, {MATRIX, "Matrix"}};

static std::map<FieldLocation, std::string> locationMap = {{NODE, "Node"}, {CELL, "Cell"}};

static std::map<unsigned long long, std::map<FieldType, std::vector<std::string>>> typeExt = {{2, {{VECTOR, {"x", "y"}}, {TENSOR, {"xx", "yy", "xy"}}}},
                                                                                              {3, {{VECTOR, {"x", "y", "z"}}, {TENSOR, {"xx", "yy", "zz", "xy", "yz", "xz"}}}}};

XdmfBuilder::XdmfBuilder(std::shared_ptr<XdmfSpecification> specification) : specification(specification) {}

std::unique_ptr<xdmfGenerator::XmlElement> xdmfGenerator::XdmfBuilder::Build() {
    // build the preamble
    std::string preamble =
        "<?xml version=\"1.0\" ?>\n"
        "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>";

    auto documentPointer = std::make_unique<XmlElement>("Xdmf", preamble);
    auto& document = *documentPointer;

    // create the single domain (better for visit)
    auto& domainElement = document["Domain"];
    domainElement("Name") = "domain";

    // add in each grid
    for (auto& xdmfGridCollection : specification->gridsCollections) {
        // If there is a time and it is positive
        auto useTime = !(xdmfGridCollection.grids.empty() || xdmfGridCollection.grids.front().time < 0);

        // build a time vector
        std::vector<double> timeVector;
        for (const auto& grid : xdmfGridCollection.grids) {
            timeVector.push_back(grid.time);
        }

        // specify if we add each grid to the domain or a timeGridBase
        auto& gridBase = useTime ? GenerateTimeGrid(domainElement, timeVector) : domainElement;

        // march over and add each grid for each time
        for (std::size_t timeIndex = 0; timeIndex < timeVector.size(); timeIndex++) {
            auto& grid = xdmfGridCollection.grids[timeIndex];

            // add in the hybrid header
            auto& timeIndexBase = grid.hybridTopology.number > 0 ? GenerateHybridSpaceGrid(gridBase, xdmfGridCollection.name) : gridBase;
            if (grid.hybridTopology.number > 0) {
                GenerateSpaceGrid(timeIndexBase, grid.hybridTopology, grid.geometry, xdmfGridCollection.name);
            }

            // write the space header
            auto& spaceGrid = GenerateSpaceGrid(timeIndexBase, grid.topology, grid.geometry, xdmfGridCollection.name);

            // add in each field
            for (auto& field : grid.fields) {
                WriteField(spaceGrid, field);
            }
        }
    }

    return documentPointer;
}

void xdmfGenerator::XdmfBuilder::WriteCells(xdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription) {
    // check for an existing reference
    auto& dataItem = element[DataItem];
    dataItem("Name") = Hdf5PathToName(topologyDescription.location.path);
    dataItem("ItemType") = "Uniform";
    dataItem("Format") = "HDF";
    dataItem("Precision") = "8";
    dataItem("NumberType") = "Float";
    dataItem("Dimensions") = std::to_string(topologyDescription.number) + " " + std::to_string(topologyDescription.numberCorners);
    dataItem() = topologyDescription.location.file + ":" + topologyDescription.location.path;
}

void xdmfGenerator::XdmfBuilder::WriteVertices(xdmfGenerator::XmlElement& element, const XdmfSpecification::FieldDescription& geometryDescription) {
    // check for an existing reference
    WriteData(element, geometryDescription);
}

xdmfGenerator::XmlElement& xdmfGenerator::XdmfBuilder::GenerateTimeGrid(xdmfGenerator::XmlElement& element, const std::vector<double>& time) {
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
    dataItem() = JoinVector(time);

    return gridItem;
}

xdmfGenerator::XmlElement& xdmfGenerator::XdmfBuilder::GenerateHybridSpaceGrid(xdmfGenerator::XmlElement& element, const std::string& domainName) {
    auto& hybridGridItem = element[Grid];
    hybridGridItem("Name") = domainName;
    hybridGridItem("GridType") = "Collection";
    return hybridGridItem;
}

xdmfGenerator::XmlElement& xdmfGenerator::XdmfBuilder::GenerateSpaceGrid(xdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription,
                                                                                   const XdmfSpecification::FieldDescription& geometryDescription, const std::string& domainName) {
    auto& gridItem = element[Grid];
    gridItem("Name") = domainName;
    gridItem("GridType") = "Uniform";

    {
        auto& topology = gridItem["Topology"];
        topology("TopologyType") = cellMap[topologyDescription.dimension][topologyDescription.numberCorners];
        if (topologyDescription.numberCorners == 0) {
            topology("NodesPerElement") = std::to_string(topologyDescription.number);

        } else {
            topology("NumberOfElements") = std::to_string(topologyDescription.number);
            WriteCells(topology, topologyDescription);
        }
    }

    auto& geometry = gridItem["Geometry"];
    geometry("GeometryType") = geometryDescription.GetDimension() > 2 ? "XYZ" : "XY";
    WriteVertices(geometry, geometryDescription);

    return gridItem;
}

XmlElement& xdmfGenerator::XdmfBuilder::WriteData(xdmfGenerator::XmlElement& element, const xdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription) {
    // determine if we need to use a HyperSlab
    if (fieldDescription.HasTimeDimension()) {
        auto& dataItem = element[DataItem];
        dataItem("ItemType") = "HyperSlab";
        dataItem("Dimensions") = Join(1, fieldDescription.GetDof(), fieldDescription.GetDimension());
        dataItem("Type") = "HyperSlab";

        {
            auto& dataItemItem = dataItem[DataItem];
            dataItemItem("Dimensions") = Join(3, 3);
            dataItemItem("Format") = "XML";
            dataItemItem() = Join(fieldDescription.timeOffset, 0, fieldDescription.componentOffset) + " " + Join(1, 1, fieldDescription.componentStride) + " " +
                             Join(1, fieldDescription.GetDof(), fieldDescription.GetDimension());  // start, stride, size
        }
        {
            auto& dataItemItem = dataItem[DataItem];
            dataItemItem("DataType") = "Float";
            dataItemItem("Dimensions") = JoinVector(fieldDescription.shape);
            dataItemItem("Format") = "HDF";
            dataItemItem("Precision") = "8";
            dataItemItem() = fieldDescription.location.file + ":" + fieldDescription.location.path;
        }
        return dataItem;
    } else {
        auto& dataItemItem = element[DataItem];
        dataItemItem("Name") = Hdf5PathToName(fieldDescription.location.path);
        dataItemItem("DataType") = "Float";
        dataItemItem("Dimensions") = JoinVector(fieldDescription.shape);
        dataItemItem("Format") = "HDF";
        dataItemItem("Precision") = "8";
        dataItemItem() = fieldDescription.location.file + ":" + fieldDescription.location.path;
        return dataItemItem;
    }
}

void xdmfGenerator::XdmfBuilder::WriteField(xdmfGenerator::XmlElement& element, xdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription) {
    auto& attribute = element["Attribute"];
    attribute("Name") = fieldDescription.name;
    attribute("Type") = typeMap[fieldDescription.fieldType];
    attribute("Center") = locationMap[fieldDescription.fieldLocation];

    WriteData(attribute, fieldDescription);
}

std::string XdmfBuilder::Hdf5PathToName(std::string hdf5Path) {
    std::replace(hdf5Path.begin(), hdf5Path.end(), '/', '_');
    return hdf5Path;
}
