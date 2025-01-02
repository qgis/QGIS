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
#include "qgsrendercontext.h"
#include "qgsdxfexport.h"
#include "qgsgeometrysimplifier.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsproperty.h"
#include "qgsexpressioncontext.h"
#include "qgssymbollayerutils.h"
#include "qgslegendpatchshape.h"
#include "qgsstyle.h"
#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"
#include "qgssymbollayerreference.h"
#include "qgsgeos.h"

#include <QSize>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>
#include <QUuid>

QgsPropertiesDefinition QgsSymbolLayer::sPropertyDefinitions;

void QgsSymbolLayer::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  QString origin = QStringLiteral( "symbol" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsSymbolLayer::Property::Size ), QgsPropertyDefinition( "size", QObject::tr( "Symbol size" ), QgsPropertyDefinition::Size, origin ) },
    { static_cast< int >( QgsSymbolLayer::Property::Angle ), QgsPropertyDefinition( "angle", QObject::tr( "Rotation angle" ), QgsPropertyDefinition::Rotation, origin ) },
    { static_cast< int >( QgsSymbolLayer::Property::Name ), QgsPropertyDefinition( "name", QObject::tr( "Symbol name" ), QgsPropertyDefinition::String, origin ) },
    { static_cast< int >( QgsSymbolLayer::Property::FillColor ), QgsPropertyDefinition( "fillColor", QObject::tr( "Symbol fill color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { static_cast< int >( QgsSymbolLayer::Property::StrokeColor ), QgsPropertyDefinition( "outlineColor", QObject::tr( "Symbol stroke color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { static_cast< int >( QgsSymbolLayer::Property::StrokeWidth ), QgsPropertyDefinition( "outlineWidth", QObject::tr( "Symbol stroke width" ), QgsPropertyDefinition::StrokeWidth, origin ) },
    { static_cast< int >( QgsSymbolLayer::Property::StrokeStyle ), QgsPropertyDefinition( "outlineStyle", QObject::tr( "Symbol stroke style" ), QgsPropertyDefinition::LineStyle, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::Offset ), QgsPropertyDefinition( "offset", QObject::tr( "Symbol offset" ), QgsPropertyDefinition::Offset, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::Character ), QgsPropertyDefinition( "char", QObject::tr( "Marker character(s)" ), QgsPropertyDefinition::String, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::FontFamily ), QgsPropertyDefinition( "fontFamily", QObject::tr( "Font family" ), QgsPropertyDefinition::String, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::FontStyle ), QgsPropertyDefinition( "fontStyle", QObject::tr( "Font style" ), QgsPropertyDefinition::String, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::Width ), QgsPropertyDefinition( "width", QObject::tr( "Symbol width" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::Height ), QgsPropertyDefinition( "height", QObject::tr( "Symbol height" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::PreserveAspectRatio ), QgsPropertyDefinition( "preserveAspectRatio", QObject::tr( "Preserve aspect ratio between width and height" ), QgsPropertyDefinition::Boolean, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::FillStyle ), QgsPropertyDefinition( "fillStyle", QObject::tr( "Symbol fill style" ), QgsPropertyDefinition::FillStyle, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::JoinStyle ), QgsPropertyDefinition( "joinStyle", QObject::tr( "Outline join style" ), QgsPropertyDefinition::PenJoinStyle, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::SecondaryColor ), QgsPropertyDefinition( "color2", QObject::tr( "Secondary fill color" ), QgsPropertyDefinition::ColorWithAlpha, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LineAngle ), QgsPropertyDefinition( "lineAngle", QObject::tr( "Angle for line fills" ), QgsPropertyDefinition::Rotation, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientType ), QgsPropertyDefinition( "gradientType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Gradient type" ),  QObject::tr( "string " ) + QLatin1String( "[<b>linear</b>|<b>radial</b>|<b>conical</b>]" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::CoordinateMode ), QgsPropertyDefinition( "gradientMode", QgsPropertyDefinition::DataTypeString, QObject::tr( "Gradient mode" ), QObject::tr( "string " ) + QLatin1String( "[<b>feature</b>|<b>viewport</b>]" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientSpread ), QgsPropertyDefinition( "gradientSpread", QgsPropertyDefinition::DataTypeString, QObject::tr( "Gradient spread" ), QObject::tr( "string " ) + QLatin1String( "[<b>pad</b>|<b>repeat</b>|<b>reflect</b>]" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientReference1X ), QgsPropertyDefinition( "gradientRef1X", QObject::tr( "Reference point 1 (X)" ), QgsPropertyDefinition::Double0To1, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientReference1Y ), QgsPropertyDefinition( "gradientRef1Y", QObject::tr( "Reference point 1 (Y)" ), QgsPropertyDefinition::Double0To1, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientReference2X ), QgsPropertyDefinition( "gradientRef2X", QObject::tr( "Reference point 2 (X)" ), QgsPropertyDefinition::Double0To1, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientReference2Y ), QgsPropertyDefinition( "gradientRef2Y", QObject::tr( "Reference point 2 (Y)" ), QgsPropertyDefinition::Double0To1, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientReference1IsCentroid ), QgsPropertyDefinition( "gradientRef1Centroid", QObject::tr( "Reference point 1 follows feature centroid" ), QgsPropertyDefinition::Boolean, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::GradientReference2IsCentroid ), QgsPropertyDefinition( "gradientRef2Centroid", QObject::tr( "Reference point 2 follows feature centroid" ), QgsPropertyDefinition::Boolean, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::BlurRadius ), QgsPropertyDefinition( "blurRadius", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Blur radius" ), QObject::tr( "Integer between 0 and 18" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LineDistance ), QgsPropertyDefinition( "lineDistance", QObject::tr( "Distance between lines" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ShapeburstUseWholeShape ), QgsPropertyDefinition( "shapeburstWholeShape", QObject::tr( "Shade whole shape" ), QgsPropertyDefinition::Boolean, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ShapeburstMaxDistance ), QgsPropertyDefinition( "shapeburstMaxDist", QObject::tr( "Maximum distance for shapeburst fill" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ShapeburstIgnoreRings ), QgsPropertyDefinition( "shapeburstIgnoreRings", QObject::tr( "Ignore rings in feature" ), QgsPropertyDefinition::Boolean, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::File ), QgsPropertyDefinition( "file", QObject::tr( "Symbol file path" ), QgsPropertyDefinition::String, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::DistanceX ), QgsPropertyDefinition( "distanceX", QObject::tr( "Horizontal distance between markers" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::DistanceY ), QgsPropertyDefinition( "distanceY", QObject::tr( "Vertical distance between markers" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::DisplacementX ), QgsPropertyDefinition( "displacementX", QObject::tr( "Horizontal displacement between rows" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::DisplacementY ), QgsPropertyDefinition( "displacementY", QObject::tr( "Vertical displacement between columns" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::OffsetX ), QgsPropertyDefinition( "offsetX", QObject::tr( "Horizontal offset" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::OffsetY ), QgsPropertyDefinition( "offsetY", QObject::tr( "Vertical offset" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::Opacity ), QgsPropertyDefinition( "alpha", QObject::tr( "Opacity" ), QgsPropertyDefinition::Opacity, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::CustomDash ), QgsPropertyDefinition( "customDash", QgsPropertyDefinition::DataTypeString, QObject::tr( "Custom dash pattern" ), QObject::tr( "[<b><dash>;<space></b>] e.g. '8;2;1;2'" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::CapStyle ), QgsPropertyDefinition( "capStyle", QObject::tr( "Line cap style" ), QgsPropertyDefinition::CapStyle, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::Placement ), QgsPropertyDefinition( "placement", QgsPropertyDefinition::DataTypeString, QObject::tr( "Marker placement" ), QObject::tr( "string " ) + "[<b>interval</b>|<b>innervertices</b>|<b>vertex</b>|<b>lastvertex</b>|<b>firstvertex</b>|<b>centerpoint</b>|<b>curvepoint</b>|<b>segmentcenter</b>]", origin )},
    { static_cast< int >( QgsSymbolLayer::Property::Interval ), QgsPropertyDefinition( "interval", QObject::tr( "Marker interval" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::OffsetAlongLine ), QgsPropertyDefinition( "offsetAlongLine", QObject::tr( "Offset along line" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::AverageAngleLength ), QgsPropertyDefinition( "averageAngleLength", QObject::tr( "Average line angles over" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::HorizontalAnchor ), QgsPropertyDefinition( "hAnchor", QObject::tr( "Horizontal anchor point" ), QgsPropertyDefinition::HorizontalAnchor, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::VerticalAnchor ), QgsPropertyDefinition( "vAnchor", QObject::tr( "Vertical anchor point" ), QgsPropertyDefinition::VerticalAnchor, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LayerEnabled ), QgsPropertyDefinition( "enabled", QObject::tr( "Layer enabled" ), QgsPropertyDefinition::Boolean, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ArrowWidth ), QgsPropertyDefinition( "arrowWidth", QObject::tr( "Arrow line width" ), QgsPropertyDefinition::StrokeWidth, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ArrowStartWidth ), QgsPropertyDefinition( "arrowStartWidth", QObject::tr( "Arrow line start width" ), QgsPropertyDefinition::StrokeWidth, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ArrowHeadLength ), QgsPropertyDefinition( "arrowHeadLength", QObject::tr( "Arrow head length" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ArrowHeadThickness ), QgsPropertyDefinition( "arrowHeadThickness", QObject::tr( "Arrow head thickness" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ArrowHeadType ), QgsPropertyDefinition( "arrowHeadType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Arrow head type" ), QObject::tr( "string " ) + QLatin1String( "[<b>single</b>|<b>reversed</b>|<b>double</b>]" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ArrowType ), QgsPropertyDefinition( "arrowType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Arrow type" ), QObject::tr( "string " ) + QLatin1String( "[<b>plain</b>|<b>lefthalf</b>|<b>righthalf</b>]" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::PointCount ), QgsPropertyDefinition( "pointCount", QObject::tr( "Point count" ), QgsPropertyDefinition::IntegerPositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::RandomSeed ), QgsPropertyDefinition( "randomSeed", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Random number seed" ), QObject::tr( "integer > 0, or 0 for completely random sequence" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ClipPoints ), QgsPropertyDefinition( "clipPoints", QObject::tr( "Clip markers" ), QgsPropertyDefinition::Boolean, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::DensityArea ), QgsPropertyDefinition( "densityArea", QObject::tr( "Density area" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::DashPatternOffset ), QgsPropertyDefinition( "dashPatternOffset", QObject::tr( "Dash pattern offset" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::TrimStart ), QgsPropertyDefinition( "trimStart", QObject::tr( "Start trim distance" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::TrimEnd ), QgsPropertyDefinition( "trimEnd", QObject::tr( "End trim distance" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LineStartWidthValue ), QgsPropertyDefinition( "lineStartWidthValue", QObject::tr( "Line start width value" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LineEndWidthValue ), QgsPropertyDefinition( "lineEndWidthValue", QObject::tr( "Line end width value" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LineStartColorValue ), QgsPropertyDefinition( "lineStartColorValue", QObject::tr( "Line start color value" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LineEndColorValue ), QgsPropertyDefinition( "lineEndColorValue", QObject::tr( "Line end color value" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::MarkerClipping ), QgsPropertyDefinition( "markerClipping", QgsPropertyDefinition::DataTypeString, QObject::tr( "Marker clipping mode" ),  QObject::tr( "string " ) + QLatin1String( "[<b>no</b>|<b>shape</b>|<b>centroid_within</b>|<b>completely_within</b>]" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::RandomOffsetX ), QgsPropertyDefinition( "randomOffsetX", QObject::tr( "Horizontal random offset" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::RandomOffsetY ), QgsPropertyDefinition( "randomOffsetY", QObject::tr( "Vertical random offset" ), QgsPropertyDefinition::Double, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::LineClipping ), QgsPropertyDefinition( "lineClipping", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line clipping mode" ),  QObject::tr( "string " ) + QLatin1String( "[<b>no</b>|<b>during_render</b>|<b>before_render</b>]" ), origin )},
    { static_cast< int >( QgsSymbolLayer::Property::SkipMultiples ), QgsPropertyDefinition( "skipMultiples", QObject::tr( "Skip multiples of" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsSymbolLayer::Property::ShowMarker ), QgsPropertyDefinition( "showMarker", QObject::tr( "Show marker" ), QgsPropertyDefinition::Boolean, origin )},
  };
}

void QgsSymbolLayer::setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property )
{
  dataDefinedProperties().setProperty( key, property );
}

void QgsSymbolLayer::startFeatureRender( const QgsFeature &feature, QgsRenderContext &context )
{
  installMasks( context, false );

  if ( QgsSymbol *lSubSymbol = subSymbol() )
    lSubSymbol->startFeatureRender( feature, context );
}

void QgsSymbolLayer::stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context )
{
  if ( QgsSymbol *lSubSymbol = subSymbol() )
    lSubSymbol->stopFeatureRender( feature, context );

  removeMasks( context, false );
}

QgsSymbol *QgsSymbolLayer::subSymbol()
{
  return nullptr;
}

bool QgsSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  delete symbol;
  return false;
}

bool QgsSymbolLayer::writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift ) const
{
  Q_UNUSED( e )
  Q_UNUSED( mmMapUnitScaleFactor )
  Q_UNUSED( layerName )
  Q_UNUSED( context )
  Q_UNUSED( shift )
  return false;
}

double QgsSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( e )
  Q_UNUSED( context )
  return 1.0;
}

double QgsSymbolLayer::dxfSize( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( e )
  Q_UNUSED( context )
  return 1.0;
}

double QgsSymbolLayer::dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( e )
  Q_UNUSED( context )
  return 0.0;
}

QColor QgsSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context )
  return color();
}

double QgsSymbolLayer::dxfAngle( QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context )
  return 0.0;
}

QVector<qreal> QgsSymbolLayer::dxfCustomDashPattern( Qgis::RenderUnit &unit ) const
{
  Q_UNUSED( unit )
  return QVector<qreal>();
}

Qt::PenStyle QgsSymbolLayer::dxfPenStyle() const
{
  return Qt::SolidLine;
}

QColor QgsSymbolLayer::dxfBrushColor( QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context )
  return color();
}

Qt::BrushStyle QgsSymbolLayer::dxfBrushStyle() const
{
  return Qt::NoBrush;
}

QgsPaintEffect *QgsSymbolLayer::paintEffect() const
{
  return mPaintEffect.get();
}

void QgsSymbolLayer::setPaintEffect( QgsPaintEffect *effect )
{
  if ( effect == mPaintEffect.get() )
    return;

  mPaintEffect.reset( effect );
}

QgsSymbolLayer::QgsSymbolLayer( const QgsSymbolLayer &other )
  : mType( other.mType )
  , mEnabled( other.mEnabled )
  , mUserFlags( other.mUserFlags )
  , mLocked( other.mLocked )
  , mColor( other.mColor )
  , mRenderingPass( other.mRenderingPass )
  , mId( other.mId )
  , mDataDefinedProperties( other.mDataDefinedProperties )
  , mPaintEffect( other.mPaintEffect ? other.mPaintEffect->clone() : nullptr )
  , mFields( other.mFields )
  , mClipPath( other.mClipPath )
{
}

QgsSymbolLayer::QgsSymbolLayer( Qgis::SymbolType type, bool locked )
  : mType( type )
  , mLocked( locked )
  , mId( QUuid::createUuid().toString() )
{
}

Qgis::SymbolLayerFlags QgsSymbolLayer::flags() const
{
  return Qgis::SymbolLayerFlags();
}

Qgis::SymbolLayerUserFlags QgsSymbolLayer::userFlags() const
{
  return mUserFlags;
}

void QgsSymbolLayer::setUserFlags( Qgis::SymbolLayerUserFlags flags )
{
  mUserFlags = flags;
}

QColor QgsSymbolLayer::color() const
{
  return mColor;
}

void QgsSymbolLayer::setColor( const QColor &color )
{
  mColor = color;
}

void QgsSymbolLayer::setStrokeColor( const QColor & )
{

}

QColor QgsSymbolLayer::strokeColor() const
{
  return QColor();
}

void QgsSymbolLayer::setFillColor( const QColor & )
{
}

QColor QgsSymbolLayer::fillColor() const
{
  return QColor();
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

QgsSymbolLayer::~QgsSymbolLayer() = default;

bool QgsSymbolLayer::isCompatibleWithSymbol( QgsSymbol *symbol ) const
{
  if ( symbol->type() == Qgis::SymbolType::Fill && mType == Qgis::SymbolType::Line )
    return true;

  return symbol->type() == mType;
}

bool QgsSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return false;
}

bool QgsSymbolLayer::usesMapUnits() const
{
  return false;
}

void QgsSymbolLayer::setRenderingPass( int renderingPass )
{
  mRenderingPass = renderingPass;
}

int QgsSymbolLayer::renderingPass() const
{
  return mRenderingPass;
}

QSet<QString> QgsSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  // calling referencedFields() with ignoreContext=true because in our expression context
  // we do not have valid QgsFields yet - because of that the field names from expressions
  // wouldn't get reported
  QSet<QString> columns = mDataDefinedProperties.referencedFields( context.expressionContext(), true );
  return columns;
}

QgsProperty propertyFromMap( const QVariantMap &map, const QString &baseName )
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
  QString expression = map.value( QStringLiteral( "%1expression" ).arg( prefix ) ).toString();
  bool useExpression = ( map.value( QStringLiteral( "%1useexpr" ).arg( prefix ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  QString field = map.value( QStringLiteral( "%1field" ).arg( prefix ), QString() ).toString();

  if ( useExpression )
    return QgsProperty::fromExpression( expression, active );
  else
    return QgsProperty::fromField( field, active );
}

void QgsSymbolLayer::restoreOldDataDefinedProperties( const QVariantMap &stringMap )
{
  // property string to type upgrade map
  static const QMap < QString, int > OLD_PROPS
  {
    { "color", static_cast< int >( QgsSymbolLayer::Property::FillColor ) },
    { "arrow_width", static_cast< int >( QgsSymbolLayer::Property::ArrowWidth ) },
    { "arrow_start_width", static_cast< int >( QgsSymbolLayer::Property::ArrowStartWidth ) },
    { "head_length", static_cast< int >( QgsSymbolLayer::Property::ArrowHeadLength ) },
    { "head_thickness", static_cast< int >( QgsSymbolLayer::Property::ArrowHeadThickness ) },
    { "offset", static_cast< int >( QgsSymbolLayer::Property::Offset ) },
    { "head_type", static_cast< int >( QgsSymbolLayer::Property::ArrowHeadType ) },
    { "arrow_type", static_cast< int >( QgsSymbolLayer::Property::ArrowType ) },
    { "width_field", static_cast< int >( QgsSymbolLayer::Property::Width ) },
    { "height_field", static_cast< int >( QgsSymbolLayer::Property::Height ) },
    { "rotation_field", static_cast< int >( QgsSymbolLayer::Property::Angle ) },
    { "outline_width_field", static_cast< int >( QgsSymbolLayer::Property::StrokeWidth ) },
    { "fill_color_field", static_cast< int >( QgsSymbolLayer::Property::FillColor ) },
    { "outline_color_field", static_cast< int >( QgsSymbolLayer::Property::StrokeColor ) },
    { "symbol_name_field", static_cast< int >( QgsSymbolLayer::Property::Name ) },
    { "outline_width", static_cast< int >( QgsSymbolLayer::Property::StrokeWidth ) },
    { "outline_style", static_cast< int >( QgsSymbolLayer::Property::StrokeStyle ) },
    { "join_style", static_cast< int >( QgsSymbolLayer::Property::JoinStyle ) },
    { "fill_color", static_cast< int >( QgsSymbolLayer::Property::FillColor ) },
    { "outline_color", static_cast< int >( QgsSymbolLayer::Property::StrokeColor ) },
    { "width", static_cast< int >( QgsSymbolLayer::Property::Width ) },
    { "height", static_cast< int >( QgsSymbolLayer::Property::Height ) },
    { "symbol_name", static_cast< int >( QgsSymbolLayer::Property::Name ) },
    { "angle", static_cast< int >( QgsSymbolLayer::Property::Angle ) },
    { "fill_style", static_cast< int >( QgsSymbolLayer::Property::FillStyle ) },
    { "color_border", static_cast< int >( QgsSymbolLayer::Property::StrokeColor ) },
    { "width_border", static_cast< int >( QgsSymbolLayer::Property::StrokeWidth ) },
    { "border_color", static_cast< int >( QgsSymbolLayer::Property::StrokeColor ) },
    { "border_style", static_cast< int >( QgsSymbolLayer::Property::StrokeStyle ) },
    { "color2", static_cast< int >( QgsSymbolLayer::Property::SecondaryColor ) },
    { "gradient_type", static_cast< int >( QgsSymbolLayer::Property::GradientType ) },
    { "coordinate_mode", static_cast< int >( QgsSymbolLayer::Property::CoordinateMode )},
    { "spread", static_cast< int >( QgsSymbolLayer::Property::GradientSpread ) },
    { "reference1_x", static_cast< int >( QgsSymbolLayer::Property::GradientReference1X ) },
    { "reference1_y", static_cast< int >( QgsSymbolLayer::Property::GradientReference1Y ) },
    { "reference2_x", static_cast< int >( QgsSymbolLayer::Property::GradientReference2X ) },
    { "reference2_y", static_cast< int >( QgsSymbolLayer::Property::GradientReference2Y )},
    { "reference1_iscentroid", static_cast< int >( QgsSymbolLayer::Property::GradientReference1IsCentroid )},
    { "reference2_iscentroid", static_cast< int >( QgsSymbolLayer::Property::GradientReference2IsCentroid )},
    { "blur_radius", static_cast< int >( QgsSymbolLayer::Property::BlurRadius ) },
    { "use_whole_shape", static_cast< int >( QgsSymbolLayer::Property::ShapeburstUseWholeShape ) },
    { "max_distance", static_cast< int >( QgsSymbolLayer::Property::ShapeburstMaxDistance ) },
    { "ignore_rings", static_cast< int >( QgsSymbolLayer::Property::ShapeburstIgnoreRings ) },
    { "svgFillColor", static_cast< int >( QgsSymbolLayer::Property::FillColor ) },
    { "svgOutlineColor", static_cast< int >( QgsSymbolLayer::Property::StrokeColor ) },
    { "svgOutlineWidth", static_cast< int >( QgsSymbolLayer::Property::StrokeWidth ) },
    { "svgFile", static_cast< int >( QgsSymbolLayer::Property::File ) },
    { "lineangle", static_cast< int >( QgsSymbolLayer::Property::LineAngle ) },
    { "distance", static_cast< int >( QgsSymbolLayer::Property::LineDistance )},
    { "distance_x", static_cast< int >( QgsSymbolLayer::Property::DistanceX )},
    { "distance_y", static_cast< int >( QgsSymbolLayer::Property::DistanceY ) },
    { "displacement_x", static_cast< int >( QgsSymbolLayer::Property::DisplacementX )},
    { "displacement_y", static_cast< int >( QgsSymbolLayer::Property::DisplacementY ) },
    { "file", static_cast< int >( QgsSymbolLayer::Property::File ) },
    { "alpha", static_cast< int >( QgsSymbolLayer::Property::Opacity )},
    { "customdash", static_cast< int >( QgsSymbolLayer::Property::CustomDash ) },
    { "line_style", static_cast< int >( QgsSymbolLayer::Property::StrokeStyle ) },
    { "joinstyle", static_cast< int >( QgsSymbolLayer::Property::JoinStyle ) },
    { "capstyle", static_cast< int >( QgsSymbolLayer::Property::CapStyle ) },
    { "placement", static_cast< int >( QgsSymbolLayer::Property::Placement ) },
    { "interval", static_cast< int >( QgsSymbolLayer::Property::Interval ) },
    { "offset_along_line", static_cast< int >( QgsSymbolLayer::Property::OffsetAlongLine ) },
    { "name", static_cast< int >( QgsSymbolLayer::Property::Name ) },
    { "size", static_cast< int >( QgsSymbolLayer::Property::Size ) },
    { "fill", static_cast< int >( QgsSymbolLayer::Property::FillColor ) },
    { "outline", static_cast< int >( QgsSymbolLayer::Property::StrokeColor )},
    { "char", static_cast< int >( QgsSymbolLayer::Property::Character )},
    { "enabled", static_cast< int >( QgsSymbolLayer::Property::LayerEnabled ) },
    { "rotation", static_cast< int >( QgsSymbolLayer::Property::Angle )},
    { "horizontal_anchor_point", static_cast< int >( QgsSymbolLayer::Property::HorizontalAnchor ) },
    { "vertical_anchor_point", static_cast< int >( QgsSymbolLayer::Property::VerticalAnchor ) },
  };

  QVariantMap::const_iterator propIt = stringMap.constBegin();
  for ( ; propIt != stringMap.constEnd(); ++propIt )
  {
    std::unique_ptr<QgsProperty> prop;
    QString propertyName;

    if ( propIt.key().endsWith( QLatin1String( "_dd_expression" ) ) )
    {
      //found a data defined property

      //get data defined property name by stripping "_dd_expression" from property key
      propertyName = propIt.key().left( propIt.key().length() - 14 );

      prop = std::make_unique<QgsProperty>( propertyFromMap( stringMap, propertyName ) );
    }
    else if ( propIt.key().endsWith( QLatin1String( "_expression" ) ) )
    {
      //old style data defined property, upgrade

      //get data defined property name by stripping "_expression" from property key
      propertyName = propIt.key().left( propIt.key().length() - 11 );

      prop = std::make_unique<QgsProperty>( QgsProperty::fromExpression( propIt.value().toString() ) );
    }

    if ( !prop || !OLD_PROPS.contains( propertyName ) )
      continue;

    int key = OLD_PROPS.value( propertyName );

    if ( type() == Qgis::SymbolType::Line )
    {
      //these keys had different meaning for line symbol layers
      if ( propertyName == QLatin1String( "width" ) )
        key = static_cast< int >( QgsSymbolLayer::Property::StrokeWidth );
      else if ( propertyName == QLatin1String( "color" ) )
        key = static_cast< int >( QgsSymbolLayer::Property::StrokeColor );
    }

    setDataDefinedProperty( static_cast< QgsSymbolLayer::Property >( key ), QgsProperty( *prop.get() ) );
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

  if ( !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect.get() ) )
    destLayer->setPaintEffect( mPaintEffect->clone() );
  else
    destLayer->setPaintEffect( nullptr );
}

QgsMarkerSymbolLayer::QgsMarkerSymbolLayer( bool locked )
  : QgsSymbolLayer( Qgis::SymbolType::Marker, locked )
{

}

QgsLineSymbolLayer::QgsLineSymbolLayer( bool locked )
  : QgsSymbolLayer( Qgis::SymbolType::Line, locked )
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
  : QgsSymbolLayer( Qgis::SymbolType::Fill, locked )
{
}

QgsMarkerSymbolLayer::QgsMarkerSymbolLayer( const QgsMarkerSymbolLayer &other )
  : QgsSymbolLayer( other )
  , mAngle( other.mAngle )
  , mLineAngle( other.mLineAngle )
  , mSize( other.mSize )
  , mSizeUnit( other.mSizeUnit )
  , mSizeMapUnitScale( other.mSizeMapUnitScale )
  , mOffset( other.mOffset )
  , mOffsetUnit( other.mOffsetUnit )
  , mOffsetMapUnitScale( other.mOffsetMapUnitScale )
  , mScaleMethod( other.mScaleMethod )
  , mHorizontalAnchorPoint( other.mHorizontalAnchorPoint )
  , mVerticalAnchorPoint( other.mVerticalAnchorPoint )
{

}

void QgsMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsMarkerSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  startRender( context );
  QgsPaintEffect *effect = paintEffect();

  QPolygonF points = context.patchShape() ? context.patchShape()->toQPolygonF( Qgis::SymbolType::Marker, size ).value( 0 ).value( 0 )
                     : QgsStyle::defaultStyle()->defaultPatchAsQPolygonF( Qgis::SymbolType::Marker, size ).value( 0 ).value( 0 );

  std::unique_ptr< QgsEffectPainter > effectPainter;
  if ( effect && effect->enabled() )
    effectPainter = std::make_unique< QgsEffectPainter >( context.renderContext(), effect );

  for ( QPointF point : std::as_const( points ) )
    renderPoint( point, context );

  effectPainter.reset();

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
    Qgis::RenderUnit widthUnit, Qgis::RenderUnit heightUnit,
    double &offsetX, double &offsetY, const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const
{
  offsetX = mOffset.x();
  offsetY = mOffset.y();

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Offset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::Offset, context.renderContext().expressionContext() );
    bool ok = false;
    const QPointF offset = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
    if ( ok )
    {
      offsetX = offset.x();
      offsetY = offset.y();
    }
  }

  offsetX = context.renderContext().convertToPainterUnits( offsetX, mOffsetUnit, mOffsetMapUnitScale );
  offsetY = context.renderContext().convertToPainterUnits( offsetY, mOffsetUnit, mOffsetMapUnitScale );

  HorizontalAnchorPoint horizontalAnchorPoint = mHorizontalAnchorPoint;
  VerticalAnchorPoint verticalAnchorPoint = mVerticalAnchorPoint;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::HorizontalAnchor ) )
  {
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::HorizontalAnchor, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      horizontalAnchorPoint = decodeHorizontalAnchorPoint( exprVal.toString() );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::VerticalAnchor ) )
  {
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::VerticalAnchor, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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
  if ( widthUnit == Qgis::RenderUnit::MetersInMapUnits && context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
  {
    // rendering for symbol previews -- an size in meters in map units can't be calculated, so treat the size as millimeters
    // and clamp it to a reasonable range. It's the best we can do in this situation!
    anchorPointCorrectionX = std::min( std::max( context.renderContext().convertToPainterUnits( width, Qgis::RenderUnit::Millimeters ), 3.0 ), 100.0 ) / 2.0;
  }

  double anchorPointCorrectionY = context.renderContext().convertToPainterUnits( height, heightUnit, heightMapUnitScale ) / 2.0;
  if ( heightUnit == Qgis::RenderUnit::MetersInMapUnits && context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
  {
    // rendering for symbol previews -- an size in meters in map units can't be calculated, so treat the size as millimeters
    // and clamp it to a reasonable range. It's the best we can do in this situation!
    anchorPointCorrectionY = std::min( std::max( context.renderContext().convertToPainterUnits( height, Qgis::RenderUnit::Millimeters ), 3.0 ), 100.0 ) / 2.0;
  }

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

void QgsMarkerSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  mSizeUnit = unit;
  mOffsetUnit = unit;
}

Qgis::RenderUnit QgsMarkerSymbolLayer::outputUnit() const
{
  if ( mOffsetUnit != mSizeUnit )
  {
    return Qgis::RenderUnit::Unknown;
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

void QgsLineSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  mWidthUnit = unit;
  mOffsetUnit = unit;
}

Qgis::RenderUnit QgsLineSymbolLayer::outputUnit() const
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
  const QList< QList< QPolygonF > > points = context.patchShape() ? context.patchShape()->toQPolygonF( Qgis::SymbolType::Line, size )
      : QgsStyle::defaultStyle()->defaultPatchAsQPolygonF( Qgis::SymbolType::Line, size );
  startRender( context );
  QgsPaintEffect *effect = paintEffect();

  std::unique_ptr< QgsEffectPainter > effectPainter;
  if ( effect && effect->enabled() )
    effectPainter = std::make_unique< QgsEffectPainter >( context.renderContext(), effect );

  for ( const QList< QPolygonF > &line : points )
    renderPolyline( line.value( 0 ), context );

  effectPainter.reset();

  stopRender( context );
}

void QgsLineSymbolLayer::renderPolygonStroke( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QgsExpressionContextScope *scope = nullptr;
  std::unique_ptr< QgsExpressionContextScopePopper > scopePopper;
  if ( hasDataDefinedProperties() )
  {
    scope = new QgsExpressionContextScope();
    scopePopper = std::make_unique< QgsExpressionContextScopePopper >( context.renderContext().expressionContext(), scope );
  }

  switch ( mRingFilter )
  {
    case AllRings:
    case ExteriorRingOnly:
    {
      if ( scope )
        scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, 0, true ) );
      renderPolyline( points, context );
      break;
    }
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
        int ringIndex = 1;
        for ( const QPolygonF &ring : std::as_const( *rings ) )
        {
          if ( scope )
            scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, ringIndex, true ) );

          renderPolyline( ring, context );
          ringIndex++;
        }
      }
      break;
      case ExteriorRingOnly:
        break;
    }
  }
}

