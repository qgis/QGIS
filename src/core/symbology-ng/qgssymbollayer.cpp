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
#include "qgsproperty.h"
#include "qgsexpressioncontext.h"

#include <QSize>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>

const QgsPropertyDefinition QgsSymbolLayer::sPropertyNameMap {{ QgsSymbolLayer::PropertySize, "size" },
  { QgsSymbolLayer::PropertyAngle, "angle"},
  { QgsSymbolLayer::PropertyName, "name"},
  { QgsSymbolLayer::PropertyFillColor, "fillColor"},
  { QgsSymbolLayer::PropertyOutlineColor, "outlineColor"},
  { QgsSymbolLayer::PropertyOutlineWidth, "outlineWidth"},
  { QgsSymbolLayer::PropertyOutlineStyle, "outlineStyle"},
  { QgsSymbolLayer::PropertyOffset, "offset"},
  { QgsSymbolLayer::PropertyCharacter, "char"},
  { QgsSymbolLayer::PropertyWidth, "width"},
  { QgsSymbolLayer::PropertyHeight, "height"},
  { QgsSymbolLayer::PropertyFillStyle, "fillStyle"},
  { QgsSymbolLayer::PropertyJoinStyle, "joinStyle"},
  { QgsSymbolLayer::PropertySecondaryColor, "color2"},
  { QgsSymbolLayer::PropertyLineAngle, "lineAngle"},
  { QgsSymbolLayer::PropertyGradientType, "gradientType"},
  { QgsSymbolLayer::PropertyCoordinateMode, "gradientMode"},
  { QgsSymbolLayer::PropertyGradientSpread, "gradientSpread"},
  { QgsSymbolLayer::PropertyGradientReference1X, "gradientRef1X"},
  { QgsSymbolLayer::PropertyGradientReference1Y, "gradientRef1Y"},
  { QgsSymbolLayer::PropertyGradientReference2X, "gradientRef2X"},
  { QgsSymbolLayer::PropertyGradientReference2Y, "gradientRef2Y"},
  { QgsSymbolLayer::PropertyGradientReference1IsCentroid, "gradientRef1Centroid"},
  { QgsSymbolLayer::PropertyGradientReference2IsCentroid, "gradientRef2Centroid"},
  { QgsSymbolLayer::PropertyBlurRadius, "blurRadius"},
  { QgsSymbolLayer::PropertyLineDistance, "lineDistance"},
  { QgsSymbolLayer::PropertyShapeburstUseWholeShape, "shapeburstWholeShape"},
  { QgsSymbolLayer::PropertyShapeburstMaxDistance, "shapeburstMaxDist"},
  { QgsSymbolLayer::PropertyShapeburstIgnoreRings, "shapeburstIgnoreRings"},
  { QgsSymbolLayer::PropertyFile, "file"},
  { QgsSymbolLayer::PropertyDistanceX, "distanceX"},
  { QgsSymbolLayer::PropertyDistanceY, "distanceY"},
  { QgsSymbolLayer::PropertyDisplacementX, "displacementX"},
  { QgsSymbolLayer::PropertyDisplacementY, "displacementY"},
  { QgsSymbolLayer::PropertyAlpha, "alpha"},
  { QgsSymbolLayer::PropertyCustomDash, "customDash"},
  { QgsSymbolLayer::PropertyCapStyle, "capStyle"},
  { QgsSymbolLayer::PropertyPlacement, "placement"},
  { QgsSymbolLayer::PropertyInterval, "interval"},
  { QgsSymbolLayer::PropertyOffsetAlongLine, "offsetAlongLine"},
  { QgsSymbolLayer::PropertyHorizontalAnchor, "hAnchor"},
  { QgsSymbolLayer::PropertyVerticalAnchor, "vAnchor"},
  { QgsSymbolLayer::PropertyLayerEnabled, "enabled" },
  { QgsSymbolLayer::PropertyArrowWidth, "arrowWidth" },
  { QgsSymbolLayer::PropertyArrowStartWidth, "arrowStartWidth" },
  { QgsSymbolLayer::PropertyArrowHeadLength, "arrowHeadLength" },
  { QgsSymbolLayer::PropertyArrowHeadThickness, "arrowHeadThickness" },
  { QgsSymbolLayer::PropertyArrowHeadType, "arrowHeadType" },
  { QgsSymbolLayer::PropertyArrowType, "arrowType" },

};

