/***************************************************************************
    qgsowsconnection.h  -  OWS connection
                             -------------------
    begin                : 3 April 2005
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG

    generalized          : (C) 2012 Radim Blazek, based on qgswmsconnection.h


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOWSCONNECTION_H
#define QGSOWSCONNECTION_H

#include "qgis_core.h"
#include "qgsdatasourceuri.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"

#include <QStringList>
#include <QPushButton>

/**
 * \ingroup core
 * \brief Connections management
 */
class CORE_EXPORT QgsOwsConnection : public QObject
{
    Q_OBJECT

  public:

    static const inline QgsSettingsEntryString settingsConnectionSelected = QgsSettingsEntryString( QStringLiteral( "connections-%1/selected" ), QgsSettings::Prefix::QGIS ) SIP_SKIP;

    static const inline QgsSettingsEntryString settingsConnectionUrl = QgsSettingsEntryString( QStringLiteral( "connections-%1/%2/url" ), QgsSettings::Prefix::QGIS, QString() ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingsConnectionReferer = QgsSettingsEntryString( QStringLiteral( "connections-%1/%2/referer" ), QgsSettings::Prefix::QGIS, QString() ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingsConnectionVersion = QgsSettingsEntryString( QStringLiteral( "connections-%1/%2/version" ), QgsSettings::Prefix::QGIS, QString() ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionIgnoreGetMapURI = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/ignoreGetMapURI" ), QgsSettings::Prefix::QGIS, false ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionIgnoreGetFeatureInfoURI = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/ignoreGetFeatureInfoURI" ), QgsSettings::Prefix::QGIS, false ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionSmoothPixmapTransform = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/smoothPixmapTransform" ), QgsSettings::Prefix::QGIS, false ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionReportedLayerExtents = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/reportedLayerExtents" ), QgsSettings::Prefix::QGIS, false ) SIP_SKIP;
    static const inline QgsSettingsEntryEnumFlag<Qgis::DpiMode> settingsConnectionDpiMode = QgsSettingsEntryEnumFlag<Qgis::DpiMode>( QStringLiteral( "connections-%1/%2/dpiMode" ), QgsSettings::Prefix::QGIS, Qgis::DpiMode::All, QString(), Qgis::SettingsOption::SaveEnumFlagAsInt ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingsConnectionMaxNumFeatures = QgsSettingsEntryString( QStringLiteral( "connections-%1/%2/maxnumfeatures" ), QgsSettings::Prefix::QGIS ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingsConnectionPagesize = QgsSettingsEntryString( QStringLiteral( "connections-%1/%2/pagesize" ), QgsSettings::Prefix::QGIS ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionPagingEnabled = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/pagingenabled" ), QgsSettings::Prefix::QGIS, true ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionPreferCoordinatesForWfsT11 = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/preferCoordinatesForWfsT11" ), QgsSettings::Prefix::QGIS, false ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionIgnoreAxisOrientation = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/ignoreAxisOrientation" ), QgsSettings::Prefix::QGIS, false ) SIP_SKIP;
    static const inline QgsSettingsEntryBool settingsConnectionInvertAxisOrientation = QgsSettingsEntryBool( QStringLiteral( "connections-%1/%2/invertAxisOrientation" ), QgsSettings::Prefix::QGIS, false ) SIP_SKIP;

    static const inline QgsSettingsEntryString settingsConnectionUsername = QgsSettingsEntryString( QStringLiteral( "%1/%2/username" ), QgsSettings::Prefix::QGIS ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingsConnectionPassword = QgsSettingsEntryString( QStringLiteral( "%1/%2/password" ), QgsSettings::Prefix::QGIS ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingsConnectionAuthCfg = QgsSettingsEntryString( QStringLiteral( "%1/%2/authcfg" ), QgsSettings::Prefix::QGIS ) SIP_SKIP;

    static const inline QgsSettingsEntryGroup settingsServiceConnectionDetailsGroup = QgsSettingsEntryGroup( {&settingsConnectionUrl, &settingsConnectionReferer, &settingsConnectionVersion, &settingsConnectionIgnoreGetMapURI, &settingsConnectionIgnoreGetFeatureInfoURI, &settingsConnectionSmoothPixmapTransform, &settingsConnectionReportedLayerExtents, &settingsConnectionDpiMode, &settingsConnectionMaxNumFeatures, &settingsConnectionPagesize, &settingsConnectionPagingEnabled, &settingsConnectionPreferCoordinatesForWfsT11, &settingsConnectionIgnoreAxisOrientation, &settingsConnectionInvertAxisOrientation} );
    static const inline QgsSettingsEntryGroup settingsServiceConnectionCredentialsGroup = QgsSettingsEntryGroup( {&settingsConnectionUsername, &settingsConnectionPassword, &settingsConnectionAuthCfg} );

    /**
     * Constructor
     * \param service service name: WMS,WFS,WCS
     * \param connName connection name
     */
    QgsOwsConnection( const QString &service, const QString &connName );

    /**
     * Returns the connection name.
     * \since QGIS 3.0
     */
    QString connectionName() const;

    /**
     * Returns connection info string.
     * \since QGIS 3.0
     */
    QString connectionInfo() const;

    /**
     * Returns a string representing the service type, e.g. "WMS".
     * \since QGIS 3.0
     */
    QString service() const;

    /**
     * Returns the connection uri.
     */
    QgsDataSourceUri uri() const;

    /**
     * Adds uri parameters relating to the settings for a WMS or WCS connection to a QgsDataSourceUri \a uri.
     * Connection settings are taken from the specified QSettings \a settingsKey.
     * \since QGIS 3.0
     * \deprecated since QGIS 3.26 use addWmsWcsConnectionSettings with service and connection name parameters
     */
    Q_DECL_DEPRECATED static QgsDataSourceUri &addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey ) SIP_DEPRECATED;

    /**
     * Adds uri parameters relating to the settings for a WMS or WCS connection to a QgsDataSourceUri \a uri.
     * Connection settings are taken from the specified \a servcie and \a connName
     * \since QGIS 3.26
     */
    static QgsDataSourceUri &addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connName );

    /**
     * Adds uri parameters relating to the settings for a WFS connection to a QgsDataSourceUri \a uri.
     * Connection settings are taken from the specified QSettings \a settingsKey.
     * \since QGIS 3.0
     * \deprecated since QGIS 3.26 use addWfsConnectionSettings with service and connection name parameters
     */
    Q_DECL_DEPRECATED static QgsDataSourceUri &addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey ) SIP_DEPRECATED;

    /**
     * Adds uri parameters relating to the settings for a WFS connection to a QgsDataSourceUri \a uri.
     * Connection settings are taken from the specified \a servcie and \a connName
     * \since QGIS 3.26
     */
    static QgsDataSourceUri &addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connName );

    //! Returns the list of connections for the specified service
    static QStringList connectionList( const QString &service );

    //! Deletes the connection for the specified service with the specified name
    static void deleteConnection( const QString &service, const QString &name );

    //! Retrieves the selected connection for the specified service
    static QString selectedConnection( const QString &service );
    //! Marks the specified connection for the specified service as selected
    static void setSelectedConnection( const QString &service, const QString &name );

  protected:
    QgsDataSourceUri mUri;

  private:

    QString mConnName;
    QString mService;
    QString mConnectionInfo;

    Q_DECL_DEPRECATED static void addCommonConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey );
    static void addCommonConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connectionName );

};


#endif // QGSOWSCONNECTION_H
