/***************************************************************************
  qgsmaterialregistry.cpp
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterialregistry.h"

#include "qgsabstractmaterialsettings.h"
#include "qgsapplication.h"
#include "qgsgoochmaterialsettings.h"
#include "qgsmetalroughmaterialsettings.h"
#include "qgsnullmaterialsettings.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgssimplelinematerialsettings.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsMaterialRegistry::QgsMaterialRegistry()
{}

QgsMaterialRegistry::~QgsMaterialRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsMaterialRegistry::populate()
{
  if ( !mMetadata.empty() )
    return false;

  addMaterialSettingsType(
    new QgsMaterialSettingsMetadata( u"null"_s, QObject::tr( "Embedded Textures" ), QgsNullMaterialSettings::create, QgsNullMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( u"/mIconPhongTexturedMaterial.svg"_s ) )
  );
  addMaterialSettingsType(
    new QgsMaterialSettingsMetadata( u"phong"_s, QObject::tr( "Realistic (Phong)" ), QgsPhongMaterialSettings::create, QgsPhongMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( u"/mIconPhongMaterial.svg"_s ) )
  );
  addMaterialSettingsType(
    new QgsMaterialSettingsMetadata( u"phongtextured"_s, QObject::tr( "Realistic Textured (Phong)" ), QgsPhongTexturedMaterialSettings::create, QgsPhongTexturedMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( u"/mIconPhongTexturedMaterial.svg"_s ) )
  );
  addMaterialSettingsType(
    new QgsMaterialSettingsMetadata( u"simpleline"_s, QObject::tr( "Single Color (Unlit)" ), QgsSimpleLineMaterialSettings::create, QgsSimpleLineMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( u"/mIconSimpleLineMaterial.svg"_s ) )
  );
  addMaterialSettingsType(
    new QgsMaterialSettingsMetadata( u"gooch"_s, QObject::tr( "CAD (Gooch)" ), QgsGoochMaterialSettings::create, QgsGoochMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( u"/mIconGoochMaterial.svg"_s ) )
  );
  addMaterialSettingsType(
    new QgsMaterialSettingsMetadata( u"metalrough"_s, QObject::tr( "Metal Roughness" ), QgsMetalRoughMaterialSettings::create, QgsMetalRoughMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( u"/mIconGoochMaterial.svg"_s ) )
  );
  return true;
}

bool QgsMaterialRegistry::addMaterialSettingsType( QgsMaterialSettingsAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  mMaterialsOrder << metadata->type();
  return true;
}

std::unique_ptr< QgsAbstractMaterialSettings > QgsMaterialRegistry::createMaterialSettings( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return std::unique_ptr< QgsAbstractMaterialSettings >( mMetadata[type]->create() );
}

QgsMaterialSettingsAbstractMetadata *QgsMaterialRegistry::materialSettingsMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

QStringList QgsMaterialRegistry::materialSettingsTypes() const
{
  QStringList types;
  for ( const QString &material : mMaterialsOrder )
  {
    if ( mMetadata.value( material ) )
      types << material;
  }
  return types;
}
