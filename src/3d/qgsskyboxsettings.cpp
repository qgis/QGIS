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
  Q_UNUSED( context );
  mIsSkyboxEnabled = element.attribute( QStringLiteral( "skybox-enabled" ) ).toInt();
  mSkyboxType = element.attribute( QStringLiteral( "skybox-type" ) );
  mSkyboxBaseName = element.attribute( QStringLiteral( "base-name" ) );
  mSkyboxExt = element.attribute( QStringLiteral( "extension" ) );
  mHDRTexturePath = element.attribute( QStringLiteral( "HDR-texture-path" ) );
  mCubeMapFacesPaths.clear();
  mCubeMapFacesPaths[ QStringLiteral( "posX" ) ] = element.attribute( QStringLiteral( "posX-texture-path" ) );
  mCubeMapFacesPaths[ QStringLiteral( "posY" ) ] = element.attribute( QStringLiteral( "posY-texture-path" ) );
  mCubeMapFacesPaths[ QStringLiteral( "posZ" ) ] = element.attribute( QStringLiteral( "posZ-texture-path" ) );
  mCubeMapFacesPaths[ QStringLiteral( "negX" ) ] = element.attribute( QStringLiteral( "negX-texture-path" ) );
  mCubeMapFacesPaths[ QStringLiteral( "negY" ) ] = element.attribute( QStringLiteral( "negY-texture-path" ) );
  mCubeMapFacesPaths[ QStringLiteral( "negZ" ) ] = element.attribute( QStringLiteral( "negZ-texture-path" ) );
}

void QgsSkyboxSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );
  element.setAttribute( QStringLiteral( "skybox-enabled" ), mIsSkyboxEnabled );
  element.setAttribute( QStringLiteral( "skybox-type" ), mSkyboxType );
  element.setAttribute( QStringLiteral( "base-name" ), mSkyboxBaseName );
  element.setAttribute( QStringLiteral( "extension" ), mSkyboxExt );
  element.setAttribute( QStringLiteral( "HDR-texture-path" ), mHDRTexturePath );
  element.setAttribute( QStringLiteral( "posX-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "posX" ) ] );
  element.setAttribute( QStringLiteral( "posY-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "posY" ) ] );
  element.setAttribute( QStringLiteral( "posZ-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "posZ" ) ] );
  element.setAttribute( QStringLiteral( "negX-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "negX" ) ] );
  element.setAttribute( QStringLiteral( "negY-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "negY" ) ] );
  element.setAttribute( QStringLiteral( "negZ-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "negZ" ) ] );
}
