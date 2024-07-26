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
#include <QThread>

#include "qgssettingsregistrycore.h"

#include "qgis.h"

#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettings.h"

#include "qgsbabelformatregistry.h"
#include "qgslayout.h"
#include "qgslocator.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsowsconnection.h"
#include "qgsprocessing.h"
#include "qgsvectortileconnection.h"
#include "qgsgpsdetector.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include "pal.h"

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsSettingsPre330Migrated = new QgsSettingsEntryBool( QStringLiteral( "settings-pre3.30-migrated" ), QgsSettingsTree::sTreeCore,  false );

const QgsSettingsEntryEnumFlag<Qgis::SnappingMode> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapMode = new QgsSettingsEntryEnumFlag<Qgis::SnappingMode>( QStringLiteral( "default-snap-mode" ), QgsSettingsTree::sTreeDigitizing, Qgis::SnappingMode::AllLayers );

const QgsSettingsEntryEnumFlag<Qgis::SnappingType> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType = new QgsSettingsEntryEnumFlag<Qgis::SnappingType>( QStringLiteral( "default-snap-type" ), QgsSettingsTree::sTreeDigitizing, Qgis::SnappingType::Vertex );

const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit = new QgsSettingsEntryEnumFlag<Qgis::MapToolUnit>( QStringLiteral( "default-snapping-tolerance-unit" ), QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_SNAP_UNITS );

const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit = new QgsSettingsEntryEnumFlag<Qgis::MapToolUnit>( QStringLiteral( "search-radius-vertex-edit-unit" ), QgsSettingsTree::sTreeDigitizing, Qgis::MapToolUnit::Pixels );

const QgsSettingsEntryEnumFlag<Qgis::JoinStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle = new QgsSettingsEntryEnumFlag<Qgis::JoinStyle>( QStringLiteral( "offset-join-style" ), QgsSettingsTree::sTreeDigitizing, Qgis::JoinStyle::Round );

const QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle = new QgsSettingsEntryEnumFlag<Qgis::EndCapStyle>( QStringLiteral( "offset-cap-style" ), QgsSettingsTree::sTreeDigitizing,  Qgis::EndCapStyle::Round );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingStreamTolerance = new QgsSettingsEntryInteger( QStringLiteral( "stream-tolerance" ), QgsSettingsTree::sTreeDigitizing, 2 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingLineWidth = new QgsSettingsEntryInteger( QStringLiteral( "line-width" ), QgsSettingsTree::sTreeDigitizing, 1 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingLineColor = new QgsSettingsEntryColor( QStringLiteral( "line-color" ), QgsSettingsTree::sTreeDigitizing, QColor( 255, 0, 0, 200 ) );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale = new QgsSettingsEntryDouble( QStringLiteral( "line-color-alpha-scale" ), QgsSettingsTree::sTreeDigitizing, 0.75 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingFillColor = new QgsSettingsEntryColor( QStringLiteral( "fill-color" ), QgsSettingsTree::sTreeDigitizing, QColor( 255, 0, 0, 30 ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingLineGhost = new QgsSettingsEntryBool( QStringLiteral( "line-ghost" ), QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultZValue = new QgsSettingsEntryDouble( QStringLiteral( "default-z-value" ), QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_Z_COORDINATE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultMValue = new QgsSettingsEntryDouble( QStringLiteral( "default-m-value" ), QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_M_COORDINATE );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled = new QgsSettingsEntryBool( QStringLiteral( "default-snap-enabled" ), QgsSettingsTree::sTreeDigitizing,  false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance = new QgsSettingsEntryDouble( QStringLiteral( "default-snapping-tolerance" ), QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_SNAP_TOLERANCE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit = new QgsSettingsEntryDouble( QStringLiteral( "search-radius-vertex-edit" ), QgsSettingsTree::sTreeDigitizing, 10 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingSnapColor = new QgsSettingsEntryColor( QStringLiteral( "snap-color" ), QgsSettingsTree::sTreeDigitizing, QColor( Qt::magenta ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapTooltip = new QgsSettingsEntryBool( QStringLiteral( "snap-tooltip" ), QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapInvisibleFeature = new QgsSettingsEntryBool( QStringLiteral( "snap-invisible-feature" ), QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected = new QgsSettingsEntryBool( QStringLiteral( "marker-only-for-selected" ), QgsSettingsTree::sTreeDigitizing, true );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsDigitizingMarkerStyle = new QgsSettingsEntryString( QStringLiteral( "marker-style" ), QgsSettingsTree::sTreeDigitizing, "Cross" );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm = new QgsSettingsEntryDouble( QStringLiteral( "marker-size-mm" ), QgsSettingsTree::sTreeDigitizing, 2.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingReuseLastValues = new QgsSettingsEntryBool( QStringLiteral( "reuse-last-values" ), QgsSettingsTree::sTreeDigitizing, false );


const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog = new QgsSettingsEntryBool( QStringLiteral( "disable-enter-attribute-values-dialog" ), QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingValidateGeometries = new QgsSettingsEntryInteger( QStringLiteral( "validate-geometries" ), QgsSettingsTree::sTreeDigitizing, 1 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg = new QgsSettingsEntryInteger( QStringLiteral( "offset-quad-seg" ), QgsSettingsTree::sTreeDigitizing, 8 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit = new QgsSettingsEntryDouble( QStringLiteral( "offset-miter-limit" ), QgsSettingsTree::sTreeDigitizing, 5.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingConvertToCurve = new QgsSettingsEntryBool( QStringLiteral( "convert-to-curve" ), QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance = new QgsSettingsEntryDouble( QStringLiteral( "convert-to-curve-angle-tolerance" ), QgsSettingsTree::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance = new QgsSettingsEntryDouble( QStringLiteral( "convert-to-curve-distance-tolerance" ), QgsSettingsTree::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingOffsetShowAdvanced = new QgsSettingsEntryBool( QStringLiteral( "offset-show-advanced" ), QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingTracingMaxFeatureCount = new QgsSettingsEntryInteger( QStringLiteral( "tracing-max-feature-count" ), QgsSettingsTree::sTreeDigitizing, 10000 );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsGpsBabelPath = new QgsSettingsEntryString( QStringLiteral( "gpsbabelPath" ), QgsSettingsTree::sTreeGps, QStringLiteral( "gpsbabel" ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerTreeShowFeatureCountForNewLayers = new QgsSettingsEntryBool( QStringLiteral( "show-feature-count-for-new-layers" ), QgsSettingsTree::sTreeLayerTree, false, QStringLiteral( "If true, feature counts will be shown in the layer tree for all newly added layers." ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsEnableWMSTilePrefetching = new QgsSettingsEntryBool( QStringLiteral( "enable_wms_tile_prefetch" ), QgsSettingsTree::sTreeWms, false, QStringLiteral( "Whether to include WMS layers when rendering tiles adjacent to the visible map area" ) );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsMapScales = new QgsSettingsEntryStringList( QStringLiteral( "default_scales" ), QgsSettingsTree::sTreeMap, Qgis::defaultProjectScales().split( ',' ) );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsLayerParallelLoadingMaxCount = new QgsSettingsEntryInteger( QStringLiteral( "provider-parallel-loading-max-count" ), QgsSettingsTree::sTreeCore, QThread::idealThreadCount(), QStringLiteral( "Maximum thread used to load layers in parallel" ), Qgis::SettingsOption(), 1 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerParallelLoading = new QgsSettingsEntryBool( QStringLiteral( "provider-parallel-loading" ), QgsSettingsTree::sTreeCore, true, QStringLiteral( "Load layers in parallel (only available for some providers (GDAL and PostgreSQL)" ), Qgis::SettingsOption() );

QgsSettingsRegistryCore::QgsSettingsRegistryCore()
  : QgsSettingsRegistry()
{
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
}

void QgsSettingsRegistryCore::migrateOldSettings()
{
  if ( !settingsSettingsPre330Migrated->value() )
  {
    // copy values from old keys to new keys and delete the old ones
    // for backward compatibility, old keys are recreated when the registry gets deleted

    // single settings - added in 3.30
    QgsLayout::settingsSearchPathForTemplates->copyValueFromKey( QStringLiteral( "core/Layout/searchPathsForTemplates" ) );

    QgsProcessing::settingsPreferFilenameAsLayerName->copyValueFromKey( QStringLiteral( "Processing/Configuration/PREFER_FILENAME_AS_LAYER_NAME" ) );
    QgsProcessing::settingsTempPath->copyValueFromKey( QStringLiteral( "Processing/Configuration/TEMP_PATH2" ) );
    QgsProcessing::settingsDefaultOutputVectorLayerExt->copyValueFromKey( QStringLiteral( "Processing/Configuration/DefaultOutputVectorLayerExt" ) );
    QgsProcessing::settingsDefaultOutputRasterLayerExt->copyValueFromKey( QStringLiteral( "Processing/Configuration/DefaultOutputRasterLayerExt" ) );

    QgsNetworkAccessManager::settingsNetworkTimeout->copyValueFromKey( QStringLiteral( "qgis/networkAndProxy/networkTimeout" ) );

    settingsLayerTreeShowFeatureCountForNewLayers->copyValueFromKey( QStringLiteral( "core/layer-tree/show_feature_count_for_new_layers" ) );

#if defined( HAVE_QTSERIALPORT )
    QgsGpsDetector::settingsGpsStopBits->copyValueFromKey( QStringLiteral( "core/gps/stop_bits" ) );
    QgsGpsDetector::settingsGpsFlowControl->copyValueFromKey( QStringLiteral( "core/gps/flow_control" ) );
    QgsGpsDetector::settingsGpsDataBits->copyValueFromKey( QStringLiteral( "core/gps/data_bits" ) );
    QgsGpsDetector::settingsGpsParity->copyValueFromKey( QStringLiteral( "core/gps/parity" ) );
#endif

    QgsRasterLayer::settingsRasterDefaultOversampling->copyValueFromKey( QStringLiteral( "Raster/defaultOversampling" ) );
    QgsRasterLayer::settingsRasterDefaultEarlyResampling->copyValueFromKey( QStringLiteral( "Raster/defaultEarlyResampling" ) );

    pal::Pal::settingsRenderingLabelCandidatesLimitPoints->copyValueFromKey( QStringLiteral( "core/rendering/label_candidates_limit_points" ) );
    pal::Pal::settingsRenderingLabelCandidatesLimitLines->copyValueFromKey( QStringLiteral( "core/rendering/label_candidates_limit_lines" ) );
    pal::Pal::settingsRenderingLabelCandidatesLimitPolygons->copyValueFromKey( QStringLiteral( "core/rendering/label_candidates_limit_polygons" ) );

    // handle bad migration - Fix profiles for old QGIS versions (restore the Map/scales key on windows)
    // TODO: Remove this from QGIS 3.36
    // PR Link: https://github.com/qgis/QGIS/pull/52580
    if ( QgsSettings().contains( QStringLiteral( "Map/scales" ) ) )
    {
      const QStringList oldScales = QgsSettings().value( QStringLiteral( "Map/scales" ) ).toStringList();
      if ( ! oldScales.isEmpty() && !oldScales.at( 0 ).isEmpty() )
        QgsSettings().setValue( QStringLiteral( "Map/scales" ), oldScales.join( ',' ) );
      else
        QgsSettings().setValue( QStringLiteral( "Map/scales" ), Qgis::defaultProjectScales() );
    }

    // migrate only one way for map scales
    if ( !settingsMapScales->exists() )
    {
      // Handle bad migration. Prefer map/scales over Map/scales
      // TODO: Discard this part starting from QGIS 3.36
      const QStringList oldScales = QgsSettings().value( QStringLiteral( "map/scales" ) ).toStringList();
      if ( ! oldScales.isEmpty() && !oldScales.at( 0 ).isEmpty() )
      {
        // If migration has failed before (QGIS < 3.30.2), all scales might be
        // concatenated in the first element of the list
        QStringList actualScales;
        for ( const QString &element : oldScales )
        {
          actualScales << element.split( "," );
        }
        settingsMapScales->setValue( actualScales );
      }
      // TODO: keep only this part of the migration starting from QGIS 3.36
      else if ( QgsSettings().contains( QStringLiteral( "Map/scales" ) ) )
      {
        settingsMapScales->setValue( QgsSettings().value( QStringLiteral( "Map/scales" ) ).toString().split( ',' ) );
      }
    }

    // digitizing settings - added in 3.30
    {
      settingsDigitizingLineColor->copyValueFromKeys( QStringLiteral( "qgis/digitizing/line_color_red" ), QStringLiteral( "qgis/digitizing/line_color_green" ), QStringLiteral( "qgis/digitizing/line_color_blue" ), QStringLiteral( "qgis/digitizing/line_color_alpha" ) );
      settingsDigitizingFillColor->copyValueFromKeys( QStringLiteral( "qgis/digitizing/fill_color_red" ), QStringLiteral( "qgis/digitizing/fill_color_green" ), QStringLiteral( "qgis/digitizing/fill_color_blue" ), QStringLiteral( "qgis/digitizing/fill_color_alpha" ) );

      const QList<const QgsSettingsEntryBase *> settings = QgsSettingsTree::sTreeDigitizing->childrenSettings();
      for ( const QgsSettingsEntryBase *setting : settings )
      {
        QString name = setting->name();
        if (
          name == settingsDigitizingStreamTolerance->name() ||
          name == settingsDigitizingLineColor->name() ||
          name == settingsDigitizingFillColor->name()
        )
          continue;
        if ( name == settingsDigitizingReuseLastValues->name() )
        {
          name = QStringLiteral( "reuseLastValues" );
        }
        else
        {
          name.replace( '-', '_' );
        }
        setting->copyValueFromKey( QString( "qgis/digitizing/%1" ).arg( name ) );
      }
    }

    // locator filters - added in 3.30
    {
      QgsSettings settings;
      settings.beginGroup( QStringLiteral( "gui/locator_filters" ) );
      const QStringList childKeys = settings.childKeys();
      for ( const QString &childKey : childKeys )
      {
        if ( childKey.startsWith( QLatin1String( "enabled" ) ) )
        {
          QString filter = childKey;
          filter.remove( QStringLiteral( "enabled_" ) );
          QgsLocator::settingsLocatorFilterEnabled->copyValueFromKey( QStringLiteral( "gui/locator_filters/enabled_%1" ), {filter} );
          QgsLocator::settingsLocatorFilterDefault->copyValueFromKey( QStringLiteral( "gui/locator_filters/default_%1" ), {filter} );
          QgsLocator::settingsLocatorFilterPrefix->copyValueFromKey( QStringLiteral( "gui/locator_filters/prefix_%1" ), {filter} );
        }
      }
    }

    // connections settings - added in 3.30
    const QStringList services = {QStringLiteral( "WMS" ), QStringLiteral( "WFS" ), QStringLiteral( "WCS" )};
    for ( const QString &service : services )
    {
      QgsSettings settings;
      settings.beginGroup( QStringLiteral( "qgis/connections-%1" ).arg( service.toLower() ) );
      const QStringList connections = settings.childGroups();
      if ( connections.count() == 0 )
        continue;
      for ( const QString &connection : connections )
      {
        QgsOwsConnection::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/url" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsVersion->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/version" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreGetMapURI->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetMapURI" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetFeatureInfoURI" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsSmoothPixmapTransform->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/smoothPixmapTransform" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsReportedLayerExtents->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/reportedLayerExtents" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsDpiMode->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/dpiMode" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsTilePixelRatio->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/tilePixelRatio" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsMaxNumFeatures->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/maxnumfeatures" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPagesize->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/pagesize" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPagingEnabled->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/pagingenabled" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPreferCoordinatesForWfsT11->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/preferCoordinatesForWfsT11" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreAxisOrientation->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreAxisOrientation" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsInvertAxisOrientation->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/invertAxisOrientation" ), {service.toLower(), connection} );

        Q_NOWARN_DEPRECATED_PUSH
        settings.beginGroup( connection );
        QgsOwsConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), {service.toLower(), connection} );
        settings.remove( QStringLiteral( "http-header" ) );
        settings.endGroup();
        Q_NOWARN_DEPRECATED_POP

        QgsOwsConnection::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/connections/%1/%2/username" ).arg( service, connection ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/connections/%1/%2/password" ).arg( service, connection ), {service.toLower(), connection} );
        QgsOwsConnection::settingsAuthCfg->copyValueFromKey( QStringLiteral( "qgis/connections/%1/%2/authcfg" ).arg( service, connection ), {service.toLower(), connection} );
      }
      if ( settings.contains( QStringLiteral( "selected" ) ) )
        QgsOwsConnection::sTreeOwsConnections->setSelectedItem( settings.value( QStringLiteral( "selected" ) ).toString(), {service.toLower()} );
    }

    // Vector tile - added in 3.30
    {
      QgsSettings settings;
      settings.beginGroup( QStringLiteral( "qgis/connections-vector-tile" ) );
      const QStringList connections = settings.childGroups();
      for ( const QString &connection : connections )
      {
        QgsVectorTileProviderConnection::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/url" ), {connection} );
        QgsVectorTileProviderConnection::settingsZmin->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmin" ), {connection} );
        QgsVectorTileProviderConnection::settingsZmax->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmax" ), {connection} );
        QgsVectorTileProviderConnection::settingsAuthcfg->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/authcfg" ), {connection} );
        QgsVectorTileProviderConnection::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/username" ), {connection} );
        QgsVectorTileProviderConnection::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/password" ), {connection} );
        QgsVectorTileProviderConnection::settingsStyleUrl->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/styleUrl" ), {connection} );
        QgsVectorTileProviderConnection::settingsServiceType->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/serviceType" ), {connection} );
        Q_NOWARN_DEPRECATED_PUSH
        settings.beginGroup( connection );
        QgsVectorTileProviderConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
        settings.remove( QStringLiteral( "http-header" ) );
        settings.endGroup();
        Q_NOWARN_DEPRECATED_POP
      }
    }

    // xyz - added in 3.30
    {
      QgsSettings settings;
      settings.beginGroup( QStringLiteral( "qgis/connections-xyz" ) );
      const QStringList connections = settings.childGroups();
      for ( const QString &connection : connections )
      {
        QgsXyzConnectionSettings::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/url" ), {connection} );
        QgsXyzConnectionSettings::settingsZmin->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/zmin" ), {connection} );
        QgsXyzConnectionSettings::settingsZmax->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/zmax" ), {connection} );
        QgsXyzConnectionSettings::settingsAuthcfg->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/authcfg" ), {connection} );
        QgsXyzConnectionSettings::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/username" ), {connection} );
        QgsXyzConnectionSettings::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/password" ), {connection} );
        QgsXyzConnectionSettings::settingsTilePixelRatio->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/tilePixelRatio" ), {connection} );
        QgsXyzConnectionSettings::settingsHidden->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/hidden" ), {connection} );
        QgsXyzConnectionSettings::settingsInterpretation->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/interpretation" ), {connection} );
        Q_NOWARN_DEPRECATED_PUSH
        settings.beginGroup( connection );
        QgsXyzConnectionSettings::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
        settings.remove( QStringLiteral( "http-header" ) );
        settings.endGroup();
        Q_NOWARN_DEPRECATED_POP
      }
    }

    // arcgis - added in 3.30
    {
      // arcgismapserver entries are not used anymore (even in 3.28, only arcgisfeature server is used)
      const QStringList serviceKeys = {QStringLiteral( "qgis/connections-arcgisfeatureserver" ), QStringLiteral( "qgis/connections-arcgismapserver" )};
      QgsSettings settings;
      for ( const QString &serviceKey : serviceKeys )
      {
        settings.beginGroup( serviceKey );
        const QStringList connections = settings.childGroups();
        for ( const QString &connection : connections )
        {
          QgsArcGisConnectionSettings::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-arcgisfeatureserver/%1/url" ), {connection} );
          QgsArcGisConnectionSettings::settingsAuthcfg->copyValueFromKey( QStringLiteral( "qgis/ARCGISFEATURESERVER/%1/authcfg" ), {connection} );
          QgsArcGisConnectionSettings::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/ARCGISFEATURESERVER/%1/username" ), {connection} );
          QgsArcGisConnectionSettings::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/ARCGISFEATURESERVER/%1/password" ), {connection} );
          QgsArcGisConnectionSettings::settingsContentEndpoint->copyValueFromKey( QStringLiteral( "qgis/connections-arcgisfeatureserver/%1/content_endpoint" ), {connection} );
          QgsArcGisConnectionSettings::settingsCommunityEndpoint->copyValueFromKey( QStringLiteral( "qgis/connections-arcgisfeatureserver/%1/community_endpoint" ), {connection} );
          Q_NOWARN_DEPRECATED_PUSH
          settings.beginGroup( connection );
          QgsArcGisConnectionSettings::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
          settings.remove( QStringLiteral( "http-header" ) );
          settings.endGroup();
          Q_NOWARN_DEPRECATED_POP
        }
        settings.remove( serviceKey );
      }
    }

    // babel devices settings - added in 3.30
    {
      if ( QgsBabelFormatRegistry::sTreeBabelDevices->items().count() == 0 )
      {
        const QStringList deviceNames = QgsSettings().value( QStringLiteral( "/Plugin-GPS/devices/deviceList" ) ).toStringList();

        for ( const QString &device : deviceNames )
        {
          QgsBabelFormatRegistry::settingsBabelWptDownload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/wptdownload" ), {device} );
          QgsBabelFormatRegistry::settingsBabelWptUpload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/wptupload" ), {device} );
          QgsBabelFormatRegistry::settingsBabelRteDownload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/rtedownload" ), {device} );
          QgsBabelFormatRegistry::settingsBabelRteUpload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/rteupload" ), {device} );
          QgsBabelFormatRegistry::settingsBabelTrkDownload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/trkdownload" ), {device} );
          QgsBabelFormatRegistry::settingsBabelTrkUpload->copyValueFromKey( QStringLiteral( "/Plugin-GPS/devices/%1/trkupload" ), {device} );
        }
      }
    }

    settingsSettingsPre330Migrated->setValue( true );
  }
}

// TODO QGIS 4.0: Remove
void QgsSettingsRegistryCore::backwardCompatibility()
{}

