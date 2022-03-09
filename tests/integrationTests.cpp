#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "generators.hpp"

class SingleHdf5FileTextFixture : public ::testing::TestWithParam<std::string> {
   protected:
    std::filesystem::path inputFilePath = "inputs";
    std::filesystem::path expectedOutputFilePath = "outputs";

   public:
    void SetUp() {
        inputFilePath /= (GetParam() + ".hdf5");
        expectedOutputFilePath /= (GetParam() + ".xmf");
    }
};

TEST_P(SingleHdf5FileTextFixture, ShouldGenerateExpectedXml) {
    // arrange
    // read in the expectedResults
    std::ifstream expectedResultFile(expectedOutputFilePath);
    std::stringstream expectedOutput;
    expectedOutput << expectedResultFile.rdbuf();

    std::stringstream resultStream;

    // act
    petscXdmfGenerator::Generate(inputFilePath, resultStream);

    // assert
    ASSERT_EQ(resultStream.str(), expectedOutput.str());
}

INSTANTIATE_TEST_SUITE_P(Tests, SingleHdf5FileTextFixture,
                         ::testing::Values("flowField.0", "steadyState.0", "swarmStaticMesh.0", "flowWithParticles.0", "particlesOnly.0", "particlesDynamic3D", "flowWithMultipleComponents",
                                           "particleWithExtraFields", "flowWithComponentNames"));

class MultiHdf5FileTextFixture : public ::testing::TestWithParam<std::string> {
   protected:
    std::filesystem::path inputDirPath = "inputs";
    std::filesystem::path expectedOutputFilePath = "outputs";

   public:
    void SetUp() {
        inputDirPath /= GetParam();
        expectedOutputFilePath /= (GetParam() + ".xmf");
    }
};

TEST_P(MultiHdf5FileTextFixture, ShouldGenerateExpectedXml) {
    // arrange
    // read in the expectedResults
    std::ifstream expectedResultFile(expectedOutputFilePath);
    std::stringstream expectedOutput;
    expectedOutput << expectedResultFile.rdbuf();

    std::stringstream resultStream;

    // get a list of input files
    std::vector<std::filesystem::path> inputFilePaths;
    for (const auto& file : std::filesystem::directory_iterator(inputDirPath)) {
        if (file.path().extension() == ".hdf5") {
            inputFilePaths.push_back(file.path());
        }
    }
    std::sort(inputFilePaths.begin(), inputFilePaths.end());

    // act
    petscXdmfGenerator::Generate(inputFilePaths, resultStream);

    // assert
    ASSERT_EQ(resultStream.str(), expectedOutput.str());
}

INSTANTIATE_TEST_SUITE_P(Tests, MultiHdf5FileTextFixture, ::testing::Values("multiFileFlowField.0", "multiFileParticles.0"));