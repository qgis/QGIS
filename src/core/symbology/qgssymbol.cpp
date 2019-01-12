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

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QSvgGenerator>

#include <cmath>
#include <map>
#include <random>

#include "qgssymbol.h"
#include "qgssymbollayer.h"

#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgslogger.h"
#include "qgsrendercontext.h" // for bigSymbolPreview
#include "qgsproject.h"
#include "qgsstyle.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsmultipoint.h"
#include "qgsgeometrycollection.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsclipper.h"
#include "qgsproperty.h"
#include "qgscolorschemeregistry.h"

inline
QgsProperty rotateWholeSymbol( double additionalRotation, const QgsProperty &property )
{
  QString exprString = property.asExpression();
  return QgsProperty::fromExpression( QString::number( additionalRotation ) + " + (" + exprString + ')' );
}

inline
QgsProperty scaleWholeSymbol( double scaleFactor, const QgsProperty &property )
{
  QString exprString = property.asExpression();
  return QgsProperty::fromExpression( QString::number( scaleFactor ) + "*(" + exprString + ')' );
}

inline
QgsProperty scaleWholeSymbol( double scaleFactorX, double scaleFactorY, const QgsProperty &property )
{
  QString exprString = property.asExpression();
  return QgsProperty::fromExpression(
           ( !qgsDoubleNear( scaleFactorX, 0.0 ) ? "tostring(" + QString::number( scaleFactorX ) + "*(" + exprString + "))" : QStringLiteral( "'0'" ) ) +
           "|| ',' || " +
           ( !qgsDoubleNear( scaleFactorY, 0.0 ) ? "tostring(" + QString::number( scaleFactorY ) + "*(" + exprString + "))" : QStringLiteral( "'0'" ) ) );
}


////////////////////

Q_NOWARN_DEPRECATED_PUSH // because of deprecated mLayer
QgsSymbol::QgsSymbol( SymbolType type, const QgsSymbolLayerList &layers )
  : mType( type )
  , mLayers( layers )
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
Q_NOWARN_DEPRECATED_POP

QPolygonF QgsSymbol::_getLineString( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent )
{
  const unsigned int nPoints = curve.numPoints();

  QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel &mtp = context.mapToPixel();
  QPolygonF pts;

  //apply clipping for large lines to achieve a better rendering performance
  if ( clipToExtent && nPoints > 1 )
  {
    const QgsRectangle &e = context.extent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );
    pts = QgsClipper::clippedLine( curve, clipRect );
  }
  else
  {
    pts = curve.asQPolygonF();
  }

  //transform the QPolygonF to screen coordinates
  if ( ct.isValid() )
  {
    ct.transformPolygon( pts );
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  pts.erase( std::remove_if( pts.begin(), pts.end(),
                             []( const QPointF point )
  {
    return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
  } ), pts.end() );

  QPointF *ptr = pts.data();
  for ( int i = 0; i < pts.size(); ++i, ++ptr )
  {
    mtp.transformInPlace( ptr->rx(), ptr->ry() );
  }

  return pts;
}

QPolygonF QgsSymbol::_getPolygonRing( QgsRenderContext &context, const QgsCurve &curve, const bool clipToExtent, const bool isExteriorRing, const bool correctRingOrientation )
{
  const QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel &mtp = context.mapToPixel();
  const QgsRectangle &e = context.extent();
  const double cw = e.width() / 10;
  const double ch = e.height() / 10;
  QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );

  QPolygonF poly = curve.asQPolygonF();

  if ( curve.numPoints() < 1 )
    return QPolygonF();

  if ( correctRingOrientation )
  {
    // ensure consistent polygon ring orientation
    if ( isExteriorRing && curve.orientation() != QgsCurve::Clockwise )
      std::reverse( poly.begin(), poly.end() );
    else if ( !isExteriorRing && curve.orientation() != QgsCurve::CounterClockwise )
      std::reverse( poly.begin(), poly.end() );
  }

  //clip close to view extent, if needed
  const QRectF ptsRect = poly.boundingRect();
  if ( clipToExtent && !context.extent().contains( ptsRect ) )
  {
    QgsClipper::trimPolygon( poly, clipRect );
  }

  //transform the QPolygonF to screen coordinates
  if ( ct.isValid() )
  {
    ct.transformPolygon( poly );
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  poly.erase( std::remove_if( poly.begin(), poly.end(),
                              []( const QPointF point )
  {
    return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
  } ), poly.end() );

  QPointF *ptr = poly.data();
  for ( int i = 0; i < poly.size(); ++i, ++ptr )
  {
    mtp.transformInPlace( ptr->rx(), ptr->ry() );
  }

  return poly;
}

void QgsSymbol::_getPolygon( QPolygonF &pts, QList<QPolygonF> &holes, QgsRenderContext &context, const QgsPolygon &polygon, const bool clipToExtent, const bool correctRingOrientation )
{
  holes.clear();

  pts = _getPolygonRing( context, *polygon.exteriorRing(), clipToExtent, true, correctRingOrientation );
  for ( int idx = 0; idx < polygon.numInteriorRings(); idx++ )
  {
    const QPolygonF hole = _getPolygonRing( context, *( polygon.interiorRing( idx ) ), clipToExtent, false, correctRingOrientation );
    if ( !hole.isEmpty() ) holes.append( hole );
  }
}

QgsSymbol::~QgsSymbol()
{
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
    if ( ( *it )->outputUnit() != unit )
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
    if ( ( *it )->mapUnitScale() != scale )
    {
      return QgsMapUnitScale();
    }
  }
  return scale;
}

void QgsSymbol::setOutputUnit( QgsUnitTypes::RenderUnit u )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    layer->setOutputUnit( u );
  }
}

void QgsSymbol::setMapUnitScale( const QgsMapUnitScale &scale )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    layer->setMapUnitScale( scale );
  }
}

