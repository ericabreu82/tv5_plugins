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
\file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/TrackDeadClassifier.cpp

\brief This class implements a concrete tool to track dead roots classifier
*/

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/LineString.h>
#include <terralib/geometry/MultiPoint.h>
#include <terralib/geometry/Point.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/DataSetLayer.h>
#include <terralib/maptools/MarkRendererManager.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/se/Fill.h>
#include <terralib/se/Stroke.h>
#include <terralib/se/Mark.h>
#include <terralib/se/Utils.h>
#include <terralib/qt/widgets/canvas/Canvas.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include "TrackDeadClassifier.h"

// Qt
#include <QApplication>
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

#define DISTANCE 2.0
#define NDVI_THRESHOLD 120.0
#define TOLERANCE_FACTOR 0.2

te::qt::plugins::tv5plugins::TrackDeadClassifier::TrackDeadClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::map::AbstractLayerPtr rasterLayer, QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_point0(0),
  m_objId0(0),
  m_point1(0),
  m_objId1(0),
  m_starterId(0),
  m_panStarted(false)
{
  m_distLineEdit = 0;
  m_distanceToleranceFactorLineEdit = 0;

  m_dx = 0.;
  m_dy = 0.;
  m_distance = DISTANCE;

  setCursor(cursor);
  
  display->setFocus();
  
  createRTree();

  getStartIdValue();

  //get raster
  std::auto_ptr<te::da::DataSet> ds = rasterLayer->getData();

  m_ndviRaster = ds->getRaster(0).release();

  m_totalDistance = 0;
}

te::qt::plugins::tv5plugins::TrackDeadClassifier::~TrackDeadClassifier()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_centroidRtree.clear();
  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);

  delete m_point0;
  delete m_point1;

  delete m_ndviRaster;
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::setLineEditComponents(QLineEdit* distLineEdit, QLineEdit* distanceToleranceFactorLineEdit, QLineEdit* threshold)
{
  m_distLineEdit = distLineEdit;
  m_distanceToleranceFactorLineEdit = distanceToleranceFactorLineEdit;
  m_thresholdLineEdit = threshold;
}

