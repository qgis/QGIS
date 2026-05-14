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

#include "qgsabstract3dmapbackgroundsettings.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <QString>

using namespace Qt::StringLiterals;

QgsSkyboxSettings *QgsSkyboxSettings::clone() const
{
  return new QgsSkyboxSettings( *this );
}


QgsSkyboxSettings::QgsSkyboxSettings( const QgsSkyboxSettings &other )
#if ENABLE_PANORAMIC_SKYBOX
  : mPanoramicTexturePath( other.mPanoramicTexturePath )
  , mCubeMapping( other.mCubeMapping )
#else
  : mCubeMapping( other.mCubeMapping )
#endif
  , mCubeMapFacesPaths( other.mCubeMapFacesPaths )
{}

QgsSkyboxSettings &QgsSkyboxSettings::operator=( QgsSkyboxSettings const &rhs )
{
  if ( &rhs == this )
    return *this;

#if ENABLE_PANORAMIC_SKYBOX
  this->mPanoramicTexturePath = rhs.mPanoramicTexturePath;
#endif
  this->mCubeMapFacesPaths = rhs.mCubeMapFacesPaths;
  this->mCubeMapping = rhs.mCubeMapping;
  return *this;
}

void QgsSkyboxSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QgsPathResolver &pathResolver = context.pathResolver();
#if ENABLE_PANORAMIC_SKYBOX
  mPanoramicTexturePath = pathResolver.readPath( element.attribute( u"panoramic-texture-path"_s ) );
#endif
  mCubeMapFacesPaths.clear();
  mCubeMapFacesPaths[u"posX"_s] = pathResolver.readPath( element.attribute( u"posX-texture-path"_s ) );
  mCubeMapFacesPaths[u"posY"_s] = pathResolver.readPath( element.attribute( u"posY-texture-path"_s ) );
  mCubeMapFacesPaths[u"posZ"_s] = pathResolver.readPath( element.attribute( u"posZ-texture-path"_s ) );
  mCubeMapFacesPaths[u"negX"_s] = pathResolver.readPath( element.attribute( u"negX-texture-path"_s ) );
  mCubeMapFacesPaths[u"negY"_s] = pathResolver.readPath( element.attribute( u"negY-texture-path"_s ) );
  mCubeMapFacesPaths[u"negZ"_s] = pathResolver.readPath( element.attribute( u"negZ-texture-path"_s ) );
  mCubeMapping = qgsEnumKeyToValue( element.attribute( u"mapping"_s ), Qgis::SkyboxCubeMapping::NativeZUp );
}

void QgsSkyboxSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  const QgsPathResolver &pathResolver = context.pathResolver();
#if ENABLE_PANORAMIC_SKYBOX
  element.setAttribute( u"panoramic-texture-path"_s, pathResolver.writePath( mPanoramicTexturePath ) );
#endif
  element.setAttribute( u"posX-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"posX"_s] ) );
  element.setAttribute( u"posY-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"posY"_s] ) );
  element.setAttribute( u"posZ-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"posZ"_s] ) );
  element.setAttribute( u"negX-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"negX"_s] ) );
  element.setAttribute( u"negY-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"negY"_s] ) );
  element.setAttribute( u"negZ-texture-path"_s, pathResolver.writePath( mCubeMapFacesPaths[u"negZ"_s] ) );
  element.setAttribute( u"mapping"_s, qgsEnumValueToKey( mCubeMapping ) );
}

Qgis::SkyboxCubeMapping QgsSkyboxSettings::cubeMapping() const
{
  return mCubeMapping;
}

void QgsSkyboxSettings::setCubeMapping( Qgis::SkyboxCubeMapping mapping )
{
  mCubeMapping = mapping;
}
