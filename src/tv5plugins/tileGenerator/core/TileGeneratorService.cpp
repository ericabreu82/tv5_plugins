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

#define GOOGLE_SRID 3857

//TerraLib Includes
#include <terralib/common/StringUtils.h>
#include <terralib/raster/Utils.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include "TileGeneratorService.h"
#include "Tile.h"

//STL Includes
#include <cassert>
#include <exception>
#include <iosfwd>
#include <stdio.h>

//Qt Includes
#include <QDir>
#include <QPainter>

te::qt::plugins::tv5plugins::TileGeneratorService::TileGeneratorService():
  m_display(0),
  m_zoomLevelMin(-1),
  m_zoomLevelMax(-1),
  m_tileSize(0),
  m_path(""),
  m_format(""),
  m_logFile("")
{
}

te::qt::plugins::tv5plugins::TileGeneratorService::~TileGeneratorService()
{
  delete m_display;
}

void te::qt::plugins::tv5plugins::TileGeneratorService::setInputParameters(std::list<te::map::AbstractLayerPtr> layers, te::gm::Envelope env, int srid, int zoomLevelMin, int zoomLevelMax, int tileSize, std::string path, std::string format)
{
  m_layers = layers;

  m_env = env;
  m_srid = srid;
  m_zoomLevelMin = zoomLevelMin;
  m_zoomLevelMax = zoomLevelMax;
  m_tileSize = tileSize;
  m_path = path;
  m_format = format;

  m_env.transform(m_srid, GOOGLE_SRID);

  buildDisplay();
}

void te::qt::plugins::tv5plugins::TileGeneratorService::runValidation(bool createMissingTiles)
{
  //check input parameters
  checkParameters();

  m_logFile = m_path + "/ValidationLog.txt";

  //create file
  FILE* fp = fopen(m_logFile.c_str(), "w");

  if(!fp)
    throw std::exception("Error creating log file.");

  fprintf(fp, "\n\n\t\tTiles Validation LOG\n\n");

  fprintf(fp, "Missing Tiles:\n");

  //generate tiles
  for(int i = m_zoomLevelMax; i >= m_zoomLevelMin; --i)
  {
    Tile* tile = new Tile(i, m_tileSize);

    long tIdxX1, tIdxY1, tIdxX2, tIdxY2;

    tile->tileMatrix(m_env, tIdxX1, tIdxY1, tIdxX2, tIdxY2);
    
    for(int k = tIdxY1 - 1; k <= tIdxY2; ++k)
    {
      for(int j = tIdxX1 - 1; j <= tIdxX2; ++j)
      {
        te::gm::Envelope env = tile->tileBox(j, k);

        if(env.isValid())
        {
          try
          {
            bool isValid;

            validateTile(fp, tile, i, j, k, isValid);

            if(!isValid && createMissingTiles)
            {
              QPixmap* pix = drawTile(env);

              if(pix)
                savePixmap(tile, pix, i, j, k);
            }
          }
          catch(...)
          {
            fprintf(fp, "\n\t Unexpected error. Level: %d Y: %d X: %d \n", i, k, j);
            continue;
          }
        }
        else
        {
          fprintf(fp, "\n\t Invalid Box. Level: %d Y: %d X: %d \n", i, k, j);
        }
      }
    }

    delete tile;
  }
}

void te::qt::plugins::tv5plugins::TileGeneratorService::runService(bool isRaster)
{
  //check input parameters
  checkParameters();

  //generate tiles
  for(int i = m_zoomLevelMax; i >= m_zoomLevelMin; --i)
  {
    Tile* tile = new Tile(i, m_tileSize);

    long tIdxX1, tIdxY1, tIdxX2, tIdxY2;

    tile->tileMatrix(m_env, tIdxX1, tIdxY1, tIdxX2, tIdxY2);
    
    for(int k = tIdxY1 - 1; k <= tIdxY2; ++k)
    {
      for(int j = tIdxX1 - 1; j <= tIdxX2; ++j)
      {
        te::gm::Envelope env = tile->tileBox(j, k);

        if(env.isValid())
        {
          try
          {
            if(isRaster)
            {
              if(i == m_zoomLevelMax)// use original images
              {
                QPixmap* pix = drawTile(env);

                if(pix)
                  savePixmap(tile, pix, i, j, k);
              }
              else //use generated tiles
              {
                QPixmap pix = drawTile(env, i + 1);

                if(!pix.isNull())
                  savePixmap(tile, &pix, i, j, k);
              }
            }
            else
            {
              QPixmap* pix = drawTile(env);

                if(pix)
                  savePixmap(tile, pix, i, j, k);
            }
          }
          catch(...)
          {
            continue;
          }
        }
      }
    }

    delete tile;
  }
}

