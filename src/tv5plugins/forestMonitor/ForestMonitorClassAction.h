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
  \file terralib/qt/plugins/thirdParty/ForestMonitorAction.h

  \brief This file defines the Forest Monitor Class Action class
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSACTION_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSACTION_H

// TerraLib
#include "../Config.h"
#include "../AbstractAction.h"

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        /*!
          \class ForestMonitorClassAction
          
          \brief This file defines the Forest Monitor Class Action class

        */
        class ForestMonitorClassAction : public te::qt::plugins::tv5plugins::AbstractAction
        {
          Q_OBJECT

          public:

            ForestMonitorClassAction(QMenu* menu);

            virtual ~ForestMonitorClassAction();

          protected slots:

            virtual void onActionActivated(bool checked);
        };

      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_FORESTMONITORCLASSACTION_H
