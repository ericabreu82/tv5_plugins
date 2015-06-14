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
  \file terralib/qt/plugins/thirdParty/forestMonitor/qt/ForestMonitorClassDialog.h

  \brief This interface is used to get the input parameters for forest monitor classification information.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSDIALOG_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSDIALOG_H

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/geometry/Envelope.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/raster/Raster.h>
#include <terralib/se/Style.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class ForestMonitorClassDialogForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        /*!
          \class ForestMonitorClassDialog

          \brief This interface is used to get the input parameters for forest monitor classification information.
        */
        class ForestMonitorClassDialog : public QDialog
        {
          Q_OBJECT

          public:

            ForestMonitorClassDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

            ~ForestMonitorClassDialog();

          public:

            void setExtentInfo(te::gm::Envelope env, int srid);

            void setLayerList(std::list<te::map::AbstractLayerPtr> list);

            te::map::AbstractLayerPtr getOutputLayer();

          protected slots:

            void onGenerateNDVISampleClicked();

            void onThresholdSliderReleased();

            void onGenerateErosionSampleClicked();

            void onDilationPushButtonClicked();

            void onErosionPushButtonClicked();

            void onTargetFileToolButtonPressed();

            void onOkPushButtonClicked();

          protected:

            void drawRaster(te::rst::Raster* raster, te::qt::widgets::MapDisplay* mapDisplay, te::se::Style* style = 0);

            te::da::DataSourcePtr createDataSource(std::string repository, std::map<std::string, std::string>& dsInfo);

          private:

            std::auto_ptr<Ui::ForestMonitorClassDialogForm> m_ui;

            std::auto_ptr<te::qt::widgets::MapDisplay> m_thresholdDisplay;

            std::auto_ptr<te::qt::widgets::MapDisplay> m_erosionDisplay;

            std::auto_ptr<te::rst::Raster> m_thresholdRaster;

            std::auto_ptr<te::rst::Raster> m_filterRaster;

            std::auto_ptr<te::rst::Raster> m_filterDilRaster;
            
            std::auto_ptr<te::se::Style> m_styleThresholdRaster;

            te::map::AbstractLayerPtr m_outputLayer;                                          //!< Generated Layer.

            te::qt::widgets::ProgressViewerDialog* m_progressDlg;

            int m_progressId;

            te::gm::Envelope m_env;

            int m_srid;
        }; 
      }   // end namespace thirdParty
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSDIALOG_H

