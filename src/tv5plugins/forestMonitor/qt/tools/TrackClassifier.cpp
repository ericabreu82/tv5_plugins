/* Copyright (C) 2014-2015 Geopixel Soluções em Geotecnologia e TI (Brazil)

This file is part of Geopixel GLUE (R)  - Geographic Leverage Universal Engine
a Framework for building GIS applications in heterogeneous environment.

GLUE is a proprietary software of Geopixel (R) and is licensed under a GLUE License.

GLUE is developed on top of TerraLib, an open source Library developed by
National Institute For Space Research (INPE) - Brazil distributed  under the
terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.
*/

/*!
\file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/TrackClassifier.cpp

\brief This class implements a concrete tool to track classifier
*/

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/LineString.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/DataSetLayer.h>
#include <terralib/maptools/MarkRendererManager.h>
#include <terralib/se/Fill.h>
#include <terralib/se/Stroke.h>
#include <terralib/se/Mark.h>
#include <terralib/se/Utils.h>
#include <terralib/qt/widgets/canvas/Canvas.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include "TrackClassifier.h"

// Qt
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

te::qt::plugins::tv5plugins::TrackClassifier::TrackClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_objIdSet(0)
{
  setCursor(cursor);
}

te::qt::plugins::tv5plugins::TrackClassifier::~TrackClassifier()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  delete m_objIdSet;
}

bool te::qt::plugins::tv5plugins::TrackClassifier::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton)
  {
    selectObjects(e);

    return true;
  }

  if (e->button() == Qt::RightButton)
  {
    if (m_objIdSet->size() < 3)
      return false;

    classifyObjects();

    return true;
  }

  return false;
}

void te::qt::plugins::tv5plugins::TrackClassifier::selectObjects(QMouseEvent* e)
{
  if (!m_coordLayer.get())
    return;

  QPointF pixelOffset(4.0, 4.0);
#if (QT_VERSION >= 0x050000)
  QRectF rect = QRectF(e->localPos() - pixelOffset, e->localPos() + pixelOffset);
#else
  QRectF rect = QRectF(e->posF() - pixelOffset, e->posF() + pixelOffset);
#endif

  // Converts rect boundary to world coordinates
  QPointF ll(rect.left(), rect.bottom());
  QPointF ur(rect.right(), rect.top());
  ll = m_display->transform(ll);
  ur = m_display->transform(ur);

  // Bulding the query box
  te::gm::Envelope envelope(ll.x(), ll.y(), ur.x(), ur.y());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_coordLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_coordLayer->getExtent()))
    return;

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  if (!schema->hasGeom())
    return;

  te::da::ObjectIdSet* oids = 0;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::auto_ptr<te::da::DataSet> dataset = m_coordLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    // Let's generate the oids
    te::da::GetEmptyOIDSet(schema.get(), oids);
    assert(oids);

    std::vector<std::string> pnames;
    te::da::GetOIDPropertyNames(schema.get(), pnames);

    // Generates a geometry from the given extent. It will be used to refine the results
    std::auto_ptr<te::gm::Geometry> geometryFromEnvelope(te::gm::GetGeomFromEnvelope(&reprojectedEnvelope, m_coordLayer->getSRID()));

    while (dataset->moveNext())
    {
      std::auto_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      if (!g->intersects(geometryFromEnvelope.get()))
        continue;

      // Feature found
      oids->add(te::da::GenerateOID(dataset.get(), pnames));
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  if (!m_objIdSet)
  {
    te::da::GetEmptyOIDSet(schema.get(), m_objIdSet);
  }

  m_objIdSet->symDifference(oids);

  drawSelecteds();

  //repaint the layer
  m_display->repaint();
}

void te::qt::plugins::tv5plugins::TrackClassifier::classifyObjects()
{
  if (!m_coordLayer.get())
    return;

  try
  {

  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error classifing track. Details:") + " %1.").arg(e.what()));
    return;
  }

  //repaint the layer
  m_display->refresh();
}

