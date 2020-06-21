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
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgslegendpatchshape.h"

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
    try
    {
      ct.transformPolygon( pts );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
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
    try
    {
      ct.transformPolygon( poly );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
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

  if ( !poly.empty() && !poly.isClosed() )
    poly << poly.at( 0 );

  return poly;
}

void QgsSymbol::_getPolygon( QPolygonF &pts, QVector<QPolygonF> &holes, QgsRenderContext &context, const QgsPolygon &polygon, const bool clipToExtent, const bool correctRingOrientation )
{
  holes.clear();

  pts = _getPolygonRing( context, *polygon.exteriorRing(), clipToExtent, true, correctRingOrientation );
  const int ringCount = polygon.numInteriorRings();
  holes.reserve( ringCount );
  for ( int idx = 0; idx < ringCount; idx++ )
  {
    const QPolygonF hole = _getPolygonRing( context, *( polygon.interiorRing( idx ) ), clipToExtent, false, correctRingOrientation );
    if ( !hole.isEmpty() )
      holes.append( hole );
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    layer->setOutputUnit( u );
  }
}

void QgsSymbol::setMapUnitScale( const QgsMapUnitScale &scale )
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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

const QgsSymbolLayer *QgsSymbol::symbolLayer( int layer ) const
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

  mSymbolRenderContext.reset( new QgsSymbolRenderContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, false, mRenderHints, nullptr, fields ) );

  // Why do we need a copy here ? Is it to make sure the symbol layer rendering does not mess with the symbol render context ?
  // Or is there another profound reason ?
  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, false, mRenderHints, nullptr, fields );

  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::updateSymbolScope( this, new QgsExpressionContextScope() ) );
  mSymbolRenderContext->setExpressionContextScope( scope.release() );

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( !layer->enabled() || !context.isSymbolLayerEnabled( layer ) )
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
    const auto constMLayers = mLayers;
    for ( QgsSymbolLayer *layer : constMLayers )
    {
      if ( !layer->enabled()  || !context.isSymbolLayerEnabled( layer ) )
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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

void QgsSymbol::drawPreviewIcon( QPainter *painter, QSize size, QgsRenderContext *customContext, bool selected, const QgsExpressionContext *expressionContext, const QgsLegendPatchShape *patchShape )
{
  QgsRenderContext *context = customContext;
  std::unique_ptr< QgsRenderContext > tempContext;
  if ( !context )
  {
    tempContext.reset( new QgsRenderContext( QgsRenderContext::fromQPainter( painter ) ) );
    context = tempContext.get();
    context->setFlag( QgsRenderContext::RenderSymbolPreview, true );
  }

  const bool prevForceVector = context->forceVectorOutput();
  context->setForceVectorOutput( true );
  QgsSymbolRenderContext symbolContext( *context, QgsUnitTypes::RenderUnknownUnit, mOpacity, false, mRenderHints, nullptr );
  symbolContext.setSelected( selected );
  symbolContext.setOriginalGeometryType( mType == Fill ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::UnknownGeometry );
  if ( patchShape )
    symbolContext.setPatchShape( *patchShape );

  if ( !customContext && expressionContext )
  {
    context->setExpressionContext( *expressionContext );
  }
  else if ( !customContext )
  {
    // if no render context was passed, build a minimal expression context
    QgsExpressionContext expContext;
    expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
    context->setExpressionContext( expContext );
  }

  for ( QgsSymbolLayer *layer : qgis::as_const( mLayers ) )
  {
    if ( !layer->enabled()  || ( customContext && !customContext->isSymbolLayerEnabled( layer ) ) )
      continue;

    if ( mType == Fill && layer->type() == Line )
    {
      // line symbol layer would normally draw just a line
      // so we override this case to force it to draw a polygon stroke
      QgsLineSymbolLayer *lsl = dynamic_cast<QgsLineSymbolLayer *>( layer );
      if ( lsl )
      {
        // from QgsFillSymbolLayer::drawPreviewIcon() -- would be nicer to add the
        // symbol type to QgsSymbolLayer::drawPreviewIcon so this logic could be avoided!

        // hmm... why was this using size -1 ??
        const QSizeF targetSize = QSizeF( size.width() - 1, size.height() - 1 );

        const QList< QList< QPolygonF > > polys = patchShape ? patchShape->toQPolygonF( QgsSymbol::Fill, targetSize )
            : QgsStyle::defaultStyle()->defaultPatchAsQPolygonF( QgsSymbol::Fill, targetSize );

        lsl->startRender( symbolContext );
        QgsPaintEffect *effect = lsl->paintEffect();

        std::unique_ptr< QgsEffectPainter > effectPainter;
        if ( effect && effect->enabled() )
          effectPainter = qgis::make_unique< QgsEffectPainter >( symbolContext.renderContext(), effect );

        for ( const QList< QPolygonF > &poly : polys )
        {
          QVector< QPolygonF > rings;
          for ( int i = 1; i < poly.size(); ++i )
            rings << poly.at( i );
          lsl->renderPolygonStroke( poly.value( 0 ), &rings, symbolContext );
        }

        effectPainter.reset();
        lsl->stopRender( symbolContext );
      }
    }
    else
      layer->drawPreviewIcon( symbolContext, size );
  }

  context->setForceVectorOutput( prevForceVector );
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
  p.translate( 0.5, 0.5 ); // shift by half a pixel to avoid blurring due antialiasing

  if ( mType == QgsSymbol::Marker )
  {
    p.setPen( QPen( Qt::gray ) );
    p.drawLine( 0, 50, 100, 50 );
    p.drawLine( 50, 0, 50, 100 );
  }

  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );
  context.setFlag( QgsRenderContext::RenderSymbolPreview );
  if ( expressionContext )
    context.setExpressionContext( *expressionContext );

  context.setIsGuiPreview( true );
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->hasDataDefinedProperties() )
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

