#include "xmlElement.hpp"

xdmfGenerator::XmlElement::XmlElement(std::string name, std::string preamble, std::string currentPath) : preamble(preamble), name(name), path(currentPath + "/" + name) {}

namespace xdmfGenerator {
std::ostream& operator<<(std::ostream& os, const xdmfGenerator::XmlElement& object) {
    os << object.preamble;
    os << "<" << object.name;
    for (const auto& attribute : object.attributes) {
        os << " " << attribute.first << "=\"" << attribute.second << "\"";
    }
    os << ">";
    os << object.value;
    for (const auto& element : object.elements) {
        os << *(element);
    }
    os << "</" << object.name << ">";

    return os;
}
}  // namespace xdmfGenerator

xdmfGenerator::XmlElement& xdmfGenerator::XmlElement::operator[](const std::string&& childName) {
    elements.push_back(std::make_unique<XmlElement>(childName, "", path));
    return *elements[elements.size() - 1];
}

xdmfGenerator::XmlElement& xdmfGenerator::XmlElement::operator[](const size_t&& index) { return *elements[index]; }

std::string& xdmfGenerator::XmlElement::operator()(const std::string&& childName) { return attributes[childName]; }

std::string& xdmfGenerator::XmlElement::operator()() { return value; }

void xdmfGenerator::XmlElement::PrettyPrint(std::ostream& stream, size_t depth) {
    const auto t = std::string(2, ' ');
    const auto d = std::string(depth * 2, ' ');

    if (preamble.size() > 0) {
        stream << d << preamble;
    }
    stream << std::endl << d << "<" << name;
    for (const auto& attribute : attributes) {
        stream << " " << attribute.first << "=\"" << attribute.second << "\"";
    }
    stream << ">";

    if (!value.empty()) {
        stream << std::endl << d << t << value;
    }
    for (const auto& element : elements) {
        element->PrettyPrint(stream, depth + 1);
    }

    stream << std::endl << d << "</" << name << ">";
}
