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

/*! \file terralib/qt/plugins/thirdParty/tileGenerator/core/TileGeneratorService.h

    \brief This file implements the service to create tiles over a set of layers.

*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILEGENERATORSERVICE_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILEGENERATORSERVICE_H

//TerraLib Includes
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/geometry/Envelope.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include "../../Config.h"

//STL Includes
#include <string>

//QT Includes
#include <QPixmap>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        //forward declarations
        class Tile;

        class TileGeneratorService
        {
          public:

            TileGeneratorService();

            ~TileGeneratorService();

          public:

            void setInputParameters(std::list<te::map::AbstractLayerPtr> layers, te::gm::Envelope env, int srid, int zoomLevelMin, int zoomLevelMax, int tileSize, std::string path, std::string format);

            void runValidation(bool createMissingTiles);

            void runService(bool isRaster);

          protected:

            void checkParameters();

            void buildDisplay();

            QPixmap* drawTile(te::gm::Envelope env);

            QPixmap drawTile(te::gm::Envelope env, int level);

            void validateTile(FILE* file, Tile* tile, int level, int tileIdxX, int tileIdxY, bool& isValid);

            void savePixmap(Tile* tile, QPixmap* pix, int level, int tileIdxX, int tileIdxY);

            std::string getOSMTileFilePath(Tile* tile, int level, int tileIdxX, int tileIdxY, bool save);

          protected:
            te::qt::widgets::MapDisplay* m_display;

            std::list<te::map::AbstractLayerPtr> m_layers;

            te::gm::Envelope m_env;

            int m_srid;

            int m_zoomLevelMin;
            int m_zoomLevelMax;

            int m_tileSize;

            std::string m_path;
            std::string m_format;
            std::string m_logFile;
        };
      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILEGENERATORSERVICE_H