void QgsSymbol::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker, int currentVertexMarkerType, double currentVertexMarkerSize )
{
  if ( context.renderingStopped() )
    return;

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
  bool needsSegmentizing = QgsWkbTypes::isCurvedType( geom.constGet()->wkbType() ) || geom.constGet()->hasCurvedSegments();
  if ( !needsSegmentizing )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() ) )
    {
      for ( int i = 0; i < collection->numGeometries(); ++i )
      {
        if ( QgsWkbTypes::isCurvedType( collection->geometryN( i )->wkbType() ) )
        {
          needsSegmentizing = true;
          break;
        }
      }
    }
  }

  if ( needsSegmentizing )
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

  const bool needsExpressionContext = hasDataDefinedProperties();
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

  QgsGeometry renderedBoundsGeom;

  // Step 1 - collect the set of painter coordinate geometries to render.
  // We do this upfront, because we only want to ever do this once, regardless how many symbol layers we need to render.

  struct PointInfo
  {
    QPointF renderPoint;
    const QgsPoint *originalGeometry = nullptr;
  };
  QVector< PointInfo > pointsToRender;

  struct LineInfo
  {
    QPolygonF renderLine;
    const QgsCurve *originalGeometry = nullptr;
  };
  QVector< LineInfo > linesToRender;

  struct PolygonInfo
  {
    QPolygonF renderExterior;
    QVector< QPolygonF > renderRings;
    const QgsPolygon *originalGeometry = nullptr;
  };
  QVector< PolygonInfo > polygonsToRender;

  std::function< void ( const QgsAbstractGeometry * )> getPartGeometry;
  getPartGeometry = [&pointsToRender, &linesToRender, &polygonsToRender, &getPartGeometry, &context, &tileMapRendering, &markers, &feature, this]( const QgsAbstractGeometry * part )
  {
    Q_UNUSED( feature )

    if ( !part )
      return;

    switch ( QgsWkbTypes::flatType( part->wkbType() ) )
    {
      case QgsWkbTypes::Point:
      {
        if ( mType != QgsSymbol::Marker )
        {
          QgsDebugMsgLevel( QStringLiteral( "point can be drawn only with marker symbol!" ), 2 );
          break;
        }

        PointInfo info;
        info.originalGeometry = qgsgeometry_cast< const QgsPoint * >( part );
        info.renderPoint = _getPoint( context, *info.originalGeometry );
        pointsToRender << info;
        break;
      }

      case QgsWkbTypes::LineString:
      {
        if ( mType != QgsSymbol::Line )
        {
          QgsDebugMsgLevel( QStringLiteral( "linestring can be drawn only with line symbol!" ), 2 );
          break;
        }

        LineInfo info;
        info.originalGeometry = qgsgeometry_cast<const QgsCurve *>( part );
        info.renderLine = _getLineString( context, *info.originalGeometry, !tileMapRendering && clipFeaturesToExtent() );
        linesToRender << info;
        break;
      }

      case QgsWkbTypes::Polygon:
      case QgsWkbTypes::Triangle:
      {
        QPolygonF pts;
        QVector<QPolygonF> holes;
        if ( mType != QgsSymbol::Fill )
        {
          QgsDebugMsgLevel( QStringLiteral( "polygon can be drawn only with fill symbol!" ), 2 );
          break;
        }

        PolygonInfo info;
        info.originalGeometry = qgsgeometry_cast<const QgsPolygon *>( part );
        if ( !info.originalGeometry->exteriorRing() )
        {
          QgsDebugMsg( QStringLiteral( "cannot render polygon with no exterior ring" ) );
          break;
        }

        _getPolygon( info.renderExterior, info.renderRings, context, *info.originalGeometry, !tileMapRendering && clipFeaturesToExtent(), mForceRHR );
        polygonsToRender << info;
        break;
      }

      case QgsWkbTypes::MultiPoint:
      {
        const QgsMultiPoint *mp = qgsgeometry_cast< const QgsMultiPoint * >( part );
        markers.reserve( mp->numGeometries() );
      }
      FALLTHROUGH
      case QgsWkbTypes::MultiCurve:
      case QgsWkbTypes::MultiLineString:
      case QgsWkbTypes::GeometryCollection:
      {
        const QgsGeometryCollection *geomCollection = qgsgeometry_cast<const QgsGeometryCollection *>( part );

        const unsigned int num = geomCollection->numGeometries();
        for ( unsigned int i = 0; i < num; ++i )
        {
          if ( context.renderingStopped() )
            break;

          getPartGeometry( geomCollection->geometryN( i ) );
        }
        break;
      }

      case QgsWkbTypes::MultiSurface:
      case QgsWkbTypes::MultiPolygon:
      {
        if ( mType != QgsSymbol::Fill )
        {
          QgsDebugMsgLevel( QStringLiteral( "multi-polygon can be drawn only with fill symbol!" ), 2 );
          break;
        }

        QPolygonF pts;
        QVector<QPolygonF> holes;

        const QgsGeometryCollection *geomCollection = dynamic_cast<const QgsGeometryCollection *>( part );
        const unsigned int num = geomCollection->numGeometries();

        // Sort components by approximate area (probably a bit faster than using
        // area() )
        std::map<double, QList<unsigned int> > thisAreaToPartNum;
        for ( unsigned int i = 0; i < num; ++i )
        {
          const QgsPolygon *polygon = qgsgeometry_cast<const QgsPolygon *>( geomCollection->geometryN( i ) );
          const QgsRectangle r( polygon->boundingBox() );
          thisAreaToPartNum[ r.width() * r.height()] << i;
        }

        // Draw starting with larger parts down to smaller parts, so that in
        // case of a part being incorrectly inside another part, it is drawn
        // on top of it (#15419)
        std::map<double, QList<unsigned int> >::const_reverse_iterator iter = thisAreaToPartNum.rbegin();
        for ( ; iter != thisAreaToPartNum.rend(); ++iter )
        {
          const QList<unsigned int> &listPartIndex = iter->second;
          for ( int idx = 0; idx < listPartIndex.size(); ++idx )
          {
            const unsigned i = listPartIndex[idx];
            const QgsPolygon *polygon = qgsgeometry_cast<const QgsPolygon *>( geomCollection->geometryN( i ) );
            getPartGeometry( polygon );
          }
        }
        break;
      }

      default:
        QgsDebugMsg( QStringLiteral( "feature %1: unsupported wkb type %2/%3 for rendering" )
                     .arg( feature.id() )
                     .arg( QgsWkbTypes::displayString( part->wkbType() ) )
                     .arg( part->wkbType(), 0, 16 ) );
    }
  };

  getPartGeometry( segmentizedGeometry.constGet() );

  // step 2 - determine which layers to render
  std::vector< int > layers;
  if ( layer == -1 )
  {
    layers.reserve( mLayers.count() );
    for ( int i = 0; i < mLayers.count(); ++i )
      layers.emplace_back( i );
  }
  else
  {
    layers.emplace_back( layer );
  }

  // step 3 - render these geometries using the desired symbol layers.

  if ( needsExpressionContext )
    mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_layer_count" ), mLayers.count(), true ) );

  for ( const int symbolLayerIndex : layers )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( symbolLayerIndex );
    if ( !symbolLayer || !symbolLayer->enabled() )
      continue;

    if ( needsExpressionContext )
      mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_layer_index" ), symbolLayerIndex + 1, true ) );

    symbolLayer->startFeatureRender( feature, context );

    switch ( mType )
    {
      case QgsSymbol::Marker:
      {
        int geometryPartNumber = 0;
        for ( const PointInfo &point : qgis::as_const( pointsToRender ) )
        {
          if ( context.renderingStopped() )
            break;

          mSymbolRenderContext->setGeometryPartNum( geometryPartNumber + 1 );
          if ( needsExpressionContext )
            mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, geometryPartNumber + 1, true ) );

          static_cast<QgsMarkerSymbol *>( this )->renderPoint( point.renderPoint, &feature, context, symbolLayerIndex, selected );
          geometryPartNumber++;
        }

        break;
      }

      case QgsSymbol::Line:
      {
        if ( linesToRender.empty() )
          break;

        int geometryPartNumber = 0;
        for ( const LineInfo &line : linesToRender )
        {
          if ( context.renderingStopped() )
            break;

          mSymbolRenderContext->setGeometryPartNum( geometryPartNumber + 1 );
          if ( needsExpressionContext )
            mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, geometryPartNumber + 1, true ) );

          context.setGeometry( line.originalGeometry );
          static_cast<QgsLineSymbol *>( this )->renderPolyline( line.renderLine, &feature, context, symbolLayerIndex, selected );
          geometryPartNumber++;
        }
        break;
      }

      case QgsSymbol::Fill:
      {
        int geometryPartNumber = 0;
        for ( const PolygonInfo &info : polygonsToRender )
        {
          if ( context.renderingStopped() )
            break;

          mSymbolRenderContext->setGeometryPartNum( geometryPartNumber + 1 );
          if ( needsExpressionContext )
            mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, geometryPartNumber + 1, true ) );

          context.setGeometry( info.originalGeometry );
          static_cast<QgsFillSymbol *>( this )->renderPolygon( info.renderExterior, ( !info.renderRings.isEmpty() ? &info.renderRings : nullptr ), &feature, context, symbolLayerIndex, selected );
          geometryPartNumber++;
        }

        break;
      }

      case QgsSymbol::Hybrid:
        break;
    }

    symbolLayer->stopFeatureRender( feature, context );
  }

  // step 4 - handle post processing steps
  switch ( mType )
  {
    case QgsSymbol::Marker:
    {
      markers.reserve( pointsToRender.size() );
      for ( const PointInfo &info : qgis::as_const( pointsToRender ) )
      {
        if ( context.hasRenderedFeatureHandlers() || context.testFlag( QgsRenderContext::DrawSymbolBounds ) )
        {
          const QRectF bounds = static_cast<QgsMarkerSymbol *>( this )->bounds( info.renderPoint, context, feature );
          if ( context.hasRenderedFeatureHandlers() )
          {
            renderedBoundsGeom = renderedBoundsGeom.isNull() ? QgsGeometry::fromRect( bounds )
                                 : QgsGeometry::collectGeometry( QVector< QgsGeometry>() << QgsGeometry::fromRect( QgsRectangle( bounds ) ) << renderedBoundsGeom );
          }
          if ( context.testFlag( QgsRenderContext::DrawSymbolBounds ) )
          {
            //draw debugging rect
            context.painter()->setPen( Qt::red );
            context.painter()->setBrush( QColor( 255, 0, 0, 100 ) );
            context.painter()->drawRect( bounds );
          }
        }

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          markers.append( info.renderPoint );
        }
      }
      break;
    }

    case QgsSymbol::Line:
    {
      for ( const LineInfo &info : qgis::as_const( linesToRender ) )
      {
        if ( context.hasRenderedFeatureHandlers() )
        {
          renderedBoundsGeom = renderedBoundsGeom.isNull() ? QgsGeometry::fromQPolygonF( info.renderLine )
                               : QgsGeometry::collectGeometry( QVector< QgsGeometry>() << QgsGeometry::fromQPolygonF( info.renderLine ) << renderedBoundsGeom );
        }

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          markers << info.renderLine;
        }
      }
      break;
    }

    case QgsSymbol::Fill:
    {
      int i = 0;
      for ( const PolygonInfo &info : qgis::as_const( polygonsToRender ) )
      {
        if ( context.hasRenderedFeatureHandlers() )
        {
          renderedBoundsGeom = renderedBoundsGeom.isNull() ? QgsGeometry::fromQPolygonF( info.renderExterior )
                               : QgsGeometry::collectGeometry( QVector< QgsGeometry>() << QgsGeometry::fromQPolygonF( info.renderExterior ) << renderedBoundsGeom );
          // TODO: consider holes?
        }

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          markers << info.renderExterior;

          for ( const QPolygonF &hole : info.renderRings )
          {
            markers << hole;
          }
        }
        i++;
      }
      break;
    }

    case QgsSymbol::Hybrid:
      break;
  }

  if ( context.hasRenderedFeatureHandlers() )
  {
    QgsRenderedFeatureHandlerInterface::RenderedFeatureContext featureContext( context );
    const QList< QgsRenderedFeatureHandlerInterface * > handlers = context.renderedFeatureHandlers();
    for ( QgsRenderedFeatureHandlerInterface *handler : handlers )
      handler->handleRenderedFeature( feature, renderedBoundsGeom, featureContext );
  }

  if ( drawVertexMarker )
  {
    if ( !markers.isEmpty() && !context.renderingStopped() )
    {
      const auto constMarkers = markers;
      for ( QPointF marker : constMarkers )
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

void QgsSymbol::renderVertexMarker( QPointF pt, QgsRenderContext &context, int currentVertexMarkerType, double currentVertexMarkerSize )
{
  int markerSize = context.convertToPainterUnits( currentVertexMarkerSize, QgsUnitTypes::RenderMillimeters );
  QgsSymbolLayerUtils::drawVertexMarker( pt.x(), pt.y(), *context.painter(), static_cast< QgsSymbolLayerUtils::VertexMarkerType >( currentVertexMarkerType ), markerSize );
}

void QgsSymbol::startFeatureRender( const QgsFeature &feature, QgsRenderContext &context, const int layer )
{
  if ( layer != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layer );
    if ( symbolLayer && symbolLayer->enabled() )
    {
      symbolLayer->startFeatureRender( feature, context );
    }
    return;
  }
  else
  {
    const QList< QgsSymbolLayer * > layers = mLayers;
    for ( QgsSymbolLayer *symbolLayer : layers )
    {
      if ( !symbolLayer->enabled() )
        continue;

      symbolLayer->startFeatureRender( feature, context );
    }
  }
}

void QgsSymbol::stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context, int layer )
{
  if ( layer != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layer );
    if ( symbolLayer && symbolLayer->enabled() )
    {
      symbolLayer->stopFeatureRender( feature, context );
    }
    return;
  }
  else
  {
    const QList< QgsSymbolLayer * > layers = mLayers;
    for ( QgsSymbolLayer *symbolLayer : layers )
    {
      if ( !symbolLayer->enabled() )
        continue;

      symbolLayer->stopFeatureRender( feature, context );
    }
  }
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

