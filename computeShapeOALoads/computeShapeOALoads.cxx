#include "StatisticalModel.h"
#include "vtkPolyData.h"
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include "vtkStandardMeshRepresenter.h"
#include "vtkPolyDataReader.h"
#include "computeShapeOALoadsCLP.h"
#include <fstream>
#include "vtkSmartPointer.h"

using namespace statismo;
typedef vtkStandardMeshRepresenter RepresenterType;
typedef StatisticalModel<vtkPolyData> StatisticalModelType;

int main(int argc, char ** argv)
{
    PARSE_ARGS;

    // Load h5 file
    RepresenterType* representer = RepresenterType::Create();
    boost::scoped_ptr<StatisticalModelType> model(StatisticalModelType::Load(representer, modelfile));

    // Load VTK data into program
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(vtkfile.c_str());
    reader->Update();
    vtkSmartPointer<vtkPolyData> VTKShape = vtkSmartPointer<vtkPolyData>::New();
    VTKShape->ShallowCopy(reader->GetOutput());

    // Compute shape loads for current training model
    VectorType ShapeOAVectorLoads = model->ComputeCoefficientsForDataset(VTKShape);
    std::cout<<ShapeOAVectorLoads.size()<<std::endl;

    // Store the shape loads in a csv file
    ofstream myfile;
    std::string csvfile = resultdir + "/ShapeOAVectorLoadsG" + std::to_string(groupnumber) + ".csv";
    myfile.open(csvfile);
    myfile << "ShapeOALoads" << std::endl;
    for(int i = 0; i < ShapeOAVectorLoads.size(); i++)
    {
        myfile << ShapeOAVectorLoads[i] << std::endl;
    }
    myfile.close();

    return 0;
}
