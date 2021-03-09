#ifndef PETSCXDMFGENERATOR_OBJECT_HPP
#define PETSCXDMFGENERATOR_OBJECT_HPP

#include <hdf5.h>
#include <string>
#include <iostream>

namespace petscXdmfGenerator{
class Object : public std::enable_shared_from_this<Object>{
private:
    const H5O_info_t information;
    const std::shared_ptr<Object> parent = nullptr;
    const std::string name = "";
    hid_t locId;

protected:
    Object(std::shared_ptr<Object> parent, H5O_info_t information, std::string name);

public:
    Object(hid_t file, H5O_info_t information);
    ~Object();

    inline H5O_type_t Type() const{
        return information.type;
    }

    std::string TypeName() const;

    /**
     * Simple function to get child node
     * @param name
     * @return
     */
    std::shared_ptr<Object> Get(std::string name);

    /**
     * Provide simple function for outputing to stream
     * @param os
     * @param dt
     * @return
     */
    friend std::ostream& operator<<(std::ostream& os, const Object& object);

};

}


#endif //PETSCXDMFGENERATOR_OBJECT_HPP