void te::qt::plugins::tv5plugins::TrackClassifier::drawSelecteds()
{
  if (!m_coordLayer.get())
    return;

  if (!m_objIdSet || m_objIdSet->size() == 0)
  {
    return;
  }

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& displayExtent = m_display->getExtent();

  te::qt::widgets::Canvas canvas(draft);
  canvas.setWindow(displayExtent.m_llx, displayExtent.m_lly, displayExtent.m_urx, displayExtent.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

  switch (gp->getGeometryType())
  {
    case te::gm::PolygonType:
    case te::gm::PolygonZType:
    case te::gm::PolygonMType:
    case te::gm::PolygonZMType:
    case te::gm::MultiPolygonType:
    case te::gm::MultiPolygonZType:
    case te::gm::MultiPolygonMType:
    case te::gm::MultiPolygonZMType:
    {
      canvas.setPolygonContourWidth(2);
      canvas.setPolygonContourColor(te::color::RGBAColor(255, 0, 0, 128));
      canvas.setPolygonFillColor(te::color::RGBAColor(255, 255, 255, 128));
    }
    break;

    case te::gm::LineStringType:
    case te::gm::LineStringZType:
    case te::gm::LineStringMType:
    case te::gm::LineStringZMType:
    case te::gm::MultiLineStringType:
    case te::gm::MultiLineStringZType:
    case te::gm::MultiLineStringMType:
    case te::gm::MultiLineStringZMType:
    {
      canvas.setLineColor(te::color::RGBAColor(255, 0, 0, 128));
      canvas.setLineWidth(6);
    }
    break;

    case te::gm::PointType:
    case te::gm::PointZType:
    case te::gm::PointMType:
    case te::gm::PointZMType:
    case te::gm::MultiPointType:
    case te::gm::MultiPointZType:
    case te::gm::MultiPointMType:
    case te::gm::MultiPointZMType:
    {
      std::size_t size = 24;

      te::se::Stroke* stroke = te::se::CreateStroke("#FF0000", "2", "0.5");
      te::se::Fill* fill = te::se::CreateFill("#FFFFFF", "0.5");
      te::se::Mark* mark = te::se::CreateMark("square", stroke, fill);

      te::color::RGBAColor** rgba = te::map::MarkRendererManager::getInstance().render(mark, size);

      canvas.setPointColor(te::color::RGBAColor(0, 0, 0, TE_TRANSPARENT));
      canvas.setPointPattern(rgba, size, size);

      te::common::Free(rgba, size);
      delete mark;
    }
    break;

    default:
      return;
  }

  try
  {
    // Gets the dataset
    std::auto_ptr<te::da::DataSet> dataset = m_coordLayer->getData(m_objIdSet);
    assert(dataset.get());

    while (dataset->moveNext())
    {
      std::auto_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      canvas.draw(g.get());
    }

    if (m_objIdSet->size() >= 3)
    {
      te::gm::Geometry* buffer = createBuffer(dataset, m_coordLayer->getSRID(), gp->getName());

      canvas.draw(buffer);

      delete buffer;
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }
}

te::gm::Geometry* te::qt::plugins::tv5plugins::TrackClassifier::createBuffer(std::auto_ptr<te::da::DataSet> dataset, int srid, std::string gpName)
{
  //get first point selected (root)
  dataset->moveFirst();

  std::auto_ptr<te::gm::Geometry> rootGeom(dataset->getGeometry(gpName));

  //get sample info
  double distance, angle;

  getTrackInfo(dataset, gpName, distance, angle);

  //get parcel geom
  std::auto_ptr<te::gm::Geometry> parcelGeom = getParcelGeeom(rootGeom.get());

  //create track
  std::list<te::gm::Point*> track;

  bool insideParcel = parcelGeom->covers(rootGeom.get());

  while (insideParcel)
  {

  }

  //create buffer
  te::gm::LineString* line = new te::gm::LineString(track.size(), te::gm::LineStringType, srid);

  for (std::size_t t = 0; t < track.size(); ++t)
  {
    line->setPoint(t, track[t]->getX(), track[t]->getY());
  }

  return line->buffer(DISTANCE_BUFFER);
}

void te::qt::plugins::tv5plugins::TrackClassifier::getTrackInfo(std::auto_ptr<te::da::DataSet> dataset, std::string gpName, double& distance, double& angle)
{
  distance = 0.;
  angle = 0.;
  dataset->moveFirst();

  std::auto_ptr<te::gm::Geometry> gFirst(dataset->getGeometry(gpName));

  while (dataset->moveNext())
  {
    std::auto_ptr<te::gm::Geometry> gNext(dataset->getGeometry(gpName));

    distance += gFirst->distance(gNext.get());
    angle += getAngle(gFirst.get(), gNext.get());

    gFirst = gNext;
  }

  distance = distance / (dataset->size() - 1);
  angle = angle / (dataset->size() - 1);
}

std::auto_ptr<te::gm::Geometry> te::qt::plugins::tv5plugins::TrackClassifier::getParcelGeeom(te::gm::Geometry* root)
{
  if (!m_parcelLayer.get())
    return;

  // Bulding the query box
  te::gm::Envelope envelope(*root->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_parcelLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_parcelLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_parcelLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_parcelLayer->getExtent()))
    return;

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_parcelLayer->getSchema());

  if (!schema->hasGeom())
    return;

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  // Gets the dataset
  std::auto_ptr<te::da::DataSet> dataset = m_parcelLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);

  assert(dataset.get());

  dataset->moveFirst();

  std::auto_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

  return g;
}

