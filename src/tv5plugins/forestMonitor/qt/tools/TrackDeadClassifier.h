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
  \file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/TrackDeadClassifier.h

  \brief This class implements a concrete tool to track dead roots classifier
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKDEADCLASSIFIER_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKDEADCLASSIFIER_H

// TerraLib
#include <terralib/dataaccess/dataset/ObjectIdSet.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/memory/DataSet.h>
#include <terralib/qt/widgets/tools/AbstractTool.h>
#include "../../../Config.h"

// STL
#include <list>
#include <string>

// QT
#include <QLineEdit>

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
          \class TrackDeadClassifier

          \brief This class implements a concrete tool to track dead roots classifier

          \ingroup widgets
          */
        class TrackDeadClassifier : public te::qt::widgets::AbstractTool
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
          TrackDeadClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::map::AbstractLayerPtr rasterLayer, QObject* parent = 0);

          /*! \brief Destructor. */
          ~TrackDeadClassifier();

          void setLineEditComponents(QLineEdit* distLineEdit, QLineEdit* distanceToleranceFactorLineEdit, QLineEdit* threshold);

          //@}

          /** @name AbstractTool Methods
           *  Methods related with tool behavior.
           */
          //@{

          bool eventFilter(QObject* watched, QEvent* e);

          //@}

        protected:

          void selectObjects(QMouseEvent* e);

          void classifyObjects();

          void cancelOperation();

          void drawSelecteds();

          void createDeadTrack(int srid, std::string gpName, std::list<te::gm::Point*>& track);

          void getTrackInfo();

          std::auto_ptr<te::gm::Geometry> getParcelGeeom(te::gm::Geometry* root, int& parcelId);

          te::gm::Point* createGuessPoint(te::gm::Point* p, double dx, double dy, int srid);

          void createRTree();

          te::gm::Point* getPoint(te::gm::Geometry* g);

          void getStartIdValue();

          bool deadTrackMouseMove(QMouseEvent* e);

          bool panMousePressEvent(QMouseEvent* e);

          bool panMouseMoveEvent(QMouseEvent* e);

          bool panMouseReleaseEvent(QMouseEvent* e);

        private:

          te::map::AbstractLayerPtr m_coordLayer;         //!<The layer that will be classified.
          te::map::AbstractLayerPtr m_parcelLayer;        //!<The layer with geometry restriction.
          
          te::rst::Raster* m_ndviRaster;

          te::sam::rtree::Index<int> m_centroidRtree;
          std::map<int, te::gm::Geometry*> m_centroidGeomMap;
          std::map<int, te::da::ObjectId*> m_centroidObjIdMap;

          te::gm::Point* m_point0;
          //te::da::ObjectId* m_objId0;

          te::gm::Point* m_point1;
          //te::da::ObjectId* m_objId1;

          std::auto_ptr<te::mem::DataSet> m_dataSet;

          QLineEdit* m_distLineEdit;
          QLineEdit* m_distanceToleranceFactorLineEdit;
          QLineEdit* m_thresholdLineEdit;

          int m_starterId;

          double m_dx;
          double m_dy;
          double m_distance;

          //pan attributes
          bool m_panStarted;      //!< Flag that indicates if pan operation was started.
          QPoint m_origin;        //!< Origin point on mouse pressed.
          QPoint m_delta;         //!< Difference between pressed point and destination point on mouse move.
          QCursor m_actionCursor; //!< An optional cursor to be used during the pan user action.
          QCursor m_curCursor;

          double m_totalDistance;
        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKDEADCLASSIFIER_H
