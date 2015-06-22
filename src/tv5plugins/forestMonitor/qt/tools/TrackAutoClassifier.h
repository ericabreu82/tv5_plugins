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
  \file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/TrackAutoClassifier.h

  \brief This class implements a concrete tool to track classifier
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKAUTOCLASSIFIER_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKAUTOCLASSIFIER_H

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
          \class TrackClassifier

          \brief This class implements a concrete tool to track classifier

          \ingroup widgets
          */
        class TrackAutoClassifier : public te::qt::widgets::AbstractTool
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
          TrackAutoClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::map::AbstractLayerPtr polyLayer, QObject* parent = 0);

          /*! \brief Destructor. */
          ~TrackAutoClassifier();

          void setLineEditComponents(QLineEdit* dist, QLineEdit* dx, QLineEdit* dy);

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

          void cancelOperation(bool restart = false);

          void drawSelecteds();

          te::gm::Geometry* createBuffer(int srid, std::string gpName, te::gm::LineString*& lineBuffer, std::list<te::gm::Point*>& track);

          void getTrackInfo(double& distance, double& dx, double& dy);

          std::auto_ptr<te::gm::Geometry> getParcelGeeom(te::gm::Geometry* root, int& parcelId);

          te::gm::Point* createGuessPoint(te::gm::Point* p, double dx, double dy, int srid);

          te::da::ObjectIdSet* getBufferObjIdSet();

          void getClassDataSets(te::da::DataSetType* dsType, te::mem::DataSet*& liveDataSet, te::mem::DataSet*& intruderDataSet);

          void createRTree();

          te::gm::Point* getPoint(te::gm::Geometry* g);

          void getStartIdValue();

          bool isClassified(te::da::ObjectId* objId);

          void addGuessPoint(te::gm::Point* p, int parcelId);

          te::gm::Point* getCandidatePoint(te::gm::Point* pRoot, te::gm::Point* pGuess, int srid, std::vector<int>& resultsTree, te::da::ObjectId*& candidateOjbId);

          std::auto_ptr<te::da::DataSetType> createTreeDataSetType();

        private:

          te::map::AbstractLayerPtr m_coordLayer;         //!<The layer that will be classified.
          te::map::AbstractLayerPtr m_parcelLayer;        //!<The layer with geometry restriction.
          te::map::AbstractLayerPtr m_polyLayer;          //!<The layer with polygons geometry.

          te::sam::rtree::Index<int> m_polyRtree;
          std::map<int, te::gm::Geometry*> m_polyGeomMap;

          te::sam::rtree::Index<int> m_centroidRtree;
          std::map<int, te::gm::Geometry*> m_centroidGeomMap;
          std::map<int, te::da::ObjectId*> m_centroidObjIdMap;

          te::gm::Point* m_point0;
          te::da::ObjectId* m_objId0;

          te::gm::Point* m_point1;
          te::da::ObjectId* m_objId1;

          te::gm::Geometry* m_buffer;
          te::da::ObjectIdSet* m_track;

          std::auto_ptr<te::mem::DataSet> m_dataSet;

          int m_starterId;

          QLineEdit* m_distLineEdit;
          QLineEdit* m_dxLineEdit;
          QLineEdit* m_dyLineEdit;

          bool m_sameParcel;
          bool m_classify;
        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACKAUTOCLASSIFIER_H
