CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(DiagnosticIndexCLI)


# saveModel: allow to create a shape model with a list of vtk files
option(saveModel "saveModel executable" ON)
if (saveModel)
  add_subdirectory(saveModel)
endif ()

# computeShapeOALoads: Compute of shape loads of a sample for the current shape model 
option(computeShapeOALoads "computeShapeOALoads executable" ON)
if (computeShapeOALoads)
  add_subdirectory(computeShapeOALoads)
endif ()

# computeMean: Compute the mean from a HDF5 file 
option(computeMean "computeMean executable" ON)
if (computeMean)
  add_subdirectory(computeMean)
endif ()