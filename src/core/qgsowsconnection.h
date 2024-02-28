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
#include "qgssettingstree.h"

#include <QStringList>
#include <QPushButton>


class QgsSettingsEntryBool;
class QgsSettingsEntryDouble;
class QgsSettingsEntryInteger;
class QgsSettingsEntryString;
class QgsSettingsEntryVariantMap;
template<class T> class QgsSettingsEntryEnumFlag;


/**
 * \ingroup core
 * \brief Connections settings for XYZ
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsXyzConnectionSettings SIP_SKIP
{
  public:
    static inline QgsSettingsTreeNamedListNode *sTreeXyzConnections = QgsSettingsTree::sTreeConnections->createNamedListNode( QStringLiteral( "xyz" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsUrl;
    static const QgsSettingsEntryVariantMap *settingsHeaders;
    static const QgsSettingsEntryInteger *settingsZmin;
    static const QgsSettingsEntryInteger *settingsZmax;
    static const QgsSettingsEntryDouble *settingsTilePixelRatio;
    static const QgsSettingsEntryBool *settingsHidden;
    static const QgsSettingsEntryString *settingsInterpretation;

    static const QgsSettingsEntryString *settingsUsername;
    static const QgsSettingsEntryString *settingsPassword;
    static const QgsSettingsEntryString *settingsAuthcfg;
};


/**
 * \ingroup core
 * \brief Connections settings for Arcgis
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsArcGisConnectionSettings SIP_SKIP
{
  public:
    static inline QgsSettingsTreeNamedListNode *sTreeConnectionArcgis = QgsSettingsTree::sTreeConnections->createNamedListNode( QStringLiteral( "arcgisfeatureserver" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsUrl;
    static const QgsSettingsEntryString *settingsAuthcfg;
    static const QgsSettingsEntryString *settingsUsername;
    static const QgsSettingsEntryString *settingsPassword;
    static const QgsSettingsEntryVariantMap *settingsHeaders;
    static const QgsSettingsEntryString *settingsContentEndpoint;
    static const QgsSettingsEntryString *settingsCommunityEndpoint;
    static const QgsSettingsEntryString *settingsUrlPrefix;
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
    static inline QgsSettingsTreeNamedListNode *sTtreeOwsServices = QgsSettingsTree::sTreeConnections->createNamedListNode( QStringLiteral( "ows" ) );
    static inline QgsSettingsTreeNamedListNode *sTreeOwsConnections = sTtreeOwsServices->createNamedListNode( QStringLiteral( "connections" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsUrl;
    static const QgsSettingsEntryVariantMap *settingsHeaders;
    static const QgsSettingsEntryString *settingsVersion;
    static const QgsSettingsEntryBool *settingsIgnoreGetMapURI;
    static const QgsSettingsEntryBool *settingsIgnoreGetFeatureInfoURI;
    static const QgsSettingsEntryBool *settingsSmoothPixmapTransform;
    static const QgsSettingsEntryBool *settingsReportedLayerExtents;
    static const QgsSettingsEntryEnumFlag<Qgis::DpiMode> *settingsDpiMode;
    static const QgsSettingsEntryEnumFlag<Qgis::TilePixelRatio> *settingsTilePixelRatio;
    static const QgsSettingsEntryString *settingsMaxNumFeatures;
    static const QgsSettingsEntryString *settingsPagesize;
    static const QgsSettingsEntryString *settingsPagingEnabled;
    static const QgsSettingsEntryBool *settingsPreferCoordinatesForWfsT11;
    static const QgsSettingsEntryBool *settingsIgnoreAxisOrientation;
    static const QgsSettingsEntryBool *settingsInvertAxisOrientation;
    static const QgsSettingsEntryString *settingsUsername;
    static const QgsSettingsEntryString *settingsPassword;
    static const QgsSettingsEntryString *settingsAuthCfg;

#endif

    /**
     * Constructor
     * \param service service name: WMS,WFS,WCS
     * \param connName connection name
     */
    QgsOwsConnection( const QString &service, const QString &connName );

    /**
     * Returns the connection name.
     */
    QString connectionName() const;

    /**
     * Returns connection info string.
     */
    QString connectionInfo() const;

    /**
     * Returns a string representing the service type, e.g. "WMS".
     */
    QString service() const;

    /**
     * Returns the connection uri.
     */
    QgsDataSourceUri uri() const;

    /**
     * Adds uri parameters relating to the settings for a WMS or WCS connection to a QgsDataSourceUri \a uri.
     * Connection settings are taken from the specified QSettings \a settingsKey.
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