QgsSymbol *QgsSymbol::defaultSymbol( QgsWkbTypes::GeometryType geomType )
{
  std::unique_ptr< QgsSymbol > s;

  // override global default if project has a default for this type
  QString defaultSymbol;
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry :
      defaultSymbol = QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Marker" ) );
      break;
    case QgsWkbTypes::LineGeometry :
      defaultSymbol = QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Line" ) );
      break;
    case QgsWkbTypes::PolygonGeometry :
      defaultSymbol = QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Fill" ) );
      break;
    default:
      break;
  }
  if ( !defaultSymbol.isEmpty() )
    s.reset( QgsStyle::defaultStyle()->symbol( defaultSymbol ) );

  // if no default found for this type, get global default (as previously)
  if ( !s )
  {
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        s = qgis::make_unique< QgsMarkerSymbol >();
        break;
      case QgsWkbTypes::LineGeometry:
        s = qgis::make_unique< QgsLineSymbol >();
        break;
      case QgsWkbTypes::PolygonGeometry:
        s = qgis::make_unique< QgsFillSymbol >();
        break;
      default:
        QgsDebugMsg( QStringLiteral( "unknown layer's geometry type" ) );
        return nullptr;
    }
  }

  // set opacity
  double opacity = 1.0;
  bool ok = false;
  // upgrade old setting
  double alpha = QgsProject::instance()->readDoubleEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/AlphaInt" ), 255, &ok );
  if ( ok )
    opacity = alpha / 255.0;
  double newOpacity = QgsProject::instance()->readDoubleEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Opacity" ), 1.0, &ok );
  if ( ok )
    opacity = newOpacity;
  s->setOpacity( opacity );

  // set random color, it project prefs allow
  if ( defaultSymbol.isEmpty() ||
       QgsProject::instance()->readBoolEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/RandomColors" ), true ) )
  {
    s->setColor( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor() );
  }

  return s.release();
}

QgsSymbolLayer *QgsSymbol::symbolLayer( int layer )
{
  return mLayers.value( layer );
}

bool QgsSymbol::insertSymbolLayer( int index, QgsSymbolLayer *layer )
{
  if ( index < 0 || index > mLayers.count() ) // can be added also after the last index
    return false;

  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  mLayers.insert( index, layer );
  return true;
}


bool QgsSymbol::appendSymbolLayer( QgsSymbolLayer *layer )
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


QgsSymbolLayer *QgsSymbol::takeSymbolLayer( int index )
{
  if ( index < 0 || index >= mLayers.count() )
    return nullptr;

  return mLayers.takeAt( index );
}


bool QgsSymbol::changeSymbolLayer( int index, QgsSymbolLayer *layer )
{
  QgsSymbolLayer *oldLayer = mLayers.value( index );

  if ( oldLayer == layer )
    return false;

  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  delete oldLayer; // first delete the original layer
  mLayers[index] = layer; // set new layer
  return true;
}


void QgsSymbol::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  Q_ASSERT_X( !mStarted, "startRender", "Rendering has already been started for this symbol instance!" );
  mStarted = true;

  mSymbolRenderContext.reset( new QgsSymbolRenderContext( context, outputUnit(), mOpacity, false, mRenderHints, nullptr, fields, mapUnitScale() ) );

  QgsSymbolRenderContext symbolContext( context, outputUnit(), mOpacity, false, mRenderHints, nullptr, fields, mapUnitScale() );

  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::updateSymbolScope( this, new QgsExpressionContextScope() ) );
  mSymbolRenderContext->setExpressionContextScope( scope.release() );

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( !layer->enabled() )
      continue;

    layer->prepareExpressions( symbolContext );
    layer->startRender( symbolContext );
  }
}

void QgsSymbol::stopRender( QgsRenderContext &context )
{
  Q_ASSERT_X( mStarted, "startRender", "startRender was not called for this symbol instance!" );
  mStarted = false;

  Q_UNUSED( context )
  if ( mSymbolRenderContext )
  {
    Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
    {
      if ( !layer->enabled() )
        continue;

      layer->stopRender( *mSymbolRenderContext );
    }
  }

  mSymbolRenderContext.reset( nullptr );

  Q_NOWARN_DEPRECATED_PUSH
  mLayer = nullptr;
  Q_NOWARN_DEPRECATED_POP
}

void QgsSymbol::setColor( const QColor &color )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
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

void QgsSymbol::drawPreviewIcon( QPainter *painter, QSize size, QgsRenderContext *customContext )
{
  QgsRenderContext context = customContext ? *customContext : QgsRenderContext::fromQPainter( painter );
  context.setForceVectorOutput( true );
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mOpacity, false, mRenderHints, nullptr, QgsFields(), mapUnitScale() );
  symbolContext.setOriginalGeometryType( mType == Fill ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::UnknownGeometry );

  if ( !customContext )
  {
    // if no render context was passed, build a minimal expression context
    QgsExpressionContext expContext;
    expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
    context.setExpressionContext( expContext );
  }

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( !layer->enabled() )
      continue;

    if ( mType == Fill && layer->type() == Line )
    {
      // line symbol layer would normally draw just a line
      // so we override this case to force it to draw a polygon stroke
      QgsLineSymbolLayer *lsl = dynamic_cast<QgsLineSymbolLayer *>( layer );

      if ( lsl )
      {
        // from QgsFillSymbolLayer::drawPreviewIcon()
        QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width() - 1, size.height() - 1 ) );
        lsl->startRender( symbolContext );
        lsl->renderPolygonStroke( poly, nullptr, symbolContext );
        lsl->stopRender( symbolContext );
      }
    }
    else
      layer->drawPreviewIcon( symbolContext, size );
  }
}

