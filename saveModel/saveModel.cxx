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
#include <libgen.h>
#include <vtkPolyDataWriter.h>
#include <string.h>
#include <vtkPolyData.h>

#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>


using namespace statismo;
typedef vtkStandardMeshRepresenter RepresenterType;
typedef DataManager<vtkPolyData> DataManagerType;
typedef PCAModelBuilder<vtkPolyData> ModelBuilderType;
typedef StatisticalModel<vtkPolyData> StatisticalModelType;


vtkSmartPointer<vtkPolyData> moveToOrigin(vtkSmartPointer<vtkPolyData> polydata)
{
    // Sum up Original Points
    double sum[3];
    sum[0] = 0;
    sum[1] = 0;
    sum[2] = 0;

    for (int i = 0; i < polydata->GetNumberOfPoints(); i++)
    {
        double curPoint[3];
        polydata->GetPoint(i, curPoint);
        for( unsigned int dim = 0; dim < 3; dim++ )
        {
            sum[dim] += curPoint[dim];

        }
    }

    //Calculate MC
    double MC[3];
    for( unsigned int dim = 0; dim < 3; dim++ )
    {
        MC[dim] = (double) sum[dim] / (polydata->GetNumberOfPoints() + 1);
    }

    // Create a New Point Set "sftpoints" with the Shifted Values
//    vtkSmartPointer<vtkPoints> sftpoints = vtkSmartPointer<vtkPoints>::New();
//    sftpoints = polydata->GetPoints();

//    for( int pointID = 0; pointID < polydata->GetNumberOfPoints(); pointID++ )
//    {
//        double curPoint[3];
//        polydata->GetPoint(pointID, curPoint);
//        double sftPoint[3];
//        for( unsigned int dim = 0; dim < 3; dim++ )
//        {
//            sftPoint[dim] = curPoint[dim] - MC[dim];
//        }
//        sftpoints->SetPoint(pointID, sftPoint[0], sftPoint[1], sftPoint[2]);
//    }
//    // Set the Shifted Points back to original mesh
//    polydata->SetPoints(sftpoints);
    // Save the new mesh in resultdir

    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Identity();
    transform->Translate(-MC[0], -MC[1], -MC[2]);

    vtkSmartPointer<vtkTransformPolyDataFilter> transformfilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformfilter->SetInputData(polydata);
    transformfilter->SetTransform(transform);

    transformfilter->Update();

    return transformfilter->GetOutput();
}


std::string saveVTKFile(vtkSmartPointer<vtkPolyData> polydata, const std::string& filepath, std::string resultdir)
{
    std::string filename;
    filename = resultdir + filepath.substr(filepath.find_last_of('/'));

    vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
    writer->SetInputData(polydata);
    writer->SetFileName(filename.c_str());
    writer->Write();

    return filename;
}

vtkSmartPointer<vtkPolyData> loadVTKPolyData(const std::string& filename) {
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    return reader->GetOutput();
}

int main(int argc, char ** argv)
{
    PARSE_ARGS;

    if(argc < 7)
    {
        std::cout << "Usage " << argv[0] << " [--groupnumber <int>] [--vtkfilelist <std::vector<std::string>>] [--resultdir <std::string>]" << std::endl;
        return 1;
    }

    // Move to Origin and save the vtk files
    std::vector< vtkSmartPointer<vtkPolyData> > polydatalist;
    std::vector< std::string > filepathlist;
    vtkSmartPointer<vtkPolyData> polydata0 = loadVTKPolyData(vtkfilelist[0]);
    vtkSmartPointer<vtkPolyData> polydata0MovedToOrigin = moveToOrigin(polydata0);
    std::string filepath0 = saveVTKFile(polydata0MovedToOrigin, vtkfilelist[0], resultdir);
    polydatalist.push_back(polydata0MovedToOrigin);
    filepathlist.push_back(filepath0);
    int numPts = polydata0MovedToOrigin->GetPoints()->GetNumberOfPoints();
    for(int meshID = 1; meshID < vtkfilelist.size(); meshID++)
    {
        vtkSmartPointer<vtkPolyData> polydata = loadVTKPolyData(vtkfilelist[meshID]);
        vtkSmartPointer<vtkPolyData> polydataMovedToOrigin = moveToOrigin(polydata);
        std::string filepath = saveVTKFile(polydataMovedToOrigin, vtkfilelist[meshID], resultdir);
        polydatalist.push_back(polydataMovedToOrigin);
        filepathlist.push_back(filepath);
    }


    // Average of all the VTK meshes
    //   Mean of the 3 coordinates
    vtkSmartPointer<vtkCellArray> verts = polydatalist[0]->GetVerts();
    vtkSmartPointer<vtkCellArray> lines = polydatalist[0]->GetLines();
    vtkSmartPointer<vtkCellArray> polys = polydatalist[0]->GetPolys();
    vtkSmartPointer<vtkCellArray> strips = polydatalist[0]->GetStrips();

    vtkSmartPointer<vtkPolyData> polydata_MeanGroup = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points = polydata0MovedToOrigin->GetPoints();
    for(int meshID = 1; meshID < vtkfilelist.size(); meshID++)
    {
        for(int ptID = 0; ptID < numPts; ptID++)
        {
            double coord[3];
            polydatalist[meshID]->GetPoint(ptID, coord);
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
        dataManager->AddDataset(polydatalist[j], filepathlist[j]);
    }
    boost::scoped_ptr<ModelBuilderType> modelBuilder(ModelBuilderType::Create());
    boost::scoped_ptr<StatisticalModelType> model(modelBuilder->BuildNewModel(dataManager->GetData(), 0.01));

    // Once we have built the model, we can save in the directory choosen by the user.
    std::string H5File = resultdir + "/G" + std::to_string(groupnumber) + ".h5";
    model->Save(H5File);
    std::cout << "Successfully saved shape model as " << "G" << groupnumber << ".h5" << std::endl;

    return 0;
}
