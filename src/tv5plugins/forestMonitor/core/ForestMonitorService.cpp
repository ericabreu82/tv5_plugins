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

/*! \file terralib/qt/plugins/thirdParty/forestMonitor/core/ForestMonitorService.cpp

    \brief This file implements the service to monitor the forest information.
*/


//TerraLib Includes
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/memory/DataSet.h>
#include "ForestMonitorService.h"
#include "ForestMonitor.h"

//STL Includes
#include <cassert>
#include <exception>

te::qt::plugins::tv5plugins::ForestMonitorService::ForestMonitorService() :
  m_angleTol(0.),
  m_centroidDist(0.),
  m_distTol(0.),
  m_outputDataSetName("")
{
}

te::qt::plugins::tv5plugins::ForestMonitorService::~ForestMonitorService()
{
}

void te::qt::plugins::tv5plugins::ForestMonitorService::setInputParameters(te::map::AbstractLayerPtr centroidLayer, 
                                                                           te::map::AbstractLayerPtr parcelLayer,
                                                                           te::map::AbstractLayerPtr angleLayer,
                                                                           double angleTol, double centroidDist, double distanceTol)
{
  m_centroidLayer = centroidLayer;

  m_parcelLayer = parcelLayer;

  m_angleLayer = angleLayer;

  m_angleTol = angleTol;

  m_centroidDist = centroidDist;

  m_distTol = distanceTol;
}

void te::qt::plugins::tv5plugins::ForestMonitorService::setOutputParameters(te::da::DataSourcePtr ds, std::string outputDataSetName)
{
  m_ds = ds;

  m_outputDataSetName = outputDataSetName;
}

void te::qt::plugins::tv5plugins::ForestMonitorService::runService()
{
  //check input parameters
  checkParameters();

  //get input data
  std::auto_ptr<te::da::DataSet> parcelDataSet = m_parcelLayer->getData();
  std::auto_ptr<te::da::DataSetType> parcelDsType = m_parcelLayer->getSchema();
  int parcelIdIdx, parcelGeomIdx;
  getDataSetTypeInfo(parcelDsType.get(), parcelIdIdx, parcelGeomIdx);

  std::auto_ptr<te::da::DataSet> angleDataSet = m_angleLayer->getData();
  std::auto_ptr<te::da::DataSetType> angleDsType = m_angleLayer->getSchema();
  int angleIdIdx, angleGeomIdx;
  getDataSetTypeInfo(angleDsType.get(), angleIdIdx, angleGeomIdx);

  std::auto_ptr<te::da::DataSet> centroidDataSet = m_centroidLayer->getData();
  std::auto_ptr<te::da::DataSetType> centroidDsType = m_centroidLayer->getSchema();
  int centroidIdIdx, centroidGeomIdx;
  getDataSetTypeInfo(centroidDsType.get(), centroidIdIdx, centroidGeomIdx);

  //get srid
  int srid = getParcelSRID();

  //create output dataset
  std::auto_ptr<te::da::DataSetType> dsType = createDataSetType(srid);

  std::auto_ptr<te::mem::DataSet> ds(new te::mem::DataSet(dsType.get()));

  //generate tracks
  te::qt::plugins::tv5plugins::ForestMonitor fm(m_angleTol, m_centroidDist, m_distTol,ds.get());

  fm.execute(parcelDataSet, parcelGeomIdx, parcelIdIdx, 
             angleDataSet, angleGeomIdx, angleIdIdx, 
             centroidDataSet, centroidGeomIdx, centroidIdIdx);

  //save output information
  saveDataSet(ds.get(), dsType.get());
}

void te::qt::plugins::tv5plugins::ForestMonitorService::checkParameters()
{
  if(!m_centroidLayer.get())
    throw std::exception("Centroid Layer not defined.");

  if(!m_parcelLayer.get())
    throw std::exception("Parcel Layer not defined.");

  if(!m_angleLayer.get())
    throw std::exception("Angle Layer not defined.");

  if(!m_ds.get())
    throw std::exception("Data Source not defined.");

  if(m_outputDataSetName.empty())
    throw std::exception("Data Source name not defined.");
}

std::auto_ptr<te::da::DataSetType> te::qt::plugins::tv5plugins::ForestMonitorService::createDataSetType(int srid)
{
  std::auto_ptr<te::da::DataSetType> dsType(new te::da::DataSetType(m_outputDataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("trackId", te::dt::INT32_TYPE);
  dsType->add(idProperty);

  //create parcel id property
  te::dt::SimpleProperty* parcelIdProperty = new te::dt::SimpleProperty("parcelId", te::dt::INT32_TYPE);
  dsType->add(parcelIdProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::LineStringType);
  dsType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_" + m_outputDataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dsType.get());
  pk->add(idProperty);

  return dsType;
}

void te::qt::plugins::tv5plugins::ForestMonitorService::saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType)
{
  assert(dataSet);
  assert(dsType);

  //save dataset
  dataSet->moveBeforeFirst();

  std::map<std::string, std::string> options;

  m_ds->createDataSet(dsType, options);

  m_ds->add(m_outputDataSetName, dataSet, options);
}

void te::qt::plugins::tv5plugins::ForestMonitorService::getDataSetTypeInfo(te::da::DataSetType* dsType, int& idIdx, int& geomIdx)
{
  //geom property info
  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(dsType);

  if(gmProp)
  {
    geomIdx = te::da::GetPropertyPos(dsType, gmProp->getName());
  }

  //id info
  te::da::PrimaryKey* pk = dsType->getPrimaryKey();

  if(pk && !pk->getProperties().empty())
  {
    idIdx = te::da::GetPropertyPos(dsType, pk->getProperties()[0]->getName());
  }
}

int te::qt::plugins::tv5plugins::ForestMonitorService::getParcelSRID()
{
  assert(m_parcelLayer);

  std::auto_ptr<te::da::DataSetType> parcelDsType = m_parcelLayer->getSchema();

  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(parcelDsType.get());

  int srid = 0;

  if(gmProp)
  {
    srid = gmProp->getSRID();
  }

  return srid;
}
