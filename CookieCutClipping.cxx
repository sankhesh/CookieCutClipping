#include <vtkActor.h>
#include <vtkBandedPolyDataContourFilter.h>
#include <vtkCamera.h>
#include <vtkGlyph2D.h>
#include <vtkGlyphSource2D.h>
#include <vtkNew.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkPointData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkCellData.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkThreshold.h>
#include <vtkDataSetMapper.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkLinearSubdivisionFilter.h>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <input vtp file> " << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkXMLPolyDataReader> reader;
  reader->SetFileName(argv[1]);

  vtkNew<vtkBandedPolyDataContourFilter> contour;
  contour->SetInputConnection(reader->GetOutputPort());
  contour->SetNumberOfContours(2);
  contour->SetValue(0, 20);
  contour->SetValue(1, 50);
  contour->ClippingOn();
  contour->SetClipTolerance(0.);
  contour->Update();

  vtkNew<vtkPolyDataConnectivityFilter> connectivity;
  connectivity->SetInputConnection(contour->GetOutputPort());
  connectivity->SetExtractionModeToAllRegions();
  connectivity->ColorRegionsOn();
  connectivity->Update();

  //vtkNew<vtkThreshold> threshold;
  //threshold->SetInputConnection(connectivity->GetOutputPort());
  //threshold->ThresholdBetween(58, 58);
  vtkNew<vtkPolyDataConnectivityFilter> extract;
  extract->SetInputConnection(connectivity->GetOutputPort());
  extract->ScalarConnectivityOn();
  extract->SetScalarRange(58, 58);
  extract->FullScalarConnectivityOn();

  vtkNew<vtkLinearExtrusionFilter> extrude;
  extrude->SetExtrusionTypeToNormalExtrusion();
  extrude->SetScaleFactor(1.0);
  extrude->SetInputConnection(extract->GetOutputPort());

  vtkPolyData* pd = contour->GetOutput();
  double bounds[6];
  pd->GetBounds(bounds);

  vtkNew<vtkPlaneSource> plane;
  plane->SetOrigin(bounds[0], bounds[2], 0.0);
  plane->SetPoint1(bounds[1], bounds[2], 0.0);
  plane->SetPoint2(bounds[0], bounds[3], 0.0);
  plane->SetXResolution(30);
  plane->SetYResolution(30);

  vtkNew<vtkGlyphSource2D> source;
  source->SetGlyphTypeToThickCross();
  source->FilledOn();
  source->SetScale(6.4);

  vtkNew<vtkGlyph2D> glyph;
  glyph->SetInputConnection(plane->GetOutputPort());
  glyph->SetSourceConnection(source->GetOutputPort());

  vtkNew<vtkTriangleFilter> triangulate;
  triangulate->SetInputConnection(glyph->GetOutputPort());
  vtkNew<vtkLinearExtrusionFilter> extrude1;
  extrude1->SetExtrusionTypeToNormalExtrusion();
  extrude1->SetScaleFactor(1.0);
  extrude1->SetInputConnection(glyph->GetOutputPort());

  vtkNew<vtkLinearSubdivisionFilter> subdivide;
  subdivide->SetInputConnection(triangulate->GetOutputPort());

  vtkNew<vtkBooleanOperationPolyDataFilter> boolean;
  boolean->SetInputConnection(0, subdivide->GetOutputPort());
  boolean->SetInputConnection(1, extract->GetOutputPort());
  boolean->SetOperationToIntersection();

  //vtkNew<vtkDataSetMapper> mapper;
  //mapper->SetInputConnection(threshold->GetOutputPort());
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(subdivide->GetOutputPort());
  mapper->SetInterpolateScalarsBeforeMapping(1);
  mapper->SetColorModeToMapScalars();
  mapper->SetScalarRange(0, connectivity->GetNumberOfExtractedRegions());

  vtkNew<vtkPolyDataMapper> pointsMapper;
  pointsMapper->SetInputConnection(boolean->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper.GetPointer());
  vtkNew<vtkActor> pointsActor;
  pointsActor->SetMapper(pointsMapper.GetPointer());
  //pointsActor->GetProperty()->SetRepresentationToWireframe();

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(601, 600); // Intentional NPOT size
  //renWin->SetNumberOfLayers(2);

  vtkNew<vtkRenderer> ren;
  renWin->AddRenderer(ren.GetPointer());
  //ren->SetLayer(0);
  //vtkNew<vtkRenderer> pointsRen;
  //renWin->AddRenderer(pointsRen.GetPointer());
  //pointsRen->SetLayer(1);

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  //ren->AddActor(actor.GetPointer());
  ren->AddActor(pointsActor.GetPointer());
  ren->ResetCamera();
  //pointsRen->AddActor(pointsActor.GetPointer());
  //pointsRen->ResetCamera();

  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