void QgsSymbol::exportImage( const QString &path, const QString &format, QSize size )
{
  if ( format.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
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

QImage QgsSymbol::asImage( QSize size, QgsRenderContext *customContext )
{
  QImage image( size, QImage::Format_ARGB32_Premultiplied );
  image.fill( 0 );

  QPainter p( &image );
  p.setRenderHint( QPainter::Antialiasing );

  drawPreviewIcon( &p, size, customContext );

  return image;
}


QImage QgsSymbol::bigSymbolPreviewImage( QgsExpressionContext *expressionContext )
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

  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );
  if ( expressionContext )
    context.setExpressionContext( *expressionContext );

  startRender( context );

  if ( mType == QgsSymbol::Line )
  {
    QPolygonF poly;
    poly << QPointF( 0, 50 ) << QPointF( 99, 50 );
    static_cast<QgsLineSymbol *>( this )->renderPolyline( poly, nullptr, context );
  }
  else if ( mType == QgsSymbol::Fill )
  {
    QPolygonF polygon;
    polygon << QPointF( 20, 20 ) << QPointF( 80, 20 ) << QPointF( 80, 80 ) << QPointF( 20, 80 ) << QPointF( 20, 20 );
    static_cast<QgsFillSymbol *>( this )->renderPolygon( polygon, nullptr, nullptr, context );
  }
  else // marker
  {
    static_cast<QgsMarkerSymbol *>( this )->renderPoint( QPointF( 50, 50 ), nullptr, context );
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
      t = QStringLiteral( "MARKER" );
      break;
    case QgsSymbol::Line:
      t = QStringLiteral( "LINE" );
      break;
    case QgsSymbol::Fill:
      t = QStringLiteral( "FILL" );
      break;
    default:
      Q_ASSERT( false && "unknown symbol type" );
  }
  QString s = QStringLiteral( "%1 SYMBOL (%2 layers) color %3" ).arg( t ).arg( mLayers.count() ).arg( QgsSymbolLayerUtils::encodeColor( color() ) );

  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // TODO:
  }
  return s;
}

void QgsSymbol::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  props[ QStringLiteral( "alpha" )] = QString::number( opacity() );
  double scaleFactor = 1.0;
  props[ QStringLiteral( "uom" )] = QgsSymbolLayerUtils::encodeSldUom( outputUnit(), &scaleFactor );
  props[ QStringLiteral( "uomScale" )] = ( !qgsDoubleNear( scaleFactor, 1.0 ) ? qgsDoubleToString( scaleFactor ) : QString() );

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
    QgsSymbolLayer *layer = ( *it )->clone();
    layer->setLocked( ( *it )->isLocked() );
    layer->setRenderingPass( ( *it )->renderingPass() );
    layer->setEnabled( ( *it )->enabled() );
    lst.append( layer );
  }
  return lst;
}

void QgsSymbol::renderUsingLayer( QgsSymbolLayer *layer, QgsSymbolRenderContext &context )
{
  Q_ASSERT( layer->type() == Hybrid );

  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsGeometryGeneratorSymbolLayer *generatorLayer = static_cast<QgsGeometryGeneratorSymbolLayer *>( layer );

  QgsPaintEffect *effect = generatorLayer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext(), effect );
    generatorLayer->render( context );
  }
  else
  {
    generatorLayer->render( context );
  }
}

QSet<QString> QgsSymbol::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes;
  QgsSymbolLayerList::const_iterator sIt = mLayers.constBegin();
  for ( ; sIt != mLayers.constEnd(); ++sIt )
  {
    if ( *sIt )
    {
      attributes.unite( ( *sIt )->usedAttributes( context ) );
    }
  }
  return attributes;
}

bool QgsSymbol::hasDataDefinedProperties() const
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->dataDefinedProperties().hasActiveProperties() )
      return true;
    // we treat geometry generator layers like they have data defined properties,
    // since the WHOLE layer is based on expressions and requires the full expression
    // context
    if ( layer->layerType() == QLatin1String( "GeometryGenerator" ) )
      return true;
  }
  return false;
}

void QgsSymbol::setLayer( const QgsVectorLayer *layer )
{
  Q_NOWARN_DEPRECATED_PUSH
  mLayer = layer;
  Q_NOWARN_DEPRECATED_POP
}

const QgsVectorLayer *QgsSymbol::layer() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mLayer;
  Q_NOWARN_DEPRECATED_POP
}

///@cond PRIVATE

/**
 * RAII class to pop scope from an expression context on destruction
 */
class ExpressionContextScopePopper
{
  public:

    ExpressionContextScopePopper() = default;

    ~ExpressionContextScopePopper()
    {
      if ( context )
        context->popScope();
    }

    QgsExpressionContext *context = nullptr;
};

/**
 * RAII class to restore original geometry on a render context on destruction
 */
class GeometryRestorer
{
  public:
    GeometryRestorer( QgsRenderContext &context )
      : mContext( context ),
        mGeometry( context.geometry() )
    {}

    ~GeometryRestorer()
    {
      mContext.setGeometry( mGeometry );
    }

  private:
    QgsRenderContext &mContext;
    const QgsAbstractGeometry *mGeometry;
};
///@endcond PRIVATE

