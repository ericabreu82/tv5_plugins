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
  \file terralib/qt/plugins/thirdParty/tileGenerator/qt/TileGeneratorDialog.cpp

  \brief This interface i used to get the input parameters for  tile generation.
*/

// TerraLib
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/widgets/tools/ExtentAcquire.h>

#include "../core/TileGeneratorService.h"
#include "TileGeneratorDialog.h"
#include "ui_TileGeneratorDialogForm.h"

// Qt
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>

te::qt::plugins::tv5plugins::TileGeneratorDialog::TileGeneratorDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::TileGeneratorDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_dirToolButton->setIcon(QIcon::fromTheme("folder-open"));
  m_ui->m_toolButton->setIcon(QIcon::fromTheme("pointer"));

  //set zoom level range
  m_ui->m_zoomMinSpinBox->setMinimum(0);
  m_ui->m_zoomMinSpinBox->setMaximum(25);
  m_ui->m_zoomMinSpinBox->setValue(0);

  m_ui->m_zoomMaxSpinBox->setMinimum(0);
  m_ui->m_zoomMaxSpinBox->setMaximum(25);
  m_ui->m_zoomMaxSpinBox->setValue(25);

  //tile info
  m_ui->m_tileSizeLineEdit->setValidator(new QIntValidator(this));
  m_ui->m_tileSizeLineEdit->setText("256");

  //output file formats
  QList<QByteArray> list = QImageWriter::supportedImageFormats();

  m_ui->m_formatComboBox->clear();
  for(int i = 0; i < list.size(); ++i)
    m_ui->m_formatComboBox->addItem(list[i].constData());

  //create action
  m_action = new QAction(this);
  m_action->setIcon(QIcon::fromTheme("pointer"));
  m_action->setToolTip("Acquire extent from map display.");
  m_action->setCheckable(true);
  m_action->setEnabled(true);
  m_action->setObjectName("tileGen_boxTool");

  //connects
  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(onToolButtonClicked(bool)));
  connect(m_ui->m_dirToolButton, SIGNAL(clicked()), this, SLOT(onDirToolButtonClicked()));
  connect(m_ui->m_validatePushButton, SIGNAL(clicked()), this, SLOT(onValidatePushButtonClicked()));
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), this, SLOT(onOkPushButtonClicked()));

  // Get the action group of map tools.
  m_ui->m_toolButton->setDefaultAction(m_action);

  QActionGroup* toolsGroup = te::qt::af::AppCtrlSingleton::getInstance().findActionGroup("Map.ToolsGroup");
  
  if (toolsGroup)
    toolsGroup->addAction(m_action);
  
  m_clearTool = false;
}

te::qt::plugins::tv5plugins::TileGeneratorDialog::~TileGeneratorDialog()
{
  if (m_clearTool)
    m_appDisplay->getDisplay()->setCurrentTool(0);
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::setExtentInfo(te::gm::Envelope env, int srid)
{
  m_srid = srid;

  m_env = env;

  m_ui->m_llxLineEdit->setText(QString::number(env.getLowerLeftX(), 'f', 5));
  m_ui->m_llyLineEdit->setText(QString::number(env.getLowerLeftY(), 'f', 5));
  m_ui->m_urxLineEdit->setText(QString::number(env.getUpperRightX(), 'f', 5));
  m_ui->m_uryLineEdit->setText(QString::number(env.getUpperRightY(), 'f', 5));
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::setMapDisplay(te::qt::af::MapDisplay* mapDisplay)
{
  m_appDisplay = mapDisplay;
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  m_layerList = list;
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::onEnvelopeAcquired(te::gm::Envelope env)
{
  m_env = env;

  m_ui->m_llxLineEdit->setText(QString::number(env.getLowerLeftX(), 'f', 5));
  m_ui->m_llyLineEdit->setText(QString::number(env.getLowerLeftY(), 'f', 5));
  m_ui->m_urxLineEdit->setText(QString::number(env.getUpperRightX(), 'f', 5));
  m_ui->m_uryLineEdit->setText(QString::number(env.getUpperRightY(), 'f', 5));
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::onToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  te::qt::widgets::ExtentAcquire* ea = new te::qt::widgets::ExtentAcquire(m_appDisplay->getDisplay(), Qt::BlankCursor);
  m_appDisplay->getDisplay()->setCurrentTool(ea);

  connect(ea, SIGNAL(extentAcquired(te::gm::Envelope)), this, SLOT(onEnvelopeAcquired(te::gm::Envelope)));

  m_clearTool = true;
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::onDirToolButtonClicked()
{
  QString dirName = QFileDialog::getExistingDirectory(this, tr("Select a directory to save tiles"), "", QFileDialog::ShowDirsOnly);

  if(!dirName.isEmpty())
    m_ui->m_dirLineEdit->setText(dirName);
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::onValidatePushButtonClicked()
{
  //get dir info
  if(m_ui->m_dirLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Output path not defined."));
    return;
  }
  std::string path = m_ui->m_dirLineEdit->text().toStdString();

  //get tile size
  if(m_ui->m_tileSizeLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Tile Size defined."));
    return;
  }
  int tileSize = m_ui->m_tileSizeLineEdit->text().toInt();

  //get format
  std::string format = m_ui->m_formatComboBox->currentText().toStdString();

  bool createMissingTiles = false;
  //create missing tiles
  int response = QMessageBox::question(this, tr("Tile Generator"), tr("Create missing tiles?"), tr("Yes"), tr("No"));

  if(response == 0)
    createMissingTiles = true;

  //run operation
  te::qt::plugins::tv5plugins::TileGeneratorService service;

  try
  {
    service.setInputParameters(m_layerList, m_env, m_srid, m_ui->m_zoomMinSpinBox->value(), m_ui->m_zoomMaxSpinBox->value(), tileSize, path, format);

    service.runValidation(createMissingTiles);
  }
  catch(std::exception e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());
    return;
  }
  catch(...)
  {
     QMessageBox::warning(this, tr("Warning"), tr("Internal Error."));
    return;
  }

  QMessageBox::information(this, tr("Information"), tr("Tile Validation done!"));
}

void te::qt::plugins::tv5plugins::TileGeneratorDialog::onOkPushButtonClicked()
{
  //get dir info
  if(m_ui->m_dirLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Output path not defined."));
    return;
  }
  std::string path = m_ui->m_dirLineEdit->text().toStdString();

  //get tile size
  if(m_ui->m_tileSizeLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Tile Size defined."));
    return;
  }
  int tileSize = m_ui->m_tileSizeLineEdit->text().toInt();

  //get format
  std::string format = m_ui->m_formatComboBox->currentText().toStdString();

  //get tile export type
  bool isRaster = true;

  if(m_ui->m_rasterRadioButton->isChecked())
    isRaster = true;

  if(m_ui->m_vectorRadioButton->isChecked())
    isRaster = false;

  //run operation
  te::qt::plugins::tv5plugins::TileGeneratorService service;

  try
  {
    service.setInputParameters(m_layerList, m_env, m_srid, m_ui->m_zoomMinSpinBox->value(), m_ui->m_zoomMaxSpinBox->value(), tileSize, path, format);

    service.runService(isRaster);
  }
  catch(std::exception e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());
    return;
  }
  catch(...)
  {
     QMessageBox::warning(this, tr("Warning"), tr("Internal Error."));
    return;
  }

  QMessageBox::information(this, tr("Information"), tr("Tile Generation done!"));

  accept();
}
