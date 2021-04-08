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

  addSettingsEntry( &QgsNetworkAccessManager::Settings::networkTimeout );

  addSettingsEntry( &QgsNewsFeedParser::Settings::feedLastFetchTime );
  addSettingsEntry( &QgsNewsFeedParser::Settings::feedLanguage );
  addSettingsEntry( &QgsNewsFeedParser::Settings::feedLatitude );
  addSettingsEntry( &QgsNewsFeedParser::Settings::feedLongitude );

  addSettingsEntry( &QgsProcessing::Settings::preferFilenameAsLayerName );
  addSettingsEntry( &QgsProcessing::Settings::tempPath );
  addSettingsEntry( &QgsProcessing::Settings::defaultOutputVectorLayerExt );
  addSettingsEntry( &QgsProcessing::Settings::defaultOutputRasterLayerExt );

  addSettingsEntry( &QgsApplication::Settings::localeUserLocale );
  addSettingsEntry( &QgsApplication::Settings::localeOverrideFlag );
  addSettingsEntry( &QgsApplication::Settings::localeGlobalLocale );
  addSettingsEntry( &QgsApplication::Settings::localeShowGroupSeparator );
  addSettingsEntry( &QgsApplication::Settings::searchPathsForSVG );

  addSettingsEntry( &QgsGeometryOptions::Settings::geometryValidationDefaultChecks );

  addSettingsEntry( &QgsLocalizedDataPathRegistry::Settings::localizedDataPaths );

  addSettingsEntry( &QgsMapRendererJob::Settings::logCanvasRefreshEvent );
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
}

