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
#include "tools/Creator.h"
#include "tools/Eraser.h"
#include "tools/TrackClassifier.h"
#include "tools/TrackDeadClassifier.h"
#include "tools/TrackAutoClassifier.h"
#include "tools/UpdateClass.h"
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
  m_ui->m_updateToolButton->setIcon(QIcon::fromTheme("view-refresh"));
  m_ui->m_creatorToolButton->setIcon(QIcon::fromTheme("bullet-blue"));
  m_ui->m_creatorLiveToolButton->setIcon(QIcon::fromTheme("bullet-green"));
  m_ui->m_creatorDeadToolButton->setIcon(QIcon::fromTheme("bullet-black"));
  m_ui->m_trackAutoClassifierToolButton->setIcon(QIcon::fromTheme("wand"));
  m_ui->m_trackDeadClassifierToolButton->setIcon(QIcon::fromTheme("vp-line-length-hint"));

  connect(m_ui->m_eraserToolButton, SIGNAL(toggled(bool)), this, SLOT(onEraserToolButtonClicked(bool)));
  connect(m_ui->m_updateToolButton, SIGNAL(toggled(bool)), this, SLOT(onUpdateToolButtonClicked(bool)));
  connect(m_ui->m_creatorToolButton, SIGNAL(toggled(bool)), this, SLOT(onCreatorToolButtonClicked(bool)));
  connect(m_ui->m_creatorLiveToolButton, SIGNAL(toggled(bool)), this, SLOT(onCreatorLiveToolButtonClicked(bool)));
  connect(m_ui->m_creatorDeadToolButton, SIGNAL(toggled(bool)), this, SLOT(onCreatorDeadToolButtonClicked(bool)));
  connect(m_ui->m_trackAutoClassifierToolButton, SIGNAL(toggled(bool)), this, SLOT(onTrackAutoClassifierToolButtonClicked(bool)));
  connect(m_ui->m_trackDeadClassifierToolButton, SIGNAL(toggled(bool)), this, SLOT(onTrackDeadClassifierToolButtonClicked(bool)));
  

  m_ui->m_distLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_distTolLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_distBufferLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_polyAreaMinLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_polyAreaMaxLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_maxDeadLineEdit->setValidator(new QIntValidator(this));
  m_ui->m_deadTolLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_thresholdLineEdit->setValidator(new QDoubleValidator(this));
}

te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::~ForestMonitorToolBarDialog()
{
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  //clear combos
  m_ui->m_layerParcelComboBox->clear();
  m_ui->m_layerPointsComboBox->clear();
  m_ui->m_layerPolyComboBox->clear();
  m_ui->m_layerDirComboBox->clear();

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
        m_ui->m_layerParcelComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_layerPointsComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_layerDirComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
      else if (dsType->hasRaster())
      {
        m_ui->m_layerPolyComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
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

  QVariant varLayer = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  QPixmap pxmap = QIcon::fromTheme("pointer-remove-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::Eraser* tool = new te::qt::plugins::tv5plugins::Eraser(m_appDisplay->getDisplay(), cursor, layer);
  m_appDisplay->setCurrentTool(tool);
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onUpdateToolButtonClicked(bool flag)
{
  if (!flag)
    return;

  if (!m_appDisplay)
    return;

  QVariant varLayer = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  QPixmap pxmap = QIcon::fromTheme("pointer-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::UpdateClass* tool = new te::qt::plugins::tv5plugins::UpdateClass(m_appDisplay->getDisplay(), cursor, layer);
  m_appDisplay->setCurrentTool(tool);
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onCreatorToolButtonClicked(bool flag)
{
  if (!flag)
    return;

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QPixmap pxmap = QIcon::fromTheme("pointer-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::Creator* tool = new te::qt::plugins::tv5plugins::Creator(m_appDisplay->getDisplay(), cursor, layerPoints, layerParcel, te::qt::plugins::tv5plugins::CREATED_TYPE);
  m_appDisplay->setCurrentTool(tool);
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onCreatorLiveToolButtonClicked(bool flag)
{
  if (!flag)
    return;

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QPixmap pxmap = QIcon::fromTheme("pointer-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::Creator* tool = new te::qt::plugins::tv5plugins::Creator(m_appDisplay->getDisplay(), cursor, layerPoints, layerParcel, te::qt::plugins::tv5plugins::LIVE_TYPE);
  m_appDisplay->setCurrentTool(tool);
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onCreatorDeadToolButtonClicked(bool flag)
{
  if (!flag)
    return;

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QPixmap pxmap = QIcon::fromTheme("pointer-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::Creator* tool = new te::qt::plugins::tv5plugins::Creator(m_appDisplay->getDisplay(), cursor, layerPoints, layerParcel, te::qt::plugins::tv5plugins::DEAD_TYPE);
  m_appDisplay->setCurrentTool(tool);
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onTrackAutoClassifierToolButtonClicked(bool flag)
{
  if (!flag)
    return;

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QVariant varLayerPoly = m_ui->m_layerPolyComboBox->itemData(m_ui->m_layerPolyComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoly = varLayerPoly.value<te::map::AbstractLayerPtr>();

  QVariant varLayerDir = m_ui->m_layerDirComboBox->itemData(m_ui->m_layerDirComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerDir = varLayerDir.value<te::map::AbstractLayerPtr>();

  QPixmap pxmap = QIcon::fromTheme("pointer-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::TrackAutoClassifier* tool = new te::qt::plugins::tv5plugins::TrackAutoClassifier(m_appDisplay->getDisplay(), cursor, layerPoints, layerParcel, layerPoly, layerDir);
  tool->setLineEditComponents(m_ui->m_distLineEdit, m_ui->m_distBufferLineEdit, m_ui->m_distTolLineEdit, m_ui->m_polyAreaMinLineEdit, m_ui->m_polyAreaMaxLineEdit, m_ui->m_maxDeadLineEdit, m_ui->m_deadTolLineEdit, m_ui->m_thresholdLineEdit);
  m_appDisplay->setCurrentTool(tool);
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onTrackDeadClassifierToolButtonClicked(bool flag)
{
  if (!flag)
    return;

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QVariant varLayerPoly = m_ui->m_layerPolyComboBox->itemData(m_ui->m_layerPolyComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoly = varLayerPoly.value<te::map::AbstractLayerPtr>();

  QPixmap pxmap = QIcon::fromTheme("pointer-selection").pixmap(QSize(16, 16));
  QCursor cursor(pxmap, 0, 0);

  te::qt::plugins::tv5plugins::TrackDeadClassifier* tool = new te::qt::plugins::tv5plugins::TrackDeadClassifier(m_appDisplay->getDisplay(), cursor, layerPoints, layerParcel, layerPoly);
  tool->setLineEditComponents(m_ui->m_distLineEdit, m_ui->m_distTolLineEdit, m_ui->m_thresholdLineEdit);
  m_appDisplay->setCurrentTool(tool);
}
