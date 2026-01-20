/***************************************************************************
  qgs3dsymbolregistry.cpp
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

#include "qgs3dsymbolregistry.h"

#include "qgsabstract3dsymbol.h"

Qgs3DSymbolRegistry::Qgs3DSymbolRegistry()
{
}

Qgs3DSymbolRegistry::~Qgs3DSymbolRegistry()
{
  qDeleteAll( mMetadata );
}

bool Qgs3DSymbolRegistry::addSymbolType( Qgs3DSymbolAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  return true;
}

QgsAbstract3DSymbol *Qgs3DSymbolRegistry::createSymbol( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->create();
}

QgsAbstract3DSymbol *Qgs3DSymbolRegistry::defaultSymbolForGeometryType( Qgis::GeometryType type )
{
  switch ( type )
  {
    case Qgis::GeometryType::Point:
      return createSymbol( u"point"_s );
    case Qgis::GeometryType::Line:
      return createSymbol( u"line"_s );
    case Qgis::GeometryType::Polygon:
      return createSymbol( u"polygon"_s );
    default:
      return nullptr;
  }
}

QgsFeature3DHandler *Qgs3DSymbolRegistry::createHandlerForSymbol( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol )
{
  if ( !symbol )
    return nullptr;

  auto it = mMetadata.constFind( symbol->type() );
  if ( it == mMetadata.constEnd() )
    return nullptr;

  return it.value()->createFeatureHandler( layer, symbol );
}

Qgs3DSymbolAbstractMetadata *Qgs3DSymbolRegistry::symbolMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

QStringList Qgs3DSymbolRegistry::symbolTypes() const
{
  return mMetadata.keys();
}
