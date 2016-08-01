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

/*! \file terralib/qt/plugins/thirdParty/forestMonitor/core/NDVI.cpp

    \brief This file contains structures and definitions NDVI operation.
*/

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/Exception.h>
#include <terralib/common/STLUtils.h>
#include <terralib/memory/ExpansibleRaster.h>
#include <terralib/raster/BandProperty.h>
#include <terralib/raster/Grid.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/RasterIterator.h>
#include "NDVI.h"

//STL Includes
#include <cassert>
#include <numeric>

std::auto_ptr<te::rst::Raster> te::qt::plugins::tv5plugins::GenerateNDVIRaster(te::rst::Raster* rasterNIR, int bandNIR, 
                                                                               te::rst::Raster* rasterVIS, int bandVIS, 
                                                                               double gain, double offset, bool normalize, 
                                                                               std::map<std::string, std::string> rInfo,
                                                                               std::string type, int srid,
                                                                               bool invert, bool rgbVIS)
{
  //check input parameters
  if(!rasterNIR || ! rasterVIS)
  {
    throw te::common::Exception("Invalid input rasters.");
  }

  if(rasterNIR->getNumberOfColumns() != rasterVIS->getNumberOfColumns() ||
     rasterNIR->getNumberOfRows() != rasterVIS->getNumberOfRows())
  {
    throw te::common::Exception("Incompatible rasters.");
  }

  std::string typeNDVI = type;

  if(normalize)
    typeNDVI = "MEM";

  //create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::DOUBLE_TYPE);
  bandProp->m_nblocksx = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksx;
  bandProp->m_nblocksy = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksy;
  bandProp->m_blkh = rasterNIR->getBand(bandNIR)->getProperty()->m_blkh;
  bandProp->m_blkw = rasterNIR->getBand(bandNIR)->getProperty()->m_blkw;
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(rasterNIR->getGrid()));
  grid->setSRID(srid);

  te::rst::Raster* rasterNDVI = 0;

  if(normalize)
  {
    rasterNDVI = new te::mem::ExpansibleRaster(10, grid, bandsProperties);
  }
  else
  {
    rasterNDVI = te::rst::RasterFactory::make(typeNDVI, grid, bandsProperties, rInfo);
  }

  /*if(invert)
  {
    rasterNIR = InvertRaster(rasterNIR, bandNIR);
    bandNIR = 0;
  }*/

  //start NDVI operation
  std::size_t nRows = rasterNDVI->getNumberOfRows();
  std::size_t nCols = rasterNDVI->getNumberOfColumns();

  double nirValue = 0.;
  double visValue = 0.;

  double minValue = std::numeric_limits<double>::max();
  double maxValue = -std::numeric_limits<double>::max();

  {
    te::common::TaskProgress task("Calculating NDVI.");
    task.setTotalSteps(nRows);

    for(std::size_t t = 0; t < nRows; ++t)
    {
      if(task.isActive() == false)
        throw te::common::Exception("Operation Canceled.");

      for(std::size_t q = 0; q < nCols; ++q)
      {
        try
        {
          rasterNIR->getValue(q, t, nirValue, bandNIR);

          if (invert)
            nirValue = (nirValue * (-1.)) + 255.;

          if (rgbVIS)
          {
            std::vector<double> visValueVec;

            rasterVIS->getValues(q, t, visValueVec);

            for (std::size_t a = 0; a < visValueVec.size(); ++a)
            {
              visValue += visValueVec[a];
            }

            if (visValue != 0.)
              visValue = visValue / (double)rasterVIS->getNumberOfBands();
          }
          else
          {
            rasterVIS->getValue(q, t, visValue, bandVIS);
          }
          
          double value = 0.;

          if(nirValue + visValue != 0.)
          {
            value = (gain *((nirValue - visValue) / (nirValue + visValue))) + offset;
          }

          rasterNDVI->setValue(q, t, value, 0);

          if(value > maxValue)
          {
            maxValue = value;
          }

          if(value < minValue)
          {
            minValue = value;
          }
        }
        catch (...)
        {
          continue;
        }
      }

      task.pulse();
    }
  }

  //if (invert)
  //{
  //  delete rasterNIR;
  //}

  std::auto_ptr<te::rst::Raster> rasterOut;

  if(normalize)
  {
    rasterOut = NormalizeRaster(rasterNDVI, minValue, maxValue, 0., 255., rInfo, type);

    delete rasterNDVI;
  }
  else
  {
    rasterOut.reset(rasterNDVI);
  }

  rasterOut->getGrid()->setSRID(srid);

  return rasterOut;
}


