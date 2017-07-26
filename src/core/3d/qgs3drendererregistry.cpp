#include "qgs3drendererregistry.h"


Qgs3DRendererAbstractMetadata::Qgs3DRendererAbstractMetadata( const QString &name )
  : mName( name )
{
}

QString Qgs3DRendererAbstractMetadata::name() const
{
  return mName;
}


// ----------


Qgs3DRendererRegistry::Qgs3DRendererRegistry()
{
}

Qgs3DRendererRegistry::~Qgs3DRendererRegistry()
{
  qDeleteAll( mRenderers );
}

void Qgs3DRendererRegistry::addRenderer( Qgs3DRendererAbstractMetadata *metadata )
{
  mRenderers.insert( metadata->name(), metadata );
}

void Qgs3DRendererRegistry::removeRenderer( const QString &name )
{
  delete mRenderers.take( name );
}

Qgs3DRendererAbstractMetadata *Qgs3DRendererRegistry::rendererMetadata( const QString &name ) const
{
  return mRenderers.value( name );
}

QStringList Qgs3DRendererRegistry::renderersList() const
{
  return mRenderers.keys();
}
