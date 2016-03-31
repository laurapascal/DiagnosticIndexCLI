#include "PCAModelBuilder.h"
#include "StatisticalModel.h"
#include "vtkPolyData.h"
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include "vtkStandardMeshRepresenter.h"
#include "DataManager.h"
#include "vtkPolyDataReader.h"
#include "saveModelCLP.h"
#include <fstream>

using namespace statismo;
typedef vtkStandardMeshRepresenter RepresenterType;
typedef DataManager<vtkPolyData> DataManagerType;
typedef PCAModelBuilder<vtkPolyData> ModelBuilderType;
typedef StatisticalModel<vtkPolyData> StatisticalModelType;

vtkPolyData* loadVTKPolyData(const std::string& filename) {
    vtkPolyDataReader* reader = vtkPolyDataReader::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    vtkPolyData* pd = vtkPolyData::New();
    pd->ShallowCopy(reader->GetOutput());
    return pd;
}

int main(int argc, char ** argv)
{
    PARSE_ARGS;

    vtkSmartPointer<vtkPolyData> reference = loadVTKPolyData(vtkfilelist[0]);
    boost::scoped_ptr<RepresenterType> representer(RepresenterType::Create(reference));
    boost::scoped_ptr<DataManagerType> dataManager(DataManagerType::Create(representer.get()));
    for(int j = 0; j < vtkfilelist.size(); j++)
    {
        dataManager->AddDataset(loadVTKPolyData(vtkfilelist[j]), vtkfilelist[j]);
    }
    boost::scoped_ptr<ModelBuilderType> modelBuilder(ModelBuilderType::Create());
    boost::scoped_ptr<StatisticalModelType> model(modelBuilder->BuildNewModel(dataManager->GetData(), 0.01));
    
    // Once we have built the model, we can save in the directory choosen by the user.
    std::string H5File = resultdir + "/G" + std::to_string(groupnumber) + ".h5";
    model->Save(H5File);
    std::cout << "Successfully saved shape model as " << "model.h5" << std::endl;
    
    return 0;
}
