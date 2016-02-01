/*  Copyright (C) 2008-2014 National Institute For Space Research (INPE) - Brazil.

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
  \file terralib/qt/plugins/thirdParty/Config.h

  \brief Configuration flags for the third Party Qt Widget plugin.
*/

#ifndef __TERRALIB_QT_PLUGINS_THIRDPARTY_INTERNAL_CONFIG_H
#define __TERRALIB_QT_PLUGINS_THIRDPARTY_INTERNAL_CONFIG_H

// TerraLib
//#include <terralib_config.h>


/*!
  \def TE_QT_PLUGIN_THIRDPARTY_HAVE_TILEGENERATOR

  \brief It defines if the third Party Qt Plugin has the tile generator operation.
*/
#define TE_QT_PLUGIN_THIRDPARTY_HAVE_TILEGENERATOR

/*!
  \def TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITOR

  \brief It defines if the third Party Qt Plugin has the forest monitor operation.
*/
#define TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITOR

/*!
  \def TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORCLASS

  \brief It defines if the third Party Qt Plugin has the forest monitor class operation.
*/
#define TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORCLASS

/*!
\def TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORTOOLBAR

\brief It defines if the third Party Qt Plugin has the forest monitor tool bar
*/
#define TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORTOOLBAR

/*!
  \def TE_QT_PLUGIN_THIRDPARTY_HAVE_NDVI

  \brief It defines if the third Party Qt Plugin has the NDVI operation.
*/
#define TE_QT_PLUGIN_THIRDPARTY_HAVE_NDVI

/*!
  \def TE_QT_PLUGIN_THIRDPARTY_HAVE_PHOTOINDEX

  \brief It defines if the third Party Qt Plugin has the photo index operation.
*/
#define TE_QT_PLUGIN_THIRDPARTY_HAVE_PHOTOINDEX

/*!
\def TE_QT_PLUGIN_THIRDPARTY_HAVE_PROXIMITY

\brief It defines if the third Party Qt Plugin has the proximity operation.
*/
#define TE_QT_PLUGIN_THIRDPARTY_HAVE_PROXIMITY

/*!
  \def TE_QT_PLUGIN_THIRDPARTY_PLUGIN_NAME

  \brief It contains the plugin name.
*/
#define TE_QT_PLUGIN_THIRDPARTY_PLUGIN_NAME "te.qt.tv5plugins"

/*!
  \def TEQTPLUGINTHIRDPARTYEXPORT

  \brief You can use this macro in order to export/import classes and functions from all plug-ins files.

  \note To compile plug-ins in Windows, remember to insert TEQTPLUGINTHIRDPARTYEXPORT into the project's list of defines.
 */
#ifdef WIN32
  #ifdef TV5PLUGINSDLL
    #define TV5PLUGINSDLLEXPORT  __declspec(dllexport)   // export DLL information
  #else
    #define TV5PLUGINSDLLEXPORT  __declspec(dllimport)   // import DLL information
  #endif 
#else
  #define TV5PLUGINSDLLEXPORT
#endif

#endif  // __TERRALIB_QT_PLUGINS_THIRDPARTY_INTERNAL_CONFIG_H

