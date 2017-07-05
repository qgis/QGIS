/***************************************************************************
    qgsgeocmsconnection.h
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOCMSCONNECTION_H
#define QGSGEOCMSCONNECTION_H

#include "qgis_core.h"
#include "qgsdatasourceuri.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"

#include <QString>
#include <QMultiMap>
#include <QNetworkReply>

struct LayerStruct
{
  QString uuid;
  QString name;
  QString typeName;
  QString title;
  QString wmsURL;
  QString wfsURL;
  QString xyzURL;
};


/** \ingroup core
 * Base class for all GeoCMS connection.
 * Parent/children hierarchy is not based on QObject.
*/
class CORE_EXPORT QgsGeoCMSConnection : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param geoCMSName GeoCMS name eg: geonode, qgiscloud, etc
     * \param connName connection name
     */
    QgsGeoCMSConnection( const QString &geoCMSName, const QString &connName );

    //! Destructor
    ~QgsGeoCMSConnection();

    //! Returns the list of connections for the specified GeoCMS
    static QStringList connectionList( const QString &geoCMSName );

    //! Deletes the connection for the specified GeoCMS with the specified name
    static void deleteConnection( const QString &geoCMSName, const QString &name );

    //! Retrieves the selected connection for the specified GeoCMS
    static QString selectedConnection( const QString &geoCMSName );

    //! Marks the specified connection for the specified GeoCMS as selected
    static void setSelectedConnection( const QString &geoCMSName, const QString &name );

    //! Return list of available layers
    virtual QList<LayerStruct> getLayers() = 0;
    virtual QList<LayerStruct> getLayers( QString serviceType ) = 0;

    //! Return list of available layers
    virtual QVariantList getMaps() = 0;

    //! Return available service urls
    //! Return WMS / WFS url for the layer / map / resource ID
    virtual QStringList serviceUrl( QString &resourceID, QString serviceType ) = 0;
    virtual QStringList serviceUrl( QString serviceType ) = 0;

    //! The GeoCMS name
    QString mGeoCMSName;

    //! The connection name
    QString mConnName;

    //! Getter for mUri
    QgsDataSourceUri uri();

    //! Property of mUri
    QgsDataSourceUri mUri;

    QMultiMap<QString, QString> mLayers;
    QMultiMap<QString, QString> mMaps;
    QString mData;
};


#endif //QGSGEOCMSCONNECTION_H
