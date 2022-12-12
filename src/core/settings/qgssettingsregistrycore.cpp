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

  addSettingsEntry( &QgsApplication::settingsLocaleUserLocale );
  addSettingsEntry( &QgsApplication::settingsLocaleOverrideFlag );
  addSettingsEntry( &QgsApplication::settingsLocaleGlobalLocale );
  addSettingsEntry( &QgsApplication::settingsLocaleShowGroupSeparator );
  addSettingsEntry( &QgsApplication::settingsSearchPathsForSVG );

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

  addSettingsEntry( &QgsOwsConnection::settingsConnectionSelected );
  addSettingsEntryGroup( &QgsOwsConnection::settingsServiceConnectionDetailsGroup );
  addSettingsEntryGroup( &QgsOwsConnection::settingsServiceConnectionCredentialsGroup );

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

  addSettingsEntry( &QgsBabelFormatRegistry::settingsBabelDeviceList );
  addSettingsEntryGroup( &QgsBabelFormatRegistry::settingsBabelDeviceGroup );

  addSettingsEntry( &QgsGpsLogger::settingsGpsStoreAttributeInMValues );
  addSettingsEntry( &QgsGpsLogger::settingsGpsMValueComponent );
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
}