QgsSymbolRenderContext::~QgsSymbolRenderContext() = default;

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

// cppcheck-suppress operatorEqVarError
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

const QgsLegendPatchShape *QgsSymbolRenderContext::patchShape() const
{
  return mPatchShape.get();
}

void QgsSymbolRenderContext::setPatchShape( const QgsLegendPatchShape &patchShape )
{
  mPatchShape.reset( new QgsLegendPatchShape( patchShape ) );
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer );
    if ( markerLayer )
      markerLayer->setAngle( markerLayer->angle() + angleDiff );
  }
}

double QgsMarkerSymbol::angle() const
{
  for ( QgsSymbolLayer *layer : qgis::as_const( mLayers ) )
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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


  for ( QgsSymbolLayer *layer : qgis::as_const( mLayers ) )
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
  const auto layers = mLayers;
  for ( QgsSymbolLayer *layer : layers )
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
  for ( QgsSymbolLayer *layer : layers )
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

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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

double QgsMarkerSymbol::size( const QgsRenderContext &context ) const
{
  // return size of the largest symbol
  double maxSize = 0;
  for ( QgsSymbolLayer *layer : mLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    const double layerSize = context.convertToPainterUnits( markerLayer->size(), markerLayer->sizeUnit(), markerLayer->sizeMapUnitScale() );
    maxSize = std::max( maxSize, layerSize );
  }
  return maxSize;
}

void QgsMarkerSymbol::setSizeUnit( QgsUnitTypes::RenderUnit unit )
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;

    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setSizeMapUnitScale( scale );
  }
}