bool te::qt::plugins::tv5plugins::TrackDeadClassifier::eventFilter(QObject* watched, QEvent* e)
{
  if (e->type() == QEvent::MouseButtonRelease)
  {
    m_display->setFocus();

    QMouseEvent* event = static_cast<QMouseEvent*>(e);

    if (event->button() == Qt::LeftButton)
    {
      selectObjects(event);

      return true;
    }
    else
    {
      return panMouseReleaseEvent(event);
    }
  }
  else if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* event = static_cast<QMouseEvent*>(e);

    return panMousePressEvent(event);
  }
  else if (e->type() == QEvent::MouseMove)
  {
    QMouseEvent* event = static_cast<QMouseEvent*>(e);

    panMouseMoveEvent(event);

    deadTrackMouseMove(event);

    return true;
  }
  else if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* event = static_cast<QKeyEvent*>(e);

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Control) && m_point1 && m_point0)
      classifyObjects();

    if (event->key() == Qt::Key_Escape)
      cancelOperation();

    return true;
  }

  return false;
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::selectObjects(QMouseEvent* e)
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

    // Let's generate the oid
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

      if (!m_point0)
      {
        m_point0 = getPoint(dynamic_cast<te::gm::Geometry*>(g->clone()));
        m_objId0 = te::da::GenerateOID(dataset.get(), pnames);
        break;
      }

      if (!m_point1)
      {
        m_point1 = getPoint(dynamic_cast<te::gm::Geometry*>(g->clone()));
        m_objId1 = te::da::GenerateOID(dataset.get(), pnames);

        m_totalDistance = m_point0->distance(m_point1);
        break;
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  drawSelecteds();

  //repaint the layer
  m_display->repaint();
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::classifyObjects()
{
  if (!m_coordLayer.get())
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  std::auto_ptr<te::da::DataSetType> dsType(m_coordLayer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(dsType.get());

  //get parcel geom
  int parcelId;
  std::auto_ptr<te::gm::Geometry> parcelGeom = getParcelGeeom(m_point0, parcelId);

  std::list<te::gm::Point*> track;

  createDeadTrack(m_coordLayer->getSRID(), gp->getName(), track);

  try
  {
    //create dataset type
    std::auto_ptr<te::da::DataSetType> dsType = m_coordLayer->getSchema();

    //create dataset type
    std::auto_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dsType->getName()));

    //create id property
    te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
    dataSetType->add(idProperty);

    //create origin id property
    te::dt::SimpleProperty* originIdProperty = new te::dt::SimpleProperty("originId", te::dt::INT32_TYPE);
    dataSetType->add(originIdProperty);

    //create area property
    te::dt::SimpleProperty* areaProperty = new te::dt::SimpleProperty("area", te::dt::DOUBLE_TYPE);
    dataSetType->add(areaProperty);

    //create forest type
    te::dt::StringProperty* typeProperty = new te::dt::StringProperty("type");
    dataSetType->add(typeProperty);

    //create geometry property
    te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", m_coordLayer->getSRID(), te::gm::PointType);
    dataSetType->add(geomProperty);

    m_dataSet.reset(new te::mem::DataSet(dataSetType.get()));

    std::list<te::gm::Point*>::iterator it;

    for (it = track.begin(); it != track.end(); ++it)
    {
      te::gm::Point* pointItem = *it;
      //create dataset item
      te::mem::DataSetItem* item = new te::mem::DataSetItem(m_dataSet.get());

      //set id
      item->setInt32(0, m_starterId);

      //set origin id
      item->setInt32(1, parcelId);

      //set area
      item->setDouble(2, 0.);

      //forest type
      item->setString(3, "DEAD");

      //set geometry
      item->setGeometry(4, new te::gm::Point(*pointItem));

      m_dataSet->add(item);

      ++m_starterId;
    }

    //class
    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_coordLayer.get());

    if (dsLayer)
    {
      te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

      //add dead dataset
      if (m_dataSet.get())
      {
        std::map<std::string, std::string> options;

        dataSource->add(dsType->getName(), m_dataSet.get(), options);
      }
    }
  }
  catch (std::exception& e)
  {
    QApplication::restoreOverrideCursor();

    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error classifing track. Details:") + " %1.").arg(e.what()));
    return;
  }

  delete m_point0;
  m_point0 = 0;
  delete m_objId0;
  m_objId0 = 0;

  delete m_point1;
  m_point1 = 0;
  delete m_objId1;
  m_objId1 = 0;

  //createRTree();

  m_dataSet.reset();

  //repaint the layer
  m_display->refresh();

  QApplication::restoreOverrideCursor();
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::cancelOperation()
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  delete m_point0;
  m_point0 = 0;
  delete m_objId0;
  m_objId0 = 0;

  delete m_point1;
  m_point1 = 0;
  delete m_objId1;
  m_objId1 = 0;

  m_dataSet.reset();

  std::auto_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  //repaint the layer
  m_display->repaint();
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::drawSelecteds()
{
  if (!m_coordLayer.get())
    return;

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

  //configure for polygons
  canvas.setPolygonContourWidth(1);
  canvas.setPolygonContourColor(te::color::RGBAColor(0, 0, 0, 128));
  canvas.setPolygonFillColor(te::color::RGBAColor(255, 255, 255, 128));
   
  //configure for lines
  canvas.setLineColor(te::color::RGBAColor(255, 0, 0, 128));
  canvas.setLineWidth(6);
   
  //configure for points
  std::size_t size = 24;

  te::se::Stroke* stroke = te::se::CreateStroke("#FF0000", "2", "0.5");
  te::se::Fill* fill = te::se::CreateFill("#FFFFFF", "0.5");
  te::se::Mark* mark = te::se::CreateMark("square", stroke, fill);

  te::color::RGBAColor** rgba = te::map::MarkRendererManager::getInstance().render(mark, size);

  canvas.setPointColor(te::color::RGBAColor(0, 0, 0, TE_TRANSPARENT));
  canvas.setPointPattern(rgba, size, size);

  te::common::Free(rgba, size);
  delete mark;
  
  try
  {
    // Gets the dataset
    if (m_point0)
      canvas.draw(m_point0);

    if (m_point1)
      canvas.draw(m_point1);
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error creating buffer. Details:") + " %1.").arg(e.what()));
    return;
  }
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::createDeadTrack(int srid, std::string gpName,  std::list<te::gm::Point*>& track)
{
  std::auto_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  //get sample info
  getTrackInfo();

  double dx = m_dx;
  double dy = m_dy;

  //get parcel geom
  int parcelId;
  std::auto_ptr<te::gm::Geometry> parcelGeom = getParcelGeeom(m_point0, parcelId);

  parcelGeom->setSRID(srid);

  bool insideParcel = parcelGeom->covers(m_point0);

  te::gm::Point* rootPoint = m_point0;
  
  QApplication::setOverrideCursor(Qt::WaitCursor);

  while (true)
  {
    te::gm::Point* guestPoint = createGuessPoint(rootPoint, dx, dy, srid);
    
    //create envelope to find if guest point exist
    te::gm::Envelope ext(guestPoint->getX(), guestPoint->getY(), guestPoint->getX(), guestPoint->getY());

    double toleranceFactor = 0.;

    if (m_distanceToleranceFactorLineEdit->text().isEmpty())
    {
      toleranceFactor = TOLERANCE_FACTOR;
    }
    else
    {
      toleranceFactor = m_distanceToleranceFactorLineEdit->text().toDouble();
    }

    ext.m_llx -= (m_distance * toleranceFactor);
    ext.m_lly -= (m_distance * toleranceFactor);
    ext.m_urx += (m_distance * toleranceFactor);
    ext.m_ury += (m_distance * toleranceFactor);

    rootPoint = guestPoint;

    double curDistance = m_point0->distance(rootPoint);

    if (curDistance > m_totalDistance)
      break;

    //check on tree
    std::vector<int> resultsTree;

    m_centroidRtree.search(ext, resultsTree);

    if (resultsTree.empty())
    {
      //dead point
      insideParcel = parcelGeom->covers(rootPoint);

      if (insideParcel)
      {
        track.push_back(new te::gm::Point(*rootPoint));
      }
      else
      {
        break;
      }
    }
  }

  QApplication::restoreOverrideCursor();
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::getTrackInfo()
{
  if (m_distLineEdit->text().isEmpty())
    m_distance = DISTANCE;
  else
    m_distance = m_distLineEdit->text().toDouble();

  double bigDistance = m_point0->distance(m_point1);

  double big_dx = m_point1->getX() - m_point0->getX();
  double big_dy = m_point1->getY() - m_point0->getY();

  m_dx = m_distance * big_dx / bigDistance;
  m_dy = m_distance * big_dy / bigDistance;
}

std::auto_ptr<te::gm::Geometry> te::qt::plugins::tv5plugins::TrackDeadClassifier::getParcelGeeom(te::gm::Geometry* root, int& parcelId)
{
  if (!m_parcelLayer.get())
    throw;

  // Bulding the query box
  te::gm::Envelope envelope(*root->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((root->getSRID() != TE_UNKNOWN_SRS) && (m_parcelLayer->getSRID() != TE_UNKNOWN_SRS) && (root->getSRID() != m_parcelLayer->getSRID()))
    reprojectedEnvelope.transform(root->getSRID(), m_parcelLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_parcelLayer->getExtent()))
    throw;

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_parcelLayer->getSchema());

  if (!schema->hasGeom())
    throw;

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  te::da::PrimaryKey* pk = schema->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();

  // Gets the dataset
  std::auto_ptr<te::da::DataSet> dataset = m_parcelLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);

  assert(dataset.get());

  dataset->moveBeforeFirst();

  std::auto_ptr<te::gm::Geometry> g;

  while (dataset->moveNext())
  {
    g = dataset->getGeometry(gp->getName());

    if (g->getSRID() == TE_UNKNOWN_SRS)
      g->setSRID(m_coordLayer->getSRID());

    if (g->getSRID() != root->getSRID())
      g->transform(root->getSRID());

    if (g->covers(root))
    {
      parcelId = dataset->getInt32(name);

      break;
    }
  }

  return g;
}

