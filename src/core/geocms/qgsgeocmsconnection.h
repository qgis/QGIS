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
#include "qgsgeonoderequest.h"

#include <QString>
#include <QMultiMap>
#include <QNetworkReply>


/** \ingroup core
 * Base class for all GeoCMS connection.
 * Parent/children hierarchy is not based on QObject.
*/
class CORE_EXPORT QgsGeoCmsConnection : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param geoCMSName GeoCMS name eg: geonode, qgiscloud, etc
     * \param connName connection name
     */
    QgsGeoCmsConnection( const QString &geoCMSName, const QString &connName );

    //! Destructor
    ~QgsGeoCmsConnection();


    // Getter and Setter
    QString geoCMSName() const;
    void setGeoCMSName( const QString &geoCMSName );

    QString connName() const;
    void setConnName( const QString &connName );

    QgsDataSourceUri uri();
    void setUri( const QgsDataSourceUri &uri );

    //! Returns the list of connections for the specified GeoCMS
    static QStringList connectionList( const QString &geoCMSName );

    //! Deletes the connection for the specified GeoCMS with the specified name
    static void deleteConnection( const QString &geoCMSName, const QString &name );

    //! Retrieves the selected connection for the specified GeoCMS
    static QString selectedConnection( const QString &geoCMSName );

    //! Marks the specified connection for the specified GeoCMS as selected
    static void setSelectedConnection( const QString &geoCMSName, const QString &name );

  private:
    //! The GeoCMS name
    QString mGeoCMSName;

    //! The connection name
    QString mConnName;

    //! Property of mUri
    QgsDataSourceUri mUri;
};


#endif //QGSGEOCMSCONNECTION_H
