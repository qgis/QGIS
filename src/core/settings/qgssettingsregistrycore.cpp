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

#include "qgslayout.h"
#include "qgslocator.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnewsfeedparser.h"
#include "qgsprocessing.h"
#include "qgsapplication.h"
#include "qgsgeometryoptions.h"
#include "qgslocalizeddatapathregistry.h"
#include "qgsmaprendererjob.h"


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
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
}

