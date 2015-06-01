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
\file terraview5plugins/src/tv5plugins/forestMonitor/core/Track.h

\brief This class implements a track definition, a set of trees
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACK_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACK_H

// TerraLib
#include "../../Config.h"

// STL
#include <list>


namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {

        class Tree;

        /*!
        \class Track

        \brief This class defines a track, that is a set of trees, used in forest monitor operation

        \ingroup widgets
        */
        class Track
        {
        public:

          /** @name Initializer Methods
          *  Methods related to instantiation and destruction.
          */
          //@{

          /*!
          \brief It constructs a tree object  */
          Track();

          /*! \brief Destructor. */
          ~Track();

          //@}

        public:

          void pushFront(te::qt::plugins::tv5plugins::Tree* tree);

          void pushBack(te::qt::plugins::tv5plugins::Tree* tree);

        private:

          std::list<te::qt::plugins::tv5plugins::Tree*> m_treeSet;

          double m_avgDistance;
        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TRACK_H
