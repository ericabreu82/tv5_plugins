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
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>

#include "../core/ForestMonitorToolBar.h"
#include "tools/Creator.h"
#include "tools/Eraser.h"
#include "tools/TrackDeadClassifier.h"
#include "tools/TrackAutoClassifier.h"
#include "tools/UpdateClass.h"
#include "ForestMonitorToolBarDialog.h"
#include "ui_ForestMonitorToolBarDialogForm.h"

#include <QMessageBox>
//Qt

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::ForestMonitorToolBarDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::ForestMonitorToolBarDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_distLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_distTolLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_distTrackLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_polyAreaMinLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_polyAreaMaxLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_maxDeadLineEdit->setValidator(new QIntValidator(this));
  m_ui->m_deadTolLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_thresholdLineEdit->setValidator(new QDoubleValidator(this));

  createActions();
}

te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::~ForestMonitorToolBarDialog()
{
  if (m_clearTool)
    m_appDisplay->getDisplay()->setCurrentTool(0);
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::reject()
{
  QMessageBox::StandardButton resBtn = QMessageBox::Yes;

  resBtn = QMessageBox::question(this, tr("Forest Monitor"), tr("Close Dialog?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

  if (resBtn == QMessageBox::Yes)
    QDialog::reject();
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

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::createActions()
{
  QActionGroup* toolsGroup = te::qt::af::AppCtrlSingleton::getInstance().findActionGroup("Map.ToolsGroup");

  //create actions
  m_actionEraser = new QAction(this);
  m_actionEraser->setIcon(QIcon::fromTheme("pointer-remove-selection"));
  m_actionEraser->setToolTip("Erases a existent object.");
  m_actionEraser->setCheckable(true);
  m_actionEraser->setEnabled(true);
  m_actionEraser->setObjectName("eraser_Tool");

  m_actionUpdate = new QAction(this);
  m_actionUpdate->setIcon(QIcon::fromTheme("view-refresh"));
  m_actionUpdate->setToolTip("Updates a class to Live or if Live turns to Dead.");
  m_actionUpdate->setCheckable(true);
  m_actionUpdate->setEnabled(true);
  m_actionUpdate->setObjectName("update_Tool");

  m_actionCreator = new QAction(this);
  m_actionCreator->setIcon(QIcon::fromTheme("bullet-blue"));
  m_actionCreator->setToolTip("Creates a new point with CREATED class.");
  m_actionCreator->setCheckable(true);
  m_actionCreator->setEnabled(true);
  m_actionCreator->setObjectName("created_Tool");

  m_actionCreatorLive = new QAction(this);
  m_actionCreatorLive->setIcon(QIcon::fromTheme("bullet-green"));
  m_actionCreatorLive->setToolTip("Creates a new point with LIVE class.");
  m_actionCreatorLive->setCheckable(true);
  m_actionCreatorLive->setEnabled(true);
  m_actionCreatorLive->setObjectName("live_Tool");

  m_actionCreatorDead = new QAction(this);
  m_actionCreatorDead->setIcon(QIcon::fromTheme("bullet-black"));
  m_actionCreatorDead->setToolTip("Creates a new point with DEAD class.");
  m_actionCreatorDead->setCheckable(true);
  m_actionCreatorDead->setEnabled(true);
  m_actionCreatorDead->setObjectName("dead_Tool");

  m_actionAutoTrack = new QAction(this);
  m_actionAutoTrack->setIcon(QIcon::fromTheme("wand"));
  m_actionAutoTrack->setToolTip("Automatic Track Classifier.");
  m_actionAutoTrack->setCheckable(true);
  m_actionAutoTrack->setEnabled(true);
  m_actionAutoTrack->setObjectName("autoTrack_Tool");

  m_actionDeadTrack = new QAction(this);
  m_actionDeadTrack->setIcon(QIcon::fromTheme("vp-line-length-hint"));
  m_actionDeadTrack->setToolTip("Automatic Dead Track Generator.");
  m_actionDeadTrack->setCheckable(true);
  m_actionDeadTrack->setEnabled(true);
  m_actionDeadTrack->setObjectName("autoDeadTrack_Tool");

  //connects
  connect(m_actionEraser, SIGNAL(triggered(bool)), this, SLOT(onEraserToolButtonClicked(bool)));
  connect(m_actionUpdate, SIGNAL(triggered(bool)), this, SLOT(onUpdateToolButtonClicked(bool)));
  connect(m_actionCreator, SIGNAL(triggered(bool)), this, SLOT(onCreatorToolButtonClicked(bool)));
  connect(m_actionCreatorLive, SIGNAL(triggered(bool)), this, SLOT(onCreatorLiveToolButtonClicked(bool)));
  connect(m_actionCreatorDead, SIGNAL(triggered(bool)), this, SLOT(onCreatorDeadToolButtonClicked(bool)));
  connect(m_actionAutoTrack, SIGNAL(triggered(bool)), this, SLOT(onTrackAutoClassifierToolButtonClicked(bool)));
  connect(m_actionDeadTrack, SIGNAL(triggered(bool)), this, SLOT(onTrackDeadClassifierToolButtonClicked(bool)));
  
  //associate to button
  m_ui->m_eraserToolButton->setDefaultAction(m_actionEraser);
  m_ui->m_updateToolButton->setDefaultAction(m_actionUpdate);
  m_ui->m_creatorToolButton->setDefaultAction(m_actionCreator);
  m_ui->m_creatorLiveToolButton->setDefaultAction(m_actionCreatorLive);
  m_ui->m_creatorDeadToolButton->setDefaultAction(m_actionCreatorDead);
  m_ui->m_trackAutoClassifierToolButton->setDefaultAction(m_actionAutoTrack);
  m_ui->m_trackDeadClassifierToolButton->setDefaultAction(m_actionDeadTrack);

  m_ui->m_updateToolButton->setVisible(false);
  
  //add to tool group from app
  toolsGroup->addAction(m_actionEraser);
  toolsGroup->addAction(m_actionUpdate);
  toolsGroup->addAction(m_actionCreator);
  toolsGroup->addAction(m_actionCreatorLive);
  toolsGroup->addAction(m_actionCreatorDead);
  toolsGroup->addAction(m_actionAutoTrack);
  toolsGroup->addAction(m_actionDeadTrack);

  m_clearTool = false;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onEraserToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayer = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  te::qt::plugins::tv5plugins::Eraser* tool = new te::qt::plugins::tv5plugins::Eraser(m_appDisplay->getDisplay(), Qt::ArrowCursor, layer);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onUpdateToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayer = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  te::qt::plugins::tv5plugins::UpdateClass* tool = new te::qt::plugins::tv5plugins::UpdateClass(m_appDisplay->getDisplay(), Qt::ArrowCursor, layer);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onCreatorToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  te::qt::plugins::tv5plugins::Creator* tool = new te::qt::plugins::tv5plugins::Creator(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, te::qt::plugins::tv5plugins::CREATED_TYPE);
  tool->setLineEditComponents(m_ui->m_distLineEdit);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onCreatorLiveToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  te::qt::plugins::tv5plugins::Creator* tool = new te::qt::plugins::tv5plugins::Creator(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, te::qt::plugins::tv5plugins::LIVE_TYPE);
  tool->setLineEditComponents(m_ui->m_distLineEdit);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onCreatorDeadToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  te::qt::plugins::tv5plugins::Creator* tool = new te::qt::plugins::tv5plugins::Creator(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, te::qt::plugins::tv5plugins::DEAD_TYPE);
  tool->setLineEditComponents(m_ui->m_distLineEdit);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onTrackAutoClassifierToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

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

  te::qt::plugins::tv5plugins::TrackAutoClassifier* tool = new te::qt::plugins::tv5plugins::TrackAutoClassifier(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, layerPoly, layerDir);
  tool->setLineEditComponents(m_ui->m_distLineEdit, m_ui->m_distTrackLineEdit, m_ui->m_distTolLineEdit, m_ui->m_distTrackTolLineEdit,  m_ui->m_polyAreaMinLineEdit, m_ui->m_polyAreaMaxLineEdit, m_ui->m_maxDeadLineEdit, m_ui->m_deadTolLineEdit, m_ui->m_thresholdLineEdit);
  tool->setAdjustTrack(m_ui->m_adaptTrackCheckBox->isChecked(), m_ui->m_adjustTrackSpinBox->value());
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void te::qt::plugins::tv5plugins::ForestMonitorToolBarDialog::onTrackDeadClassifierToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QVariant varLayerPoly = m_ui->m_layerPolyComboBox->itemData(m_ui->m_layerPolyComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoly = varLayerPoly.value<te::map::AbstractLayerPtr>();

  te::qt::plugins::tv5plugins::TrackDeadClassifier* tool = new te::qt::plugins::tv5plugins::TrackDeadClassifier(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, layerPoly);
  tool->setLineEditComponents(m_ui->m_distLineEdit, m_ui->m_distTrackLineEdit, m_ui->m_distTolLineEdit, m_ui->m_distTrackTolLineEdit, m_ui->m_polyAreaMinLineEdit, m_ui->m_polyAreaMaxLineEdit, m_ui->m_deadTolLineEdit, m_ui->m_thresholdLineEdit);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}