QgsMapUnitScale QgsMarkerSymbol::sizeMapUnitScale() const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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
  const auto layers = mLayers;
  for ( QgsSymbolLayer *layer : layers )
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
  for ( QgsSymbolLayer *layer : layers )
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != QgsSymbol::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setScaleMethod( scaleMethod );
  }
}

QgsSymbol::ScaleMethod QgsMarkerSymbol::scaleMethod()
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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
  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, selected, mRenderHints, f );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() && context.isSymbolLayerEnabled( symbolLayer ) )
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


  for ( QgsSymbolLayer *symbolLayer : qgis::as_const( mLayers ) )
  {
    if ( context.renderingStopped() )
      break;

    if ( !symbolLayer->enabled() || !context.isSymbolLayerEnabled( symbolLayer ) )
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
  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, false, mRenderHints, &feature, feature.fields() );

  QRectF bound;
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *symbolLayer : constMLayers )
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

double QgsLineSymbol::width( const QgsRenderContext &context ) const
{
  // return width of the largest symbol
  double maxWidth = 0;
  for ( QgsSymbolLayer *layer : mLayers )
  {
    if ( layer->type() != QgsSymbol::Line )
      continue;
    const QgsLineSymbolLayer *lineLayer = static_cast<const QgsLineSymbolLayer *>( layer );
    const double layerWidth = lineLayer->width( context );
    maxWidth = std::max( maxWidth, layerWidth );
  }
  return maxWidth;
}