void te::qt::plugins::tv5plugins::TileGeneratorService::checkParameters()
{
  if(m_layers.empty())
    throw std::exception("Select a list of layers firts.");

  if(!m_env.isValid())
    throw std::exception("Box defined is not valid.");

  if(m_zoomLevelMin < 0 || m_zoomLevelMax < 0 || m_zoomLevelMax < m_zoomLevelMin)
    throw std::exception("Invalid range for zoom levels.");

  if(m_tileSize <= 0)
    throw std::exception("Invalid value for tile size.");

  if(m_path.empty())
    throw std::exception("Invalid path value.");

  QDir dir(m_path.c_str());
  if(!dir.exists())
    throw std::exception("Invalid path value.");

  if(m_format.empty())
    throw std::exception("Invalid format value.");
}

void te::qt::plugins::tv5plugins::TileGeneratorService::buildDisplay()
{
  m_display = new te::qt::widgets::MapDisplay(QSize(m_tileSize, m_tileSize), false);

  m_display->setLayerList(m_layers);

  m_display->setSRID(GOOGLE_SRID, false);
}

QPixmap* te::qt::plugins::tv5plugins::TileGeneratorService::drawTile(te::gm::Envelope env)
{
  m_display->setExtent(env, true);

  return m_display->getDisplayPixmap();
}

