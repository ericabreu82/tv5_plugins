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
\file terraview5plugins/src/tv5plugins/forestMonitor/core/Parcel.cpp

\brief This class implements a parcel definition
*/

// TerraLib
#include <terralib/common/STLUtils.h>

#include "Parcel.h"

te::qt::plugins::tv5plugins::Parcel::Parcel() :m_poly(0)
{
}

te::qt::plugins::tv5plugins::Parcel::~Parcel()
{
  delete m_poly;

  te::common::FreeContents(m_trackList);

  m_trackList.clear();
}

void te::qt::plugins::tv5plugins::Parcel::addTrack(te::qt::plugins::tv5plugins::Track* track)
{
  m_trackList.push_back(track);
}

