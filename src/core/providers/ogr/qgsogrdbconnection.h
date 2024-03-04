/***************************************************************************
    qgsogrdbconnection.h  -  QgsOgrDbConnection
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

#ifndef QGSGOGRDBSCONNECTION_H
#define QGSGOGRDBSCONNECTION_H

#include "qgsdatasourceuri.h"

#include <QStringList>
#include "qgis_sip.h"
#include "qgssettingsentryimpl.h"

///@cond PRIVATE
#define SIP_NO_FILE

/*!
 * \brief  Generic OGR DB Connections management
 */
class CORE_EXPORT QgsOgrDbConnection : public QObject
{
    Q_OBJECT

  public:
    static const QgsSettingsEntryString *settingsOgrConnectionPath;
    static const QgsSettingsEntryString *settingsOgrConnectionSelected;

    //! Constructor
    explicit QgsOgrDbConnection( const QString &connName, const QString &settingsKey );

    static const QStringList connectionList( const QString &driverName = QStringLiteral( "GPKG" ) );
    static void deleteConnection( const QString &connName );
    static QString selectedConnection( const QString &driverName );
    static void setSelectedConnection( const QString &connName, const QString &settingsKey );

  public:

    /**
     * Returns the uri
     * \see QgsDataSourceUri
     */
    QgsDataSourceUri uri();
    //! Returns the path
    QString path( ) const { return mPath; }
    //! Returns the connection name
    QString name() const { return mConnName; }
    //! Sets the \a path for the connection
    void setPath( const QString &path );
    //! Store the connection data in the settings
    void save();
    //! Returns true if the DB supports QGIS project storage
    bool allowProjectsInDatabase();

  private:
    QString mConnName;
    QString mPath;
    QString mSettingsKey;

};

///@endcond
#endif // QGSGOGRDBSCONNECTION_H