void QgsSymbol::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker, int currentVertexMarkerType, int currentVertexMarkerSize )
{
  const QgsGeometry geom = feature.geometry();
  if ( geom.isNull() )
  {
    return;
  }

  GeometryRestorer geomRestorer( context );
  QgsGeometry segmentizedGeometry = geom;
  bool usingSegmentizedGeometry = false;
  context.setGeometry( geom.constGet() );

  bool tileMapRendering = context.testFlag( QgsRenderContext::RenderMapTile );

  //convert curve types to normal point/line/polygon ones
  if ( QgsWkbTypes::isCurvedType( geom.constGet()->wkbType() ) )
  {
    QgsAbstractGeometry *g = geom.constGet()->segmentize( context.segmentationTolerance(), context.segmentationToleranceType() );
    if ( !g )
    {
      return;
    }
    segmentizedGeometry = QgsGeometry( g );
    usingSegmentizedGeometry = true;
  }

  mSymbolRenderContext->setGeometryPartCount( segmentizedGeometry.constGet()->partCount() );
  mSymbolRenderContext->setGeometryPartNum( 1 );

  bool needsExpressionContext = hasDataDefinedProperties();
  ExpressionContextScopePopper scopePopper;
  if ( mSymbolRenderContext->expressionContextScope() )
  {
    if ( needsExpressionContext )
    {
      // this is somewhat nasty - by appending this scope here it's now owned
      // by both mSymbolRenderContext AND context.expressionContext()
      // the RAII scopePopper is required to make sure it always has ownership transferred back
      // from context.expressionContext(), even if exceptions of other early exits occur in this
      // function
      context.expressionContext().appendScope( mSymbolRenderContext->expressionContextScope() );
      scopePopper.context = &context.expressionContext();

      QgsExpressionContextUtils::updateSymbolScope( this, mSymbolRenderContext->expressionContextScope() );
      mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT, mSymbolRenderContext->geometryPartCount(), true ) );
      mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, 1, true ) );
    }
  }

  // Collection of markers to paint, only used for no curve types.
  QPolygonF markers;

  // Simplify the geometry, if needed.
  if ( context.vectorSimplifyMethod().forceLocalOptimization() )
  {
    const int simplifyHints = context.vectorSimplifyMethod().simplifyHints();
    const QgsMapToPixelSimplifier simplifier( simplifyHints, context.vectorSimplifyMethod().tolerance(),
        static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( context.vectorSimplifyMethod().simplifyAlgorithm() ) );
    segmentizedGeometry = simplifier.simplify( segmentizedGeometry );
  }

  switch ( QgsWkbTypes::flatType( segmentizedGeometry.constGet()->wkbType() ) )
  {
    case QgsWkbTypes::Point:
    {
      if ( mType != QgsSymbol::Marker )
      {
        QgsDebugMsg( QStringLiteral( "point can be drawn only with marker symbol!" ) );
        break;
      }

      const QgsPoint *point = static_cast< const QgsPoint * >( segmentizedGeometry.constGet() );
      const QPointF pt = _getPoint( context, *point );
      static_cast<QgsMarkerSymbol *>( this )->renderPoint( pt, &feature, context, layer, selected );

      if ( context.testFlag( QgsRenderContext::DrawSymbolBounds ) )
      {
        //draw debugging rect
        context.painter()->setPen( Qt::red );
        context.painter()->setBrush( QColor( 255, 0, 0, 100 ) );
        context.painter()->drawRect( static_cast<QgsMarkerSymbol *>( this )->bounds( pt, context, feature ) );
      }

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers << pt;
      }
    }
    break;
    case QgsWkbTypes::LineString:
    {
      if ( mType != QgsSymbol::Line )
      {
        QgsDebugMsg( QStringLiteral( "linestring can be drawn only with line symbol!" ) );
        break;
      }
      const QgsCurve &curve = dynamic_cast<const QgsCurve &>( *segmentizedGeometry.constGet() );
      const QPolygonF pts = _getLineString( context, curve, !tileMapRendering && clipFeaturesToExtent() );
      static_cast<QgsLineSymbol *>( this )->renderPolyline( pts, &feature, context, layer, selected );

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers = pts;
      }
    }
    break;
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Triangle:
    {
      QPolygonF pts;
      QList<QPolygonF> holes;
      if ( mType != QgsSymbol::Fill )
      {
        QgsDebugMsg( QStringLiteral( "polygon can be drawn only with fill symbol!" ) );
        break;
      }
      const QgsPolygon &polygon = dynamic_cast<const QgsPolygon &>( *segmentizedGeometry.constGet() );
      if ( !polygon.exteriorRing() )
      {
        QgsDebugMsg( QStringLiteral( "cannot render polygon with no exterior ring" ) );
        break;
      }
      _getPolygon( pts, holes, context, polygon, !tileMapRendering && clipFeaturesToExtent(), mForceRHR );
      static_cast<QgsFillSymbol *>( this )->renderPolygon( pts, ( !holes.isEmpty() ? &holes : nullptr ), &feature, context, layer, selected );

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers = pts;

        Q_FOREACH ( const QPolygonF &hole, holes )
        {
          markers << hole;
        }
      }
    }
    break;

    case QgsWkbTypes::MultiPoint:
    {
      if ( mType != QgsSymbol::Marker )
      {
        QgsDebugMsg( QStringLiteral( "multi-point can be drawn only with marker symbol!" ) );
        break;
      }

      const QgsMultiPoint &mp = static_cast< const QgsMultiPoint & >( *segmentizedGeometry.constGet() );

      if ( drawVertexMarker && !usingSegmentizedGeometry )
      {
        markers.reserve( mp.numGeometries() );
      }

      for ( int i = 0; i < mp.numGeometries(); ++i )
      {
        mSymbolRenderContext->setGeometryPartNum( i + 1 );
        if ( needsExpressionContext )
          mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, i + 1, true ) );

        const QgsPoint &point = static_cast< const QgsPoint & >( *mp.geometryN( i ) );
        const QPointF pt = _getPoint( context, point );
        static_cast<QgsMarkerSymbol *>( this )->renderPoint( pt, &feature, context, layer, selected );

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
      if ( mType != QgsSymbol::Line )
      {
        QgsDebugMsg( QStringLiteral( "multi-linestring can be drawn only with line symbol!" ) );
        break;
      }

      const QgsGeometryCollection &geomCollection = dynamic_cast<const QgsGeometryCollection &>( *segmentizedGeometry.constGet() );

      const unsigned int num = geomCollection.numGeometries();
      for ( unsigned int i = 0; i < num; ++i )
      {
        mSymbolRenderContext->setGeometryPartNum( i + 1 );
        if ( needsExpressionContext )
          mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, i + 1, true ) );

        context.setGeometry( geomCollection.geometryN( i ) );
        const QgsCurve &curve = dynamic_cast<const QgsCurve &>( *geomCollection.geometryN( i ) );
        const QPolygonF pts = _getLineString( context, curve, !tileMapRendering && clipFeaturesToExtent() );
        static_cast<QgsLineSymbol *>( this )->renderPolyline( pts, &feature, context, layer, selected );

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
        QgsDebugMsg( QStringLiteral( "multi-polygon can be drawn only with fill symbol!" ) );
        break;
      }

      QPolygonF pts;
      QList<QPolygonF> holes;

      const QgsGeometryCollection &geomCollection = dynamic_cast<const QgsGeometryCollection &>( *segmentizedGeometry.constGet() );
      const unsigned int num = geomCollection.numGeometries();

      // Sort components by approximate area (probably a bit faster than using
      // area() )
      std::map<double, QList<unsigned int> > mapAreaToPartNum;
      for ( unsigned int i = 0; i < num; ++i )
      {
        const QgsPolygon &polygon = dynamic_cast<const QgsPolygon &>( *geomCollection.geometryN( i ) );
        const QgsRectangle r( polygon.boundingBox() );
        mapAreaToPartNum[ r.width() * r.height()] << i;
      }

      // Draw starting with larger parts down to smaller parts, so that in
      // case of a part being incorrectly inside another part, it is drawn
      // on top of it (#15419)
      std::map<double, QList<unsigned int> >::const_reverse_iterator iter = mapAreaToPartNum.rbegin();
      for ( ; iter != mapAreaToPartNum.rend(); ++iter )
      {
        const QList<unsigned int> &listPartIndex = iter->second;
        for ( int idx = 0; idx < listPartIndex.size(); ++idx )
        {
          const unsigned i = listPartIndex[idx];
          mSymbolRenderContext->setGeometryPartNum( i + 1 );
          if ( needsExpressionContext )
            mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, i + 1, true ) );

          context.setGeometry( geomCollection.geometryN( i ) );
          const QgsPolygon &polygon = dynamic_cast<const QgsPolygon &>( *geomCollection.geometryN( i ) );
          if ( !polygon.exteriorRing() )
            break;

          _getPolygon( pts, holes, context, polygon, !tileMapRendering && clipFeaturesToExtent(), mForceRHR );
          static_cast<QgsFillSymbol *>( this )->renderPolygon( pts, ( !holes.isEmpty() ? &holes : nullptr ), &feature, context, layer, selected );

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

            Q_FOREACH ( const QPolygonF &hole, holes )
            {
              markers << hole;
            }
          }
        }
      }
      break;
    }
    case QgsWkbTypes::GeometryCollection:
    {
      const QgsGeometryCollection &geomCollection = dynamic_cast<const QgsGeometryCollection &>( *segmentizedGeometry.constGet() );
      if ( geomCollection.numGeometries() == 0 )
      {
        // skip noise from empty geometry collections from simplification
        break;
      }

      FALLTHROUGH
    }
    default:
      QgsDebugMsg( QStringLiteral( "feature %1: unsupported wkb type %2/%3 for rendering" )
                   .arg( feature.id() )
                   .arg( QgsWkbTypes::displayString( geom.constGet()->wkbType() ) )
                   .arg( geom.wkbType(), 0, 16 ) );
  }

  if ( drawVertexMarker )
  {
    if ( !markers.isEmpty() )
    {
      Q_FOREACH ( QPointF marker, markers )
      {
        renderVertexMarker( marker, context, currentVertexMarkerType, currentVertexMarkerSize );
      }
    }
    else
    {
      QgsCoordinateTransform ct = context.coordinateTransform();
      const QgsMapToPixel &mtp = context.mapToPixel();

      QgsPoint vertexPoint;
      QgsVertexId vertexId;
      double x, y, z;
      QPointF mapPoint;
      while ( geom.constGet()->nextVertex( vertexId, vertexPoint ) )
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
}

QgsSymbolRenderContext *QgsSymbol::symbolRenderContext()
{
  return mSymbolRenderContext.get();
}

void QgsSymbol::renderVertexMarker( QPointF pt, QgsRenderContext &context, int currentVertexMarkerType, int currentVertexMarkerSize )
{
  QgsVectorLayer::drawVertexMarker( pt.x(), pt.y(), *context.painter(), static_cast< QgsVectorLayer::VertexMarkerType >( currentVertexMarkerType ), currentVertexMarkerSize );
}

////////////////////


QgsSymbolRenderContext::QgsSymbolRenderContext( QgsRenderContext &c, QgsUnitTypes::RenderUnit u, qreal opacity, bool selected, QgsSymbol::RenderHints renderHints, const QgsFeature *f, const QgsFields &fields, const QgsMapUnitScale &mapUnitScale )
  : mRenderContext( c )
  , mOutputUnit( u )
  , mMapUnitScale( mapUnitScale )
  , mOpacity( opacity )
  , mSelected( selected )
  , mRenderHints( renderHints )
  , mFeature( f )
  , mFields( fields )
  , mGeometryPartCount( 0 )
  , mGeometryPartNum( 0 )
{
}

void QgsSymbolRenderContext::setOriginalValueVariable( const QVariant &value )
{
  mRenderContext.expressionContext().setOriginalValueVariable( value );
}

double QgsSymbolRenderContext::outputLineWidth( double width ) const
{
  return mRenderContext.convertToPainterUnits( width, mOutputUnit, mMapUnitScale );
}

double QgsSymbolRenderContext::outputPixelSize( double size ) const
{
  return mRenderContext.convertToPainterUnits( size, mOutputUnit, mMapUnitScale );
}

QgsSymbolRenderContext &QgsSymbolRenderContext::operator=( const QgsSymbolRenderContext & )
{
  // This is just a dummy implementation of assignment.
  // sip 4.7 generates a piece of code that needs this function to exist.
  // It's not generated automatically by the compiler because of
  // mRenderContext member which is a reference (and thus can't be changed).
  Q_ASSERT( false );
  return *this;
}

QgsExpressionContextScope *QgsSymbolRenderContext::expressionContextScope()
{
  return mExpressionContextScope.get();
}

void QgsSymbolRenderContext::setExpressionContextScope( QgsExpressionContextScope *contextScope )
{
  mExpressionContextScope.reset( contextScope );
}

///////////////////

QgsMarkerSymbol *QgsMarkerSymbol::createSimple( const QgsStringMap &properties )
{
  QgsSymbolLayer *sl = QgsSimpleMarkerSymbolLayer::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsMarkerSymbol( layers );
}

QgsLineSymbol *QgsLineSymbol::createSimple( const QgsStringMap &properties )
{
  QgsSymbolLayer *sl = QgsSimpleLineSymbolLayer::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsLineSymbol( layers );
}

QgsFillSymbol *QgsFillSymbol::createSimple( const QgsStringMap &properties )
{
  QgsSymbolLayer *sl = QgsSimpleFillSymbolLayer::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsFillSymbol( layers );
}

///////////////////

QgsMarkerSymbol::QgsMarkerSymbol( const QgsSymbolLayerList &layers )
  : QgsSymbol( Marker, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleMarkerSymbolLayer() );
}

