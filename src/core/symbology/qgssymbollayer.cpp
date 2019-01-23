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
#include "qgssymbollayerutils.h"

#include <QSize>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>

QgsPropertiesDefinition QgsSymbolLayer::sPropertyDefinitions;

void QgsSymbolLayer::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  QString origin = QStringLiteral( "symbol" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsSymbolLayer::PropertySize, QgsPropertyDefinition( "size", QObject::tr( "Symbol size" ), QgsPropertyDefinition::Size, origin ) },
    { QgsSymbolLayer::PropertyAngle, QgsPropertyDefinition( "angle", QObject::tr( "Rotation angle" ), QgsPropertyDefinition::Rotation, origin ) },
    { QgsSymbolLayer::PropertyName, QgsPropertyDefinition( "name", QObject::tr( "Symbol name" ), QgsPropertyDefinition::String, origin ) },
    { QgsSymbolLayer::PropertyFillColor, QgsPropertyDefinition( "fillColor", QObject::tr( "Symbol fill color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { QgsSymbolLayer::PropertyStrokeColor, QgsPropertyDefinition( "outlineColor", QObject::tr( "Symbol stroke color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { QgsSymbolLayer::PropertyStrokeWidth, QgsPropertyDefinition( "outlineWidth", QObject::tr( "Symbol stroke width" ), QgsPropertyDefinition::StrokeWidth, origin ) },
    { QgsSymbolLayer::PropertyStrokeStyle, QgsPropertyDefinition( "outlineStyle", QObject::tr( "Symbol stroke style" ), QgsPropertyDefinition::LineStyle, origin )},
    { QgsSymbolLayer::PropertyOffset, QgsPropertyDefinition( "offset", QObject::tr( "Symbol offset" ), QgsPropertyDefinition::Offset, origin )},
    { QgsSymbolLayer::PropertyCharacter, QgsPropertyDefinition( "char", QObject::tr( "Marker character(s)" ), QgsPropertyDefinition::String, origin )},
    { QgsSymbolLayer::PropertyWidth, QgsPropertyDefinition( "width", QObject::tr( "Symbol width" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyHeight, QgsPropertyDefinition( "height", QObject::tr( "Symbol height" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyPreserveAspectRatio, QgsPropertyDefinition( "preserveAspectRatio", QObject::tr( "Preserve aspect ratio between width and height" ), QgsPropertyDefinition::Boolean, origin )},
    { QgsSymbolLayer::PropertyFillStyle, QgsPropertyDefinition( "fillStyle", QObject::tr( "Symbol fill style" ), QgsPropertyDefinition::FillStyle, origin )},
    { QgsSymbolLayer::PropertyJoinStyle, QgsPropertyDefinition( "joinStyle", QObject::tr( "Outline join style" ), QgsPropertyDefinition::PenJoinStyle, origin )},
    { QgsSymbolLayer::PropertySecondaryColor, QgsPropertyDefinition( "color2", QObject::tr( "Secondary fill color" ), QgsPropertyDefinition::ColorWithAlpha, origin )},
    { QgsSymbolLayer::PropertyLineAngle, QgsPropertyDefinition( "lineAngle", QObject::tr( "Angle for line fills" ), QgsPropertyDefinition::Rotation, origin )},
    { QgsSymbolLayer::PropertyGradientType, QgsPropertyDefinition( "gradientType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Gradient type" ),  QObject::tr( "string " ) + QLatin1String( "[<b>linear</b>|<b>radial</b>|<b>conical</b>]" ), origin )},
    { QgsSymbolLayer::PropertyCoordinateMode, QgsPropertyDefinition( "gradientMode", QgsPropertyDefinition::DataTypeString, QObject::tr( "Gradient mode" ), QObject::tr( "string " ) + QLatin1String( "[<b>feature</b>|<b>viewport</b>]" ), origin )},
    { QgsSymbolLayer::PropertyGradientSpread, QgsPropertyDefinition( "gradientSpread", QgsPropertyDefinition::DataTypeString, QObject::tr( "Gradient spread" ), QObject::tr( "string " ) + QLatin1String( "[<b>pad</b>|<b>repeat</b>|<b>reflect</b>]" ), origin )},
    { QgsSymbolLayer::PropertyGradientReference1X, QgsPropertyDefinition( "gradientRef1X", QObject::tr( "Reference point 1 (X)" ), QgsPropertyDefinition::Double0To1, origin )},
    { QgsSymbolLayer::PropertyGradientReference1Y, QgsPropertyDefinition( "gradientRef1Y", QObject::tr( "Reference point 1 (Y)" ), QgsPropertyDefinition::Double0To1, origin )},
    { QgsSymbolLayer::PropertyGradientReference2X, QgsPropertyDefinition( "gradientRef2X", QObject::tr( "Reference point 2 (X)" ), QgsPropertyDefinition::Double0To1, origin )},
    { QgsSymbolLayer::PropertyGradientReference2Y, QgsPropertyDefinition( "gradientRef2Y", QObject::tr( "Reference point 2 (Y)" ), QgsPropertyDefinition::Double0To1, origin )},
    { QgsSymbolLayer::PropertyGradientReference1IsCentroid, QgsPropertyDefinition( "gradientRef1Centroid", QObject::tr( "Reference point 1 follows feature centroid" ), QgsPropertyDefinition::Boolean, origin )},
    { QgsSymbolLayer::PropertyGradientReference2IsCentroid, QgsPropertyDefinition( "gradientRef2Centroid", QObject::tr( "Reference point 2 follows feature centroid" ), QgsPropertyDefinition::Boolean, origin )},
    { QgsSymbolLayer::PropertyBlurRadius, QgsPropertyDefinition( "blurRadius", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Blur radius" ), QObject::tr( "Integer between 0 and 18" ), origin )},
    { QgsSymbolLayer::PropertyLineDistance, QgsPropertyDefinition( "lineDistance", QObject::tr( "Distance between lines" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyShapeburstUseWholeShape, QgsPropertyDefinition( "shapeburstWholeShape", QObject::tr( "Shade whole shape" ), QgsPropertyDefinition::Boolean, origin )},
    { QgsSymbolLayer::PropertyShapeburstMaxDistance, QgsPropertyDefinition( "shapeburstMaxDist", QObject::tr( "Maximum distance for shapeburst fill" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyShapeburstIgnoreRings, QgsPropertyDefinition( "shapeburstIgnoreRings", QObject::tr( "Ignore rings in feature" ), QgsPropertyDefinition::Boolean, origin )},
    { QgsSymbolLayer::PropertyFile, QgsPropertyDefinition( "file", QObject::tr( "Symbol file path" ), QgsPropertyDefinition::String, origin )},
    { QgsSymbolLayer::PropertyDistanceX, QgsPropertyDefinition( "distanceX", QObject::tr( "Horizontal distance between markers" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyDistanceY, QgsPropertyDefinition( "distanceY", QObject::tr( "Vertical distance between markers" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyDisplacementX, QgsPropertyDefinition( "displacementX", QObject::tr( "Horizontal displacement between rows" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyDisplacementY, QgsPropertyDefinition( "displacementY", QObject::tr( "Vertical displacement between columns" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyOpacity, QgsPropertyDefinition( "alpha", QObject::tr( "Opacity" ), QgsPropertyDefinition::Opacity, origin )},
    { QgsSymbolLayer::PropertyCustomDash, QgsPropertyDefinition( "customDash", QgsPropertyDefinition::DataTypeString, QObject::tr( "Custom dash pattern" ), QObject::tr( "[<b><dash>;<space></b>] e.g. '8;2;1;2'" ), origin )},
    { QgsSymbolLayer::PropertyCapStyle, QgsPropertyDefinition( "capStyle", QObject::tr( "Line cap style" ), QgsPropertyDefinition::CapStyle, origin )},
    { QgsSymbolLayer::PropertyPlacement, QgsPropertyDefinition( "placement", QgsPropertyDefinition::DataTypeString, QObject::tr( "Marker placement" ), QObject::tr( "string " ) + "[<b>interval</b>|<b>vertex</b>|<b>lastvertex</b>|<b>firstvertex</b>|<b>centerpoint</b>|<b>curvepoint</b>]", origin )},
    { QgsSymbolLayer::PropertyInterval, QgsPropertyDefinition( "interval", QObject::tr( "Marker interval" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyOffsetAlongLine, QgsPropertyDefinition( "offsetAlongLine", QObject::tr( "Offset along line" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyHorizontalAnchor, QgsPropertyDefinition( "hAnchor", QObject::tr( "Horizontal anchor point" ), QgsPropertyDefinition::HorizontalAnchor, origin )},
    { QgsSymbolLayer::PropertyVerticalAnchor, QgsPropertyDefinition( "vAnchor", QObject::tr( "Vertical anchor point" ), QgsPropertyDefinition::VerticalAnchor, origin )},
    { QgsSymbolLayer::PropertyLayerEnabled, QgsPropertyDefinition( "enabled", QObject::tr( "Layer enabled" ), QgsPropertyDefinition::Boolean, origin )},
    { QgsSymbolLayer::PropertyArrowWidth, QgsPropertyDefinition( "arrowWidth", QObject::tr( "Arrow line width" ), QgsPropertyDefinition::StrokeWidth, origin )},
    { QgsSymbolLayer::PropertyArrowStartWidth, QgsPropertyDefinition( "arrowStartWidth", QObject::tr( "Arrow line start width" ), QgsPropertyDefinition::StrokeWidth, origin )},
    { QgsSymbolLayer::PropertyArrowHeadLength, QgsPropertyDefinition( "arrowHeadLength", QObject::tr( "Arrow head length" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyArrowHeadThickness, QgsPropertyDefinition( "arrowHeadThickness", QObject::tr( "Arrow head thickness" ), QgsPropertyDefinition::DoublePositive, origin )},
    { QgsSymbolLayer::PropertyArrowHeadType, QgsPropertyDefinition( "arrowHeadType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Arrow head type" ), QObject::tr( "string " ) + QLatin1String( "[<b>single</b>|<b>reversed</b>|<b>double</b>]" ) )},
    { QgsSymbolLayer::PropertyArrowType, QgsPropertyDefinition( "arrowType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Arrow type" ), QObject::tr( "string " ) + QLatin1String( "[<b>plain</b>|<b>lefthalf</b>|<b>righthalf</b>]" ) )},

  };
}

void QgsSymbolLayer::setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property )
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

double QgsSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( e );
  Q_UNUSED( context );
  return 1.0;
}

double QgsSymbolLayer::dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
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

QVector<qreal> QgsSymbolLayer::dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const
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

{
  mPaintEffect = QgsPaintEffectRegistry::defaultStack();
  mPaintEffect->setEnabled( false );
}

void QgsSymbolLayer::prepareExpressions( const QgsSymbolRenderContext &context )
{
  mDataDefinedProperties.prepare( context.renderContext().expressionContext() );

  if ( !context.fields().isEmpty() )
  {
    //QgsFields is implicitly shared, so it's cheap to make a copy
    mFields = context.fields();
  }
}

bool QgsSymbolLayer::hasDataDefinedProperties() const
{
  return mDataDefinedProperties.hasActiveProperties();
}

const QgsPropertiesDefinition &QgsSymbolLayer::propertyDefinitions()
{
  QgsSymbolLayer::initPropertyDefinitions();
  return sPropertyDefinitions;
}

QgsSymbolLayer::~QgsSymbolLayer()
{
  delete mPaintEffect;
}

bool QgsSymbolLayer::isCompatibleWithSymbol( QgsSymbol *symbol ) const
{
  if ( symbol->type() == QgsSymbol::Fill && mType == QgsSymbol::Line )
    return true;

  return symbol->type() == mType;
}

QSet<QString> QgsSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> columns = mDataDefinedProperties.referencedFields( context.expressionContext() );
  return columns;
}

QgsProperty propertyFromMap( const QgsStringMap &map, const QString &baseName )
{
  QString prefix;
  if ( !baseName.isEmpty() )
  {
    prefix.append( QStringLiteral( "%1_dd_" ).arg( baseName ) );
  }

  if ( !map.contains( QStringLiteral( "%1expression" ).arg( prefix ) ) )
  {
    //requires at least the expression value
    return QgsProperty();
  }

  bool active = ( map.value( QStringLiteral( "%1active" ).arg( prefix ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  QString expression = map.value( QStringLiteral( "%1expression" ).arg( prefix ) );
  bool useExpression = ( map.value( QStringLiteral( "%1useexpr" ).arg( prefix ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  QString field = map.value( QStringLiteral( "%1field" ).arg( prefix ), QString() );

  if ( useExpression )
    return QgsProperty::fromExpression( expression, active );
  else
    return QgsProperty::fromField( field, active );
}

// property string to type upgrade map
static const QMap< QString, QgsSymbolLayer::Property > OLD_PROPS
{
  { "color", QgsSymbolLayer::PropertyFillColor },
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
  { "outline_width_field", QgsSymbolLayer::PropertyStrokeWidth },
  { "fill_color_field", QgsSymbolLayer::PropertyFillColor },
  { "outline_color_field", QgsSymbolLayer::PropertyStrokeColor },
  { "symbol_name_field", QgsSymbolLayer::PropertyName },
  { "outline_width", QgsSymbolLayer::PropertyStrokeWidth },
  { "outline_style", QgsSymbolLayer::PropertyStrokeStyle },
  { "join_style", QgsSymbolLayer::PropertyJoinStyle },
  { "fill_color", QgsSymbolLayer::PropertyFillColor },
  { "outline_color", QgsSymbolLayer::PropertyStrokeColor },
  { "width", QgsSymbolLayer::PropertyWidth },
  { "height", QgsSymbolLayer::PropertyHeight },
  { "symbol_name", QgsSymbolLayer::PropertyName },
  { "angle", QgsSymbolLayer::PropertyAngle },
  { "fill_style", QgsSymbolLayer::PropertyFillStyle },
  { "color_border", QgsSymbolLayer::PropertyStrokeColor },
  { "width_border", QgsSymbolLayer::PropertyStrokeWidth },
  { "border_color", QgsSymbolLayer::PropertyStrokeColor },
  { "border_style", QgsSymbolLayer::PropertyStrokeStyle },
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
  { "svgOutlineColor", QgsSymbolLayer::PropertyStrokeColor },
  { "svgOutlineWidth", QgsSymbolLayer::PropertyStrokeWidth },
  { "svgFile", QgsSymbolLayer::PropertyFile },
  { "lineangle", QgsSymbolLayer::PropertyLineAngle },
  { "distance", QgsSymbolLayer::PropertyLineDistance },
  { "distance_x", QgsSymbolLayer::PropertyDistanceX },
  { "distance_y", QgsSymbolLayer::PropertyDistanceY },
  { "displacement_x", QgsSymbolLayer::PropertyDisplacementX },
  { "displacement_y", QgsSymbolLayer::PropertyDisplacementY },
  { "file", QgsSymbolLayer::PropertyFile },
  { "alpha", QgsSymbolLayer::PropertyOpacity },
  { "customdash", QgsSymbolLayer::PropertyCustomDash },
  { "line_style", QgsSymbolLayer::PropertyStrokeStyle },
  { "joinstyle", QgsSymbolLayer::PropertyJoinStyle },
  { "capstyle", QgsSymbolLayer::PropertyCapStyle },
  { "placement", QgsSymbolLayer::PropertyPlacement },
  { "interval", QgsSymbolLayer::PropertyInterval },
  { "offset_along_line", QgsSymbolLayer::PropertyOffsetAlongLine },
  { "name", QgsSymbolLayer::PropertyName },
  { "size", QgsSymbolLayer::PropertySize },
  { "fill", QgsSymbolLayer::PropertyFillColor },
  { "outline", QgsSymbolLayer::PropertyStrokeColor },
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
    QgsProperty prop;
    QString propertyName;

    if ( propIt.key().endsWith( QLatin1String( "_dd_expression" ) ) )
    {
      //found a data defined property

      //get data defined property name by stripping "_dd_expression" from property key
      propertyName = propIt.key().left( propIt.key().length() - 14 );

      prop = propertyFromMap( stringMap, propertyName );
    }
    else if ( propIt.key().endsWith( QLatin1String( "_expression" ) ) )
    {
      //old style data defined property, upgrade

      //get data defined property name by stripping "_expression" from property key
      propertyName = propIt.key().left( propIt.key().length() - 11 );

      prop = QgsProperty::fromExpression( propIt.value() );
    }

    if ( !prop || !OLD_PROPS.contains( propertyName ) )
      continue;

    QgsSymbolLayer::Property key = static_cast< QgsSymbolLayer::Property >( OLD_PROPS.value( propertyName ) );

    if ( type() == QgsSymbol::Line )
    {
      //these keys had different meaning for line symbol layers
      if ( propertyName == QLatin1String( "width" ) )
        key = QgsSymbolLayer::PropertyStrokeWidth;
      else if ( propertyName == QLatin1String( "color" ) )
        key = QgsSymbolLayer::PropertyStrokeColor;
    }

    setDataDefinedProperty( key, prop );
  }
}

void QgsSymbolLayer::copyDataDefinedProperties( QgsSymbolLayer *destLayer ) const
{
  if ( !destLayer )
    return;

  destLayer->setDataDefinedProperties( mDataDefinedProperties );
}

void QgsSymbolLayer::copyPaintEffect( QgsSymbolLayer *destLayer ) const
{
  if ( !destLayer || !mPaintEffect )
    return;

  destLayer->setPaintEffect( mPaintEffect->clone() );
}

QgsMarkerSymbolLayer::QgsMarkerSymbolLayer( bool locked )
  : QgsSymbolLayer( QgsSymbol::Marker, locked )
{

}

QgsLineSymbolLayer::QgsLineSymbolLayer( bool locked )
  : QgsSymbolLayer( QgsSymbol::Line, locked )
{
}

QgsLineSymbolLayer::RenderRingFilter QgsLineSymbolLayer::ringFilter() const
{
  return mRingFilter;
}

void QgsLineSymbolLayer::setRingFilter( const RenderRingFilter filter )
{
  mRingFilter = filter;
}

QgsFillSymbolLayer::QgsFillSymbolLayer( bool locked )
  : QgsSymbolLayer( QgsSymbol::Fill, locked )
{
}

void QgsMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );
}

void QgsMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );
}

void QgsMarkerSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  startRender( context );
  QgsPaintEffect *effect = paintEffect();
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

void QgsMarkerSymbolLayer::markerOffset( QgsSymbolRenderContext &context, double &offsetX, double &offsetY ) const
{
  markerOffset( context, mSize, mSize, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayer::markerOffset( QgsSymbolRenderContext &context, double width, double height, double &offsetX, double &offsetY ) const
{
  markerOffset( context, width, height, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayer::markerOffset( QgsSymbolRenderContext &context, double width, double height,
    QgsUnitTypes::RenderUnit widthUnit, QgsUnitTypes::RenderUnit heightUnit,
    double &offsetX, double &offsetY, const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const
{
  offsetX = mOffset.x();
  offsetY = mOffset.y();

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext() );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHorizontalAnchor ) )
  {
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyHorizontalAnchor, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      horizontalAnchorPoint = decodeHorizontalAnchorPoint( exprVal.toString() );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyVerticalAnchor ) )
  {
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyVerticalAnchor, context.renderContext().expressionContext() );
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
  double c = std::cos( angle ), s = std::sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

QgsMarkerSymbolLayer::HorizontalAnchorPoint QgsMarkerSymbolLayer::decodeHorizontalAnchorPoint( const QString &str )
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

QgsMarkerSymbolLayer::VerticalAnchorPoint QgsMarkerSymbolLayer::decodeVerticalAnchorPoint( const QString &str )
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

void QgsLineSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsLineSymbolLayer::mapUnitScale() const
{
  return mWidthMapUnitScale;
}


void QgsLineSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  QPolygonF points;
  // we're adding 0.5 to get rid of blurred preview:
  // drawing antialiased lines of width 1 at (x,0)-(x,100) creates 2px line
  points << QPointF( 0, int( size.height() / 2 ) + 0.5 ) << QPointF( size.width(), int( size.height() / 2 ) + 0.5 );

  startRender( context );
  QgsPaintEffect *effect = paintEffect();
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

void QgsLineSymbolLayer::renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  switch ( mRingFilter )
  {
    case AllRings:
    case ExteriorRingOnly:
      renderPolyline( points, context );
      break;
    case InteriorRingsOnly:
      break;
  }

  if ( rings )
  {
    switch ( mRingFilter )
    {
      case AllRings:
      case InteriorRingsOnly:
      {
        for ( const QPolygonF &ring : qgis::as_const( *rings ) )
          renderPolyline( ring, context );
      }
      break;
      case ExteriorRingOnly:
        break;
    }
  }
}

double QgsLineSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context );
  return width() * e.mapUnitScaleFactor( e.symbologyScale(), widthUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
}


void QgsFillSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width(), size.height() ) );
  startRender( context );
  QgsPaintEffect *effect = paintEffect();
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

void QgsFillSymbolLayer::_renderPolygon( QPainter *p, const QPolygonF &points, const QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
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
    path.addPolygon( points );

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

void QgsMarkerSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PointSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ) );

  writeSldMarker( doc, symbolizerElem, props );
}


