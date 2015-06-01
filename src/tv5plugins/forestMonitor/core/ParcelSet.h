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
\file terraview5plugins/src/tv5plugins/forestMonitor/core/ParcelSet.h

\brief This class implements a parcelSet definition
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_PARCELSET_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_PARCELSET_H

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <map>
#include <set>

namespace te
{
  namespace rst { class Raster; }

  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        class Parcel;

        /*!
        \class ParcelSet

        \brief This class defines a parcel set used in forest monitor operation

        \ingroup widgets
        */
        class ParcelSet
        {
        public:

          /** @name Initializer Methods
          *  Methods related to instantiation and destruction.
          */
          //@{

          /*!
          \brief It constructs a tree object  */
          ParcelSet(te::map::AbstractLayerPtr parcels, te::map::AbstractLayerPtr angles);

          /*! \brief Destructor. */
          ~ParcelSet();

          //@}

          void classify(te::rst::Raster* ndviRaster, int ndviBand, double threshold, int nErosion, int nDilation, bool exportRaster, std::string rasterPath = "");

          void export(std::string type, std::map<std::string, std::string> connInfo);

        protected:

          void getDataSetTypeInfo(te::da::DataSetType* dsType, int& idIdx, int& geomIdx);


        private:

          std::set<te::qt::plugins::tv5plugins::Parcel*> m_parcelSet;
          
          te::map::AbstractLayerPtr m_parcelLayer;

          te::map::AbstractLayerPtr m_angleLayer;

        };

      } // end namespace tv5plugins
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif  // __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TOOL_PARCELSET_H
