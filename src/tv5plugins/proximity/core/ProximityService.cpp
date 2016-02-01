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

/*! \file terralib/qt/plugins/thirdParty/Proximity/core/ProximityService.cpp

    \brief This file implements the service to photo index information.
*/


//TerraLib Includes
#include <terralib/common/Exception.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/GeometryProperty.h>
#include "ProximityService.h"
#include "Proximity.h"

//STL Includes
#include <cassert>
#include <exception>

te::qt::plugins::tv5plugins::ProximityService::ProximityService() : 
  m_attrName(""),
  m_distBuffer(0.)
{
}

te::qt::plugins::tv5plugins::ProximityService::~ProximityService()
{
}

void te::qt::plugins::tv5plugins::ProximityService::setInputParameters(std::auto_ptr<te::da::DataSet> inputData, std::auto_ptr<te::da::DataSetType> inputDataType,
                                                                       std::auto_ptr<te::da::DataSet> outputData, std::auto_ptr<te::da::DataSetType> outputDataType,
                                                                       std::string attrName, double distBuffer)
{
  m_inputData = inputData;
  m_inputDataType = inputDataType;

  m_outputData = outputData;
  m_outputDataType = outputDataType;
  m_attrName = attrName;

  m_distBuffer = distBuffer;
}

void te::qt::plugins::tv5plugins::ProximityService::runService()
{
  //check input parameters
  checkParameters();

  m_result.clear();

  //build tree with output elements
  te::sam::rtree::Index<int> rtree;

  std::map<int, te::gm::Geometry*> mapGeom;
  std::map<int, std::string> mapAttr;

  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(m_outputDataType.get());

  int outputSRID = gmProp->getSRID();

  int geomIdx = te::da::GetPropertyPos(m_outputDataType.get(), gmProp->getName());

  te::da::PrimaryKey* pk = m_outputDataType->getPrimaryKey();

  int idIdx = te::da::GetPropertyPos(m_outputDataType.get(), pk->getProperties()[0]->getName());
  int attrIdx = te::da::GetPropertyPos(m_outputDataType.get(), m_attrName);

  m_outputData->moveBeforeFirst();

  while (m_outputData->moveNext())
  {
    std::string strId = m_outputData->getAsString(idIdx);
    std::string strAttr = m_outputData->getAsString(attrIdx);

    int id = atoi(strId.c_str());

    te::gm::Geometry* g = m_outputData->getGeometry(geomIdx).release();
    const te::gm::Envelope* box = g->getMBR();

    rtree.insert(*box, id);
    mapGeom.insert(std::map<int, te::gm::Geometry*>::value_type(id, g));
    mapAttr.insert(std::map<int, std::string>::value_type(id, strAttr));
  }

  //start search
  te::gm::GeometryProperty* gmPropInput = te::da::GetFirstGeomProperty(m_inputDataType.get());

  int inputSRID = gmPropInput->getSRID();

  int geomIdxInput = te::da::GetPropertyPos(m_inputDataType.get(), gmPropInput->getName());

  m_inputData->moveBeforeFirst();

  while (m_inputData->moveNext())
  {
    std::auto_ptr<te::gm::Geometry> g = m_inputData->getGeometry(geomIdxInput);

    if (g->isValid())
    {
      if (inputSRID != outputSRID)
        g->transform(outputSRID);

      //calculate buffer
      te::gm::Geometry* geomBuffer = g->buffer(m_distBuffer);

      const te::gm::Envelope* box = geomBuffer->getMBR();

      //search
      std::vector<int> resultsTree;

      rtree.search(*box, resultsTree);

      //check and save info
      for (std::size_t t = 0; t < resultsTree.size(); ++t)
      {
        te::gm::Geometry* gItem = mapGeom[resultsTree[t]];

        if (gItem->intersects(geomBuffer))
        {
          m_result.push_back(mapAttr[resultsTree[t]]);
        }
      }
    }
  }

  te::common::FreeContents(mapGeom);
  mapAttr.clear();
}

std::vector<std::string> te::qt::plugins::tv5plugins::ProximityService::getResult()
{
  return m_result;
}

void te::qt::plugins::tv5plugins::ProximityService::checkParameters()
{
  if (!m_inputData.get() || !m_inputDataType.get())
  {
    throw te::common::Exception("Error on input data.");
  }

  if (!m_outputData.get() || !m_outputDataType.get())
  {
    throw te::common::Exception("Error on output data.");
  }

  if (m_attrName.empty())
  {
    throw te::common::Exception("Error Attr Name is empty.");
  }
}