void QgsLineSymbol::setDataDefinedWidth( const QgsProperty &property )
{
  const double symbolWidth = width();

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
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
  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, selected, mRenderHints, f );
  symbolContext.setOriginalGeometryType( QgsWkbTypes::LineGeometry );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() && context.isSymbolLayerEnabled( symbolLayer ) )
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

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *symbolLayer : constMLayers )
  {
    if ( context.renderingStopped() )
      break;

    if ( !symbolLayer->enabled() || !context.isSymbolLayerEnabled( symbolLayer ) )
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

void QgsFillSymbol::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, const QgsFeature *f, QgsRenderContext &context, int layerIdx, bool selected )
{
  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, selected, mRenderHints, f );
  symbolContext.setOriginalGeometryType( QgsWkbTypes::PolygonGeometry );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() && context.isSymbolLayerEnabled( symbolLayer ) )
    {
      if ( symbolLayer->type() == Fill || symbolLayer->type() == Line )
        renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
      else
        renderUsingLayer( symbolLayer, symbolContext );
    }
    return;
  }

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *symbolLayer : constMLayers )
  {
    if ( context.renderingStopped() )
      break;

    if ( !symbolLayer->enabled() || !context.isSymbolLayerEnabled( symbolLayer ) )
      continue;

    if ( symbolLayer->type() == Fill || symbolLayer->type() == Line )
      renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
    else
      renderUsingLayer( symbolLayer, symbolContext );
  }
}

