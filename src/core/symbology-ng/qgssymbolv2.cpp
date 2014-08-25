/***************************************************************************
 qgssymbolv2.cpp
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"

#include "qgslinesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"

#include "qgslogger.h"
#include "qgsrendercontext.h" // for bigSymbolPreview

#include "qgsproject.h"
#include "qgsstylev2.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QSize>

#include <cmath>

QgsSymbolV2::QgsSymbolV2( SymbolType type, QgsSymbolLayerV2List layers )
    : mType( type ), mLayers( layers ), mAlpha( 1.0 ), mRenderHints( 0 ), mLayer( NULL )
{

  // check they're all correct symbol layers
  for ( int i = 0; i < mLayers.count(); i++ )
  {
    if ( mLayers[i] == NULL )
    {
      mLayers.removeAt( i-- );
    }
    else if ( !isSymbolLayerCompatible( mLayers[i]->type() ) )
    {
      delete mLayers[i];
      mLayers.removeAt( i-- );
    }
  }

}

QgsSymbolV2::~QgsSymbolV2()
{
  // delete all symbol layers (we own them, so it's okay)
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
    delete *it;
}

QgsSymbolV2::OutputUnit QgsSymbolV2::outputUnit() const
{
  if ( mLayers.empty() )
  {
    return QgsSymbolV2::Mixed;
  }

  QgsSymbolLayerV2List::const_iterator it = mLayers.constBegin();

  QgsSymbolV2::OutputUnit unit = ( *it )->outputUnit();

  for ( ; it != mLayers.constEnd(); ++it )
  {
    if (( *it )->outputUnit() != unit )
    {
      return QgsSymbolV2::Mixed;
    }
  }
  return unit;
}

QgsMapUnitScale QgsSymbolV2::mapUnitScale() const
{
  if ( mLayers.empty() )
  {
    return QgsMapUnitScale();
  }

  QgsSymbolLayerV2List::const_iterator it = mLayers.constBegin();
  if ( it == mLayers.constEnd() )
    return QgsMapUnitScale();

  QgsMapUnitScale scale = ( *it )->mapUnitScale();
  ++it;

  for ( ; it != mLayers.constEnd(); ++it )
  {
    if (( *it )->mapUnitScale() != scale )
    {
      return QgsMapUnitScale();
    }
  }
  return scale;
}

void QgsSymbolV2::setOutputUnit( QgsSymbolV2::OutputUnit u )
{
  QgsSymbolLayerV2List::iterator it = mLayers.begin();
  for ( ; it != mLayers.end(); ++it )
  {
    ( *it )->setOutputUnit( u );
  }
}

void QgsSymbolV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsSymbolLayerV2List::iterator it = mLayers.begin();
  for ( ; it != mLayers.end(); ++it )
  {
    ( *it )->setMapUnitScale( scale );
  }
}

QgsSymbolV2* QgsSymbolV2::defaultSymbol( QGis::GeometryType geomType )
{
  QgsSymbolV2* s = 0;

  // override global default if project has a default for this type
  QString defaultSymbol;
  switch ( geomType )
  {
    case QGis::Point :
      defaultSymbol = QgsProject::instance()->readEntry( "DefaultStyles", "/Marker", "" );
      break;
    case QGis::Line :
      defaultSymbol = QgsProject::instance()->readEntry( "DefaultStyles", "/Line", "" );
      break;
    case QGis::Polygon :
      defaultSymbol = QgsProject::instance()->readEntry( "DefaultStyles", "/Fill", "" );
      break;
    default: defaultSymbol = ""; break;
  }
  if ( defaultSymbol != "" )
    s = QgsStyleV2::defaultStyle()->symbol( defaultSymbol );

  // if no default found for this type, get global default (as previously)
  if ( ! s )
  {
    switch ( geomType )
    {
      case QGis::Point: s = new QgsMarkerSymbolV2(); break;
      case QGis::Line:  s = new QgsLineSymbolV2(); break;
      case QGis::Polygon: s = new QgsFillSymbolV2(); break;
      default: QgsDebugMsg( "unknown layer's geometry type" ); return NULL;
    }
  }

  // set alpha transparency
  s->setAlpha( QgsProject::instance()->readDoubleEntry( "DefaultStyles", "/AlphaInt", 255 ) / 255.0 );

  // set random color, it project prefs allow
  if ( defaultSymbol == "" ||
       QgsProject::instance()->readBoolEntry( "DefaultStyles", "/RandomColors", true ) )
  {
    s->setColor( QColor::fromHsv( rand() % 360, 64 + rand() % 192, 128 + rand() % 128 ) );
  }

  return s;
}

QgsSymbolLayerV2* QgsSymbolV2::symbolLayer( int layer )
{
  if ( layer < 0 || layer >= mLayers.count() )
    return NULL;

  return mLayers[layer];
}


bool QgsSymbolV2::isSymbolLayerCompatible( SymbolType t )
{
  // fill symbol can contain also line symbol layers for drawing of outlines
  if ( mType == Fill && t == Line )
    return true;

  return mType == t;
}


bool QgsSymbolV2::insertSymbolLayer( int index, QgsSymbolLayerV2* layer )
{
  if ( index < 0 || index > mLayers.count() ) // can be added also after the last index
    return false;
  if ( layer == NULL || !isSymbolLayerCompatible( layer->type() ) )
    return false;

  mLayers.insert( index, layer );
  return true;
}


bool QgsSymbolV2::appendSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer == NULL || !isSymbolLayerCompatible( layer->type() ) )
    return false;

  mLayers.append( layer );
  return true;
}


bool QgsSymbolV2::deleteSymbolLayer( int index )
{
  if ( index < 0 || index >= mLayers.count() )
    return false;

  delete mLayers[index];
  mLayers.removeAt( index );
  return true;
}


QgsSymbolLayerV2* QgsSymbolV2::takeSymbolLayer( int index )
{
  if ( index < 0 || index >= mLayers.count() )
    return NULL;

  return mLayers.takeAt( index );
}


bool QgsSymbolV2::changeSymbolLayer( int index, QgsSymbolLayerV2* layer )
{
  if ( index < 0 || index >= mLayers.count() )
    return false;
  if ( layer == NULL || !isSymbolLayerCompatible( layer->type() ) )
    return false;

  delete mLayers[index]; // first delete the original layer
  mLayers[index] = layer; // set new layer
  return true;
}


void QgsSymbolV2::startRender( QgsRenderContext& context, const QgsFields* fields )
{
  QgsSymbolV2RenderContext symbolContext( context, outputUnit(), mAlpha, false, mRenderHints, 0, fields, mapUnitScale() );


  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
    ( *it )->startRender( symbolContext );
}

void QgsSymbolV2::stopRender( QgsRenderContext& context )
{
  QgsSymbolV2RenderContext symbolContext( context, outputUnit(), mAlpha, false, mRenderHints, 0, 0, mapUnitScale() );

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
    ( *it )->stopRender( symbolContext );

  mLayer = NULL;
}

void QgsSymbolV2::setColor( const QColor& color )
{
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    if ( !( *it )->isLocked() )
      ( *it )->setColor( color );
  }
}

QColor QgsSymbolV2::color() const
{
  for ( QgsSymbolLayerV2List::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // return color of the first unlocked layer
    if ( !( *it )->isLocked() )
      return ( *it )->color();
  }
  return QColor( 0, 0, 0 );
}

void QgsSymbolV2::drawPreviewIcon( QPainter* painter, QSize size, QgsRenderContext* customContext )
{
  QgsRenderContext context = customContext ? *customContext : QgsSymbolLayerV2Utils::createRenderContext( painter );
  context.setForceVectorOutput( true );
  QgsSymbolV2RenderContext symbolContext( context, outputUnit(), mAlpha, false, mRenderHints, 0, 0, mapUnitScale() );

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    if ( mType == Fill && ( *it )->type() == Line )
    {
      // line symbol layer would normally draw just a line
      // so we override this case to force it to draw a polygon outline
      QgsLineSymbolLayerV2* lsl = ( QgsLineSymbolLayerV2* ) * it;

      // from QgsFillSymbolLayerV2::drawPreviewIcon()
      QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width() - 1, size.height() - 1 ) );
      lsl->startRender( symbolContext );
      lsl->renderPolygonOutline( poly, NULL, symbolContext );
      lsl->stopRender( symbolContext );
    }
    else
      ( *it )->drawPreviewIcon( symbolContext, size );
  }
}


QImage QgsSymbolV2::bigSymbolPreviewImage()
{
  QImage preview( QSize( 100, 100 ), QImage::Format_ARGB32_Premultiplied );
  preview.fill( 0 );

  QPainter p( &preview );
  p.setRenderHint( QPainter::Antialiasing );
  p.translate( 0.5, 0.5 ); // shift by half a pixel to avoid blurring due antialising

  if ( mType == QgsSymbolV2::Marker )
  {
    p.setPen( QPen( Qt::gray ) );
    p.drawLine( 0, 50, 100, 50 );
    p.drawLine( 50, 0, 50, 100 );
  }

  QgsRenderContext context = QgsSymbolLayerV2Utils::createRenderContext( &p );
  startRender( context );

  if ( mType == QgsSymbolV2::Line )
  {
    QPolygonF poly;
    poly << QPointF( 0, 50 ) << QPointF( 99, 50 );
    static_cast<QgsLineSymbolV2*>( this )->renderPolyline( poly, 0, context );
  }
  else if ( mType == QgsSymbolV2::Fill )
  {
    QPolygonF polygon;
    polygon << QPointF( 20, 20 ) << QPointF( 80, 20 ) << QPointF( 80, 80 ) << QPointF( 20, 80 ) << QPointF( 20, 20 );
    static_cast<QgsFillSymbolV2*>( this )->renderPolygon( polygon, NULL, 0, context );
  }
  else // marker
  {
    static_cast<QgsMarkerSymbolV2*>( this )->renderPoint( QPointF( 50, 50 ), 0, context );
  }

  stopRender( context );
  return preview;
}


QString QgsSymbolV2::dump() const
{
  QString t;
  switch ( type() )
  {
    case QgsSymbolV2::Marker: t = "MARKER"; break;
    case QgsSymbolV2::Line: t = "LINE"; break;
    case QgsSymbolV2::Fill: t = "FILL"; break;
    default: Q_ASSERT( 0 && "unknown symbol type" );
  }
  QString s = QString( "%1 SYMBOL (%2 layers) color %3" ).arg( t ).arg( mLayers.count() ).arg( QgsSymbolLayerV2Utils::encodeColor( color() ) );

  for ( QgsSymbolLayerV2List::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // TODO:
  }
  return s;
}

void QgsSymbolV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  props[ "alpha" ] = QString::number( alpha() );
  double scaleFactor = 1.0;
  props[ "uom" ] = QgsSymbolLayerV2Utils::encodeSldUom( outputUnit(), &scaleFactor );
  props[ "uomScale" ] = scaleFactor != 1 ? QString::number( scaleFactor ) : "";

  for ( QgsSymbolLayerV2List::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    ( *it )->toSld( doc, element, props );
  }
}

QgsSymbolLayerV2List QgsSymbolV2::cloneLayers() const
{
  QgsSymbolLayerV2List lst;
  for ( QgsSymbolLayerV2List::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsSymbolLayerV2* layer = ( *it )->clone();
    layer->setLocked(( *it )->isLocked() );
    layer->setRenderingPass(( *it )->renderingPass() );
    lst.append( layer );
  }
  return lst;
}

QSet<QString> QgsSymbolV2::usedAttributes() const
{
  QSet<QString> attributes;
  QgsSymbolLayerV2List::const_iterator sIt = mLayers.constBegin();
  for ( ; sIt != mLayers.constEnd(); ++sIt )
  {
    if ( *sIt )
    {
      attributes.unite(( *sIt )->usedAttributes() );
    }
  }
  return attributes;
}

////////////////////


QgsSymbolV2RenderContext::QgsSymbolV2RenderContext( QgsRenderContext& c, QgsSymbolV2::OutputUnit u, qreal alpha, bool selected, int renderHints, const QgsFeature* f, const QgsFields* fields, const QgsMapUnitScale& mapUnitScale )
    : mRenderContext( c ), mOutputUnit( u ), mMapUnitScale( mapUnitScale ), mAlpha( alpha ), mSelected( selected ), mRenderHints( renderHints ), mFeature( f ), mFields( fields )
{

}

QgsSymbolV2RenderContext::~QgsSymbolV2RenderContext()
{

}


double QgsSymbolV2RenderContext::outputLineWidth( double width ) const
{
  return width * QgsSymbolLayerV2Utils::lineWidthScaleFactor( mRenderContext, mOutputUnit, mMapUnitScale );
}

double QgsSymbolV2RenderContext::outputPixelSize( double size ) const
{
  return size * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( mRenderContext, mOutputUnit, mMapUnitScale );
}

QgsSymbolV2RenderContext& QgsSymbolV2RenderContext::operator=( const QgsSymbolV2RenderContext& )
{
  // This is just a dummy implementation of assignment.
  // sip 4.7 generates a piece of code that needs this function to exist.
  // It's not generated automatically by the compiler because of
  // mRenderContext member which is a reference (and thus can't be changed).
  Q_ASSERT( false );
  return *this;
}

///////////////////

QgsMarkerSymbolV2* QgsMarkerSymbolV2::createSimple( const QgsStringMap& properties )
{
  QgsSymbolLayerV2* sl = QgsSimpleMarkerSymbolLayerV2::create( properties );
  if ( sl == NULL )
    return NULL;

  QgsSymbolLayerV2List layers;
  layers.append( sl );
  return new QgsMarkerSymbolV2( layers );
}

QgsLineSymbolV2* QgsLineSymbolV2::createSimple( const QgsStringMap& properties )
{
  QgsSymbolLayerV2* sl = QgsSimpleLineSymbolLayerV2::create( properties );
  if ( sl == NULL )
    return NULL;

  QgsSymbolLayerV2List layers;
  layers.append( sl );
  return new QgsLineSymbolV2( layers );
}

QgsFillSymbolV2* QgsFillSymbolV2::createSimple( const QgsStringMap& properties )
{
  QgsSymbolLayerV2* sl = QgsSimpleFillSymbolLayerV2::create( properties );
  if ( sl == NULL )
    return NULL;

  QgsSymbolLayerV2List layers;
  layers.append( sl );
  return new QgsFillSymbolV2( layers );
}

///////////////////


QgsMarkerSymbolV2::QgsMarkerSymbolV2( QgsSymbolLayerV2List layers )
    : QgsSymbolV2( Marker, layers )
{
  if ( mLayers.count() == 0 )
    mLayers.append( new QgsSimpleMarkerSymbolLayerV2() );
}

void QgsMarkerSymbolV2::setAngle( double ang )
{
  double origAngle = angle();
  double angleDiff = ang - origAngle;
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsMarkerSymbolLayerV2* layer = ( QgsMarkerSymbolLayerV2* ) * it;
    layer->setAngle( layer->angle() + angleDiff );
  }
}

double QgsMarkerSymbolV2::angle()
{
  QgsSymbolLayerV2List::const_iterator it = mLayers.begin();

  if ( it == mLayers.end() )
    return 0;

  // return angle of the first symbol layer
  const QgsMarkerSymbolLayerV2* layer = static_cast<const QgsMarkerSymbolLayerV2 *>( *it );
  return layer->angle();
}

void QgsMarkerSymbolV2::setSize( double s )
{
  double origSize = size();

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2*>( *it );
    if ( layer->size() == origSize )
      layer->setSize( s );
    else
    {
      // proportionally scale size
      if ( origSize != 0 )
        layer->setSize( layer->size() * s / origSize );
    }
  }
}

double QgsMarkerSymbolV2::size()
{
  // return size of the largest symbol
  double maxSize = 0;
  for ( QgsSymbolLayerV2List::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    const QgsMarkerSymbolLayerV2* layer = static_cast<const QgsMarkerSymbolLayerV2 *>( *it );
    double lsize = layer->size();
    if ( lsize > maxSize )
      maxSize = lsize;
  }
  return maxSize;
}


void QgsMarkerSymbolV2::setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod )
{
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2*>( *it );
    layer->setScaleMethod( scaleMethod );
  }
}

QgsSymbolV2::ScaleMethod QgsMarkerSymbolV2::scaleMethod()
{
  QgsSymbolLayerV2List::const_iterator it = mLayers.begin();

  if ( it == mLayers.end() )
    return DEFAULT_SCALE_METHOD;

  // return scale method of the first symbol layer
  const QgsMarkerSymbolLayerV2* layer = static_cast<const QgsMarkerSymbolLayerV2 *>( *it );
  return layer->scaleMethod();
}

void QgsMarkerSymbolV2::renderPoint( const QPointF& point, const QgsFeature* f, QgsRenderContext& context, int layer, bool selected )
{
  QgsSymbolV2RenderContext symbolContext( context, outputUnit(), mAlpha, selected, mRenderHints, f, 0, mapUnitScale() );

  if ( layer != -1 )
  {
    if ( layer >= 0 && layer < mLayers.count() )
      (( QgsMarkerSymbolLayerV2* ) mLayers[layer] )->renderPoint( point, symbolContext );
    return;
  }

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsMarkerSymbolLayerV2* layer = ( QgsMarkerSymbolLayerV2* ) * it;
    layer->renderPoint( point, symbolContext );
  }
}

QgsSymbolV2* QgsMarkerSymbolV2::clone() const
{
  QgsSymbolV2* cloneSymbol = new QgsMarkerSymbolV2( cloneLayers() );
  cloneSymbol->setAlpha( mAlpha );
  return cloneSymbol;
}


///////////////////
// LINE

QgsLineSymbolV2::QgsLineSymbolV2( QgsSymbolLayerV2List layers )
    : QgsSymbolV2( Line, layers )
{
  if ( mLayers.count() == 0 )
    mLayers.append( new QgsSimpleLineSymbolLayerV2() );
}

void QgsLineSymbolV2::setWidth( double w )
{
  double origWidth = width();

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsLineSymbolLayerV2* layer = ( QgsLineSymbolLayerV2* ) * it;
    if ( layer->width() == origWidth )
    {
      layer->setWidth( w );
    }
    else
    {
      // proportionally scale the width
      if ( origWidth != 0 )
        layer->setWidth( layer->width() * w / origWidth );
    }
  }
}

double QgsLineSymbolV2::width()
{
  double maxWidth = 0;
  for ( QgsSymbolLayerV2List::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    const QgsLineSymbolLayerV2* layer = ( const QgsLineSymbolLayerV2* ) * it;
    double width = layer->width();
    if ( width > maxWidth )
      maxWidth = width;
  }
  return maxWidth;
}

void QgsLineSymbolV2::renderPolyline( const QPolygonF& points, const QgsFeature* f, QgsRenderContext& context, int layer, bool selected )
{
  QgsSymbolV2RenderContext symbolContext( context, outputUnit(), mAlpha, selected, mRenderHints, f, 0, mapUnitScale() );

  if ( layer != -1 )
  {
    if ( layer >= 0 && layer < mLayers.count() )
      (( QgsLineSymbolLayerV2* ) mLayers[layer] )->renderPolyline( points, symbolContext );
    return;
  }

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsLineSymbolLayerV2* layer = ( QgsLineSymbolLayerV2* ) * it;
    layer->renderPolyline( points, symbolContext );
  }
}


QgsSymbolV2* QgsLineSymbolV2::clone() const
{
  QgsSymbolV2* cloneSymbol = new QgsLineSymbolV2( cloneLayers() );
  cloneSymbol->setAlpha( mAlpha );
  return cloneSymbol;
}

///////////////////
// FILL

QgsFillSymbolV2::QgsFillSymbolV2( QgsSymbolLayerV2List layers )
    : QgsSymbolV2( Fill, layers )
{
  if ( mLayers.count() == 0 )
    mLayers.append( new QgsSimpleFillSymbolLayerV2() );
}

void QgsFillSymbolV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layer, bool selected )
{
  QgsSymbolV2RenderContext symbolContext( context, outputUnit(), mAlpha, selected, mRenderHints, f, 0, mapUnitScale() );

  if ( layer != -1 )
  {
    if ( layer >= 0 && layer < mLayers.count() )
    {
      QgsSymbolV2::SymbolType layertype = mLayers.at( layer )->type();
      if ( layertype == QgsSymbolV2::Fill )
        (( QgsFillSymbolLayerV2* ) mLayers[layer] )->renderPolygon( points, rings, symbolContext );
      else if ( layertype == QgsSymbolV2::Line )
        (( QgsLineSymbolLayerV2* ) mLayers[layer] )->renderPolygonOutline( points, rings, symbolContext );
    }
    return;
  }

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsSymbolV2::SymbolType layertype = ( *it )->type();
    if ( layertype == QgsSymbolV2::Fill )
    {
      QgsFillSymbolLayerV2* layer = ( QgsFillSymbolLayerV2* ) * it;
      layer->renderPolygon( points, rings, symbolContext );
    }
    else if ( layertype == QgsSymbolV2::Line )
    {
      QgsLineSymbolLayerV2* layer = ( QgsLineSymbolLayerV2* ) * it;
      layer->renderPolygonOutline( points, rings, symbolContext );
    }
  }
}


QgsSymbolV2* QgsFillSymbolV2::clone() const
{
  QgsSymbolV2* cloneSymbol = new QgsFillSymbolV2( cloneLayers() );
  cloneSymbol->setAlpha( mAlpha );
  return cloneSymbol;
}

void QgsFillSymbolV2::setAngle( double angle )
{
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsFillSymbolLayerV2* layer = ( QgsFillSymbolLayerV2* ) * it;
    layer->setAngle( angle );
  }
}


