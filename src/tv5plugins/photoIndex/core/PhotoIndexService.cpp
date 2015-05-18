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

/*! \file terralib/qt/plugins/thirdParty/PhotoIndex/core/PhotoIndexService.cpp

    \brief This file implements the service to photo index information.
*/


//TerraLib Includes
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/memory/DataSet.h>
#include "PhotoIndexService.h"
#include "PhotoIndex.h"

//STL Includes
#include <cassert>
#include <exception>

te::qt::plugins::tv5plugins::PhotoIndexService::PhotoIndexService() :
  m_path(""),
  m_outputDataSetName("")
{
}

te::qt::plugins::tv5plugins::PhotoIndexService::~PhotoIndexService()
{
}

void te::qt::plugins::tv5plugins::PhotoIndexService::setInputParameters(std::string path)
{
  m_path = path;
}

void te::qt::plugins::tv5plugins::PhotoIndexService::setOutputParameters(te::da::DataSourcePtr ds, std::string outputDataSetName)
{
  m_ds = ds;

  m_outputDataSetName = outputDataSetName;
}

void te::qt::plugins::tv5plugins::PhotoIndexService::runService()
{
  //check input parameters
  checkParameters();

  //get input data source
  std::map<std::string, std::string> connInfo;
  connInfo["URI"] = m_path;

  te::da::DataSourcePtr inputDataSource = te::da::DataSourceFactory::make("GDAL");
  inputDataSource->setConnectionInfo(connInfo);
  inputDataSource->open();

  //get srid
  int srid = getSRID(inputDataSource);

  //create output dataset
  std::auto_ptr<te::da::DataSetType> dsType = createDataSetType(srid);

  std::auto_ptr<te::mem::DataSet> ds(new te::mem::DataSet(dsType.get()));

  //generate tracks
  te::qt::plugins::tv5plugins::PhotoIndex pi;

  pi.execute(inputDataSource, ds.get());

  //save output information
  saveDataSet(ds.get(), dsType.get());
}

void te::qt::plugins::tv5plugins::PhotoIndexService::checkParameters()
{
  if(!m_ds.get())
    throw std::exception("Data Source not defined.");

  if(m_outputDataSetName.empty())
    throw std::exception("Data Source name not defined.");

  if(m_path.empty())
    throw std::exception("Path not defined.");
}

std::auto_ptr<te::da::DataSetType> te::qt::plugins::tv5plugins::PhotoIndexService::createDataSetType(int srid)
{
  std::auto_ptr<te::da::DataSetType> dsType(new te::da::DataSetType(m_outputDataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
  dsType->add(idProperty);

  //create parcel id property
  te::dt::SimpleProperty* parcelIdProperty = new te::dt::SimpleProperty("fileName", te::dt::STRING_TYPE);
  dsType->add(parcelIdProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::PolygonType);
  dsType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_" + m_outputDataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dsType.get());
  pk->add(idProperty);

  return dsType;
}

void te::qt::plugins::tv5plugins::PhotoIndexService::saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType)
{
  assert(dataSet);
  assert(dsType);

  //save dataset
  dataSet->moveBeforeFirst();

  std::map<std::string, std::string> options;

  m_ds->createDataSet(dsType, options);

  m_ds->add(m_outputDataSetName, dataSet, options);
}

int te::qt::plugins::tv5plugins::PhotoIndexService::getSRID(te::da::DataSourcePtr ds)
{
  std::vector<std::string> dsNames = ds->getDataSetNames();

  std::auto_ptr<te::da::DataSet> firstDataSet = ds->getDataSet(dsNames[0]);

  std::size_t rpos = te::da::GetFirstPropertyPos(firstDataSet.get(), te::dt::RASTER_TYPE);

  std::auto_ptr<te::rst::Raster> inputRst = firstDataSet->getRaster(rpos);

  return inputRst->getSRID();
}
