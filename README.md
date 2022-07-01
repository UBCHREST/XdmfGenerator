# Xdmf Generator
Simple library that can be used in other projects to generate the Xdmf files for PETSc generated hdf5 files.  This library requires that a PETSc configure (built with hdf5) can be found in the package configuration path.  The project also generates a self-contained command line executable.

## Running Tests Locally
The tests can be run locally using an IDE or cmake directly (ctest command).  You may also use the ```--keepOutputFile=true```  command line argument to preserve output files.  To run the tests using the testing environment (docker), first make sure that [Docker](https://www.docker.com) installed.

```bash
# Login to github to access base image (follow prompt instructions)
docker login ghcr.io

# Build the docker testing image
docker build -t testing_image -f DockerTestFile .

# Run the built tests and view results
docker run --rm testing_image 

```