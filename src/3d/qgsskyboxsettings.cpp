/***************************************************************************
  qgsskyboxsettings.cpp
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsskyboxsettings.h"

#include <QDomDocument>

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

void QgsSkyboxSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QgsPathResolver &pathResolver = context.pathResolver();
  mIsSkyboxEnabled = element.attribute( QStringLiteral( "skybox-enabled" ) ).toInt();
  QString skyboxTypeStr = element.attribute( QStringLiteral( "skybox-type" ) );
  if ( skyboxTypeStr == QStringLiteral( "Textures collection" ) )
    mSkyboxType = QgsSkyboxEntity::TexturesCollectionSkybox;
  else if ( skyboxTypeStr == QStringLiteral( "Distinct Faces" ) )
    mSkyboxType = QgsSkyboxEntity::DistinctTexturesSkybox;
  else if ( skyboxTypeStr == QStringLiteral( "HDR texture" ) )
    mSkyboxType = QgsSkyboxEntity::HDRSkybox;
  mSkyboxBaseName = element.attribute( QStringLiteral( "base-name" ) );
  mSkyboxExt = element.attribute( QStringLiteral( "extension" ) );
  mHDRTexturePath = element.attribute( pathResolver.readPath( QStringLiteral( "HDR-texture-path" ) ) );
  mCubeMapFacesPaths.clear();
  mCubeMapFacesPaths[ QStringLiteral( "posX" ) ] = element.attribute( pathResolver.readPath( QStringLiteral( "posX-texture-path" ) ) );
  mCubeMapFacesPaths[ QStringLiteral( "posY" ) ] = element.attribute( pathResolver.readPath( QStringLiteral( "posY-texture-path" ) ) );
  mCubeMapFacesPaths[ QStringLiteral( "posZ" ) ] = element.attribute( pathResolver.readPath( QStringLiteral( "posZ-texture-path" ) ) );
  mCubeMapFacesPaths[ QStringLiteral( "negX" ) ] = element.attribute( pathResolver.readPath( QStringLiteral( "negX-texture-path" ) ) );
  mCubeMapFacesPaths[ QStringLiteral( "negY" ) ] = element.attribute( pathResolver.readPath( QStringLiteral( "negY-texture-path" ) ) );
  mCubeMapFacesPaths[ QStringLiteral( "negZ" ) ] = element.attribute( pathResolver.readPath( QStringLiteral( "negZ-texture-path" ) ) );
}

void QgsSkyboxSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "skybox-enabled" ), mIsSkyboxEnabled );
  switch ( mSkyboxType )
  {
    case QgsSkyboxEntity::TexturesCollectionSkybox:
      element.setAttribute( QStringLiteral( "skybox-type" ), QStringLiteral( "Textures collection" ) );
      break;
    case QgsSkyboxEntity::DistinctTexturesSkybox:
      element.setAttribute( QStringLiteral( "skybox-type" ), QStringLiteral( "Distinct Faces" ) );
      break;
    case QgsSkyboxEntity::HDRSkybox:
      element.setAttribute( QStringLiteral( "skybox-type" ), QStringLiteral( "HDR texture" ) );
      break;
  }

  const QgsPathResolver &pathResolver = context.pathResolver();
  element.setAttribute( QStringLiteral( "base-name" ), mSkyboxBaseName );
  element.setAttribute( QStringLiteral( "extension" ), mSkyboxExt );
  element.setAttribute( QStringLiteral( "HDR-texture-path" ), pathResolver.writePath( mHDRTexturePath ) );
  element.setAttribute( QStringLiteral( "posX-texture-path" ), pathResolver.writePath( mCubeMapFacesPaths[ QStringLiteral( "posX" ) ] ) );
  element.setAttribute( QStringLiteral( "posY-texture-path" ), pathResolver.writePath( mCubeMapFacesPaths[ QStringLiteral( "posY" ) ] ) );
  element.setAttribute( QStringLiteral( "posZ-texture-path" ), pathResolver.writePath( mCubeMapFacesPaths[ QStringLiteral( "posZ" ) ] ) );
  element.setAttribute( QStringLiteral( "negX-texture-path" ), pathResolver.writePath( mCubeMapFacesPaths[ QStringLiteral( "negX" ) ] ) );
  element.setAttribute( QStringLiteral( "negY-texture-path" ), pathResolver.writePath( mCubeMapFacesPaths[ QStringLiteral( "negY" ) ] ) );
  element.setAttribute( QStringLiteral( "negZ-texture-path" ), pathResolver.writePath( mCubeMapFacesPaths[ QStringLiteral( "negZ" ) ] ) );
}
