#ifndef XDMFGENERATOR_HDFOBJECT_HPP
#define XDMFGENERATOR_HDFOBJECT_HPP

#define H5_USE_18_API_DEFAULT
#include <hdf5.h>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace xdmfGenerator {
class HdfObject : public std::enable_shared_from_this<HdfObject> {
   private:
    const std::shared_ptr<HdfObject> parent = nullptr;
    const std::string name;
    hid_t locId;
    H5O_info_t information{};

    inline static std::map<std::type_index, hid_t> nativeHdfTypes = {{typeid(char), H5T_NATIVE_CHAR},
                                                                     {typeid(signed char), H5T_NATIVE_SCHAR},
                                                                     {typeid(unsigned int), H5T_NATIVE_UCHAR},
                                                                     {typeid(short), H5T_NATIVE_SHORT},
                                                                     {typeid(unsigned short), H5T_NATIVE_USHORT},
                                                                     {typeid(int), H5T_NATIVE_INT},
                                                                     {typeid(unsigned int), H5T_NATIVE_UINT},
                                                                     {typeid(long), H5T_NATIVE_LONG},
                                                                     {typeid(unsigned long), H5T_NATIVE_ULONG},
                                                                     {typeid(long long), H5T_NATIVE_LLONG},
                                                                     {typeid(unsigned long long), H5T_NATIVE_ULLONG},
                                                                     {typeid(float), H5T_NATIVE_FLOAT},
                                                                     {typeid(double), H5T_NATIVE_DOUBLE},
                                                                     {typeid(long double), H5T_NATIVE_LDOUBLE},
                                                                     {typeid(size_t), H5T_NATIVE_HSIZE},
                                                                     {typeid(bool), H5T_NATIVE_HBOOL}};

    /**
     * Static method passed into the hdf5 library to add all children to a list
     * @param locId
     * @param name
     * @param info
     * @param operator_data
     * @return
     */
    static herr_t addChildToList(hid_t locId, const char *name, const H5L_info_t *info, void *operator_data);

   protected:
    HdfObject(const std::shared_ptr<HdfObject> &parent, H5O_info_t information, std::string name);

   public:
    explicit HdfObject(const std::filesystem::path &filePath);
    ~HdfObject();

    inline H5O_type_t Type() const { return information.type; }

    std::string TypeName() const;

    /**
     * Simple function to get child node
     * @param key
     * @return
     */
    std::shared_ptr<HdfObject> Get(const std::string &key);

    /**
     * Get all of the child items
     * @param name
     * @return
     */
    std::vector<std::shared_ptr<HdfObject>> Items();

    /**
     * Simple function to get child node
     * @param key
     * @return
     */
    bool Contains(const std::string &key) const;

    /**
     * The name of the node
     * @return
     */
    const std::string &Name() const { return name; }

    /**
     * Get the path to the current node from root
     * @return
     */
    std::string Path() const;

    /**
     * Gets the shape for a dataset
     * @return
     */
    std::vector<hsize_t> Shape() const;

    /**
     * Checks to see if this object hold an attirbute
     * @param key
     * @return
     */
    bool HasAttribute(const std::string &key) const;

    /**
     * Get an attribute as a certain type
     * @tparam T
     * @param attributeName
     * @return
     */
    template <typename T>
    T Attribute(const std::string &attributeName) const {
        // get the attribute
        auto attLocation = H5Aopen_name(locId, attributeName.c_str());
        if (attLocation < 0) {
            throw std::invalid_argument("unable to find " + attributeName + " on object " + attributeName);
        }

        // get the requestedType
        auto requestedType = nativeHdfTypes[typeid(T)];

        // define and get the data
        T data;
        auto status = H5Aread(attLocation, requestedType, &data);
        if (status < 0) {
            throw std::runtime_error("cannot obtain attribute " + attributeName);
        }
        H5Aclose(attLocation);
        return data;
    }

    /**
     * Get an attribute as a string
     * @tparam T
     * @param attributeName
     * @return
     */
    std::string AttributeString(const std::string &attributeName) const {
        // get the attribute
        auto attLocation = H5Aopen_name(locId, attributeName.c_str());
        if (attLocation < 0) {
            throw std::invalid_argument("unable to find " + attributeName + " on object " + attributeName);
        }

        // Get the datatype and its size.
        auto filetype = H5Aget_type(attLocation);
        auto sdim = H5Tget_size(filetype);
        sdim++; /* Make room for null terminator */

        /*
         * Get dataspace and allocate memory for read buffer.  This is a
         * two dimensional attribute so the dynamic allocation must be done
         * in steps.
         */
        auto space = H5Aget_space(attLocation);
        hsize_t dims[1] = {1};
        H5Sget_simple_extent_dims(space, dims, nullptr);

        // define and allocate the read buffer
        // Allocate array of pointers to rows.
        char **rdata = (char **)malloc(dims[0] * sizeof(char *));
        // Allocate space for integer data.
        rdata[0] = (char *)malloc(dims[0] * sdim * sizeof(char));

        // Set the rest of the pointers to rows to the correct addresses.
        for (hsize_t i = 1; i < dims[0]; i++) {
            rdata[i] = rdata[0] + i * sdim;
        }

        // Create the memory datatype.
        auto memtype = H5Tcopy(H5T_C_S1);
        auto status = H5Tset_size(memtype, sdim);

        // read the data
        status = H5Aread(attLocation, memtype, rdata[0]);
        if (status < 0) {
            throw std::runtime_error("cannot obtain attribute " + attributeName);
        }

        std::string data(rdata[0]);

        // cleanup
        free(rdata[0]);
        free(rdata);

        H5Aclose(attLocation);
        H5Tclose(memtype);
        H5Sclose(space);
        H5Tclose(filetype);

        return data;
    }

    /**
     * Gets the raw data as a flattened array
     * @tparam T
     * @return
     */
    template <typename T>
    std::vector<T> RawData() const {
        if (Type() != H5O_TYPE_DATASET) {
            throw std::runtime_error("Shape can only be called on H5O_TYPE_DATASET objects");
        }

        // get the object as a dataspace
        auto dataspace = H5Dget_space(locId); /* dataspace handle */
        const auto size = H5Sget_simple_extent_npoints(dataspace);

        // get the requestedType
        auto requestedType = nativeHdfTypes[typeid(T)];

        // prepare data vector
        std::vector<T> data(size);

        auto status = H5Dread(locId, requestedType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0]);
        if (status < 0) {
            throw std::runtime_error("cannot obtain raw data for " + name);
        }

        H5Sclose(dataspace);

        return data;
    }

    /**
     * Provide simple function for outputing to stream
     * @param os
     * @param dt
     * @return
     */
    friend std::ostream &operator<<(std::ostream &os, const HdfObject &object);
};

}  // namespace xdmfGenerator

#endif  // XDMFGENERATOR_HDFOBJECT_HPP
