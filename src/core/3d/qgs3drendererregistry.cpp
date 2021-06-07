/***************************************************************************
  qgs3drendererregistry.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
