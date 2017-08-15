#include "vectorlayer3drenderer.h"

#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "lineentity.h"
#include "pointentity.h"
#include "polygonentity.h"

#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"


VectorLayer3DRendererMetadata::VectorLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( "vector" )
{
}

QgsAbstract3DRenderer *VectorLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  VectorLayer3DRenderer *r = new VectorLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}


// ---------


VectorLayer3DRenderer::VectorLayer3DRenderer( QgsAbstract3DSymbol *s )
  : mSymbol( s )
{
}

VectorLayer3DRenderer::~VectorLayer3DRenderer()
{
}

VectorLayer3DRenderer *VectorLayer3DRenderer::clone() const
{
  VectorLayer3DRenderer *r = new VectorLayer3DRenderer( mSymbol ? mSymbol->clone() : nullptr );
  r->layerRef = layerRef;
  return r;
}

void VectorLayer3DRenderer::setLayer( QgsVectorLayer *layer )
{
  layerRef = QgsMapLayerRef( layer );
}

QgsVectorLayer *VectorLayer3DRenderer::layer() const
{
  return qobject_cast<QgsVectorLayer *>( layerRef.layer );
}

void VectorLayer3DRenderer::setSymbol( QgsAbstract3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

const QgsAbstract3DSymbol *VectorLayer3DRenderer::symbol() const
{
  return mSymbol.get();
}

Qt3DCore::QEntity *VectorLayer3DRenderer::createEntity( const Map3D &map ) const
{
  QgsVectorLayer *vl = layer();

  if ( !mSymbol || !vl )
    return nullptr;

  if ( mSymbol->type() == "polygon" )
    return new PolygonEntity( map, vl, *static_cast<QgsPolygon3DSymbol *>( mSymbol.get() ) );
  else if ( mSymbol->type() == "point" )
    return new PointEntity( map, vl, *static_cast<QgsPoint3DSymbol *>( mSymbol.get() ) );
  else if ( mSymbol->type() == "line" )
    return new LineEntity( map, vl, *static_cast<QgsLine3DSymbol *>( mSymbol.get() ) );
  else
    return nullptr;
}

void VectorLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
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

void VectorLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
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

void VectorLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
}