double QgsLineSymbolLayer::width( const QgsRenderContext &context ) const
{
  return context.convertToPainterUnits( mWidth, mWidthUnit, mWidthMapUnitScale );
}

double QgsLineSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context )
  return width() * QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), widthUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
}


void QgsFillSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  const QList< QList< QPolygonF > > polys = context.patchShape() ? context.patchShape()->toQPolygonF( Qgis::SymbolType::Fill, size )
      : QgsStyle::defaultStyle()->defaultPatchAsQPolygonF( Qgis::SymbolType::Fill, size );

  startRender( context );
  QgsPaintEffect *effect = paintEffect();

  std::unique_ptr< QgsEffectPainter > effectPainter;
  if ( effect && effect->enabled() )
    effectPainter = std::make_unique< QgsEffectPainter >( context.renderContext(), effect );

  for ( const QList< QPolygonF > &poly : polys )
  {
    QVector< QPolygonF > rings;
    for ( int i = 1; i < poly.size(); ++i )
      rings << poly.at( i );
    renderPolygon( poly.value( 0 ), &rings, context );
  }

  effectPainter.reset();

  stopRender( context );
}

QImage QgsFillSymbolLayer::toTiledPatternImage() const
{
  return QImage();
}

void QgsFillSymbolLayer::_renderPolygon( QPainter *p, const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  if ( !p )
  {
    return;
  }

  // Disable 'Antialiasing' if the geometry was generalized in the current RenderContext (We known that it must have least #5 points).
  if ( points.size() <= 5 &&
       ( context.renderContext().vectorSimplifyMethod().simplifyHints() & Qgis::VectorRenderingSimplificationFlag::AntialiasingSimplification ) &&
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
      for ( auto it = rings->constBegin(); it != rings->constEnd(); ++it )
      {
        QPolygonF ring = *it;
        path.addPolygon( ring );
      }
    }

    p->drawPath( path );
  }
}

void QgsMarkerSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PointSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

  writeSldMarker( doc, symbolizerElem, props );
}

QList<QgsSymbolLayerReference> QgsSymbolLayer::masks() const
{
  return {};
}

double QgsMarkerSymbolLayer::dxfSize( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  double size = mSize;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Size ) )
  {
    bool ok = false;
    size = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Size, context.renderContext().expressionContext(), mSize, &ok );

    if ( ok )
    {
      switch ( mScaleMethod )
      {
        case Qgis::ScaleMethod::ScaleArea:
          size = std::sqrt( size );
          break;
        case Qgis::ScaleMethod::ScaleDiameter:
          break;
      }
    }
  }
  return size * QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mSizeUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
}

double QgsMarkerSymbolLayer::dxfAngle( QgsSymbolRenderContext &context ) const
{
  double angle = mAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Angle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Angle, context.renderContext().expressionContext(), mAngle );
  }
  return angle;
}

QPainterPath generateClipPath( const QgsRenderContext &renderContext, const QString &id, const QRectF *rect, bool &foundGeometries )
{
  foundGeometries = false;
  const QVector<QgsGeometry> clipGeometries = rect
      ? QgsSymbolLayerUtils::collectSymbolLayerClipGeometries( renderContext, id, *rect )
      : renderContext.symbolLayerClipGeometries( id );
  if ( !clipGeometries.empty() )
  {
    foundGeometries = true;
    QgsGeometry mergedGeom = QgsGeometry::unaryUnion( clipGeometries );
    if ( renderContext.maskSettings().simplifyTolerance() > 0 )
    {
      QgsGeos geos( mergedGeom.constGet() );
      mergedGeom = QgsGeometry( geos.simplify( renderContext.maskSettings().simplifyTolerance() ) );
    }
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<10
    // structure would be better, but too old GEOS
    mergedGeom = mergedGeom.makeValid( Qgis::MakeValidMethod::Linework );
#else
    mergedGeom = mergedGeom.makeValid( Qgis::MakeValidMethod::Structure );
#endif
    if ( !mergedGeom.isEmpty() )
    {
      QgsGeometry exterior;
      const QgsRectangle contextBounds( 0, 0,
                                        renderContext.outputSize().width(),
                                        renderContext.outputSize().height() );
      if ( rect )
      {
        exterior = QgsGeometry::fromRect( QgsRectangle( *rect ).intersect( contextBounds ) );
      }
      else
      {
        exterior = QgsGeometry::fromRect( contextBounds );
      }
      const QgsGeometry maskGeom = exterior.difference( mergedGeom );
      if ( !maskGeom.isNull() )
      {
        return maskGeom.constGet()->asQPainterPath();
      }
    }
  }
  return QPainterPath();
}

