/*  Copyright (C) 2011-2012 National Institute For Space Research (INPE) - Brazil.

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
\file terralib/qt/plugins/thirdParty/ForestMonitorToolBarAction.cpp

\brief This file defines the Forest Monitor Tool Bar Action class
*/

// Terralib
#include <terralib/qt/af/connectors/MapDisplay.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>
#include <terralib/qt/af/Project.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include "ForestMonitorToolBarAction.h"

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

te::qt::plugins::tv5plugins::ForestMonitorToolBarAction::ForestMonitorToolBarAction(QMenu* menu) :te::qt::plugins::tv5plugins::AbstractAction(menu)
{
  createAction(tr("Forest Monitor Tool Bar...").toStdString(), "");

  m_dlg = 0;
}

te::qt::plugins::tv5plugins::ForestMonitorToolBarAction::~ForestMonitorToolBarAction()
{
  delete m_dlg;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarAction::onActionActivated(bool checked)
{
  //get input layers
  te::qt::af::Project* prj = te::qt::af::ApplicationController::getInstance().getProject();

  std::list<te::map::AbstractLayerPtr> list;

  if(prj)
    list = prj->getVisibleSingleLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::ApplicationController::getInstance().getMainWindow());

  if (!ba || !ba->getDisplay())
  {
    return;
  }

  //show interface
  if (m_dlg)
    delete m_dlg;

  m_dlg = new te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog(te::qt::af::ApplicationController::getInstance().getMainWindow(), Qt::Tool);

  m_dlg->setLayerList(list);

  m_dlg->setMapDisplay(ba->getDisplay());

  m_dlg->show();
}
