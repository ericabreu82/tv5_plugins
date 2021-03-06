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
  \file terralib/qt/plugins/thirdParty/forestMonitor/qt/NDVIDialog.cpp

  \brief This interface is used to get the input parameters for NDVI operation.
*/

// TerraLib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/qt/widgets/rp/Utils.h>
#include "../core/NDVI.h"
#include "NDVIDialog.h"
#include "ui_NDVIDialogForm.h"

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QValidator>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::tv5plugins::NDVIDialog::NDVIDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::NDVIDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  // connectors
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), this, SLOT(onOkPushButtonClicked()));
  connect(m_ui->m_targetFileToolButton, SIGNAL(pressed()), this,  SLOT(onTargetFileToolButtonPressed()));
  connect(m_ui->m_nirLayerComboBox, SIGNAL(activated(int)), this, SLOT(onNIRLayerCmbActivated(int)));
  connect(m_ui->m_visLayerComboBox, SIGNAL(activated(int)), this, SLOT(onVISLayerCmbActivated(int)));

  //validators
  m_ui->m_gainLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_offsetLineEdit->setValidator(new QDoubleValidator(this));
}

te::qt::plugins::tv5plugins::NDVIDialog::~NDVIDialog()
{

}

void te::qt::plugins::tv5plugins::NDVIDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  //clear combos
  m_ui->m_nirLayerComboBox->clear();
  m_ui->m_visLayerComboBox->clear();

  //fill combos
  std::list<te::map::AbstractLayerPtr>::iterator it = list.begin();

  while(it != list.end())
  {
    te::map::AbstractLayerPtr l = *it;

    if(l->isValid())
    {
      std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

      if(dsType->hasRaster())
      {
        m_ui->m_nirLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_visLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
    }

    ++it;
  }

  if(m_ui->m_nirLayerComboBox->count() != 0)
    onNIRLayerCmbActivated(0);

  if(m_ui->m_visLayerComboBox->count() != 0)
    onVISLayerCmbActivated(0);
}

te::map::AbstractLayerPtr te::qt::plugins::tv5plugins::NDVIDialog::getOutputLayer()
{
  return m_outputLayer;
}

void te::qt::plugins::tv5plugins::NDVIDialog::onNIRLayerCmbActivated(int idx)
{
  m_ui->m_nirBandComboBox->clear();

  QVariant varLayer = m_ui->m_nirLayerComboBox->itemData(idx, Qt::UserRole);
  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  //get input raster
  std::auto_ptr<te::da::DataSet> ds = layer->getData();

  if(ds.get())
  {
    std::size_t rpos = te::da::GetFirstPropertyPos(ds.get(), te::dt::RASTER_TYPE);

    std::auto_ptr<te::rst::Raster> inputRst = ds->getRaster(rpos);

    if(inputRst.get())
    {
      for(unsigned int i = 0; i < inputRst->getNumberOfBands(); ++i)
      {
        m_ui->m_nirBandComboBox->addItem(QString::number(i));
      }
    }
  }
}

void te::qt::plugins::tv5plugins::NDVIDialog::onVISLayerCmbActivated(int idx)
{
  m_ui->m_visBandComboBox->clear();

  QVariant varLayer = m_ui->m_visLayerComboBox->itemData(idx, Qt::UserRole);
  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  //get input raster
  std::auto_ptr<te::da::DataSet> ds = layer->getData();

  if(ds.get())
  {
    std::size_t rpos = te::da::GetFirstPropertyPos(ds.get(), te::dt::RASTER_TYPE);

    std::auto_ptr<te::rst::Raster> inputRst = ds->getRaster(rpos);

    if(inputRst.get())
    {
      for(unsigned int i = 0; i < inputRst->getNumberOfBands(); ++i)
      {
        m_ui->m_visBandComboBox->addItem(QString::number(i));
      }
    }
  }
}

void te::qt::plugins::tv5plugins::NDVIDialog::onOkPushButtonClicked()
{  
  // check input parameters
  if(m_ui->m_repositoryLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a repository for the result."));
    return;
  }
       
  double gain;
  if(m_ui->m_gainLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Gain value not defined."));
    return;
  }
  gain = m_ui->m_gainLineEdit->text().toDouble();

  double offset;
  if(m_ui->m_offsetLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Offset value not defined."));
    return;
  }
  offset = m_ui->m_offsetLineEdit->text().toDouble();

  //get NIR layer
  QVariant nirVarLayer = m_ui->m_nirLayerComboBox->itemData(m_ui->m_nirLayerComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr nirLayer = nirVarLayer.value<te::map::AbstractLayerPtr>();

  std::auto_ptr<te::da::DataSet> nirDS = nirLayer->getData();
  std::size_t rpos = te::da::GetFirstPropertyPos(nirDS.get(), te::dt::RASTER_TYPE);
  std::auto_ptr<te::rst::Raster> nirRaster = nirDS->getRaster(rpos);

  int nirBand = m_ui->m_nirBandComboBox->currentText().toInt();

  //get vis layer
  QVariant visVarLayer = m_ui->m_visLayerComboBox->itemData(m_ui->m_visLayerComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr visLayer = visVarLayer.value<te::map::AbstractLayerPtr>();

  std::auto_ptr<te::da::DataSet> visDS = visLayer->getData();
  rpos = te::da::GetFirstPropertyPos(visDS.get(), te::dt::RASTER_TYPE);
  std::auto_ptr<te::rst::Raster> visRaster = visDS->getRaster(rpos);

  int visBand = m_ui->m_visBandComboBox->currentText().toInt();

  bool normalize = m_ui->m_normalizeCheckBox->isChecked();

  //rinfo information
  std::string type = "GDAL";
  std::map<std::string, std::string> rInfo;
  rInfo["URI"] = m_ui->m_repositoryLineEdit->text().toStdString();

  //progress
  te::qt::widgets::ProgressViewerDialog v(this);
  int id = te::common::ProgressManager::getInstance().addViewer(&v);

  QApplication::setOverrideCursor(Qt::WaitCursor);

  try
  {
    std::auto_ptr<te::rst::Raster> rOut = te::qt::plugins::tv5plugins::GenerateNDVIRaster(nirRaster.get(), nirBand, visRaster.get(), visBand, gain, offset, normalize, rInfo, type, m_ui->m_invertCheckBox->isChecked());
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());

    te::common::ProgressManager::getInstance().removeViewer(id);

    QApplication::restoreOverrideCursor();

    return;
  }
  catch(...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error."));

    te::common::ProgressManager::getInstance().removeViewer(id);

    QApplication::restoreOverrideCursor();

    return;
  }

  //set output layer
  m_outputLayer = te::qt::widgets::createLayer(type, rInfo);

  te::common::ProgressManager::getInstance().removeViewer(id);

  QApplication::restoreOverrideCursor();

  accept();
}

void te::qt::plugins::tv5plugins::NDVIDialog::onTargetFileToolButtonPressed()
{
  m_ui->m_repositoryLineEdit->clear();
  
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("Geotiff (*.tif *.TIF);;"),0, QFileDialog::DontConfirmOverwrite);
  
  if (fileName.isEmpty())
    return;
  
  m_ui->m_repositoryLineEdit->setText(fileName);
}
