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

/*! \file terralib/qt/plugins/thirdParty/PhotoIndex/core/PhotoIndex.cpp

    \brief This file contains structures and definitions to photo index information.
*/

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/Utils.h>
#include <terralib/memory/DataSetItem.h>
#include "PhotoIndex.h"

//STL Includes
#include <cassert>

te::qt::plugins::tv5plugins::PhotoIndex::PhotoIndex()
{
}

te::qt::plugins::tv5plugins::PhotoIndex::~PhotoIndex()
{
}

void te::qt::plugins::tv5plugins::PhotoIndex::execute(te::da::DataSourcePtr dataSource, te::mem::DataSet* dataSet)
{
  std::vector<std::string> dataSetNames = dataSource->getDataSetNames();

  te::common::TaskProgress task("Creating Photo Index");
  task.setTotalSteps(dataSetNames.size());

  for(std::size_t t = 0; t < dataSetNames.size(); ++t)
  {
    if(task.isActive() == false)
      break;

    std::string name = dataSetNames[t];

    std::auto_ptr<te::da::DataSet> dataSetRaster = dataSource->getDataSet(name);

    std::size_t rpos = te::da::GetFirstPropertyPos(dataSetRaster.get(), te::dt::RASTER_TYPE);

    std::auto_ptr<te::rst::Raster> inputRst = dataSetRaster->getRaster(rpos);

    te::gm::Geometry* geom = te::gm::GetGeomFromEnvelope(inputRst->getExtent(), inputRst->getSRID());

    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(dataSet);

    //set id
    item->setInt32("id", t);

    //set parcel id
    item->setString("fileName", name);

    //set geometry
    item->setGeometry("geom", geom);

    dataSet->add(item);

    task.pulse();
  }
}