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
  mSceneName = settings.value( u"UI/last3DSceneExportName"_s, u"Scene"_s ).toString();
  mSceneFolderPath = settings.value( u"UI/last3DSceneExportDir"_s, QDir::homePath() ).toString();
  mTerrainResolution = settings.value( u"UI/last3DSceneExportTerrainResolution"_s, 128 ).toInt();
  mTerrainTextureResolution = settings.value( u"UI/last3DSceneExportTerrainTextureResolution"_s, 512 ).toInt();
  mScale = settings.value( u"UI/last3DSceneExportModelScale"_s, 1.0f ).toFloat();
  mSmoothEdges = settings.value( u"UI/last3DSceneExportSmoothEdges"_s, false ).toBool();
  mExportNormals = settings.value( u"UI/last3DSceneExportExportNormals"_s, true ).toBool();
  mExportTextures = settings.value( u"UI/last3DSceneExportExportTextures"_s, true ).toBool();
}

Qgs3DMapExportSettings::~Qgs3DMapExportSettings()
{
  QgsSettings settings;
  settings.setValue( u"UI/last3DSceneExportName"_s, mSceneName );
  settings.setValue( u"UI/last3DSceneExportDir"_s, mSceneFolderPath );
  settings.setValue( u"UI/last3DSceneExportTerrainResolution"_s, mTerrainResolution );
  settings.setValue( u"UI/last3DSceneExportTerrainTextureResolution"_s, mTerrainTextureResolution );
  settings.setValue( u"UI/last3DSceneExportModelScale"_s, mScale );
  settings.setValue( u"UI/last3DSceneExportSmoothEdges"_s, mSmoothEdges );
  settings.setValue( u"UI/last3DSceneExportExportNormals"_s, mExportNormals );
  settings.setValue( u"UI/last3DSceneExportExportTextures"_s, mExportTextures );
}
