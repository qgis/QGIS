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

QgsMaterialRegistry::QgsMaterialRegistry()
{
}

QgsMaterialRegistry::~QgsMaterialRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsMaterialRegistry::addMaterialSettingsType( QgsMaterialSettingsAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  mMaterialsOrder << metadata->type();
  return true;
}

QgsAbstractMaterialSettings *QgsMaterialRegistry::createMaterialSettings( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->create();
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
