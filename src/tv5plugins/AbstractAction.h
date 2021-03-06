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

/*!
  \file terralib/qt/plugins/thirdParty/AbstractAction.h

  \brief This file defines the abstract class AbstractAction
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_ABSTRACTACTION_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_ABSTRACTACTION_H

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "Config.h"

// Qt
#include <QObject>
#include <QMenu>
#include <QAction>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        /*!
          \class AbstractAction

          \brief This is an abstract class used to register actions into rp pluging.

        */
        class AbstractAction : public QObject
        {
          Q_OBJECT

          public:

            /*!
              \brief Constructor.

              \param menu The parent menu object.
            */
            AbstractAction(QMenu* menu);

            /*! 
              \brief Destructor. 
            */
            virtual ~AbstractAction();

          protected slots:

            /*!
              \brief Slot function used when a action was selected.

              \param checked Flag used in case a toggle action.
            */
            virtual void onActionActivated(bool checked) = 0;

          protected:

            /*!
              \brief Create and set the actions parameters.

              \param name The action name.

              \param pixmap The action pixmap name.
            */
            void createAction(std::string name, std::string pixmap = "");

            /*!
              \brief Add a new layer into layer explorer widget

              \param layer The layer auto pointer

            */
            void addNewLayer(te::map::AbstractLayerPtr layer);

            /*!
              \brief Get the list of layers from app

              \return The list pf layer auto pointers

            */
            std::list<te::map::AbstractLayerPtr> getLayers();

          protected:

            QMenu* m_menu;          //!< Parent Menu.
            QAction* m_action;      //!< Action used to call the process.
        };

      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_ABSTRACTACTION_H