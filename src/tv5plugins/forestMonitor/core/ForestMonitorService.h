﻿/*  Copyright (C) 2011-2012 National Institute For Space Research (INPE) - Brazil.

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

/*! \file terralib/qt/plugins/thirdParty/forestMonitor/core/ForestMonitorService.h

    \brief This file implements the service to monitor the forest information.

    - get all centroids from centroidLayer and add into a rtree
    - get all lines from angleLayer and add into another rtree
    - get all polygons from parcelLayer and iterate over the elements
        - get line and calculate the angle for the current polygon
        - get all centroids that is inside the current polygon box
        - for each centroid check if in inside the current polygon geometry
              - get the centroid neighboors from the current centroid using the centroidDistance
                  - check if the centroid neighbor is inside polygon geometry and is at a correctly angle
                      - if ok create a line from current centroid to this point
    - check if all lines do not intercept each other
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORSERVICE_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORSERVICE_H

//TerraLib Includes
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

namespace te
{
  //forward declarations
  namespace da  { class DataSetType; }
  namespace mem { class DataSet; }

  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        //forward declarations
        class ForestMonitor;

        class ForestMonitorService
        {
          public:

            ForestMonitorService();

            ~ForestMonitorService();

          public:

            void setInputParameters(te::map::AbstractLayerPtr centroidLayer, 
                                    te::map::AbstractLayerPtr parcelLayer,
                                    te::map::AbstractLayerPtr angleLayer,
                                    double angleTol, double centroidDist, double distanceTol);

            void setOutputParameters(te::da::DataSourcePtr ds, std::string outputDataSetName);

            void runService();

          protected:

            void checkParameters();

            /*! Function used to create the output dataset type */
            std::auto_ptr<te::da::DataSetType> createDataSetType(int srid);

            /*! Function used to save the output dataset */
            void saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType);

            void getDataSetTypeInfo(te::da::DataSetType* dsType, int& idIdx, int& geomIdx);

            int getParcelSRID();

          protected:

            te::map::AbstractLayerPtr m_centroidLayer;

            te::map::AbstractLayerPtr m_parcelLayer;

            te::map::AbstractLayerPtr m_angleLayer;

            double m_angleTol;

            double m_centroidDist;

            double m_distTol;

            te::da::DataSourcePtr m_ds;                       //!< Pointer to the output datasource.

            std::string m_outputDataSetName;                  //!< Attribute that defines the output dataset name
        };
      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORSERVICE_H
