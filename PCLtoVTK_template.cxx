// STL
#include <iostream>
#include <vector>

// PCL
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

// VTK
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>

//Some shorthand notation
typedef pcl::PointCloud<pcl::PointXYZRGB>::Ptr          ColorCloudPtr;
typedef pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr    ColorCloudNormalPtr;
typedef vtkSmartPointer<vtkPoints>                      VTKPointsPtr;
typedef vtkSmartPointer<vtkPolyData>                    VTKPolyDataPtr;

//Template function declarations for inserting points of arbitrary dimension
template <typename PointT> 
void PCLtoVTK(typename pcl::PointCloud<PointT>::Ptr cloud, VTKPolyDataPtr pdata)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for (size_t i = 0; i < cloud->points.size (); ++i)
    {
    points->InsertNextPoint ( cloud->points[i].x, cloud->points[i].y, cloud->points[i].z );
    }
    //Add the points to the polydata
    pdata->SetPoints(points);
}

//Specialization for points with RGB values
template <>
void PCLtoVTK<pcl::PointXYZRGB> (ColorCloudPtr cloud, VTKPolyDataPtr pdata)  
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkFloatArray> rgbs = vtkSmartPointer<vtkFloatArray>::New();
  rgbs->SetNumberOfComponents(1);
  rgbs->SetName("RGB");

  for (size_t i = 0; i < cloud->points.size (); ++i)
    {
    points->InsertNextPoint ( cloud->points[i].x, cloud->points[i].y, cloud->points[i].z);
	rgbs->InsertNextValue(cloud->points[i].rgb);
    }
  //Add points to the polydata and add the RGB array to it
  pdata->SetPoints(points);
  pdata->GetPointData()->AddArray(rgbs);
}

//Specialization for points with RGB values and normals
template <> 
void PCLtoVTK<pcl::PointXYZRGBNormal> (ColorCloudNormalPtr cloud, VTKPolyDataPtr pdata)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkFloatArray> rgbs = vtkSmartPointer<vtkFloatArray>::New();
  rgbs->SetNumberOfComponents(1);
  rgbs->SetName("RGB");

  vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New();
  normals->SetNumberOfComponents(3); //3d normals (ie x,y,z)
  normals->SetNumberOfTuples(cloud->points.size());

  for (size_t i = 0; i < cloud->points.size (); ++i)
    {
    points->InsertNextPoint ( cloud->points[i].x, cloud->points[i].y, cloud->points[i].z);
    rgbs->InsertNextValue(cloud->points[i].rgb);
    float norm_tuple[3] = {cloud->points[i].normal_x, cloud->points[i].normal_y, cloud->points[i].normal_z};
    normals->SetTuple(i, norm_tuple);
    }
  //Add the points to the polydata and add the RGB array and normals to it
  pdata->SetPoints(points);
  pdata->GetPointData()->AddArray(rgbs);
  pdata->GetPointData()->SetNormals(normals);
}

int main (int argc, char*argv[])
{
  // Verify arguments
  if(argc < 3)
    {
    std::cerr << "Required arguments: input.pcd output.vtp" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string inputFileName = argv[1];
  std::string outputFileName = argv[2];
  
  // Output arguments
  std::cout << "Reading " << inputFileName << " and writing " << outputFileName << std::endl;
  
  // Read the PCD file
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  std::cout << pcl::getFieldsList<pcl::PointXYZ>(*cloud);

  if (pcl::io::loadPCDFile<pcl::PointXYZ> (inputFileName.c_str(), *cloud) == -1) //* load the file
  {
    PCL_ERROR ("Couldn't read file \n");
    return EXIT_FAILURE;
  }

  // Create a polydata object and add the points to it.
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

  PCLtoVTK<pcl::PointXYZ>(cloud, polydata);
 
  // Write the file
  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetFileName(outputFileName.c_str());
  writer->SetInputConnection(polydata->GetProducerPort());
  writer->Write();
  
  return EXIT_SUCCESS;
}
