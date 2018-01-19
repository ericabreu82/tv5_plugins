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
  \file terralib/qt/plugins/thirdParty/PhotoIndex/qt/PhotoIndexDialog.cpp

  \brief This interface is used to get the input parameters for photo index information.
*/

// TerraLib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/qt/widgets/datasource/selector/DataSourceSelectorDialog.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include "../core/PhotoIndexService.h"
#include "PhotoIndexDialog.h"
#include "ui_PhotoIndexDialogForm.h"

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QValidator>

// Boost
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::tv5plugins::PhotoIndexDialog::PhotoIndexDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::PhotoIndexDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_dirToolButton->setIcon(QIcon::fromTheme("folder"));
  m_ui->m_targetDatasourceToolButton->setIcon(QIcon::fromTheme("datasource"));

  // connectors
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), this, SLOT(onOkPushButtonClicked()));
  connect(m_ui->m_targetDatasourceToolButton, SIGNAL(pressed()), this, SLOT(onTargetDatasourceToolButtonPressed()));
  connect(m_ui->m_targetFileToolButton, SIGNAL(pressed()), this,  SLOT(onTargetFileToolButtonPressed()));
  connect(m_ui->m_dirToolButton, SIGNAL(clicked()), this, SLOT(onDirToolButtonClicked()));
}

te::qt::plugins::tv5plugins::PhotoIndexDialog::~PhotoIndexDialog()
{

}

te::map::AbstractLayerPtr te::qt::plugins::tv5plugins::PhotoIndexDialog::getOutputLayer()
{
  return m_outputLayer;
}

void te::qt::plugins::tv5plugins::PhotoIndexDialog::onDirToolButtonClicked()
{
  QString filePath = QFileDialog::getExistingDirectory(this, tr("Select a folder with images."));
  
  if(filePath.isEmpty())
    return;

  m_ui->m_dirLineEdit->setText(filePath);
}

void te::qt::plugins::tv5plugins::PhotoIndexDialog::onOkPushButtonClicked()
{  
  // check input parameters
  if(m_ui->m_repositoryLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a repository for the result."));
    return;
  }
       
  if(m_ui->m_newLayerNameLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a name for the resulting layer."));
    return;
  }

  if(m_ui->m_dirLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define the path for input images."));
    return;
  }

  std::string path = m_ui->m_dirLineEdit->text().toStdString();

  //get datasource
  te::da::DataSourcePtr outputDataSource;

  if(m_toFile)
  {
    std::string repository = m_ui->m_repositoryLineEdit->text().toStdString();

    //create new data source
    boost::filesystem::path uri(repository);

    std::string dsinfo("file://" + uri.string());

    boost::uuids::basic_random_generator<boost::mt19937> gen;
    boost::uuids::uuid u = gen();
    std::string id_ds = boost::uuids::to_string(u);

    te::da::DataSourceInfoPtr dsInfoPtr(new te::da::DataSourceInfo);
    dsInfoPtr->setConnInfo(dsinfo);
    dsInfoPtr->setTitle(uri.stem().string());
    dsInfoPtr->setAccessDriver("OGR");
    dsInfoPtr->setType("OGR");
    dsInfoPtr->setDescription(uri.string());
    dsInfoPtr->setId(id_ds);

    te::da::DataSourceInfoManager::getInstance().add(dsInfoPtr);

    outputDataSource = te::da::DataSourceManager::getInstance().get(id_ds, "OGR", dsInfoPtr->getConnInfo());

    m_outputDatasource = dsInfoPtr;
  }
  else
  {
    outputDataSource = te::da::GetDataSource(m_outputDatasource->getId());
  }

  //create datasource to save the output information
  std::string dataSetName = m_ui->m_newLayerNameLineEdit->text().toStdString();

  std::size_t idx = dataSetName.find(".");
  if (idx != std::string::npos)
        dataSetName=dataSetName.substr(0,idx);


  //progress
  te::qt::widgets::ProgressViewerDialog v(this);
  int id = te::common::ProgressManager::getInstance().addViewer(&v);

  QApplication::setOverrideCursor(Qt::WaitCursor);

  try
  {
    te::qt::plugins::tv5plugins::PhotoIndexService pis;

    pis.setInputParameters(path);

    pis.setOutputParameters(outputDataSource, dataSetName);

    pis.runService();

    //create layer
    te::da::DataSourcePtr outDataSource = te::da::GetDataSource(m_outputDatasource->getId());
    
    te::qt::widgets::DataSet2Layer converter(m_outputDatasource->getId());
      
    te::da::DataSetTypePtr dt(outDataSource->getDataSetType(dataSetName).release());

    m_outputLayer = converter(dt);
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

  te::common::ProgressManager::getInstance().removeViewer(id);

  QApplication::restoreOverrideCursor();

  accept();
}

void te::qt::plugins::tv5plugins::PhotoIndexDialog::onTargetDatasourceToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_newLayerNameLineEdit->setEnabled(true);

  te::qt::widgets::DataSourceSelectorDialog dlg(this);
  dlg.exec();

  std::list<te::da::DataSourceInfoPtr> dsPtrList = dlg.getSelecteds();

  if(dsPtrList.size() <= 0)
    return;

  std::list<te::da::DataSourceInfoPtr>::iterator it = dsPtrList.begin();

  m_ui->m_repositoryLineEdit->setText(QString(it->get()->getTitle().c_str()));

  m_outputDatasource = *it;
  
  m_toFile = false;
}

void te::qt::plugins::tv5plugins::PhotoIndexDialog::onTargetFileToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_repositoryLineEdit->clear();
  
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("Shapefile (*.shp *.SHP);;"),0, QFileDialog::DontConfirmOverwrite);
  
  if (fileName.isEmpty())
    return;
  
  boost::filesystem::path outfile(fileName.toStdString());

  m_ui->m_repositoryLineEdit->setText(outfile.string().c_str());

  m_ui->m_newLayerNameLineEdit->setText(outfile.leaf().string().c_str());

  m_ui->m_newLayerNameLineEdit->setEnabled(false);
  
  m_toFile = true;
}
