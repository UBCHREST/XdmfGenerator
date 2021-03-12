#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "xdmfBuilder.hpp"

class PETScHdf5ToXdmfTestFixture : public ::testing::TestWithParam<std::string> {
   protected:
    std::filesystem::path inputFilePath = "inputs";
    std::filesystem::path expectedOutputFilePath = "outputs";

   public:
    void SetUp() {
        inputFilePath /= (GetParam() + ".hdf5");
        expectedOutputFilePath /= (GetParam() + ".xmf");
    }
};

TEST_P(PETScHdf5ToXdmfTestFixture, ShouldGenerateExpectedXml) {
    // arrange
    auto hdfObject = std::make_shared<petscXdmfGenerator::HdfObject>(inputFilePath);
    auto specification = petscXdmfGenerator::XdmfSpecification::FromPetscHdf(hdfObject);
    auto builder = petscXdmfGenerator::XdmfBuilder(specification);
    // read in the expectedResults
    std::ifstream expectedResultFile(expectedOutputFilePath);
    std::stringstream expectedOutput;
    expectedOutput << expectedResultFile.rdbuf();

    std::stringstream resultStream;

    // act
    auto xml = builder.Build();
    xml->PrettyPrint(resultStream);

    // assert
    ASSERT_EQ(resultStream.str(), expectedOutput.str());
}

INSTANTIATE_TEST_SUITE_P(Tests, PETScHdf5ToXdmfTestFixture, ::testing::Values("flowField.0", "steadyState.0", "swarm.static.mesh.0"));