void QgsMarkerSymbol::setAngle( double symbolAngle )
{
  double origAngle = angle();
  double angleDiff = symbolAngle - origAngle;
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer );
    if ( markerLayer )
      markerLayer->setAngle( markerLayer->angle() + angleDiff );
  }
}

double QgsMarkerSymbol::angle() const
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    return markerLayer->angle();
  }
  return 0;
}

void QgsMarkerSymbol::setLineAngle( double lineAng )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setLineAngle( lineAng );
  }
}

void QgsMarkerSymbol::setDataDefinedAngle( const QgsProperty &property )
{
  const double symbolRotation = angle();

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    if ( !property )
    {
      layer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty() );
    }
    else
    {
      if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) )
      {
        layer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, property );
      }
      else
      {
        QgsProperty rotatedDD = rotateWholeSymbol( markerLayer->angle() - symbolRotation, property );
        layer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, rotatedDD );
      }
    }
  }
}

QgsProperty QgsMarkerSymbol::dataDefinedAngle() const
{
  const double symbolRotation = angle();
  QgsProperty symbolDD;

  // find the base of the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) && markerLayer->dataDefinedProperties().isActive( QgsSymbolLayer::PropertyAngle ) )
    {
      symbolDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyAngle );
      break;
    }
  }

  if ( !symbolDD )
    return QgsProperty();

  // check that all layer's angle expressions match the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );

    QgsProperty layerAngleDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyAngle );

    if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) )
    {
      if ( !layerAngleDD || layerAngleDD != symbolDD )
        return QgsProperty();
    }
    else
    {
      QgsProperty rotatedDD( rotateWholeSymbol( markerLayer->angle() - symbolRotation, symbolDD ) );
      if ( !layerAngleDD || layerAngleDD != rotatedDD )
        return QgsProperty();
    }
  }
  return symbolDD;
}


