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

#include "pal.h"
#include "qgis.h"
#include "qgsbabelformatregistry.h"
#include "qgsgpsdetector.h"
#include "qgslayout.h"
#include "qgslocator.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsowsconnection.h"
#include "qgsprocessing.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsproxy.h"
#include "qgsvectorlayer.h"
#include "qgsvectortileconnection.h"

#include <QThread>

const QgsSettingsEntryEnumFlag<Qgis::SnappingMode> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapMode = new QgsSettingsEntryEnumFlag<Qgis::SnappingMode>( u"default-snap-mode"_s, QgsSettingsTree::sTreeDigitizing, Qgis::SnappingMode::AllLayers );

const QgsSettingsEntryEnumFlag<Qgis::SnappingType> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType = new QgsSettingsEntryEnumFlag<Qgis::SnappingType>( u"default-snap-type"_s, QgsSettingsTree::sTreeDigitizing, Qgis::SnappingType::Vertex );

const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit = new QgsSettingsEntryEnumFlag<Qgis::MapToolUnit>( u"default-snapping-tolerance-unit"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_SNAP_UNITS );

const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit = new QgsSettingsEntryEnumFlag<Qgis::MapToolUnit>( u"search-radius-vertex-edit-unit"_s, QgsSettingsTree::sTreeDigitizing, Qgis::MapToolUnit::Pixels );

const QgsSettingsEntryEnumFlag<Qgis::JoinStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle = new QgsSettingsEntryEnumFlag<Qgis::JoinStyle>( u"offset-join-style"_s, QgsSettingsTree::sTreeDigitizing, Qgis::JoinStyle::Round );

const QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle = new QgsSettingsEntryEnumFlag<Qgis::EndCapStyle>( u"offset-cap-style"_s, QgsSettingsTree::sTreeDigitizing,  Qgis::EndCapStyle::Round );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingStreamTolerance = new QgsSettingsEntryInteger( u"stream-tolerance"_s, QgsSettingsTree::sTreeDigitizing, 2 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingLineWidth = new QgsSettingsEntryInteger( u"line-width"_s, QgsSettingsTree::sTreeDigitizing, 1 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingLineColor = new QgsSettingsEntryColor( u"line-color"_s, QgsSettingsTree::sTreeDigitizing, QColor( 255, 0, 0, 200 ) );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale = new QgsSettingsEntryDouble( u"line-color-alpha-scale"_s, QgsSettingsTree::sTreeDigitizing, 0.75 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingFillColor = new QgsSettingsEntryColor( u"fill-color"_s, QgsSettingsTree::sTreeDigitizing, QColor( 255, 0, 0, 30 ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingLineGhost = new QgsSettingsEntryBool( u"line-ghost"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultZValue = new QgsSettingsEntryDouble( u"default-z-value"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_Z_COORDINATE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultMValue = new QgsSettingsEntryDouble( u"default-m-value"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_M_COORDINATE );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled = new QgsSettingsEntryBool( u"default-snap-enabled"_s, QgsSettingsTree::sTreeDigitizing,  false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance = new QgsSettingsEntryDouble( u"default-snapping-tolerance"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_SNAP_TOLERANCE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit = new QgsSettingsEntryDouble( u"search-radius-vertex-edit"_s, QgsSettingsTree::sTreeDigitizing, 10 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingSnapColor = new QgsSettingsEntryColor( u"snap-color"_s, QgsSettingsTree::sTreeDigitizing, QColor( Qt::magenta ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapTooltip = new QgsSettingsEntryBool( u"snap-tooltip"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapInvisibleFeature = new QgsSettingsEntryBool( u"snap-invisible-feature"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected = new QgsSettingsEntryBool( u"marker-only-for-selected"_s, QgsSettingsTree::sTreeDigitizing, true );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsDigitizingMarkerStyle = new QgsSettingsEntryString( u"marker-style"_s, QgsSettingsTree::sTreeDigitizing, "Cross" );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm = new QgsSettingsEntryDouble( u"marker-size-mm"_s, QgsSettingsTree::sTreeDigitizing, 2.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingReuseLastValues = new QgsSettingsEntryBool( u"reuse-last-values"_s, QgsSettingsTree::sTreeDigitizing, false );


const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog = new QgsSettingsEntryBool( u"disable-enter-attribute-values-dialog"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingValidateGeometries = new QgsSettingsEntryInteger( u"validate-geometries"_s, QgsSettingsTree::sTreeDigitizing, 1 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg = new QgsSettingsEntryInteger( u"offset-quad-seg"_s, QgsSettingsTree::sTreeDigitizing, 8 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit = new QgsSettingsEntryDouble( u"offset-miter-limit"_s, QgsSettingsTree::sTreeDigitizing, 5.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingConvertToCurve = new QgsSettingsEntryBool( u"convert-to-curve"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance = new QgsSettingsEntryDouble( u"convert-to-curve-angle-tolerance"_s, QgsSettingsTree::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance = new QgsSettingsEntryDouble( u"convert-to-curve-distance-tolerance"_s, QgsSettingsTree::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingOffsetShowAdvanced = new QgsSettingsEntryBool( u"offset-show-advanced"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingTracingMaxFeatureCount = new QgsSettingsEntryInteger( u"tracing-max-feature-count"_s, QgsSettingsTree::sTreeDigitizing, 10000 );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsGpsBabelPath = new QgsSettingsEntryString( u"gpsbabelPath"_s, QgsSettingsTree::sTreeGps, u"gpsbabel"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerTreeShowFeatureCountForNewLayers = new QgsSettingsEntryBool( u"show-feature-count-for-new-layers"_s, QgsSettingsTree::sTreeLayerTree, false, u"If true, feature counts will be shown in the layer tree for all newly added layers."_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsEnableWMSTilePrefetching = new QgsSettingsEntryBool( u"enable_wms_tile_prefetch"_s, QgsSettingsTree::sTreeWms, false, u"Whether to include WMS layers when rendering tiles adjacent to the visible map area"_s );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsMapScales = new QgsSettingsEntryStringList( u"default_scales"_s, QgsSettingsTree::sTreeMap, Qgis::defaultProjectScales().split( ',' ) );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsLayerParallelLoadingMaxCount = new QgsSettingsEntryInteger( u"provider-parallel-loading-max-count"_s, QgsSettingsTree::sTreeCore, QThread::idealThreadCount(), u"Maximum thread used to load layers in parallel"_s, Qgis::SettingsOption(), 1 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerParallelLoading = new QgsSettingsEntryBool( u"provider-parallel-loading"_s, QgsSettingsTree::sTreeCore, true, u"Load layers in parallel (only available for some providers (GDAL and PostgreSQL)"_s, Qgis::SettingsOption() );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsNetworkCacheDirectory = new QgsSettingsEntryString( u"directory"_s, QgsSettingsTree::sTreeNetworkCache, QString(), u"Network disk cache directory"_s );

const QgsSettingsEntryInteger64 *QgsSettingsRegistryCore::settingsNetworkCacheSize = new QgsSettingsEntryInteger64( u"size-bytes"_s, QgsSettingsTree::sTreeNetworkCache, 0, u"Network disk cache size in bytes (0 = automatic size)"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsAutosizeAttributeTable = new QgsSettingsEntryBool( u"autosize-attribute-table"_s, QgsSettingsTree::sTreeAttributeTable, false );

const QgsSettingsEntryEnumFlag<Qgis::EmbeddedScriptMode> *QgsSettingsRegistryCore::settingsCodeExecutionBehaviorUndeterminedProjects = new QgsSettingsEntryEnumFlag<Qgis::EmbeddedScriptMode>( u"code-execution-behavior-undetermined-projects"_s, QgsSettingsTree::sTreeCore, Qgis::EmbeddedScriptMode::Ask, u"Behavior for embedded scripts within projects of undetermined trust"_s );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders = new QgsSettingsEntryStringList( u"code-execution-trusted-projects-folders"_s, QgsSettingsTree::sTreeCore, QStringList(), u"Projects and folders that are trusted and allowed execution of embedded scripts"_s );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsCodeExecutionUntrustedProjectsFolders = new QgsSettingsEntryStringList( u"code-execution-denied-projects-folders"_s, QgsSettingsTree::sTreeCore, QStringList(), u"Projects and folders that are untrusted and denied execution of embedded scripts"_s );

QgsSettingsRegistryCore::QgsSettingsRegistryCore()
  : QgsSettingsRegistry()
{
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
}

void QgsSettingsRegistryCore::migrateOldSettings()
{
  // This method triggers a ton of QgsSettings constructions and destructions, which is very expensive
  // as it involves writing new values to the underlying ini files.
  // Accordingly we place a hold on constructing new QgsSettings objects for the duration of the method,
  // so that only a single QgsSettings object is created and destroyed at the end of this method.
  QgsSettings::holdFlush();

  auto settings = QgsSettings::get();

  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // single settings - added in 3.30
  QgsLayout::settingsSearchPathForTemplates->copyValueFromKey( u"core/Layout/searchPathsForTemplates"_s );

  QgsProcessing::settingsPreferFilenameAsLayerName->copyValueFromKey( u"Processing/Configuration/PREFER_FILENAME_AS_LAYER_NAME"_s );
  QgsProcessing::settingsTempPath->copyValueFromKey( u"Processing/Configuration/TEMP_PATH2"_s );

  QgsNetworkAccessManager::settingsNetworkTimeout->copyValueFromKey( u"qgis/networkAndProxy/networkTimeout"_s );

  settingsLayerTreeShowFeatureCountForNewLayers->copyValueFromKey( u"core/layer-tree/show_feature_count_for_new_layers"_s );

#if defined( HAVE_QTSERIALPORT )
  QgsGpsDetector::settingsGpsStopBits->copyValueFromKey( u"core/gps/stop_bits"_s );
  QgsGpsDetector::settingsGpsFlowControl->copyValueFromKey( u"core/gps/flow_control"_s );
  QgsGpsDetector::settingsGpsDataBits->copyValueFromKey( u"core/gps/data_bits"_s );
  QgsGpsDetector::settingsGpsParity->copyValueFromKey( u"core/gps/parity"_s );
#endif

  QgsRasterLayer::settingsRasterDefaultOversampling->copyValueFromKey( u"Raster/defaultOversampling"_s, true );
  QgsRasterLayer::settingsRasterDefaultEarlyResampling->copyValueFromKey( u"Raster/defaultEarlyResampling"_s, true );

  pal::Pal::settingsRenderingLabelCandidatesLimitPoints->copyValueFromKey( u"core/rendering/label_candidates_limit_points"_s, true );
  pal::Pal::settingsRenderingLabelCandidatesLimitLines->copyValueFromKey( u"core/rendering/label_candidates_limit_lines"_s, true );
  pal::Pal::settingsRenderingLabelCandidatesLimitPolygons->copyValueFromKey( u"core/rendering/label_candidates_limit_polygons"_s, true );

  // handle bad migration - Fix profiles for old QGIS versions (restore the Map/scales key on windows)
  // TODO: Remove this from QGIS 3.36
  // PR Link: https://github.com/qgis/QGIS/pull/52580
  if ( settings->contains( u"Map/scales"_s ) )
  {
    const QStringList oldScales = settings->value( u"Map/scales"_s ).toStringList();
    if ( ! oldScales.isEmpty() && !oldScales.at( 0 ).isEmpty() )
      settings->setValue( u"Map/scales"_s, oldScales.join( ',' ) );
    else
      settings->setValue( u"Map/scales"_s, Qgis::defaultProjectScales() );
  }

  // migrate only one way for map scales
  if ( !settingsMapScales->exists() )
  {
    // Handle bad migration. Prefer map/scales over Map/scales
    // TODO: Discard this part starting from QGIS 3.36
    const QStringList oldScales = settings->value( u"map/scales"_s ).toStringList();
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
    else if ( settings->contains( u"Map/scales"_s ) )
    {
      settingsMapScales->setValue( settings->value( u"Map/scales"_s ).toString().split( ',' ) );
    }
  }

  // digitizing settings - added in 3.30
  {
    settingsDigitizingLineColor->copyValueFromKeys( u"qgis/digitizing/line_color_red"_s, u"qgis/digitizing/line_color_green"_s, u"qgis/digitizing/line_color_blue"_s, u"qgis/digitizing/line_color_alpha"_s );
    settingsDigitizingFillColor->copyValueFromKeys( u"qgis/digitizing/fill_color_red"_s, u"qgis/digitizing/fill_color_green"_s, u"qgis/digitizing/fill_color_blue"_s, u"qgis/digitizing/fill_color_alpha"_s );

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
        name = u"reuseLastValues"_s;
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
    settings->beginGroup( u"gui/locator_filters"_s );
    const QStringList childKeys = settings->childKeys();
    settings->endGroup();
    for ( const QString &childKey : childKeys )
    {
      if ( childKey.startsWith( "enabled"_L1 ) )
      {
        QString filter = childKey;
        filter.remove( u"enabled_"_s );
        QgsLocator::settingsLocatorFilterEnabled->copyValueFromKey( u"gui/locator_filters/enabled_%1"_s, {filter}, true );
        QgsLocator::settingsLocatorFilterDefault->copyValueFromKey( u"gui/locator_filters/default_%1"_s, {filter}, true );
        QgsLocator::settingsLocatorFilterPrefix->copyValueFromKey( u"gui/locator_filters/prefix_%1"_s, {filter}, true );
      }
    }
  }

  // connections settings - added in 3.30
  const QStringList services = {u"WMS"_s, u"WFS"_s, u"WCS"_s};
  for ( const QString &service : services )
  {
    QgsSettings settings;
    settings.beginGroup( u"qgis/connections-%1"_s.arg( service.toLower() ) );
    const QStringList connections = settings.childGroups();
    if ( connections.count() == 0 )
      continue;
    for ( const QString &connection : connections )
    {
      if ( settings.value( u"%1/url"_s.arg( connection ) ).toString().isEmpty() )
        continue;

      QgsOwsConnection::settingsUrl->copyValueFromKey( u"qgis/connections-%1/%2/url"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsVersion->copyValueFromKey( u"qgis/connections-%1/%2/version"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsIgnoreGetMapURI->copyValueFromKey( u"qgis/connections-%1/%2/ignoreGetMapURI"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->copyValueFromKey( u"qgis/connections-%1/%2/ignoreGetFeatureInfoURI"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsSmoothPixmapTransform->copyValueFromKey( u"qgis/connections-%1/%2/smoothPixmapTransform"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsReportedLayerExtents->copyValueFromKey( u"qgis/connections-%1/%2/reportedLayerExtents"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsDpiMode->copyValueFromKey( u"qgis/connections-%1/%2/dpiMode"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsTilePixelRatio->copyValueFromKey( u"qgis/connections-%1/%2/tilePixelRatio"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsMaxNumFeatures->copyValueFromKey( u"qgis/connections-%1/%2/maxnumfeatures"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsPagesize->copyValueFromKey( u"qgis/connections-%1/%2/pagesize"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsPagingEnabled->copyValueFromKey( u"qgis/connections-%1/%2/pagingenabled"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsPreferCoordinatesForWfsT11->copyValueFromKey( u"qgis/connections-%1/%2/preferCoordinatesForWfsT11"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsIgnoreAxisOrientation->copyValueFromKey( u"qgis/connections-%1/%2/ignoreAxisOrientation"_s, {service.toLower(), connection}, true );
      QgsOwsConnection::settingsInvertAxisOrientation->copyValueFromKey( u"qgis/connections-%1/%2/invertAxisOrientation"_s, {service.toLower(), connection}, true );

      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      QgsOwsConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), {service.toLower(), connection} );
      settings.remove( u"http-header"_s );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP

      QgsOwsConnection::settingsUsername->copyValueFromKey( u"qgis/connections/%1/%2/username"_s.arg( service, connection ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsPassword->copyValueFromKey( u"qgis/connections/%1/%2/password"_s.arg( service, connection ), {service.toLower(), connection}, true );
      QgsOwsConnection::settingsAuthCfg->copyValueFromKey( u"qgis/connections/%1/%2/authcfg"_s.arg( service, connection ), {service.toLower(), connection}, true );
    }
    if ( settings.contains( u"selected"_s ) )
      QgsOwsConnection::sTreeOwsConnections->setSelectedItem( settings.value( u"selected"_s ).toString(), {service.toLower()} );
  }

  // Vector tile - added in 3.30
  {
    QgsSettings settings;
    settings.beginGroup( u"qgis/connections-vector-tile"_s );
    const QStringList connections = settings.childGroups();
    for ( const QString &connection : connections )
    {
      if ( settings.value( u"%1/url"_s.arg( connection ) ).toString().isEmpty() )
        continue;

      QgsVectorTileProviderConnection::settingsUrl->copyValueFromKey( u"qgis/connections-vector-tile/%1/url"_s, {connection}, true );
      QgsVectorTileProviderConnection::settingsZmin->copyValueFromKey( u"qgis/connections-vector-tile/%1/zmin"_s, {connection}, true );
      QgsVectorTileProviderConnection::settingsZmax->copyValueFromKey( u"qgis/connections-vector-tile/%1/zmax"_s, {connection}, true );
      QgsVectorTileProviderConnection::settingsAuthcfg->copyValueFromKey( u"qgis/connections-vector-tile/%1/authcfg"_s, {connection}, true );
      QgsVectorTileProviderConnection::settingsUsername->copyValueFromKey( u"qgis/connections-vector-tile/%1/username"_s, {connection}, true );
      QgsVectorTileProviderConnection::settingsPassword->copyValueFromKey( u"qgis/connections-vector-tile/%1/password"_s, {connection}, true );
      QgsVectorTileProviderConnection::settingsStyleUrl->copyValueFromKey( u"qgis/connections-vector-tile/%1/styleUrl"_s, {connection}, true );
      QgsVectorTileProviderConnection::settingsServiceType->copyValueFromKey( u"qgis/connections-vector-tile/%1/serviceType"_s, {connection}, true );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      QgsVectorTileProviderConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
      settings.remove( u"http-header"_s );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP
    }
  }

  // xyz - added in 3.30
  {
    QgsSettings settings;
    settings.beginGroup( u"qgis/connections-xyz"_s );
    const QStringList connections = settings.childGroups();
    for ( const QString &connection : connections )
    {
      if ( settings.value( u"%1/url"_s.arg( connection ) ).toString().isEmpty() )
        continue;

      QgsXyzConnectionSettings::settingsUrl->copyValueFromKey( u"qgis/connections-xyz/%1/url"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsZmin->copyValueFromKey( u"qgis/connections-xyz/%1/zmin"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsZmax->copyValueFromKey( u"qgis/connections-xyz/%1/zmax"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsAuthcfg->copyValueFromKey( u"qgis/connections-xyz/%1/authcfg"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsUsername->copyValueFromKey( u"qgis/connections-xyz/%1/username"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsPassword->copyValueFromKey( u"qgis/connections-xyz/%1/password"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsTilePixelRatio->copyValueFromKey( u"qgis/connections-xyz/%1/tilePixelRatio"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsHidden->copyValueFromKey( u"qgis/connections-xyz/%1/hidden"_s, {connection}, true );
      QgsXyzConnectionSettings::settingsInterpretation->copyValueFromKey( u"qgis/connections-xyz/%1/interpretation"_s, {connection}, true );
      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      QgsXyzConnectionSettings::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
      settings.remove( u"http-header"_s );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP
    }
  }

  // arcgis - added in 3.30
  {
    // arcgismapserver entries are not used anymore (even in 3.28, only arcgisfeature server is used)
    const QStringList serviceKeys = {u"qgis/connections-arcgisfeatureserver"_s, u"qgis/connections-arcgismapserver"_s};
    QgsSettings settings;
    for ( const QString &serviceKey : serviceKeys )
    {
      settings.beginGroup( serviceKey );
      const QStringList connections = settings.childGroups();
      for ( const QString &connection : connections )
      {
        QgsArcGisConnectionSettings::settingsUrl->copyValueFromKey( u"qgis/connections-arcgisfeatureserver/%1/url"_s, {connection}, true );
        QgsArcGisConnectionSettings::settingsAuthcfg->copyValueFromKey( u"qgis/ARCGISFEATURESERVER/%1/authcfg"_s, {connection}, true );
        QgsArcGisConnectionSettings::settingsUsername->copyValueFromKey( u"qgis/ARCGISFEATURESERVER/%1/username"_s, {connection}, true );
        QgsArcGisConnectionSettings::settingsPassword->copyValueFromKey( u"qgis/ARCGISFEATURESERVER/%1/password"_s, {connection}, true );
        QgsArcGisConnectionSettings::settingsContentEndpoint->copyValueFromKey( u"qgis/connections-arcgisfeatureserver/%1/content_endpoint"_s, {connection}, true );
        QgsArcGisConnectionSettings::settingsCommunityEndpoint->copyValueFromKey( u"qgis/connections-arcgisfeatureserver/%1/community_endpoint"_s, {connection}, true );
        Q_NOWARN_DEPRECATED_PUSH
        settings.beginGroup( connection );
        QgsArcGisConnectionSettings::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), connection );
        settings.remove( u"http-header"_s );
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
      const QStringList deviceNames = settings->value( u"/Plugin-GPS/devices/deviceList"_s ).toStringList();

      for ( const QString &device : deviceNames )
      {
        QgsBabelFormatRegistry::settingsBabelWptDownload->copyValueFromKey( u"/Plugin-GPS/devices/%1/wptdownload"_s, {device}, true );
        QgsBabelFormatRegistry::settingsBabelWptUpload->copyValueFromKey( u"/Plugin-GPS/devices/%1/wptupload"_s, {device}, true );
        QgsBabelFormatRegistry::settingsBabelRteDownload->copyValueFromKey( u"/Plugin-GPS/devices/%1/rtedownload"_s, {device}, true );
        QgsBabelFormatRegistry::settingsBabelRteUpload->copyValueFromKey( u"/Plugin-GPS/devices/%1/rteupload"_s, {device}, true );
        QgsBabelFormatRegistry::settingsBabelTrkDownload->copyValueFromKey( u"/Plugin-GPS/devices/%1/trkdownload"_s, {device}, true );
        QgsBabelFormatRegistry::settingsBabelTrkUpload->copyValueFromKey( u"/Plugin-GPS/devices/%1/trkupload"_s, {device}, true );
      }
    }
  }

  QgsSettings::releaseFlush();
}

// TODO QGIS 5.0: Remove
void QgsSettingsRegistryCore::backwardCompatibility()
{
  // This method triggers a ton of QgsSettings constructions and destructions, which is very expensive
  // as it involves writing new values to the underlying ini files.
  // Accordingly we place a hold on constructing new QgsSettings objects for the duration of the method,
  // so that only a single QgsSettings object is created and destroyed at the end of this method.
  QgsSettings::holdFlush();
  auto settings = QgsSettings::get();

  // CAREFUL! There's a mix of copyValueToKeyIfChanged and copyValueToKey used here.
  // copyValueToKeyIfChanged should be used if copyValueFromKey did NOT have the removeSettingAtKey argument set to True
  // in migrateOldSettings

  // single settings - added in 3.30
  QgsLayout::settingsSearchPathForTemplates->copyValueToKeyIfChanged( u"core/Layout/searchPathsForTemplates"_s );

  QgsProcessing::settingsPreferFilenameAsLayerName->copyValueToKeyIfChanged( u"Processing/Configuration/PREFER_FILENAME_AS_LAYER_NAME"_s );
  QgsProcessing::settingsTempPath->copyValueToKeyIfChanged( u"Processing/Configuration/TEMP_PATH2"_s );

  QgsNetworkAccessManager::settingsNetworkTimeout->copyValueToKeyIfChanged( u"qgis/networkAndProxy/networkTimeout"_s );

  settingsLayerTreeShowFeatureCountForNewLayers->copyValueToKeyIfChanged( u"core/layer-tree/show_feature_count_for_new_layers"_s );

#if defined( HAVE_QTSERIALPORT )
  QgsGpsDetector::settingsGpsStopBits->copyValueToKeyIfChanged( u"core/gps/stop_bits"_s );
  QgsGpsDetector::settingsGpsFlowControl->copyValueToKeyIfChanged( u"core/gps/flow_control"_s );
  QgsGpsDetector::settingsGpsDataBits->copyValueToKeyIfChanged( u"core/gps/data_bits"_s );
  QgsGpsDetector::settingsGpsParity->copyValueToKeyIfChanged( u"core/gps/parity"_s );
#endif

  QgsRasterLayer::settingsRasterDefaultOversampling->copyValueToKey( u"Raster/defaultOversampling"_s );
  QgsRasterLayer::settingsRasterDefaultEarlyResampling->copyValueToKey( u"Raster/defaultEarlyResampling"_s );

  pal::Pal::settingsRenderingLabelCandidatesLimitPoints->copyValueToKey( u"core/rendering/label_candidates_limit_points"_s );
  pal::Pal::settingsRenderingLabelCandidatesLimitLines->copyValueToKey( u"core/rendering/label_candidates_limit_lines"_s );
  pal::Pal::settingsRenderingLabelCandidatesLimitPolygons->copyValueToKey( u"core/rendering/label_candidates_limit_polygons"_s );

  // digitizing settings - added in 3.30
  {
    if ( settingsDigitizingLineColor->hasChanged() )
      settingsDigitizingLineColor->copyValueToKeys( u"qgis/digitizing/line_color_red"_s, u"qgis/digitizing/line_color_green"_s, u"qgis/digitizing/line_color_blue"_s, u"qgis/digitizing/line_color_alpha"_s );
    if ( settingsDigitizingFillColor->hasChanged() )
      settingsDigitizingFillColor->copyValueToKeys( u"qgis/digitizing/fill_color_red"_s, u"qgis/digitizing/fill_color_green"_s, u"qgis/digitizing/fill_color_blue"_s, u"qgis/digitizing/fill_color_alpha"_s );

    const QList<const QgsSettingsEntryBase *> settings = QgsSettingsTree::sTreeDigitizing->childrenSettings();
    for ( const QgsSettingsEntryBase *setting : settings )
    {
      QString name = setting->name();
      if (
        name == settingsDigitizingLineColor->name() ||
        name == settingsDigitizingFillColor->name()
      )
        continue;
      if ( name == settingsDigitizingReuseLastValues->name() )
      {
        name = u"reuseLastValues"_s;
      }
      else
      {
        name.replace( '-', '_' );
      }
      setting->copyValueToKeyIfChanged( QString( "qgis/digitizing/%1" ).arg( name ) );
    }
  }

  // locator filters - added in 3.30
  {
    const QStringList filters = QgsLocator::sTreeLocatorFilters->items();
    for ( const QString &filter : filters )
    {
      QgsLocator::settingsLocatorFilterEnabled->copyValueToKey( u"gui/locator_filters/enabled_%1"_s, {filter} );
      QgsLocator::settingsLocatorFilterDefault->copyValueToKey( u"gui/locator_filters/default_%1"_s, {filter} );
      QgsLocator::settingsLocatorFilterPrefix->copyValueToKey( u"gui/locator_filters/prefix_%1"_s, {filter} );
    }
  }

  // OWS connections settings - added in 3.30
  {
    const QStringList services = {u"WMS"_s, u"WFS"_s, u"WCS"_s};
    for ( const QString &service : services )
    {
      const QStringList connections = QgsOwsConnection::sTreeOwsConnections->items( {service.toLower()} );
      if ( connections.count() == 0 )
        continue;

      for ( const QString &connection : connections )
      {
        QgsOwsConnection::settingsUrl->copyValueToKey( u"qgis/connections-%1/%2/url"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsVersion->copyValueToKey( u"qgis/connections-%1/%2/version"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreGetMapURI->copyValueToKey( u"qgis/connections-%1/%2/ignoreGetMapURI"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->copyValueToKey( u"qgis/connections-%1/%2/ignoreGetFeatureInfoURI"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsSmoothPixmapTransform->copyValueToKey( u"qgis/connections-%1/%2/smoothPixmapTransform"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsReportedLayerExtents->copyValueToKey( u"qgis/connections-%1/%2/reportedLayerExtents"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsDpiMode->copyValueToKey( u"qgis/connections-%1/%2/dpiMode"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsTilePixelRatio->copyValueToKey( u"qgis/connections-%1/%2/tilePixelRatio"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsMaxNumFeatures->copyValueToKey( u"qgis/connections-%1/%2/maxnumfeatures"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsPagesize->copyValueToKey( u"qgis/connections-%1/%2/pagesize"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsPagingEnabled->copyValueToKey( u"qgis/connections-%1/%2/pagingenabled"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsPreferCoordinatesForWfsT11->copyValueToKey( u"qgis/connections-%1/%2/preferCoordinatesForWfsT11"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsIgnoreAxisOrientation->copyValueToKey( u"qgis/connections-%1/%2/ignoreAxisOrientation"_s, {service.toLower(), connection} );
        QgsOwsConnection::settingsInvertAxisOrientation->copyValueToKey( u"qgis/connections-%1/%2/invertAxisOrientation"_s, {service.toLower(), connection} );

        if ( QgsOwsConnection::settingsHeaders->exists( {service.toLower(), connection} ) )
        {
          Q_NOWARN_DEPRECATED_PUSH
          const QgsHttpHeaders headers = QgsHttpHeaders( QgsOwsConnection::settingsHeaders->value( {service.toLower(), connection} ) );
          settings->beginGroup( u"qgis/connections-%1/%2"_s.arg( service.toLower(), connection ) );
          headers.updateSettings( *settings );
          settings->endGroup();
          Q_NOWARN_DEPRECATED_POP
        }

        QgsOwsConnection::settingsUsername->copyValueToKey( u"qgis/connections/%1/%2/username"_s.arg( service, connection ), {service.toLower(), connection} );
        QgsOwsConnection::settingsPassword->copyValueToKey( u"qgis/connections/%1/%2/password"_s.arg( service, connection ), {service.toLower(), connection} );
        QgsOwsConnection::settingsAuthCfg->copyValueToKey( u"qgis/connections/%1/%2/authcfg"_s.arg( service, connection ), {service.toLower(), connection} );
      }
    }
  }

// Vector tile - added in 3.30
  {
    const QStringList connections = QgsVectorTileProviderConnection::sTreeConnectionVectorTile->items();

    for ( const QString &connection : connections )
    {
      // do not overwrite already set setting
      QgsVectorTileProviderConnection::settingsUrl->copyValueToKey( u"qgis/connections-vector-tile/%1/url"_s, {connection} );
      QgsVectorTileProviderConnection::settingsZmin->copyValueToKey( u"qgis/connections-vector-tile/%1/zmin"_s, {connection} );
      QgsVectorTileProviderConnection::settingsZmax->copyValueToKey( u"qgis/connections-vector-tile/%1/zmax"_s, {connection} );
      QgsVectorTileProviderConnection::settingsAuthcfg->copyValueToKey( u"qgis/connections-vector-tile/%1/authcfg"_s, {connection} );
      QgsVectorTileProviderConnection::settingsUsername->copyValueToKey( u"qgis/connections-vector-tile/%1/username"_s, {connection} );
      QgsVectorTileProviderConnection::settingsPassword->copyValueToKey( u"qgis/connections-vector-tile/%1/password"_s, {connection} );
      QgsVectorTileProviderConnection::settingsStyleUrl->copyValueToKey( u"qgis/connections-vector-tile/%1/styleUrl"_s, {connection} );
      QgsVectorTileProviderConnection::settingsServiceType->copyValueToKey( u"qgis/connections-vector-tile/%1/serviceType"_s, {connection} );

      if ( QgsVectorTileProviderConnection::settingsHeaders->exists( connection ) )
      {
        Q_NOWARN_DEPRECATED_PUSH        const QgsHttpHeaders headers = QgsHttpHeaders( QgsVectorTileProviderConnection::settingsHeaders->value( connection ) );
        settings->beginGroup( u"qgis/connections-vector-tile/%1"_s.arg( connection ) );
        headers.updateSettings( *settings );
        settings->endGroup();
        Q_NOWARN_DEPRECATED_POP
      }
    }
  }

  // xyz - added in 3.30
  {
    const QStringList connections = QgsXyzConnectionSettings::sTreeXyzConnections->items();
    for ( const QString &connection : connections )
    {
      QgsXyzConnectionSettings::settingsUrl->copyValueToKey( u"qgis/connections-xyz/%1/url"_s, {connection} );
      QgsXyzConnectionSettings::settingsZmin->copyValueToKey( u"qgis/connections-xyz/%1/zmin"_s, {connection} );
      QgsXyzConnectionSettings::settingsZmax->copyValueToKey( u"qgis/connections-xyz/%1/zmax"_s, {connection} );
      QgsXyzConnectionSettings::settingsAuthcfg->copyValueToKey( u"qgis/connections-xyz/%1/authcfg"_s, {connection} );
      QgsXyzConnectionSettings::settingsUsername->copyValueToKey( u"qgis/connections-xyz/%1/username"_s, {connection} );
      QgsXyzConnectionSettings::settingsPassword->copyValueToKey( u"qgis/connections-xyz/%1/password"_s, {connection} );
      QgsXyzConnectionSettings::settingsTilePixelRatio->copyValueToKey( u"qgis/connections-xyz/%1/tilePixelRatio"_s, {connection} );
      QgsXyzConnectionSettings::settingsHidden->copyValueToKey( u"qgis/connections-xyz/%1/hidden"_s, {connection} );
      QgsXyzConnectionSettings::settingsInterpretation->copyValueToKey( u"qgis/connections-xyz/%1/interpretation"_s, {connection} );

      if ( QgsXyzConnectionSettings::settingsHeaders->exists( connection ) )
      {
        Q_NOWARN_DEPRECATED_PUSH
        const QgsHttpHeaders headers = QgsHttpHeaders( QgsXyzConnectionSettings::settingsHeaders->value( connection ) );
        settings->beginGroup( u"qgis/connections-xyz/%1"_s.arg( connection ) );
        headers.updateSettings( *settings );
        settings->endGroup();
        Q_NOWARN_DEPRECATED_POP
      }
    }
  }

  // Arcgis - added in 3.30
  {
    const QStringList connections = QgsArcGisConnectionSettings::sTreeConnectionArcgis->items();
    for ( const QString &connection : connections )
    {
      // do not overwrite already set setting
      QgsArcGisConnectionSettings::settingsUrl->copyValueToKey( u"qgis/connections-arcgisfeatureserver/%1/url"_s, {connection} );
      QgsArcGisConnectionSettings::settingsAuthcfg->copyValueToKey( u"qgis/ARCGISFEATURESERVER/%1/authcfg"_s, {connection} );
      QgsArcGisConnectionSettings::settingsUsername->copyValueToKey( u"qgis/ARCGISFEATURESERVER/%1/username"_s, {connection} );
      QgsArcGisConnectionSettings::settingsPassword->copyValueToKey( u"qgis/ARCGISFEATURESERVER/%1/password"_s, {connection} );
      QgsArcGisConnectionSettings::settingsContentEndpoint->copyValueToKey( u"qgis/connections-arcgisfeatureserver/%1/content_endpoint"_s, {connection} );
      QgsArcGisConnectionSettings::settingsCommunityEndpoint->copyValueToKey( u"qgis/connections-arcgisfeatureserver/%1/community_endpoint"_s, {connection} );
      if ( QgsArcGisConnectionSettings::settingsHeaders->exists( connection ) )
      {
        if ( QgsArcGisConnectionSettings::settingsHeaders->exists( connection ) )
        {
          Q_NOWARN_DEPRECATED_PUSH
          const QgsHttpHeaders headers = QgsHttpHeaders( QgsArcGisConnectionSettings::settingsHeaders->value( connection ) );
          settings->beginGroup( u"qgis/connections-arcgisfeatureserver/%1"_s.arg( connection ) );
          headers.updateSettings( *settings );
          settings->endGroup();
          Q_NOWARN_DEPRECATED_POP
        }
      }
    }
  }

  // babel devices settings - added in 3.30
  {
    const QStringList devices = QgsBabelFormatRegistry::sTreeBabelDevices->items();
    settings->setValue( u"/Plugin-GPS/devices/deviceList"_s, devices );
    for ( const QString &device : devices )
    {
      QgsBabelFormatRegistry::settingsBabelWptDownload->copyValueToKey( u"/Plugin-GPS/devices/%1/wptdownload"_s, {device} );
      QgsBabelFormatRegistry::settingsBabelWptUpload->copyValueToKey( u"/Plugin-GPS/devices/%1/wptupload"_s, {device} );
      QgsBabelFormatRegistry::settingsBabelRteDownload->copyValueToKey( u"/Plugin-GPS/devices/%1/rtedownload"_s, {device} );
      QgsBabelFormatRegistry::settingsBabelRteUpload->copyValueToKey( u"/Plugin-GPS/devices/%1/rteupload"_s, {device} );
      QgsBabelFormatRegistry::settingsBabelTrkDownload->copyValueToKey( u"/Plugin-GPS/devices/%1/trkdownload"_s, {device} );
      QgsBabelFormatRegistry::settingsBabelTrkUpload->copyValueToKey( u"/Plugin-GPS/devices/%1/trkupload"_s, {device} );
    }
  }

  QgsSettings::releaseFlush();
}

