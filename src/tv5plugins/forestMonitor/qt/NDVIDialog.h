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
  \file terralib/qt/plugins/thirdParty/forestMonitor/qt/NDVIDialog.h

  \brief This interface is used to get the input parameters for NDVI operation.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_NDVIDIALOG_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_NDVIDIALOG_H

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class NDVIDialogForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        /*!
          \class NDVIDialog

          \brief This interface is used to get the input parameters for NDVI operation.
        */
        class NDVIDialog : public QDialog
        {
          Q_OBJECT

          public:

            NDVIDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

            ~NDVIDialog();

          public:

            void setLayerList(std::list<te::map::AbstractLayerPtr> list);

            te::map::AbstractLayerPtr getOutputLayer();

          protected slots:

            void onNIRLayerCmbActivated(int idx);

            void onVISLayerCmbActivated(int idx);

            void onExportLayerCmbActivated(int idx);

            void onOkPushButtonClicked();

            void onTargetFileToolButtonPressed();

          private:

            std::auto_ptr<Ui::NDVIDialogForm> m_ui;

            te::map::AbstractLayerPtr m_outputLayer;                                          //!< Generated Layer.
        }; 
      }   // end namespace thirdParty
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_NDVIDIALOG_H

