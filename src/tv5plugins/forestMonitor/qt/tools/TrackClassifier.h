/* Copyright (C) 2014-2015 Geopixel Soluções em Geotecnologia e TI (Brazil)

    This file is part of Geopixel GLUE (R)  - Geographic Leverage Universal Engine 
    a Framework for building GIS applications in heterogeneous environment.

    GLUE is a proprietary software of Geopixel (R) and is licensed under a GLUE License.

    GLUE is developed on top of TerraLib, an open source Library developed by 
    National Institute For Space Research (INPE) - Brazil distributed  under the
    terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.
 */

/*!
  \file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/TrackClassifier.h

  \brief This class implements a concrete tool to track classifier
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKCLASSIFIER_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKCLASSIFIER_H

// TerraLib
#include <terralib/dataaccess/dataset/ObjectIdSet.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/qt/widgets/tools/AbstractTool.h>
#include "../../../Config.h"

// STL
#include <list>
#include <string>

namespace te
{
  namespace gm { class Geometry; }

  namespace qt
  {
    namespace widgets
    {
      class MapDisplay;
    }

    namespace plugins
    {
      namespace tv5plugins
      {

        /*!
          \class TrackClassifier

          \brief This class implements a concrete tool to track classifier

          \ingroup widgets
          */
        class TrackClassifier : public te::qt::widgets::AbstractTool
        {
          Q_OBJECT

        public:

          /** @name Initializer Methods
           *  Methods related to instantiation and destruction.
           */
          //@{

          /*!
            \brief It constructs a info tool associated with the given map display.

            \param display The map display associated with the tool.
            \param cursor The tool cursor.
            \param layers  The layer list that will be queried.
            \param parent  The tool's parent.

            \note The tool will NOT take the ownership of the given pointers.
            */
          TrackClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, QObject* parent = 0);

          /*! \brief Destructor. */
          ~TrackClassifier();

          //@}

          /** @name AbstractTool Methods
           *  Methods related with tool behavior.
           */
          //@{

          bool mouseReleaseEvent(QMouseEvent* e);

          //@}

        protected:

          void selectObjects(QMouseEvent* e);

          void classifyObjects();

          void drawSelecteds();

          te::gm::Geometry* createBuffer(std::auto_ptr<te::da::DataSet> dataset, int srid, std::string gpName);

          void getTrackInfo(std::auto_ptr<te::da::DataSet> dataset, std::string gpName, double& distance, double& angle, double& invertedAngle);

          std::auto_ptr<te::gm::Geometry> getParcelGeeom(te::gm::Geometry* root);

          double getAngle(te::gm::Geometry* first, te::gm::Geometry* last);

          bool centroidsSameTrack(te::gm::Point* first, te::gm::Point* last, double parcelAngle);

          te::gm::Point* createGuessPoint(te::gm::Point* p, double distance, double angle, int srid);

          bool rotate(te::gm::Coord2D pr, te::gm::LineString* l, double angle, te::gm::LineString* lOut);

        private:

          te::map::AbstractLayerPtr m_coordLayer;         //!<The layer that will be classified.
          te::map::AbstractLayerPtr m_parcelLayer;        //!<The layer with geometry restriction.

          te::da::ObjectIdSet* m_objIdSet;

          te::sam::rtree::Index<int> m_centroidRtree;
          std::map<int, te::gm::Geometry*> m_centroidGeomMap;
        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKCLASSIFIER_H