void QgsMarkerSymbol::setSize( double s )
{
  double origSize = size();

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
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

double QgsMarkerSymbol::size() const
{
  // return size of the largest symbol
  double maxSize = 0;
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    double lsize = markerLayer->size();
    if ( lsize > maxSize )
      maxSize = lsize;
  }
  return maxSize;
}

void QgsMarkerSymbol::setSizeUnit( QgsUnitTypes::RenderUnit unit )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;

    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setSizeUnit( unit );
  }
}

QgsUnitTypes::RenderUnit QgsMarkerSymbol::sizeUnit() const
{
  bool first = true;
  QgsUnitTypes::RenderUnit unit = QgsUnitTypes::RenderUnknownUnit;

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );

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

void QgsMarkerSymbol::setSizeMapUnitScale( const QgsMapUnitScale &scale )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;

    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setSizeMapUnitScale( scale );
  }
}

QgsMapUnitScale QgsMarkerSymbol::sizeMapUnitScale() const
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;

    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    return markerLayer->sizeMapUnitScale();
  }
  return QgsMapUnitScale();
}

void QgsMarkerSymbol::setDataDefinedSize( const QgsProperty &property )
{
  const double symbolSize = size();

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );

    if ( !property )
    {
      markerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty() );
      markerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsProperty() );
    }
    else
    {
      if ( qgsDoubleNear( symbolSize, 0.0 ) || qgsDoubleNear( markerLayer->size(), symbolSize ) )
      {
        markerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, property );
      }
      else
      {
        markerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, scaleWholeSymbol( markerLayer->size() / symbolSize, property ) );
      }

      if ( !qgsDoubleNear( markerLayer->offset().x(), 0.0 ) || !qgsDoubleNear( markerLayer->offset().y(), 0.0 ) )
      {
        markerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, scaleWholeSymbol(
                                               markerLayer->offset().x() / symbolSize,
                                               markerLayer->offset().y() / symbolSize, property ) );
      }
    }
  }
}

QgsProperty QgsMarkerSymbol::dataDefinedSize() const
{
  const double symbolSize = size();

  QgsProperty symbolDD;

  // find the base of the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    if ( qgsDoubleNear( markerLayer->size(), symbolSize ) && markerLayer->dataDefinedProperties().isActive( QgsSymbolLayer::PropertySize ) )
    {
      symbolDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertySize );
      break;
    }
  }

  if ( !symbolDD )
    return QgsProperty();

  // check that all layers size expressions match the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );

    QgsProperty layerSizeDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertySize );
    QgsProperty layerOffsetDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyOffset );

    if ( qgsDoubleNear( markerLayer->size(), symbolSize ) )
    {
      if ( !layerSizeDD || layerSizeDD != symbolDD )
        return QgsProperty();
    }
    else
    {
      if ( qgsDoubleNear( symbolSize, 0.0 ) )
        return QgsProperty();

      QgsProperty scaledDD( scaleWholeSymbol( markerLayer->size() / symbolSize, symbolDD ) );
      if ( !layerSizeDD || layerSizeDD != scaledDD )
        return QgsProperty();
    }

    QgsProperty scaledOffsetDD( scaleWholeSymbol( markerLayer->offset().x() / symbolSize, markerLayer->offset().y() / symbolSize, symbolDD ) );
    if ( layerOffsetDD && layerOffsetDD != scaledOffsetDD )
      return QgsProperty();
  }

  return symbolDD;
}

void QgsMarkerSymbol::setScaleMethod( QgsSymbol::ScaleMethod scaleMethod )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setScaleMethod( scaleMethod );
  }
}

QgsSymbol::ScaleMethod QgsMarkerSymbol::scaleMethod()
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    // return scale method of the first symbol layer
    return markerLayer->scaleMethod();
  }

  return DEFAULT_SCALE_METHOD;
}

void QgsMarkerSymbol::renderPointUsingLayer( QgsMarkerSymbolLayer *layer, QPointF point, QgsSymbolRenderContext &context )
{
  static QPointF nullPoint( 0, 0 );

  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsPaintEffect *effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext() );
    p->translate( point );
    p.setEffect( effect );
    layer->renderPoint( nullPoint, context );
  }
  else
  {
    layer->renderPoint( point, context );
  }
}

