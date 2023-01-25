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


#include "qgis.h"

#include "qgsbabelformatregistry.h"
#include "qgslayout.h"
#include "qgslocator.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsowsconnection.h"
#include "qgsprocessing.h"
#include "qgsvectortileconnection.h"
#include "qgsgpsdetector.h"


const QgsSettingsEntryEnumFlag<Qgis::SnappingMode> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapMode = new QgsSettingsEntryEnumFlag<Qgis::SnappingMode>( QStringLiteral( "default_snap_mode" ), QgsSettings::sTreeDigitizing, Qgis::SnappingMode::AllLayers );

const QgsSettingsEntryEnumFlag<Qgis::SnappingType> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType = new QgsSettingsEntryEnumFlag<Qgis::SnappingType>( QStringLiteral( "default_snap_type" ), QgsSettings::sTreeDigitizing, Qgis::SnappingType::Vertex );

const QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit = new QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "default_snapping_tolerance_unit" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_SNAP_UNITS );

const QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit = new QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "search_radius_vertex_edit_unit" ), QgsSettings::sTreeDigitizing, QgsTolerance::Pixels );

const QgsSettingsEntryEnumFlag<Qgis::JoinStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle = new QgsSettingsEntryEnumFlag<Qgis::JoinStyle>( QStringLiteral( "offset_join_style" ), QgsSettings::sTreeDigitizing, Qgis::JoinStyle::Round );

const QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle = new QgsSettingsEntryEnumFlag<Qgis::EndCapStyle>( QStringLiteral( "offset_cap_style" ), QgsSettings::sTreeDigitizing,  Qgis::EndCapStyle::Round );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingStreamTolerance = new QgsSettingsEntryInteger( QStringLiteral( "stream_tolerance" ), QgsSettings::sTreeDigitizing, 2 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingLineWidth = new QgsSettingsEntryInteger( QStringLiteral( "line_width" ), QgsSettings::sTreeDigitizing, 1 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingLineColor = new QgsSettingsEntryColor( QStringLiteral( "line-color" ), QgsSettings::sTreeDigitizing, QColor( 255, 0, 0, 200 ) );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale = new QgsSettingsEntryDouble( QStringLiteral( "line_color_alpha_scale" ), QgsSettings::sTreeDigitizing, 0.75 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingFillColor = new QgsSettingsEntryColor( QStringLiteral( "fill-color" ), QgsSettings::sTreeDigitizing, QColor( 255, 0, 0, 30 ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingLineGhost = new QgsSettingsEntryBool( QStringLiteral( "line_ghost" ), QgsSettings::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultZValue = new QgsSettingsEntryDouble( QStringLiteral( "default_z_value" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_Z_COORDINATE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultMValue = new QgsSettingsEntryDouble( QStringLiteral( "default_m_value" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_M_COORDINATE );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled = new QgsSettingsEntryBool( QStringLiteral( "default_snap_enabled" ), QgsSettings::sTreeDigitizing,  false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance = new QgsSettingsEntryDouble( QStringLiteral( "default_snapping_tolerance" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_SNAP_TOLERANCE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit = new QgsSettingsEntryDouble( QStringLiteral( "search_radius_vertex_edit" ), QgsSettings::sTreeDigitizing, 10 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingSnapColor = new QgsSettingsEntryColor( QStringLiteral( "snap_color" ), QgsSettings::sTreeDigitizing, QColor( Qt::magenta ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapTooltip = new QgsSettingsEntryBool( QStringLiteral( "snap_tooltip" ), QgsSettings::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapInvisibleFeature = new QgsSettingsEntryBool( QStringLiteral( "snap_invisible_feature" ), QgsSettings::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected = new QgsSettingsEntryBool( QStringLiteral( "marker_only_for_selected" ), QgsSettings::sTreeDigitizing, true );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsDigitizingMarkerStyle = new QgsSettingsEntryString( QStringLiteral( "marker_style" ), QgsSettings::sTreeDigitizing, "Cross" );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm = new QgsSettingsEntryDouble( QStringLiteral( "marker_size_mm" ), QgsSettings::sTreeDigitizing, 2.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingReuseLastValues = new QgsSettingsEntryBool( QStringLiteral( "reuseLastValues" ), QgsSettings::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog = new QgsSettingsEntryBool( QStringLiteral( "disable_enter_attribute_values_dialog" ), QgsSettings::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingValidateGeometries = new QgsSettingsEntryInteger( QStringLiteral( "validate_geometries" ), QgsSettings::sTreeDigitizing, 1 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg = new QgsSettingsEntryInteger( QStringLiteral( "offset_quad_seg" ), QgsSettings::sTreeDigitizing, 8 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit = new QgsSettingsEntryDouble( QStringLiteral( "offset_miter_limit" ), QgsSettings::sTreeDigitizing, 5.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingConvertToCurve = new QgsSettingsEntryBool( QStringLiteral( "convert_to_curve" ), QgsSettings::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance = new QgsSettingsEntryDouble( QStringLiteral( "convert_to_curve_angle_tolerance" ), QgsSettings::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance = new QgsSettingsEntryDouble( QStringLiteral( "convert_to_curve_distance_tolerance" ), QgsSettings::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingOffsetShowAdvanced = new QgsSettingsEntryBool( QStringLiteral( "offset_show_advanced" ), QgsSettings::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingTracingMaxFeatureCount = new QgsSettingsEntryInteger( QStringLiteral( "tracing_max_feature_count" ), QgsSettings::sTreeDigitizing, 10000 );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsGpsBabelPath = new QgsSettingsEntryString( QStringLiteral( "gpsbabelPath" ), QgsSettings::sTreeGps, QStringLiteral( "gpsbabel" ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerTreeShowFeatureCountForNewLayers = new QgsSettingsEntryBool( QStringLiteral( "show-feature-count-for-new-layers" ), QgsSettings::sTreeLayerTree, false, QStringLiteral( "If true, feature counts will be shown in the layer tree for all newly added layers." ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsEnableWMSTilePrefetching = new QgsSettingsEntryBool( QStringLiteral( "enable_wms_tile_prefetch" ), QgsSettings::sTreeWms, false, QStringLiteral( "Whether to include WMS layers when rendering tiles adjacent to the visible map area" ) );

QgsSettingsRegistryCore::QgsSettingsRegistryCore()
  : QgsSettingsRegistry()
{
  migrateOldSettings();
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
  backwardCompatibility();
}

void QgsSettingsRegistryCore::migrateOldSettings()
{
  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // single settings - added in 3.30
  settingsDigitizingLineColor->copyValueFromKeys( QStringLiteral( "qgis/digitizing/line_color_red" ), QStringLiteral( "qgis/digitizing/line_color_green" ), QStringLiteral( "qgis/digitizing/line_color_blue" ), QStringLiteral( "qgis/digitizing/line_color_alpha" ) );
  settingsDigitizingFillColor->copyValueFromKeys( QStringLiteral( "qgis/digitizing/fill_color_red" ), QStringLiteral( "qgis/digitizing/fill_color_green" ), QStringLiteral( "qgis/digitizing/fill_color_blue" ), QStringLiteral( "qgis/digitizing/fill_color_alpha" ) );
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

  // locator filters - added in 3.30
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "gui/locator_filters" ) );
    const QStringList filters = settings.childGroups();
    for ( const QString &filter : filters )
    {
      QgsLocator::settingsLocatorFilterEnabled->copyValueFromKey( QStringLiteral( "gui/locator_filters/enabled_%1" ), {filter}, true );
      QgsLocator::settingsLocatorFilterDefault->copyValueFromKey( QStringLiteral( "gui/locator_filters/default_%1" ), {filter}, true );
      QgsLocator::settingsLocatorFilterPrefix->copyValueFromKey( QStringLiteral( "gui/locator_filters/prefix_%1" ), {filter}, true );
    }
  }

  // connections settings - added in 3.30
  const QStringList services = {QStringLiteral( "WMS" ), QStringLiteral( "WFS" ), QStringLiteral( "WCS" ), QStringLiteral( "GeoNode" )};
  for ( const QString &service : services )
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "qgis/connections-%1" ).arg( service.toLower() ) );
    const QStringList connections = settings.childGroups();
    if ( connections.count() == 0 )
      continue;
    for ( const QString &connection : connections )
    {
      QgsOwsConnection::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/url" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsVersion->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/version" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsIgnoreGetMapURI->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetMapURI" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetFeatureInfoURI" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsSmoothPixmapTransform->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/smoothPixmapTransform" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsReportedLayerExtents->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/reportedLayerExtents" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsDpiMode->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/dpiMode" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsTilePixelRatio->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/tilePixelRatio" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsMaxNumFeatures->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/maxnumfeatures" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsPagesize->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/pagesize" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsPagingEnabled->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/pagingenabled" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsPreferCoordinatesForWfsT11->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/preferCoordinatesForWfsT11" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsIgnoreAxisOrientation->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/ignoreAxisOrientation" ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsInvertAxisOrientation->copyValueFromKey( QStringLiteral( "qgis/connections-%1/%2/invertAxisOrientation" ), {service.toLower(), connection}, true );

      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( service );
      QgsOwsConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), {service.toLower(), connection} );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP

      QgsOwsConnection::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/connections/%1/%2/username" ), {service, connection}, true );
      QgsOwsConnection::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/connections/%1/%2/password" ), {service, connection}, true );
      QgsOwsConnection::settingsAuthCfg->copyValueFromKey( QStringLiteral( "qgis/connections/%1/%2/authcfg" ), {service, connection}, true );
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
      QgsVectorTileProviderConnection::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/url" ), {connection}, true );
      QgsVectorTileProviderConnection::settingsZmin->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmin" ), {connection}, true );
      QgsVectorTileProviderConnection::settingsZmax->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmax" ), {connection}, true );
      QgsVectorTileProviderConnection::settingsAuthcfg->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/authcfg" ), {connection}, true );
      QgsVectorTileProviderConnection::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/username" ), {connection}, true );
      QgsVectorTileProviderConnection::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/password" ), {connection}, true );
      QgsVectorTileProviderConnection::settingsStyleUrl->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/styleUrl" ), {connection}, true );
      QgsVectorTileProviderConnection::settingsServiceType->copyValueFromKey( QStringLiteral( "qgis/connections-vector-tile/%1/serviceType" ), {connection}, true );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      QgsVectorTileProviderConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
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
      QgsXyzConnectionSettings::settingsUrl->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/url" ), {connection}, true );
      QgsXyzConnectionSettings::settingsZmin->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/zmin" ), {connection}, true );
      QgsXyzConnectionSettings::settingsZmax->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/zmax" ), {connection}, true );
      QgsXyzConnectionSettings::settingsAuthcfg->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/authcfg" ), {connection}, true );
      QgsXyzConnectionSettings::settingsUsername->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/username" ), {connection}, true );
      QgsXyzConnectionSettings::settingsPassword->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/password" ), {connection}, true );
      QgsXyzConnectionSettings::settingsTilePixelRatio->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/tilePixelRatio" ), {connection}, true );
      QgsXyzConnectionSettings::settingsHidden->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/hidden" ), {connection}, true );
      QgsXyzConnectionSettings::settingsInterpretation->copyValueFromKey( QStringLiteral( "qgis/connections-xyz/%1/interpretation" ), {connection}, true );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      QgsXyzConnectionSettings::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
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

// TODO QGIS 4.0: Remove
void QgsSettingsRegistryCore::backwardCompatibility()
{
  // single settings - added in 3.30
  settingsDigitizingLineColor->copyValueToKeys( QStringLiteral( "qgis/digitizing/line_color_red" ), QStringLiteral( "qgis/digitizing/line_color_green" ), QStringLiteral( "qgis/digitizing/line_color_blue" ), QStringLiteral( "qgis/digitizing/line_color_alpha" ) );
  settingsDigitizingFillColor->copyValueToKeys( QStringLiteral( "qgis/digitizing/fill_color_red" ), QStringLiteral( "qgis/digitizing/fill_color_green" ), QStringLiteral( "qgis/digitizing/fill_color_blue" ), QStringLiteral( "qgis/digitizing/fill_color_alpha" ) );
  QgsLayout::settingsSearchPathForTemplates->copyValueToKey( QStringLiteral( "core/Layout/searchPathsForTemplates" ) );

  QgsProcessing::settingsPreferFilenameAsLayerName->copyValueToKey( QStringLiteral( "Processing/Configuration/PREFER_FILENAME_AS_LAYER_NAME" ) );
  QgsProcessing::settingsTempPath->copyValueToKey( QStringLiteral( "Processing/Configuration/TEMP_PATH2" ) );
  QgsProcessing::settingsDefaultOutputVectorLayerExt->copyValueToKey( QStringLiteral( "Processing/Configuration/DefaultOutputVectorLayerExt" ) );
  QgsProcessing::settingsDefaultOutputRasterLayerExt->copyValueToKey( QStringLiteral( "Processing/Configuration/DefaultOutputRasterLayerExt" ) );

  QgsNetworkAccessManager::settingsNetworkTimeout->copyValueToKey( QStringLiteral( "qgis/networkAndProxy/networkTimeout" ) );

  settingsLayerTreeShowFeatureCountForNewLayers->copyValueToKey( QStringLiteral( "core/layer-tree/show_feature_count_for_new_layers" ) );

#if defined( HAVE_QTSERIALPORT )
  QgsGpsDetector::settingsGpsStopBits->copyValueToKey( QStringLiteral( "core/gps/stop_bits" ) );
  QgsGpsDetector::settingsGpsFlowControl->copyValueToKey( QStringLiteral( "core/gps/flow_control" ) );
  QgsGpsDetector::settingsGpsDataBits->copyValueToKey( QStringLiteral( "core/gps/data_bits" ) );
  QgsGpsDetector::settingsGpsParity->copyValueToKey( QStringLiteral( "core/gps/parity" ) );
#endif

  // locator filters - added in 3.30
  {
    const QStringList filters = QgsLocator::sTreeLocatorFilters->items();
    for ( const QString &filter : filters )
    {
      QgsLocator::settingsLocatorFilterEnabled->copyValueToKey( QStringLiteral( "gui/locator_filters/enabled_%1" ), {filter} );
      QgsLocator::settingsLocatorFilterDefault->copyValueToKey( QStringLiteral( "gui/locator_filters/default_%1" ), {filter} );
      QgsLocator::settingsLocatorFilterPrefix->copyValueToKey( QStringLiteral( "gui/locator_filters/prefix_%1" ), {filter} );
    }
  }

  // OWS connections settings - added in 3.30
  {
    const QStringList services = {QStringLiteral( "WMS" ), QStringLiteral( "WFS" ), QStringLiteral( "WCS" ), QStringLiteral( "GeoNode" )};
    for ( const QString &service : services )
    {
      const QStringList connections = QgsOwsConnection::sTreeOwsConnections->items( {service.toLower()} );
      if ( connections.count() == 0 )
        continue;
      QgsSettings settings;
      settings.beginGroup( QStringLiteral( "qgis/connections-%1" ).arg( service.toLower() ) );
      for ( const QString &connection : connections )
      {
        QgsOwsConnection::settingsUrl->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/url" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsVersion->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/version" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreGetMapURI->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetMapURI" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/ignoreGetFeatureInfoURI" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsSmoothPixmapTransform->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/smoothPixmapTransform" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsReportedLayerExtents->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/reportedLayerExtents" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsDpiMode->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/dpiMode" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsTilePixelRatio->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/tilePixelRatio" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsMaxNumFeatures->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/maxnumfeatures" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPagesize->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/pagesize" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPagingEnabled->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/pagingenabled" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPreferCoordinatesForWfsT11->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/preferCoordinatesForWfsT11" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreAxisOrientation->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/ignoreAxisOrientation" ), {service.toLower(), connection} );
        QgsOwsConnection::settingsInvertAxisOrientation->copyValueToKey( QStringLiteral( "qgis/connections-%1/%2/invertAxisOrientation" ), {service.toLower(), connection} );

        Q_NOWARN_DEPRECATED_PUSH
        settings.beginGroup( service );
        if ( QgsOwsConnection::settingsHeaders->exists( connection ) )
          QgsHttpHeaders( QgsOwsConnection::settingsHeaders->value( {service.toLower(), service} ) ).updateSettings( settings );
        settings.endGroup();
        Q_NOWARN_DEPRECATED_POP

        QgsOwsConnection::settingsUsername->copyValueToKey( QStringLiteral( "qgis/connections/%1/%2/username" ), {service, connection} );
        QgsOwsConnection::settingsPassword->copyValueToKey( QStringLiteral( "qgis/connections/%1/%2/password" ), {service, connection} );
        QgsOwsConnection::settingsAuthCfg->copyValueToKey( QStringLiteral( "qgis/connections/%1/%2/authcfg" ), {service, connection} );

        if ( settings.contains( QStringLiteral( "selected" ) ) )
          QgsOwsConnection::sTreeOwsConnections->setSelectedItem( settings.value( QStringLiteral( "selected" ) ).toString(), {service.toLower()} );
      }
    }
  }

  // Vector tile - added in 3.30
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "qgis/connections-vector-tile" ) );

    const QStringList connections = QgsVectorTileProviderConnection::sTreeConnectionVectorTile->items();
    for ( const QString &connection : connections )
    {
      // do not overwrite already set setting
      QgsVectorTileProviderConnection::settingsUrl->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/url" ), {connection} );
      QgsVectorTileProviderConnection::settingsZmin->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmin" ), {connection} );
      QgsVectorTileProviderConnection::settingsZmax->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/zmax" ), {connection} );
      QgsVectorTileProviderConnection::settingsAuthcfg->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/authcfg" ), {connection} );
      QgsVectorTileProviderConnection::settingsUsername->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/username" ), {connection} );
      QgsVectorTileProviderConnection::settingsPassword->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/password" ), {connection} );
      QgsVectorTileProviderConnection::settingsStyleUrl->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/styleUrl" ), {connection} );
      QgsVectorTileProviderConnection::settingsServiceType->copyValueToKey( QStringLiteral( "qgis/connections-vector-tile/%1/serviceType" ), {connection} );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      if ( QgsVectorTileProviderConnection::settingsHeaders->exists( connection ) )
        QgsHttpHeaders( QgsVectorTileProviderConnection::settingsHeaders->value( connection ) ).updateSettings( settings );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP
    }
  }

  // xyz - added in 3.30
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "qgis/connections-xyz" ) );
    const QStringList connections = QgsXyzConnectionSettings::sTreeXyzConnections->items();
    for ( const QString &connection : connections )
    {
      QgsXyzConnectionSettings::settingsUrl->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/url" ), {connection} );
      QgsXyzConnectionSettings::settingsZmin->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/zmin" ), {connection} );
      QgsXyzConnectionSettings::settingsZmax->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/zmax" ), {connection} );
      QgsXyzConnectionSettings::settingsAuthcfg->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/authcfg" ), {connection} );
      QgsXyzConnectionSettings::settingsUsername->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/username" ), {connection} );
      QgsXyzConnectionSettings::settingsPassword->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/password" ), {connection} );
      QgsXyzConnectionSettings::settingsTilePixelRatio->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/tilePixelRatio" ), {connection} );
      QgsXyzConnectionSettings::settingsHidden->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/hidden" ), {connection} );
      QgsXyzConnectionSettings::settingsInterpretation->copyValueToKey( QStringLiteral( "qgis/connections-xyz/%1/interpretation" ), {connection} );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      if ( QgsXyzConnectionSettings::settingsHeaders->exists( connection ) )
        QgsHttpHeaders( QgsXyzConnectionSettings::settingsHeaders->value( connection ) ).updateSettings( settings );
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

