/***************************************************************************
  salayer.h
  Definition of vector layer backed by a SQL Anywhere database
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SALAYER_H
#define SALAYER_H
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgspluginlayer.h>
#include <qgslogger.h>

#include "sqlanyconnection.h"

/*! \class SaLayer
 * \brief Vector layer backed by a SQL Anywhere database.
 */
class SaLayer : public QgsVectorLayer
{
    Q_OBJECT

  public:
    //! Constructor
    SaLayer( QString path = QString::null
                            , QString baseName = QString::null
                                                 , bool loadDefaultStyleFlag = true )
        : QgsVectorLayer( path, baseName, "sqlanywhere", loadDefaultStyleFlag )
    {
      if ( isValid() )
      {
        // The parent QgsMapLayer initialized mDataSource = path.
        // Reset this to the value mDataProvider.dataSourceUri()
        // so that any modifications to the URI made by the
        // data provider make it back into the layer definition.
        mDataSource = dataProvider()->dataSourceUri();
        SaDebugMsg( "Modified layer source: " + mDataSource );
      }
    }

    //! Destructor
    ~SaLayer()
    {
    }
};

#endif //  SALAYER_H