std::auto_ptr<te::rst::Raster>  te::qt::plugins::tv5plugins::ExportRasterBand(te::rst::Raster* raster, int band,
                                                                              double gain, double offset, bool normalize,
                                                                              std::map<std::string, std::string> rInfo,
                                                                              std::string type, int srid)
{
  double minValue = std::numeric_limits<double>::max();
  double maxValue = -std::numeric_limits<double>::max();

  te::rst::Raster* exportRaster = 0;

  //check input parameters
  if (!raster)
  {
    throw te::common::Exception("Invalid input rasters.");
  }

  std::string typeRaster = type;

  if (normalize)
    typeRaster = "MEM";

  //create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, raster->getBand(band)->getProperty()->getType());
  bandProp->m_nblocksx = raster->getBand(band)->getProperty()->m_nblocksx;
  bandProp->m_nblocksy = raster->getBand(band)->getProperty()->m_nblocksy;
  bandProp->m_blkh = raster->getBand(band)->getProperty()->m_blkh;
  bandProp->m_blkw = raster->getBand(band)->getProperty()->m_blkw;
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(raster->getGrid()));
  grid->setSRID(srid);

  if (normalize)
  {
    exportRaster = new te::mem::ExpansibleRaster(10, grid, bandsProperties);
  }
  else
  {
    exportRaster = te::rst::RasterFactory::make(typeRaster, grid, bandsProperties, rInfo);
  }

  //start export operation
  std::size_t nRows = raster->getNumberOfRows();
  std::size_t nCols = raster->getNumberOfColumns();

  {
    te::common::TaskProgress task("Exporting Band.");
    task.setTotalSteps(nRows);

    for (std::size_t t = 0; t < nRows; ++t)
    {
      if (task.isActive() == false)
        throw te::common::Exception("Operation Canceled.");

      for (std::size_t q = 0; q < nCols; ++q)
      {
        try
        {
          double value = 0.;

          raster->getValue(q, t, value, band);

          value = (gain * value) + offset;

          exportRaster->setValue(q, t, value, 0);

          if (value > maxValue)
          {
            maxValue = value;
          }

          if (value < minValue)
          {
            minValue = value;
          }
        }
        catch (...)
        {
          continue;
        }
      }

      task.pulse();
    }
  }

  std::auto_ptr<te::rst::Raster> rasterOut;

  if (normalize)
  {
    rasterOut = NormalizeRaster(exportRaster, minValue, maxValue, 0., 255., rInfo, type);

    delete exportRaster;
  }
  else
  {
    rasterOut.reset(exportRaster);
  }

  rasterOut->getGrid()->setSRID(srid);

  return rasterOut;
}

te::rst::Raster* te::qt::plugins::tv5plugins::InvertRaster(te::rst::Raster* rasterNIR, int bandNIR)
{
  //create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::UCHAR_TYPE);
  bandProp->m_nblocksx = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksx;
  bandProp->m_nblocksy = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksy;
  bandProp->m_blkh = rasterNIR->getBand(bandNIR)->getProperty()->m_blkh;
  bandProp->m_blkw = rasterNIR->getBand(bandNIR)->getProperty()->m_blkw;
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(rasterNIR->getGrid()));

  te::rst::Raster* rasterInverted = new te::mem::ExpansibleRaster(10, grid, bandsProperties);

  te::common::TaskProgress task("Invert Raster NIR.");
  task.setTotalSteps(rasterNIR->getNumberOfRows());

  double value;

  for (std::size_t t = 0; t < rasterNIR->getNumberOfRows(); ++t)
  {
    for (std::size_t q = 0; q < rasterNIR->getNumberOfColumns(); ++q)
    {
      try
      {
        double value;

        rasterNIR->getValue(q, t, value, bandNIR);

        double invertedValue = (value * -1) + 255.;

        rasterInverted->setValue(q, t, invertedValue, 0);
      }
      catch (...)
      {
        continue;
      }
    }

    task.pulse();
  }

  return rasterInverted;
}

std::auto_ptr<te::rst::Raster> te::qt::plugins::tv5plugins::NormalizeRaster(te::rst::Raster* inraster, double min, double max, double nmin, double nmax, 
                                                                            std::map<std::string, std::string> rInfo, std::string type)
{
//create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::UCHAR_TYPE);
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(inraster->getGrid()));

  te::rst::Raster* rasterNormalized = te::rst::RasterFactory::make(type, grid, bandsProperties, rInfo);

  //start Normalize operation
  std::size_t nRows = inraster->getNumberOfRows();
  std::size_t nCols = inraster->getNumberOfColumns();

  te::common::TaskProgress task("Normalize NDVI.");
  task.setTotalSteps(nRows);

  double gain = (double)(nmax-nmin)/(max-min);
  double offset = -1*gain*min+nmin;

  double value;

  for(std::size_t t = 0; t < nRows; ++t)
  {
    if(task.isActive() == false)
      throw te::common::Exception("Operation Canceled.");

    for(std::size_t q = 0; q < nCols; ++q)
    {
      try
      {
        inraster->getValue(q, t, value, 0);

        double normalizeValue = (value * gain + offset);

        rasterNormalized->setValue(q, t, normalizeValue, 0);
      }
      catch (...)
      {
        continue;
      }
    }

    task.pulse();
  }

  std::auto_ptr<te::rst::Raster> rOut(rasterNormalized);

  return rOut;
}