QPixmap te::qt::plugins::tv5plugins::TileGeneratorService::drawTile(te::gm::Envelope env, int level)
{
  Tile* tileGroup = new Tile(level, m_tileSize);

  long tIdxX1Group, tIdxY1Group, tIdxX2Group, tIdxY2Group;

  tileGroup->tileMatrix(env, tIdxX1Group, tIdxY1Group, tIdxX2Group, tIdxY2Group);

  int width = m_tileSize * (tIdxX2Group - tIdxX1Group);
  int height = m_tileSize * (tIdxY2Group - tIdxY1Group);

  QPixmap pixGroup(width, height);
  pixGroup.fill(Qt::white);

  QPainter painter(&pixGroup);

  int countY = tIdxY2Group - tIdxY1Group - 1;
  for(int p = tIdxY1Group; p <= tIdxY2Group - 1; ++p)
  {
    int countX = 0;
    for(int q = tIdxX1Group; q <= tIdxX2Group; ++q)
    {
      std::string tileFileName = getOSMTileFilePath(tileGroup, level, q, p, false);

      QPixmap pixGroupItem(tileFileName.c_str());

      if(!pixGroupItem.isNull())
      {
        painter.drawPixmap(countX * m_tileSize, countY * m_tileSize, m_tileSize, m_tileSize, pixGroupItem);
      }

      ++countX;
    }

    --countY;
  }

  painter.end();

  delete tileGroup;

  //smoth scale
  return pixGroup.scaled(m_tileSize, m_tileSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void te::qt::plugins::tv5plugins::TileGeneratorService::validateTile(FILE* file, Tile* tile, int level, int tileIdxX, int tileIdxY, bool& isValid)
{
//change tile index to google system
  long tileIdxXOSM;
  long tileIdxYOSM;

  tile->OSMTile(tileIdxX, tileIdxY, tileIdxXOSM, tileIdxYOSM);

  isValid = false;

  //file info
  //std::string pathOSM   = m_path + "/MapquestOSM";
  std::string pathLevel   = m_path + "/" + te::common::Convert2String(level);
  std::string pathColumn  = pathLevel + "/" + te::common::Convert2String(te::rst::Round(tileIdxXOSM));
  std::string fileName    = pathColumn + "/" + te::common::Convert2String(te::rst::Round(tileIdxYOSM))+ "." + te::common::Convert2LCase(m_format);

  //check MapquestOSM directory
  //QDir dirOSM(pathOSM.c_str());
  //if(dirOSM.exists())
  //{
    //check dir level
    QDir dirLevel(pathLevel.c_str());
    if(dirLevel.exists())
    {
      //check dir column
      QDir dirColumn(pathColumn.c_str());
      if(dirColumn.exists())
      {
        //check file
        QFile file(fileName.c_str());

        isValid = file.exists();
      }
    }
  //}

  //check validation
  if(!isValid)
  {
    fprintf(file, "\n\t Tile File: %s \n", fileName.c_str());
  }
}

void te::qt::plugins::tv5plugins::TileGeneratorService::savePixmap(Tile* tile, QPixmap* pix, int level, int tileIdxX, int tileIdxY)
{
  //create file
  std::string fileName = getOSMTileFilePath(tile, level, tileIdxX, tileIdxY, true);

  pix->save(fileName.c_str(), m_format.c_str());
}

std::string te::qt::plugins::tv5plugins::TileGeneratorService::getOSMTileFilePath(Tile* tile, int level, int tileIdxX, int tileIdxY, bool save)
{
//change tile index to google system
  long tileIdxXOSM;
  long tileIdxYOSM;

  tile->OSMTile(tileIdxX, tileIdxY, tileIdxXOSM, tileIdxYOSM);

  // MapquestOSM directory
  //std::string pathOSM = m_path + "/MapquestOSM";
  //QDir dirOSM(pathOSM.c_str());
  //if(!dirOSM.exists() && save)
  //  dirOSM.mkdir(pathOSM.c_str());

  //create dir level
  std::string pathLevel = m_path + "/" + te::common::Convert2String(level);
  QDir dirLevel(pathLevel.c_str());
  if(!dirLevel.exists() && save)
    dirLevel.mkdir(pathLevel.c_str());

  //create dir column
  std::string pathColumn = pathLevel + "/" + te::common::Convert2String(te::rst::Round(tileIdxXOSM));
  QDir dirColumn(pathColumn.c_str());
  if(!dirColumn.exists() && save)
    dirColumn.mkdir(pathColumn.c_str());

  //create file
  std::string fileName = pathColumn + "/" + te::common::Convert2String(te::rst::Round(tileIdxYOSM))+ "." + te::common::Convert2LCase(m_format);

  return fileName;
}

/*
 Tile* tileGroup = new Tile(level, m_tileSize);

  te::gm::Coord2D ll = env.getLowerLeft();

  double stepW = env.getWidth() / 4.;
  double stepH = env.getHeight() / 4.;

  // -------------
  // |  1  |  2  |
  // -------------
  // |  3  |  4  |
  // -------------

  te::gm::Coord2D tile1(ll.getX() + stepW, ll.getY() + (stepH * 2.5));
  te::gm::Coord2D tile2(ll.getX() + stepW + stepW + stepW, ll.getY() + (stepH * 2.5));
  te::gm::Coord2D tile3(ll.getX() + stepW, ll.getY() + stepH);
  te::gm::Coord2D tile4(ll.getX() + stepW + stepW + stepW, ll.getY() + stepH);

  long tIdxX, tIdxY;

  int width = m_tileSize * 2;
  int height = m_tileSize * 2;

  QPixmap pixGroup(width, height);
  pixGroup.fill(Qt::white);

  QPainter painter(&pixGroup);

  //tile 3
  tileGroup->tileIndex(tile3.getX(), tile3.getY(), tIdxX, tIdxY);

  std::string tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX, tIdxY);

  QPixmap pixItem3(tileFileName.c_str());

  if(!pixItem3.isNull())
  {
    painter.drawPixmap(0 * m_tileSize, 1 * m_tileSize, m_tileSize, m_tileSize, pixItem3);
  }

  //tile 1
  tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX, tIdxY + 1);

  QPixmap pixItem1(tileFileName.c_str());

  if(!pixItem1.isNull())
  {
    painter.drawPixmap(0 * m_tileSize, 0 * m_tileSize, m_tileSize, m_tileSize, pixItem1);
  }

  //tile 2
  tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX + 1, tIdxY + 1);

  QPixmap pixItem2(tileFileName.c_str());

  if(!pixItem2.isNull())
  {
    painter.drawPixmap(1 * m_tileSize, 0 * m_tileSize, m_tileSize, m_tileSize, pixItem2);
  }

  //tile 4
  tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX + 1, tIdxY);

  QPixmap pixItem4(tileFileName.c_str());

  if(!pixItem4.isNull())
  {
    painter.drawPixmap(1 * m_tileSize, 1 * m_tileSize, m_tileSize, m_tileSize, pixItem4);
  }

  painter.end();

  delete tileGroup;

  //smoth scale
  return pixGroup.scaled(m_tileSize, m_tileSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  */