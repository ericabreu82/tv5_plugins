/*  Copyright (C) 2008 National Institute For Space Research (INPE) - Brazil.

    This file is part of the TerraLib - a Framework for building GIS enabled applications.

    TerraLib is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    TerraLib is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TerraLib. See COPYING. If not, write to
    TerraLib Team at <terralib-team@terralib.org>.
 */

/*!
\file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/Creator.cpp

\brief This class implements a concrete tool to create points from layer
*/

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
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
#include "Creator.h"

// Qt
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

te::qt::plugins::tv5plugins::Creator::Creator(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_starterId(0)
{
  getStartIdValue();

  setCursor(cursor);

  display->setFocus();
}

te::qt::plugins::tv5plugins::Creator::~Creator()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);
}

bool te::qt::plugins::tv5plugins::Creator::eventFilter(QObject* watched, QEvent* e)
{
  if (e->type() == QEvent::MouseButtonRelease)
  {
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

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Control)
      saveObjects();

    if (event->key() == Qt::Key_Escape)
      cancelOperation();

    return true;
  }

  return false;
}

void te::qt::plugins::tv5plugins::Creator::selectObjects(QMouseEvent* e)
{
  if (!m_coordLayer.get() || !m_parcelLayer.get())
    return;

  QPointF pixelOffset(4.0, 4.0);
#if (QT_VERSION >= 0x050000)
  QPointF qtPoint = e->localPos();
#else
  QPointF qtPoint = e->posF();
#endif

  // Converts point to world coordinates
  QPointF qtWorldPoint = m_display->transform(qtPoint);

  // Bulding the query box
  te::gm::Point* point = new te::gm::Point(qtWorldPoint.x(), qtWorldPoint.y(), m_display->getSRID());

  if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
    point->transform(m_coordLayer->getSRID());

  //get parcel parent id
  int parcelId;
  if (!getParcelParentId(point, parcelId))
  {
    delete point;
    return;
  }

  if (!m_dataSet.get())
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

    m_dataSet.reset(new te::mem::DataSet(dataSetType.get()));
  }

  //create dataset item
  te::mem::DataSetItem* item = new te::mem::DataSetItem(m_dataSet.get());

  //set id
    item->setInt32(0, m_starterId);

  //set origin id
  int originIdPos = te::da::GetPropertyIndex(m_dataSet.get(), "originId");
  item->setInt32(1, parcelId);

  //set area
  int areaId = te::da::GetPropertyIndex(m_dataSet.get(), "area");
  item->setDouble(2, 0.);

  //forest type
  int typeId = te::da::GetPropertyIndex(m_dataSet.get(), "type");
  item->setString(3, "CREATED");

  //set geometry
  item->setGeometry(4, point);

  m_dataSet->add(item);

  ++m_starterId;

  drawSelecteds();

  //repaint the layer
  m_display->repaint();
}

void te::qt::plugins::tv5plugins::Creator::saveObjects()
{
  if (!m_coordLayer.get())
    return;

  try
  {
    //remove entries
    if (m_dataSet.get())
    {
      m_dataSet->moveBeforeFirst();

      std::auto_ptr<te::da::DataSetType> dsType = m_coordLayer->getSchema();

      te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_coordLayer.get());

      if (dsLayer)
      {
        te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

        std::map<std::string, std::string> options;

        dataSource->add(dsType->getName(), m_dataSet.get(), options);
      }
    }

  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error saving geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  m_dataSet.reset();

  //repaint the layer
  m_display->refresh();
}

void te::qt::plugins::tv5plugins::Creator::drawSelecteds()
{
  if (!m_dataSet.get())
    return;

  // Gets the layer schema
  std::size_t gmPos = te::da::GetFirstSpatialPropertyPos(m_dataSet.get());

  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& displayExtent = m_display->getExtent();

  te::qt::widgets::Canvas canvas(draft);
  canvas.setWindow(displayExtent.m_llx, displayExtent.m_lly, displayExtent.m_urx, displayExtent.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

  //set visual
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
    m_dataSet->moveBeforeFirst();

    while (m_dataSet->moveNext())
    {
      std::auto_ptr<te::gm::Geometry> g(m_dataSet->getGeometry(gmPos));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      if (g->getSRID() != m_display->getSRID())
        g->transform(m_display->getSRID());

      canvas.draw(g.get());
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }
}

bool te::qt::plugins::tv5plugins::Creator::getParcelParentId(te::gm::Point* point, int& id)
{
  if (!m_parcelLayer.get())
    throw;

  // Bulding the query box
  te::gm::Envelope envelope(point->getX(), point->getY(), point->getX(), point->getY());

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_parcelLayer->getSchema());

  if (!schema->hasGeom())
    throw;

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  te::da::PrimaryKey* pk = schema->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();

  // Gets the dataset
  std::auto_ptr<te::da::DataSet> dataset = m_parcelLayer->getData(gp->getName(), &envelope, te::gm::INTERSECTS);

  assert(dataset.get());

  dataset->moveBeforeFirst();

  std::auto_ptr<te::gm::Geometry> g;

  while (dataset->moveNext())
  {
    g = dataset->getGeometry(gp->getName());

    if (g->getSRID() == TE_UNKNOWN_SRS)
      g->setSRID(m_parcelLayer->getSRID());

    if (g->getSRID() != point->getSRID())
      g->transform(point->getSRID());

    if (g->covers(point))
    {
      id = dataset->getInt32(name);

      return true;
    }
  }

  return false;
}

void te::qt::plugins::tv5plugins::Creator::getStartIdValue()
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

void te::qt::plugins::tv5plugins::Creator::cancelOperation()
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_dataSet.reset();

  //repaint the layer
  m_display->repaint();
}
