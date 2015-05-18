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
  \file terralib/qt/plugins/thirdParty/forestMonitor/qt/ForestMonitorDialog.h

  \brief This interface is used to get the input parameters for forest monitor information.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORDIALOG_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORDIALOG_H

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class ForestMonitorDialogForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        /*!
          \class ForestMonitorDialog

          \brief This interface is used to get the input parameters for forest monitor information.
        */
        class ForestMonitorDialog : public QDialog
        {
          Q_OBJECT

          public:

            ForestMonitorDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

            ~ForestMonitorDialog();

          public:

            void setLayerList(std::list<te::map::AbstractLayerPtr> list);

            te::map::AbstractLayerPtr getOutputLayer();

          protected slots:

            void onOkPushButtonClicked();

            void onTargetDatasourceToolButtonPressed();

            void onTargetFileToolButtonPressed();

          private:

            std::auto_ptr<Ui::ForestMonitorDialogForm> m_ui;

            te::da::DataSourceInfoPtr m_outputDatasource;

            te::map::AbstractLayerPtr m_outputLayer;                                          //!< Generated Layer.

            bool m_toFile;
        }; 
      }   // end namespace thirdParty
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORDIALOG_H

