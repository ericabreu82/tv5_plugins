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
\file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/UpdateClass.cpp

\brief This class implements a concrete tool to Update Class points from layer
*/

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
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
#include "UpdateClass.h"

// Qt
#include <QtCore/QPointF>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

te::qt::plugins::tv5plugins::UpdateClass::UpdateClass(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr layer, QObject* parent)
  : AbstractTool(display, parent),
    m_layer(layer),
    m_roots(0)
{
  setCursor(cursor);

  display->setFocus();
}

te::qt::plugins::tv5plugins::UpdateClass::~UpdateClass()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  delete m_roots;
}

bool te::qt::plugins::tv5plugins::UpdateClass::eventFilter(QObject* watched, QEvent* e)
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

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Control)
      updateClassObjects();

    if (event->key() == Qt::Key_Escape)
      cancelOperation();

    return true;
  }

  return false;
}

void te::qt::plugins::tv5plugins::UpdateClass::setLayer(te::map::AbstractLayerPtr layer)
{
  m_layer = layer;
}

void te::qt::plugins::tv5plugins::UpdateClass::selectObjects(QMouseEvent* e)
{
  if (!m_layer.get())
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

  if ((m_layer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_layer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_layer->getSRID());

  if (!reprojectedEnvelope.intersects(m_layer->getExtent()))
    return;

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_layer->getSchema());

  if (!schema->hasGeom())
    return;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::auto_ptr<te::da::DataSet> dataset = m_layer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    // Let's generate the oid
    std::vector<std::string> pnames;
    te::da::GetOIDPropertyNames(schema.get(), pnames);

    // Generates a geometry from the given extent. It will be used to refine the results
    std::auto_ptr<te::gm::Geometry> geometryFromEnvelope(te::gm::GetGeomFromEnvelope(&reprojectedEnvelope, m_layer->getSRID()));

    while (dataset->moveNext())
    {
      std::auto_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_layer->getSRID());

      if (!g->intersects(geometryFromEnvelope.get()))
        continue;

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

void te::qt::plugins::tv5plugins::UpdateClass::updateClassObjects()
{
  if (!m_layer.get())
    return;

  if (!m_roots)
  {
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // Gets the layer schema
  std::auto_ptr<te::da::DataSetType> schema(m_layer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  try
  {
    // Gets the dataset
    std::auto_ptr<te::da::DataSet> dataset = m_layer->getData(m_roots);

    assert(dataset.get());

    te::mem::DataSet* liveDataSet = new te::mem::DataSet(schema.get());

    //create dataset updated
    while (dataset->moveNext())
    {
      std::auto_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_layer->getSRID());

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

      std::string currentClass = dataset->getString("type");

      std::string outputClass;

      if (currentClass == "LIVE")
      {
        outputClass = "DEAD";
      }
      else
      {
        outputClass = "LIVE";
      }

      //forest type
      item->setString(4, outputClass);

      //set geometry
      item->setGeometry(5, g.release());

      liveDataSet->add(item);
    }


    //save dataset updated
    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_layer.get());

    if (dsLayer)
    {
      te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

      //update live dataset
      std::vector<size_t> ids;
      ids.push_back(0);

      if (liveDataSet)
      {
        std::vector< std::set<int> > properties;
        std::size_t dsSize = liveDataSet->size();

        for (std::size_t t = 0; t < dsSize; ++t)
        {
          std::set<int> setPos;
          setPos.insert(4);

          properties.push_back(setPos);
        }

        liveDataSet->moveBeforeFirst();

        dataSource->update(schema->getName(), liveDataSet, properties, ids);
      }
    }
  }
  catch (std::exception& e)
  {
    QApplication::restoreOverrideCursor();

    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  QApplication::restoreOverrideCursor();

  delete m_roots;
  m_roots = 0;

  //repaint the layer
  m_display->refresh();
}

void te::qt::plugins::tv5plugins::UpdateClass::drawSelecteds()
{
  if (!m_layer.get())
    return;

  // Gets the layer schema
  std::auto_ptr<const te::map::LayerSchema> schema(m_layer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& displayExtent = m_display->getExtent();

  te::qt::widgets::Canvas canvas(draft);
  canvas.setWindow(displayExtent.m_llx, displayExtent.m_lly, displayExtent.m_urx, displayExtent.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

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
    if (m_roots)
    {
      std::auto_ptr<te::da::DataSet> dsCoords = m_layer->getData(m_roots);

      while (dsCoords->moveNext())
      {
        std::auto_ptr<te::gm::Geometry> g(dsCoords->getGeometry(gp->getName()));

        if (g->getSRID() == TE_UNKNOWN_SRS)
          g->setSRID(m_layer->getSRID());

        canvas.draw(g.get());
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }
}

void te::qt::plugins::tv5plugins::UpdateClass::cancelOperation()
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  delete m_roots;
  m_roots = 0;

  //repaint the layer
  m_display->repaint();
}

