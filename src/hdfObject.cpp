#include "hdfObject.hpp"
#include <map>

static std::map<H5O_type_t, std::string> h50bjectTypes = {{H5O_TYPE_UNKNOWN, "H5O_TYPE_UNKNOWN"},
                                                          {H5O_TYPE_GROUP, "H5O_TYPE_GROUP"},
                                                          {H5O_TYPE_DATASET, "H5O_TYPE_DATASET"},
                                                          {H5O_TYPE_NAMED_DATATYPE, "H5O_TYPE_NAMED_DATATYPE"},
                                                          {H5O_TYPE_MAP, "H5O_TYPE_MAP"}};

xdmfGenerator::HdfObject::HdfObject(std::filesystem::path filePath) : parent(nullptr), name(filePath.filename()) {
    locId = H5Fopen(filePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (locId < 0) {
        throw std::runtime_error("cannot open hdf5 file " + filePath.string());
    }
    auto status = H5Oget_info(locId, &information);
    if (status < 0) {
        throw std::runtime_error("cannot get root object " + filePath.string());
    }
}

xdmfGenerator::HdfObject::HdfObject(std::shared_ptr<HdfObject> parent, H5O_info_t information, std::string name) : parent(parent), name(name), information(information) {
    // Open the location
    locId = H5Oopen_by_addr(parent->locId, information.addr);
    if (locId < 0) {
        throw std::runtime_error("unable to open hdf5 object");
    }
}

xdmfGenerator::HdfObject::~HdfObject() {
    if (parent) {
        H5Oclose(locId);
    } else {
        H5Fclose(locId);
    }
}

std::shared_ptr<xdmfGenerator::HdfObject> xdmfGenerator::HdfObject::Get(std::string name) {
    H5O_info_t information;
    auto err = H5Oget_info_by_name(locId, name.c_str(), &information, H5P_DEFAULT);
    if (err < 0) {
        return nullptr;
    } else {
        return std::shared_ptr<HdfObject>(new HdfObject(shared_from_this(), information, name));
    }
}

struct listDataContext {
    std::vector<std::shared_ptr<xdmfGenerator::HdfObject>> childrenList;
    std::shared_ptr<xdmfGenerator::HdfObject> parent;
};

herr_t xdmfGenerator::HdfObject::addChildToList(hid_t locId, const char *name, const H5L_info_t *info, void *operator_data) {
    // get the children list
    auto context = (listDataContext *)operator_data;

    H5O_info_t information;
    auto err = H5Oget_info_by_name(locId, name, &information, H5P_DEFAULT);
    if (err >= 0) {
        context->childrenList.push_back(std::shared_ptr<xdmfGenerator::HdfObject>(new xdmfGenerator::HdfObject(context->parent, information, name)));
    }
    return err;
}

std::vector<std::shared_ptr<xdmfGenerator::HdfObject>> xdmfGenerator::HdfObject::Items() {
    listDataContext context{.childrenList = {}, .parent = shared_from_this()};

    // march over each child
    auto status = H5Lvisit(locId, H5_INDEX_NAME, H5_ITER_NATIVE, addChildToList, &context);
    if (status < 0) {
        throw std::runtime_error("cannot traverse items in " + name);
    }

    return context.childrenList;
}

bool xdmfGenerator::HdfObject::Contains(std::string name) const {
    auto linkCheck = H5Lexists(locId, name.c_str(), H5P_DEFAULT);
    if (linkCheck < 0) {
        throw std::runtime_error("cannot check link " + name + " in " + this->name);
    } else if (linkCheck == 0) {
        return false;
    }

    // Check to see if the link is an object
    auto objectCheck = H5Oexists_by_name(locId, name.c_str(), H5P_DEFAULT);
    if (objectCheck < 0) {
        throw std::runtime_error("cannot check object " + name + " in " + this->name);
    } else if (objectCheck == 0) {
        return false;
    } else {
        return true;
    }
}

namespace xdmfGenerator {
std::ostream &operator<<(std::ostream &os, const xdmfGenerator::HdfObject &object) {
    os << object.name << ": " << object.TypeName();
    return os;
}
}  // namespace xdmfGenerator

std::string xdmfGenerator::HdfObject::TypeName() const { return h50bjectTypes.count(Type()) > 0 ? h50bjectTypes.at(Type()) : h50bjectTypes.at(H5O_TYPE_UNKNOWN); }

std::string xdmfGenerator::HdfObject::Path() const {
    std::string path = "";
    if (parent) {
        path = parent->Path() + "/";
        path += name;
    }

    return path;
}

std::vector<hsize_t> xdmfGenerator::HdfObject::Shape() const {
    if (Type() != H5O_TYPE_DATASET) {
        throw std::runtime_error("Shape can only be called on H5O_TYPE_DATASET objects");
    }

    // get the object as a dataspace
    auto dataspace = H5Dget_space(locId); /* dataspace handle */
    const auto ndims = H5Sget_simple_extent_ndims(dataspace);

    // prepare the shape vector
    std::vector<hsize_t> shape(ndims);

    H5Sget_simple_extent_dims(dataspace, &shape[0], NULL);
    H5Sclose(dataspace);

    return shape;
}

bool xdmfGenerator::HdfObject::HasAttribute(std::string name) const {
    // Check to see if the link is an attribute
    auto objectCheck = H5Aexists_by_name(locId, ".", name.c_str(), H5P_DEFAULT);
    if (objectCheck < 0) {
        throw std::runtime_error("cannot check attribute " + name + " in " + this->name);
    } else if (objectCheck == 0) {
        return false;
    } else {
        return true;
    }
}
