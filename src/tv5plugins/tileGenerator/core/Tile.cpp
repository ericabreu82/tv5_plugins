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

/*! \file terralib/qt/plugins/thirdParty/tileGenerator/core/Tile.cpp

    \brief This file contains structures and definitions to build a Tile box information.
*/

//TerraLib Includes
#include <terralib/raster/Utils.h>
#include "Tile.h"

//STL
#include <cmath>

te::qt::plugins::tv5plugins::Tile::Tile(double resolution, int tileSize):
  m_worldExtent(40075016.686),
  m_resolution(resolution),
  m_tileSize(tileSize)
{
  m_zoomLevel = bestZoomLevel(m_resolution);
}

te::qt::plugins::tv5plugins::Tile::Tile(int zoomLevel, int tileSize):
  m_worldExtent(40075016.686),
  m_zoomLevel(zoomLevel),
  m_tileSize(tileSize)
{
  m_resolution = resolution(m_zoomLevel);
}

te::qt::plugins::tv5plugins::Tile::~Tile()
{
}

double te::qt::plugins::tv5plugins::Tile::resolution(int zoomLevel)
{
  return m_worldExtent / (pow(2., zoomLevel) * m_tileSize);
}

int te::qt::plugins::tv5plugins::Tile::bestZoomLevel(double resolution)
{
  return (int)floor(log(m_worldExtent / (resolution * m_tileSize)));
}

void te::qt::plugins::tv5plugins::Tile::tileIndex(double xPos, double yPos, long& tileIndexX, long& tileIndexY)
{
  tileIndexX =  te::rst::Round((xPos / m_worldExtent) * pow(2., m_zoomLevel));
  tileIndexY =  te::rst::Round((yPos / m_worldExtent) * pow(2., m_zoomLevel));
}

te::gm::Envelope te::qt::plugins::tv5plugins::Tile::tileBox(long tileIndexX, long tileIndexY)
{
  te::gm::Envelope env;

  env.m_llx = tileIndexX * (m_worldExtent / pow(2., m_zoomLevel));
  env.m_lly = tileIndexY * (m_worldExtent / pow(2., m_zoomLevel));
  env.m_urx = env.m_llx + (m_worldExtent / pow(2., m_zoomLevel));
  env.m_ury = env.m_lly + (m_worldExtent / pow(2., m_zoomLevel));

  return env;
}

long te::qt::plugins::tv5plugins::Tile::tileMatrixDimension(int zoomLevel)
{
  return (long)pow(2., zoomLevel);
}

void te::qt::plugins::tv5plugins::Tile::tileMatrix(te::gm::Envelope env, long& tileIndexX1, long& tileIndexY1, long& tileIndexX2, long& tileIndexY2)
{
  tileIndex(env.getLowerLeftX(), env.getLowerLeftY(), tileIndexX1, tileIndexY1);
  tileIndex(env.getUpperRightX(), env.getUpperRightY(), tileIndexX2, tileIndexY2);
}

void te::qt::plugins::tv5plugins::Tile::OSMTile(long tileX, long tileY, long& OSMTileX, long& OSMTileY)
{
  OSMTileX = (tileMatrixDimension(m_zoomLevel) / 2.) + tileX;
  OSMTileY = (tileMatrixDimension(m_zoomLevel) / 2.) - (tileY + 1);
}

void te::qt::plugins::tv5plugins::Tile::TMSTile(long tileX, long tileY, long& TMSTileX, long& TMSTileY)
{
  TMSTileX = (tileMatrixDimension(m_zoomLevel) / 2.) + tileX;
  TMSTileY = (tileMatrixDimension(m_zoomLevel) / 2.) + tileY;
}
