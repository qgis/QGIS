/***************************************************************************
  qgs3drendererregistry.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3drendererregistry.h"


Qgs3DRendererAbstractMetadata::Qgs3DRendererAbstractMetadata( const QString &type )
  : mType( type )
{
}

QString Qgs3DRendererAbstractMetadata::type() const
{
  return mType;
}


// ----------


Qgs3DRendererRegistry::~Qgs3DRendererRegistry()
{
  qDeleteAll( mRenderers );
}

void Qgs3DRendererRegistry::addRenderer( Qgs3DRendererAbstractMetadata *metadata )
{
  mRenderers.insert( metadata->type(), metadata );
}

void Qgs3DRendererRegistry::removeRenderer( const QString &type )
{
  delete mRenderers.take( type );
}

Qgs3DRendererAbstractMetadata *Qgs3DRendererRegistry::rendererMetadata( const QString &type ) const
{
  return mRenderers.value( type );
}

QStringList Qgs3DRendererRegistry::renderersList() const
{
  return mRenderers.keys();
}
