#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkVersion.h>
#include "StatisticalModel.h"
#include "vtkStandardMeshRepresenter.h"
#include "computeMeanCLP.h"

using namespace statismo;

void saveSample(const vtkPolyData* pd, const std::string& resdir, const std::string& basename)
{
    std::string filename = resdir +std::string("/") + basename;
    
    vtkPolyDataWriter* w = vtkPolyDataWriter::New();
#if (VTK_MAJOR_VERSION == 5 )
    w->SetInput(const_cast<vtkPolyData*>(pd));
#else
    w->SetInputData(const_cast<vtkPolyData*>(pd));
#endif
    w->SetFileName(filename.c_str());
    w->Update();
}

// illustrates how to load a shape model and the basic sampling functinality
int main(int argc, char** argv)
{
    PARSE_ARGS;

    if(argc < 7)
    {
        std::cout << "Usage " << argv[0] << " [--groupnumber <int>] [--shapemodel <std::string>] [--resultdir <std::string>] " << std::endl;
        return 1;
    }
    
    typedef vtkStandardMeshRepresenter RepresenterType;
    typedef StatisticalModel<vtkPolyData> StatisticalModelType;

    RepresenterType* representer = RepresenterType::Create();
    boost::scoped_ptr<StatisticalModelType> model(StatisticalModelType::Load(representer, shapemodel));

    // Get the model mean
    vtkPolyData* mean = model->DrawMean();
    std::string vtkFile = "meanGroup" + std::to_string(groupnumber) + ".vtk";
    saveSample(mean, resultdir, vtkFile);
    mean->Delete();

    std::cout << "Successfully saved mean as " << "meanGroup" << groupnumber << ".vtk" << std::endl;

    return 0;
}

