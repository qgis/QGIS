/***************************************************************************
    qgsgeonodeconnection.h
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEONODECONNECTION_H
#define QGSGEONODECONNECTION_H

#include "qgis_core.h"
#include "qgsdatasourceuri.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsstyle.h"
//#include "../../providers/wms/qgswmsconnection.h"
//#include "../../providers/wfs/qgswfsconnection.h"

#include <QString>


/*!
 * \brief   GeoNode Connections management
 */
class CORE_EXPORT QgsGeoNodeConnection : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsGeoNodeConnection( const QString &connName );

    //! Destructor
    ~QgsGeoNodeConnection();

    //! Retrieve all geonode connection
    static QStringList connectionList();

    //! Delete connection with name, name
    static void deleteConnection( const QString &name );

    //! Get selected connection
    static QString selectedConnection();

    //! Set selected connection
    static void setSelectedConnection( const QString &name );

    // Methods below can be moved to another class. I will put here first until I decide. (Ismail)

//    //! Get all layer IDs from the geonode instances
//    QStringList layerIDs();

//    //! Get all map IDs from the geonode instances
//    QStringList mapIDs();

//    //! Get all style IDs from the geonode instances
//    QStringList styleIDs();

//    //! Get all style IDs of a layer from the geonode instances
//    QStringList layerStyleIDs( QString &layerID );

//    QgsWMSConnection layerWMSConnection( QString &layerID );

//    QgsWfsConnection layerWFSConnection( QString &layerID );

//    QgsWMSConnection mapWMSConnection( QString &mapID );

//    void downloadLayer( QString &layerID, QString &location );
//    void downloadStyle( QString &styleID, QString &location );
//    void downloadLayerMetadata( QString &layerID, QString &location );

//    void downloadQGISProject( QString &mapID, QString &location );

//    void publishLayerFile( QgsMapLayer &layer );
//    void publishQGISProject( QgsProject &project );
//    void publishStyle( QgsStyle &style );


  public:
    //! The connection name
    QString mConnName;

    //! Getter for mUri
    QgsDataSourceUri uri();

    //! Property of mUri
    QgsDataSourceUri mUri;

    //! List of wms connection of the geonode instance
    QStringList wmsConnectionNames;

    //! List of wfs connection of the geonode instance
    QStringList wfsConnectionNames;
};


#endif //QGSGEONODECONNECTION_H