double te::qt::plugins::tv5plugins::TrackClassifier::getAngle(te::gm::Geometry* first, te::gm::Geometry* last)
{
  te::gm::Point* lastPoint = dynamic_cast<te::gm::Point*>(last);
  te::gm::Point* firstPoint = dynamic_cast<te::gm::Point*>(first);

  double dx = lastPoint->getX() - firstPoint->getX();
  double ax = fabs(dx);
  double dy = lastPoint->getY() - firstPoint->getY();
  double ay = fabs(dy);

  double t = 0.0;

  if ((dx == 0.0) && (dy == 0.0))
    t = 0.0;
  else
    t = dy / (ax + ay);

  if (dx < 0.0)
    t = 2 - t;
  else if (dy < 0.0)
    t = 4.0 + t;

  double angle = t * 90.0;

  return angle;
}


/*
/ Classify track
{
// on mouse down
Point start (e.X(),e.y());
createTrack (start,trees, parcels,0.2)

//Show track
// on mouse right selection
switch (option) {
case classify:
track.persist(trees);
break;
case cancel:
}
}

Class Track {
List track;
Public:
Void createTree (Point position,string atribute){}
Tree getLast() {
if (!track.empty()){
track.last();
Tree tree=track.get();
track.remove();
return tree;
} else return null;
}
Boolean persit(Layer trees) {
while !track.empty() {
// update Layer trees
}
}
}

Class Tree () {
Point position;
attribute
}











// Creates and classify a tree track
// Gets all trees in a track defined by a start tree using parcel inter tree distance and direction.
// The track will include live and dead trees in both directions.
// Input:
// start - a good tree in a track from will be automatically created a track
// trees - a Layer where the trees are
// parcels - a Layer with homogeneous plantation (track direction, inter trees distance, inter track distance)
// toleranceFactor - a factor of track inter tree distance, normally 0.1 or 0.2, to search for the next tree.
// Output:
// the line track
Boolean createTrack (STD::Deck& track, Point start,Layer trees, Layer parcels,Layer direction,long toleranceFactor){
parcel=parcels.get(start);
long direction = parcel.direction();
long interTreeDistance = parcel.distance();
long deltaX = Math.cos(direction)*interTreeDistance;
long deltaY = Math.sin(parcel.direction())*interTreeDistance;
long tolerance=interTreeDistance*toleranceFactor;
Track trackForward = new Track();
Track trackBackard = new Track();
trackForward.insert(start);
Point treePositon = Point (start.X()+deltaX,start.Y()+deltaY);
//Creates a continuous tree track forward
while  parcel.inside(treePosition) {
if getTree(treePosition,tolerance) then { trackForward.createTree(treePosition,"live");}
else{
trackForward.createTree(treePosition,"dead");
};
treePosition = Point(treePosition.X()+deltaX,newtree.Y()+deltaY);
}
// Creates a continuous tree track backward
while !trackForward.empty() {trackBackward.insert(trackForward.getLast());}
treePosition= Point(start.X()-deltaX,start.Y()-deltaY);
while  parcel.inside(treePosition) {
if getTree(treePosition,tolerance) then { tracKbackward.createTree(treePosition,"live");}
else{
trackBackward.createTree(treePosition,"dead");};
}
treePosition = Point (treePosition.X()-deltaX,newtree.Y()-deltaY);
}
return trackBackward;
}


int assignIntruders(Line track, Layer trees, bufferDistance) {
Polygon exclusion=createBuffer(track, bufferDistance); //??????
List intruders = trees.inside(exclusion); //??????????
while !intruders.empty() {
//Assign intruder
Tree intruder = intruders.get();
trees.classifyTree(intruder. location(),"intruder");
intruderCount++;
}
return intruderCount;
}

// Gets the closest tree from given treePosition
// Input:
// treePosition - the theoretical position of next tree in that track
// tolerance - distance used to create a box around treePosition
// Output:
// treePosition - the tree closest from treePosition or treePosition unchanged if none tree was found
//Return:
// true - a nearest tree from treePosition was found
// false - no tree was found
boolean getTree (Layer trees,Point treePosition,float tolerance){
Box b = Box(treePosition.X()-tolerance,treePosition.Y()-tolerance,treePosition.X()+tolerance,treePosition.Y()+tolerance);
// Retrieve all points inside the defined box in the Layer
List candidates = trees.getPoints (treePosition, b);
if candidates.empty() then return false;
if candidates.size() == 1 { treePosition=candidates.get(); return true }
Tree nearest= candidates.get();
long mindistance=sqrt(pow(nearest.X()-treePosition.X(),2)+pow(nearest.Y()-treePosition.Y(),2))
while !candidates.empty() {
Tree candidate=candidates.get();
if (mindistance > long d=sqrt(pow(candidate.X()-treePosition.X(),2)+pow(candidate.Y()-treePosition.Y(),2)) then {
nearest = candidate;
mindistance = d;
}
treePosition = nearest;
return true;
}

}*/
