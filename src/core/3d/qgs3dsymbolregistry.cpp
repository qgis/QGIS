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

Qgs3DSymbolAbstractMetadata *Qgs3DSymbolRegistry::symbolMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

QStringList Qgs3DSymbolRegistry::symbolTypes() const
{
  return mMetadata.keys();
}
