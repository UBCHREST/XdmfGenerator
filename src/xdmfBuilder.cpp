
#include "xdmfBuilder.hpp"
#include <algorithm>
#include <map>
#include <string>

static const auto DataItem = "DataItem";
static const auto Grid = "Grid";

using namespace petscXdmfGenerator;
static std::map<unsigned long long, std::map<unsigned long long, std::string>> cellMap = {{1, {{0, "Polyvertex"}, {1, "Polyvertex"}, {2, "Polyline"}}},
                                                                                          {2, {{0, "Polyvertex"}, {3, "Triangle"}, {4, "Quadrilateral"}}},
                                                                                          {3, {{0, "Polyvertex"}, {4, "Tetrahedron"}, {6, "Wedge"}, {8, "Hexahedron"}}}};

static std::map<FieldType, std::string> typeMap = {{SCALAR, "Scalar"}, {VECTOR, "Vector"}, {TENSOR, "Tensor6"}, {MATRIX, "Matrix"}};

static std::map<FieldLocation, std::string> locationMap = {{NODE, "Node"}, {CELL, "Cell"}};

static std::map<unsigned long long, std::map<FieldType, std::vector<std::string>>> typeExt = {{2, {{VECTOR, {"x", "y"}}, {TENSOR, {"xx", "yy", "xy"}}}},
                                                                                              {3, {{VECTOR, {"x", "y", "z"}}, {TENSOR, {"xx", "yy", "zz", "xy", "yz", "xz"}}}}};

XdmfBuilder::XdmfBuilder(std::shared_ptr<XdmfSpecification> specification) : specification(specification) {}

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
    for (auto& xdmfGrid : specification->grids) {
        // store a central reference for time invariant data
        if (!xdmfGrid.geometry.HasTimeDimension()) {
            if (xdmfGrid.topology.number > 0) {
                WriteCells(domainElement, xdmfGrid.topology);
            }
            if (xdmfGrid.hybridTopology.number > 0) {
                WriteCells(domainElement, xdmfGrid.hybridTopology);
            }
            // and the vertices
            if (xdmfGrid.geometry.GetDof() > 0) {
                WriteVertices(domainElement, xdmfGrid.geometry);
            }
        }

        // check if we should use time
        if (xdmfGrid.time.empty()) {
            xdmfGrid.time.push_back(-1);  // make sure we do at least one time
        }
        auto useTime = !(xdmfGrid.time.size() < 2 && xdmfGrid.time[0] == -1);

        // specify if we add each grid to the domain or a timeGridBase
        auto& gridBase = useTime ? GenerateTimeGrid(domainElement, xdmfGrid.time) : domainElement;

        // march over and add each grid for each time
        for (auto timeIndex = 0; timeIndex < xdmfGrid.time.size(); timeIndex++) {
            auto gridTimeIndex = xdmfGrid.geometry.HasTimeDimension() ? timeIndex : TimeInvariant;

            // add in the hybrid header
            auto& timeIndexBase = xdmfGrid.hybridTopology.number > 0 ? GenerateHybridSpaceGrid(gridBase, xdmfGrid.name) : gridBase;
            if (xdmfGrid.hybridTopology.number > 0) {
                GenerateSpaceGrid(timeIndexBase, xdmfGrid.hybridTopology, xdmfGrid.geometry, gridTimeIndex, xdmfGrid.name);
            }

            // write the space header
            auto& spaceGrid = GenerateSpaceGrid(timeIndexBase, xdmfGrid.topology, xdmfGrid.geometry, gridTimeIndex, xdmfGrid.name);

            // add in each field
            for (auto& field : xdmfGrid.fields) {
                WriteField(spaceGrid, field, timeIndex);
            }
        }
    }

    return documentPointer;
}

void petscXdmfGenerator::XdmfBuilder::WriteCells(petscXdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription, unsigned long long timeStep) {
    // check for an existing reference
    if (HasReference(topologyDescription.path) && timeStep == TimeInvariant) {
        UseReference(element, topologyDescription.path);
    } else {
        auto& dataItem = element[DataItem];
        dataItem("Name") = Hdf5PathToName(topologyDescription.path);
        dataItem("ItemType") = "Uniform";
        dataItem("Format") = "HDF";
        dataItem("Precision") = "8";
        dataItem("NumberType") = "Float";
        dataItem("Dimensions") = std::to_string(topologyDescription.number) + " " + std::to_string(topologyDescription.numberCorners);
        dataItem() = "&HeavyData;:" + topologyDescription.path;

        if (timeStep == TimeInvariant) {
            AddReference(topologyDescription.path, dataItem.Path());
        }
    }
}