te::gm::Point* te::qt::plugins::tv5plugins::TrackDeadClassifier::createGuessPoint(te::gm::Point* p, double dx, double dy, int srid)
{
  return new te::gm::Point(p->getX() + dx, p->getY() + dy, srid);
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::createRTree()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);

  m_centroidRtree.clear();
  m_centroidGeomMap.clear();
  m_centroidObjIdMap.clear();

  //create rtree
  std::auto_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());
  std::auto_ptr<te::da::DataSet> ds(m_coordLayer->getData());

  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(schema.get(), pnames);

  //geom property info
  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(schema.get());

  int geomIdx = te::da::GetPropertyPos(schema.get(), gmProp->getName());

  //id info
  te::da::PrimaryKey* pk = schema->getPrimaryKey();

  int idIdx = te::da::GetPropertyPos(schema.get(), pk->getProperties()[0]->getName());

  ds->moveBeforeFirst();

  while (ds->moveNext())
  {
    std::string strId = ds->getAsString(idIdx);

    int id = atoi(strId.c_str());

    te::gm::Geometry* g = ds->getGeometry(geomIdx).release();
    const te::gm::Envelope* box = g->getMBR();

    m_centroidRtree.insert(*box, id);

    m_centroidGeomMap.insert(std::map<int, te::gm::Geometry*>::value_type(id, g));

    m_centroidObjIdMap.insert(std::map<int, te::da::ObjectId*>::value_type(id, te::da::GenerateOID(ds.get(), pnames)));
  }

  QApplication::restoreOverrideCursor();
}

