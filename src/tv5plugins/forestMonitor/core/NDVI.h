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

/*! \file terralib/qt/plugins/thirdParty/forestMonitor/core/NDVI.h

    \brief This file contains structures and definitions NDVI operation.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_NDVI_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_NDVI_H

// TerraLib
#include "../../Config.h"

//STL Includes
#include <map>
#include <memory>

namespace te
{
  namespace rst { class Raster; }

  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {

        std::auto_ptr<te::rst::Raster> GenerateNDVIRaster(te::rst::Raster* rasterNIR, int bandNIR, 
                                                          te::rst::Raster* rasterVIS, int bandVIS, 
                                                          double gain, double offset, bool normalize, 
                                                          std::map<std::string, std::string> rInfo,
                                                          std::string type, int srid,
                                                          bool invert, bool rgbVIS);

        std::auto_ptr<te::rst::Raster> ExportRasterBand(te::rst::Raster* raster, int band,
                                                        double gain, double offset, bool normalize,
                                                        std::map<std::string, std::string> rInfo,
                                                        std::string type, int srid);

        te::rst::Raster* InvertRaster(te::rst::Raster* rasterNIR, int bandNIR);

        std::auto_ptr<te::rst::Raster> NormalizeRaster(te::rst::Raster* inraster, double min, double max, double nmin, double nmax, 
                                                       std::map<std::string, std::string> rInfo, std::string type);

      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_NDVI_H
