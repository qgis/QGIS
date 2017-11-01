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


QgsLabelingEngineSettings::QgsLabelingEngineSettings()
  : mFlags( RenderOutlineLabels | UsePartialCandidates )
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
  mCandPoint = prj->readNumEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesPoint" ), 16, &saved );
  mCandLine = prj->readNumEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesLine" ), 50, &saved );
  mCandPolygon = prj->readNumEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesPolygon" ), 30, &saved );

  mFlags = 0;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingCandidates" ), false, &saved ) ) mFlags |= DrawCandidates;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawRectOnly" ), false, &saved ) ) mFlags |= DrawLabelRectOnly;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingAllLabels" ), false, &saved ) ) mFlags |= UseAllLabels;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingPartialsLabels" ), true, &saved ) ) mFlags |= UsePartialCandidates;
  if ( prj->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true, &saved ) ) mFlags |= RenderOutlineLabels;
}

void QgsLabelingEngineSettings::writeSettingsToProject( QgsProject *project )
{
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/SearchMethod" ), static_cast< int >( mSearchMethod ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesPoint" ), mCandPoint );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesLine" ), mCandLine );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/CandidatesPolygon" ), mCandPolygon );

  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingCandidates" ), mFlags.testFlag( DrawCandidates ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawRectOnly" ), mFlags.testFlag( DrawLabelRectOnly ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingAllLabels" ), mFlags.testFlag( UseAllLabels ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/ShowingPartialsLabels" ), mFlags.testFlag( UsePartialCandidates ) );
  project->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), mFlags.testFlag( RenderOutlineLabels ) );
}
