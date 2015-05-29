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
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/dataset/ObjectIdSet.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/DataSetLayer.h>
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
    m_layer(layer)
{
  setCursor(cursor);
}

te::qt::plugins::tv5plugins::Eraser::~Eraser()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);
}

bool te::qt::plugins::tv5plugins::Eraser::mouseReleaseEvent(QMouseEvent* e)
{
  if(e->button() != Qt::LeftButton)
    return false;

  if(!m_layer.get())
    return false;

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
    return false;

  try
  {
    // Gets the layer schema
    std::auto_ptr<const te::map::LayerSchema> schema(m_layer->getSchema());

    if (!schema->hasGeom())
      return false;

    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::auto_ptr<te::da::DataSet> dataset = m_layer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    // Let's generate the oids
    te::da::ObjectIdSet* oids = 0;
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
     
    //remove entries
    if (oids->size() != 0)
    {
      te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_layer.get());

      if (dsLayer)
      {
        te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

        dataSource->remove(schema->getName(), oids);
      }
    }

  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return false;
  }

  //repaint the layer
  m_display->refresh();

  return true;
}

void te::qt::plugins::tv5plugins::Eraser::setLayer(te::map::AbstractLayerPtr layer)
{
  m_layer = layer;
}

