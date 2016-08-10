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
  \file terraview5plugins/src/tv5plugins/forestMonitor/qt/tools/Creator.h

  \brief This class implements a concrete tool to create points from layer
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_CREATOR_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_CREATOR_H

// TerraLib
#include <terralib/dataaccess/dataset/DataSetType.h>
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

        enum CreatorType
        {
          CREATED_TYPE,
          LIVE_TYPE,
          DEAD_TYPE
        };

        /*!
          \class Info

          \brief This class implements a concrete tool to create points from layer

          \ingroup widgets
          */
        class Creator : public te::qt::widgets::AbstractTool
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
          Creator(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::qt::plugins::tv5plugins::CreatorType type, QObject* parent = 0);

          /*! \brief Destructor. */
          ~Creator();

          void setLineEditComponents(QLineEdit* distLineEdit);

          //@}

          /** @name AbstractTool Methods
           *  Methods related with tool behavior.
           */
          //@{

          bool eventFilter(QObject* watched, QEvent* e);

          //@}

        protected:

          void selectObjects(QMouseEvent* e);

          void saveObjects();

          void drawSelecteds();

          bool getParcelParentId(te::gm::Point* point, int& id);

          void getStartIdValue();

          void cancelOperation();

          bool panMousePressEvent(QMouseEvent* e);

          bool panMouseMoveEvent(QMouseEvent* e);

          bool panMouseReleaseEvent(QMouseEvent* e);

          void clearBufferPoints(te::da::DataSet* ds);

        private:

          te::map::AbstractLayerPtr m_coordLayer;         //!<The layer that will be classified.
          te::map::AbstractLayerPtr m_parcelLayer;        //!<The layer with geometry restriction.

          std::auto_ptr<te::mem::DataSet> m_dataSet;

          int m_starterId;

          QLineEdit* m_distLineEdit;

          te::qt::plugins::tv5plugins::CreatorType m_type;

          //pan attributes
          bool m_panStarted;      //!< Flag that indicates if pan operation was started.
          QPoint m_origin;        //!< Origin point on mouse pressed.
          QPoint m_delta;         //!< Difference between pressed point and destination point on mouse move.
          QCursor m_actionCursor; //!< An optional cursor to be used during the pan user action.
        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_CREATOR_H
