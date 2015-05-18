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

/*! \file terralib/qt/plugins/thirdParty/PhotoIndex/core/PhotoIndexService.h

    \brief This file implements the service to photo index information.
*/

#ifndef __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PHOTOINDEXSERVICE_H
#define __TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PHOTOINDEXSERVICE_H

//TerraLib Includes
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

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
        class PhotoIndex;

        class PhotoIndexService
        {
          public:

            PhotoIndexService();

            ~PhotoIndexService();

          public:

            void setInputParameters(std::string path);

            void setOutputParameters(te::da::DataSourcePtr ds, std::string outputDataSetName);

            void runService();

          protected:

            void checkParameters();

            /*! Function used to create the output dataset type */
            std::auto_ptr<te::da::DataSetType> createDataSetType(int srid);

            /*! Function used to save the output dataset */
            void saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType);

            /*! Function used to get the SRID information */
            int getSRID(te::da::DataSourcePtr ds);

          protected:

            std::string m_path;                               //!< Path from input rasters

            te::da::DataSourcePtr m_ds;                       //!< Pointer to the output datasource.

            std::string m_outputDataSetName;                  //!< Attribute that defines the output dataset name
        };
      } // end namespace thirdParty
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__TE_QT_PLUGINS_THIRDPARTY_INTERNAL_PhotoIndexSERVICE_H
