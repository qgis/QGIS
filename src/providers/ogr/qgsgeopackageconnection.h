/***************************************************************************
    qgsgeopackageconnection.h  -  GeoPackage connection
                             -------------------
    begin                : August 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOPACKAGECONNECTION_H
#define QGSGEOPACKAGECONNECTION_H

#include "qgsdatasourceuri.h"

#include <QStringList>

/*!
 * \brief   Connections management
 */
class QgsGeoPackageConnection : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsGeoPackageConnection( const QString &connName );

    ~QgsGeoPackageConnection();

    static QStringList connectionList();

    static void deleteConnection( const QString &name );

    static QString selectedConnection();
    static void setSelectedConnection( const QString &name );

  public:
    //! Return the uri
    //! \see QgsDataSourceUri
    QgsDataSourceUri uri();
    //! Return the path
    QString path( ) const { return mPath; }
    //! Returns the connection name
    QString name() const { return mConnName; }
    //! Set the \a path fo the connection
    void setPath( const QString &path );
    //! Store the connection data in the settings
    void save();
    const static QString SETTINGS_PREFIX;

  private:

    static QString connectionsPath( );
    QString mConnName;
    QString mPath;

};

#endif // QGSGEOPACKAGECONNECTION_H
