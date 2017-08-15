#include "qgsvectorlayer3drenderer.h"

#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsline3dsymbol_p.h"
#include "qgspoint3dsymbol_p.h"
#include "qgspolygon3dsymbol_p.h"

#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"


QgsVectorLayer3DRendererMetadata::QgsVectorLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( "vector" )
{
}

QgsAbstract3DRenderer *QgsVectorLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}


// ---------


QgsVectorLayer3DRenderer::QgsVectorLayer3DRenderer( QgsAbstract3DSymbol *s )
  : mSymbol( s )
{
}

QgsVectorLayer3DRenderer::~QgsVectorLayer3DRenderer()
{
}

QgsVectorLayer3DRenderer *QgsVectorLayer3DRenderer::clone() const
{
  QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer( mSymbol ? mSymbol->clone() : nullptr );
  r->layerRef = layerRef;
  return r;
}

void QgsVectorLayer3DRenderer::setLayer( QgsVectorLayer *layer )
{
  layerRef = QgsMapLayerRef( layer );
}

QgsVectorLayer *QgsVectorLayer3DRenderer::layer() const
{
  return qobject_cast<QgsVectorLayer *>( layerRef.layer );
}

void QgsVectorLayer3DRenderer::setSymbol( QgsAbstract3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

const QgsAbstract3DSymbol *QgsVectorLayer3DRenderer::symbol() const
{
  return mSymbol.get();
}

Qt3DCore::QEntity *QgsVectorLayer3DRenderer::createEntity( const Qgs3DMapSettings &map ) const
{
  QgsVectorLayer *vl = layer();

  if ( !mSymbol || !vl )
    return nullptr;

  if ( mSymbol->type() == "polygon" )
    return new QgsPolygon3DSymbolEntity( map, vl, *static_cast<QgsPolygon3DSymbol *>( mSymbol.get() ) );
  else if ( mSymbol->type() == "point" )
    return new QgsPoint3DSymbolEntity( map, vl, *static_cast<QgsPoint3DSymbol *>( mSymbol.get() ) );
  else if ( mSymbol->type() == "line" )
    return new QgsLine3DSymbolEntity( map, vl, *static_cast<QgsLine3DSymbol *>( mSymbol.get() ) );
  else
    return nullptr;
}

void QgsVectorLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( "layer", layerRef.layerId );

  QDomElement elemSymbol = doc.createElement( "symbol" );
  if ( mSymbol )
  {
    elemSymbol.setAttribute( "type", mSymbol->type() );
    mSymbol->writeXml( elemSymbol, context );
  }
  elem.appendChild( elemSymbol );
}

void QgsVectorLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  layerRef = QgsMapLayerRef( elem.attribute( "layer" ) );

  QDomElement elemSymbol = elem.firstChildElement( "symbol" );
  QString symbolType = elemSymbol.attribute( "type" );
  QgsAbstract3DSymbol *symbol = nullptr;
  if ( symbolType == "polygon" )
    symbol = new QgsPolygon3DSymbol;
  else if ( symbolType == "point" )
    symbol = new QgsPoint3DSymbol;
  else if ( symbolType == "line" )
    symbol = new QgsLine3DSymbol;

  if ( symbol )
    symbol->readXml( elemSymbol, context );
  mSymbol.reset( symbol );
}

void QgsVectorLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
}
