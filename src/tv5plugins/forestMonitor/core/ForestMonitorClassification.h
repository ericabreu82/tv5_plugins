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

/*! \file terralib/qt/plugins/thirdParty/forestMonitor/core/ForestMonitorClassification.h

    \brief This file contains structures and definitions for forest monitor classification operation.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSIFICATION_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSIFICATION_H

// TerraLib
#include <terralib/core/uri/URI.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/rp/Filter.h>
#include "../../Config.h"

//STL Includes
#include <map>
#include <memory>

namespace te
{
  namespace rst { class Raster; }

  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        enum ForetType
        {
          FOREST_UNKNOWN,
          FOREST_LIVE,
          FOREST_DEAD
        };

        struct CentroidInfo
        {
          te::gm::Point* m_point;
          int m_parentId;
          double m_area;
          te::qt::plugins::tv5plugins::ForetType type;
        };



        std::auto_ptr<te::rst::Raster> GenerateFilterRaster(te::rst::Raster* raster, int band, int nIter, te::rp::Filter::InputParameters::FilterType fType,
                                                            std::string type, std::map<std::string, std::string> rinfo);

        std::auto_ptr<te::rst::Raster> GenerateThresholdRaster(te::rst::Raster* raster, int band, double value,
                                                               std::string type, std::map<std::string, std::string> rinfo);


        void ExportRaster(te::rst::Raster* rasterIn, std::string fileName);

        std::vector<te::gm::Geometry*> Raster2Vector(te::rst::Raster* raster, int band);

        void ExtractCentroids(std::vector<te::gm::Geometry*>& geomVec, std::vector<CentroidInfo*>& centroids, int parcelId);

        void AssociateObjects(te::map::AbstractLayer* layer, std::vector<te::qt::plugins::tv5plugins::CentroidInfo*>& points, int srid);

        void ExportVector(std::vector<te::qt::plugins::tv5plugins::CentroidInfo*>& ciVec, std::string dataSetName, std::string dsType, const te::core::URI& connInfo, int srid);

        void ExportPolyVector(std::vector<te::gm::Geometry*>& geomVec, std::string dataSetName, std::string dsType, const te::core::URI& connInfo, int srid);

        void ClearData(te::map::AbstractLayerPtr layer);

      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSIFICATION_H
