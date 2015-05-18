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
  \file terralib/qt/plugins/thirdParty/tileGenerator/qt/TileGeneratorDialog.h

  \brief This interface i used to get the input parameters for  tile generation.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILEGENERATORDIALOG_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILEGENERATORDIALOG_H

// TerraLib
#include <terralib/geometry/Envelope.h>
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class TileGeneratorDialogForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        /*!
          \class TileGeneratorDialog

          \brief This interface i used to get the input parameters for  tile generation.
        */
        class TileGeneratorDialog : public QDialog
        {
          Q_OBJECT

          public:

            TileGeneratorDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

            ~TileGeneratorDialog();

          public:

            void setExtentInfo(te::gm::Envelope env, int srid);

            void setLayerList(std::list<te::map::AbstractLayerPtr> list);

          protected slots:

            void onDirToolButtonClicked();

            void onValidatePushButtonClicked();

            void onOkPushButtonClicked();

          private:

            std::auto_ptr<Ui::TileGeneratorDialogForm> m_ui;

            std::list<te::map::AbstractLayerPtr> m_layerList;

            te::gm::Envelope m_env;

            int m_srid;
        }; 
      }   // end namespace thirdParty
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILEGENERATORDIALOG_H

