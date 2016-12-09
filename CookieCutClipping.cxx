#include <vtkActor.h>
#include <vtkBandedPolyDataContourFilter.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkCookieCutter.h>
#include <vtkFeatureEdges.h>
#include <vtkGlyph2D.h>
#include <vtkGlyphSource2D.h>
#include <vtkNew.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkContourLoopExtraction.h>

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

  vtkNew<vtkPolyDataConnectivityFilter> extract;
  extract->SetInputConnection(connectivity->GetOutputPort());
  extract->ScalarConnectivityOn();
  extract->SetScalarRange(58, 58);
  //extract->SetScalarRange(6, 6);

  vtkNew<vtkFeatureEdges> edge;
  edge->SetInputConnection(extract->GetOutputPort());
  edge->BoundaryEdgesOn();
  edge->FeatureEdgesOff();
  edge->ManifoldEdgesOff();
  edge->NonManifoldEdgesOff();

  vtkNew<vtkContourLoopExtraction> extractLoop;
  extractLoop->SetInputConnection(edge->GetOutputPort());
  extractLoop->Update();

  vtkPolyData* pd = contour->GetOutput();
  double bounds[6];
  pd->GetBounds(bounds);

  vtkNew<vtkPlaneSource> plane;
  plane->SetOrigin(bounds[0], bounds[2], 0.0);
  plane->SetPoint1(bounds[1], bounds[2], 0.0);
  plane->SetPoint2(bounds[0], bounds[3], 0.0);
  plane->SetXResolution(100);
  plane->SetYResolution(60);

  vtkNew<vtkGlyphSource2D> source;
  source->SetGlyphTypeToCircle();
  source->FilledOn();
  source->SetScale(2.0);

  vtkNew<vtkGlyph2D> glyph;
  glyph->SetInputConnection(plane->GetOutputPort());
  glyph->SetSourceConnection(source->GetOutputPort());

  vtkNew<vtkCookieCutter> cutter;
  cutter->SetInputConnection(glyph->GetOutputPort());
  cutter->SetLoopsData(extractLoop->GetOutput());

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(edge->GetOutputPort());
  mapper->SetInterpolateScalarsBeforeMapping(1);
  mapper->SetColorModeToMapScalars();
  mapper->SetScalarRange(0, connectivity->GetNumberOfExtractedRegions());

  vtkNew<vtkPolyDataMapper> pointsMapper;
  pointsMapper->SetInputConnection(cutter->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper.GetPointer());
  vtkNew<vtkActor> pointsActor;
  pointsActor->SetMapper(pointsMapper.GetPointer());

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(601, 600); // Intentional NPOT size

  vtkNew<vtkRenderer> ren;
  renWin->AddRenderer(ren.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  ren->AddActor(actor.GetPointer());
  ren->AddActor(pointsActor.GetPointer());
  ren->ResetCamera();

  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
