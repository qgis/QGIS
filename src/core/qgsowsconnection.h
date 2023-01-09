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
 * \brief Connections settingss for XYZ
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsXyzConnectionSettings SIP_SKIP
{
  public:
    static inline QgsSettingsTreeNamedListElement *sTreeXyzConnections = QgsSettings::sTreeConnections->createNamedListElement( QStringLiteral( "xyz" ), QgsSettingsTreeElement::Option::NamedListSelectedItemSetting );

    static const inline QgsSettingsEntryString *settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeXyzConnections, QString() ) ;
    static const inline QgsSettingsEntryVariantMap *settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeXyzConnections ) ;
    static const inline QgsSettingsEntryInteger *settingsZmin = new QgsSettingsEntryInteger( QStringLiteral( "zmin" ), sTreeXyzConnections, -1 );
    static const inline QgsSettingsEntryInteger *settingsZmax = new QgsSettingsEntryInteger( QStringLiteral( "zmax" ), sTreeXyzConnections, -1 );
    static const inline QgsSettingsEntryDouble *settingsTilePixelRatio = new QgsSettingsEntryDouble( QStringLiteral( "tile-pixel-ratio" ), sTreeXyzConnections, 0, QStringLiteral( "0 = unknown (not scaled), 1.0 = 256x256, 2.0 = 512x512" ) ) ;
    static const inline QgsSettingsEntryBool *settingsHidden = new QgsSettingsEntryBool( QStringLiteral( "hidden" ), sTreeXyzConnections, false ) ;
    static const inline QgsSettingsEntryString *settingsInterpretation = new QgsSettingsEntryString( QStringLiteral( "interpretation" ), sTreeXyzConnections, QString() ) ;

    static const inline QgsSettingsEntryString *settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeXyzConnections ) ;
    static const inline QgsSettingsEntryString *settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeXyzConnections ) ;
    static const inline QgsSettingsEntryString *settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeXyzConnections ) ;
};


/**
 * \ingroup core
 * \brief Connections management
 */
class CORE_EXPORT QgsOwsConnection : public QObject
{
    Q_OBJECT

  public:

#ifndef SIP_RUN
    static inline QgsSettingsTreeNamedListElement *sTtreeOwsServices = QgsSettings::sTreeConnections->createNamedListElement( QStringLiteral( "ows" ) );
    static inline QgsSettingsTreeNamedListElement *sTreeOwsConnections = sTtreeOwsServices->createNamedListElement( QStringLiteral( "connections" ) );

    static const inline QgsSettingsEntryString *settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeOwsConnections, QString() ) ;
    static const inline QgsSettingsEntryVariantMap *settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeOwsConnections ) ;
    static const inline QgsSettingsEntryString *settingsVersion = new QgsSettingsEntryString( QStringLiteral( "version" ), sTreeOwsConnections, QString() ) ;
    static const inline QgsSettingsEntryBool *settingsIgnoreGetMapURI = new QgsSettingsEntryBool( QStringLiteral( "ignore-get-map-uri" ), sTreeOwsConnections, false ) ;
    static const inline QgsSettingsEntryBool *settingsIgnoreGetFeatureInfoURI = new QgsSettingsEntryBool( QStringLiteral( "ignore-get-feature-info-uri" ), sTreeOwsConnections, false ) ;
    static const inline QgsSettingsEntryBool *settingsSmoothPixmapTransform = new QgsSettingsEntryBool( QStringLiteral( "smooth-pixmap-transform" ), sTreeOwsConnections, false ) ;
    static const inline QgsSettingsEntryBool *settingsReportedLayerExtents = new QgsSettingsEntryBool( QStringLiteral( "reported-layer-extents" ), sTreeOwsConnections, false ) ;
    static const inline QgsSettingsEntryEnumFlag<Qgis::DpiMode> *settingsDpiMode = new QgsSettingsEntryEnumFlag<Qgis::DpiMode>( QStringLiteral( "dpi-mode" ), sTreeOwsConnections, Qgis::DpiMode::All, QString(), Qgis::SettingsOption::SaveEnumFlagAsInt ) ;
    static const inline QgsSettingsEntryEnumFlag<Qgis::TilePixelRatio> *settingsTilePixelRatio = new QgsSettingsEntryEnumFlag<Qgis::TilePixelRatio>( QStringLiteral( "tile-pixel-ratio" ), sTreeOwsConnections, Qgis::TilePixelRatio::Undefined, QString(), Qgis::SettingsOption::SaveEnumFlagAsInt ) ;
    static const inline QgsSettingsEntryString *settingsMaxNumFeatures = new QgsSettingsEntryString( QStringLiteral( "max-num-features" ), sTreeOwsConnections ) ;
    static const inline QgsSettingsEntryString *settingsPagesize = new QgsSettingsEntryString( QStringLiteral( "page-size" ), sTreeOwsConnections ) ;
    static const inline QgsSettingsEntryBool *settingsPagingEnabled = new QgsSettingsEntryBool( QStringLiteral( "paging-enabled" ), sTreeOwsConnections, true ) ;
    static const inline QgsSettingsEntryBool *settingsPreferCoordinatesForWfsT11 = new QgsSettingsEntryBool( QStringLiteral( "prefer-coordinates-for-wfs-T11" ), sTreeOwsConnections, false ) ;
    static const inline QgsSettingsEntryBool *settingsIgnoreAxisOrientation = new QgsSettingsEntryBool( QStringLiteral( "ignore-axis-orientation" ), sTreeOwsConnections, false ) ;
    static const inline QgsSettingsEntryBool *settingsInvertAxisOrientation = new QgsSettingsEntryBool( QStringLiteral( "invert-axis-orientation" ), sTreeOwsConnections, false ) ;
    static const inline QgsSettingsEntryString *settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeOwsConnections ) ;
    static const inline QgsSettingsEntryString *settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeOwsConnections ) ;
    static const inline QgsSettingsEntryString *settingsAuthCfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeOwsConnections ) ;

#endif

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
