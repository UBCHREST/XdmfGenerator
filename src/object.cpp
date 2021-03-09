#include "object.hpp"
#include <map>

static std::map<H5O_type_t, std::string> h50bjectTypes = {
        {H5O_TYPE_UNKNOWN, "H5O_TYPE_UNKNOWN"},
        {H5O_TYPE_GROUP, "H5O_TYPE_GROUP"},
        {H5O_TYPE_DATASET, "H5O_TYPE_DATASET"},
        {H5O_TYPE_NAMED_DATATYPE, "H5O_TYPE_NAMED_DATATYPE"},
        {H5O_TYPE_MAP, "H5O_TYPE_MAP"}
        };

petscXdmfGenerator::Object::Object(hid_t locId, H5O_info_t information): locId(locId), information(information) {

}

petscXdmfGenerator::Object::Object(std::shared_ptr<Object> parent, H5O_info_t information, std::string name): information(information), parent(parent), name(name) {
    // Open the location
    locId = H5Oopen_by_addr(parent->locId, information.addr);
    if(locId < 0){
        throw std::runtime_error("unable to open hdf5 object");
    }
}

petscXdmfGenerator::Object::~Object() {
    if(parent) {
        H5Oclose(locId);
    }
}


std::shared_ptr<petscXdmfGenerator::Object> petscXdmfGenerator::Object::Get(std::string name) {
    // check to see if the child is here
    H5O_info_t information;
    auto err = H5Oget_info_by_name(locId, name.c_str(), &information, H5P_DEFAULT);
    if(err < 0){
        return nullptr;
    }else{
        return std::shared_ptr<Object>(new Object(shared_from_this(), information, name));
    }
}

std::ostream &petscXdmfGenerator::operator<<(std::ostream &os, const petscXdmfGenerator::Object &object) {
    os << object.name << ": " << object.TypeName();
    return os;
}

std::string petscXdmfGenerator::Object::TypeName() const {
    return h50bjectTypes.count(Type()) > 0 ? h50bjectTypes.at(Type()) : h50bjectTypes.at(H5O_TYPE_UNKNOWN);
}



