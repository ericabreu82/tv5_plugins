/*  Copyright (C) 2008 National Institute For Space Research (INPE) - Brazil.

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
\file terraview5plugins/src/tv5plugins/forestMonitor/core/ParcelSet.cpp

\brief This class implements a parcelSet definition
*/

// TerraLib
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/raster/Utils.h>

#include "ForestMonitorClassification.h"
#include "ParcelSet.h"

te::qt::plugins::tv5plugins::ParcelSet::ParcelSet(te::map::AbstractLayerPtr parcels, te::map::AbstractLayerPtr angles)
{
  m_parcelLayer = parcels;

  m_angleLayer = angles;
}

te::qt::plugins::tv5plugins::ParcelSet::~ParcelSet()
{
  te::common::FreeContents(m_parcelSet);

  m_parcelSet.clear();
}

void te::qt::plugins::tv5plugins::ParcelSet::classify(te::rst::Raster* ndviRaster, int ndviBand, double threshold, int nErosion, int nDilation, bool exportRaster, std::string rasterPath)
{
  assert(ndviRaster);

  //get input data
  std::auto_ptr<te::da::DataSet> parcelDataSet = m_parcelLayer->getData();
  std::auto_ptr<te::da::DataSetType> parcelDsType = m_parcelLayer->getSchema();
  int parcelIdIdx, parcelGeomIdx;
  getDataSetTypeInfo(parcelDsType.get(), parcelIdIdx, parcelGeomIdx);

  //define the output raster parameters
  std::map<std::string, std::string> rInfo;
  rInfo["FORCE_MEM_DRIVER"] = "TRUE";
  std::string type = "MEM";

  //create task
  std::size_t size = parcelDataSet->size();
  te::common::TaskProgress task("Creating Trees");
  task.setTotalSteps(size);

  //move over the data set
  parcelDataSet->moveBeforeFirst();

  while (parcelDataSet->moveNext())
  {
    if (!task.isActive())
      break;

    //get parcel (polygon)
    std::auto_ptr<te::gm::Geometry> g = parcelDataSet->getGeometry(parcelGeomIdx);

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

    //get parcelId
    std::string strId = parcelDataSet->getAsString(parcelIdIdx);

    int id = atoi(strId.c_str());

    //create raster crop from parcel
    te::rst::RasterPtr parcelRaster(te::rst::CropRaster(*ndviRaster, *poly, rInfo, type));

    std::auto_ptr<te::rst::Raster> thresholdRaster = GenerateThresholdRaster(parcelRaster.get(), ndviBand, threshold, type, rInfo);

    //create erosion raster
    std::auto_ptr<te::rst::Raster> erosionRaster = GenerateFilterRaster(thresholdRaster.get(), 0, nErosion, te::rp::Filter::InputParameters::DilationFilterT, type, rInfo);

    thresholdRaster.reset(0);

    //create dilation raster
    std::auto_ptr<te::rst::Raster> dilationRaster = GenerateFilterRaster(erosionRaster.get(), 0, nDilation, te::rp::Filter::InputParameters::ErosionFilterT, type, rInfo);

    erosionRaster.reset(0);

    if (exportRaster)
    {
      te::qt::plugins::tv5plugins::ExportRaster(dilationRaster.get(), rasterPath);
    }

    //create geometries
    std::vector<te::gm::Geometry*> geomVec = te::qt::plugins::tv5plugins::Raster2Vector(dilationRaster.get(), 0);

    dilationRaster.reset(0);

    //get centroids
   // std::vector<te::qt::plugins::tv5plugins::CentroidInfo*> centroidsVec = te::qt::plugins::tv5plugins::ExtractCentroids(geomVec);

    te::common::FreeContents(geomVec);

    geomVec.clear();

    task.pulse();
  }
}

void te::qt::plugins::tv5plugins::ParcelSet::export(std::string type, std::map<std::string, std::string> connInfo)
{
}

void te::qt::plugins::tv5plugins::ParcelSet::getDataSetTypeInfo(te::da::DataSetType* dsType, int& idIdx, int& geomIdx)
{
  //geom property info
  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(dsType);

  if (gmProp)
  {
    geomIdx = te::da::GetPropertyPos(dsType, gmProp->getName());
  }

  //id info
  te::da::PrimaryKey* pk = dsType->getPrimaryKey();

  if (pk && !pk->getProperties().empty())
  {
    idIdx = te::da::GetPropertyPos(dsType, pk->getProperties()[0]->getName());
  }
}