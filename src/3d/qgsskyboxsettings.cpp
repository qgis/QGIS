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

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>

QgsSkyboxSettings::QgsSkyboxSettings( const QgsSkyboxSettings &other )
  : mSkyboxType( other.mSkyboxType )
  , mPanoramicTexturePath( other.mPanoramicTexturePath )
  , mCubeMapFacesPaths( other.mCubeMapFacesPaths )
{
}

QgsSkyboxSettings &QgsSkyboxSettings::operator=( QgsSkyboxSettings const &rhs )
{
  if ( &rhs == this )
    return *this;

  this->mSkyboxType = rhs.mSkyboxType;
  this->mPanoramicTexturePath = rhs.mPanoramicTexturePath;
  this->mCubeMapFacesPaths = rhs.mCubeMapFacesPaths;
  return *this;
}

void QgsSkyboxSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QgsPathResolver &pathResolver = context.pathResolver();
  const QString skyboxTypeStr = element.attribute( u"skybox-type"_s );
  if ( skyboxTypeStr == "Distinct Faces"_L1 )
    mSkyboxType = QgsSkyboxEntity::DistinctTexturesSkybox;
  else if ( skyboxTypeStr == "Panoramic Texture"_L1 )
    mSkyboxType = QgsSkyboxEntity::PanoramicSkybox;
  mPanoramicTexturePath = pathResolver.readPath( element.attribute( u"panoramic-texture-path"_s ) );
  mCubeMapFacesPaths.clear();
  mCubeMapFacesPaths[u"posX"_s] = pathResolver.readPath( element.attribute( u"posX-texture-path"_s ) );
  mCubeMapFacesPaths[u"posY"_s] = pathResolver.readPath( element.attribute( u"posY-texture-path"_s ) );
  mCubeMapFacesPaths[u"posZ"_s] = pathResolver.readPath( element.attribute( u"posZ-texture-path"_s ) );
  mCubeMapFacesPaths[u"negX"_s] = pathResolver.readPath( element.attribute( u"negX-texture-path"_s ) );
  mCubeMapFacesPaths[u"negY"_s] = pathResolver.readPath( element.attribute( u"negY-texture-path"_s ) );
  mCubeMapFacesPaths[u"negZ"_s] = pathResolver.readPath( element.attribute( u"negZ-texture-path"_s ) );
}

void QgsSkyboxSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  switch ( mSkyboxType )
  {
    case QgsSkyboxEntity::DistinctTexturesSkybox:
      element.setAttribute( u"skybox-type"_s, u"Distinct Faces"_s );
      break;
    case QgsSkyboxEntity::PanoramicSkybox:
      element.setAttribute( u"skybox-type"_s, u"Panoramic Texture"_s );
      break;
  }

  const QgsPathResolver &pathResolver = context.pathResolver();
  element.setAttribute( u"panoramic-texture-path"_s, pathResolver.writePath( mPanoramicTexturePath ) );
  element.setAttribute( u"posX-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"posX"_s] ) );
  element.setAttribute( u"posY-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"posY"_s] ) );
  element.setAttribute( u"posZ-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"posZ"_s] ) );
  element.setAttribute( u"negX-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"negX"_s] ) );
  element.setAttribute( u"negY-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"negY"_s] ) );
  element.setAttribute( u"negZ-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"negZ"_s] ) );
}