void QgsMarkerSymbol::renderPoint( QPointF point, const QgsFeature *f, QgsRenderContext &context, int layerIdx, bool selected )
{
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mOpacity, selected, mRenderHints, f, QgsFields(), mapUnitScale() );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() )
    {
      if ( symbolLayer->type() == QgsSymbol::Marker )
      {
        QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( symbolLayer );
        renderPointUsingLayer( markerLayer, point, symbolContext );
      }
      else
        renderUsingLayer( symbolLayer, symbolContext );
    }
    return;
  }

  Q_FOREACH ( QgsSymbolLayer *symbolLayer, mLayers )
  {
    if ( !symbolLayer->enabled() )
      continue;

    if ( symbolLayer->type() == QgsSymbol::Marker )
    {
      QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( symbolLayer );
      renderPointUsingLayer( markerLayer, point, symbolContext );
    }
    else
      renderUsingLayer( symbolLayer, symbolContext );
  }
}

QRectF QgsMarkerSymbol::bounds( QPointF point, QgsRenderContext &context, const QgsFeature &feature ) const
{
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mOpacity, false, mRenderHints, &feature, feature.fields(), mapUnitScale() );

  QRectF bound;
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() == QgsSymbol::Marker )
    {
      QgsMarkerSymbolLayer *symbolLayer = static_cast< QgsMarkerSymbolLayer * >( layer );
      if ( bound.isNull() )
        bound = symbolLayer->bounds( point, symbolContext );
      else
        bound = bound.united( symbolLayer->bounds( point, symbolContext ) );
    }
  }
  return bound;
}

QgsMarkerSymbol *QgsMarkerSymbol::clone() const
{
  QgsMarkerSymbol *cloneSymbol = new QgsMarkerSymbol( cloneLayers() );
  cloneSymbol->setOpacity( mOpacity );
  Q_NOWARN_DEPRECATED_PUSH
  cloneSymbol->setLayer( mLayer );
  Q_NOWARN_DEPRECATED_POP
  cloneSymbol->setClipFeaturesToExtent( mClipFeaturesToExtent );
  cloneSymbol->setForceRHR( mForceRHR );
  return cloneSymbol;
}


///////////////////
// LINE

QgsLineSymbol::QgsLineSymbol( const QgsSymbolLayerList &layers )
  : QgsSymbol( Line, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleLineSymbolLayer() );
}

void QgsLineSymbol::setWidth( double w )
{
  double origWidth = width();

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    QgsLineSymbolLayer *lineLayer = dynamic_cast<QgsLineSymbolLayer *>( layer );

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

double QgsLineSymbol::width() const
{
  double maxWidth = 0;
  if ( mLayers.isEmpty() )
    return maxWidth;

  Q_FOREACH ( QgsSymbolLayer *symbolLayer, mLayers )
  {
    const QgsLineSymbolLayer *lineLayer = dynamic_cast<QgsLineSymbolLayer *>( symbolLayer );
    if ( lineLayer )
    {
      double width = lineLayer->width();
      if ( width > maxWidth )
        maxWidth = width;
    }
  }
  return maxWidth;
}

void QgsLineSymbol::setDataDefinedWidth( const QgsProperty &property )
{
  const double symbolWidth = width();

  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    QgsLineSymbolLayer *lineLayer = dynamic_cast<QgsLineSymbolLayer *>( layer );

    if ( lineLayer )
    {
      if ( !property )
      {
        lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, QgsProperty() );
        lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsProperty() );
      }
      else
      {
        if ( qgsDoubleNear( symbolWidth, 0.0 ) || qgsDoubleNear( lineLayer->width(), symbolWidth ) )
        {
          lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, property );
        }
        else
        {
          lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, scaleWholeSymbol( lineLayer->width() / symbolWidth, property ) );
        }

        if ( !qgsDoubleNear( lineLayer->offset(), 0.0 ) )
        {
          lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, scaleWholeSymbol( lineLayer->offset() / symbolWidth, property ) );
        }
      }
    }
  }
}

QgsProperty QgsLineSymbol::dataDefinedWidth() const
{
  const double symbolWidth = width();

  QgsProperty symbolDD;

  // find the base of the "en masse" pattern
  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    const QgsLineSymbolLayer *layer = dynamic_cast<const QgsLineSymbolLayer *>( *it );
    if ( layer && qgsDoubleNear( layer->width(), symbolWidth ) && layer->dataDefinedProperties().isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
    {
      symbolDD = layer->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeWidth );
      break;
    }
  }

  if ( !symbolDD )
    return QgsProperty();

  // check that all layers width expressions match the "en masse" pattern
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Line )
      continue;
    const QgsLineSymbolLayer *lineLayer = static_cast<const QgsLineSymbolLayer *>( layer );

    QgsProperty layerWidthDD = lineLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeWidth );
    QgsProperty layerOffsetDD = lineLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyOffset );

    if ( qgsDoubleNear( lineLayer->width(), symbolWidth ) )
    {
      if ( !layerWidthDD || layerWidthDD != symbolDD )
        return QgsProperty();
    }
    else
    {
      if ( qgsDoubleNear( symbolWidth, 0.0 ) )
        return QgsProperty();

      QgsProperty scaledDD( scaleWholeSymbol( lineLayer->width() / symbolWidth, symbolDD ) );
      if ( !layerWidthDD || layerWidthDD != scaledDD )
        return QgsProperty();
    }

    QgsProperty scaledOffsetDD( scaleWholeSymbol( lineLayer->offset() / symbolWidth, symbolDD ) );
    if ( layerOffsetDD && layerOffsetDD != scaledOffsetDD )
      return QgsProperty();
  }

  return symbolDD;
}

