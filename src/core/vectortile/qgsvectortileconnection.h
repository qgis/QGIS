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

#include "qgsabstractproviderconnection.h"

class CORE_EXPORT QgsVectorTileProviderConnection : public QgsAbstractProviderConnection
{

  public:
    QgsVectorTileProviderConnection( const QString &name );
    QgsVectorTileProviderConnection( const QString &uri, const QVariantMap &configuration );

    virtual void store( const QString &name ) const override;
    virtual void remove( const QString &name ) const override;

    /**
     * Vector tile service type.
     *
     * \since QGIS 3.16
     */
    enum ServiceType
    {
      Generic, //!< Generic (XYZ) connection
      ArcgisVectorTileService, //!< ArcGIS VectorTileServer connection
    };

    //! Represents decoded data of a connection
    struct Data
    {
      QString url;
      int zMin = -1;
      int zMax = -1;

      ServiceType serviceType = Generic;

      //! Authentication configuration id
      QString authCfg;
      //! HTTP Basic username
      QString username;
      //! HTTP Basic password
      QString password;
      //! HTTP headers
      QgsHttpHeaders httpHeaders;

      //! Optional style URL (will override any default styles)
      QString styleUrl;

    };

    //! Returns connection data encoded as a string
    static QString encodedUri( const Data &conn );
    //! Decodes connection string to a data structure
    static Data decodedUri( const QString &uri );

    //! Returns connection data encoded as a string containing URI for QgsVectorTileLayer
    static QString encodedLayerUri( const Data &conn );

    //! Returns list of existing connections, unless the hidden ones
    static QStringList connectionList();
    //! Returns connection details
    static Data connection( const QString &name );
    //! Removes a connection from the list
    static void deleteConnection( const QString &name );
    //! Adds a new connection to the list
    static void addConnection( const QString &name, Data conn );
    //! Returns last used connection
    static QString selectedConnection();
    //! Saves name of the last used connection
    static void setSelectedConnection( const QString &connName );
};

///@endcond

#endif // QGSVECTORTILECONNECTION_H
