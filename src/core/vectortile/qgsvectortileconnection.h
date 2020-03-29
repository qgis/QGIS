/***************************************************************************
    qgsvectortileconnection.h
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILECONNECTION_H
#define QGSVECTORTILECONNECTION_H

#include "qgis_core.h"

///@cond PRIVATE
#define SIP_NO_FILE

#include <QStringList>

struct QgsVectorTileConnection
{
  QString name;
  QString url;
  int zMin = -1;
  int zMax = -1;

  QString encodedUri() const;
};

//! Utility class for handling list of connections to vector tile layers
class CORE_EXPORT QgsVectorTileConnectionUtils
{
  public:
    //! Returns list of existing connections, unless the hidden ones
    static QStringList connectionList();

    //! Returns connection details
    static QgsVectorTileConnection connection( const QString &name );

    //! Removes a connection from the list
    static void deleteConnection( const QString &name );

    //! Adds a new connection to the list
    static void addConnection( const QgsVectorTileConnection &conn );
};

///@endcond

#endif // QGSVECTORTILECONNECTION_H
