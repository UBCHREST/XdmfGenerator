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

## xdmfGenerator Executable
When using the xdmfGenerator executable the following command line arguments be used.

 - Single HDF5 file: This option is for a single hdf5 file that contains one or more time steps." << std::endl;
 - Directory: This option is for a multiple hdf5 files inside a directory.  Only hdf5/hdf files will be used." << std::endl; 
   - Optional Arguments:" << std::endl; 
     - +n : Starts processing directory hdf5 files at item n" << std::endl; 
     - -n : Stop processing directory hdf5 files at total items - n" << std::endl; 
     - ~n : Only process every n files" << std::endl;


### Example
The following example skips the first 3 files, omits the last file, and only processes every 5 files.
```bash
xdmfGenerator /path/to/output/dir +3 -1 ~5 
```