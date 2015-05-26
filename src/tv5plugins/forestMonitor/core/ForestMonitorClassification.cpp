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

/*! \file terralib/qt/plugins/thirdParty/forestMonitor/core/ForestMonitorClassification.cpp

    \brief This file contains structures and definitions for forest monitor classification operation.
*/

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/Exception.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/dataset/PrimaryKey.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Utils.h>
#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/raster/Band.h>
#include <terralib/raster/BandProperty.h>
#include <terralib/raster/Grid.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/Utils.h>
#include "ForestMonitorClassification.h"

//STL Includes
#include <cassert>

std::auto_ptr<te::rst::Raster> te::qt::plugins::tv5plugins::GenerateFilterRaster(te::rst::Raster* raster, int band, int nIter,
  te::rp::Filter::InputParameters::FilterType fType, std::string type, std::map<std::string, std::string> rinfo)
{
  std::auto_ptr<te::rst::Raster> rasterOut;

  te::rp::Filter algorithmInstance;

  te::rp::Filter::InputParameters algoInputParams;
  algoInputParams.m_iterationsNumber = nIter;
  algoInputParams.m_filterType = fType;
  algoInputParams.m_inRasterBands.push_back(band);
  algoInputParams.m_inRasterPtr = raster;
  algoInputParams.m_enableProgress = true;

  te::rp::Filter::OutputParameters algoOutputParams;
  algoOutputParams.m_rType = type;
  algoOutputParams.m_rInfo = rinfo;

  if (algorithmInstance.initialize(algoInputParams))
  {
    if (algorithmInstance.execute(algoOutputParams))
    {
      rasterOut = algoOutputParams.m_outputRasterPtr;
    }
  }

  return rasterOut;
}

std::auto_ptr<te::rst::Raster> te::qt::plugins::tv5plugins::GenerateThresholdRaster(te::rst::Raster* raster, int band, double value,
  std::string type, std::map<std::string, std::string> rinfo)
{
  std::auto_ptr<te::rst::Raster> rasterOut;

  //create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::UCHAR_TYPE);
  bandProp->m_nblocksx = raster->getBand(band)->getProperty()->m_nblocksx;
  bandProp->m_nblocksy = raster->getBand(band)->getProperty()->m_nblocksy;
  bandProp->m_blkh = raster->getBand(band)->getProperty()->m_blkh;
  bandProp->m_blkw = raster->getBand(band)->getProperty()->m_blkw;
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(raster->getGrid()));

  te::rst::Raster* rOut = te::rst::RasterFactory::make(type, grid, bandsProperties, rinfo);

  rasterOut.reset(rOut);

  te::common::TaskProgress task("Generating Threshold Raster");
  task.setTotalSteps(raster->getNumberOfRows());

  //fill threshold raster
  for (unsigned int i = 0; i < raster->getNumberOfRows(); ++i)
  {
    for (unsigned int j = 0; j < raster->getNumberOfColumns(); ++j)
    {
      double curValue;

      raster->getValue(j, i, curValue);

      if (curValue <= value)
      {
        rasterOut->setValue(j, i, 255.);
      }
      else
      {
        rasterOut->setValue(j, i, 0.);
      }
    }

    task.pulse();
  }

  return rasterOut;
}

void te::qt::plugins::tv5plugins::ExportRaster(te::rst::Raster* rasterIn, std::string fileName)
{
  assert(rasterIn);

  te::rst::CreateCopy(*rasterIn, fileName);
}

std::vector<te::gm::Geometry*> te::qt::plugins::tv5plugins::Raster2Vector(te::rst::Raster* raster, int band)
{
  assert(raster);

  //create vectors
  std::vector<te::gm::Geometry*> geomVec;

  raster->vectorize(geomVec, band, 0);

  return geomVec;
}

