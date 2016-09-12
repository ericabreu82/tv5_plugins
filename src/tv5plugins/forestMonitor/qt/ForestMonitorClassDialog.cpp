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
  \file terralib/qt/plugins/thirdParty/forestMonitor/qt/ForestMonitorClassDialog.cpp

  \brief This interface is used to get the input parameters for forest monitor classification information.
*/

// TerraLib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/STLUtils.h>
#include <terralib/common/StringUtils.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/DataSetLayer.h>
#include <terralib/maptools/Grouping.h>
#include <terralib/maptools/GroupingItem.h>
#include <terralib/maptools/Utils.h>
#include <terralib/qt/widgets/canvas/Canvas.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/RasterSummary.h>
#include <terralib/raster/RasterSummaryManager.h>
#include <terralib/raster/Utils.h>
#include <terralib/rp/Filter.h>
#include <terralib/se/Categorize.h>
#include <terralib/se/ColorMap.h>
#include <terralib/se/CoverageStyle.h>
#include <terralib/se/ParameterValue.h>
#include <terralib/se/RasterSymbolizer.h>
#include <terralib/se/PointSymbolizer.h>
#include <terralib/se/Rule.h>
#include <terralib/se/Utils.h>
#include "../core/ForestMonitorClassification.h"
#include "ForestMonitorClassDialog.h"
#include "ui_ForestMonitorClassDialogForm.h"

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QValidator>

// Boost
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::tv5plugins::ForestMonitorClassDialog::ForestMonitorClassDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::ForestMonitorClassDialogForm),
	m_progressDlg(0)
{
  // add controls
  m_ui->setupUi(this);

  //build form
  QGridLayout* displayLayout3 = new QGridLayout(m_ui->m_thresholdFrame);
  m_thresholdDisplay.reset(new te::qt::widgets::MapDisplay(m_ui->m_thresholdFrame->size(), m_ui->m_thresholdFrame));
  displayLayout3->addWidget(m_thresholdDisplay.get());
  displayLayout3->setContentsMargins(0,0,0,0);

  QGridLayout* displayLayout4 = new QGridLayout(m_ui->m_erosionFrame);
  m_erosionDisplay.reset(new te::qt::widgets::MapDisplay(m_ui->m_erosionFrame->size(), m_ui->m_erosionFrame));
  displayLayout4->addWidget(m_erosionDisplay.get());
  displayLayout4->setContentsMargins(0,0,0,0);

  // connectors
  connect(m_ui->m_thresholdHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(onThresholdSliderReleased()));
  connect(m_ui->m_thresholdHorizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(onThresholdSliderMoved(int)));
  connect(m_ui->m_generateNDVISamplePushButton, SIGNAL(clicked()), this, SLOT(onGenerateNDVISampleClicked()));
  connect(m_ui->m_generateThresholdPushButton, SIGNAL(clicked()), this, SLOT(onGenerateErosionSampleClicked()));
  connect(m_ui->m_dilationPushButton, SIGNAL(clicked()), this, SLOT(onDilationPushButtonClicked()));
  connect(m_ui->m_erosionPushButton, SIGNAL(clicked()), this, SLOT(onErosionPushButtonClicked()));
  connect(m_ui->m_targetFileToolButton, SIGNAL(pressed()), this, SLOT(onTargetFileToolButtonPressed()));
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), this, SLOT(onOkPushButtonClicked()));
  connect(m_ui->m_runFromFilePushButton, SIGNAL(clicked()), this, SLOT(onLoadToolButtonClicked()));
  connect(m_ui->m_saveToFilePushButton, SIGNAL(clicked()), this, SLOT(onSaveToolButtonClicked()));

  //validators
  m_ui->m_dilationLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_erosionLineEdit->setValidator(new QDoubleValidator(this));

  this->setSizeGripEnabled(true);

  m_thresholdRasterMin = 0.;
  m_thresholdRasterMax = 0.;
}

te::qt::plugins::tv5plugins::ForestMonitorClassDialog::~ForestMonitorClassDialog()
{
  te::common::ProgressManager::getInstance().removeViewer(m_progressId);
  delete m_progressDlg;
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::setExtentInfo(te::gm::Envelope env, int srid)
{
  m_srid = srid;

  m_env = env;
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  //clear combos
  m_ui->m_originalLayerComboBox->clear();
  m_ui->m_ndviLayerComboBox->clear();
  m_ui->m_vecComboBox->clear();

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
         m_ui->m_originalLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
         m_ui->m_ndviLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
      else if (dsType->hasGeom())
      {
        m_ui->m_vecComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
    }

    ++it;
  }
}