void QgsSymbolLayer::setDataDefinedProperty( QgsSymbolLayer::Property key, QgsAbstractProperty* property )
{
  dataDefinedProperties().setProperty( key, property );
}

bool QgsSymbolLayer::writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift ) const
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

double QgsSymbolLayer::dxfOffset( const QgsDxfExport& e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( e );
  Q_UNUSED( context );
  return 0.0;
}

QColor QgsSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context );
  return color();
}

double QgsSymbolLayer::dxfAngle( QgsSymbolRenderContext &context ) const
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

QColor QgsSymbolLayer::dxfBrushColor( QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context );
  return color();
}

Qt::BrushStyle QgsSymbolLayer::dxfBrushStyle() const
{
  return Qt::NoBrush;
}

QgsPaintEffect *QgsSymbolLayer::paintEffect() const
{
  return mPaintEffect;
}

void QgsSymbolLayer::setPaintEffect( QgsPaintEffect *effect )
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
  mProperties.prepare( context.renderContext().expressionContext() );

  if ( !context.fields().isEmpty() )
  {
    //QgsFields is implicitly shared, so it's cheap to make a copy
    mFields = context.fields();
  }
}

QgsSymbolLayer::~QgsSymbolLayer()
{
  delete mPaintEffect;
}

bool QgsSymbolLayer::isCompatibleWithSymbol( QgsSymbol* symbol ) const
{
  if ( symbol->type() == QgsSymbol::Fill && mType == QgsSymbol::Line )
    return true;

  return symbol->type() == mType;
}

QSet<QString> QgsSymbolLayer::usedAttributes( const QgsRenderContext& context ) const
{
  QSet<QString> columns = mProperties.referencedFields( context.expressionContext() );
  return columns;
}

