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
  \file terralib/qt/plugins/thirdParty/ForestMonitorClassAction.cppp

  \brief This file defines the Forest Monitor Class action class.
*/

// Terralib
#include <terralib/qt/af/connectors/MapDisplay.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include "qt/ForestMonitorClassDialog.h"
#include "ForestMonitorClassAction.h"

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

te::qt::plugins::tv5plugins::ForestMonitorClassAction::ForestMonitorClassAction(QMenu* menu):te::qt::plugins::tv5plugins::AbstractAction(menu)
{
  createAction(tr("Forest Monitor Class...").toStdString(), "");
}

te::qt::plugins::tv5plugins::ForestMonitorClassAction::~ForestMonitorClassAction()
{
}

void te::qt::plugins::tv5plugins::ForestMonitorClassAction::onActionActivated(bool checked)
{
  //get input layers
  std::list<te::map::AbstractLayerPtr> layersList = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  te::gm::Envelope env;
  int srid = TE_UNKNOWN_SRS;

  if (ba && ba->getMapDisplay())
  {
    env = ba->getMapDisplay()->getExtent();
    srid = ba->getMapDisplay()->getSRID();
  }

  //show interface
  te::qt::plugins::tv5plugins::ForestMonitorClassDialog dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  dlg.setLayerList(layersList);

  dlg.setExtentInfo(env, srid);

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    addNewLayer(dlg.getOutputLayer());
  }
}
