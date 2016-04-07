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
  \file terralib/qt/plugins/thirdParty/PhotoIndexAction.cppp

  \brief This file defines the Photo Index Action class
*/

// Terralib
#include <terralib/qt/af/ApplicationController.h>
#include "qt/PhotoIndexDialog.h"
#include "PhotoIndexAction.h"

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

te::qt::plugins::tv5plugins::PhotoIndexAction::PhotoIndexAction(QMenu* menu):te::qt::plugins::tv5plugins::AbstractAction(menu)
{
  createAction(tr("Photo Index...").toStdString(), "");
}

te::qt::plugins::tv5plugins::PhotoIndexAction::~PhotoIndexAction()
{
}

void te::qt::plugins::tv5plugins::PhotoIndexAction::onActionActivated(bool checked)
{
  //show interface
  te::qt::plugins::tv5plugins::PhotoIndexDialog dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    addNewLayer(dlg.getOutputLayer());
  }
}
