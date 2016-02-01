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
  \file terralib/qt/plugins/thirdParty/Proximity/qt/ProximityDialog.cpp

  \brief This interface is used to get the input parameters for photo index information.
*/

// TerraLib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/dataset/ObjectIdSet.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include "../core/ProximityService.h"
#include "ProximityDialog.h"
#include "ui_ProximityDialogForm.h"

// Qt
#include <QMessageBox>
#include <QValidator>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::tv5plugins::ProximityDialog::ProximityDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::ProximityDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_distanceLineEdit->setValidator(new QDoubleValidator(this));

  // connectors
  connect(m_ui->m_targetLayerComboBox, SIGNAL(activated(int)), this, SLOT(onTargetLayerComboBoxActivated(int)));
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), this, SLOT(onOkPushButtonClicked()));
}

te::qt::plugins::tv5plugins::ProximityDialog::~ProximityDialog()
{

}

void te::qt::plugins::tv5plugins::ProximityDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  //clear combos
  m_ui->m_originLayerComboBox->clear();
  m_ui->m_targetLayerComboBox->clear();

  //fill combos
  std::list<te::map::AbstractLayerPtr>::iterator it = list.begin();

  while (it != list.end())
  {
    te::map::AbstractLayerPtr l = *it;

    if (l->isValid())
    {
      std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

      if (dsType->hasGeom())
      {
        m_ui->m_originLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_targetLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
    }

    ++it;
  }

  // fill attributes combo
  if (m_ui->m_targetLayerComboBox->count() > 0)
    onTargetLayerComboBoxActivated(0);
}

void te::qt::plugins::tv5plugins::ProximityDialog::onTargetLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_targetLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr l = varLayer.value<te::map::AbstractLayerPtr>();

  std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

  std::vector<te::dt::Property*> propVec = dsType->getProperties();

  m_ui->m_targetAttrComboBox->clear();

  for (std::size_t t = 0; t < propVec.size(); ++t)
  {
    int dataType = propVec[t]->getType();

    m_ui->m_targetAttrComboBox->addItem(propVec[t]->getName().c_str(), dataType);
  }
}

void te::qt::plugins::tv5plugins::ProximityDialog::onOkPushButtonClicked()
{
  // check input parameters
  if (m_ui->m_originLayerComboBox->currentText().isEmpty() || 
      m_ui->m_targetLayerComboBox->currentText().isEmpty() ||
      m_ui->m_targetAttrComboBox->currentText().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Input parameters not defined."));
    return;
  }

  double distance = 0.;
  if (m_ui->m_bufferGroupBox->isChecked() && m_ui->m_distanceLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Buffer distance not defined."));
    return;
  }

  if (m_ui->m_bufferGroupBox->isChecked())
    distance = m_ui->m_distanceLineEdit->text().toDouble();
  
  //get selected obj id from input layer
  QVariant varLayer = m_ui->m_originLayerComboBox->itemData(m_ui->m_originLayerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr l = varLayer.value<te::map::AbstractLayerPtr>();

  const te::da::ObjectIdSet* objIdSet = l->getSelected();

  if (!objIdSet || objIdSet->size() == 0)
  {
    QMessageBox::information(this, tr("Warning"), tr("Select at least one geometry on input layer."));
    return;
  }

  //get output layer
  QVariant varLayerOut = m_ui->m_targetLayerComboBox->itemData(m_ui->m_targetLayerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr lOut = varLayerOut.value<te::map::AbstractLayerPtr>();

  //execute operation
  ProximityService ps;

  //set input parameters
  std::auto_ptr<te::da::DataSet> inputData = l->getData(objIdSet);
  std::auto_ptr<te::da::DataSetType> inputDataType = l->getSchema();

  std::auto_ptr<te::da::DataSet> outputData = lOut->getData();
  std::auto_ptr<te::da::DataSetType> outputDataType = lOut->getSchema();
  std::string attrName = m_ui->m_targetAttrComboBox->currentText().toStdString();

  ps.setInputParameters(inputData, inputDataType, outputData, outputDataType, attrName, distance);

  //run service
  try
  {
    ps.runService();
  }
  catch (te::common::Exception& e)
  {
    QMessageBox::warning(this, tr("Proximity"), e.what());
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Proximity"), tr("Internal Error"));
  }

  //fill interface with result
  std::vector<std::string> result = ps.getResult();

  m_ui->m_listWidget->clear();

  for (std::size_t t = 0; t < result.size(); ++t)
  {
    m_ui->m_listWidget->addItem(result[t].c_str());
  }
}
