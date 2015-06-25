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
\file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/TrackAutoClassifier.cpp

\brief This class implements a concrete tool to track classifier
*/

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/common/progress/TaskProgress.h>
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
#include "TrackAutoClassifier.h"

// Qt
#include <QApplication>
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

#define DISTANCE 2.0
#define DISTANCE_BUFFER 1.5
#define TOLERANCE_FACTOR 0.2
#define POLY_AREA_MIN 0.1
#define POLY_AREA_MAX 2.0

te::qt::plugins::tv5plugins::TrackAutoClassifier::TrackAutoClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::map::AbstractLayerPtr polyLayer, QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_polyLayer(polyLayer),
  m_track(0),
  m_point0(0),
  m_objId0(0),
  m_point1(0),
  m_objId1(0),
  m_starterId(0),
  m_roots(0)
{
  m_distLineEdit = 0;
  m_distanceBufferLineEdit = 0;
  m_distanceToleranceFactorLineEdit = 0;

  m_dx = 0.;
  m_dy = 0.;
  m_distance = DISTANCE;

  m_classify = false;

  setCursor(cursor);
  
  display->setFocus();
  
  createRTree();

  getStartIdValue();
}

te::qt::plugins::tv5plugins::TrackAutoClassifier::~TrackAutoClassifier()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_polyRtree.clear();
  te::common::FreeContents(m_polyGeomMap);
  
  m_centroidRtree.clear();
  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);

  delete m_point0;
  delete m_point1;

  delete m_roots;
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::setLineEditComponents(QLineEdit* distLineEdit, QLineEdit* distanceBufferLineEdit, QLineEdit* distanceToleranceFactorLineEdit, QLineEdit* polyAreaMin, QLineEdit* polyAreaMax)
{
  m_distLineEdit = distLineEdit;
  m_distanceBufferLineEdit = distanceBufferLineEdit;
  m_distanceToleranceFactorLineEdit = distanceToleranceFactorLineEdit;
  m_polyAreaMin = polyAreaMin;
  m_polyAreaMax = polyAreaMax;
}

bool te::qt::plugins::tv5plugins::TrackAutoClassifier::eventFilter(QObject* watched, QEvent* e)
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
  }
  else if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* event = static_cast<QKeyEvent*>(e);

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Control) && m_classify && m_point0)
      classifyObjects();

    if (event->key() == Qt::Key_Escape)
      cancelOperation(false);

    if (event->key() == Qt::Key_Backspace)
      cancelOperation(true);

    return true;
  }

  return false;
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::selectObjects(QMouseEvent* e)
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

        getTrackInfo();

        break;
      }

      te::da::ObjectIdSet* oids = 0;
      te::da::GetEmptyOIDSet(schema.get(), oids);

      oids->add(te::da::GenerateOID(dataset.get(), pnames));

      if (!m_roots)
      {
        te::da::GetEmptyOIDSet(schema.get(), m_roots);
      }

      m_roots->symDifference(oids);
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

