static char help[] = "Command line executable to generate xdmf file hdf5 visualization. \n";

#include <petsc.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include "xdmfBuilder.hpp"

int main(int argc, char **args) {
    // initialize petsc and mpi
    PetscErrorCode err = PetscInitialize(&argc, &args, NULL, help);
    CHKERRQ(err);

    // check to see if we should print options
    char filename[PETSC_MAX_PATH_LEN] = "";
    PetscBool fileSpecified = PETSC_FALSE;
    PetscOptionsGetString(NULL, NULL, "--input", filename, PETSC_MAX_PATH_LEN, &fileSpecified);
    CHKERRQ(err);
    if (!fileSpecified) {
        throw std::invalid_argument("the hdf5 file must be specified using the --input flag");
    }

    std::filesystem::path filePath(filename);
    if (!std::filesystem::exists(filePath)) {
        throw std::invalid_argument("unable to locate input file: " + filePath.string());
    }

    // prepare
    auto root = std::make_shared<petscXdmfGenerator::HdfObject>(filePath);
    auto builder = petscXdmfGenerator::XdmfBuilder::FromPetscHdf(root);
    auto xml = builder->Build();

    // build the path to the output file
    std::filesystem::path outputFile = filePath.parent_path();
    outputFile /= (filePath.stem().string() + ".xmf");

    // write to the file
    std::ofstream xmlFile;
    xmlFile.open(outputFile);
    xml->PrettyPrint(xmlFile);
    xmlFile.close();

    std::cout << "XDMF file written to " << outputFile << std::endl;

    return PetscFinalize();
}
