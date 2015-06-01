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
\file terralib/qt/plugins/thirdParty/forestMonitor/qt/ForestMonitorToolBarDialog.cpp

\brief This interface is a tool bar for forest monitor classification operations.
*/

// TerraLib
#include "../core/ForestMonitorToolBar.h"
#include "tools/Eraser.h"
#include "ForestMonitorToolBarDialog.h"
#include "ui_ForestMonitorToolBarDialogForm.h"

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::ForestMonitorToolBarDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::ForestMonitorToolBarDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_eraserToolButton->setIcon(QIcon::fromTheme("pointer-remove-selection"));

  connect(m_ui->m_eraserToolButton, SIGNAL(toggled(bool)), this, SLOT(onEraserToolButtonClicked(bool)));
}

te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::~ForestMonitorToolBarDialog()
{
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  //clear combos
  m_ui->m_layerComboBox->clear();

  //fill combos
  std::list<te::map::AbstractLayerPtr>::iterator it = list.begin();

  while(it != list.end())
  {
    te::map::AbstractLayerPtr l = *it;

    if(l->isValid())
    {
      std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

      if (dsType->hasGeom())
      {
        m_ui->m_layerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
    }

    ++it;
  }
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::setMapDisplay(te::qt::af::MapDisplay* mapDisplay)
{
  m_appDisplay = mapDisplay;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onEraserToolButtonClicked(bool flag)
{
  if (!flag )
    return;

  if (!m_appDisplay)
    return;

  std::list<te::map::AbstractLayerPtr> list;

  QVariant varLayer = m_ui->m_layerComboBox->itemData(m_ui->m_layerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  list.push_back(layer);

  QPixmap pxmap = QIcon::fromTheme("pointer-remove-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::Eraser* tool = new te::qt::plugins::tv5plugins::Eraser(m_appDisplay->getDisplay(), cursor, list);
  m_appDisplay->setCurrentTool(tool);
}