te::gm::Point* te::qt::plugins::tv5plugins::TrackDeadClassifier::getPoint(te::gm::Geometry* g)
{
  te::gm::Point* point = 0;

  if (g->getGeomTypeId() == te::gm::MultiPointType)
  {
    te::gm::MultiPoint* mPoint = dynamic_cast<te::gm::MultiPoint*>(g);
    point = dynamic_cast<te::gm::Point*>(mPoint->getGeometryN(0));
  }
  else if (g->getGeomTypeId() == te::gm::PointType)
  {
    point = dynamic_cast<te::gm::Point*>(g);
  }

  return point;
}

void te::qt::plugins::tv5plugins::TrackDeadClassifier::getStartIdValue()
{
  if (!m_coordLayer.get())
    throw;

  std::auto_ptr<te::da::DataSet> dataset = m_coordLayer->getData();
  std::auto_ptr<te::da::DataSetType> dataSetType = m_coordLayer->getSchema();

  te::da::PrimaryKey* pk = dataSetType->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();

  dataset->moveBeforeFirst();

  while (dataset->moveNext())
  {
    int id = dataset->getInt32(name);

    if (id > m_starterId)
      m_starterId = id;
  }

  ++m_starterId;
}

bool te::qt::plugins::tv5plugins::TrackDeadClassifier::deadTrackMouseMove(QMouseEvent* e)
{
  if (!m_point0 || m_point1)
    return false;

  QPointF pw = m_display->transform(e->localPos());

  te::gm::Coord2D cFinal = te::gm::Coord2D(pw.x(), pw.y());

  // Clear!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& env = m_display->getExtent();

  // Prepares the canvas
  te::qt::widgets::Canvas canvas(m_display->width(), m_display->height());
  canvas.setDevice(draft, false);
  canvas.setWindow(env.m_llx, env.m_lly, env.m_urx, env.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

  // Build the geometry
  te::gm::LineString* line = new te::gm::LineString(2, te::gm::LineStringType);

  line->setPoint(0, m_point0->getX(), m_point0->getY());
  line->setPoint(1, cFinal.getX(), cFinal.getY());

  // Setup canvas style
  QPen pen;
  pen.setColor(QColor(100, 177, 216));
  pen.setWidth(3);

  canvas.setLineColor(pen.color().rgba());
  canvas.setLineWidth(pen.width());

  canvas.draw(line);

  delete line;

  m_display->repaint();

  return true;
}

bool te::qt::plugins::tv5plugins::TrackDeadClassifier::panMousePressEvent(QMouseEvent* e)
{
  if (e->button() != Qt::MiddleButton)
    return false;

  m_panStarted = true;
  m_origin = e->pos();
  m_delta *= 0;
  m_actionCursor.setShape(Qt::SizeAllCursor);

  // Adjusting the action cursor
  if (m_actionCursor.shape() != Qt::BlankCursor)
    m_display->setCursor(m_actionCursor);

  return true;
}

bool te::qt::plugins::tv5plugins::TrackDeadClassifier::panMouseMoveEvent(QMouseEvent* e)
{
  if (!m_panStarted)
    return false;

  // Calculates the delta value
  m_delta = e->pos() - m_origin;

  // Gets the draft map display pixmap
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill();

  // Gets the current result of map display, i.e. The draw layer composition.
  QPixmap* result = m_display->getDisplayPixmap();

  // Let's draw!
  QPainter painter(draft);
  painter.drawPixmap(0, 0, *result); // Draw the current result.
  painter.save();
  painter.setOpacity(0.3); // Adjusting transparency feedback.
  painter.drawPixmap(m_delta, *result); // Draw the current result translated.
  painter.restore();

  m_display->repaint();

  return true;
}

bool te::qt::plugins::tv5plugins::TrackDeadClassifier::panMouseReleaseEvent(QMouseEvent* e)
{
  m_panStarted = false;

  // Roll back the default tool cursor
  m_display->setCursor(m_cursor);

  if (e->button() != Qt::MiddleButton || m_delta.isNull())
    return false;

  // Clears the draft map display pixmap
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  // Calculates the extent translated
  QRect rec(0, 0, m_display->width(), m_display->height());
  QPoint center = rec.center();
  center -= m_delta;
  rec.moveCenter(center);

  // Conversion to world coordinates
  QPointF ll(rec.left(), rec.bottom());
  QPointF ur(rec.right(), rec.top());
  ll = m_display->transform(ll);
  ur = m_display->transform(ur);

  // Updates the map display with the new extent
  te::gm::Envelope envelope(ll.x(), ll.y(), ur.x(), ur.y());
  m_display->setExtent(envelope);

  drawSelecteds();

  return true;
}

