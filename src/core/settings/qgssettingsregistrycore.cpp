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
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsbabelformatregistry.h"
#include "qgscolorscheme.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscptcityarchive.h"
#include "qgsdirectoryitem.h"
#include "qgsfilebaseddataitemprovider.h"
#include "qgsgpsdetector.h"
#include "qgsimagecache.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayout.h"
#include "qgslayoutgridsettings.h"
#include "qgslayoutsnapper.h"
#include "qgslocator.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnewsfeedparser.h"
#include "qgsogrdbconnection.h"
#include "qgsogrproviderutils.h"
#include "qgsowsconnection.h"
#include "qgsprocessing.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrasterminmaxorigin.h"
#include "qgsrasterrendererregistry.h"
#include "qgssettings.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsproxy.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsvectortileconnection.h"

#include <QString>

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

#include <QSettings>
#include <QThread>

using namespace Qt::StringLiterals;

const QgsSettingsEntryEnumFlag<Qgis::SnappingMode> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapMode
  = new QgsSettingsEntryEnumFlag<Qgis::SnappingMode>( u"default-snap-mode"_s, QgsSettingsTree::sTreeDigitizing, Qgis::SnappingMode::AllLayers );

const QgsSettingsEntryEnumFlag<Qgis::SnappingType> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType
  = new QgsSettingsEntryEnumFlag<Qgis::SnappingType>( u"default-snap-type"_s, QgsSettingsTree::sTreeDigitizing, Qgis::SnappingType::Vertex );

const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit
  = new QgsSettingsEntryEnumFlag<Qgis::MapToolUnit>( u"default-snapping-tolerance-unit"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_SNAP_UNITS );

const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit
  = new QgsSettingsEntryEnumFlag<Qgis::MapToolUnit>( u"search-radius-vertex-edit-unit"_s, QgsSettingsTree::sTreeDigitizing, Qgis::MapToolUnit::Pixels );

const QgsSettingsEntryEnumFlag<Qgis::JoinStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle
  = new QgsSettingsEntryEnumFlag<Qgis::JoinStyle>( u"offset-join-style"_s, QgsSettingsTree::sTreeDigitizing, Qgis::JoinStyle::Round );

const QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> *QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle
  = new QgsSettingsEntryEnumFlag<Qgis::EndCapStyle>( u"offset-cap-style"_s, QgsSettingsTree::sTreeDigitizing, Qgis::EndCapStyle::Round );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingStreamTolerance = new QgsSettingsEntryInteger( u"stream-tolerance"_s, QgsSettingsTree::sTreeDigitizing, 2 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingNurbsDegree = new QgsSettingsEntryInteger( u"nurbs-degree"_s, QgsSettingsTree::sTreeDigitizing, 3 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingLineWidth = new QgsSettingsEntryInteger( u"line-width"_s, QgsSettingsTree::sTreeDigitizing, 1 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingLineColor = new QgsSettingsEntryColor( u"line-color"_s, QgsSettingsTree::sTreeDigitizing, QColor( 255, 0, 0, 200 ) );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale = new QgsSettingsEntryDouble( u"line-color-alpha-scale"_s, QgsSettingsTree::sTreeDigitizing, 0.75 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingFillColor = new QgsSettingsEntryColor( u"fill-color"_s, QgsSettingsTree::sTreeDigitizing, QColor( 255, 0, 0, 30 ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingLineGhost = new QgsSettingsEntryBool( u"line-ghost"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultZValue = new QgsSettingsEntryDouble( u"default-z-value"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_Z_COORDINATE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultMValue = new QgsSettingsEntryDouble( u"default-m-value"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_M_COORDINATE );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled = new QgsSettingsEntryBool( u"default-snap-enabled"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance
  = new QgsSettingsEntryDouble( u"default-snapping-tolerance"_s, QgsSettingsTree::sTreeDigitizing, Qgis::DEFAULT_SNAP_TOLERANCE );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit = new QgsSettingsEntryDouble( u"search-radius-vertex-edit"_s, QgsSettingsTree::sTreeDigitizing, 10 );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDigitizingSnapColor = new QgsSettingsEntryColor( u"snap-color"_s, QgsSettingsTree::sTreeDigitizing, QColor( Qt::magenta ) );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapTooltip = new QgsSettingsEntryBool( u"snap-tooltip"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingSnapInvisibleFeature = new QgsSettingsEntryBool( u"snap-invisible-feature"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected = new QgsSettingsEntryBool( u"marker-only-for-selected"_s, QgsSettingsTree::sTreeDigitizing, true );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsDigitizingMarkerStyle = new QgsSettingsEntryString( u"marker-style"_s, QgsSettingsTree::sTreeDigitizing, "Cross" );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm = new QgsSettingsEntryDouble( u"marker-size-mm"_s, QgsSettingsTree::sTreeDigitizing, 2.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingReuseLastValues = new QgsSettingsEntryBool( u"reuse-last-values"_s, QgsSettingsTree::sTreeDigitizing, false );


const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog
  = new QgsSettingsEntryBool( u"disable-enter-attribute-values-dialog"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingValidateGeometries = new QgsSettingsEntryInteger( u"validate-geometries"_s, QgsSettingsTree::sTreeDigitizing, 1 );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg = new QgsSettingsEntryInteger( u"offset-quad-seg"_s, QgsSettingsTree::sTreeDigitizing, 8 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit = new QgsSettingsEntryDouble( u"offset-miter-limit"_s, QgsSettingsTree::sTreeDigitizing, 5.0 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingConvertToCurve = new QgsSettingsEntryBool( u"convert-to-curve"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance
  = new QgsSettingsEntryDouble( u"convert-to-curve-angle-tolerance"_s, QgsSettingsTree::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryDouble *QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance
  = new QgsSettingsEntryDouble( u"convert-to-curve-distance-tolerance"_s, QgsSettingsTree::sTreeDigitizing, 1e-6 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsDigitizingOffsetShowAdvanced = new QgsSettingsEntryBool( u"offset-show-advanced"_s, QgsSettingsTree::sTreeDigitizing, false );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsDigitizingTracingMaxFeatureCount = new QgsSettingsEntryInteger( u"tracing-max-feature-count"_s, QgsSettingsTree::sTreeDigitizing, 10000 );

const QgsSettingsEntryEnumFlag< Qgis::CadMeasurementDisplayType > *QgsSettingsRegistryCore::settingsDigitizingStatusBarAreaDisplay = new QgsSettingsEntryEnumFlag<
  Qgis::CadMeasurementDisplayType>( u"status-bar-area-display"_s, QgsSettingsTree::sTreeDigitizing, Qgis::CadMeasurementDisplayType::Hidden, u"Area measurement to show in status bar while digitizing"_s );

const QgsSettingsEntryEnumFlag< Qgis::CadMeasurementDisplayType > *QgsSettingsRegistryCore::settingsDigitizingStatusBarTotalLengthDisplay = new QgsSettingsEntryEnumFlag<
  Qgis::CadMeasurementDisplayType>( u"status-bar-total-length-display"_s, QgsSettingsTree::sTreeDigitizing, Qgis::CadMeasurementDisplayType::Hidden, u"Total length/perimeter measurement to show in status bar while digitizing"_s );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsGpsBabelPath = new QgsSettingsEntryString( u"gpsbabelPath"_s, QgsSettingsTree::sTreeGps, u"gpsbabel"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerTreeShowFeatureCountForNewLayers
  = new QgsSettingsEntryBool( u"show-feature-count-for-new-layers"_s, QgsSettingsTree::sTreeLayerTree, false, u"If true, feature counts will be shown in the layer tree for all newly added layers."_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerTreeShowLegendClassifiers
  = new QgsSettingsEntryBool( u"show-legend-classifiers"_s, QgsSettingsTree::sTreeLayerTree, false, u"If true, classification attribute name is shown in the legend."_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerTreeShowIdInLayerTooltips
  = new QgsSettingsEntryBool( u"show-id-in-layer-tooltips"_s, QgsSettingsTree::sTreeLayerTree, false, u"If true, layer IDs will be shown in the layer tooltips."_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsEnableWMSTilePrefetching
  = new QgsSettingsEntryBool( u"enable_wms_tile_prefetch"_s, QgsSettingsTree::sTreeWms, false, u"Whether to include WMS layers when rendering tiles adjacent to the visible map area"_s );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsMapScales = new QgsSettingsEntryStringList( u"default_scales"_s, QgsSettingsTree::sTreeMap, Qgis::defaultProjectScales().split( ',' ) );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsLayerParallelLoadingMaxCount
  = new QgsSettingsEntryInteger( u"provider-parallel-loading-max-count"_s, QgsSettingsTree::sTreeCore, QThread::idealThreadCount(), u"Maximum thread used to load layers in parallel"_s, Qgis::SettingsOption(), 1 );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsLayerParallelLoading
  = new QgsSettingsEntryBool( u"provider-parallel-loading"_s, QgsSettingsTree::sTreeCore, true, u"Load layers in parallel (only available for some providers (GDAL and PostgreSQL)"_s, Qgis::SettingsOption() );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsNetworkCacheDirectory
  = new QgsSettingsEntryString( u"directory"_s, QgsSettingsTree::sTreeNetworkCache, QString(), u"Network disk cache directory"_s );

const QgsSettingsEntryInteger64 *QgsSettingsRegistryCore::settingsNetworkCacheSize
  = new QgsSettingsEntryInteger64( u"size-bytes"_s, QgsSettingsTree::sTreeNetworkCache, 0, u"Network disk cache size in bytes (0 = automatic size)"_s );

const QgsSettingsEntryEnumFlag<Qgis::EmbeddedScriptMode> *QgsSettingsRegistryCore::settingsCodeExecutionBehaviorUndeterminedProjects = new QgsSettingsEntryEnumFlag<
  Qgis::EmbeddedScriptMode>( u"code-execution-behavior-undetermined-projects"_s, QgsSettingsTree::sTreeCore, Qgis::EmbeddedScriptMode::Ask, u"Behavior for embedded scripts within projects of undetermined trust"_s );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders
  = new QgsSettingsEntryStringList( u"code-execution-trusted-projects-folders"_s, QgsSettingsTree::sTreeCore, QStringList(), u"Projects and folders that are trusted and allowed execution of embedded scripts"_s );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsCodeExecutionUntrustedProjectsFolders
  = new QgsSettingsEntryStringList( u"code-execution-denied-projects-folders"_s, QgsSettingsTree::sTreeCore, QStringList(), u"Projects and folders that are untrusted and denied execution of embedded scripts"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsMeasurePlanimetric
  = new QgsSettingsEntryBool( u"planimetric"_s, QgsSettingsTree::sTreeMeasure, true, u"Whether measurements should be planimetric (ellipsoid off) or use the ellipsoid"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryCore::settingsMeasureKeepBaseUnit
  = new QgsSettingsEntryBool( u"keep-base-unit"_s, QgsSettingsTree::sTreeMeasure, true, u"Whether to keep base measurement units instead of converting to larger units"_s );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsMeasureDecimalPlaces
  = new QgsSettingsEntryInteger( u"decimal-places"_s, QgsSettingsTree::sTreeMeasure, 3, u"Number of decimal places for measurements"_s );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsMeasureDisplayUnits
  = new QgsSettingsEntryString( u"display-units"_s, QgsSettingsTree::sTreeMeasure, QString(), u"Distance display units (encoded unit string)"_s );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsMeasureAreaUnits
  = new QgsSettingsEntryString( u"area-units"_s, QgsSettingsTree::sTreeMeasure, QString(), u"Area display units (encoded unit string)"_s );

const QgsSettingsEntryEnumFlag<Qgis::UnknownLayerCrsBehavior> *QgsSettingsRegistryCore::settingsUnknownCrsBehavior = new QgsSettingsEntryEnumFlag<
  Qgis::UnknownLayerCrsBehavior>( u"unknown-crs-behavior"_s, QgsSettingsTree::sTreeCrs, Qgis::UnknownLayerCrsBehavior::NoAction, u"Behavior when encountering a layer with an unknown CRS"_s );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsLayerDefaultCrs
  = new QgsSettingsEntryString( u"layer-default-crs"_s, QgsSettingsTree::sTreeCrs, u"EPSG:4326"_s, u"Default CRS used for layers with unknown CRS when the unknown CRS behavior is set to UseDefaultCrs"_s );

const QgsSettingsEntryEnumFlag<Qgis::LayerTreeInsertionMethod> *QgsSettingsRegistryCore::settingsLayerTreeInsertionMethod = new QgsSettingsEntryEnumFlag<
  Qgis::LayerTreeInsertionMethod>( u"insertion-method"_s, QgsSettingsTree::sTreeLayerTree, Qgis::LayerTreeInsertionMethod::AboveInsertionPoint, u"Method for inserting layers into the layer tree"_s );

const QgsSettingsEntryString *QgsSettingsRegistryCore::settingsScanZipInBrowser
  = new QgsSettingsEntryString( u"scan-zip-in-browser"_s, QgsSettingsTree::sTreeQgis, u"basic"_s, u"Zip scanning behavior in browser (no, basic, full)"_s );

const QgsSettingsEntryStringList *QgsSettingsRegistryCore::settingsScanItemsFastScanUris
  = new QgsSettingsEntryStringList( u"scan-items-fast-scan-uris"_s, QgsSettingsTree::sTreeQgis, QStringList(), u"URIs for fast scanning in browser"_s );

const QgsSettingsEntryInteger *QgsSettingsRegistryCore::settingsSymbolsListGroupsIndex
  = new QgsSettingsEntryInteger( u"symbols-list-groups-index"_s, QgsSettingsTree::sTreeQgis, 0, u"Currently selected group index in symbols list"_s );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDefaultCanvasColor
  = new QgsSettingsEntryColor( u"default-canvas-color"_s, QgsSettingsTree::sTreeQgis, QColor( 255, 255, 255 ), u"Default canvas background color"_s );

const QgsSettingsEntryColor *QgsSettingsRegistryCore::settingsDefaultSelectionColor
  = new QgsSettingsEntryColor( u"default-selection-color"_s, QgsSettingsTree::sTreeQgis, QColor( 255, 255, 0, 255 ), u"Default selection color"_s );

QgsSettingsRegistryCore::QgsSettingsRegistryCore()
  : QgsSettingsRegistry()
{}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{}

void QgsSettingsRegistryCore::migrateOldSettings()
{
  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // TODO: remove in QGIS 4.4 (after LTR 4.2)

  // single settings - added in 3.30
  QgsLayout::settingsSearchPathForTemplates->copyValueFromKey( u"core/Layout/searchPathsForTemplates"_s, true );
  QgsLayout::settingsLayoutDefaultFont->copyValueFromKey( u"gui/LayoutDesigner/defaultFont"_s, true );
  QgsLayout::settingsLayoutDefaultNorthArrow->copyValueFromKey( u"gui/LayoutDesigner/defaultNorthArrow"_s, true );

  QgsProcessing::settingsPreferFilenameAsLayerName->copyValueFromKey( u"Processing/Configuration/PREFER_FILENAME_AS_LAYER_NAME"_s, true );
  QgsProcessing::settingsTempPath->copyValueFromKey( u"Processing/Configuration/TEMP_PATH2"_s, true );

  QgsNetworkAccessManager::settingsNetworkTimeout->copyValueFromKey( u"qgis/networkAndProxy/networkTimeout"_s, true );
  QgsNetworkAccessManager::settingsUserAgent->copyValueFromKey( u"qgis/networkAndProxy/userAgent"_s, true );
  QgsNetworkAccessManager::settingsUserAgent->copyValueFromKey( u"/qgis/networkAndProxy/userAgent"_s, true );

  settingsLayerTreeShowFeatureCountForNewLayers->copyValueFromKey( u"core/layer-tree/show_feature_count_for_new_layers"_s, true );
  settingsLayerTreeShowLegendClassifiers->copyValueFromKey( u"qgis/showLegendClassifiers"_s, true );
  settingsLayerTreeShowLegendClassifiers->copyValueFromKey( u"/qgis/showLegendClassifiers"_s, true );

  // single settings - added in 4.2
  // Old code used QgsSettings::Core section, so the actual QSettings key has a "core/" prefix
  settingsMeasurePlanimetric->copyValueFromKey( u"core/measure/planimetric"_s, true );
  settingsMeasureKeepBaseUnit->copyValueFromKey( u"qgis/measure/keepbaseunit"_s, true );
  settingsMeasureKeepBaseUnit->copyValueFromKey( u"/qgis/measure/keepbaseunit"_s, true );
  settingsMeasureDecimalPlaces->copyValueFromKey( u"qgis/measure/decimalplaces"_s, true );
  settingsMeasureDecimalPlaces->copyValueFromKey( u"/qgis/measure/decimalplaces"_s, true );
  settingsMeasureDisplayUnits->copyValueFromKey( u"qgis/measure/displayunits"_s, true );
  settingsMeasureDisplayUnits->copyValueFromKey( u"/qgis/measure/displayunits"_s, true );
  settingsMeasureAreaUnits->copyValueFromKey( u"qgis/measure/areaunits"_s, true );
  settingsMeasureAreaUnits->copyValueFromKey( u"/qgis/measure/areaunits"_s, true );
  settingsLayerTreeInsertionMethod->copyValueFromKey( u"qgis/layerTreeInsertionMethod"_s, true );
  settingsLayerTreeInsertionMethod->copyValueFromKey( u"/qgis/layerTreeInsertionMethod"_s, true );
  settingsScanZipInBrowser->copyValueFromKey( u"qgis/scanZipInBrowser2"_s, true );
  settingsScanZipInBrowser->copyValueFromKey( u"/qgis/scanZipInBrowser2"_s, true );
  QgsProject::settingsAnonymizeNewProjects->copyValueFromKey( u"core/projects/anonymize_new_projects"_s, true );
  QgsProject::settingsAnonymizeSavedProjects->copyValueFromKey( u"core/projects/anonymize_saved_projects"_s, true );
  QgsProject::settingsDefaultProjectPathsRelative->copyValueFromKey( u"qgis/defaultProjectPathsRelative"_s, true );
  QgsProject::settingsDefaultProjectPathsRelative->copyValueFromKey( u"/qgis/defaultProjectPathsRelative"_s, true );
  // old key was stored under QgsSettings::App, i.e. "app/projections/unknownCrsBehavior"
  settingsUnknownCrsBehavior->copyValueFromKey( u"app/projections/unknownCrsBehavior"_s, true );
  settingsLayerDefaultCrs->copyValueFromKey( u"Projections/layerDefaultCrs"_s, true );
  settingsLayerDefaultCrs->copyValueFromKey( u"/Projections/layerDefaultCrs"_s, true );
  QgsApplication::settingsApplicationFullName->copyValueFromKey( u"qgis/application_full_name"_s, true );
  QgsApplication::settingsApplicationFullName->copyValueFromKey( u"/qgis/application_full_name"_s, true );

  // gdal/skipDrivers was a comma-joined string; convert to a proper QStringList
  {
    QgsSettings s;
    if ( s.contains( u"gdal/skipDrivers"_s ) )
    {
      const QString joined = s.value( u"gdal/skipDrivers"_s ).toString();
      QgsApplication::settingsSkippedGdalDrivers->setValue( joined.isEmpty() ? QStringList() : joined.split( ','_L1 ) );
      s.remove( u"gdal/skipDrivers"_s );
    }
  }

  QgsDirectoryParamWidget::settingsDirectoryHiddenColumns->copyValueFromKey( u"dataitem/directoryHiddenColumns"_s, true );
  QgsDirectoryParamWidget::settingsDirectoryHiddenColumns->copyValueFromKey( u"/dataitem/directoryHiddenColumns"_s, true );
  QgsDirectoryItem::settingsMonitorDirectoriesInBrowser->copyValueFromKey( u"qgis/monitorDirectoriesInBrowser"_s, true );
  QgsDirectoryItem::settingsMonitorDirectoriesInBrowser->copyValueFromKey( u"/qgis/monitorDirectoriesInBrowser"_s, true );
  QgsFileBasedDataItemProvider::settingsScanItemsInBrowser->copyValueFromKey( u"qgis/scanItemsInBrowser2"_s, {}, true );
  QgsFileBasedDataItemProvider::settingsScanItemsInBrowser->copyValueFromKey( u"/qgis/scanItemsInBrowser2"_s, {}, true );
  settingsScanItemsFastScanUris->copyValueFromKey( u"qgis/scanItemsFastScanUris"_s, true );
  settingsScanItemsFastScanUris->copyValueFromKey( u"/qgis/scanItemsFastScanUris"_s, true );
  settingsSymbolsListGroupsIndex->copyValueFromKey( u"qgis/symbolsListGroupsIndex"_s, true );
  settingsSymbolsListGroupsIndex->copyValueFromKey( u"/qgis/symbolsListGroupsIndex"_s, true );
  settingsDefaultCanvasColor->copyValueFromKeys( u"qgis/default_canvas_color_red"_s, u"qgis/default_canvas_color_green"_s, u"qgis/default_canvas_color_blue"_s, QString(), true );
  settingsDefaultCanvasColor->copyValueFromKeys( u"/qgis/default_canvas_color_red"_s, u"/qgis/default_canvas_color_green"_s, u"/qgis/default_canvas_color_blue"_s, QString(), true );
  settingsDefaultSelectionColor
    ->copyValueFromKeys( u"qgis/default_selection_color_red"_s, u"qgis/default_selection_color_green"_s, u"qgis/default_selection_color_blue"_s, u"qgis/default_selection_color_alpha"_s, true );
  settingsDefaultSelectionColor
    ->copyValueFromKeys( u"/qgis/default_selection_color_red"_s, u"/qgis/default_selection_color_green"_s, u"/qgis/default_selection_color_blue"_s, u"/qgis/default_selection_color_alpha"_s, true );

#if defined( HAVE_QTSERIALPORT )
  QgsGpsDetector::settingsGpsStopBits->copyValueFromKey( u"core/gps/stop_bits"_s, true );
  QgsGpsDetector::settingsGpsFlowControl->copyValueFromKey( u"core/gps/flow_control"_s, true );
  QgsGpsDetector::settingsGpsDataBits->copyValueFromKey( u"core/gps/data_bits"_s, true );
  QgsGpsDetector::settingsGpsParity->copyValueFromKey( u"core/gps/parity"_s, true );
#endif

  QgsRasterLayer::settingsRasterDefaultOversampling->copyValueFromKey( u"Raster/defaultOversampling"_s, true );
  QgsRasterLayer::settingsRasterDefaultEarlyResampling->copyValueFromKey( u"Raster/defaultEarlyResampling"_s, true );
  QgsRasterLayer::settingsRasterDefaultZoomedInResampling->copyValueFromKey( u"Raster/defaultZoomedInResampling"_s, true );
  QgsRasterLayer::settingsRasterDefaultZoomedInResampling->copyValueFromKey( u"/Raster/defaultZoomedInResampling"_s, true );
  QgsRasterLayer::settingsRasterDefaultZoomedOutResampling->copyValueFromKey( u"Raster/defaultZoomedOutResampling"_s, true );
  QgsRasterLayer::settingsRasterDefaultZoomedOutResampling->copyValueFromKey( u"/Raster/defaultZoomedOutResampling"_s, true );
  for ( const QString &rendererKey : { u"singleBand"_s, u"multiBandSingleByte"_s, u"multiBandMultiByte"_s } )
  {
    QgsRasterLayer::settingsRasterDefaultContrastEnhancementAlgorithm->copyValueFromKey( u"Raster/defaultContrastEnhancementAlgorithm/%1"_s, { rendererKey }, true );
    QgsRasterLayer::settingsRasterDefaultContrastEnhancementLimits->copyValueFromKey( u"Raster/defaultContrastEnhancementLimits/%1"_s, { rendererKey }, true );
  }
  // No copyValueFromKey for settingsFavoriteDirs: old key "browser/favourites" is identical to new key path
  QgsNetworkAccessManager::settingsProxyEnabled->copyValueFromKey( u"proxy/proxyEnabled"_s, true );
  QgsNetworkAccessManager::settingsProxyHost->copyValueFromKey( u"proxy/proxyHost"_s, true );
  QgsNetworkAccessManager::settingsProxyPort->copyValueFromKey( u"proxy/proxyPort"_s, true );
  QgsNetworkAccessManager::settingsProxyUser->copyValueFromKey( u"proxy/proxyUser"_s, true );
  QgsNetworkAccessManager::settingsProxyPassword->copyValueFromKey( u"proxy/proxyPassword"_s, true );
  QgsNetworkAccessManager::settingsProxyType->copyValueFromKey( u"proxy/proxyType"_s, true );
  QgsNetworkAccessManager::settingsProxyExcludedUrls->copyValueFromKey( u"proxy/proxyExcludedUrls"_s, true );
  QgsNetworkAccessManager::settingsNoProxyUrls->copyValueFromKey( u"proxy/noProxyUrls"_s, true );
  QgsNetworkAccessManager::settingsProxyAuthCfg->copyValueFromKey( u"proxy/authcfg"_s, true );
  QgsApplication::settingsNullRepresentation->copyValueFromKey( u"qgis/nullValue"_s, true );
  QgsImageCache::settingsMaxImageCacheSize->copyValueFromKey( u"qgis/maxImageCacheSize"_s, true );
  QgsSymbolLegendNode::settingsLegendSymbolMinimumSize->copyValueFromKey( u"qgis/legendsymbolMinimumSize"_s, true );
  QgsSymbolLegendNode::settingsLegendSymbolMaximumSize->copyValueFromKey( u"qgis/legendsymbolMaximumSize"_s, true );
#ifdef HAVE_OPENCL
  QgsOpenClUtils::settingsOpenClEnabled->copyValueFromKey( u"core/OpenClEnabled"_s, true );
  QgsOpenClUtils::settingsOpenClDefaultDevice->copyValueFromKey( u"core/OpenClDefaultDevice"_s, true );
#endif
  QgsOgrProviderUtils::settingsWalForSqlite3->copyValueFromKey( u"qgis/walForSqlite3"_s, true );

  QgsLayoutGridSettings::settingsGridStyle->copyValueFromKey( u"gui/LayoutDesigner/gridStyle"_s, true );
  QgsLayoutGridSettings::settingsGridColor->copyValueFromKeys( u"gui/LayoutDesigner/gridRed"_s, u"gui/LayoutDesigner/gridGreen"_s, u"gui/LayoutDesigner/gridBlue"_s, u"gui/LayoutDesigner/gridAlpha"_s, true );
  QgsLayoutGridSettings::settingsGridResolution->copyValueFromKey( u"gui/LayoutDesigner/defaultSnapGridResolution"_s, true );
  QgsLayoutGridSettings::settingsGridOffsetX->copyValueFromKey( u"gui/LayoutDesigner/defaultSnapGridOffsetX"_s, true );
  QgsLayoutGridSettings::settingsGridOffsetY->copyValueFromKey( u"gui/LayoutDesigner/defaultSnapGridOffsetY"_s, true );
  QgsLayoutSnapper::settingsSnapTolerance->copyValueFromKey( u"gui/LayoutDesigner/defaultSnapTolerancePixels"_s, true );

  QgsRasterRendererRegistry::settingsDefaultRedBand->copyValueFromKey( u"Raster/defaultRedBand"_s, true );
  QgsRasterRendererRegistry::settingsDefaultGreenBand->copyValueFromKey( u"Raster/defaultGreenBand"_s, true );
  QgsRasterRendererRegistry::settingsDefaultBlueBand->copyValueFromKey( u"Raster/defaultBlueBand"_s, true );
  QgsRasterRendererRegistry::settingsUseStandardDeviation->copyValueFromKey( u"Raster/useStandardDeviation"_s, true );
  QgsRasterRendererRegistry::settingsDefaultStandardDeviation->copyValueFromKey( u"Raster/defaultStandardDeviation"_s, true );

  pal::Pal::settingsRenderingLabelCandidatesLimitPoints->copyValueFromKey( u"core/rendering/label_candidates_limit_points"_s, true );
  pal::Pal::settingsRenderingLabelCandidatesLimitLines->copyValueFromKey( u"core/rendering/label_candidates_limit_lines"_s, true );
  pal::Pal::settingsRenderingLabelCandidatesLimitPolygons->copyValueFromKey( u"core/rendering/label_candidates_limit_polygons"_s, true );

  QgsCustomColorScheme::settingsPaletteColors->copyValueFromKey( u"colors/palettecolors"_s, {}, true );
  QgsCustomColorScheme::settingsPaletteLabels->copyValueFromKey( u"colors/palettelabels"_s, {}, true );
  QgsUserColorScheme::settingsShowInMenuList->copyValueFromKey( u"colors/showInMenuList"_s, {}, true );

  // digitizing settings - added in 3.30
  {
    settingsDigitizingLineColor->copyValueFromKeys( u"qgis/digitizing/line_color_red"_s, u"qgis/digitizing/line_color_green"_s, u"qgis/digitizing/line_color_blue"_s, u"qgis/digitizing/line_color_alpha"_s, true );
    settingsDigitizingFillColor->copyValueFromKeys( u"qgis/digitizing/fill_color_red"_s, u"qgis/digitizing/fill_color_green"_s, u"qgis/digitizing/fill_color_blue"_s, u"qgis/digitizing/fill_color_alpha"_s, true );

    const QList<const QgsSettingsEntryBase *> settings = QgsSettingsTree::sTreeDigitizing->childrenSettings();
    for ( const QgsSettingsEntryBase *setting : settings )
    {
      QString name = setting->name();
      if ( name == settingsDigitizingStreamTolerance->name() || name == settingsDigitizingLineColor->name() || name == settingsDigitizingFillColor->name() )
        continue;
      if ( name == settingsDigitizingReuseLastValues->name() )
      {
        name = u"reuseLastValues"_s;
      }
      else
      {
        name.replace( '-', '_' );
      }
      setting->copyValueFromKey( QString( "qgis/digitizing/%1" ).arg( name ), true );
    }
  }

  // locator filters - added in 3.30
  {
    QSettings &locatorSettings = QgsSettingsEntryBase::userSettings();
    locatorSettings.beginGroup( u"gui/locator_filters"_s );
    const QStringList childKeys = locatorSettings.childKeys();
    locatorSettings.endGroup();
    for ( const QString &childKey : childKeys )
    {
      if ( childKey.startsWith( "enabled"_L1 ) )
      {
        QString filter = childKey;
        filter.remove( u"enabled_"_s );
        QgsLocator::settingsLocatorFilterEnabled->copyValueFromKey( u"gui/locator_filters/enabled_%1"_s, { filter }, true );
        QgsLocator::settingsLocatorFilterDefault->copyValueFromKey( u"gui/locator_filters/default_%1"_s, { filter }, true );
        QgsLocator::settingsLocatorFilterPrefix->copyValueFromKey( u"gui/locator_filters/prefix_%1"_s, { filter }, true );
      }
    }
  }

  // connections settings - added in 3.30
  const QStringList services = { u"WMS"_s, u"WFS"_s, u"WCS"_s };
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

      QgsOwsConnection::settingsUrl->copyValueFromKey( u"qgis/connections-%1/%2/url"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsVersion->copyValueFromKey( u"qgis/connections-%1/%2/version"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsIgnoreGetMapURI->copyValueFromKey( u"qgis/connections-%1/%2/ignoreGetMapURI"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->copyValueFromKey( u"qgis/connections-%1/%2/ignoreGetFeatureInfoURI"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsSmoothPixmapTransform->copyValueFromKey( u"qgis/connections-%1/%2/smoothPixmapTransform"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsReportedLayerExtents->copyValueFromKey( u"qgis/connections-%1/%2/reportedLayerExtents"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsDpiMode->copyValueFromKey( u"qgis/connections-%1/%2/dpiMode"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsTilePixelRatio->copyValueFromKey( u"qgis/connections-%1/%2/tilePixelRatio"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsMaxNumFeatures->copyValueFromKey( u"qgis/connections-%1/%2/maxnumfeatures"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsPagesize->copyValueFromKey( u"qgis/connections-%1/%2/pagesize"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsPagingEnabled->copyValueFromKey( u"qgis/connections-%1/%2/pagingenabled"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsPreferCoordinatesForWfsT11->copyValueFromKey( u"qgis/connections-%1/%2/preferCoordinatesForWfsT11"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsIgnoreAxisOrientation->copyValueFromKey( u"qgis/connections-%1/%2/ignoreAxisOrientation"_s, { service.toLower(), connection }, true );
      QgsOwsConnection::settingsInvertAxisOrientation->copyValueFromKey( u"qgis/connections-%1/%2/invertAxisOrientation"_s, { service.toLower(), connection }, true );

      Q_NOWARN_DEPRECATED_PUSH
      settings.beginGroup( connection );
      QgsOwsConnection::settingsHeaders->setValue( QgsHttpHeaders( settings ).headers(), { service.toLower(), connection } );
      settings.remove( u"http-header"_s );
      settings.endGroup();
      Q_NOWARN_DEPRECATED_POP

      QgsOwsConnection::settingsUsername->copyValueFromKey( u"qgis/connections/%1/%2/username"_s.arg( service, connection ), { service.toLower(), connection }, true );
      QgsOwsConnection::settingsPassword->copyValueFromKey( u"qgis/connections/%1/%2/password"_s.arg( service, connection ), { service.toLower(), connection }, true );
      QgsOwsConnection::settingsAuthCfg->copyValueFromKey( u"qgis/connections/%1/%2/authcfg"_s.arg( service, connection ), { service.toLower(), connection }, true );
    }
    if ( settings.contains( u"selected"_s ) )
      QgsOwsConnection::sTreeOwsConnections->setSelectedItem( settings.value( u"selected"_s ).toString(), { service.toLower() } );
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

      QgsVectorTileProviderConnection::settingsUrl->copyValueFromKey( u"qgis/connections-vector-tile/%1/url"_s, { connection }, true );
      QgsVectorTileProviderConnection::settingsZmin->copyValueFromKey( u"qgis/connections-vector-tile/%1/zmin"_s, { connection }, true );
      QgsVectorTileProviderConnection::settingsZmax->copyValueFromKey( u"qgis/connections-vector-tile/%1/zmax"_s, { connection }, true );
      QgsVectorTileProviderConnection::settingsAuthcfg->copyValueFromKey( u"qgis/connections-vector-tile/%1/authcfg"_s, { connection }, true );
      QgsVectorTileProviderConnection::settingsUsername->copyValueFromKey( u"qgis/connections-vector-tile/%1/username"_s, { connection }, true );
      QgsVectorTileProviderConnection::settingsPassword->copyValueFromKey( u"qgis/connections-vector-tile/%1/password"_s, { connection }, true );
      QgsVectorTileProviderConnection::settingsStyleUrl->copyValueFromKey( u"qgis/connections-vector-tile/%1/styleUrl"_s, { connection }, true );
      QgsVectorTileProviderConnection::settingsServiceType->copyValueFromKey( u"qgis/connections-vector-tile/%1/serviceType"_s, { connection }, true );
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

      QgsXyzConnectionSettings::settingsUrl->copyValueFromKey( u"qgis/connections-xyz/%1/url"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsZmin->copyValueFromKey( u"qgis/connections-xyz/%1/zmin"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsZmax->copyValueFromKey( u"qgis/connections-xyz/%1/zmax"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsAuthcfg->copyValueFromKey( u"qgis/connections-xyz/%1/authcfg"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsUsername->copyValueFromKey( u"qgis/connections-xyz/%1/username"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsPassword->copyValueFromKey( u"qgis/connections-xyz/%1/password"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsTilePixelRatio->copyValueFromKey( u"qgis/connections-xyz/%1/tilePixelRatio"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsHidden->copyValueFromKey( u"qgis/connections-xyz/%1/hidden"_s, { connection }, true );
      QgsXyzConnectionSettings::settingsInterpretation->copyValueFromKey( u"qgis/connections-xyz/%1/interpretation"_s, { connection }, true );
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
    const QStringList serviceKeys = { u"qgis/connections-arcgisfeatureserver"_s, u"qgis/connections-arcgismapserver"_s };
    QgsSettings settings;
    for ( const QString &serviceKey : serviceKeys )
    {
      settings.beginGroup( serviceKey );
      const QStringList connections = settings.childGroups();
      for ( const QString &connection : connections )
      {
        QgsArcGisConnectionSettings::settingsUrl->copyValueFromKey( u"qgis/connections-arcgisfeatureserver/%1/url"_s, { connection }, true );
        QgsArcGisConnectionSettings::settingsAuthcfg->copyValueFromKey( u"qgis/ARCGISFEATURESERVER/%1/authcfg"_s, { connection }, true );
        QgsArcGisConnectionSettings::settingsUsername->copyValueFromKey( u"qgis/ARCGISFEATURESERVER/%1/username"_s, { connection }, true );
        QgsArcGisConnectionSettings::settingsPassword->copyValueFromKey( u"qgis/ARCGISFEATURESERVER/%1/password"_s, { connection }, true );
        QgsArcGisConnectionSettings::settingsContentEndpoint->copyValueFromKey( u"qgis/connections-arcgisfeatureserver/%1/content_endpoint"_s, { connection }, true );
        QgsArcGisConnectionSettings::settingsCommunityEndpoint->copyValueFromKey( u"qgis/connections-arcgisfeatureserver/%1/community_endpoint"_s, { connection }, true );
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
      const QStringList deviceNames = QgsSettingsEntryBase::userSettings().value( u"/Plugin-GPS/devices/deviceList"_s ).toStringList();

      for ( const QString &device : deviceNames )
      {
        QgsBabelFormatRegistry::settingsBabelWptDownload->copyValueFromKey( u"/Plugin-GPS/devices/%1/wptdownload"_s, { device }, true );
        QgsBabelFormatRegistry::settingsBabelWptUpload->copyValueFromKey( u"/Plugin-GPS/devices/%1/wptupload"_s, { device }, true );
        QgsBabelFormatRegistry::settingsBabelRteDownload->copyValueFromKey( u"/Plugin-GPS/devices/%1/rtedownload"_s, { device }, true );
        QgsBabelFormatRegistry::settingsBabelRteUpload->copyValueFromKey( u"/Plugin-GPS/devices/%1/rteupload"_s, { device }, true );
        QgsBabelFormatRegistry::settingsBabelTrkDownload->copyValueFromKey( u"/Plugin-GPS/devices/%1/trkdownload"_s, { device }, true );
        QgsBabelFormatRegistry::settingsBabelTrkUpload->copyValueFromKey( u"/Plugin-GPS/devices/%1/trkupload"_s, { device }, true );
      }
    }
  }

  // recent CRS
  QgsCoordinateReferenceSystemRegistry::settingsRecentProjectionsAuthId->copyValueFromKey( u"UI/recentProjectionsAuthId"_s, {}, true );
  QgsCoordinateReferenceSystemRegistry::settingsRecentProjectionsAuthId->copyValueFromKey( u"crs/recentProjectionsAuthId"_s, {}, true );
  QgsCoordinateReferenceSystemRegistry::settingsRecentProjectionsWkt->copyValueFromKey( u"UI/recentProjectionsWkt"_s, {}, true );
  QgsCoordinateReferenceSystemRegistry::settingsRecentProjectionsWkt->copyValueFromKey( u"crs/recentProjectionsWkt"_s, {}, true );
  QgsCoordinateReferenceSystemRegistry::settingsRecentProjectionsProj4->copyValueFromKey( u"UI/recentProjectionsProj4"_s, {}, true );
  QgsCoordinateReferenceSystemRegistry::settingsRecentProjectionsProj4->copyValueFromKey( u"crs/recentProjectionsProj4"_s, {}, true );

  // auth settings
  QgsAuthManager::settingsPasswordHelperInsecureFallback->copyValueFromKey( u"auth/password_helper_insecure_fallback"_s, {}, true );
  QgsAuthManager::settingsUsePasswordHelper->copyValueFromKey( u"auth/use_password_helper"_s, {}, true );
  QgsAuthManager::settingsPasswordHelperLogging->copyValueFromKey( u"auth/password_helper_logging"_s, {}, true );

  // raster cumulative cut
  QgsRasterMinMaxOrigin::settingsCumulativeCutLower->copyValueFromKey( u"Raster/cumulativeCutLower"_s, {}, true );
  QgsRasterMinMaxOrigin::settingsCumulativeCutLower->copyValueFromKey( u"raster/cumulativeCutLower"_s, {}, true );
  QgsRasterMinMaxOrigin::settingsCumulativeCutUpper->copyValueFromKey( u"Raster/cumulativeCutUpper"_s, {}, true );
  QgsRasterMinMaxOrigin::settingsCumulativeCutUpper->copyValueFromKey( u"raster/cumulativeCutUpper"_s, {}, true );

  // CptCity
  QgsCptCityArchive::settingsCptCityBaseDir->copyValueFromKey( u"CptCity/baseDir"_s, {}, true );
  QgsCptCityArchive::settingsCptCityBaseDir->copyValueFromKey( u"core/cptcity-base-dir"_s, {}, true );
  QgsCptCityArchive::settingsCptCityArchiveName->copyValueFromKey( u"CptCity/archiveName"_s, {}, true );
  QgsCptCityArchive::settingsCptCityArchiveName->copyValueFromKey( u"core/cptcity-archive-name"_s, {}, true );

  // encoding
  QgsVectorFileWriter::settingsDefaultEncoding->copyValueFromKey( u"UI/encoding"_s, {}, true );

  // browser custom directory colors - dynamic per-path key
  {
    auto settings = QgsSettings::get();
    settings->beginGroup( u"qgis/browserPathColors"_s );
    const QStringList keys = settings->childKeys();
    for ( const QString &mangledPath : keys )
    {
      QgsDirectoryItem::settingsCustomPathColor->setValue( settings->value( mangledPath ).toString(), { mangledPath } );
    }
    settings->endGroup();
    settings->remove( u"qgis/browserPathColors"_s );
  }

  // news feed disabled state - dynamic per-feed key
  {
    auto settings = QgsSettings::get();
    settings->beginGroup( u"core/NewsFeed"_s );
    const QStringList feedKeys = settings->childGroups();
    for ( const QString &feedKey : feedKeys )
    {
      const QString disabledKey = feedKey + "/disabled"_L1;
      if ( settings->contains( disabledKey ) )
      {
        QgsNewsFeedParser::settingsFeedDisabled->setValue( settings->value( disabledKey ).toBool(), { feedKey } );
        settings->remove( disabledKey );
      }
    }
    settings->endGroup();
  }

  // application custom variables - dynamic per-variable name key
  {
    auto settings = QgsSettings::get();
    settings->beginGroup( u"variables"_s );
    const QStringList keys = settings->childKeys();
    for ( const QString &name : keys )
    {
      QgsApplication::settingsCustomVariable->setValue( settings->value( name ), { name } );
    }
    settings->endGroup();
    settings->remove( u"variables"_s );
  }

  // processing default GUI parameter values - dynamic per-algorithm-id and per-parameter-name key
  {
    auto settings = QgsSettings::get();
    settings->beginGroup( u"Processing/DefaultGuiParam"_s );
    const QStringList algIds = settings->childGroups();
    for ( const QString &algId : algIds )
    {
      settings->beginGroup( algId );
      const QStringList paramNames = settings->childKeys();
      for ( const QString &paramName : paramNames )
      {
        QgsProcessing::settingsDefaultGuiParam->setValue( settings->value( paramName ), { algId, paramName } );
      }
      settings->endGroup();
    }
    settings->endGroup();
    settings->remove( u"Processing/DefaultGuiParam"_s );
  }

  // coordinate transform context - per source/destination CRS pair
  // old keys: Projections/<srcAuthId>//<destAuthId>_coordinateOp and _allowFallback
  {
    auto settings = QgsSettings::get();
    settings->beginGroup( u"Projections"_s );
    const QStringList projectionKeys = settings->allKeys();
    for ( const QString &key : projectionKeys )
    {
      if ( !key.contains( "coordinateOp"_L1 ) )
        continue;
      const QStringList split = key.split( '/' );
      if ( split.size() < 2 )
        continue;
      const QString srcAuthId = split.at( 0 );
      const QString destAuthId = split.at( 1 ).split( '_' ).at( 0 );
      if ( srcAuthId.isEmpty() || destAuthId.isEmpty() )
        continue;

      const QString proj = settings->value( key ).toString();
      const QString fallbackKey = u"%1//%2_allowFallback"_s.arg( srcAuthId, destAuthId );
      const bool allowFallback = settings->value( fallbackKey ).toBool();
      QgsCoordinateTransformContext::settingsCoordinateOperation->setValue( proj, { srcAuthId, destAuthId } );
      QgsCoordinateTransformContext::settingsAllowFallback->setValue( allowFallback, { srcAuthId, destAuthId } );
    }
    // remove all legacy entries
    for ( const QString &key : projectionKeys )
    {
      if ( key.contains( "srcTransform"_L1 ) || key.contains( "destTransform"_L1 ) || key.contains( "coordinateOp"_L1 ) || key.contains( "allowFallback"_L1 ) )
      {
        settings->remove( key );
      }
    }
    settings->endGroup();
  }

  // OGR DB connections (GeoPackage, SpatiaLite) - dynamic per-driver and per-connection key
  // old keys: providers/ogr/<driver>/connections/<conn>/path and providers/ogr/<driver>/connections/selected
  {
    auto settings = QgsSettings::get();
    settings->beginGroup( u"ogr"_s, QgsSettings::Section::Providers );
    const QStringList drivers = settings->childGroups();
    for ( const QString &driver : drivers )
    {
      settings->beginGroup( driver );
      settings->beginGroup( u"connections"_s );
      const QStringList connNames = settings->childGroups();
      for ( const QString &connName : connNames )
      {
        const QString path = settings->value( u"%1/path"_s.arg( connName ) ).toString();
        if ( !path.isEmpty() )
          QgsOgrDbConnection::settingsOgrConnectionPath->setValue( path, { driver, connName } );
      }
      const QString selected = settings->value( u"selected"_s ).toString();
      if ( !selected.isEmpty() )
        QgsOgrDbConnection::sTreeOgrConnectionItems->setSelectedItem( selected, { driver } );
      settings->endGroup(); // connections
      settings->endGroup(); // driver
    }
    settings->endGroup(); // ogr
    settings->remove( u"ogr"_s, QgsSettings::Section::Providers );
  }
}

void QgsSettingsRegistryCore::backwardCompatibility()
{}
