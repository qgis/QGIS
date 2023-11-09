/***************************************************************************
  qgs3dmapexportsettings.cpp
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapexportsettings.h"
#include "qgssettings.h"

Qgs3DMapExportSettings::Qgs3DMapExportSettings()
{
  const QgsSettings settings;
  mSceneName = settings.value( QStringLiteral( "UI/last3DSceneExportName" ), QStringLiteral( "Scene" ) ).toString();
  mSceneFolderPath = settings.value( QStringLiteral( "UI/last3DSceneExportDir" ), QDir::homePath() ).toString();
  mTerrainResolution = settings.value( QStringLiteral( "UI/last3DSceneExportTerrainResolution" ), 128 ).toInt();
  mTerrainTextureResolution = settings.value( QStringLiteral( "UI/last3DSceneExportTerrainTextureResolution" ), 512 ).toInt();
  mScale = settings.value( QStringLiteral( "UI/last3DSceneExportModelScale" ), 1.0f ).toFloat();
  mSmoothEdges = settings.value( QStringLiteral( "UI/last3DSceneExportSmoothEdges" ), false ).toBool();
  mExportNormals = settings.value( QStringLiteral( "UI/last3DSceneExportExportNormals" ), true ).toBool();
  mExportTextures = settings.value( QStringLiteral( "UI/last3DSceneExportExportTextures" ), true ).toBool();
}

Qgs3DMapExportSettings::~Qgs3DMapExportSettings()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/last3DSceneExportName" ), mSceneName );
  settings.setValue( QStringLiteral( "UI/last3DSceneExportDir" ), mSceneFolderPath );
  settings.setValue( QStringLiteral( "UI/last3DSceneExportTerrainResolution" ), mTerrainResolution );
  settings.setValue( QStringLiteral( "UI/last3DSceneExportTerrainTextureResolution" ), mTerrainTextureResolution );
  settings.setValue( QStringLiteral( "UI/last3DSceneExportModelScale" ), mScale );
  settings.setValue( QStringLiteral( "UI/last3DSceneExportSmoothEdges" ), mSmoothEdges );
  settings.setValue( QStringLiteral( "UI/last3DSceneExportExportNormals" ), mExportNormals );
  settings.setValue( QStringLiteral( "UI/last3DSceneExportExportTextures" ), mExportTextures );
}
