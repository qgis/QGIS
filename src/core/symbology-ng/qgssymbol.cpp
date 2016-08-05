/***************************************************************************
 qgssymbol.cpp
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

#include "qgssymbol.h"
#include "qgssymbollayer.h"

#include "qgslinesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgsgeometrygeneratorsymbollayerv2.h"

#include "qgslogger.h"
#include "qgsrendercontext.h" // for bigSymbolPreview

#include "qgsproject.h"
#include "qgsstylev2.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"

#include "qgsdatadefined.h"
#include "qgsvectorlayer.h"

#include "qgsgeometry.h"
#include "qgsmultipointv2.h"
#include "qgswkbptr.h"
#include "qgswkbsimplifierptr.h"
#include "qgsgeometrycollectionv2.h"
#include "qgsclipper.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QSvgGenerator>

#include <cmath>

inline
QgsDataDefined* rotateWholeSymbol( double additionalRotation, const QgsDataDefined& dd )
{
  QgsDataDefined* rotatedDD = new QgsDataDefined( dd );
  QString exprString = dd.useExpression() ? dd.expressionString() : dd.field();
  rotatedDD->setExpressionString( QString::number( additionalRotation ) + " + (" + exprString + ')' );
  rotatedDD->setUseExpression( true );
  return rotatedDD;
}

inline
QgsDataDefined* scaleWholeSymbol( double scaleFactor, const QgsDataDefined& dd )
{
  QgsDataDefined* scaledDD = new QgsDataDefined( dd );
  QString exprString = dd.useExpression() ? dd.expressionString() : dd.field();
  scaledDD->setExpressionString( QString::number( scaleFactor ) + "*(" + exprString + ')' );
  scaledDD->setUseExpression( true );
  return scaledDD;
}

inline
QgsDataDefined* scaleWholeSymbol( double scaleFactorX, double scaleFactorY, const QgsDataDefined& dd )
{
  QgsDataDefined* scaledDD = new QgsDataDefined( dd );
  QString exprString = dd.useExpression() ? dd.expressionString() : dd.field();
  scaledDD->setExpressionString(
    ( !qgsDoubleNear( scaleFactorX, 0.0 ) ? "tostring(" + QString::number( scaleFactorX ) + "*(" + exprString + "))" : "'0'" ) +
    "|| ',' || " +
    ( !qgsDoubleNear( scaleFactorY, 0.0 ) ? "tostring(" + QString::number( scaleFactorY ) + "*(" + exprString + "))" : "'0'" ) );
  scaledDD->setUseExpression( true );
  return scaledDD;
}


////////////////////

QgsSymbol::QgsSymbol( SymbolType type, const QgsSymbolLayerList& layers )
    : mType( type )
    , mLayers( layers )
    , mAlpha( 1.0 )
    , mRenderHints( 0 )
    , mClipFeaturesToExtent( true )
    , mLayer( nullptr )
    , mSymbolRenderContext( nullptr )
{

  // check they're all correct symbol layers
  for ( int i = 0; i < mLayers.count(); i++ )
  {
    if ( !mLayers.at( i ) )
    {
      mLayers.removeAt( i-- );
    }
    else if ( !mLayers.at( i )->isCompatibleWithSymbol( this ) )
    {
      delete mLayers.at( i );
      mLayers.removeAt( i-- );
    }
  }
}

QgsConstWkbPtr QgsSymbol::_getPoint( QPointF& pt, QgsRenderContext& context, QgsConstWkbPtr& wkbPtr )
{
  QgsWkbTypes::Type type = wkbPtr.readHeader();
  wkbPtr >> pt.rx() >> pt.ry();
  wkbPtr += ( QgsWkbTypes::coordDimensions( type ) - 2 ) * sizeof( double );

  if ( context.coordinateTransform().isValid() )
  {
    double z = 0; // dummy variable for coordinate transform
    context.coordinateTransform().transformInPlace( pt.rx(), pt.ry(), z );
  }

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  return wkbPtr;
}

QgsConstWkbPtr QgsSymbol::_getLineString( QPolygonF& pts, QgsRenderContext& context, QgsConstWkbPtr& wkbPtr, bool clipToExtent )
{
  QgsWkbTypes::Type wkbType = wkbPtr.readHeader();
  unsigned int nPoints;
  wkbPtr >> nPoints;

  QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel& mtp = context.mapToPixel();

  //apply clipping for large lines to achieve a better rendering performance
  if ( clipToExtent && nPoints > 1 )
  {
    const QgsRectangle& e = context.extent();
    double cw = e.width() / 10;
    double ch = e.height() / 10;
    QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );
    wkbPtr -= 1 + 2 * sizeof( int );
    wkbPtr = QgsClipper::clippedLineWKB( wkbPtr, clipRect, pts );
  }
  else
  {
    int skipZM = ( QgsWkbTypes::coordDimensions( wkbType ) - 2 ) * sizeof( double );
    Q_ASSERT( skipZM >= 0 );

    if ( static_cast<int>( nPoints * ( 2 * sizeof( double ) + skipZM ) ) > wkbPtr.remaining() )
    {
      QgsDebugMsg( QString( "%1 points exceed wkb length (%2>%3)" ).arg( nPoints ).arg( nPoints * ( 2 * sizeof( double ) + skipZM ) ).arg( wkbPtr.remaining() ) );
      return QgsConstWkbPtr( nullptr, 0 );
    }

    wkbPtr -= sizeof( unsigned int );
    wkbPtr >> pts;
    nPoints = pts.size();
  }

  //transform the QPolygonF to screen coordinates
  if ( ct.isValid() )
  {
    ct.transformPolygon( pts );
  }

  QPointF *ptr = pts.data();
  for ( int i = 0; i < pts.size(); ++i, ++ptr )
  {
    mtp.transformInPlace( ptr->rx(), ptr->ry() );
  }

  return wkbPtr;
}

QgsConstWkbPtr QgsSymbol::_getPolygon( QPolygonF &pts, QList<QPolygonF> &holes, QgsRenderContext &context, QgsConstWkbPtr& wkbPtr, bool clipToExtent )
{
  QgsWkbTypes::Type wkbType = wkbPtr.readHeader();
  unsigned int numRings;
  wkbPtr >> numRings;

  if ( numRings == 0 )  // sanity check for zero rings in polygon
    return wkbPtr;

  holes.clear();

  QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel& mtp = context.mapToPixel();
  const QgsRectangle& e = context.extent();
  double cw = e.width() / 10;
  double ch = e.height() / 10;
  QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );

  int skipZM = ( QgsWkbTypes::coordDimensions( wkbType ) - 2 ) * sizeof( double );
  Q_ASSERT( skipZM >= 0 );

  for ( unsigned int idx = 0; idx < numRings; idx++ )
  {
    unsigned int nPoints;
    wkbPtr >> nPoints;

    if ( static_cast<int>( nPoints * ( 2 * sizeof( double ) + skipZM ) ) > wkbPtr.remaining() )
    {
      QgsDebugMsg( QString( "%1 points exceed wkb length (%2>%3)" ).arg( nPoints ).arg( nPoints * ( 2 * sizeof( double ) + skipZM ) ).arg( wkbPtr.remaining() ) );
      return QgsConstWkbPtr( nullptr, 0 );
    }

    QPolygonF poly;
    wkbPtr -= sizeof( unsigned int );
    wkbPtr >> poly;
    nPoints = poly.size();

    if ( nPoints < 1 )
      continue;

    //clip close to view extent, if needed
    QRectF ptsRect = poly.boundingRect();
    if ( clipToExtent && !context.extent().contains( ptsRect ) )
    {
      QgsClipper::trimPolygon( poly, clipRect );
    }

    //transform the QPolygonF to screen coordinates
    if ( ct.isValid() )
    {
      ct.transformPolygon( poly );
    }

    QPointF *ptr = poly.data();
    for ( int i = 0; i < poly.size(); ++i, ++ptr )
    {
      mtp.transformInPlace( ptr->rx(), ptr->ry() );
    }

    if ( idx == 0 )
      pts = poly;
    else
      holes.append( poly );
  }

  return wkbPtr;
}

QgsSymbol::~QgsSymbol()
{
  delete mSymbolRenderContext;
  // delete all symbol layers (we own them, so it's okay)
  qDeleteAll( mLayers );
}

QgsUnitTypes::RenderUnit QgsSymbol::outputUnit() const
{
  if ( mLayers.empty() )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }

  QgsSymbolLayerList::const_iterator it = mLayers.constBegin();

  QgsUnitTypes::RenderUnit unit = ( *it )->outputUnit();

  for ( ; it != mLayers.constEnd(); ++it )
  {
    if (( *it )->outputUnit() != unit )
    {
      return QgsUnitTypes::RenderUnknownUnit;
    }
  }
  return unit;
}

QgsMapUnitScale QgsSymbol::mapUnitScale() const
{
  if ( mLayers.empty() )
  {
    return QgsMapUnitScale();
  }

  QgsSymbolLayerList::const_iterator it = mLayers.constBegin();
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

void QgsSymbol::setOutputUnit( QgsUnitTypes::RenderUnit u )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    layer->setOutputUnit( u );
  }
}

void QgsSymbol::setMapUnitScale( const QgsMapUnitScale &scale )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    layer->setMapUnitScale( scale );
  }
}

QgsSymbol* QgsSymbol::defaultSymbol( QgsWkbTypes::GeometryType geomType )
{
  QgsSymbol* s = nullptr;

  // override global default if project has a default for this type
  QString defaultSymbol;
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry :
      defaultSymbol = QgsProject::instance()->readEntry( "DefaultStyles", "/Marker", "" );
      break;
    case QgsWkbTypes::LineGeometry :
      defaultSymbol = QgsProject::instance()->readEntry( "DefaultStyles", "/Line", "" );
      break;
    case QgsWkbTypes::PolygonGeometry :
      defaultSymbol = QgsProject::instance()->readEntry( "DefaultStyles", "/Fill", "" );
      break;
    default:
      defaultSymbol = "";
      break;
  }
  if ( defaultSymbol != "" )
    s = QgsStyleV2::defaultStyle()->symbol( defaultSymbol );

  // if no default found for this type, get global default (as previously)
  if ( ! s )
  {
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        s = new QgsMarkerSymbolV2();
        break;
      case QgsWkbTypes::LineGeometry:
        s = new QgsLineSymbolV2();
        break;
      case QgsWkbTypes::PolygonGeometry:
        s = new QgsFillSymbolV2();
        break;
      default:
        QgsDebugMsg( "unknown layer's geometry type" );
        return nullptr;
    }
  }

  // set alpha transparency
  s->setAlpha( QgsProject::instance()->readDoubleEntry( "DefaultStyles", "/AlphaInt", 255 ) / 255.0 );

  // set random color, it project prefs allow
  if ( defaultSymbol == "" ||
       QgsProject::instance()->readBoolEntry( "DefaultStyles", "/RandomColors", true ) )
  {
    s->setColor( QColor::fromHsv( qrand() % 360, 64 + qrand() % 192, 128 + qrand() % 128 ) );
  }

  return s;
}

QgsSymbolLayer* QgsSymbol::symbolLayer( int layer )
{
  return mLayers.value( layer );
}


bool QgsSymbol::isSymbolLayerCompatible( SymbolType layerType )
{
  // fill symbol can contain also line symbol layers for drawing of outlines
  if ( mType == Fill && layerType == Line )
    return true;

  return mType == layerType;
}


bool QgsSymbol::insertSymbolLayer( int index, QgsSymbolLayer* layer )
{
  if ( index < 0 || index > mLayers.count() ) // can be added also after the last index
    return false;

  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  mLayers.insert( index, layer );
  return true;
}


bool QgsSymbol::appendSymbolLayer( QgsSymbolLayer* layer )
{
  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  mLayers.append( layer );
  return true;
}


bool QgsSymbol::deleteSymbolLayer( int index )
{
  if ( index < 0 || index >= mLayers.count() )
    return false;

  delete mLayers.at( index );
  mLayers.removeAt( index );
  return true;
}


QgsSymbolLayer* QgsSymbol::takeSymbolLayer( int index )
{
  if ( index < 0 || index >= mLayers.count() )
    return nullptr;

  return mLayers.takeAt( index );
}


bool QgsSymbol::changeSymbolLayer( int index, QgsSymbolLayer* layer )
{
  QgsSymbolLayer* oldLayer = mLayers.value( index );

  if ( oldLayer == layer )
    return false;

  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  delete oldLayer; // first delete the original layer
  mLayers[index] = layer; // set new layer
  return true;
}


void QgsSymbol::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  delete mSymbolRenderContext;
  mSymbolRenderContext = new QgsSymbolRenderContext( context, outputUnit(), mAlpha, false, mRenderHints, nullptr, fields, mapUnitScale() );

  QgsSymbolRenderContext symbolContext( context, outputUnit(), mAlpha, false, mRenderHints, nullptr, fields, mapUnitScale() );

  QgsExpressionContextScope* scope = QgsExpressionContextUtils::updateSymbolScope( this, new QgsExpressionContextScope() );

  mSymbolRenderContext->setExpressionContextScope( scope );

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
    layer->startRender( symbolContext );
}

void QgsSymbol::stopRender( QgsRenderContext& context )
{
  Q_UNUSED( context )
  if ( mSymbolRenderContext )
  {
    Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
      layer->stopRender( *mSymbolRenderContext );
  }

  delete mSymbolRenderContext;
  mSymbolRenderContext = nullptr;

  mLayer = nullptr;
}

void QgsSymbol::setColor( const QColor& color )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( !layer->isLocked() )
      layer->setColor( color );
  }
}

QColor QgsSymbol::color() const
{
  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // return color of the first unlocked layer
    if ( !( *it )->isLocked() )
      return ( *it )->color();
  }
  return QColor( 0, 0, 0 );
}

void QgsSymbol::drawPreviewIcon( QPainter* painter, QSize size, QgsRenderContext* customContext )
{
  QgsRenderContext context = customContext ? *customContext : QgsSymbolLayerUtils::createRenderContext( painter );
  context.setForceVectorOutput( true );
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mAlpha, false, mRenderHints, nullptr, QgsFields(), mapUnitScale() );

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( mType == Fill && layer->type() == Line )
    {
      // line symbol layer would normally draw just a line
      // so we override this case to force it to draw a polygon outline
      QgsLineSymbolLayerV2* lsl = dynamic_cast<QgsLineSymbolLayerV2*>( layer );

      if ( lsl )
      {
        // from QgsFillSymbolLayerV2::drawPreviewIcon()
        QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width() - 1, size.height() - 1 ) );
        lsl->startRender( symbolContext );
        lsl->renderPolygonOutline( poly, nullptr, symbolContext );
        lsl->stopRender( symbolContext );
      }
    }
    else
      layer->drawPreviewIcon( symbolContext, size );
  }
}

void QgsSymbol::exportImage( const QString& path, const QString& format, QSize size )
{
  if ( format.toLower() == "svg" )
  {
    QSvgGenerator generator;
    generator.setFileName( path );
    generator.setSize( size );
    generator.setViewBox( QRect( 0, 0, size.height(), size.height() ) );

    QPainter painter( &generator );
    drawPreviewIcon( &painter, size );
    painter.end();
  }
  else
  {
    QImage image = asImage( size );
    image.save( path );
  }
}

QImage QgsSymbol::asImage( QSize size, QgsRenderContext* customContext )
{
  QImage image( size, QImage::Format_ARGB32_Premultiplied );
  image.fill( 0 );

  QPainter p( &image );
  p.setRenderHint( QPainter::Antialiasing );

  drawPreviewIcon( &p, size, customContext );

  return image;
}


QImage QgsSymbol::bigSymbolPreviewImage( QgsExpressionContext* expressionContext )
{
  QImage preview( QSize( 100, 100 ), QImage::Format_ARGB32_Premultiplied );
  preview.fill( 0 );

  QPainter p( &preview );
  p.setRenderHint( QPainter::Antialiasing );
  p.translate( 0.5, 0.5 ); // shift by half a pixel to avoid blurring due antialising

  if ( mType == QgsSymbol::Marker )
  {
    p.setPen( QPen( Qt::gray ) );
    p.drawLine( 0, 50, 100, 50 );
    p.drawLine( 50, 0, 50, 100 );
  }

  QgsRenderContext context = QgsSymbolLayerUtils::createRenderContext( &p );
  if ( expressionContext )
    context.setExpressionContext( *expressionContext );

  startRender( context );

  if ( mType == QgsSymbol::Line )
  {
    QPolygonF poly;
    poly << QPointF( 0, 50 ) << QPointF( 99, 50 );
    static_cast<QgsLineSymbolV2*>( this )->renderPolyline( poly, nullptr, context );
  }
  else if ( mType == QgsSymbol::Fill )
  {
    QPolygonF polygon;
    polygon << QPointF( 20, 20 ) << QPointF( 80, 20 ) << QPointF( 80, 80 ) << QPointF( 20, 80 ) << QPointF( 20, 20 );
    static_cast<QgsFillSymbolV2*>( this )->renderPolygon( polygon, nullptr, nullptr, context );
  }
  else // marker
  {
    static_cast<QgsMarkerSymbolV2*>( this )->renderPoint( QPointF( 50, 50 ), nullptr, context );
  }

  stopRender( context );
  return preview;
}


QString QgsSymbol::dump() const
{
  QString t;
  switch ( type() )
  {
    case QgsSymbol::Marker:
      t = "MARKER";
      break;
    case QgsSymbol::Line:
      t = "LINE";
      break;
    case QgsSymbol::Fill:
      t = "FILL";
      break;
    default:
      Q_ASSERT( 0 && "unknown symbol type" );
  }
  QString s = QString( "%1 SYMBOL (%2 layers) color %3" ).arg( t ).arg( mLayers.count() ).arg( QgsSymbolLayerUtils::encodeColor( color() ) );

  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // TODO:
  }
  return s;
}

void QgsSymbol::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  props[ "alpha" ] = QString::number( alpha() );
  double scaleFactor = 1.0;
  props[ "uom" ] = QgsSymbolLayerUtils::encodeSldUom( outputUnit(), &scaleFactor );
  props[ "uomScale" ] = ( !qgsDoubleNear( scaleFactor, 1.0 ) ? qgsDoubleToString( scaleFactor ) : "" );

  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    ( *it )->toSld( doc, element, props );
  }
}

QgsSymbolLayerList QgsSymbol::cloneLayers() const
{
  QgsSymbolLayerList lst;
  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsSymbolLayer* layer = ( *it )->clone();
    layer->setLocked(( *it )->isLocked() );
    layer->setRenderingPass(( *it )->renderingPass() );
    lst.append( layer );
  }
  return lst;
}

void QgsSymbol::renderUsingLayer( QgsSymbolLayer* layer, QgsSymbolRenderContext& context )
{
  Q_ASSERT( layer->type() == Hybrid );

  QgsGeometryGeneratorSymbolLayerV2* generatorLayer = static_cast<QgsGeometryGeneratorSymbolLayerV2*>( layer );

  QgsPaintEffect* effect = generatorLayer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QPainter* p = context.renderContext().painter();
    p->save();

    effect->begin( context.renderContext() );
    generatorLayer->render( context );
    effect->end( context.renderContext() );

    p->restore();
  }
  else
  {
    generatorLayer->render( context );
  }
}

QSet<QString> QgsSymbol::usedAttributes() const
{
  QSet<QString> attributes;
  QgsSymbolLayerList::const_iterator sIt = mLayers.constBegin();
  for ( ; sIt != mLayers.constEnd(); ++sIt )
  {
    if ( *sIt )
    {
      attributes.unite(( *sIt )->usedAttributes() );
    }
  }
  return attributes;
}

bool QgsSymbol::hasDataDefinedProperties() const
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->hasDataDefinedProperties() )
      return true;
  }
  return false;
}

void QgsSymbol::renderFeature( const QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker, int currentVertexMarkerType, int currentVertexMarkerSize )
{
  QgsGeometry geom = feature.geometry();
  if ( geom.isEmpty() )
  {
    return;
  }

  QgsGeometry segmentizedGeometry = geom;
  bool usingSegmentizedGeometry = false;
  context.setGeometry( geom.geometry() );

  bool tileMapRendering = context.testFlag( QgsRenderContext::RenderMapTile );

  //convert curve types to normal point/line/polygon ones
  if ( QgsWkbTypes::isCurvedType( geom.geometry()->wkbType() ) )
  {
    QgsAbstractGeometryV2 *g = geom.geometry()->segmentize( context.segmentationTolerance(), context.segmentationToleranceType() );
    if ( !g )
    {
      return;
    }
    segmentizedGeometry = QgsGeometry( g );
    usingSegmentizedGeometry = true;
  }

  mSymbolRenderContext->setGeometryPartCount( segmentizedGeometry.geometry()->partCount() );
  mSymbolRenderContext->setGeometryPartNum( 1 );

  if ( mSymbolRenderContext->expressionContextScope() )
  {
    context.expressionContext().appendScope( mSymbolRenderContext->expressionContextScope() );
    QgsExpressionContextUtils::updateSymbolScope( this, mSymbolRenderContext->expressionContextScope() );
    mSymbolRenderContext->expressionContextScope()->setVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT, mSymbolRenderContext->geometryPartCount() );
    mSymbolRenderContext->expressionContextScope()->setVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, 1 );
  }

  // Collection of markers to paint, only used for no curve types.
  QPolygonF markers;

  switch ( QgsWkbTypes::flatType( segmentizedGeometry.geometry()->wkbType() ) )
  {
    case QgsWkbTypes::Point:
    {
      QPointF pt;
      if ( mType != QgsSymbol::Marker )
      {
        QgsDebugMsg( "point can be drawn only with marker symbol!" );
        break;
      }

      const QgsPointV2* point = static_cast< const QgsPointV2* >( segmentizedGeometry.geometry() );
      _getPoint( pt, context, point );
      static_cast<QgsMarkerSymbolV2*>( this )->renderPoint( pt, &feature, context, layer, selected );

      if ( context.testFlag( QgsRenderContext::DrawSymbolBounds ) )
      {
        //draw debugging rect
        context.painter()->setPen( Qt::red );
        context.painter()->setBrush( QColor( 255, 0, 0, 100 ) );
        context.painter()->drawRect( static_cast<QgsMarkerSymbolV2*>( this )->bounds( pt, context, feature ) );
      }

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers << pt;
      }
    }
    break;
    case QgsWkbTypes::LineString:
    {
      QPolygonF pts;
      if ( mType != QgsSymbol::Line )
      {
        QgsDebugMsg( "linestring can be drawn only with line symbol!" );
        break;
      }
      QgsConstWkbSimplifierPtr wkbPtr( segmentizedGeometry.asWkb(), segmentizedGeometry.wkbSize(), context.vectorSimplifyMethod() );
      _getLineString( pts, context, wkbPtr, !tileMapRendering && clipFeaturesToExtent() );
      static_cast<QgsLineSymbolV2*>( this )->renderPolyline( pts, &feature, context, layer, selected );

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers = pts;
      }
    }
    break;
    case QgsWkbTypes::Polygon:
    {
      QPolygonF pts;
      QList<QPolygonF> holes;
      if ( mType != QgsSymbol::Fill )
      {
        QgsDebugMsg( "polygon can be drawn only with fill symbol!" );
        break;
      }
      QgsConstWkbSimplifierPtr wkbPtr( segmentizedGeometry.asWkb(), segmentizedGeometry.wkbSize(), context.vectorSimplifyMethod() );
      _getPolygon( pts, holes, context, wkbPtr, !tileMapRendering && clipFeaturesToExtent() );
      static_cast<QgsFillSymbolV2*>( this )->renderPolygon( pts, ( !holes.isEmpty() ? &holes : nullptr ), &feature, context, layer, selected );

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers = pts;

        Q_FOREACH ( const QPolygonF& hole, holes )
        {
          markers << hole;
        }
      }
    }
    break;

    case QgsWkbTypes::MultiPoint:
    {
      QPointF pt;

      if ( mType != QgsSymbol::Marker )
      {
        QgsDebugMsg( "multi-point can be drawn only with marker symbol!" );
        break;
      }

      QgsMultiPointV2* mp = static_cast< QgsMultiPointV2* >( segmentizedGeometry.geometry() );

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers.reserve( mp->numGeometries() );
      }

      for ( int i = 0; i < mp->numGeometries(); ++i )
      {
        mSymbolRenderContext->setGeometryPartNum( i + 1 );
        mSymbolRenderContext->expressionContextScope()->setVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, i + 1 );

        const QgsPointV2* point = static_cast< const QgsPointV2* >( mp->geometryN( i ) );
        _getPoint( pt, context, point );
        static_cast<QgsMarkerSymbolV2*>( this )->renderPoint( pt, &feature, context, layer, selected );

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          markers.append( pt );
        }
      }
    }
    break;

    case QgsWkbTypes::MultiCurve:
    case QgsWkbTypes::MultiLineString:
    {
      QPolygonF pts;

      if ( mType != QgsSymbol::Line )
      {
        QgsDebugMsg( "multi-linestring can be drawn only with line symbol!" );
        break;
      }

      QgsConstWkbSimplifierPtr wkbPtr( segmentizedGeometry.asWkb(), segmentizedGeometry.wkbSize(), context.vectorSimplifyMethod() );
      wkbPtr.readHeader();

      unsigned int num;
      wkbPtr >> num;

      const QgsGeometryCollectionV2* geomCollection = dynamic_cast<const QgsGeometryCollectionV2*>( geom.geometry() );

      for ( unsigned int i = 0; i < num && wkbPtr; ++i )
      {
        mSymbolRenderContext->setGeometryPartNum( i + 1 );
        mSymbolRenderContext->expressionContextScope()->setVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, i + 1 );

        if ( geomCollection )
        {
          context.setGeometry( geomCollection->geometryN( i ) );
        }
        if ( _getLineString( pts, context, wkbPtr, !tileMapRendering && clipFeaturesToExtent() ) == nullptr )
        {
          break;
        }
        static_cast<QgsLineSymbolV2*>( this )->renderPolyline( pts, &feature, context, layer, selected );

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          if ( i == 0 )
          {
            markers = pts;
          }
          else
          {
            markers << pts;
          }
        }
      }
    }
    break;

    case QgsWkbTypes::MultiSurface:
    case QgsWkbTypes::MultiPolygon:
    {
      if ( mType != QgsSymbol::Fill )
      {
        QgsDebugMsg( "multi-polygon can be drawn only with fill symbol!" );
        break;
      }

      QgsConstWkbSimplifierPtr wkbPtr( segmentizedGeometry.asWkb(), segmentizedGeometry.wkbSize(), context.vectorSimplifyMethod() );
      wkbPtr.readHeader();

      unsigned int num;
      wkbPtr >> num;

      QPolygonF pts;
      QList<QPolygonF> holes;

      const QgsGeometryCollectionV2* geomCollection = dynamic_cast<const QgsGeometryCollectionV2*>( geom.geometry() );

      for ( unsigned int i = 0; i < num && wkbPtr; ++i )
      {
        mSymbolRenderContext->setGeometryPartNum( i + 1 );
        mSymbolRenderContext->expressionContextScope()->setVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, i + 1 );

        if ( geomCollection )
        {
          context.setGeometry( geomCollection->geometryN( i ) );
        }
        if ( _getPolygon( pts, holes, context, wkbPtr, !tileMapRendering && clipFeaturesToExtent() ) == nullptr )
        {
          break;
        }
        static_cast<QgsFillSymbolV2*>( this )->renderPolygon( pts, ( !holes.isEmpty() ? &holes : nullptr ), &feature, context, layer, selected );

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          if ( i == 0 )
          {
            markers = pts;
          }
          else
          {
            markers << pts;
          }

          Q_FOREACH ( const QPolygonF& hole, holes )
          {
            markers << hole;
          }
        }
      }
      break;
    }
    case QgsWkbTypes::GeometryCollection:
    {
      QgsConstWkbPtr wkbPtr( segmentizedGeometry.asWkb(), segmentizedGeometry.wkbSize() );
      wkbPtr.readHeader();

      int nGeometries;
      wkbPtr >> nGeometries;

      if ( nGeometries == 0 )
      {
        // skip noise from empty geometry collections from simplification
        break;
      }

      FALLTHROUGH;
    }
    default:
      QgsDebugMsg( QString( "feature %1: unsupported wkb type %2/%3 for rendering" )
                   .arg( feature.id() )
                   .arg( QgsWkbTypes::displayString( geom.geometry()->wkbType() ) )
                   .arg( geom.wkbType(), 0, 16 ) );
  }

  if ( drawVertexMarker )
  {
    if ( markers.size() > 0 )
    {
      Q_FOREACH ( QPointF marker, markers )
      {
        renderVertexMarker( marker, context, currentVertexMarkerType, currentVertexMarkerSize );
      }
    }
    else
    {
      QgsCoordinateTransform ct = context.coordinateTransform();
      const QgsMapToPixel& mtp = context.mapToPixel();

      QgsPointV2 vertexPoint;
      QgsVertexId vertexId;
      double x, y, z;
      QPointF mapPoint;
      while ( geom.geometry()->nextVertex( vertexId, vertexPoint ) )
      {
        //transform
        x = vertexPoint.x();
        y = vertexPoint.y();
        z = 0.0;
        if ( ct.isValid() )
        {
          ct.transformInPlace( x, y, z );
        }
        mapPoint.setX( x );
        mapPoint.setY( y );
        mtp.transformInPlace( mapPoint.rx(), mapPoint.ry() );
        renderVertexMarker( mapPoint, context, currentVertexMarkerType, currentVertexMarkerSize );
      }
    }
  }

  if ( mSymbolRenderContext->expressionContextScope() )
    context.expressionContext().popScope();
}

QgsSymbolRenderContext* QgsSymbol::symbolRenderContext()
{
  return mSymbolRenderContext;
}

void QgsSymbol::renderVertexMarker( QPointF pt, QgsRenderContext& context, int currentVertexMarkerType, int currentVertexMarkerSize )
{
  QgsVectorLayer::drawVertexMarker( pt.x(), pt.y(), *context.painter(), static_cast< QgsVectorLayer::VertexMarkerType >( currentVertexMarkerType ), currentVertexMarkerSize );
}

////////////////////


QgsSymbolRenderContext::QgsSymbolRenderContext( QgsRenderContext& c, QgsUnitTypes::RenderUnit u, qreal alpha, bool selected, int renderHints, const QgsFeature* f, const QgsFields& fields, const QgsMapUnitScale& mapUnitScale )
    : mRenderContext( c )
    , mExpressionContextScope( nullptr )
    , mOutputUnit( u )
    , mMapUnitScale( mapUnitScale )
    , mAlpha( alpha )
    , mSelected( selected )
    , mRenderHints( renderHints )
    , mFeature( f )
    , mFields( fields )
    , mGeometryPartCount( 0 )
    , mGeometryPartNum( 0 )
{
}

QgsSymbolRenderContext::~QgsSymbolRenderContext()
{
  delete mExpressionContextScope;
}

void QgsSymbolRenderContext::setOriginalValueVariable( const QVariant& value )
{
  mRenderContext.expressionContext().setOriginalValueVariable( value );
}

double QgsSymbolRenderContext::outputLineWidth( double width ) const
{
  return QgsSymbolLayerUtils::convertToPainterUnits( mRenderContext, width, mOutputUnit, mMapUnitScale );
}

double QgsSymbolRenderContext::outputPixelSize( double size ) const
{
  return size * QgsSymbolLayerUtils::pixelSizeScaleFactor( mRenderContext, mOutputUnit, mMapUnitScale );
}

QgsSymbolRenderContext& QgsSymbolRenderContext::operator=( const QgsSymbolRenderContext& )
{
  // This is just a dummy implementation of assignment.
  // sip 4.7 generates a piece of code that needs this function to exist.
  // It's not generated automatically by the compiler because of
  // mRenderContext member which is a reference (and thus can't be changed).
  Q_ASSERT( false );
  return *this;
}

QgsExpressionContextScope* QgsSymbolRenderContext::expressionContextScope()
{
  return mExpressionContextScope;
}

void QgsSymbolRenderContext::setExpressionContextScope( QgsExpressionContextScope* contextScope )
{
  mExpressionContextScope = contextScope;
}

///////////////////

QgsMarkerSymbolV2* QgsMarkerSymbolV2::createSimple( const QgsStringMap& properties )
{
  QgsSymbolLayer* sl = QgsSimpleMarkerSymbolLayerV2::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsMarkerSymbolV2( layers );
}

QgsLineSymbolV2* QgsLineSymbolV2::createSimple( const QgsStringMap& properties )
{
  QgsSymbolLayer* sl = QgsSimpleLineSymbolLayerV2::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsLineSymbolV2( layers );
}

QgsFillSymbolV2* QgsFillSymbolV2::createSimple( const QgsStringMap& properties )
{
  QgsSymbolLayer* sl = QgsSimpleFillSymbolLayerV2::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsFillSymbolV2( layers );
}

///////////////////

QgsMarkerSymbolV2::QgsMarkerSymbolV2( const QgsSymbolLayerList& layers )
    : QgsSymbol( Marker, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleMarkerSymbolLayerV2() );
}

void QgsMarkerSymbolV2::setAngle( double ang )
{
  double origAngle = angle();
  double angleDiff = ang - origAngle;
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    QgsMarkerSymbolLayerV2* markerLayer = dynamic_cast<QgsMarkerSymbolLayerV2*>( layer );
    if ( markerLayer )
      markerLayer->setAngle( markerLayer->angle() + angleDiff );
  }
}

double QgsMarkerSymbolV2::angle() const
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );
    return markerLayer->angle();
  }
  return 0;
}

void QgsMarkerSymbolV2::setLineAngle( double lineAng )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( layer );
    markerLayer->setLineAngle( lineAng );
  }
}

void QgsMarkerSymbolV2::setDataDefinedAngle( const QgsDataDefined& dd )
{
  const double symbolRotation = angle();

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );
    if ( dd.hasDefaultValues() )
    {
      layer->removeDataDefinedProperty( "angle" );
    }
    else
    {
      if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) )
      {
        layer->setDataDefinedProperty( "angle", new QgsDataDefined( dd ) );
      }
      else
      {
        QgsDataDefined* rotatedDD = rotateWholeSymbol( markerLayer->angle() - symbolRotation, dd );
        layer->setDataDefinedProperty( "angle", rotatedDD );
      }
    }
  }
}

QgsDataDefined QgsMarkerSymbolV2::dataDefinedAngle() const
{
  const double symbolRotation = angle();
  QgsDataDefined* symbolDD = nullptr;

  // find the base of the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );
    if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) && markerLayer->getDataDefinedProperty( "angle" ) )
    {
      symbolDD = markerLayer->getDataDefinedProperty( "angle" );
      break;
    }
  }

  if ( !symbolDD )
    return QgsDataDefined();

  // check that all layer's angle expressions match the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );
    QgsDataDefined* layerAngleDD = markerLayer->getDataDefinedProperty( "angle" );

    if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) )
    {
      if ( !layerAngleDD || *layerAngleDD != *symbolDD )
        return QgsDataDefined();
    }
    else
    {
      QScopedPointer< QgsDataDefined > rotatedDD( rotateWholeSymbol( markerLayer->angle() - symbolRotation, *symbolDD ) );
      if ( !layerAngleDD || *layerAngleDD != *( rotatedDD.data() ) )
        return QgsDataDefined();
    }
  }
  return QgsDataDefined( *symbolDD );
}


void QgsMarkerSymbolV2::setSize( double s )
{
  double origSize = size();

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( layer );
    if ( qgsDoubleNear( markerLayer->size(), origSize ) )
      markerLayer->setSize( s );
    else if ( !qgsDoubleNear( origSize, 0.0 ) )
    {
      // proportionally scale size
      markerLayer->setSize( markerLayer->size() * s / origSize );
    }
    // also scale offset to maintain relative position
    if ( !qgsDoubleNear( origSize, 0.0 ) && ( !qgsDoubleNear( markerLayer->offset().x(), 0.0 ) || !qgsDoubleNear( markerLayer->offset().y(), 0.0 ) ) )
      markerLayer->setOffset( QPointF( markerLayer->offset().x() * s / origSize,
                                       markerLayer->offset().y() * s / origSize ) );
  }
}

double QgsMarkerSymbolV2::size() const
{
  // return size of the largest symbol
  double maxSize = 0;
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );
    double lsize = markerLayer->size();
    if ( lsize > maxSize )
      maxSize = lsize;
  }
  return maxSize;
}

void QgsMarkerSymbolV2::setSizeUnit( QgsUnitTypes::RenderUnit unit )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;

    QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( layer );
    markerLayer->setSizeUnit( unit );
  }
}

QgsUnitTypes::RenderUnit QgsMarkerSymbolV2::sizeUnit() const
{
  bool first = true;
  QgsUnitTypes::RenderUnit unit = QgsUnitTypes::RenderUnknownUnit;

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );

    if ( first )
      unit = markerLayer->sizeUnit();
    else
    {
      if ( unit != markerLayer->sizeUnit() )
        return QgsUnitTypes::RenderUnknownUnit;
    }

    first = false;
  }
  return unit;
}

void QgsMarkerSymbolV2::setSizeMapUnitScale( const QgsMapUnitScale &scale )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;

    QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( layer );
    markerLayer->setSizeMapUnitScale( scale );
  }
}

QgsMapUnitScale QgsMarkerSymbolV2::sizeMapUnitScale() const
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;

    QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( layer );
    return markerLayer->sizeMapUnitScale();
  }
  return QgsMapUnitScale();
}

void QgsMarkerSymbolV2::setDataDefinedSize( const QgsDataDefined &dd )
{
  const double symbolSize = size();

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( layer );

    if ( dd.hasDefaultValues() )
    {
      markerLayer->removeDataDefinedProperty( "size" );
      markerLayer->removeDataDefinedProperty( "offset" );
    }
    else
    {
      if ( qgsDoubleNear( symbolSize, 0.0 ) || qgsDoubleNear( markerLayer->size(), symbolSize ) )
      {
        markerLayer->setDataDefinedProperty( "size", new QgsDataDefined( dd ) );
      }
      else
      {
        markerLayer->setDataDefinedProperty( "size", scaleWholeSymbol( markerLayer->size() / symbolSize, dd ) );
      }

      if ( !qgsDoubleNear( markerLayer->offset().x(), 0.0 ) || !qgsDoubleNear( markerLayer->offset().y(), 0.0 ) )
      {
        markerLayer->setDataDefinedProperty( "offset", scaleWholeSymbol(
                                               markerLayer->offset().x() / symbolSize,
                                               markerLayer->offset().y() / symbolSize, dd ) );
      }
    }
  }
}

QgsDataDefined QgsMarkerSymbolV2::dataDefinedSize() const
{
  const double symbolSize = size();

  QgsDataDefined* symbolDD = nullptr;

  // find the base of the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );
    if ( qgsDoubleNear( markerLayer->size(), symbolSize ) && markerLayer->getDataDefinedProperty( "size" ) )
    {
      symbolDD = markerLayer->getDataDefinedProperty( "size" );
      break;
    }
  }

  if ( !symbolDD )
    return QgsDataDefined();

  // check that all layers size expressions match the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );

    QgsDataDefined* layerSizeDD = markerLayer->getDataDefinedProperty( "size" );
    QgsDataDefined* layerOffsetDD = markerLayer->getDataDefinedProperty( "offset" );

    if ( qgsDoubleNear( markerLayer->size(), symbolSize ) )
    {
      if ( !layerSizeDD || *layerSizeDD != *symbolDD )
        return QgsDataDefined();
    }
    else
    {
      if ( qgsDoubleNear( symbolSize, 0.0 ) )
        return QgsDataDefined();

      QScopedPointer< QgsDataDefined > scaledDD( scaleWholeSymbol( markerLayer->size() / symbolSize, *symbolDD ) );
      if ( !layerSizeDD ||  *layerSizeDD != *( scaledDD.data() ) )
        return QgsDataDefined();
    }

    QScopedPointer< QgsDataDefined > scaledOffsetDD( scaleWholeSymbol( markerLayer->offset().x() / symbolSize, markerLayer->offset().y() / symbolSize, *symbolDD ) );
    if ( layerOffsetDD && *layerOffsetDD != *( scaledOffsetDD.data() ) )
      return QgsDataDefined();
  }

  return QgsDataDefined( *symbolDD );
}

void QgsMarkerSymbolV2::setScaleMethod( QgsSymbol::ScaleMethod scaleMethod )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( layer );
    markerLayer->setScaleMethod( scaleMethod );
  }
}

QgsSymbol::ScaleMethod QgsMarkerSymbolV2::scaleMethod()
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayerV2* markerLayer = static_cast<const QgsMarkerSymbolLayerV2*>( layer );
    // return scale method of the first symbol layer
    return markerLayer->scaleMethod();
  }

  return DEFAULT_SCALE_METHOD;
}

void QgsMarkerSymbolV2::renderPointUsingLayer( QgsMarkerSymbolLayerV2* layer, QPointF point, QgsSymbolRenderContext& context )
{
  static QPointF nullPoint( 0, 0 );

  QgsPaintEffect* effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QPainter* p = context.renderContext().painter();
    p->save();
    p->translate( point );

    effect->begin( context.renderContext() );
    layer->renderPoint( nullPoint, context );
    effect->end( context.renderContext() );

    p->restore();
  }
  else
  {
    layer->renderPoint( point, context );
  }
}

void QgsMarkerSymbolV2::renderPoint( QPointF point, const QgsFeature* f, QgsRenderContext& context, int layerIdx, bool selected )
{
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mAlpha, selected, mRenderHints, f, QgsFields(), mapUnitScale() );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer* symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer )
    {
      if ( symbolLayer->type() == QgsSymbol::Marker )
      {
        QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( symbolLayer );
        renderPointUsingLayer( markerLayer, point, symbolContext );
      }
      else
        renderUsingLayer( symbolLayer, symbolContext );
    }
    return;
  }

  Q_FOREACH ( QgsSymbolLayer* symbolLayer, mLayers )
  {
    if ( symbolLayer->type() == QgsSymbol::Marker )
    {
      QgsMarkerSymbolLayerV2* markerLayer = static_cast<QgsMarkerSymbolLayerV2*>( symbolLayer );
      renderPointUsingLayer( markerLayer, point, symbolContext );
    }
    else
      renderUsingLayer( symbolLayer, symbolContext );
  }
}

QRectF QgsMarkerSymbolV2::bounds( QPointF point, QgsRenderContext& context, const QgsFeature& feature ) const
{
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mAlpha, false, mRenderHints, &feature, feature.fields(), mapUnitScale() );

  QRectF bound;
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() == QgsSymbol::Marker )
    {
      QgsMarkerSymbolLayerV2* symbolLayer = static_cast< QgsMarkerSymbolLayerV2* >( layer );
      if ( bound.isNull() )
        bound = symbolLayer->bounds( point, symbolContext );
      else
        bound = bound.united( symbolLayer->bounds( point, symbolContext ) );
    }
  }
  return bound;
}

QgsMarkerSymbolV2* QgsMarkerSymbolV2::clone() const
{
  QgsMarkerSymbolV2* cloneSymbol = new QgsMarkerSymbolV2( cloneLayers() );
  cloneSymbol->setAlpha( mAlpha );
  cloneSymbol->setLayer( mLayer );
  cloneSymbol->setClipFeaturesToExtent( mClipFeaturesToExtent );
  return cloneSymbol;
}


///////////////////
// LINE

QgsLineSymbolV2::QgsLineSymbolV2( const QgsSymbolLayerList& layers )
    : QgsSymbol( Line, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleLineSymbolLayerV2() );
}

void QgsLineSymbolV2::setWidth( double w )
{
  double origWidth = width();

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    QgsLineSymbolLayerV2* lineLayer = dynamic_cast<QgsLineSymbolLayerV2*>( layer );

    if ( lineLayer )
    {
      if ( qgsDoubleNear( lineLayer->width(), origWidth ) )
      {
        lineLayer->setWidth( w );
      }
      else if ( !qgsDoubleNear( origWidth, 0.0 ) )
      {
        // proportionally scale the width
        lineLayer->setWidth( lineLayer->width() * w / origWidth );
      }
      // also scale offset to maintain relative position
      if ( !qgsDoubleNear( origWidth, 0.0 ) && !qgsDoubleNear( lineLayer->offset(), 0.0 ) )
        lineLayer->setOffset( lineLayer->offset() * w / origWidth );
    }
  }
}

double QgsLineSymbolV2::width() const
{
  double maxWidth = 0;
  if ( mLayers.isEmpty() )
    return maxWidth;

  Q_FOREACH ( QgsSymbolLayer* symbolLayer, mLayers )
  {
    const QgsLineSymbolLayerV2* lineLayer = dynamic_cast<QgsLineSymbolLayerV2*>( symbolLayer );
    if ( lineLayer )
    {
      double width = lineLayer->width();
      if ( width > maxWidth )
        maxWidth = width;
    }
  }
  return maxWidth;
}

void QgsLineSymbolV2::setDataDefinedWidth( const QgsDataDefined& dd )
{
  const double symbolWidth = width();

  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    QgsLineSymbolLayerV2* lineLayer = dynamic_cast<QgsLineSymbolLayerV2*>( layer );

    if ( lineLayer )
    {
      if ( dd.hasDefaultValues() )
      {
        lineLayer->removeDataDefinedProperty( "width" );
        lineLayer->removeDataDefinedProperty( "offset" );
      }
      else
      {
        if ( qgsDoubleNear( symbolWidth, 0.0 ) || qgsDoubleNear( lineLayer->width(), symbolWidth ) )
        {
          lineLayer->setDataDefinedProperty( "width", new QgsDataDefined( dd ) );
        }
        else
        {
          lineLayer->setDataDefinedProperty( "width", scaleWholeSymbol( lineLayer->width() / symbolWidth, dd ) );
        }

        if ( !qgsDoubleNear( lineLayer->offset(), 0.0 ) )
        {
          lineLayer->setDataDefinedProperty( "offset", scaleWholeSymbol( lineLayer->offset() / symbolWidth, dd ) );
        }
      }
    }
  }
}

QgsDataDefined QgsLineSymbolV2::dataDefinedWidth() const
{
  const double symbolWidth = width();

  QgsDataDefined* symbolDD = nullptr;

  // find the base of the "en masse" pattern
  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    const QgsLineSymbolLayerV2* layer = dynamic_cast<const QgsLineSymbolLayerV2*>( *it );
    if ( layer && qgsDoubleNear( layer->width(), symbolWidth ) && layer->getDataDefinedProperty( "width" ) )
    {
      symbolDD = layer->getDataDefinedProperty( "width" );
      break;
    }
  }

  if ( !symbolDD )
    return QgsDataDefined();

  // check that all layers width expressions match the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() !=  QgsSymbol::Line )
      continue;
    const QgsLineSymbolLayerV2* lineLayer = static_cast<const QgsLineSymbolLayerV2*>( layer );

    QgsDataDefined* layerWidthDD = lineLayer->getDataDefinedProperty( "width" );
    QgsDataDefined* layerOffsetDD = lineLayer->getDataDefinedProperty( "offset" );

    if ( qgsDoubleNear( lineLayer->width(), symbolWidth ) )
    {
      if ( !layerWidthDD || *layerWidthDD != *symbolDD )
        return QgsDataDefined();
    }
    else
    {
      if ( qgsDoubleNear( symbolWidth, 0.0 ) )
        return QgsDataDefined();

      QScopedPointer< QgsDataDefined > scaledDD( scaleWholeSymbol( lineLayer->width() / symbolWidth, *symbolDD ) );
      if ( !layerWidthDD || *layerWidthDD != *( scaledDD.data() ) )
        return QgsDataDefined();
    }

    QScopedPointer< QgsDataDefined > scaledOffsetDD( scaleWholeSymbol( lineLayer->offset() / symbolWidth, *symbolDD ) );
    if ( layerOffsetDD && *layerOffsetDD != *( scaledOffsetDD.data() ) )
      return QgsDataDefined();
  }

  return QgsDataDefined( *symbolDD );
}

void QgsLineSymbolV2::renderPolyline( const QPolygonF& points, const QgsFeature* f, QgsRenderContext& context, int layerIdx, bool selected )
{
  //save old painter
  QPainter* renderPainter = context.painter();
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mAlpha, selected, mRenderHints, f, QgsFields(), mapUnitScale() );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer* symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer )
    {
      if ( symbolLayer->type() == QgsSymbol::Line )
      {
        QgsLineSymbolLayerV2* lineLayer = static_cast<QgsLineSymbolLayerV2*>( symbolLayer );
        renderPolylineUsingLayer( lineLayer, points, symbolContext );
      }
      else
        renderUsingLayer( symbolLayer, symbolContext );
    }
    return;
  }

  Q_FOREACH ( QgsSymbolLayer* symbolLayer, mLayers )
  {
    if ( symbolLayer->type() == QgsSymbol::Line )
    {
      QgsLineSymbolLayerV2* lineLayer = static_cast<QgsLineSymbolLayerV2*>( symbolLayer );
      renderPolylineUsingLayer( lineLayer, points, symbolContext );
    }
    else
    {
      renderUsingLayer( symbolLayer, symbolContext );
    }
  }

  context.setPainter( renderPainter );
}

void QgsLineSymbolV2::renderPolylineUsingLayer( QgsLineSymbolLayerV2 *layer, const QPolygonF &points, QgsSymbolRenderContext &context )
{
  QgsPaintEffect* effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QPainter* p = context.renderContext().painter();
    p->save();
    p->translate( points.boundingRect().topLeft() );

    effect->begin( context.renderContext() );
    layer->renderPolyline( points.translated( -points.boundingRect().topLeft() ), context );
    effect->end( context.renderContext() );

    p->restore();
  }
  else
  {
    layer->renderPolyline( points, context );
  }
}


QgsLineSymbolV2* QgsLineSymbolV2::clone() const
{
  QgsLineSymbolV2* cloneSymbol = new QgsLineSymbolV2( cloneLayers() );
  cloneSymbol->setAlpha( mAlpha );
  cloneSymbol->setLayer( mLayer );
  cloneSymbol->setClipFeaturesToExtent( mClipFeaturesToExtent );
  return cloneSymbol;
}

///////////////////
// FILL

QgsFillSymbolV2::QgsFillSymbolV2( const QgsSymbolLayerList& layers )
    : QgsSymbol( Fill, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleFillSymbolLayerV2() );
}

void QgsFillSymbolV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layerIdx, bool selected )
{
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mAlpha, selected, mRenderHints, f, QgsFields(), mapUnitScale() );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer* symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer )
    {
      if ( symbolLayer->type() == Fill || symbolLayer->type() == Line )
        renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
      else
        renderUsingLayer( symbolLayer, symbolContext );
    }
    return;
  }

  Q_FOREACH ( QgsSymbolLayer* symbolLayer, mLayers )
  {
    if ( symbolLayer->type() == Fill || symbolLayer->type() == Line )
      renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
    else
      renderUsingLayer( symbolLayer, symbolContext );
  }
}

void QgsFillSymbolV2::renderPolygonUsingLayer( QgsSymbolLayer* layer, const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolRenderContext& context )
{
  QgsSymbol::SymbolType layertype = layer->type();

  QgsPaintEffect* effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QRectF bounds = polygonBounds( points, rings );
    QList<QPolygonF>* translatedRings = translateRings( rings, -bounds.left(), -bounds.top() );

    QPainter* p = context.renderContext().painter();
    p->save();
    p->translate( bounds.topLeft() );

    effect->begin( context.renderContext() );
    if ( layertype == QgsSymbol::Fill )
    {
      ( static_cast<QgsFillSymbolLayerV2*>( layer ) )->renderPolygon( points.translated( -bounds.topLeft() ), translatedRings, context );
    }
    else if ( layertype == QgsSymbol::Line )
    {
      ( static_cast<QgsLineSymbolLayerV2*>( layer ) )->renderPolygonOutline( points.translated( -bounds.topLeft() ), translatedRings, context );
    }
    delete translatedRings;

    effect->end( context.renderContext() );
    p->restore();
  }
  else
  {
    if ( layertype == QgsSymbol::Fill )
    {
      ( static_cast<QgsFillSymbolLayerV2*>( layer ) )->renderPolygon( points, rings, context );
    }
    else if ( layertype == QgsSymbol::Line )
    {
      ( static_cast<QgsLineSymbolLayerV2*>( layer ) )->renderPolygonOutline( points, rings, context );
    }
  }
}

QRectF QgsFillSymbolV2::polygonBounds( const QPolygonF& points, const QList<QPolygonF>* rings ) const
{
  QRectF bounds = points.boundingRect();
  if ( rings )
  {
    QList<QPolygonF>::const_iterator it = rings->constBegin();
    for ( ; it != rings->constEnd(); ++it )
    {
      bounds = bounds.united(( *it ).boundingRect() );
    }
  }
  return bounds;
}

QList<QPolygonF>* QgsFillSymbolV2::translateRings( const QList<QPolygonF>* rings, double dx, double dy ) const
{
  if ( !rings )
    return nullptr;

  QList<QPolygonF>* translatedRings = new QList<QPolygonF>;
  QList<QPolygonF>::const_iterator it = rings->constBegin();
  for ( ; it != rings->constEnd(); ++it )
  {
    translatedRings->append(( *it ).translated( dx, dy ) );
  }
  return translatedRings;
}

QgsFillSymbolV2* QgsFillSymbolV2::clone() const
{
  QgsFillSymbolV2* cloneSymbol = new QgsFillSymbolV2( cloneLayers() );
  cloneSymbol->setAlpha( mAlpha );
  cloneSymbol->setLayer( mLayer );
  cloneSymbol->setClipFeaturesToExtent( mClipFeaturesToExtent );
  return cloneSymbol;
}

void QgsFillSymbolV2::setAngle( double angle )
{
  Q_FOREACH ( QgsSymbolLayer* layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Fill )
      continue;

    QgsFillSymbolLayerV2* fillLayer = static_cast<QgsFillSymbolLayerV2*>( layer );

    if ( fillLayer )
      fillLayer->setAngle( angle );
  }
}


