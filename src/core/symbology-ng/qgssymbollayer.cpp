/***************************************************************************
 qgssymbollayer.cpp
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

#include "qgssymbollayer.h"
#include "qgsclipper.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgsdxfexport.h"
#include "qgsgeometrysimplifier.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgspainteffectregistry.h"
#include "qgsdatadefined.h"
#include "qgsexpressioncontext.h"

#include <QSize>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>

const QString QgsSymbolLayer::EXPR_SIZE( QStringLiteral( "size" ) );
const QString QgsSymbolLayer::EXPR_ANGLE( QStringLiteral( "angle" ) );
const QString QgsSymbolLayer::EXPR_NAME( QStringLiteral( "name" ) );
const QString QgsSymbolLayer::EXPR_COLOR( QStringLiteral( "color" ) );
const QString QgsSymbolLayer::EXPR_COLOR_BORDER( QStringLiteral( "color_border" ) );
const QString QgsSymbolLayer::EXPR_OUTLINE_WIDTH( QStringLiteral( "outline_width" ) );
const QString QgsSymbolLayer::EXPR_OUTLINE_STYLE( QStringLiteral( "outline_style" ) );
const QString QgsSymbolLayer::EXPR_FILL( QStringLiteral( "fill" ) );
const QString QgsSymbolLayer::EXPR_OUTLINE( QStringLiteral( "outline" ) );
const QString QgsSymbolLayer::EXPR_OFFSET( QStringLiteral( "offset" ) );
const QString QgsSymbolLayer::EXPR_CHAR( QStringLiteral( "char" ) );
const QString QgsSymbolLayer::EXPR_FILL_COLOR( QStringLiteral( "fill_color" ) );
const QString QgsSymbolLayer::EXPR_OUTLINE_COLOR( QStringLiteral( "outline_color" ) );
const QString QgsSymbolLayer::EXPR_WIDTH( QStringLiteral( "width" ) );
const QString QgsSymbolLayer::EXPR_HEIGHT( QStringLiteral( "height" ) );
const QString QgsSymbolLayer::EXPR_SYMBOL_NAME( QStringLiteral( "symbol_name" ) );
const QString QgsSymbolLayer::EXPR_ROTATION( QStringLiteral( "rotation" ) );
const QString QgsSymbolLayer::EXPR_FILL_STYLE( QStringLiteral( "fill_style" ) );
const QString QgsSymbolLayer::EXPR_WIDTH_BORDER( QStringLiteral( "width_border" ) );
const QString QgsSymbolLayer::EXPR_BORDER_STYLE( QStringLiteral( "border_style" ) );
const QString QgsSymbolLayer::EXPR_JOIN_STYLE( QStringLiteral( "join_style" ) );
const QString QgsSymbolLayer::EXPR_BORDER_COLOR( QStringLiteral( "border_color" ) );
const QString QgsSymbolLayer::EXPR_COLOR2( QStringLiteral( "color2" ) );
const QString QgsSymbolLayer::EXPR_LINEANGLE( QStringLiteral( "lineangle" ) );
const QString QgsSymbolLayer::EXPR_GRADIENT_TYPE( QStringLiteral( "gradient_type" ) );
const QString QgsSymbolLayer::EXPR_COORDINATE_MODE( QStringLiteral( "coordinate_mode" ) );
const QString QgsSymbolLayer::EXPR_SPREAD( QStringLiteral( "spread" ) );
const QString QgsSymbolLayer::EXPR_REFERENCE1_X( QStringLiteral( "reference1_x" ) );
const QString QgsSymbolLayer::EXPR_REFERENCE1_Y( QStringLiteral( "reference1_y" ) );
const QString QgsSymbolLayer::EXPR_REFERENCE2_X( QStringLiteral( "reference2_x" ) );
const QString QgsSymbolLayer::EXPR_REFERENCE2_Y( QStringLiteral( "reference2_y" ) );
const QString QgsSymbolLayer::EXPR_REFERENCE1_ISCENTROID( QStringLiteral( "reference1_iscentroid" ) );
const QString QgsSymbolLayer::EXPR_REFERENCE2_ISCENTROID( QStringLiteral( "reference2_iscentroid" ) );
const QString QgsSymbolLayer::EXPR_BLUR_RADIUS( QStringLiteral( "blur_radius" ) );
const QString QgsSymbolLayer::EXPR_DISTANCE( QStringLiteral( "distance" ) );
const QString QgsSymbolLayer::EXPR_USE_WHOLE_SHAPE( QStringLiteral( "use_whole_shape" ) );
const QString QgsSymbolLayer::EXPR_MAX_DISTANCE( QStringLiteral( "max_distance" ) );
const QString QgsSymbolLayer::EXPR_IGNORE_RINGS( QStringLiteral( "ignore_rings" ) );
const QString QgsSymbolLayer::EXPR_SVG_FILE( QStringLiteral( "svgFile" ) );
const QString QgsSymbolLayer::EXPR_SVG_FILL_COLOR( QStringLiteral( "svgFillColor" ) );
const QString QgsSymbolLayer::EXPR_SVG_OUTLINE_COLOR( QStringLiteral( "svgOutlineColor" ) );
const QString QgsSymbolLayer::EXPR_SVG_OUTLINE_WIDTH( QStringLiteral( "svgOutlineWidth" ) );
const QString QgsSymbolLayer::EXPR_LINEWIDTH( QStringLiteral( "linewidth" ) );
const QString QgsSymbolLayer::EXPR_DISTANCE_X( QStringLiteral( "distance_x" ) );
const QString QgsSymbolLayer::EXPR_DISTANCE_Y( QStringLiteral( "distance_y" ) );
const QString QgsSymbolLayer::EXPR_DISPLACEMENT_X( QStringLiteral( "displacement_x" ) );
const QString QgsSymbolLayer::EXPR_DISPLACEMENT_Y( QStringLiteral( "displacement_y" ) );
const QString QgsSymbolLayer::EXPR_FILE( QStringLiteral( "file" ) );
const QString QgsSymbolLayer::EXPR_ALPHA( QStringLiteral( "alpha" ) );
const QString QgsSymbolLayer::EXPR_CUSTOMDASH( QStringLiteral( "customdash" ) );
const QString QgsSymbolLayer::EXPR_LINE_STYLE( QStringLiteral( "line_style" ) );
const QString QgsSymbolLayer::EXPR_JOINSTYLE( QStringLiteral( "joinstyle" ) );
const QString QgsSymbolLayer::EXPR_CAPSTYLE( QStringLiteral( "capstyle" ) );
const QString QgsSymbolLayer::EXPR_PLACEMENT( QStringLiteral( "placement" ) );
const QString QgsSymbolLayer::EXPR_INTERVAL( QStringLiteral( "interval" ) );
const QString QgsSymbolLayer::EXPR_OFFSET_ALONG_LINE( QStringLiteral( "offset_along_line" ) );
const QString QgsSymbolLayer::EXPR_HORIZONTAL_ANCHOR_POINT( QStringLiteral( "horizontal_anchor_point" ) );
const QString QgsSymbolLayer::EXPR_VERTICAL_ANCHOR_POINT( QStringLiteral( "vertical_anchor_point" ) );
const QString QgsSymbolLayer::EXPR_LAYER_ENABLED( QStringLiteral( "enabled" ) );

QgsDataDefined* QgsSymbolLayer::getDataDefinedProperty( const QString& property ) const
{
  if ( mDataDefinedProperties.isEmpty() )
    return nullptr;

  QMap< QString, QgsDataDefined* >::const_iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.constEnd() )
  {
    return it.value();
  }
  return nullptr;
}

void QgsSymbolLayer::setDataDefinedProperty( const QString& property, QgsDataDefined* dataDefined )
{
  removeDataDefinedProperty( property );
  mDataDefinedProperties.insert( property, dataDefined );
}

void QgsSymbolLayer::removeDataDefinedProperty( const QString& property )
{
  QMap< QString, QgsDataDefined* >::iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.end() )
  {
    delete( it.value() );
    mDataDefinedProperties.erase( it );
  }
}

void QgsSymbolLayer::removeDataDefinedProperties()
{
  qDeleteAll( mDataDefinedProperties );
  mDataDefinedProperties.clear();
}

bool QgsSymbolLayer::hasDataDefinedProperties() const
{
  if ( mDataDefinedProperties.isEmpty() )
    return false;

  QMap< QString, QgsDataDefined* >::const_iterator it = mDataDefinedProperties.constBegin();
  for ( ; it != mDataDefinedProperties.constEnd(); ++it )
  {
    if ( hasDataDefinedProperty( it.key() ) )
      return true;
  }

  return false;
}

bool QgsSymbolLayer::hasDataDefinedProperty( const QString& property ) const
{
  if ( mDataDefinedProperties.isEmpty() )
    return false;

  QgsDataDefined* dd = getDataDefinedProperty( property );
  return dd && dd->isActive();
}

QVariant QgsSymbolLayer::evaluateDataDefinedProperty( const QString& property, const QgsSymbolRenderContext& context, const QVariant& defaultVal, bool* ok ) const
{
  if ( ok )
    *ok = false;

  QgsDataDefined* dd = getDataDefinedProperty( property );
  if ( !dd || !dd->isActive() )
    return defaultVal;

  if ( dd->useExpression() )
  {
    if ( dd->expression() )
    {
      QVariant result = dd->expression()->evaluate( &context.renderContext().expressionContext() );
      if ( result.isValid() )
      {
        if ( ok )
          *ok = true;
        return result;
      }
      else
        return defaultVal;
    }
    else
    {
      return defaultVal;
    }
  }
  else if ( context.feature() && !dd->field().isEmpty() && !mFields.isEmpty() )
  {
    int attributeIndex = mFields.lookupField( dd->field() );
    if ( attributeIndex >= 0 )
    {
      if ( ok )
        *ok = true;
      return context.feature()->attribute( attributeIndex );
    }
  }
  return defaultVal;
}

bool QgsSymbolLayer::writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, QgsSymbolRenderContext& context, QPointF shift ) const
{
  Q_UNUSED( e );
  Q_UNUSED( mmMapUnitScaleFactor );
  Q_UNUSED( layerName );
  Q_UNUSED( context );
  Q_UNUSED( shift );
  return false;
}

double QgsSymbolLayer::dxfWidth( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const
{
  Q_UNUSED( e );
  Q_UNUSED( context );
  return 1.0;
}

double QgsSymbolLayer::dxfOffset( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const
{
  Q_UNUSED( e );
  Q_UNUSED( context );
  return 0.0;
}

QColor QgsSymbolLayer::dxfColor( QgsSymbolRenderContext& context ) const
{
  Q_UNUSED( context );
  return color();
}

double QgsSymbolLayer::dxfAngle( QgsSymbolRenderContext& context ) const
{
  Q_UNUSED( context );
  return 0.0;
}

QVector<qreal> QgsSymbolLayer::dxfCustomDashPattern( QgsUnitTypes::RenderUnit& unit ) const
{
  Q_UNUSED( unit );
  return QVector<qreal>();
}

Qt::PenStyle QgsSymbolLayer::dxfPenStyle() const
{
  return Qt::SolidLine;
}

QColor QgsSymbolLayer::dxfBrushColor( QgsSymbolRenderContext& context ) const
{
  Q_UNUSED( context );
  return color();
}

Qt::BrushStyle QgsSymbolLayer::dxfBrushStyle() const
{
  return Qt::NoBrush;
}

QgsPaintEffect* QgsSymbolLayer::paintEffect() const
{
  return mPaintEffect;
}

void QgsSymbolLayer::setPaintEffect( QgsPaintEffect* effect )
{
  delete mPaintEffect;
  mPaintEffect = effect;
}

QgsSymbolLayer::QgsSymbolLayer( QgsSymbol::SymbolType type, bool locked )
  : mType( type )
  , mEnabled( true )
  , mLocked( locked )
  , mRenderingPass( 0 )
  , mPaintEffect( nullptr )
{
  mPaintEffect = QgsPaintEffectRegistry::defaultStack();
  mPaintEffect->setEnabled( false );
}

void QgsSymbolLayer::prepareExpressions( const QgsSymbolRenderContext& context )
{
  QMap< QString, QgsDataDefined* >::const_iterator it = mDataDefinedProperties.constBegin();
  for ( ; it != mDataDefinedProperties.constEnd(); ++it )
  {
    if ( it.value() )
    {
      it.value()->prepareExpression( context.renderContext().expressionContext() );
    }
  }

  if ( !context.fields().isEmpty() )
  {
    //QgsFields is implicitly shared, so it's cheap to make a copy
    mFields = context.fields();
  }
}

QgsSymbolLayer::~QgsSymbolLayer()
{
  removeDataDefinedProperties();
  delete mPaintEffect;
}

bool QgsSymbolLayer::isCompatibleWithSymbol( QgsSymbol* symbol ) const
{
  if ( symbol->type() == QgsSymbol::Fill && mType == QgsSymbol::Line )
    return true;

  return symbol->type() == mType;
}

QSet<QString> QgsSymbolLayer::usedAttributes() const
{
  QSet<QString> columns;

  QMap< QString, QgsDataDefined* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() && ddIt.value()->isActive() )
    {
      columns.unite( ddIt.value()->referencedColumns() );
    }
  }

  return columns;
}

void QgsSymbolLayer::saveDataDefinedProperties( QgsStringMap& stringMap ) const
{
  QMap< QString, QgsDataDefined* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() )
    {
      stringMap.unite( ddIt.value()->toMap( ddIt.key() ) );
    }
  }
}

void QgsSymbolLayer::restoreDataDefinedProperties( const QgsStringMap& stringMap )
{
  QgsStringMap::const_iterator propIt = stringMap.constBegin();
  for ( ; propIt != stringMap.constEnd(); ++propIt )
  {
    if ( propIt.key().endsWith( QLatin1String( "_dd_expression" ) ) )
    {
      //found a data defined property

      //get data defined property name by stripping "_dd_expression" from property key
      QString propertyName = propIt.key().left( propIt.key().length() - 14 );

      QgsDataDefined* dd = QgsDataDefined::fromMap( stringMap, propertyName );
      if ( dd )
        setDataDefinedProperty( propertyName, dd );
    }
    else if ( propIt.key().endsWith( QLatin1String( "_expression" ) ) )
    {
      //old style data defined property, upgrade

      //get data defined property name by stripping "_expression" from property key
      QString propertyName = propIt.key().left( propIt.key().length() - 11 );

      setDataDefinedProperty( propertyName, new QgsDataDefined( propIt.value() ) );
    }
  }
}

void QgsSymbolLayer::copyDataDefinedProperties( QgsSymbolLayer* destLayer ) const
{
  if ( !destLayer )
    return;

  destLayer->removeDataDefinedProperties();

  QMap< QString, QgsDataDefined* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() )
    {
      destLayer->setDataDefinedProperty( ddIt.key(), new QgsDataDefined( *( ddIt.value() ) ) );
    }
  }
}

void QgsSymbolLayer::copyPaintEffect( QgsSymbolLayer* destLayer ) const
{
  if ( !destLayer || !mPaintEffect )
    return;

  destLayer->setPaintEffect( mPaintEffect->clone() );
}

QgsMarkerSymbolLayer::QgsMarkerSymbolLayer( bool locked )
  : QgsSymbolLayer( QgsSymbol::Marker, locked )
  , mAngle( 0 )
  , mLineAngle( 0 )
  , mSize( 2.0 )
  , mSizeUnit( QgsUnitTypes::RenderMillimeters )
  , mOffsetUnit( QgsUnitTypes::RenderMillimeters )
  , mScaleMethod( QgsSymbol::ScaleDiameter )
  , mHorizontalAnchorPoint( HCenter )
  , mVerticalAnchorPoint( VCenter )
{

}

QgsLineSymbolLayer::QgsLineSymbolLayer( bool locked )
  : QgsSymbolLayer( QgsSymbol::Line, locked )
  , mWidth( 0 )
  , mWidthUnit( QgsUnitTypes::RenderMillimeters )
  , mOffset( 0 )
  , mOffsetUnit( QgsUnitTypes::RenderMillimeters )
{
}

QgsFillSymbolLayer::QgsFillSymbolLayer( bool locked )
  : QgsSymbolLayer( QgsSymbol::Fill, locked )
  , mAngle( 0.0 )
{
}

void QgsMarkerSymbolLayer::startRender( QgsSymbolRenderContext& context )
{
  Q_UNUSED( context );
}

void QgsMarkerSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext& context, QSize size )
{
  startRender( context );
  QgsPaintEffect* effect = paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext(), effect );
    renderPoint( QPointF( size.width() / 2, size.height() / 2 ), context );
  }
  else
  {
    renderPoint( QPointF( size.width() / 2, size.height() / 2 ), context );
  }
  stopRender( context );
}

void QgsMarkerSymbolLayer::markerOffset( QgsSymbolRenderContext& context, double& offsetX, double& offsetY ) const
{
  markerOffset( context, mSize, mSize, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayer::markerOffset( QgsSymbolRenderContext& context, double width, double height, double& offsetX, double& offsetY ) const
{
  markerOffset( context, width, height, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayer::markerOffset( QgsSymbolRenderContext& context, double width, double height,
    QgsUnitTypes::RenderUnit widthUnit, QgsUnitTypes::RenderUnit heightUnit,
    double& offsetX, double& offsetY, const QgsMapUnitScale& widthMapUnitScale, const QgsMapUnitScale& heightMapUnitScale ) const
{
  offsetX = mOffset.x();
  offsetY = mOffset.y();

  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    QPointF offset = QgsSymbolLayerUtils::decodePoint( evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET, context ).toString() );
    offsetX = offset.x();
    offsetY = offset.y();
  }

  offsetX = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), offsetX, mOffsetUnit, mOffsetMapUnitScale );
  offsetY = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), offsetY, mOffsetUnit, mOffsetMapUnitScale );

  HorizontalAnchorPoint horizontalAnchorPoint = mHorizontalAnchorPoint;
  VerticalAnchorPoint verticalAnchorPoint = mVerticalAnchorPoint;
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_HORIZONTAL_ANCHOR_POINT ) )
  {
    horizontalAnchorPoint = decodeHorizontalAnchorPoint( evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_HORIZONTAL_ANCHOR_POINT , context ).toString() );
  }
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_VERTICAL_ANCHOR_POINT ) )
  {
    verticalAnchorPoint = decodeVerticalAnchorPoint( evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_VERTICAL_ANCHOR_POINT, context ).toString() );
  }

  //correct horizontal position according to anchor point
  if ( horizontalAnchorPoint == HCenter && verticalAnchorPoint == VCenter )
  {
    return;
  }

  double anchorPointCorrectionX = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), width, widthUnit, widthMapUnitScale ) / 2.0;
  double anchorPointCorrectionY = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), height, heightUnit, heightMapUnitScale ) / 2.0;
  if ( horizontalAnchorPoint == Left )
  {
    offsetX += anchorPointCorrectionX;
  }
  else if ( horizontalAnchorPoint == Right )
  {
    offsetX -= anchorPointCorrectionX;
  }

  //correct vertical position according to anchor point
  if ( verticalAnchorPoint == Top )
  {
    offsetY += anchorPointCorrectionY;
  }
  else if ( verticalAnchorPoint == Bottom )
  {
    offsetY -= anchorPointCorrectionY;
  }
}

QPointF QgsMarkerSymbolLayer::_rotatedOffset( QPointF offset, double angle )
{
  angle = DEG2RAD( angle );
  double c = cos( angle ), s = sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

QgsMarkerSymbolLayer::HorizontalAnchorPoint QgsMarkerSymbolLayer::decodeHorizontalAnchorPoint( const QString& str )
{
  if ( str.compare( QLatin1String( "left" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayer::Left;
  }
  else if ( str.compare( QLatin1String( "right" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayer::Right;
  }
  else
  {
    return QgsMarkerSymbolLayer::HCenter;
  }
}

QgsMarkerSymbolLayer::VerticalAnchorPoint QgsMarkerSymbolLayer::decodeVerticalAnchorPoint( const QString& str )
{
  if ( str.compare( QLatin1String( "top" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayer::Top;
  }
  else if ( str.compare( QLatin1String( "bottom" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayer::Bottom;
  }
  else
  {
    return QgsMarkerSymbolLayer::VCenter;
  }
}

void QgsMarkerSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mSizeUnit = unit;
  mOffsetUnit = unit;
}

QgsUnitTypes::RenderUnit QgsMarkerSymbolLayer::outputUnit() const
{
  if ( mOffsetUnit != mSizeUnit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return mOffsetUnit;
}

void QgsMarkerSymbolLayer::setMapUnitScale( const QgsMapUnitScale& scale )
{
  mSizeMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsMarkerSymbolLayer::mapUnitScale() const
{
  if ( mSizeMapUnitScale == mOffsetMapUnitScale )
  {
    return mSizeMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsLineSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mWidthUnit = unit;
}

QgsUnitTypes::RenderUnit QgsLineSymbolLayer::outputUnit() const
{
  return mWidthUnit;
}

void QgsLineSymbolLayer::setMapUnitScale( const QgsMapUnitScale& scale )
{
  mWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsLineSymbolLayer::mapUnitScale() const
{
  return mWidthMapUnitScale;
}


void QgsLineSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext& context, QSize size )
{
  QPolygonF points;
  // we're adding 0.5 to get rid of blurred preview:
  // drawing antialiased lines of width 1 at (x,0)-(x,100) creates 2px line
  points << QPointF( 0, int( size.height() / 2 ) + 0.5 ) << QPointF( size.width(), int( size.height() / 2 ) + 0.5 );

  startRender( context );
  QgsPaintEffect* effect = paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext(), effect );
    renderPolyline( points, context );
  }
  else
  {
    renderPolyline( points, context );
  }
  stopRender( context );
}

void QgsLineSymbolLayer::renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolRenderContext& context )
{
  renderPolyline( points, context );
  if ( rings )
  {
    Q_FOREACH ( const QPolygonF& ring, *rings )
      renderPolyline( ring, context );
  }
}

double QgsLineSymbolLayer::dxfWidth( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const
{
  Q_UNUSED( context );
  return width() * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), widthUnit(), e.mapUnits() );
}


void QgsFillSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext& context, QSize size )
{
  QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width(), size.height() ) );
  startRender( context );
  QgsPaintEffect* effect = paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext(), effect );
    renderPolygon( poly, nullptr, context );
  }
  else
  {
    renderPolygon( poly, nullptr, context );
  }
  stopRender( context );
}

void QgsFillSymbolLayer::_renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolRenderContext& context )
{
  if ( !p )
  {
    return;
  }

  // Disable 'Antialiasing' if the geometry was generalized in the current RenderContext (We known that it must have least #5 points).
  if ( points.size() <= 5 &&
       ( context.renderContext().vectorSimplifyMethod().simplifyHints() & QgsVectorSimplifyMethod::AntialiasingSimplification ) &&
       QgsAbstractGeometrySimplifier::isGeneralizableByDeviceBoundingBox( points, context.renderContext().vectorSimplifyMethod().threshold() ) &&
       ( p->renderHints() & QPainter::Antialiasing ) )
  {
    p->setRenderHint( QPainter::Antialiasing, false );
    p->drawRect( points.boundingRect() );
    p->setRenderHint( QPainter::Antialiasing, true );
    return;
  }

  // polygons outlines are sometimes rendered wrongly with drawPolygon, when
  // clipped (see #13343), so use drawPath instead.
  if ( !rings && p->pen().style() == Qt::NoPen )
  {
    // simple polygon without holes
    p->drawPolygon( points );
  }
  else
  {
    // polygon with holes must be drawn using painter path
    QPainterPath path;
    QPolygonF outerRing = points;
    path.addPolygon( outerRing );

    if ( rings )
    {
      QList<QPolygonF>::const_iterator it = rings->constBegin();
      for ( ; it != rings->constEnd(); ++it )
      {
        QPolygonF ring = *it;
        path.addPolygon( ring );
      }
    }

    p->drawPath( path );
  }
}

void QgsMarkerSymbolLayer::toSld( QDomDocument& doc, QDomElement& element, const QgsStringMap& props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PointSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QLatin1String( "" ) ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QLatin1String( "" ) ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QLatin1String( "" ) ) );

  writeSldMarker( doc, symbolizerElem, props );
}


