CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(saveModel)

FIND_PACKAGE(statismo REQUIRED)

# Slicer Execution Model
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})
include(${GenerateCLP_USE_FILE})

# VTK
FIND_PACKAGE(VTK REQUIRED)
include (${VTK_USE_FILE})

# Statismo
include_directories(${statismo_INCLUDE_DIRS})
link_directories(${VTK_LIBRARY_DIRS} ${statismo_LIBRARY_DIRS})

generateCLP(saveModel.cxx saveModel.xml)

SEMMacroBuildCLI(
    NAME saveModel
    EXECUTABLE_ONLY
    TARGET_LIBRARIES ${VTK_LIBRARIES} ${statismo_LIBRARIES}
)