void QgsLineSymbol::renderPolyline( const QPolygonF &points, const QgsFeature *f, QgsRenderContext &context, int layerIdx, bool selected )
{
  //save old painter
  QPainter *renderPainter = context.painter();
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mOpacity, selected, mRenderHints, f, QgsFields(), mapUnitScale() );
  symbolContext.setOriginalGeometryType( QgsWkbTypes::LineGeometry );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() )
    {
      if ( symbolLayer->type() == QgsSymbol::Line )
      {
        QgsLineSymbolLayer *lineLayer = static_cast<QgsLineSymbolLayer *>( symbolLayer );
        renderPolylineUsingLayer( lineLayer, points, symbolContext );
      }
      else
        renderUsingLayer( symbolLayer, symbolContext );
    }
    return;
  }

  Q_FOREACH ( QgsSymbolLayer *symbolLayer, mLayers )
  {
    if ( !symbolLayer->enabled() )
      continue;

    if ( symbolLayer->type() == QgsSymbol::Line )
    {
      QgsLineSymbolLayer *lineLayer = static_cast<QgsLineSymbolLayer *>( symbolLayer );
      renderPolylineUsingLayer( lineLayer, points, symbolContext );
    }
    else
    {
      renderUsingLayer( symbolLayer, symbolContext );
    }
  }

  context.setPainter( renderPainter );
}

void QgsLineSymbol::renderPolylineUsingLayer( QgsLineSymbolLayer *layer, const QPolygonF &points, QgsSymbolRenderContext &context )
{
  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsPaintEffect *effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext() );
    p->translate( points.boundingRect().topLeft() );
    p.setEffect( effect );
    layer->renderPolyline( points.translated( -points.boundingRect().topLeft() ), context );
  }
  else
  {
    layer->renderPolyline( points, context );
  }
}


QgsLineSymbol *QgsLineSymbol::clone() const
{
  QgsLineSymbol *cloneSymbol = new QgsLineSymbol( cloneLayers() );
  cloneSymbol->setOpacity( mOpacity );
  Q_NOWARN_DEPRECATED_PUSH
  cloneSymbol->setLayer( mLayer );
  Q_NOWARN_DEPRECATED_POP
  cloneSymbol->setClipFeaturesToExtent( mClipFeaturesToExtent );
  cloneSymbol->setForceRHR( mForceRHR );
  return cloneSymbol;
}

///////////////////
// FILL

QgsFillSymbol::QgsFillSymbol( const QgsSymbolLayerList &layers )
  : QgsSymbol( Fill, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleFillSymbolLayer() );
}

void QgsFillSymbol::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, const QgsFeature *f, QgsRenderContext &context, int layerIdx, bool selected )
{
  QgsSymbolRenderContext symbolContext( context, outputUnit(), mOpacity, selected, mRenderHints, f, QgsFields(), mapUnitScale() );
  symbolContext.setOriginalGeometryType( QgsWkbTypes::PolygonGeometry );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() )
    {
      if ( symbolLayer->type() == Fill || symbolLayer->type() == Line )
        renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
      else
        renderUsingLayer( symbolLayer, symbolContext );
    }
    return;
  }

  Q_FOREACH ( QgsSymbolLayer *symbolLayer, mLayers )
  {
    if ( !symbolLayer->enabled() )
      continue;

    if ( symbolLayer->type() == Fill || symbolLayer->type() == Line )
      renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
    else
      renderUsingLayer( symbolLayer, symbolContext );
  }
}

void QgsFillSymbol::renderPolygonUsingLayer( QgsSymbolLayer *layer, const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsSymbol::SymbolType layertype = layer->type();

  QgsPaintEffect *effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QRectF bounds = polygonBounds( points, rings );
    QList<QPolygonF> *translatedRings = translateRings( rings, -bounds.left(), -bounds.top() );

    QgsEffectPainter p( context.renderContext() );
    p->translate( bounds.topLeft() );
    p.setEffect( effect );
    if ( layertype == QgsSymbol::Fill )
    {
      ( static_cast<QgsFillSymbolLayer *>( layer ) )->renderPolygon( points.translated( -bounds.topLeft() ), translatedRings, context );
    }
    else if ( layertype == QgsSymbol::Line )
    {
      ( static_cast<QgsLineSymbolLayer *>( layer ) )->renderPolygonStroke( points.translated( -bounds.topLeft() ), translatedRings, context );
    }
    delete translatedRings;
  }
  else
  {
    if ( layertype == QgsSymbol::Fill )
    {
      ( static_cast<QgsFillSymbolLayer *>( layer ) )->renderPolygon( points, rings, context );
    }
    else if ( layertype == QgsSymbol::Line )
    {
      ( static_cast<QgsLineSymbolLayer *>( layer ) )->renderPolygonStroke( points, rings, context );
    }
  }
}

QRectF QgsFillSymbol::polygonBounds( const QPolygonF &points, const QList<QPolygonF> *rings ) const
{
  QRectF bounds = points.boundingRect();
  if ( rings )
  {
    QList<QPolygonF>::const_iterator it = rings->constBegin();
    for ( ; it != rings->constEnd(); ++it )
    {
      bounds = bounds.united( ( *it ).boundingRect() );
    }
  }
  return bounds;
}

QList<QPolygonF> *QgsFillSymbol::translateRings( const QList<QPolygonF> *rings, double dx, double dy ) const
{
  if ( !rings )
    return nullptr;

  QList<QPolygonF> *translatedRings = new QList<QPolygonF>;
  QList<QPolygonF>::const_iterator it = rings->constBegin();
  for ( ; it != rings->constEnd(); ++it )
  {
    translatedRings->append( ( *it ).translated( dx, dy ) );
  }
  return translatedRings;
}

QgsFillSymbol *QgsFillSymbol::clone() const
{
  QgsFillSymbol *cloneSymbol = new QgsFillSymbol( cloneLayers() );
  cloneSymbol->setOpacity( mOpacity );
  Q_NOWARN_DEPRECATED_PUSH
  cloneSymbol->setLayer( mLayer );
  Q_NOWARN_DEPRECATED_POP
  cloneSymbol->setClipFeaturesToExtent( mClipFeaturesToExtent );
  cloneSymbol->setForceRHR( mForceRHR );
  return cloneSymbol;
}

void QgsFillSymbol::setAngle( double angle )
{
  Q_FOREACH ( QgsSymbolLayer *layer, mLayers )
  {
    if ( layer->type() != QgsSymbol::Fill )
      continue;

    QgsFillSymbolLayer *fillLayer = static_cast<QgsFillSymbolLayer *>( layer );

    if ( fillLayer )
      fillLayer->setAngle( angle );
  }
}