void petscXdmfGenerator::XdmfBuilder::WriteVertices(petscXdmfGenerator::XmlElement& element, const XdmfSpecification::FieldDescription& geometryDescription, unsigned long long timeStep) {
    // check for an existing reference
    if (HasReference(geometryDescription.path) && timeStep == TimeInvariant) {
        UseReference(element, geometryDescription.path);
    } else {
        auto& dataItem = WriteData(element, geometryDescription, timeStep);
        if (timeStep == TimeInvariant) {
            AddReference(geometryDescription.path, dataItem.Path());
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
    dataItem() = JoinVector(time);

    return gridItem;
}

petscXdmfGenerator::XmlElement& petscXdmfGenerator::XdmfBuilder::GenerateHybridSpaceGrid(petscXdmfGenerator::XmlElement& element, const std::string& domainName) {
    auto& hybridGridItem = element[Grid];
    hybridGridItem("Name") = domainName;
    hybridGridItem("GridType") = "Collection";
    return hybridGridItem;
}

petscXdmfGenerator::XmlElement& petscXdmfGenerator::XdmfBuilder::GenerateSpaceGrid(petscXdmfGenerator::XmlElement& element, const XdmfSpecification::TopologyDescription& topologyDescription,
                                                                                   const XdmfSpecification::FieldDescription& geometryDescription, unsigned long long timeStep,
                                                                                   const std::string& domainName) {
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
            WriteCells(topology, topologyDescription, timeStep);
        }
    }

    auto& geometry = gridItem["Geometry"];
    geometry("GeometryType") = geometryDescription.GetDimension() > 2 ? "XYZ" : "XY";
    WriteVertices(geometry, geometryDescription, timeStep);

    return gridItem;
}

XmlElement& petscXdmfGenerator::XdmfBuilder::WriteData(petscXdmfGenerator::XmlElement& element, const petscXdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription,
                                                       unsigned long long timeStep) {
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
            dataItemItem() = Join(timeStep, 0, 0) + " " + Join(1, 1, 1) + " " + Join(1, fieldDescription.GetDof(), fieldDescription.GetDimension());  // start, stride, size
        }
        {
            auto& dataItemItem = dataItem[DataItem];
            dataItemItem("DataType") = "Float";
            dataItemItem("Dimensions") = JoinVector(fieldDescription.shape);
            dataItemItem("Format") = "HDF";
            dataItemItem("Precision") = "8";
            dataItemItem() = "&HeavyData;:" + fieldDescription.path;
        }
        return dataItem;
    } else {
        auto& dataItemItem = element[DataItem];
        dataItemItem("Name") = Hdf5PathToName(fieldDescription.path);
        dataItemItem("DataType") = "Float";
        dataItemItem("Dimensions") = JoinVector(fieldDescription.shape);
        dataItemItem("Format") = "HDF";
        dataItemItem("Precision") = "8";
        dataItemItem() = "&HeavyData;:" + fieldDescription.path;
        return dataItemItem;
    }
}

void petscXdmfGenerator::XdmfBuilder::WriteField(petscXdmfGenerator::XmlElement& element, petscXdmfGenerator::XdmfSpecification::FieldDescription& fieldDescription, unsigned long long timeStep) {
    auto& attribute = element["Attribute"];
    attribute("Name") = fieldDescription.name;
    attribute("Type") = typeMap[fieldDescription.fieldType];
    attribute("Center") = locationMap[fieldDescription.fieldLocation];

    WriteData(attribute, fieldDescription, timeStep);
}

void XdmfBuilder::UseReference(XmlElement& element, std::string id) {
    auto& reference = element[DataItem];
    reference("Reference") = "XML";
    reference() = xmlReferences.at(id);
}
std::string XdmfBuilder::Hdf5PathToName(std::string hdf5Path) {
    std::replace(hdf5Path.begin(), hdf5Path.end(), '/', '_');
    return hdf5Path;
}
