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
  \file terralib/qt/plugins/thirdParty/Plugin.h

  \brief Plugin implementation for the RP Qt Plugin widget.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PLUGIN_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PLUGIN_H

// TerraLib
#include <terralib/core/plugin/CppPlugin.h>
#include "Config.h"

// Qt
#include <QAction>
#include <QMenu>

namespace te
{
  namespace qt
  {
    namespace af
    {
      namespace evt
      {
        struct Event;
      }
    }
    namespace plugins
    {
      namespace tv5plugins
      {
        class ForestMonitorAction;
        class ForestMonitorClassAction;
        class ForestMonitorToolBarAction;
        class NDVIAction;
        class PhotoIndexAction;
        class ProximityAction;
        class TileGeneratorAction;
        
        class Plugin : public QObject, public te::core::CppPlugin
        {
          Q_OBJECT

          public:

            Plugin(const te::core::PluginInfo& pluginInfo);

            ~Plugin();

            void startup();

            void shutdown();

          protected:

            /*!
              \brief Function used to register all raster processing actions.

            */
            void registerActions();

            /*!
              \brief Function used to unregister all raster processing actions.

            */
            void unRegisterActions();

          Q_SIGNALS:

            void triggered(te::qt::af::evt::Event* e);

          protected:

            QMenu* m_menu;                                           //!< thirdParty Main Menu registered.
            QAction* m_popupAction;                                  //!< thirdParty pop up action registered.

            te::qt::plugins::tv5plugins::TileGeneratorAction* m_tileGenerator;              //!< Tile Generator Operation Process Action
            te::qt::plugins::tv5plugins::ForestMonitorAction* m_forestMonitor;              //!< Forest Monitor Operation Process Action
            te::qt::plugins::tv5plugins::ForestMonitorClassAction* m_forestMonitorClass;    //!< Forest Monitor Class Operation Process Action
            te::qt::plugins::tv5plugins::ForestMonitorToolBarAction* m_forestMonitorToolBar;//!< Forest Monitor Tool Bar Operation Process Action
            te::qt::plugins::tv5plugins::NDVIAction* m_ndvi;                                //!< NDVI Operation Process Action
            te::qt::plugins::tv5plugins::PhotoIndexAction* m_photoIndex;                    //!< Photo Index Operation Process Action
            te::qt::plugins::tv5plugins::ProximityAction* m_proximity;                      //!< Proximity Operation Process Action
        };

      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PLUGIN_H
