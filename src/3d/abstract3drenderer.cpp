#include "abstract3drenderer.h"

#include "abstract3dsymbol.h"
#include "lineentity.h"
#include "pointentity.h"
#include "polygonentity.h"

#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"


VectorLayer3DRenderer::VectorLayer3DRenderer( Abstract3DSymbol *s )
  : mSymbol( s )
{
}

VectorLayer3DRenderer::~VectorLayer3DRenderer()
{
}

Abstract3DRenderer *VectorLayer3DRenderer::clone() const
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

void VectorLayer3DRenderer::setSymbol( Abstract3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

const Abstract3DSymbol *VectorLayer3DRenderer::symbol() const
{
  return mSymbol.get();
}

Qt3DCore::QEntity *VectorLayer3DRenderer::createEntity( const Map3D &map ) const
{
  QgsVectorLayer *vl = layer();

  if ( !mSymbol || !vl )
    return nullptr;

  if ( mSymbol->type() == "polygon" )
    return new PolygonEntity( map, vl, *static_cast<Polygon3DSymbol *>( mSymbol.get() ) );
  else if ( mSymbol->type() == "point" )
    return new PointEntity( map, vl, *static_cast<Point3DSymbol *>( mSymbol.get() ) );
  else if ( mSymbol->type() == "line" )
    return new LineEntity( map, vl, *static_cast<Line3DSymbol *>( mSymbol.get() ) );
  else
    return nullptr;
}

void VectorLayer3DRenderer::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( "layer", layerRef.layerId );

  QDomElement elemSymbol = doc.createElement( "symbol" );
  if ( mSymbol )
  {
    elemSymbol.setAttribute( "type", mSymbol->type() );
    mSymbol->writeXml( elemSymbol );
  }
  elem.appendChild( elemSymbol );
}

void VectorLayer3DRenderer::readXml( const QDomElement &elem )
{
  layerRef = QgsMapLayerRef( elem.attribute( "layer" ) );

  QDomElement elemSymbol = elem.firstChildElement( "symbol" );
  QString symbolType = elemSymbol.attribute( "type" );
  Abstract3DSymbol *symbol = nullptr;
  if ( symbolType == "polygon" )
    symbol = new Polygon3DSymbol;
  else if ( symbolType == "point" )
    symbol = new Point3DSymbol;
  else if ( symbolType == "line" )
    symbol = new Line3DSymbol;

  if ( symbol )
    symbol->readXml( elemSymbol );
  mSymbol.reset( symbol );
}

void VectorLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
}
