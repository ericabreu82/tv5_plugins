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

/*! \file terralib/qt/plugins/thirdParty/tileGenerator/core/Tile.h

    \brief This file contains structures and definitions to build a Tile box information.

    \note Classe a ser generalizada no futuro para qualquer proojeção e area de interesse
          O codigo a seguir funciona corretamente para dados em projeção mercator e 
          elipsoide WMS esferico (Terralib VirualEarthProjection ou coisa parecida)
          O sistema de numeração de tiles pode variar:  Google:(0,0)=upper-left ou 
          TMS:(0,0)=lower-left ou Outros:(0,0)=(w0,n0).
          Os metodos da classe geram o numero do tile sendo (0,0)=(w0,n0).
          Neste caso se for recuperar dados do Google ou TMS deve ajustar as origens
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILE_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILE_H

//TerraLib Includes
#include <terralib/geometry/Envelope.h>
#include "../../Config.h"

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        class Tile
        {
          public:
    
            /**
              \brief Constructor.
              \param resolution
              \param tileSize
            */
            Tile(double resolution, int tileSize);

            /**
              \brief Constructor.
              \param zoomLevel
              \param tileSize
            */
            Tile(int zoomLevel, int tileSize);

            /**
              \brief Destructor
            */
            ~Tile();

          public:

            /** 
              \brief Calculates tile resolution given the zoom level.
            */
            double resolution(int zoomLevel);

            /** 
              \brief The zoom level which generates the minimum resolution, ajusted to a zoom level,  greater than a given resolution.
            */
            int bestZoomLevel(double resolution);

            /** 
              /brief Calculates the tile index given a planar coordinates of a location.
             */
            void tileIndex(double xPos, double yPos, long& tileIndexX, long& tileIndexY);

            /**
              /brief Calculates the tile box.
            */
            te::gm::Envelope tileBox(long tileIndexX, long tileIndexY);

            /** 
              /brief Calculates the tile matrix dimension for world extent.
            */
            long tileMatrixDimension(int zoomLevel);

            /** 
              /brief Calculates the range of tiles for a given box in planar coordinates.
            */
            void tileMatrix(te::gm::Envelope env, long& tileIndexX1, long& tileIndexY1, long& tileIndexX2, long& tileIndexY2);

            /** 
              /brief Calculates the Google and Open Streetmap tile given normalized tile (0,0=n0,w0).
            */
            void OSMTile(long tileX, long tileY, long& OSMTileX, long& OSMTileY);

            /** 
              /brief Calculates the TMS tile  given normalized tile (0,0=n0,w0).
            */
            void TMSTile(long tileX, long tileY, long& TMSTileX, long& TMSTileY);

          protected:
            double m_worldExtent;
            double m_resolution;
            int m_tileSize;
            int m_zoomLevel;
        };
      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_TILE_H
