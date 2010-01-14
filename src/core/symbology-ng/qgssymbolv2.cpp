
#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"

#include "qgslinesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"

#include "qgslogger.h"
#include "qgsrendercontext.h" // for bigSymbolPreview

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QSize>

#include <cmath>

QgsSymbolV2::QgsSymbolV2( SymbolType type, QgsSymbolLayerV2List layers )
    : mType( type ), mLayers( layers ), mOutputUnit( MM )
{

  // check they're all correct symbol layers
  for ( int i = 0; i < mLayers.count(); i++ )
  {
    if ( mLayers[i] == NULL )
    {
      mLayers.removeAt( i-- );
    }
    else if ( mLayers[i]->type() != mType )
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

QgsSymbolV2* QgsSymbolV2::defaultSymbol( QGis::GeometryType geomType )
{
  QgsSymbolV2* s;
  switch ( geomType )
  {
    case QGis::Point: s = new QgsMarkerSymbolV2(); break;
    case QGis::Line:  s = new QgsLineSymbolV2(); break;
    case QGis::Polygon: s = new QgsFillSymbolV2(); break;
    default: QgsDebugMsg( "unknown layer's geometry type" ); return NULL;
  }

  s->setColor( QColor::fromHsv( rand() % 360, 64 + rand() % 192, 128 + rand() % 128 ) );
  return s;
}


QgsSymbolLayerV2* QgsSymbolV2::symbolLayer( int layer )
{
  if ( layer < 0 || layer >= mLayers.count() )
    return NULL;

  return mLayers[layer];
}



bool QgsSymbolV2::insertSymbolLayer( int index, QgsSymbolLayerV2* layer )
{
  if ( index < 0 || index > mLayers.count() ) // can be added also after the last index
    return false;
  if ( layer == NULL || layer->type() != mType )
    return false;

  mLayers.insert( index, layer );
  return true;
}


bool QgsSymbolV2::appendSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer == NULL || layer->type() != mType )
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
  if ( layer == NULL || layer->type() != mType )
    return false;

  delete mLayers[index]; // first delete the original layer
  mLayers[index] = layer; // set new layer
  return true;
}


void QgsSymbolV2::startRender( QgsRenderContext& context )
{
  QgsSymbolV2RenderContext symbolContext( &context, mOutputUnit );
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
    ( *it )->startRender( symbolContext );
}

void QgsSymbolV2::stopRender( QgsRenderContext& context )
{
  QgsSymbolV2RenderContext symbolContext( &context, mOutputUnit );
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
    ( *it )->stopRender( symbolContext );
}

void QgsSymbolV2::setColor( const QColor& color )
{
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    if ( !( *it )->isLocked() )
      ( *it )->setColor( color );
  }
}

QColor QgsSymbolV2::color()
{
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // return color of the first unlocked layer
    if ( !( *it )->isLocked() )
      return ( *it )->color();
  }
  return QColor( 0, 0, 0 );
}

void QgsSymbolV2::drawPreviewIcon( QPainter* painter, QSize size )
{
  QgsRenderContext context;
  context.setPainter( painter );
  QgsSymbolV2RenderContext symbolContext( &context, mOutputUnit );
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
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
    p.setPen( QPen( QColor( 230, 230, 230 ) ) );
    p.drawLine( 0, 50, 100, 50 );
    p.drawLine( 50, 0, 50, 100 );
  }

  QgsRenderContext context;
  context.setPainter( &p );

  startRender( context );

  if ( mType == QgsSymbolV2::Line )
  {
    QPolygonF poly;
    poly << QPointF( 0, 50 ) << QPointF( 99, 50 );
    static_cast<QgsLineSymbolV2*>( this )->renderPolyline( poly, context );
  }
  else if ( mType == QgsSymbolV2::Fill )
  {
    QPolygonF polygon;
    polygon << QPointF( 20, 20 ) << QPointF( 80, 20 ) << QPointF( 80, 80 ) << QPointF( 20, 80 ) << QPointF( 20, 20 );
    static_cast<QgsFillSymbolV2*>( this )->renderPolygon( polygon, NULL, context );
  }
  else // marker
  {
    static_cast<QgsMarkerSymbolV2*>( this )->renderPoint( QPointF( 50, 50 ), context );
  }

  stopRender( context );
  return preview;
}


QString QgsSymbolV2::dump()
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

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // TODO:
  }
  return s;
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

////////////////////

QgsSymbolV2RenderContext::QgsSymbolV2RenderContext( QgsRenderContext* c, QgsSymbolV2::OutputUnit u ): mRenderContext( c ), mOutputUnit( u )
{

}

QgsSymbolV2RenderContext::~QgsSymbolV2RenderContext()
{

}


///////////////////


QgsMarkerSymbolV2::QgsMarkerSymbolV2( QgsSymbolLayerV2List layers )
    : QgsSymbolV2( Marker, layers )
{
  if ( mLayers.count() == 0 )
    mLayers.append( new QgsSimpleMarkerSymbolLayerV2() );
}

void QgsMarkerSymbolV2::setAngle( double angle )
{
  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsMarkerSymbolLayerV2* layer = ( QgsMarkerSymbolLayerV2* ) * it;
    layer->setAngle( angle );
  }
}

double QgsMarkerSymbolV2::angle()
{
  QgsSymbolLayerV2List::const_iterator it = mLayers.begin();

  if ( it == mLayers.end() )
    return 0;

  // return angle of the first symbol layer
  const QgsMarkerSymbolLayerV2 *layer = static_cast<const QgsMarkerSymbolLayerV2 *>( *it );
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

void QgsMarkerSymbolV2::renderPoint( const QPointF& point, QgsRenderContext& context, int layer )
{
  QgsSymbolV2RenderContext symbolContext( &context, mOutputUnit );
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
  cloneSymbol->setOutputUnit( mOutputUnit );
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

void QgsLineSymbolV2::renderPolyline( const QPolygonF& points, QgsRenderContext& context, int layer )
{
  QgsSymbolV2RenderContext symbolContext( &context, mOutputUnit );
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
  cloneSymbol->setOutputUnit( mOutputUnit );
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

void QgsFillSymbolV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context, int layer )
{
  QgsSymbolV2RenderContext symbolContext( &context, mOutputUnit );
  if ( layer != -1 )
  {
    if ( layer >= 0 && layer < mLayers.count() )
      (( QgsFillSymbolLayerV2* ) mLayers[layer] )->renderPolygon( points, rings, symbolContext );
    return;
  }

  for ( QgsSymbolLayerV2List::iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsFillSymbolLayerV2* layer = ( QgsFillSymbolLayerV2* ) * it;
    layer->renderPolygon( points, rings, symbolContext );
  }
}


QgsSymbolV2* QgsFillSymbolV2::clone() const
{
  QgsSymbolV2* cloneSymbol = new QgsFillSymbolV2( cloneLayers() );
  cloneSymbol->setOutputUnit( mOutputUnit );
  return cloneSymbol;
}