te::map::AbstractLayerPtr te::qt::plugins::tv5plugins::ForestMonitorClassDialog::getOutputLayer()
{
  return m_outputLayer;
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onGenerateNDVISampleClicked()
{
  //progress
  if (!m_progressDlg)
  {
    m_progressDlg = new te::qt::widgets::ProgressViewerDialog(this);
    m_progressId = te::common::ProgressManager::getInstance().addViewer(m_progressDlg);
  }

  //get input raster
  QVariant varLayer = m_ui->m_ndviLayerComboBox->itemData(m_ui->m_ndviLayerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  //get current extent
  te::gm::Envelope ndviRasterExtent(m_env);

  ndviRasterExtent.transform(m_srid, layer->getSRID());
  
  //get ndvi raster
  std::auto_ptr<te::da::DataSet> ds = layer->getData();

  std::size_t rpos = te::da::GetFirstPropertyPos(ds.get(), te::dt::RASTER_TYPE);

  std::auto_ptr<te::rst::Raster> inputRst = ds->getRaster(rpos);

  te::gm::Envelope env = ndviRasterExtent.intersection(*inputRst->getExtent());

  std::map<std::string, std::string> rInfo;
  rInfo["FORCE_MEM_DRIVER"] = "TRUE";

  te::rst::Raster* raster = inputRst->trim(&env, rInfo);

  m_thresholdRaster.reset(raster);

  m_thresholdDisplay->setExtent(ndviRasterExtent, false);

  drawRaster(m_thresholdRaster.get(), m_thresholdDisplay.get());

  m_ui->m_thresholdHorizontalSlider->setEnabled(true);

  //set min max values
  const te::rst::RasterSummary* rsMin = te::rst::RasterSummaryManager::getInstance().get(m_thresholdRaster.get(), te::rst::SUMMARY_MIN, true);
  const te::rst::RasterSummary* rsMax = te::rst::RasterSummaryManager::getInstance().get(m_thresholdRaster.get(), te::rst::SUMMARY_MAX, true);
  const std::complex<double>* cmin = rsMin->at(0).m_minVal;
  const std::complex<double>* cmax = rsMax->at(0).m_maxVal;
  m_thresholdRasterMin = cmin->real();
  m_thresholdRasterMax = cmax->real();
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onThresholdSliderReleased()
{
  if (!m_thresholdRaster.get())
    return;

  int curSliderValue = m_ui->m_thresholdHorizontalSlider->value();

  double value = (((double)curSliderValue) / (1000.)) * (m_thresholdRasterMax - m_thresholdRasterMin) + m_thresholdRasterMin;

  //get original raster
  QVariant varLayer = m_ui->m_originalLayerComboBox->itemData(m_ui->m_originalLayerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  std::auto_ptr<te::da::DataSet> ds = layer->getData();

  std::size_t rpos = te::da::GetFirstPropertyPos(ds.get(), te::dt::RASTER_TYPE);

  std::auto_ptr<te::rst::Raster> originalRaster = ds->getRaster(rpos);

  //create erosion raster
  std::map<std::string, std::string> rInfo;
  rInfo["FORCE_MEM_DRIVER"] = "TRUE";

  te::rst::Grid* grid = new te::rst::Grid(m_thresholdRaster->getNumberOfColumns(), m_thresholdRaster->getNumberOfRows(), new te::gm::Envelope(*m_thresholdRaster->getExtent()), m_thresholdRaster->getSRID());

  std::vector<te::rst::BandProperty*> bands;

  for (std::size_t b = 0; b < originalRaster->getNumberOfBands(); b++)
  {
    bands.push_back(new te::rst::BandProperty(*originalRaster->getBand(b)->getProperty()));
    bands[ b ]->m_nblocksx = 1;
    bands[ b ]->m_nblocksy = m_thresholdRaster->getNumberOfRows();
    bands[ b ]->m_blkw = m_thresholdRaster->getNumberOfColumns();
    bands[ b ]->m_blkh = 1;
  }

  //progress
  if (!m_progressDlg)
  {
    m_progressDlg = new te::qt::widgets::ProgressViewerDialog(this);
    m_progressId = te::common::ProgressManager::getInstance().addViewer(m_progressDlg);
  }

  te::rst::Raster* raster = te::rst::RasterFactory::make(grid, bands, rInfo);

  {
    te::common::TaskProgress task("Generating Threshold Raster");
    task.setTotalSteps(m_thresholdRaster->getNumberOfRows());

    //fill threshold raster
    for (unsigned int i = 0; i < m_thresholdRaster->getNumberOfRows(); ++i)
    {
      for (unsigned int j = 0; j < m_thresholdRaster->getNumberOfColumns(); ++j)
      {
        double curValue;

        m_thresholdRaster->getValue(j, i, curValue);

        if (curValue > value)
        {
          for (std::size_t b = 0; b < originalRaster->getNumberOfBands(); b++)
          {
            raster->setValue(j, i, 255., b);
          }
        }
        else
        {
          te::gm::Coord2D cNDVIGrid = m_thresholdRaster->getGrid()->gridToGeo(j, i);
          te::gm::Coord2D cOriginalGeo = originalRaster->getGrid()->geoToGrid(cNDVIGrid.getX(), cNDVIGrid.getY());

          std::vector<double> values;
          originalRaster->getValues(te::rst::Round(cOriginalGeo.getX()), te::rst::Round(cOriginalGeo.getY()), values);

          raster->setValues(j, i, values);
        }
      }

      task.pulse();
    }
  }

  //draw erosion raster
  drawRaster(raster, m_thresholdDisplay.get());

  delete raster;

  m_ui->m_thresholdLineEdit->setText(QString::number(value));
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onThresholdSliderMoved(int value)
{
  if (!m_thresholdRaster.get())
    return;

  int v = (((double)value) / (1000.)) * (m_thresholdRasterMax - m_thresholdRasterMin) + m_thresholdRasterMin;

  m_ui->m_thresholdValueLabel->setText(QString::number(v));
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onGenerateErosionSampleClicked()
{
  int curSliderValue = m_ui->m_thresholdHorizontalSlider->value();

  double value = (((double)curSliderValue) / (1000.)) * (m_thresholdRasterMax - m_thresholdRasterMin) + m_thresholdRasterMin;

  //create erosion raster
  std::map<std::string, std::string> rInfo;
  rInfo["FORCE_MEM_DRIVER"] = "TRUE";

  te::rst::Grid* grid = new te::rst::Grid(m_thresholdRaster->getNumberOfColumns(), m_thresholdRaster->getNumberOfRows(), new te::gm::Envelope(*m_thresholdRaster->getExtent()), m_thresholdRaster->getSRID());

  std::vector<te::rst::BandProperty*> bands;

  for (std::size_t b = 0; b < m_thresholdRaster->getNumberOfBands(); b++)
  {
    bands.push_back(new te::rst::BandProperty(0, te::dt::UCHAR_TYPE));
    bands[ b ]->m_nblocksx = 1;
    bands[ b ]->m_nblocksy = m_thresholdRaster->getNumberOfRows();
    bands[ b ]->m_blkw = m_thresholdRaster->getNumberOfColumns();
    bands[ b ]->m_blkh = 1;
  }

  te::rst::Raster* raster = te::rst::RasterFactory::make(grid, bands, rInfo);

  //progress
  if (!m_progressDlg)
  {
    m_progressDlg = new te::qt::widgets::ProgressViewerDialog(this);
    m_progressId = te::common::ProgressManager::getInstance().addViewer(m_progressDlg);
  }

  te::common::TaskProgress task("Generating Filter Raster");
  task.setTotalSteps(m_thresholdRaster->getNumberOfRows());

  //fill erosion raster
  for(unsigned int i = 0; i < m_thresholdRaster->getNumberOfRows(); ++i)
  {
    for(unsigned int j = 0; j < m_thresholdRaster->getNumberOfColumns(); ++j)
    {
      double curValue;

      m_thresholdRaster->getValue(j, i, curValue);

      if(curValue <= value)
      {
        raster->setValue(j, i, 255.);
      }
      else
      {
        raster->setValue(j, i, 0.);
      }
    }
    
    task.pulse();
  }

  //draw erosion raster
  m_filterRaster.reset(raster);

  m_erosionDisplay->setExtent(*m_thresholdRaster->getExtent(), false);

  drawRaster(m_filterRaster.get(), m_erosionDisplay.get());

  m_ui->m_dilationPushButton->setEnabled(true);
  m_ui->m_dilationLineEdit->setEnabled(true);
  
  m_ui->m_erosionPushButton->setEnabled(false);
  m_ui->m_erosionLineEdit->setEnabled(false);

}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onDilationPushButtonClicked()
{
  if (m_ui->m_dilationLineEdit->text().isEmpty())
    return;

  te::rp::Filter algorithmInstance;

  te::rp::Filter::InputParameters algoInputParams;
  algoInputParams.m_iterationsNumber = m_ui->m_dilationLineEdit->text().toInt();
  algoInputParams.m_filterType = te::rp::Filter::InputParameters::DilationFilterT;
  algoInputParams.m_inRasterBands.push_back(0);
  algoInputParams.m_inRasterPtr = m_filterRaster.get();
  algoInputParams.m_enableProgress = true;

  std::map<std::string, std::string> rinfo;
  rinfo["MEM_RASTER_NROWS"] = boost::lexical_cast<std::string>(m_filterRaster->getNumberOfRows());
  rinfo["MEM_RASTER_NCOLS"] = boost::lexical_cast<std::string>(m_filterRaster->getNumberOfColumns());
  rinfo["MEM_RASTER_DATATYPE"] = boost::lexical_cast<std::string>(m_filterRaster->getBandDataType(0));
  rinfo["MEM_RASTER_NBANDS"] = boost::lexical_cast<std::string>(m_filterRaster->getNumberOfBands());
  
  te::rp::Filter::OutputParameters algoOutputParams;
  algoOutputParams.m_rType = "MEM";
  algoOutputParams.m_rInfo = rinfo;

  if(algorithmInstance.initialize(algoInputParams))
  {
    if(algorithmInstance.execute(algoOutputParams))
    {
      m_filterDilRaster = algoOutputParams.m_outputRasterPtr;

      drawRaster(m_filterDilRaster.get(), m_erosionDisplay.get());
    }
  }

  m_ui->m_dilationResLineEdit->setText(m_ui->m_dilationLineEdit->text());

  m_ui->m_erosionPushButton->setEnabled(true);
  m_ui->m_erosionLineEdit->setEnabled(true);
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onErosionPushButtonClicked()
{
  if (m_ui->m_erosionLineEdit->text().isEmpty() || m_ui->m_dilationLineEdit->text().isEmpty())
    return;

  if(!m_filterDilRaster.get())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Erosion Filter not defined."));

    return;
  }

  int dilationValue = m_ui->m_dilationLineEdit->text().toInt();
  int erosionValue = m_ui->m_erosionLineEdit->text().toInt();

  if (erosionValue > dilationValue)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Invalid dilation value."));

    m_ui->m_erosionLineEdit->setText(QString::number(dilationValue));

    return;
  }

  te::rp::Filter algorithmInstance;

  te::rp::Filter::InputParameters algoInputParams;
  algoInputParams.m_iterationsNumber = m_ui->m_erosionLineEdit->text().toInt();
  algoInputParams.m_filterType = te::rp::Filter::InputParameters::ErosionFilterT;
  algoInputParams.m_inRasterBands.push_back(0);
  algoInputParams.m_inRasterPtr = m_filterDilRaster.get();
  algoInputParams.m_enableProgress = true;

  std::map<std::string, std::string> rinfo;
  rinfo["MEM_RASTER_NROWS"] = boost::lexical_cast<std::string>(m_filterDilRaster->getNumberOfRows());
  rinfo["MEM_RASTER_NCOLS"] = boost::lexical_cast<std::string>(m_filterDilRaster->getNumberOfColumns());
  rinfo["MEM_RASTER_DATATYPE"] = boost::lexical_cast<std::string>(m_filterDilRaster->getBandDataType(0));
  rinfo["MEM_RASTER_NBANDS"] = boost::lexical_cast<std::string>(m_filterDilRaster->getNumberOfBands());
  
  te::rp::Filter::OutputParameters algoOutputParams;
  algoOutputParams.m_rType = "MEM";
  algoOutputParams.m_rInfo = rinfo;

  if(algorithmInstance.initialize(algoInputParams))
  {
    if(algorithmInstance.execute(algoOutputParams))
    {
      std::auto_ptr<te::rst::Raster> rst = algoOutputParams.m_outputRasterPtr;

      drawRaster(rst.get(), m_erosionDisplay.get());
    }
  }

  m_ui->m_erosionResLineEdit->setText(m_ui->m_erosionLineEdit->text());
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onTargetFileToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_repositoryLineEdit->clear();

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("Shapefile (*.shp *.SHP);;"), 0, QFileDialog::DontConfirmOverwrite);

  if (fileName.isEmpty())
    return;

  boost::filesystem::path outfile(fileName.toStdString());

  m_ui->m_repositoryLineEdit->setText(outfile.string().c_str());

  m_ui->m_newLayerNameLineEdit->setText(outfile.leaf().string().c_str());

  m_ui->m_newLayerNameLineEdit->setEnabled(false);
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onOkPushButtonClicked()
{
  // check input parameters
  if (m_ui->m_repositoryLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a repository for the result."));
    return;
  }
  std::string repository = m_ui->m_repositoryLineEdit->text().toStdString();

  if (m_ui->m_newLayerNameLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a name for the resulting layer."));
    return;
  }
  std::string dataSetName = m_ui->m_newLayerNameLineEdit->text().toStdString();

  if (m_ui->m_originalLayerComboBox->currentText().isEmpty() || m_ui->m_ndviLayerComboBox->currentText().isEmpty() || m_ui->m_vecComboBox->currentText().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Input parameters are not defined."));
    return;
  }

  //get ndvi layer
  QVariant varLayer = m_ui->m_ndviLayerComboBox->itemData(m_ui->m_ndviLayerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr ndviLayer = varLayer.value<te::map::AbstractLayerPtr>();

  //get input vectorial layer
  QVariant varLayerVec = m_ui->m_vecComboBox->itemData(m_ui->m_vecComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr vecLayer = varLayerVec.value<te::map::AbstractLayerPtr>();

  //get operation parameters
  if (m_ui->m_thresholdLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Threshold not defined."));
    return;
  }
  double threshold = m_ui->m_thresholdLineEdit->text().toDouble();

  if (m_ui->m_dilationResLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Erosion value not defined."));
    return;
  }
  int dilation = m_ui->m_dilationResLineEdit->text().toInt();

  if (m_ui->m_erosionResLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Dilation value not defined."));
    return;
  }
  int erosion = m_ui->m_erosionResLineEdit->text().toInt();

  te::da::DataSourcePtr vecDataSource = te::da::GetDataSource(vecLayer->getDataSourceId());
  std::map<std::string, std::string> vecConnInfo = vecDataSource->getConnectionInfo();

  int vecSRID = vecLayer->getSRID();
  std::string vecURI = vecConnInfo["URI"];

  te::da::DataSourcePtr rasterDataSource = te::da::GetDataSource(ndviLayer->getDataSourceId());
  std::map<std::string, std::string> rasterConnInfo = rasterDataSource->getConnectionInfo();

  int rasterSRID = ndviLayer->getSRID();
  std::string rasterURI = rasterConnInfo["URI"];

  //run operation
  bool res = runClassOperation(repository, dataSetName, vecURI, vecSRID, rasterURI, rasterSRID, threshold, dilation, erosion, m_ui->m_saveResultImageCheckBox->isChecked());

  if (res)
    accept();
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onSaveToolButtonClicked()
{
  // check input parameters
  if (m_ui->m_repositoryLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a repository for the result."));
    return;
  }
  std::string repository = m_ui->m_repositoryLineEdit->text().toStdString();

  if (m_ui->m_newLayerNameLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a name for the resulting layer."));
    return;
  }
  std::string dataSetName = m_ui->m_newLayerNameLineEdit->text().toStdString();

  if (m_ui->m_originalLayerComboBox->currentText().isEmpty() || m_ui->m_ndviLayerComboBox->currentText().isEmpty() || m_ui->m_vecComboBox->currentText().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Input parameters are not defined."));
    return;
  }

  //get ndvi layer
  QVariant varLayer = m_ui->m_ndviLayerComboBox->itemData(m_ui->m_ndviLayerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr ndviLayer = varLayer.value<te::map::AbstractLayerPtr>();

  //get input vectorial layer
  QVariant varLayerVec = m_ui->m_vecComboBox->itemData(m_ui->m_vecComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr vecLayer = varLayerVec.value<te::map::AbstractLayerPtr>();

  //get operation parameters
  if (m_ui->m_thresholdLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Threshold not defined."));
    return;
  }
  double threshold = m_ui->m_thresholdLineEdit->text().toDouble();

  if (m_ui->m_dilationResLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Erosion value not defined."));
    return;
  }
  int dilation = m_ui->m_dilationResLineEdit->text().toInt();

  if (m_ui->m_erosionResLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Dilation value not defined."));
    return;
  }
  int erosion = m_ui->m_erosionResLineEdit->text().toInt();

  te::da::DataSourcePtr vecDataSource = te::da::GetDataSource(vecLayer->getDataSourceId());
  std::map<std::string, std::string> vecConnInfo = vecDataSource->getConnectionInfo();

  int vecSRID = vecLayer->getSRID();
  std::string vecURI = vecConnInfo["URI"];

  te::da::DataSourcePtr rasterDataSource = te::da::GetDataSource(ndviLayer->getDataSourceId());
  std::map<std::string, std::string> rasterConnInfo = rasterDataSource->getConnectionInfo();

  int rasterSRID = ndviLayer->getSRID();
  std::string rasterURI = rasterConnInfo["URI"];

  std::string save = "FALSE";
  if (m_ui->m_saveResultImageCheckBox->isChecked())
    save = "TRUE";

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Forest Monitor Class Parameters"), "", "JSON File (*.json)");

  if (fileName.isEmpty())
    return;

  /*
  std::string repository, 
  std::string dataSetName, 
  std::string vecURI, 
  int vecSRID, 
  std::string rasterURI, 
  int rasterSRID, 
  double threshold, 
  int dilation, 
  int erosion, 
  bool saveResultImage

  repository, dataSetName, vecURI, vecSRID, rasterURI, rasterSRID, threshold, dilation, erosion, m_ui->m_saveResultImageCheckBox->isChecked()
  */

  boost::property_tree::ptree pt;

  boost::property_tree::ptree children;

  boost::property_tree::ptree child;
  child.put("repository", repository);
  child.put("dataSetName", dataSetName);

  child.put("vectorURI", vecURI);
  child.put("vectorSRID", te::common::Convert2String(vecSRID));

  child.put("rasterURI", rasterURI);
  child.put("rasterSRID", te::common::Convert2String(rasterSRID));

  child.put("threshold", te::common::Convert2String(threshold));
  child.put("dilation", te::common::Convert2String(dilation));
  child.put("erosion", te::common::Convert2String(erosion));

  child.put("saveResult", save);

  children.push_back(std::make_pair("Parameters", child));

  pt.add_child("ForestMonitor_Classification", children);

  boost::property_tree::json_parser::write_json(fileName.toStdString(), pt);
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::onLoadToolButtonClicked()
{
  QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Load Forest Monitor Class Parameters"), "", "JSON File (*.json)");

  if (fileNames.isEmpty())
    return;

  QStringList::iterator it;

  for (it = fileNames.begin(); it != fileNames.end(); ++it)
  {
    try
    {
      boost::property_tree::ptree pt;
      boost::property_tree::json_parser::read_json((*it).toStdString(), pt);

      BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("ForestMonitor_Classification"))
      {
        std::string repository = v.second.get<std::string>("repository");
        std::string dataSetName = v.second.get<std::string>("dataSetName");

        std::string vecURI = v.second.get<std::string>("vectorURI");
        std::string vecSRIDStr = v.second.get<std::string>("vectorSRID");
        int vecSRID = atoi(vecSRIDStr.c_str());

        std::string rasterURI = v.second.get<std::string>("rasterURI");
        std::string rasterSRIDStr = v.second.get<std::string>("rasterSRID");
        int rasterSRID = atoi(rasterSRIDStr.c_str());

        std::string thresholdStr = v.second.get<std::string>("threshold");
        double threshold = atof(thresholdStr.c_str());
        std::string dilationStr = v.second.get<std::string>("dilation");
        int dilation = atoi(dilationStr.c_str());
        std::string erosionStr = v.second.get<std::string>("erosion");
        int erosion = atoi(erosionStr.c_str());

        std::string saveResultStr = v.second.get<std::string>("saveResult");
        bool saveResult = false;
        if (saveResultStr == "TRUE")
          saveResult = true;

        runClassOperation(repository, dataSetName, vecURI, vecSRID, rasterURI, rasterSRID, threshold, dilation, erosion, saveResult);
      }
    }
    catch (boost::property_tree::json_parser::json_parser_error &je)
    {
      QString errmsg = tr("Error parsing: ") + je.filename().c_str() + ": " + je.message().c_str();

      QMessageBox::warning(this, tr("Warning"), errmsg);

      return;
    }
    catch (std::exception const& e)
    {
      QString errmsg = e.what();

      QMessageBox::warning(this, tr("Warning"), errmsg);
    }
  }
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::drawRaster(te::rst::Raster* raster, te::qt::widgets::MapDisplay* mapDisplay, te::se::Style* style)
{
  QPixmap* draft = mapDisplay->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& env = mapDisplay->getExtent();
  const te::gm::Envelope& envRst = *raster->getExtent();

  // Prepares the canvas
  te::qt::widgets::Canvas canvas(mapDisplay->width(), mapDisplay->height());
  canvas.setDevice(draft, false);
  canvas.setWindow(env.m_llx, env.m_lly, env.m_urx, env.m_ury);

  bool hasToDelete = false;
  
  //style
  if(!style)
  {
    style = te::se::CreateCoverageStyle(raster->getNumberOfBands());

    hasToDelete = true;
  }

  te::se::CoverageStyle* cs = dynamic_cast<te::se::CoverageStyle*>(style);

  // Draw raster
  canvas.clear();

  te::map::DrawRaster(raster, &canvas, env, mapDisplay->getSRID(), envRst, raster->getSRID(), cs, mapDisplay->getScale());

  if(hasToDelete)
    delete style;

  mapDisplay->repaint();
}

te::da::DataSourcePtr te::qt::plugins::tv5plugins::ForestMonitorClassDialog::createDataSource(std::string repository, std::map<std::string, std::string>& dsInfo)
{
  boost::filesystem::path uri(repository);

  dsInfo["URI"] = uri.string();

  boost::uuids::basic_random_generator<boost::mt19937> gen;
  boost::uuids::uuid u = gen();
  std::string id_ds = boost::uuids::to_string(u);

  te::da::DataSourceInfoPtr dsInfoPtr(new te::da::DataSourceInfo);
  dsInfoPtr->setConnInfo(dsInfo);
  dsInfoPtr->setTitle(uri.stem().string());
  dsInfoPtr->setAccessDriver("OGR");
  dsInfoPtr->setType("OGR");
  dsInfoPtr->setDescription(uri.string());
  dsInfoPtr->setId(id_ds);

  te::da::DataSourceInfoManager::getInstance().add(dsInfoPtr);

  return te::da::DataSourceManager::getInstance().get(id_ds, "OGR", dsInfoPtr->getConnInfo());
}

void te::qt::plugins::tv5plugins::ForestMonitorClassDialog::createLegend()
{
  std::string vecAttr = "type";
  int attrType = te::dt::STRING_TYPE;
  int prec = 0;

  //create legend
  std::vector<te::map::GroupingItem*> legend;

  {
    std::vector<te::se::Symbolizer*> symbVec;
    te::se::PointSymbolizer* symb = createPointSymbolizer("#000000", "0.0", "#FFFFFF", "1", "circle", "12");
    symbVec.push_back(symb);

    te::map::GroupingItem* legendItem = new te::map::GroupingItem;
    legendItem->setValue("UNKNOWN");
    legendItem->setCount(0);
    legendItem->setSymbolizers(symbVec);
    legend.push_back(legendItem);
  }

  {
    std::vector<te::se::Symbolizer*> symbVec;
    te::se::PointSymbolizer* symb = createPointSymbolizer("#000000", "0.0", "#00FF00", "2", "circle", "16");
    symbVec.push_back(symb);

    te::map::GroupingItem* legendItem = new te::map::GroupingItem;
    legendItem->setValue("LIVE");
    legendItem->setCount(0);
    legendItem->setSymbolizers(symbVec);
    legend.push_back(legendItem);
  }

  {
    std::vector<te::se::Symbolizer*> symbVec;
    te::se::PointSymbolizer* symb = createPointSymbolizer("#000000", "0.0", "#FFFFFF", "2", "square", "16");
    symbVec.push_back(symb);

    te::map::GroupingItem* legendItem = new te::map::GroupingItem;
    legendItem->setValue("DEAD");
    legendItem->setCount(0);
    legendItem->setSymbolizers(symbVec);
    legend.push_back(legendItem);
  }

  {
    std::vector<te::se::Symbolizer*> symbVec;
    te::se::PointSymbolizer* symb = createPointSymbolizer("#000000", "0.0", "#FF0000", "1", "circle", "12");
    symbVec.push_back(symb);

    te::map::GroupingItem* legendItem = new te::map::GroupingItem;
    legendItem->setValue("INTRUDER");
    legendItem->setCount(0);
    legendItem->setSymbolizers(symbVec);
    legend.push_back(legendItem);
  }

  {
    std::vector<te::se::Symbolizer*> symbVec;
    te::se::PointSymbolizer* symb = createPointSymbolizer("#000000", "0.0", "#0000FF", "1", "circle", "12");
    symbVec.push_back(symb);

    te::map::GroupingItem* legendItem = new te::map::GroupingItem;
    legendItem->setValue("CREATED");
    legendItem->setCount(0);
    legendItem->setSymbolizers(symbVec);
    legend.push_back(legendItem);
  }

  {
    std::vector<te::se::Symbolizer*> symbVec;
    te::se::PointSymbolizer* symb = createPointSymbolizer("#000000", "0.0", "#000000", "0", "square", "12");
    symbVec.push_back(symb);

    te::map::GroupingItem* legendItem = new te::map::GroupingItem;
    legendItem->setValue("REMOVED");
    legendItem->setCount(0);
    legendItem->setSymbolizers(symbVec);
    legend.push_back(legendItem);
  }
   
  //create grouping
  te::map::Grouping* group = new te::map::Grouping(vecAttr, te::map::UNIQUE_VALUE);
  group->setPropertyType(attrType);
  group->setPrecision(prec);
  group->setStdDeviation(0.);
  group->setGroupingItems(legend);

  //set grouping
  m_outputLayer->setGrouping(group);
}

te::se::PointSymbolizer* te::qt::plugins::tv5plugins::ForestMonitorClassDialog::createPointSymbolizer(std::string fillColor, std::string fillOpacity,
                                                                                                      std::string strokeColor, std::string strokeWidth,
                                                                                                      std::string markName, std::string markSize)
{
  te::se::Fill* markFill = te::se::CreateFill(fillColor, fillOpacity);
  te::se::Stroke* markStroke = te::se::CreateStroke(strokeColor, strokeWidth);
  te::se::Mark* mark = CreateMark(markName, markStroke, markFill);
  te::se::Graphic* graphic = CreateGraphic(mark, markSize, "", "");

  te::se::PointSymbolizer* symbolizer = new te::se::PointSymbolizer;
  symbolizer->setGraphic(graphic);

  return symbolizer;
}

bool te::qt::plugins::tv5plugins::ForestMonitorClassDialog::runClassOperation(std::string repository, std::string dataSetName, std::string vecURI, int vecSRID, std::string rasterURI, int rasterSRID, double threshold, int dilation, int erosion, bool saveResultImage)
{
  //get ndvi raster
  std::map<std::string, std::string> rinfo;
  rinfo["URI"] = rasterURI;

  std::auto_ptr<te::rst::Raster> ndviRst(te::rst::RasterFactory::open(rinfo));
  ndviRst->getGrid()->setSRID(rasterSRID);

  int ndviBand = 0;

  //create new data source
  std::map<std::string, std::string> dsInfo;

  te::da::DataSourcePtr outputDataSource = createDataSource(repository, dsInfo);

  std::size_t idx = dataSetName.find(".");
  if (idx != std::string::npos)
    dataSetName = dataSetName.substr(0, idx);

  std::string repName = repository;

  idx = repName.find(".");
  if (idx != std::string::npos)
    repName = repName.substr(0, idx);

  //create datasource to save polygons information
  std::string polyDataSourcePath = repName + "_polygons" + ".shp";

  std::map<std::string, std::string> polyDsInfo;

  te::da::DataSourcePtr polyOutputDataSource = createDataSource(polyDataSourcePath, polyDsInfo);

  //progress
  if (!m_progressDlg)
  {
    m_progressDlg = new te::qt::widgets::ProgressViewerDialog(this);
    m_progressId = te::common::ProgressManager::getInstance().addViewer(m_progressDlg);
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  try
  {
    std::map<std::string, std::string> rInfo;
    //rInfo["FORCE_MEM_DRIVER"] = "TRUE";

    //std::string type = "MEM";
    std::string type = "GDAL";

    std::vector<te::qt::plugins::tv5plugins::CentroidInfo*> centroidsVec;

    //open vector data
    std::map<std::string, std::string> connInfo;
    connInfo["URI"] = vecURI;

    std::auto_ptr<te::da::DataSource> dsptr = te::da::DataSourceFactory::make("OGR");
    dsptr->setConnectionInfo(connInfo);
    dsptr->open();
    std::vector<std::string> dataSetNames = dsptr->getDataSetNames();

    std::auto_ptr<te::da::DataSet> dataSet = dsptr->getDataSet(dataSetNames[0]);
    std::auto_ptr<te::da::DataSetType> dataSetType = dsptr->getDataSetType(dataSetNames[0]);

    std::size_t gpos = te::da::GetFirstPropertyPos(dataSet.get(), te::dt::GEOMETRY_TYPE);
    te::gm::GeometryProperty* geomProp = te::da::GetFirstGeomProperty(dataSetType.get());

    bool remap = false;

    if (vecSRID != rasterSRID)
      remap = true;

    te::da::PrimaryKey* pk = dataSetType->getPrimaryKey();
    std::string name = pk->getProperties()[0]->getName();

    std::size_t size = dataSet->size();

    dataSet->moveBeforeFirst();

    te::common::TaskProgress task("Associating Centroids");
    task.setTotalSteps(size);

    std::vector<te::gm::Geometry*> fullGeomVec;

    //get geometries
    while (dataSet->moveNext())
    {
      if (!task.isActive())
      {
        break;
      }

      std::auto_ptr<te::gm::Geometry> g(dataSet->getGeometry(gpos));

      if (!g->isValid())
      {
        continue;
      }

      g->setSRID(vecSRID);

      if (remap)
        g->transform(ndviRst->getSRID());

      int parcelId = dataSet->getInt32(name);

      te::gm::Polygon* poly = 0;

      if (g->getGeomTypeId() == te::gm::MultiPolygonType)
      {
        te::gm::MultiPolygon* mPoly = dynamic_cast<te::gm::MultiPolygon*>(g.get());

        poly = dynamic_cast<te::gm::Polygon*>(mPoly->getGeometryN(0));
      }
      else if (g->getGeomTypeId() == te::gm::PolygonType)
      {
        poly = dynamic_cast<te::gm::Polygon*>(g.get());
      }

      if (!poly || !poly->isValid())
        continue;

      //create raster crop from parcel
      rInfo["URI"] = repName + "_parcel_" + te::common::Convert2String(parcelId) + ".tif";
      te::rst::RasterPtr parcelRaster(te::rst::CropRaster(*ndviRst.get(), *poly, rInfo, type));

      //create threshold raster
      rInfo["URI"] = repName + "_threshold_" + te::common::Convert2String(parcelId) + ".tif";
      std::auto_ptr<te::rst::Raster> outputRaster = GenerateThresholdRaster(parcelRaster.get(), ndviBand, threshold, type, rInfo);

      //create erosion raster
      if (dilation > 0)
      {
        rInfo["URI"] = repName + "_erosion_" + te::common::Convert2String(parcelId) + ".tif";
        std::auto_ptr<te::rst::Raster> erosionRaster = GenerateFilterRaster(outputRaster.get(), 0, dilation, te::rp::Filter::InputParameters::DilationFilterT, type, rInfo);

        outputRaster.reset(0);

        outputRaster = erosionRaster;
      }

      //create dilation raster
      if (erosion > 0)
      {
        rInfo["URI"] = repName + "_dilation_" + te::common::Convert2String(parcelId) + ".tif";
        std::auto_ptr<te::rst::Raster> dilationRaster = GenerateFilterRaster(outputRaster.get(), 0, erosion, te::rp::Filter::InputParameters::ErosionFilterT, type, rInfo);

        outputRaster.reset(0);

        outputRaster = dilationRaster;
      }

      //export image
      if (saveResultImage)
      {
        std::string repName = repository;

        std::size_t idx = repName.find(".");
        if (idx != std::string::npos)
          repName = repName.substr(0, idx);

        std::string rasterFileName = repName + "_" + te::common::Convert2String(parcelId) + ".tif";

        te::qt::plugins::tv5plugins::ExportRaster(outputRaster.get(), rasterFileName);
      }

      //create geometries
      std::vector<te::gm::Geometry*> geomVec = te::qt::plugins::tv5plugins::Raster2Vector(outputRaster.get(), 0);

      outputRaster.reset(0);

      //get centroids
      te::qt::plugins::tv5plugins::ExtractCentroids(geomVec, centroidsVec, parcelId);

      if (geomVec.size() > 2)
      {
        for (std::size_t t = 1; t < geomVec.size(); ++t)
        {
          fullGeomVec.push_back(geomVec[t]);
        }
      }

      geomVec.clear();
    }

    //export data
    te::qt::plugins::tv5plugins::ExportVector(centroidsVec, dataSetName, "OGR", dsInfo, ndviRst->getSRID());

    te::common::FreeContents(centroidsVec);

    centroidsVec.clear();

    std::string polyDataSetName = dataSetName + "_polygons";

    te::qt::plugins::tv5plugins::ExportPolyVector(fullGeomVec, polyDataSetName, "OGR", polyDsInfo, ndviRst->getSRID());

    te::common::FreeContents(fullGeomVec);

    fullGeomVec.clear();

    //create layer
    te::da::DataSourcePtr outDataSource = te::da::GetDataSource(outputDataSource->getId());

    te::qt::widgets::DataSet2Layer converter(outputDataSource->getId());

    te::da::DataSetTypePtr dt(outDataSource->getDataSetType(dataSetName).release());

    m_outputLayer = converter(dt);

    //create legend
    createLegend();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());

    QApplication::restoreOverrideCursor();

    return false;
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error."));

    QApplication::restoreOverrideCursor();

    return false;
  }

  QApplication::restoreOverrideCursor();

  return true;
}
