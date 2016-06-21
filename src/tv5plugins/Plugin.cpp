/*  Copyright (C) 2008-2013 National Institute For Space Research (INPE) - Brazil.

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
  \file terralib/qt/plugins/thirdParty/Plugin.cpp

  \brief Plugin implementation for the RP Qt Plugin widget.
*/

// TerraLib
#include <terralib/core/translator/Translator.h>
#include <terralib/common/Logger.h>
#include <terralib/qt/af/ApplicationController.h>

#include "Plugin.h"

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITOR
  #include "forestMonitor/ForestMonitorAction.h"
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORCLASS
  #include "forestMonitor/ForestMonitorClassAction.h"
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORTOOLBAR
#include "forestMonitor/ForestMonitorToolBarAction.h"
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_NDVI
  #include "forestMonitor/NDVIAction.h"
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_TILEGENERATOR
  #include "tileGenerator/TileGeneratorAction.h"
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_PHOTOINDEX
  #include "photoIndex/PhotoIndexAction.h"
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_PROXIMITY
#include "proximity/ProximityAction.h"
#endif

// QT
#include <QMenu>
#include <QMenuBar>

te::qt::plugins::tv5plugins::Plugin::Plugin(const te::plugin::PluginInfo& pluginInfo)
  : QObject(), te::plugin::Plugin(pluginInfo), m_menu(0)
{
}

te::qt::plugins::tv5plugins::Plugin::~Plugin()
{
}

void te::qt::plugins::tv5plugins::Plugin::startup()
{
  if(m_initialized)
    return;

  te::qt::af::AppCtrlSingleton::getInstance().addListener(this, te::qt::af::SENDER);

  TE_LOG_TRACE(TE_TR("TerraLib Qt Third Party Plugin startup!"));

// add plugin menu
  m_menu = te::qt::af::AppCtrlSingleton::getInstance().getMenu("thirdParty");

  m_menu->setTitle(TE_TR("Third Party"));

// add pop up menu
  m_popupAction = new QAction(m_menu);
  m_popupAction->setText(TE_TR("Third Party"));

// register actions
  registerActions();

  m_initialized = true;
}

void te::qt::plugins::tv5plugins::Plugin::shutdown()
{
  if(!m_initialized)
    return;

// remove menu
  delete m_menu;

// unregister actions
  unRegisterActions();

  TE_LOG_TRACE(TE_TR("TerraLib Qt Third Party Plugin shutdown!"));

  m_initialized = false;
}

void te::qt::plugins::tv5plugins::Plugin::registerActions()
{
#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITOR
  m_forestMonitor = new te::qt::plugins::tv5plugins::ForestMonitorAction(m_menu);
  connect(m_forestMonitor, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORCLASS
  m_forestMonitorClass = new te::qt::plugins::tv5plugins::ForestMonitorClassAction(m_menu);
  connect(m_forestMonitorClass, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORTOOLBAR
  m_forestMonitorToolBar = new te::qt::plugins::tv5plugins::ForestMonitorToolBarAction(m_menu);
  connect(m_forestMonitorToolBar, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_NDVI
  m_ndvi = new te::qt::plugins::tv5plugins::NDVIAction(m_menu);
  connect(m_ndvi, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_TILEGENERATOR
  m_tileGenerator = new te::qt::plugins::tv5plugins::TileGeneratorAction(m_menu);
  connect(m_tileGenerator, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_PHOTOINDEX
  m_photoIndex = new te::qt::plugins::tv5plugins::PhotoIndexAction(m_menu);
  connect(m_photoIndex, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_PROXIMITY
  m_proximity = new te::qt::plugins::tv5plugins::ProximityAction(m_menu);
  connect(m_proximity, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif
}

void  te::qt::plugins::tv5plugins::Plugin::unRegisterActions()
{
#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITOR
    delete m_forestMonitor;
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORCLASS
    delete m_forestMonitorClass;
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_FORESTMONITORTOOLBAR
    delete m_forestMonitorToolBar;
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_NDVI
    delete m_ndvi;
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_TILEGENERATOR
    delete m_tileGenerator;
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_PHOTOINDEX
    delete m_photoIndex;
#endif

#ifdef TE_QT_PLUGIN_THIRDPARTY_HAVE_PROXIMITY
    delete m_proximity;
#endif
}

PLUGIN_CALL_BACK_IMPL(te::qt::plugins::tv5plugins::Plugin)
