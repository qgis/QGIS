/***************************************************************************
  qgssettingsregistrycore.cpp
  --------------------------------------
  Date                 : February 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingsregistrycore.h"

#include "qgsapplication.h"
#include "qgsgeometryoptions.h"
#include "qgslayout.h"
#include "qgslocalizeddatapathregistry.h"
#include "qgslocator.h"
#include "qgsmaprendererjob.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnewsfeedparser.h"
#include "qgsowsconnection.h"
#include "qgsprocessing.h"
#include "qgsogrdbconnection.h"
#include "qgsfontmanager.h"
#include "qgsgpsconnection.h"
#include "qgsbabelformatregistry.h"
#include "qgsgpslogger.h"
#include "qgsvectortileconnection.h"


QgsSettingsRegistryCore::QgsSettingsRegistryCore()
  : QgsSettingsRegistry()
{
  addSettingsEntry( &QgsLayout::settingsSearchPathForTemplates );

  addSettingsEntry( &QgsLocator::settingsLocatorFilterEnabled );
  addSettingsEntry( &QgsLocator::settingsLocatorFilterDefault );
  addSettingsEntry( &QgsLocator::settingsLocatorFilterPrefix );

  addSettingsEntry( &QgsNetworkAccessManager::settingsNetworkTimeout );

  addSettingsEntry( &QgsNewsFeedParser::settingsFeedLastFetchTime );
  addSettingsEntry( &QgsNewsFeedParser::settingsFeedLanguage );
  addSettingsEntry( &QgsNewsFeedParser::settingsFeedLatitude );
  addSettingsEntry( &QgsNewsFeedParser::settingsFeedLongitude );

  addSettingsEntry( &QgsProcessing::settingsPreferFilenameAsLayerName );
  addSettingsEntry( &QgsProcessing::settingsTempPath );
  addSettingsEntry( &QgsProcessing::settingsDefaultOutputVectorLayerExt );
  addSettingsEntry( &QgsProcessing::settingsDefaultOutputRasterLayerExt );

  addSettingsEntry( &QgsGeometryOptions::settingsGeometryValidationDefaultChecks );

  addSettingsEntry( &QgsLocalizedDataPathRegistry::settingsLocalizedDataPaths );

  addSettingsEntry( &QgsMapRendererJob::settingsLogCanvasRefreshEvent );

  addSettingsEntry( &QgsOgrDbConnection::settingsOgrConnectionSelected );
  addSettingsEntry( &QgsOgrDbConnection::settingsOgrConnectionPath );

  addSettingsEntry( &settingsDigitizingStreamTolerance );
  addSettingsEntry( &settingsDigitizingLineWidth );
  addSettingsEntry( &settingsDigitizingLineColorRed );
  addSettingsEntry( &settingsDigitizingLineColorGreen );
  addSettingsEntry( &settingsDigitizingLineColorBlue );
  addSettingsEntry( &settingsDigitizingLineColorAlpha );
  addSettingsEntry( &settingsDigitizingLineColorAlphaScale );
  addSettingsEntry( &settingsDigitizingFillColorRed );
  addSettingsEntry( &settingsDigitizingFillColorGreen );
  addSettingsEntry( &settingsDigitizingFillColorBlue );
  addSettingsEntry( &settingsDigitizingFillColorAlpha );
  addSettingsEntry( &settingsDigitizingLineGhost );
  addSettingsEntry( &settingsDigitizingDefaultZValue );
  addSettingsEntry( &settingsDigitizingDefaultMValue );
  addSettingsEntry( &settingsDigitizingDefaultSnapEnabled );
  addSettingsEntry( &settingsDigitizingDefaultSnapMode );
  addSettingsEntry( &settingsDigitizingDefaultSnapType );
  addSettingsEntry( &settingsDigitizingDefaultSnappingTolerance );
  addSettingsEntry( &settingsDigitizingDefaultSnappingToleranceUnit );
  addSettingsEntry( &settingsDigitizingSearchRadiusVertexEdit );
  addSettingsEntry( &settingsDigitizingSearchRadiusVertexEditUnit );
  addSettingsEntry( &settingsDigitizingSnapColor );
  addSettingsEntry( &settingsDigitizingSnapTooltip );
  addSettingsEntry( &settingsDigitizingSnapInvisibleFeature );
  addSettingsEntry( &settingsDigitizingMarkerOnlyForSelected );
  addSettingsEntry( &settingsDigitizingMarkerStyle );
  addSettingsEntry( &settingsDigitizingMarkerSizeMm );
  addSettingsEntry( &settingsDigitizingReuseLastValues );
  addSettingsEntry( &settingsDigitizingDisableEnterAttributeValuesDialog );
  addSettingsEntry( &settingsDigitizingValidateGeometries );
  addSettingsEntry( &settingsDigitizingOffsetJoinStyle );
  addSettingsEntry( &settingsDigitizingOffsetQuadSeg );
  addSettingsEntry( &settingsDigitizingOffsetMiterLimit );
  addSettingsEntry( &settingsDigitizingConvertToCurve );
  addSettingsEntry( &settingsDigitizingConvertToCurveAngleTolerance );
  addSettingsEntry( &settingsDigitizingConvertToCurveDistanceTolerance );
  addSettingsEntry( &settingsDigitizingOffsetCapStyle );
  addSettingsEntry( &settingsDigitizingOffsetShowAdvanced );
  addSettingsEntry( &settingsDigitizingTracingMaxFeatureCount );
  addSettingsEntry( &settingsGpsBabelPath );
  addSettingsEntry( &settingsLayerTreeShowFeatureCountForNewLayers );
  addSettingsEntry( &settingsEnableWMSTilePrefetching );


  addSettingsEntry( &QgsFontManager::settingsFontFamilyReplacements );
  addSettingsEntry( &QgsFontManager::settingsDownloadMissingFonts );

  addSettingsEntry( &QgsGpsConnection::settingsGpsConnectionType );
  addSettingsEntry( &QgsGpsConnection::settingsGpsdHostName );
  addSettingsEntry( &QgsGpsConnection::settingsGpsdPortNumber );
  addSettingsEntry( &QgsGpsConnection::settingsGpsdDeviceName );
  addSettingsEntry( &QgsGpsConnection::settingsGpsSerialDevice );
  addSettingsEntry( &QgsGpsConnection::settingGpsAcquisitionInterval );
  addSettingsEntry( &QgsGpsConnection::settingGpsDistanceThreshold );
  addSettingsEntry( &QgsGpsConnection::settingGpsBearingFromTravelDirection );
  addSettingsEntry( &QgsGpsConnection::settingGpsApplyLeapSecondsCorrection );
  addSettingsEntry( &QgsGpsConnection::settingGpsLeapSeconds );
  addSettingsEntry( &QgsGpsConnection::settingsGpsTimeStampSpecification );
  addSettingsEntry( &QgsGpsConnection::settingsGpsTimeStampTimeZone );
  addSettingsEntry( &QgsGpsConnection::settingsGpsTimeStampOffsetFromUtc );

  addSettingsEntry( &QgsGpsLogger::settingsGpsStoreAttributeInMValues );
  addSettingsEntry( &QgsGpsLogger::settingsGpsMValueComponent );

  migrateOldSettings();
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
  backwardCompatibility();
}

void QgsSettingsRegistryCore::migrateOldSettings()
{
  // the logic is to copy value from old key to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // connections settings - added in 3.30
  const QStringList connectionsList = {QStringLiteral( "WMS" ), QStringLiteral( "WFS" ), QStringLiteral( "WCS" ), QStringLiteral( "GEONODE" )};
  for ( const QString &connection : connectionsList )
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "qgis/connections-%1" ).arg( connection.toLower() ) );
    const QStringList services = settings.childGroups();
    if ( services.count() == 0 )
      continue;
    for ( const QString &service : services )
    {
      QgsOwsConnection::settingsConnectionUrl->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/url" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionVersion->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/version" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionIgnoreGetMapURI->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetMapURI" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionIgnoreGetFeatureInfoURI->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetFeatureInfoURI" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionSmoothPixmapTransform->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/smoothPixmapTransform" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionReportedLayerExtents->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/reportedLayerExtents" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionDpiMode->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/dpiMode" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionTilePixelRatio->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/tilePixelRatio" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionMaxNumFeatures->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/maxnumfeatures" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionPagesize->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/pagesize" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionPagingEnabled->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/pagingenabled" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionPreferCoordinatesForWfsT11->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/preferCoordinatesForWfsT11" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionIgnoreAxisOrientation->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreAxisOrientation" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionInvertAxisOrientation->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/invertAxisOrientation" ), {connection.toLower(), service}, true );

      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( service );
      QgsOwsConnection::settingsConnectionHeaders->setValue( QgsHttpHeaders( settings ).headers(), {connection.toLower(), service} );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP

      QgsOwsConnection::settingsConnectionUsername->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/username" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionPassword->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/password" ), {connection.toLower(), service}, true );
      QgsOwsConnection::settingsConnectionAuthCfg->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/authcfg" ), {connection.toLower(), service}, true );
    }
    if ( settings.contains( QStringLiteral( "selected" ) ) )
      QgsOwsConnection::sTreeConnectionServices->setSelectedItem( settings.value( QStringLiteral( "selected" ) ).toString(), {connection.toLower()} );
  }

  // Vector tile - added in 3.30
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "qgis/connections-vector-tile" ) );
    const QStringList services = settings.childGroups();
    for ( const QString &service : services )
    {
      QgsVectorTileProviderConnection::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/url" ), {service} );
      QgsVectorTileProviderConnection::settingsZzmin->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmin" ), {service} );
      QgsVectorTileProviderConnection::settingsZmax->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmax" ), {service} );
      QgsVectorTileProviderConnection::settingsAuthcfg->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/authcfg" ), {service} );
      QgsVectorTileProviderConnection::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/username" ), {service} );
      QgsVectorTileProviderConnection::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/password" ), {service} );
      QgsVectorTileProviderConnection::settingsStyleUrl->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/styleUrl" ), {service} );
      QgsVectorTileProviderConnection::settingsServiceType->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/serviceType" ), {service} );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( service );
      QgsVectorTileProviderConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), service );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP
    }
  }

  // babel devices settings - added in 3.30
  {
    if ( QgsBabelFormatRegistry::sTreeBabelDevices->items().count() == 0 )
    {
      const QStringList deviceNames = QgsSettings().value( QStringLiteral( "/Plugin-GPS/devices/deviceList" ) ).toStringList();

      for ( const QString &device : deviceNames )
      {
        QgsBabelFormatRegistry::settingsBabelWptDownload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/wptdownload" ), {device}, true );
        QgsBabelFormatRegistry::settingsBabelWptUpload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/wptupload" ), {device}, true );
        QgsBabelFormatRegistry::settingsBabelRteDownload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/rtedownload" ), {device}, true );
        QgsBabelFormatRegistry::settingsBabelRteUpload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/rteupload" ), {device}, true );
        QgsBabelFormatRegistry::settingsBabelTrkDownload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/trkdownload" ), {device}, true );
        QgsBabelFormatRegistry::settingsBabelTrkUpload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/trkupload" ), {device}, true );
      }
    }
  }
}

void QgsSettingsRegistryCore::backwardCompatibility()
{
  // OWS connections settings - added in 3.30
  {
    const QStringList connectionsList = {QStringLiteral( "WMS" ), QStringLiteral( "WFS" ), QStringLiteral( "WCS" ), QStringLiteral( "GEONODE" )};
    for ( const QString &connection : connectionsList )
    {
      const QStringList services = QgsOwsConnection::sTreeConnectionServices->items( {connection.toLower()} );
      if ( services.count() == 0 )
        continue;
      QgsSettings settings;
      settings.beginGroup( QStringLiteral( "qgis/connections-%1" ).arg( connection.toLower() ) );
      for ( const QString &service : services )
      {
        QgsOwsConnection::settingsConnectionUrl->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/url" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionVersion->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/version" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionIgnoreGetMapURI->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetMapURI" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionIgnoreGetFeatureInfoURI->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetFeatureInfoURI" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionSmoothPixmapTransform->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/smoothPixmapTransform" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionReportedLayerExtents->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/reportedLayerExtents" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionDpiMode->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/dpiMode" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionTilePixelRatio->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/tilePixelRatio" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionMaxNumFeatures->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/maxnumfeatures" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionPagesize->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/pagesize" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionPagingEnabled->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/pagingenabled" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionPreferCoordinatesForWfsT11->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/preferCoordinatesForWfsT11" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionIgnoreAxisOrientation->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/ignoreAxisOrientation" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionInvertAxisOrientation->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/invertAxisOrientation" ), {connection.toLower(), service} );

        Q_NOWARN_DEPRECATED_PUSH
        settings.beginGroup( service );
        QgsHttpHeaders( QgsOwsConnection::settingsConnectionHeaders->value( {connection.toLower(), service} ) ).updateSettings( settings );
        settings.endGroup();
        Q_NOWARN_DEPRECATED_POP

        QgsOwsConnection::settingsConnectionUsername->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/username" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionPassword->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/password" ), {connection.toLower(), service} );
        QgsOwsConnection::settingsConnectionAuthCfg->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/authcfg" ), {connection.toLower(), service} );
      }

      if ( settings.contains( QStringLiteral( "selected" ) ) )
        QgsOwsConnection::sTreeConnectionServices->setSelectedItem( settings.value( QStringLiteral( "selected" ) ).toString(), {connection.toLower()} );
    }
  }

  // Vector tile - added in 3.30
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "qgis/connections-vector-tile" ) );

    const QStringList services = QgsVectorTileProviderConnection::sTreeConnectionVectorTile->items();
    for ( const QString &service : services )
    {
      // do not overwrite already set setting
      if ( QgsVectorTileProviderConnection::settingsUrl->exists( service ) )
        continue;
      QgsVectorTileProviderConnection::settingsUrl->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/url" ), {service} );
      QgsVectorTileProviderConnection::settingsZzmin->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmin" ), {service} );
      QgsVectorTileProviderConnection::settingsZmax->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmax" ), {service} );
      QgsVectorTileProviderConnection::settingsAuthcfg->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/authcfg" ), {service} );
      QgsVectorTileProviderConnection::settingsUsername->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/username" ), {service} );
      QgsVectorTileProviderConnection::settingsPassword->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/password" ), {service} );
      QgsVectorTileProviderConnection::settingsStyleUrl->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/styleUrl" ), {service} );
      QgsVectorTileProviderConnection::settingsServiceType->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/serviceType" ), {service} );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( service );
      QgsHttpHeaders( QgsVectorTileProviderConnection::settingsHeaders->value( service ) ).updateSettings( settings );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP
    }
  }

  // babel devices settings - added in 3.30
  {
    const QStringList devices = QgsBabelFormatRegistry::sTreeBabelDevices->items();
    QgsSettings().setValue( QStringLiteral( "/Plugin-GPS/devices/deviceList" ), devices );
    for ( const QString &device : devices )
    {
      QgsBabelFormatRegistry::settingsBabelWptDownload->copyValueToKey( QStringLiteral( "/Plugin-GPS/devices/%1/wptdownload" ), {device} );
      QgsBabelFormatRegistry::settingsBabelWptUpload->copyValueToKey( QStringLiteral( "/Plugin-GPS/devices/%1/wptupload" ), {device} );
      QgsBabelFormatRegistry::settingsBabelRteDownload->copyValueToKey( QStringLiteral( "/Plugin-GPS/devices/%1/rtedownload" ), {device} );
      QgsBabelFormatRegistry::settingsBabelRteUpload->copyValueToKey( QStringLiteral( "/Plugin-GPS/devices/%1/rteupload" ), {device} );
      QgsBabelFormatRegistry::settingsBabelTrkDownload->copyValueToKey( QStringLiteral( "/Plugin-GPS/devices/%1/trkdownload" ), {device} );
      QgsBabelFormatRegistry::settingsBabelTrkUpload->copyValueToKey( QStringLiteral( "/Plugin-GPS/devices/%1/trkupload" ), {device} );
    }
  }
}

