/***************************************************************************
  qgslabelingenginesettings.cpp
  --------------------------------------
  Date                 : April 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingenginesettings.h"

#include "qgsproject.h"
#include "qgssymbollayerutils.h"

QgsLabelingEngineSettings::QgsLabelingEngineSettings()
{
}

void QgsLabelingEngineSettings::clear()
{
  *this = QgsLabelingEngineSettings();
}

void QgsLabelingEngineSettings::readSettingsFromProject( QgsProject *prj )
{
  bool saved = false;
  mSearchMethod = static_cast< Search >( prj->readNumEntry( QStringLiteral( "PAL" ), QStringLiteral( "/SearchMethod" ), static_cast< int >( Chain ), &saved ) );
  mMaxLineCandidatesPerCm = prj->readDoubleEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesLinePerCM" ), 5, &saved );
  mMaxPolygonCandidatesPerCmSquared = prj->readDoubleEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesPolygonPerCM" ), 2.5, &saved );

  mFlags = Qgis::LabelingFlags();
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingCandidates" ), false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawCandidates;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawRectOnly" ), false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawLabelRectOnly;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingAllLabels" ), false, &saved ) ) mFlags |= Qgis::LabelingFlag::UseAllLabels;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingPartialsLabels" ), true, &saved ) ) mFlags |= Qgis::LabelingFlag::UsePartialCandidates;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawUnplaced" ), false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawUnplacedLabels;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawLabelMetrics" ), false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawLabelMetrics;

  mDefaultTextRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;
  // if users have disabled the older PAL "DrawOutlineLabels" setting, respect that
  if ( !prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true ) )
    mDefaultTextRenderFormat = Qgis::TextRenderFormat::AlwaysText;
  // otherwise, prefer the new setting
  const int projectTextFormat = prj->readNumEntry( QStringLiteral( "PAL" ), QStringLiteral( "/TextFormat" ), -1 );
  if ( projectTextFormat >= 0 )
    mDefaultTextRenderFormat = static_cast< Qgis::TextRenderFormat >( projectTextFormat );

  mUnplacedLabelColor = QgsSymbolLayerUtils::decodeColor( prj->readEntry( QStringLiteral( "PAL" ), QStringLiteral( "/UnplacedColor" ), QStringLiteral( "#ff0000" ) ) );

  mPlacementVersion = static_cast< Qgis::LabelPlacementEngineVersion >( prj->readNumEntry( QStringLiteral( "PAL" ), QStringLiteral( "/PlacementEngineVersion" ), static_cast< int >( Qgis::LabelPlacementEngineVersion::Version1 ) ) );
}

void QgsLabelingEngineSettings::writeSettingsToProject( QgsProject *project )
{
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/SearchMethod" ), static_cast< int >( mSearchMethod ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesLinePerCM" ), mMaxLineCandidatesPerCm );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesPolygonPerCM" ), mMaxPolygonCandidatesPerCmSquared );

  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingCandidates" ), mFlags.testFlag( Qgis::LabelingFlag::DrawCandidates ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawRectOnly" ), mFlags.testFlag( Qgis::LabelingFlag::DrawLabelRectOnly ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawUnplaced" ), mFlags.testFlag( Qgis::LabelingFlag::DrawUnplacedLabels ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingAllLabels" ), mFlags.testFlag( Qgis::LabelingFlag::UseAllLabels ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingPartialsLabels" ), mFlags.testFlag( Qgis::LabelingFlag::UsePartialCandidates ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawLabelMetrics" ), mFlags.testFlag( Qgis::LabelingFlag::DrawLabelMetrics ) );

  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/TextFormat" ), static_cast< int >( mDefaultTextRenderFormat ) );

  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/UnplacedColor" ), QgsSymbolLayerUtils::encodeColor( mUnplacedLabelColor ) );

  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/PlacementEngineVersion" ), static_cast< int >( mPlacementVersion ) );
}

QColor QgsLabelingEngineSettings::unplacedLabelColor() const
{
  return mUnplacedLabelColor;
}

void QgsLabelingEngineSettings::setUnplacedLabelColor( const QColor &unplacedLabelColor )
{
  mUnplacedLabelColor = unplacedLabelColor;
}

Qgis::LabelPlacementEngineVersion QgsLabelingEngineSettings::placementVersion() const
{
  return mPlacementVersion;
}

void QgsLabelingEngineSettings::setPlacementVersion( Qgis::LabelPlacementEngineVersion placementVersion )
{
  mPlacementVersion = placementVersion;
}