void QgsFillSymbol::renderPolygonUsingLayer( QgsSymbolLayer *layer, const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsSymbol::SymbolType layertype = layer->type();

  QgsPaintEffect *effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QRectF bounds = polygonBounds( points, rings );
    QVector<QPolygonF> *translatedRings = translateRings( rings, -bounds.left(), -bounds.top() );

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

QRectF QgsFillSymbol::polygonBounds( const QPolygonF &points, const QVector<QPolygonF> *rings ) const
{
  QRectF bounds = points.boundingRect();
  if ( rings )
  {
    for ( auto it = rings->constBegin(); it != rings->constEnd(); ++it )
    {
      bounds = bounds.united( ( *it ).boundingRect() );
    }
  }
  return bounds;
}

QVector<QPolygonF> *QgsFillSymbol::translateRings( const QVector<QPolygonF> *rings, double dx, double dy ) const
{
  if ( !rings )
    return nullptr;

  QVector<QPolygonF> *translatedRings = new QVector<QPolygonF>;
  translatedRings->reserve( rings->size() );
  for ( auto it = rings->constBegin(); it != rings->constEnd(); ++it )
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
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != QgsSymbol::Fill )
      continue;

    QgsFillSymbolLayer *fillLayer = static_cast<QgsFillSymbolLayer *>( layer );

    if ( fillLayer )
      fillLayer->setAngle( angle );
  }
}


