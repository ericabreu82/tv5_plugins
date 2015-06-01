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
\file terraview5plugins/src/tv5plugins/forestMonitor/core/Parcel.h

\brief This class implements a parcel definition
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_PARCEL_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_PARCEL_H

// TerraLib
#include "../../Config.h"

// STL
#include <list>

namespace te
{
  namespace gm { class Polygon; }

  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {

        class Track;

        /*!
        \class Parcel

        \brief This class defines a parcel used in forest monitor operation

        \ingroup widgets
        */
        class Parcel
        {
        public:

          /** @name Initializer Methods
          *  Methods related to instantiation and destruction.
          */
          //@{

          /*!
          \brief It constructs a tree object  */
          Parcel();

          /*! \brief Destructor. */
          ~Parcel();

          //@}

          void addTrack(te::qt::plugins::tv5plugins::Track* track);

        private:

          int m_id;               //!< The tree identifier

          double m_area;          //!< The parcel area

          double m_angle;         //!< The angle of tree from this parcel

          std::list<te::qt::plugins::tv5plugins::Track*> m_trackList;

          te::gm::Polygon* m_poly; //!< Geometry to represent the parcel
        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_PARCEL_H
