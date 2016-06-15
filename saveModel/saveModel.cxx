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
#include "vtkCellArray.h"

using namespace statismo;
typedef vtkStandardMeshRepresenter RepresenterType;
typedef DataManager<vtkPolyData> DataManagerType;
typedef PCAModelBuilder<vtkPolyData> ModelBuilderType;
typedef StatisticalModel<vtkPolyData> StatisticalModelType;

vtkSmartPointer<vtkPolyData> moveToOrigin(vtkSmartPointer<vtkPolyData> mesh)
{
    // Sum up Original Points
    double sum[3];
    sum[0] = 0;
    sum[1] = 0;
    sum[2] = 0;

    for (int i = 0; i < mesh->GetNumberOfPoints(); i++)
    {
        double curPoint[3];
        mesh->GetPoint(i, curPoint);
        for( unsigned int dim = 0; dim < 3; dim++ )
        {
            sum[dim] += curPoint[dim];

        }
    }

    //Calculate MC
    double MC[3];
    for( unsigned int dim = 0; dim < 3; dim++ )
    {
        MC[dim] = (double) sum[dim] / (mesh->GetNumberOfPoints() + 1);
    }

    // Create a New Point Set "sftpoints" with the Shifted Values
    vtkSmartPointer<vtkPoints> sftpoints = vtkSmartPointer<vtkPoints>::New();
    sftpoints = mesh->GetPoints();

    for( int pointID = 0; pointID < mesh->GetNumberOfPoints(); pointID++ )
    {
        double curPoint[3];
        mesh->GetPoint(pointID, curPoint);
        double sftPoint[3];
        for( unsigned int dim = 0; dim < 3; dim++ )
        {
            sftPoint[dim] = curPoint[dim] - MC[dim];
        }
        sftpoints->SetPoint(pointID, sftPoint);
    }
    // Set the Shifted Points back to original mesh
    mesh->SetPoints(sftpoints);

    return mesh;
}

vtkSmartPointer<vtkPolyData> loadVTKPolyData(const std::string& filename) {
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    pd->ShallowCopy(reader->GetOutput());
    return pd;
}

int main(int argc, char ** argv)
{
    PARSE_ARGS;

    if(argc < 7)
    {
        std::cout << "Usage " << argv[0] << " [--groupnumber <int>] [--vtkfilelist <std::vector<std::string>>] [--resultdir <std::string>]" << std::endl;
        return 1;
    }

    // Average of all the VTK meshes
    //   Mean of the 3 coordinates
    vtkSmartPointer<vtkPolyData> polydata0 = loadVTKPolyData(vtkfilelist[0]);
    int numPts = polydata0->GetPoints()->GetNumberOfPoints();
    vtkSmartPointer<vtkCellArray> verts = polydata0->GetVerts();
    vtkSmartPointer<vtkCellArray> lines = polydata0->GetLines();
    vtkSmartPointer<vtkCellArray> polys = polydata0->GetPolys();
    vtkSmartPointer<vtkCellArray> strips = polydata0->GetStrips();

    vtkSmartPointer<vtkPolyData> polydata_MeanGroup = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points = polydata0->GetPoints();
    for(int meshID = 1; meshID < vtkfilelist.size(); meshID++)
    {
        vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

        polydata = loadVTKPolyData(vtkfilelist[meshID]);
        vtkSmartPointer<vtkPolyData> polydataMovedToOrigin = moveToOrigin(polydata);

        for(int ptID = 0; ptID < numPts; ptID++)
        {
            double coord[3];
            polydataMovedToOrigin->GetPoint(ptID, coord);
            double sum[3];
            points->GetPoint(ptID, sum);
            for(int dim = 0; dim < 3; dim++)
            {
                sum[dim] = sum[dim] + coord[dim];
            }
            points->InsertPoint(ptID, sum);
        }
    }
    double mean[3];
    for(int ptID = 0; ptID < numPts; ptID++)
    {
        for(int dim = 0; dim < 3; dim++)
        {
            double sum[3];
            points->GetPoint(ptID, sum);
            mean[dim] = sum[dim]/vtkfilelist.size();
        }
        points->InsertPoint(ptID, mean);
    }

    //   Creation of the polydata of the mean of the given group
    polydata_MeanGroup->SetPoints(points);
    polydata_MeanGroup->SetVerts(verts);
    polydata_MeanGroup->SetLines(lines);
    polydata_MeanGroup->SetPolys(polys);
    polydata_MeanGroup->SetStrips(strips);

    // Creation of the shape model
    vtkSmartPointer<vtkPolyData> reference = polydata_MeanGroup;

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
    std::cout << "Successfully saved shape model as " << "G" << groupnumber << ".h5" << std::endl;

    return 0;
}