void QgsSymbolLayer::prepareMasks( const QgsSymbolRenderContext &context )
{
  const QgsRenderContext &renderContext = context.renderContext();

  bool foundGeometries = false;
  mClipPath = generateClipPath( renderContext, id(), nullptr, foundGeometries );
}

bool QgsSymbolLayer::installMasks( QgsRenderContext &context, bool recursive, const QRectF &rect )
{
  bool res = false;
  if ( !mClipPath.isEmpty() )
  {
    context.painter()->save();
    context.painter()->setClipPath( mClipPath, Qt::IntersectClip );
    res = true;
  }
  else if ( rect.isValid() )
  {
    // find just the clip geometries within the area the symbol layer will be drawn over
    bool foundGeometries = false;
    const QPainterPath clipPath = generateClipPath( context, id(), &rect, foundGeometries );
    if ( !clipPath.isEmpty() )
    {
      context.painter()->setClipPath( clipPath, context.painter()->clipPath().isEmpty() ? Qt::ReplaceClip : Qt::IntersectClip );
      res = true;
    }
  }

  if ( QgsSymbol *lSubSymbol = recursive ? subSymbol() : nullptr )
  {
    const QList<QgsSymbolLayer *> layers = lSubSymbol->symbolLayers();
    for ( QgsSymbolLayer *sl : layers )
      res = sl->installMasks( context, true ) || res;
  }

  return res;
}

void QgsSymbolLayer::removeMasks( QgsRenderContext &context, bool recursive )
{
  if ( !mClipPath.isEmpty() )
  {
    context.painter()->restore();
  }

  if ( QgsSymbol *lSubSymbol = recursive ? subSymbol() : nullptr )
  {
    const QList<QgsSymbolLayer *> layers = lSubSymbol->symbolLayers();
    for ( QgsSymbolLayer *sl : layers )
      sl->removeMasks( context, true );
  }
}

bool QgsSymbolLayer::shouldRenderUsingSelectionColor( const QgsSymbolRenderContext &context ) const
{
  return context.selected() && !( mUserFlags & Qgis::SymbolLayerUserFlag::DisableSelectionRecoloring );
}

void QgsSymbolLayer::setId( const QString &id )
{
  mId = id;
}

QString QgsSymbolLayer::id() const
{
  return mId;
}
