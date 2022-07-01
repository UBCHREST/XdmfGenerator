#ifndef PETSCXDMFGENERATOR_XMLELEMENT_HPP
#define PETSCXDMFGENERATOR_XMLELEMENT_HPP

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace xdmfGenerator {

class XmlElement {
   private:
    std::string preamble;
    std::string name;
    std::string value;
    std::string path;
    std::map<std::string, std::string> attributes;
    std::vector<std::unique_ptr<XmlElement>> elements;

   public:
    XmlElement(const XmlElement&) = delete;
    void operator=(const XmlElement&) = delete;

    explicit XmlElement(std::string name, std::string preamble = "", std::string currentPath = "");

    /**
     * creates a new child element with the name
     * @param childName
     * @return
     */
    XmlElement& operator[](const std::string&& childName);

    /**
     * Gets the element at the index
     * @param name
     * @return
     */
    XmlElement& operator[](const std::size_t&& index);

    /**
     * gets/sets the attribute
     * @param childName
     * @return
     */
    std::string& operator()(const std::string&& childName);

    /**
     * gets/sets the value for the element
     * @param name
     * @return
     */
    std::string& operator()();

    /**
     * Get the path to this element
     * @return
     */
    const std::string& Path() const { return path; }

    /**
     * prints the xml object to the stream
     * @param os
     * @param object
     * @return
     */
    friend std::ostream& operator<<(std::ostream& os, const XmlElement& object);

    /**
     * prints the xml object to the stream
     * @param os
     * @param object
     * @return
     */
    void PrettyPrint(std::ostream& stream, size_t depth = 0);
};

}  // namespace xdmfGenerator

#endif  // PETSCXDMFGENERATOR_XMLELEMENT_HPP