QgsAbstractProperty* propertyFromMap( const QgsStringMap &map, const QString &baseName )
{
  QString prefix;
  if ( !baseName.isEmpty() )
  {
    prefix.append( QStringLiteral( "%1_dd_" ).arg( baseName ) );
  }

  if ( !map.contains( QStringLiteral( "%1expression" ).arg( prefix ) ) )
  {
    //requires at least the expression value
    return nullptr;
  }

  bool active = ( map.value( QStringLiteral( "%1active" ).arg( prefix ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  QString expression = map.value( QStringLiteral( "%1expression" ).arg( prefix ) );
  bool useExpression = ( map.value( QStringLiteral( "%1useexpr" ).arg( prefix ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  QString field = map.value( QStringLiteral( "%1field" ).arg( prefix ), QString() );

  if ( useExpression )
    return new QgsExpressionBasedProperty( expression, active );
  else
    return new QgsFieldBasedProperty( field, active );
}

// property string to type upgrade map
static const QMap< QString, QgsSymbolLayer::Property > OLD_PROPS
{
  { "color" , QgsSymbolLayer::PropertyFillColor },
  { "arrow_width", QgsSymbolLayer::PropertyArrowWidth },
  { "arrow_start_width", QgsSymbolLayer::PropertyArrowStartWidth },
  { "head_length", QgsSymbolLayer::PropertyArrowHeadLength },
  { "head_thickness", QgsSymbolLayer::PropertyArrowHeadThickness },
  { "offset", QgsSymbolLayer::PropertyOffset },
  { "head_type", QgsSymbolLayer::PropertyArrowHeadType },
  { "arrow_type", QgsSymbolLayer::PropertyArrowType },
  { "width_field", QgsSymbolLayer::PropertyWidth },
  { "height_field", QgsSymbolLayer::PropertyHeight },
  { "rotation_field", QgsSymbolLayer::PropertyAngle },
  { "outline_width_field", QgsSymbolLayer::PropertyOutlineWidth },
  { "fill_color_field", QgsSymbolLayer::PropertyFillColor },
  { "outline_color_field", QgsSymbolLayer::PropertyOutlineColor },
  { "symbol_name_field", QgsSymbolLayer::PropertyName },
  { "outline_width", QgsSymbolLayer::PropertyOutlineWidth },
  { "outline_style", QgsSymbolLayer::PropertyOutlineStyle },
  { "join_style", QgsSymbolLayer::PropertyJoinStyle },
  { "fill_color", QgsSymbolLayer::PropertyFillColor },
  { "outline_color", QgsSymbolLayer::PropertyOutlineColor },
  { "width", QgsSymbolLayer::PropertyWidth },
  { "height", QgsSymbolLayer::PropertyHeight },
  { "symbol_name", QgsSymbolLayer::PropertyName },
  { "angle", QgsSymbolLayer::PropertyAngle },
  { "fill_style", QgsSymbolLayer::PropertyFillStyle },
  { "color_border", QgsSymbolLayer::PropertyOutlineColor },
  { "width_border", QgsSymbolLayer::PropertyOutlineWidth },
  { "border_color", QgsSymbolLayer::PropertyOutlineColor },
  { "border_style", QgsSymbolLayer::PropertyOutlineStyle },
  { "color2", QgsSymbolLayer::PropertySecondaryColor },
  { "gradient_type", QgsSymbolLayer::PropertyGradientType },
  { "coordinate_mode", QgsSymbolLayer::PropertyCoordinateMode },
  { "spread", QgsSymbolLayer::PropertyGradientSpread },
  { "reference1_x", QgsSymbolLayer::PropertyGradientReference1X },
  { "reference1_y", QgsSymbolLayer::PropertyGradientReference1Y },
  { "reference2_x", QgsSymbolLayer::PropertyGradientReference2X },
  { "reference2_y", QgsSymbolLayer::PropertyGradientReference2Y },
  { "reference1_iscentroid", QgsSymbolLayer::PropertyGradientReference1IsCentroid },
  { "reference2_iscentroid", QgsSymbolLayer::PropertyGradientReference2IsCentroid },
  { "blur_radius", QgsSymbolLayer::PropertyBlurRadius },
  { "use_whole_shape", QgsSymbolLayer::PropertyShapeburstUseWholeShape },
  { "max_distance", QgsSymbolLayer::PropertyShapeburstMaxDistance },
  { "ignore_rings", QgsSymbolLayer::PropertyShapeburstIgnoreRings },
  { "svgFillColor", QgsSymbolLayer::PropertyFillColor },
  { "svgOutlineColor", QgsSymbolLayer::PropertyOutlineColor },
  { "svgOutlineWidth", QgsSymbolLayer::PropertyOutlineWidth },
  { "svgFile", QgsSymbolLayer::PropertyFile },
  { "lineangle", QgsSymbolLayer::PropertyLineAngle },
  { "distance", QgsSymbolLayer::PropertyLineDistance },
  { "distance_x", QgsSymbolLayer::PropertyDistanceX },
  { "distance_y", QgsSymbolLayer::PropertyDistanceY },
  { "displacement_x", QgsSymbolLayer::PropertyDisplacementX },
  { "displacement_y", QgsSymbolLayer::PropertyDisplacementY },
  { "file", QgsSymbolLayer::PropertyFile },
  { "alpha", QgsSymbolLayer::PropertyAlpha },
  { "customdash", QgsSymbolLayer::PropertyCustomDash },
  { "line_style", QgsSymbolLayer::PropertyOutlineStyle },
  { "joinstyle", QgsSymbolLayer::PropertyJoinStyle },
  { "capstyle", QgsSymbolLayer::PropertyCapStyle },
  { "placement", QgsSymbolLayer::PropertyPlacement },
  { "interval", QgsSymbolLayer::PropertyInterval },
  { "offset_along_line", QgsSymbolLayer::PropertyOffsetAlongLine },
  { "name", QgsSymbolLayer::PropertyName },
  { "size", QgsSymbolLayer::PropertySize },
  { "fill", QgsSymbolLayer::PropertyFillColor },
  { "outline", QgsSymbolLayer::PropertyOutlineColor },
  { "char", QgsSymbolLayer::PropertyCharacter },
  { "enabled", QgsSymbolLayer::PropertyLayerEnabled },
  { "rotation", QgsSymbolLayer::PropertyAngle },
  { "horizontal_anchor_point", QgsSymbolLayer::PropertyHorizontalAnchor },
  { "vertical_anchor_point", QgsSymbolLayer::PropertyVerticalAnchor },
};

void QgsSymbolLayer::restoreOldDataDefinedProperties( const QgsStringMap &stringMap )
{
  QgsStringMap::const_iterator propIt = stringMap.constBegin();
  for ( ; propIt != stringMap.constEnd(); ++propIt )
  {
    QScopedPointer< QgsAbstractProperty > prop;
    QString propertyName;

    if ( propIt.key().endsWith( QLatin1String( "_dd_expression" ) ) )
    {
      //found a data defined property

      //get data defined property name by stripping "_dd_expression" from property key
      propertyName = propIt.key().left( propIt.key().length() - 14 );

      prop.reset( propertyFromMap( stringMap, propertyName ) );
    }
    else if ( propIt.key().endsWith( QLatin1String( "_expression" ) ) )
    {
      //old style data defined property, upgrade

      //get data defined property name by stripping "_expression" from property key
      propertyName = propIt.key().left( propIt.key().length() - 11 );

      prop.reset( new QgsExpressionBasedProperty( propIt.value() ) );
    }

    if ( !prop || !OLD_PROPS.contains( propertyName ) )
      continue;

    QgsSymbolLayer::Property key = static_cast< QgsSymbolLayer::Property >( OLD_PROPS.value( propertyName ) );

    if ( type() == QgsSymbol::Line )
    {
      //these keys had different meaning for line symbol layers
      if ( propertyName == "width" )
        key = QgsSymbolLayer::PropertyOutlineWidth;
      else if ( propertyName == "color" )
        key = QgsSymbolLayer::PropertyOutlineColor;
    }

    setDataDefinedProperty( key, prop.take() );
  }
}

void QgsSymbolLayer::copyDataDefinedProperties( QgsSymbolLayer* destLayer ) const
{
  if ( !destLayer )
    return;

  destLayer->setDataDefinedProperties( mProperties );
}

void QgsSymbolLayer::copyPaintEffect( QgsSymbolLayer *destLayer ) const
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

  if ( mProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    QVariant exprVal = mProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      QPointF offset = QgsSymbolLayerUtils::decodePoint( exprVal.toString() );
      offsetX = offset.x();
      offsetY = offset.y();
    }
  }

  offsetX = context.renderContext().convertToPainterUnits( offsetX, mOffsetUnit, mOffsetMapUnitScale );
  offsetY = context.renderContext().convertToPainterUnits( offsetY, mOffsetUnit, mOffsetMapUnitScale );

  HorizontalAnchorPoint horizontalAnchorPoint = mHorizontalAnchorPoint;
  VerticalAnchorPoint verticalAnchorPoint = mVerticalAnchorPoint;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyHorizontalAnchor ) )
  {
    QVariant exprVal = mProperties.value( QgsSymbolLayer::PropertyHorizontalAnchor, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      horizontalAnchorPoint = decodeHorizontalAnchorPoint( exprVal.toString() );
    }
  }
  if ( mProperties.isActive( QgsSymbolLayer::PropertyVerticalAnchor ) )
  {
    QVariant exprVal = mProperties.value( QgsSymbolLayer::PropertyVerticalAnchor, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      verticalAnchorPoint = decodeVerticalAnchorPoint( exprVal.toString() );
    }
  }

  //correct horizontal position according to anchor point
  if ( horizontalAnchorPoint == HCenter && verticalAnchorPoint == VCenter )
  {
    return;
  }

  double anchorPointCorrectionX = context.renderContext().convertToPainterUnits( width, widthUnit, widthMapUnitScale ) / 2.0;
  double anchorPointCorrectionY = context.renderContext().convertToPainterUnits( height, heightUnit, heightMapUnitScale ) / 2.0;
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

void QgsMarkerSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
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

double QgsLineSymbolLayer::dxfWidth( const QgsDxfExport& e, QgsSymbolRenderContext &context ) const
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

void QgsMarkerSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PointSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QLatin1String( "" ) ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QLatin1String( "" ) ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QLatin1String( "" ) ) );

  writeSldMarker( doc, symbolizerElem, props );
}


