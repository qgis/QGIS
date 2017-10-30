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

/*!
 * \brief  Generic OGR DB Connections management
 */
class QgsOgrDbConnection : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsOgrDbConnection( const QString &connName, const QString &settingsKey );

    static const QStringList connectionList( const QString &settingsKey );
    static void deleteConnection( const QString &connName, const QString &settingsKey );
    static QString selectedConnection( const QString &settingsKey );
    static void setSelectedConnection( const QString &connName, const QString &settingsKey );

  public:

    /**
     * Return the uri
     * \see QgsDataSourceUri
     */
    QgsDataSourceUri uri();
    //! Return the path
    QString path( ) const { return mPath; }
    //! Returns the connection name
    QString name() const { return mConnName; }
    //! Set the \a path fo the connection
    void setPath( const QString &path );
    //! Store the connection data in the settings
    void save();

  private:
    static QString fullKey( const QString &settingsKey );
    static QString connectionsPath( const QString &settingsKey );
    QString mConnName;
    QString mPath;
    QString mSettingsKey;

};

#endif // QGSGOGRDBSCONNECTION_H
