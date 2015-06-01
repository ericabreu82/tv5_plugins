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
\file terraview5plugins/src/tv5plugins/forestMonitor/core/Track.cpp

\brief This class implements a track definition, a set of trees
*/

// TerraLib
#include <terralib/common/STLUtils.h>

#include "Track.h"

te::qt::plugins::tv5plugins::Track::Track()
{
}

te::qt::plugins::tv5plugins::Track::~Track()
{
  m_treeList.clear();
}

void te::qt::plugins::tv5plugins::Track::pushFront(te::qt::plugins::tv5plugins::Tree* tree)
{
  m_treeList.push_front(tree);
}

void te::qt::plugins::tv5plugins::Track::pushBack(te::qt::plugins::tv5plugins::Tree* tree)
{
  m_treeList.push_back(tree);
}