void te::qt::plugins::tv5plugins::TrackAutoClassifier::classifyObjects()
{
  if (!m_roots || m_roots->size() == 0)
    return;

  std::auto_ptr<te::da::DataSetType> dsType(m_coordLayer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(dsType.get());

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // Let's generate the oid
  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(dsType.get(), pnames);

  std::auto_ptr<te::da::DataSet> dsRoots = m_coordLayer->getData(m_roots);

  std::size_t dsSize = dsRoots->size();

  te::common::TaskProgress task("Classifying...");
  task.setTotalSteps(dsSize);

  dsRoots->moveBeforeFirst();

  while (dsRoots->moveNext())
  {
    std::auto_ptr<te::gm::Geometry> g(dsRoots->getGeometry(gp->getName()));

    te::gm::Point* rootPoint = getPoint(g.get());

    te::da::ObjectId* objIdRoot = te::da::GenerateOID(dsRoots.get(), pnames);

    te::gm::LineString* line = 0;

    std::list<te::gm::Point*> track;

    std::auto_ptr<te::gm::Geometry> buffer(createBuffer(rootPoint, objIdRoot, m_coordLayer->getSRID(), gp->getName(), line, track));

    try
    {
      //create dataset type
      te::mem::DataSet* liveDS = 0;
      te::mem::DataSet* intruderDS = 0;

      getClassDataSets(dsType.get(), liveDS, intruderDS, buffer.get());

      //class
      te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_coordLayer.get());

      if (dsLayer)
      {
        te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

        //update live dataset
        std::vector<size_t> ids;
        ids.push_back(0);

        if (liveDS)
        {
          std::vector< std::set<int> > properties;
          std::size_t dsSize = liveDS->size();

          for (std::size_t t = 0; t < dsSize; ++t)
          {
            std::set<int> setPos;
            setPos.insert(4);

            properties.push_back(setPos);
          }

          dataSource->update(dsType->getName(), liveDS, properties, ids);
        }

        //update intruder dataset
        if (intruderDS)
        {
          std::vector< std::set<int> > properties;
          std::size_t dsSize = intruderDS->size();

          for (std::size_t t = 0; t < dsSize; ++t)
          {
            std::set<int> setPos;
            setPos.insert(4);

            properties.push_back(setPos);
          }

          dataSource->update(dsType->getName(), intruderDS, properties, ids);
        }

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

      QMessageBox::critical(m_display, tr("Error"), QString(tr("Error classifying track. Details:") + " %1.").arg(e.what()));
      break;
    }

    task.pulse();
  }

  QApplication::restoreOverrideCursor();

  m_classify = true;

  createRTree();

  m_dataSet.reset();

  delete m_roots;
  m_roots = 0;

  //repaint the layer
  m_display->refresh();
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::cancelOperation(bool restart)
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  if (restart)
  {
    delete m_point0;
    m_point0 = 0;
    delete m_objId0;
    m_objId0 = 0;

    delete m_point1;
    m_point1 = 0;
    delete m_objId1;
    m_objId1 = 0;

    m_classify = false;
  }

  delete m_roots;
  m_roots = 0;
  
  m_dataSet.reset();

  std::auto_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  te::da::GetEmptyOIDSet(schema.get(), m_track);

  //repaint the layer
  m_display->repaint();
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::drawSelecteds()
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

  try
  {
    {
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
    }

    // Gets the dataset
    if (m_point0)
      canvas.draw(m_point0);

    if (m_point1)
      canvas.draw(m_point1);

    {
      //configure for points
      std::size_t size = 24;

      te::se::Stroke* stroke = te::se::CreateStroke("#00FF00", "2", "0.5");
      te::se::Fill* fill = te::se::CreateFill("#FFFFFF", "0.5");
      te::se::Mark* mark = te::se::CreateMark("square", stroke, fill);

      te::color::RGBAColor** rgba = te::map::MarkRendererManager::getInstance().render(mark, size);

      canvas.setPointColor(te::color::RGBAColor(0, 0, 0, TE_TRANSPARENT));
      canvas.setPointPattern(rgba, size, size);

      te::common::Free(rgba, size);
      delete mark;
    }

    if (m_roots)
    {
      std::auto_ptr<te::da::DataSet> dsCoords = m_coordLayer->getData(m_roots);

      while (dsCoords->moveNext())
      {
        std::auto_ptr<te::gm::Geometry> g(dsCoords->getGeometry(gp->getName()));

        if (g->getSRID() == TE_UNKNOWN_SRS)
          g->setSRID(m_coordLayer->getSRID());

        canvas.draw(g.get());
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error creating buffer. Details:") + " %1.").arg(e.what()));
    return;
  }
}

te::gm::Geometry* te::qt::plugins::tv5plugins::TrackAutoClassifier::createBuffer(te::gm::Point* rootPoint, te::da::ObjectId* objIdRoot, int srid, std::string gpName, te::gm::LineString*& lineBuffer, std::list<te::gm::Point*>& track)
{
  std::auto_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(schema.get(), pnames);

  //get parcel geom
  int parcelId;
  std::auto_ptr<te::gm::Geometry> parcelGeom = getParcelGeeom(m_point0, parcelId);

  parcelGeom->setSRID(srid);

  te::da::GetEmptyOIDSet(schema.get(), m_track);

  bool insideParcel = parcelGeom->covers(m_point0);

  track.push_back(new te::gm::Point(*rootPoint));

  te::gm::Point* starter = new te::gm::Point(*rootPoint);

  m_track->add(objIdRoot);

  bool invert = false;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  double dx = m_dx;
  double dy = m_dy;

  double toleranceFactor = 0.;

  if (m_distanceToleranceFactorLineEdit->text().isEmpty())
  {
    toleranceFactor = TOLERANCE_FACTOR;
  }
  else
  {
    toleranceFactor = m_distanceToleranceFactorLineEdit->text().toDouble();
  }

  while (insideParcel)
  {
    //te::gm::Point* guestPoint = createGuessPoint(rootPoint, distance, angle, srid);

    te::gm::Point* guestPoint = createGuessPoint(rootPoint, dx, dy, srid);
    
    //create envelope to find if guest point exist
    te::gm::Envelope ext(guestPoint->getX(), guestPoint->getY(), guestPoint->getX(), guestPoint->getY());

    ext.m_llx -= (m_distance * toleranceFactor);
    ext.m_lly -= (m_distance * toleranceFactor);
    ext.m_urx += (m_distance * toleranceFactor);
    ext.m_ury += (m_distance * toleranceFactor);

    //check on tree
    std::vector<int> resultsTree;

    m_centroidRtree.search(ext, resultsTree);

    if (resultsTree.empty())
    {
      //dead point
      rootPoint = guestPoint;

      insideParcel = parcelGeom->covers(rootPoint);

      if (insideParcel)
      {
        if (!invert)
        {
          track.push_back(new te::gm::Point(*rootPoint));
        }
        else
        {
          track.push_front(new te::gm::Point(*rootPoint));
        }

        addGuessPoint(rootPoint, parcelId);
      }
      else
      {
        if (!invert)
        {
          invert = true;
          insideParcel = true;
          dx = dx * -1;
          dy = dy * -1;
          rootPoint = starter;
        }
      }
       
    }
    else
    {
      //live point
      te::da::ObjectId* objIdCandidate = 0;
      te::gm::Point* pCandidate = getCandidatePoint(rootPoint, guestPoint, srid, resultsTree, objIdCandidate);

      if (!pCandidate)
      {
        rootPoint = guestPoint;

        addGuessPoint(rootPoint, parcelId);
      }
      else
      {
        rootPoint = pCandidate;
        m_track->add(objIdCandidate);
      }

      insideParcel = parcelGeom->covers(rootPoint);

      if (insideParcel)
      {
        if (!invert)
        {
          track.push_back(new te::gm::Point(*rootPoint));
        }
        else
        {
          track.push_front(new te::gm::Point(*rootPoint));
        }
      }
      else
      {
        if (!invert)
        {
          invert = true;
          insideParcel = true;
          dx = dx * -1;
          dy = dy * -1;
          rootPoint = starter;
        }
      }
    }
  }

  QApplication::restoreOverrideCursor();

  delete starter;

  if (track.size() < 2)
  {
    return 0;
  }

  //create buffer
  lineBuffer = new te::gm::LineString(track.size(), te::gm::LineStringType, srid);

  int count = 0;
  std::list<te::gm::Point*>::iterator it = track.begin();
  while (it != track.end())
  {
    lineBuffer->setPoint(count, (*it)->getX(), (*it)->getY());

    ++count;
    ++it;
  }

  double distanceBuffer = 0.;

  if (m_distanceBufferLineEdit->text().isEmpty())
  {
    distanceBuffer = DISTANCE_BUFFER;
  }
  else
  {
    distanceBuffer = m_distanceBufferLineEdit->text().toDouble();
  }


  return lineBuffer->buffer(distanceBuffer, 16, te::gm::CapButtType);
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::getTrackInfo()
{
  if (m_distLineEdit->text().isEmpty())
    m_distance = DISTANCE;
  else
    m_distance = m_distLineEdit->text().toDouble();
  
  if (m_point1)
  {
    double bigDistance = m_point0->distance(m_point1);

    double big_dx = m_point1->getX() - m_point0->getX();
    double big_dy = m_point1->getY() - m_point0->getY();

    m_dx = m_distance * big_dx / bigDistance;
    m_dy = m_distance * big_dy / bigDistance;
  }

  m_classify = true;
}

std::auto_ptr<te::gm::Geometry> te::qt::plugins::tv5plugins::TrackAutoClassifier::getParcelGeeom(te::gm::Geometry* root, int& parcelId)
{
  if (!m_parcelLayer.get())
    throw;

  // Bulding the query box
  te::gm::Envelope envelope(*root->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_parcelLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_parcelLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_parcelLayer->getSRID());

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

    if (g->covers(root))
    {
      parcelId = dataset->getInt32(name);

      break;
    }
  }

  return g;
}

te::gm::Point* te::qt::plugins::tv5plugins::TrackAutoClassifier::createGuessPoint(te::gm::Point* p, double dx, double dy, int srid)
{
  return new te::gm::Point(p->getX() + dx, p->getY() + dy, srid);
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::getClassDataSets(te::da::DataSetType* dsType, te::mem::DataSet*& liveDataSet, te::mem::DataSet*& intruderDataSet, te::gm::Geometry* buffer)
{
  // Bulding the query box
  te::gm::Envelope envelope(*buffer->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_coordLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_coordLayer->getExtent()))
    throw;

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  if (!schema->hasGeom())
    throw;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::auto_ptr<te::da::DataSet> dataset = m_coordLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    std::vector<std::string> pnames;
    te::da::GetOIDPropertyNames(schema.get(), pnames);

    // Generates a geometry from the given extent. It will be used to refine the results
    std::auto_ptr<te::gm::Geometry> geometryFromEnvelope(te::gm::GetGeomFromEnvelope(&reprojectedEnvelope, m_coordLayer->getSRID()));

    while (dataset->moveNext())
    {
      std::auto_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      if (!buffer->contains(g.get()))
        continue;

      // Feature found
      te::da::ObjectId* objId = te::da::GenerateOID(dataset.get(), pnames);

      if (m_track->contains(objId))//live
      {
        if (!liveDataSet)
        {
          liveDataSet = new te::mem::DataSet(dsType);
        }

        //create dataset item
        te::mem::DataSetItem* item = new te::mem::DataSetItem(liveDataSet);

        //fid
        item->setInt32(0, dataset->getInt32("FID"));

        //set id
        item->setInt32(1, dataset->getInt32("id"));

        //set origin id
        item->setInt32(2, dataset->getInt32("originId"));

        //set area
        item->setDouble(3, dataset->getDouble("area"));

        //forest type
        item->setString(4, "LIVE");

        //set geometry
        item->setGeometry(5, g.release());

        liveDataSet->add(item);
      }
      else//intruder
      {
        if (!intruderDataSet)
        {
          intruderDataSet = new te::mem::DataSet(dsType);
        }

        //create dataset item
        te::mem::DataSetItem* item = new te::mem::DataSetItem(intruderDataSet);

        //fid
        item->setInt32(0, dataset->getInt32("FID"));

        //set id
        item->setInt32(1, dataset->getInt32("id"));

        //set origin id
        item->setInt32(2, dataset->getInt32("originId"));

        //set area
        item->setDouble(3, dataset->getDouble("area"));

        //forest type
        item->setString(4, "INTRUDER");

        //set geometry
        item->setGeometry(5, g.release());

        intruderDataSet->add(item);
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error getting geometry. Details:") + " %1.").arg(e.what()));
    throw;
  }
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::createRTree()
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

  //create polygons rtree
  if (m_polyGeomMap.empty())
  {
    std::auto_ptr<const te::map::LayerSchema> schema(m_polyLayer->getSchema());
    std::auto_ptr<te::da::DataSet> ds(m_polyLayer->getData());

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

      m_polyRtree.insert(*box, id);

      m_polyGeomMap.insert(std::map<int, te::gm::Geometry*>::value_type(id, g));
    }
  }

  QApplication::restoreOverrideCursor();
}

te::gm::Point* te::qt::plugins::tv5plugins::TrackAutoClassifier::getPoint(te::gm::Geometry* g)
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

void te::qt::plugins::tv5plugins::TrackAutoClassifier::getStartIdValue()
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

bool te::qt::plugins::tv5plugins::TrackAutoClassifier::isClassified(te::da::ObjectId* objId)
{
  if (!m_coordLayer.get())
    throw;

  double polyAreaMin = 0.;
  double polyAreaMax = 0.;

  if (m_polyAreaMin->text().isEmpty())
  {
    polyAreaMin = POLY_AREA_MIN;
  }
  else
  {
    polyAreaMin = m_polyAreaMin->text().toDouble();
  }

  if (m_polyAreaMax->text().isEmpty())
  {
    polyAreaMax = POLY_AREA_MAX;
  }
  else
  {
    polyAreaMax = m_polyAreaMax->text().toDouble();
  }

  std::auto_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  te::da::ObjectIdSet* objIdSet;

  te::da::GetEmptyOIDSet(schema.get(), objIdSet);

  objIdSet->add(objId);

  std::auto_ptr<te::da::DataSet> ds = m_coordLayer->getData(objIdSet);

  if (!ds->isEmpty())
  {
    ds->moveFirst();

    std::string classValue = ds->getString("type");

  if (classValue != "UNKNOWN" && classValue != "CREATED")
      return true;

  //get area attribute and check threshold
    double area = ds->getDouble("area");

    if (area <= polyAreaMin || area >= polyAreaMax)
    {
      return true;
    }
  }

  return false;
}

void te::qt::plugins::tv5plugins::TrackAutoClassifier::addGuessPoint(te::gm::Point* p, int parcelId)
{
  if (!m_dataSet.get())
  {
    std::auto_ptr<te::da::DataSetType> dsType = m_coordLayer->getSchema();

    //create dataset type
    std::auto_ptr<te::da::DataSetType> dataSetType = createTreeDataSetType();

    m_dataSet.reset(new te::mem::DataSet(dataSetType.get()));
  }

  std::string forestType = "UNKNOWN";

  te::gm::Envelope extPoint(p->getX(), p->getY(), p->getX(), p->getY());

  std::vector<int> resultsPolyTree;

  m_polyRtree.search(extPoint, resultsPolyTree);

  if (resultsPolyTree.empty())
  {
    forestType = "DEAD";
  }
  else
  {
    bool found = false;
    for (std::size_t t = 0; t < resultsPolyTree.size(); ++t)
    {
      std::map<int, te::gm::Geometry*>::iterator it = m_polyGeomMap.find(resultsPolyTree[t]);

      te::gm::Geometry* g = it->second;

      g->setSRID(p->getSRID());

      bool covers = false;

      if (g->isValid())
      {
        covers = g->covers(p);
      }
      else
      {
        if (resultsPolyTree.size() == 1)
        {
          found = true;
          break;
        }
      } 

      if (covers)
      {
        found = true;
        break;
      }
    }

    if (found)
      forestType = "LIVE";
    else
      forestType = "DEAD";
  }

  //create dataset item
  te::mem::DataSetItem* item = new te::mem::DataSetItem(m_dataSet.get());

  //set id
  item->setInt32(0, m_starterId);

  //set origin id
  item->setInt32(1, parcelId);

  //set area
  item->setDouble(2, 0.);

  //forest type
  item->setString(3, forestType);

  //set geometry
  item->setGeometry(4, new te::gm::Point(*p));

  m_dataSet->add(item);

  ++m_starterId;
}

te::gm::Point* te::qt::plugins::tv5plugins::TrackAutoClassifier::getCandidatePoint(te::gm::Point* pRoot, te::gm::Point* pGuess, int srid, std::vector<int>& resultsTree, te::da::ObjectId*& candidateOjbId)
{
  double lowerDistance = std::numeric_limits<double>::max();

  te::gm::Point* point = 0;

  for (std::size_t t = 0; t < resultsTree.size(); ++t)
  {
    std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(resultsTree[t]);

    std::map<int, te::da::ObjectId*>::iterator itObjId = m_centroidObjIdMap.find(resultsTree[t]);

    if (!isClassified(itObjId->second))
    {
      te::gm::Point* pCandidate = getPoint(it->second);

      pCandidate->setSRID(srid);

      if (pRoot->getX() != pCandidate->getX() || pRoot->getY() != pCandidate->getY())
      {
        //check for lower distance from guest point
        double dist = std::abs(pGuess->distance(pCandidate));

        if (dist < lowerDistance)
        {
          lowerDistance = dist;
          point = pCandidate;
          candidateOjbId = itObjId->second;
        }
      }
    }
  }

  return point;
}

std::auto_ptr<te::da::DataSetType> te::qt::plugins::tv5plugins::TrackAutoClassifier::createTreeDataSetType()
{
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

  return dataSetType;
}
