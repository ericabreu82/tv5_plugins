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
\file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/Eraser.cpp

\brief This class implements a concrete tool to remove points from layer
*/

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/DataSetLayer.h>
#include <terralib/maptools/MarkRendererManager.h>
#include <terralib/se/Fill.h>
#include <terralib/se/Stroke.h>
#include <terralib/se/Mark.h>
#include <terralib/se/Utils.h>
#include <terralib/qt/widgets/canvas/Canvas.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include "Eraser.h"

// Qt
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

te::qt::plugins::tv5plugins::Eraser::Eraser(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr layer, QObject* parent)
  : AbstractTool(display, parent),
    m_layer(layer),
    m_objIdSet(0)
{
  setCursor(cursor);

  display->setFocus();
}

te::qt::plugins::tv5plugins::Eraser::~Eraser()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  delete m_objIdSet;
}

bool te::qt::plugins::tv5plugins::Eraser::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton)
  {
    selectObjects(e);

    return true;
  }

  return false;
}

bool te::qt::plugins::tv5plugins::Eraser::eventFilter(QObject* watched, QEvent* e)
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

    if (event->key() == Qt::Key_Delete)
      removeObjects();

    return true;
  }

  return false;
}

void te::qt::plugins::tv5plugins::Eraser::setLayer(te::map::AbstractLayerPtr layer)
{
  m_layer = layer;
}

void te::qt::plugins::tv5plugins::Eraser::selectObjects(QMouseEvent* e)
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

  te::da::ObjectIdSet* oids = 0;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::auto_ptr<te::da::DataSet> dataset = m_layer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    // Let's generate the oids
    te::da::GetEmptyOIDSet(schema.get(), oids);
    assert(oids);

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

void te::qt::plugins::tv5plugins::Eraser::removeObjects()
{
  if (!m_layer.get())
    return;

  try
  {
    //remove entries
    if (m_objIdSet->size() != 0)
    {
      te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_layer.get());

      if (dsLayer)
      {
        std::auto_ptr<const te::map::LayerSchema> schema(m_layer->getSchema());

        te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

        dataSource->remove(schema->getName(), m_objIdSet);

        delete m_objIdSet;
        m_objIdSet = 0;
      }
    }

  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  //repaint the layer
  m_display->refresh();
}

void te::qt::plugins::tv5plugins::Eraser::drawSelecteds()
{
  if (!m_layer.get())
    return;

  if (!m_objIdSet || m_objIdSet->size() == 0)
  {
    return;
  }

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
    std::auto_ptr<te::da::DataSet> dataset = m_layer->getData(m_objIdSet);
    assert(dataset.get());

    while (dataset->moveNext())
    {
      std::auto_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_layer->getSRID());

      canvas.draw(g.get());
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }
}

