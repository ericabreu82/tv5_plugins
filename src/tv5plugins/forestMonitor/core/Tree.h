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
\file terraview5plugins/src/tv5plugins/forestMonitor/core/Tree.h

\brief This class implements a tree definition
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TREE_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TREE_H

// TerraLib
#include "../../Config.h"


namespace te
{
  namespace gm { class Point; }

  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {

        /*!
        \class Tree

        \brief This class defines a tree used in forest monitor operation

        \ingroup widgets
        */
        class Tree
        {
          public:

          /** @name Initializer Methods
          *  Methods related to instantiation and destruction.
          */
          //@{

          /*!
          \brief It constructs a tree object  */
          Tree();

          /*! \brief Destructor. */
          ~Tree();

          //@}

        private:

          int m_id;               //!< The tree identifier

          int m_parcelParentId;   //!< The parcel parent identifier

          double m_area;          //!< The tree area

          int m_status;           //!< The tree status (LIVE, DEAD, INTRUDER)

          te::gm::Point* m_point; //!< Geometry to represent the tree
        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_TREE_H
