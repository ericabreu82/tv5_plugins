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

/*! \file terralib/qt/plugins/thirdParty/Proximity/core/ProximityService.h

    \brief This file implements the service to photo index information.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PROXIMITYSERVICE_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PROXIMITYSERVICE_H

//TerraLib Includes
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include "../../Config.h"

#include <memory>

namespace te
{
  //forward declarations
  namespace da  { class DataSetType; }
  namespace mem { class DataSet; }

  namespace qt
  {
    namespace plugins
    {
      namespace tv5plugins
      {
        //forward declarations
        class Proximity;

        class ProximityService
        {
          public:

            ProximityService();

            ~ProximityService();

          public:

            void setInputParameters(std::auto_ptr<te::da::DataSet> inputData, std::auto_ptr<te::da::DataSetType> inputDataType, 
                                    std::auto_ptr<te::da::DataSet> outputData, std::auto_ptr<te::da::DataSetType> outputDataType, 
                                    std::string m_attrName, double m_distBuffer);

            void runService();

            std::vector<std::string> getResult();

          protected:

            void checkParameters();

          protected:

            std::auto_ptr<te::da::DataSet> m_inputData;
            std::auto_ptr<te::da::DataSetType> m_inputDataType;

            std::auto_ptr<te::da::DataSet> m_outputData;
            std::auto_ptr<te::da::DataSetType> m_outputDataType;
            std::string m_attrName;

            double m_distBuffer;

            std::vector<std::string> m_result;
        };
      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_ProximitySERVICE_H
