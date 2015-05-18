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

/*! \file terralib/qt/plugins/thirdParty/forestMonitor/core/ForestMonitor.h

    \brief This file contains structures and definitions to monitor forest information.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITOR_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITOR_H

// TerraLib
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/memory/DataSet.h>
#include "../../Config.h"

//STL Includes
#include <memory>
#include <set>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        /*!
          \class ForestMonitor
          
          \brief This file contains structures and definitions to monitor forest information.

        */
        class ForestMonitor
        {
        public:

          struct TrackPair
          {
            int m_parcelId;
            int m_parcelSRID;
            double m_parcelAngle;
            std::set<int> m_startCentroids;
          };

          public:

            ForestMonitor(double tolAngle, double distance, double distTol, te::mem::DataSet* ds);

            virtual ~ForestMonitor();

          public:

            void execute(std::auto_ptr<te::da::DataSet> parcelDs, int parcelGeomIdx, int parcelIdIdx,
                         std::auto_ptr<te::da::DataSet> angleDs, int angleGeomIdx, int angleIdIdx,
                         std::auto_ptr<te::da::DataSet> centroidDs, int centroidGeomIdx, int centroidIdIdx);

          protected:

            void setParcelDataSet(std::auto_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

            void setCentroidDataSet(std::auto_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

            void setAngleDataSet(std::auto_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

            void createRTree(te::sam::rtree::Index<int> &tree, std::map<int, te::gm::Geometry*> &geomMap, std::auto_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

            void createParcelLines(te::gm::Geometry* parcelGeom, int parcelId, std::vector<int> centroidsIdx, double angle);

            void createParcelLine(te::gm::Geometry* parcelGeom, int parcelId, std::vector<int> centroidsIdx, double angle, int centroidId);

            std::vector<int> getParcelCentroids(te::gm::Geometry* geom);

            std::vector<int> getCentroidNeighborsCandidates(te::gm::Geometry* parcelGeom, double angle, te::gm::Geometry* centroidGeom);

            double getParcelLineAngle(te::gm::Geometry* geom);

            bool centroidsSameTrack(te::gm::Point* first, te::gm::Point* last, double parcelAngle);

            double getAngle(te::gm::Point* first, te::gm::Point* last);

            te::gm::Envelope createCentroidBox(te::gm::Geometry* geom);

            void saveTrackLines();

            void checkConsistency();

          protected:

            te::sam::rtree::Index<int> m_centroidRtree;
            std::map<int, te::gm::Geometry*> m_centroidGeomMap;

            te::sam::rtree::Index<int> m_angleRtree;
            std::map<int, te::gm::Geometry*> m_angleGeomMap;

            double m_tolAngle;
            double m_distance;
            double m_distTol;

            te::mem::DataSet* m_ds;

            std::map<int, TrackPair> m_trackMap;

            std::set<int> m_ignoredCentroids;

            std::set<int> m_usedCentroids;

            int m_count;
        };

      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITOR_H