std::vector<te::qt::plugins::tv5plugins::CentroidInfo*> te::qt::plugins::tv5plugins::ExtractCentroids(std::vector<te::gm::Geometry*>& geomVec)
{
  std::vector<te::qt::plugins::tv5plugins::CentroidInfo*> points;

  for(std::size_t t = 0; t < geomVec.size(); ++t)
  {
    te::gm::Geometry* geom = geomVec[t];

    te::gm::Point* point = 0;

    double area = 0.;

    if(geom->getGeomTypeId() == te::gm::MultiPolygonType)
    {
      te::gm::MultiPolygon* mp = dynamic_cast<te::gm::MultiPolygon*>(geom);

      te::gm::Polygon* p = dynamic_cast<te::gm::Polygon*>(mp->getGeometryN(0));

      point = p->getCentroid();

      area = p->getArea();
    }
    else if(geom->getGeomTypeId() == te::gm::PolygonType)
    {
      te::gm::Polygon* p = dynamic_cast<te::gm::Polygon*>(geom);

      point = p->getCentroid();

      area = p->getArea();
    }

    if (point)
    {
      te::qt::plugins::tv5plugins::CentroidInfo* ci = new te::qt::plugins::tv5plugins::CentroidInfo();

      ci->m_point = point;
      ci->m_area = area;
      ci->m_parentId = -1;

      points.push_back(ci);
    }
  }

  return points;
}

void te::qt::plugins::tv5plugins::AssociateObjects(te::map::AbstractLayer* layer, std::vector<te::qt::plugins::tv5plugins::CentroidInfo*>& points, int srid)
{
  std::auto_ptr<te::da::DataSet> dataSet = layer->getData();
  std::auto_ptr<te::da::DataSetType> dataSetType = layer->getSchema();

  std::size_t gpos = te::da::GetFirstPropertyPos(dataSet.get(), te::dt::GEOMETRY_TYPE);
  te::gm::GeometryProperty* geomProp = te::da::GetFirstGeomProperty(dataSetType.get());

  bool remap = false;

  if (layer->getSRID() != srid)
    remap = true;

  te::da::PrimaryKey* pk = dataSetType->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();

  dataSet->moveBeforeFirst();

  //get geometries
  while (dataSet->moveNext())
  {
    std::auto_ptr<te::gm::Geometry> g(dataSet->getGeometry(gpos));

    g->setSRID(layer->getSRID());

    if (remap)
      g->transform(srid);

    int id = dataSet->getInt32(name);

    for (std::size_t t = 0; t < points.size(); ++t)
    {
      if (g->contains(points[t]->m_point))
      {
        points[t]->m_parentId = id;
      }
    }
  }

  return;
}

void te::qt::plugins::tv5plugins::ExportVector(std::vector<te::qt::plugins::tv5plugins::CentroidInfo*>& ciVec, std::string dataSetName, std::string dsType, std::map<std::string, std::string> connInfo, int srid)
{
  assert(!ciVec.empty());

  //create dataset type
  std::auto_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
  dataSetType->add(idProperty);

  //create origin id property
  te::dt::SimpleProperty* originIdProperty = new te::dt::SimpleProperty("originId", te::dt::INT32_TYPE);
  dataSetType->add(originIdProperty);

  //create area property
  te::dt::SimpleProperty* areaProperty = new te::dt::SimpleProperty("area", te::dt::DOUBLE_TYPE);
  dataSetType->add(areaProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::PointType);
  dataSetType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_id";
  pkName += "_" + dataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dataSetType.get());
  pk->add(idProperty);

  //create data set
  std::auto_ptr<te::mem::DataSet> dataSetMem(new te::mem::DataSet(dataSetType.get()));

  for (std::size_t t = 0; t < ciVec.size(); ++t)
  {
    if (ciVec[t]->m_parentId == -1)
      continue;

    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(dataSetMem.get());

    //set id
    item->setInt32("id", (int)t);

    //set origin id
    item->setInt32("originId", ciVec[t]->m_parentId);

    //set area
    item->setDouble("area", ciVec[t]->m_area);

    //set geometry
    te::gm::Point* pClone = new te::gm::Point(*ciVec[t]->m_point);

    item->setGeometry("geom", pClone);

    dataSetMem->add(item);
  }

  dataSetMem->moveBeforeFirst();

  //save data set
  std::auto_ptr<te::da::DataSource> dataSource = te::da::DataSourceFactory::make(dsType);
  dataSource->setConnectionInfo(connInfo);
  dataSource->open();

  std::map<std::string, std::string> options;
  dataSource->createDataSet(dataSetType.get(), options);
  dataSource->add(dataSetName, dataSetMem.get(), options);
}
