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

te::qt::plugins::tv5plugins::Eraser::Eraser(te::qt::widgets::MapDisplay* display, const QCursor& cursor, const std::list<te::map::AbstractLayerPtr>& layers, QObject* parent)
  : AbstractTool(display, parent),
    m_layers(layers)
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

  if(m_layers.empty())
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

  m_display->repaint();

  //remove geom using datasource - execute query

  return true;
}

void te::qt::plugins::tv5plugins::Eraser::setLayers(const std::list<te::map::AbstractLayerPtr>& layers)
{
  m_layers = layers;
}

