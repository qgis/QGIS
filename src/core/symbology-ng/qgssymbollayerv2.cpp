/***************************************************************************
 qgssymbollayerv2.cpp
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

#include "qgssymbollayerv2.h"
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

const QString QgsSymbolLayerV2::EXPR_SIZE( "size" );
const QString QgsSymbolLayerV2::EXPR_ANGLE( "angle" );
const QString QgsSymbolLayerV2::EXPR_NAME( "name" );
const QString QgsSymbolLayerV2::EXPR_COLOR( "color" );
const QString QgsSymbolLayerV2::EXPR_COLOR_BORDER( "color_border" );
const QString QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH( "outline_width" );
const QString QgsSymbolLayerV2::EXPR_OUTLINE_STYLE( "outline_style" );
const QString QgsSymbolLayerV2::EXPR_FILL( "fill" );
const QString QgsSymbolLayerV2::EXPR_OUTLINE( "outline" );
const QString QgsSymbolLayerV2::EXPR_OFFSET( "offset" );
const QString QgsSymbolLayerV2::EXPR_CHAR( "char" );
const QString QgsSymbolLayerV2::EXPR_FILL_COLOR( "fill_color" );
const QString QgsSymbolLayerV2::EXPR_OUTLINE_COLOR( "outline_color" );
const QString QgsSymbolLayerV2::EXPR_WIDTH( "width" );
const QString QgsSymbolLayerV2::EXPR_HEIGHT( "height" );
const QString QgsSymbolLayerV2::EXPR_SYMBOL_NAME( "symbol_name" );
const QString QgsSymbolLayerV2::EXPR_ROTATION( "rotation" );
const QString QgsSymbolLayerV2::EXPR_FILL_STYLE( "fill_style" );
const QString QgsSymbolLayerV2::EXPR_WIDTH_BORDER( "width_border" );
const QString QgsSymbolLayerV2::EXPR_BORDER_STYLE( "border_style" );
const QString QgsSymbolLayerV2::EXPR_JOIN_STYLE( "join_style" );
const QString QgsSymbolLayerV2::EXPR_BORDER_COLOR( "border_color" );
const QString QgsSymbolLayerV2::EXPR_COLOR2( "color2" );
const QString QgsSymbolLayerV2::EXPR_LINEANGLE( "lineangle" );
const QString QgsSymbolLayerV2::EXPR_GRADIENT_TYPE( "gradient_type" );
const QString QgsSymbolLayerV2::EXPR_COORDINATE_MODE( "coordinate_mode" );
const QString QgsSymbolLayerV2::EXPR_SPREAD( "spread" );
const QString QgsSymbolLayerV2::EXPR_REFERENCE1_X( "reference1_x" );
const QString QgsSymbolLayerV2::EXPR_REFERENCE1_Y( "reference1_y" );
const QString QgsSymbolLayerV2::EXPR_REFERENCE2_X( "reference2_x" );
const QString QgsSymbolLayerV2::EXPR_REFERENCE2_Y( "reference2_y" );
const QString QgsSymbolLayerV2::EXPR_REFERENCE1_ISCENTROID( "reference1_iscentroid" );
const QString QgsSymbolLayerV2::EXPR_REFERENCE2_ISCENTROID( "reference2_iscentroid" );
const QString QgsSymbolLayerV2::EXPR_BLUR_RADIUS( "blur_radius" );
const QString QgsSymbolLayerV2::EXPR_DISTANCE( "distance" );
const QString QgsSymbolLayerV2::EXPR_USE_WHOLE_SHAPE( "use_whole_shape" );
const QString QgsSymbolLayerV2::EXPR_MAX_DISTANCE( "max_distance" );
const QString QgsSymbolLayerV2::EXPR_IGNORE_RINGS( "ignore_rings" );
const QString QgsSymbolLayerV2::EXPR_SVG_FILE( "svgFile" );
const QString QgsSymbolLayerV2::EXPR_SVG_FILL_COLOR( "svgFillColor" );
const QString QgsSymbolLayerV2::EXPR_SVG_OUTLINE_COLOR( "svgOutlineColor" );
const QString QgsSymbolLayerV2::EXPR_SVG_OUTLINE_WIDTH( "svgOutlineWidth" );
const QString QgsSymbolLayerV2::EXPR_LINEWIDTH( "linewidth" );
const QString QgsSymbolLayerV2::EXPR_DISTANCE_X( "distance_x" );
const QString QgsSymbolLayerV2::EXPR_DISTANCE_Y( "distance_y" );
const QString QgsSymbolLayerV2::EXPR_DISPLACEMENT_X( "displacement_x" );
const QString QgsSymbolLayerV2::EXPR_DISPLACEMENT_Y( "displacement_y" );
const QString QgsSymbolLayerV2::EXPR_FILE( "file" );
const QString QgsSymbolLayerV2::EXPR_ALPHA( "alpha" );
const QString QgsSymbolLayerV2::EXPR_CUSTOMDASH( "customdash" );
const QString QgsSymbolLayerV2::EXPR_LINE_STYLE( "line_style" );
const QString QgsSymbolLayerV2::EXPR_JOINSTYLE( "joinstyle" );
const QString QgsSymbolLayerV2::EXPR_CAPSTYLE( "capstyle" );
const QString QgsSymbolLayerV2::EXPR_PLACEMENT( "placement" );
const QString QgsSymbolLayerV2::EXPR_INTERVAL( "interval" );
const QString QgsSymbolLayerV2::EXPR_OFFSET_ALONG_LINE( "offset_along_line" );
const QString QgsSymbolLayerV2::EXPR_HORIZONTAL_ANCHOR_POINT( "horizontal_anchor_point" );
const QString QgsSymbolLayerV2::EXPR_VERTICAL_ANCHOR_POINT( "vertical_anchor_point" );

const QgsExpression* QgsSymbolLayerV2::dataDefinedProperty( const QString& property ) const
{
  Q_NOWARN_DEPRECATED_PUSH
  return expression( property );
  Q_NOWARN_DEPRECATED_POP
}

QgsDataDefined *QgsSymbolLayerV2::getDataDefinedProperty( const QString &property ) const
{
  if ( mDataDefinedProperties.isEmpty() )
    return 0;

  QMap< QString, QgsDataDefined* >::const_iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.constEnd() )
  {
    return it.value();
  }
  return 0;
}

QgsExpression* QgsSymbolLayerV2::expression( const QString& property ) const
{
  QgsDataDefined* dd = getDataDefinedProperty( property );
  return dd ? dd->expression() : 0;
}

QString QgsSymbolLayerV2::dataDefinedPropertyString( const QString& property ) const
{
  const QgsDataDefined* dd = getDataDefinedProperty( property );
  return dd ? dd->expressionString() : QString();
}

void QgsSymbolLayerV2::setDataDefinedProperty( const QString& property, const QString& expressionString )
{
  setDataDefinedProperty( property, new QgsDataDefined( expressionString ) );
}

void QgsSymbolLayerV2::setDataDefinedProperty( const QString &property, QgsDataDefined *dataDefined )
{
  removeDataDefinedProperty( property );
  mDataDefinedProperties.insert( property, dataDefined );
}

void QgsSymbolLayerV2::removeDataDefinedProperty( const QString& property )
{
  QMap< QString, QgsDataDefined* >::iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.end() )
  {
    delete( it.value() );
    mDataDefinedProperties.erase( it );
  }
}

void QgsSymbolLayerV2::removeDataDefinedProperties()
{
  QMap< QString, QgsDataDefined* >::iterator it = mDataDefinedProperties.begin();
  for ( ; it != mDataDefinedProperties.constEnd(); ++it )
  {
    delete( it.value() );
  }
  mDataDefinedProperties.clear();
}

bool QgsSymbolLayerV2::hasDataDefinedProperties() const
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

bool QgsSymbolLayerV2::hasDataDefinedProperty( const QString& property ) const
{
  if ( mDataDefinedProperties.isEmpty() )
    return false;

  QgsDataDefined* dd = getDataDefinedProperty( property );
  return dd && dd->isActive();
}

QVariant QgsSymbolLayerV2::evaluateDataDefinedProperty( const QString &property, const QgsFeature* feature, const QVariant& defaultVal, bool *ok ) const
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
      QgsExpressionContext context = feature ? QgsExpressionContextUtils::createFeatureBasedContext( *feature, QgsFields() ) : QgsExpressionContext();
      QVariant result = dd->expression()->evaluate( &context );
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
  else if ( feature && !dd->field().isEmpty() && !mFields.isEmpty() )
  {
    int attributeIndex = mFields.fieldNameIndex( dd->field() );
    if ( attributeIndex >= 0 )
    {
      if ( ok )
        *ok = true;
      return feature->attribute( attributeIndex );
    }
  }
  return defaultVal;
}

QVariant QgsSymbolLayerV2::evaluateDataDefinedProperty( const QString& property, const QgsSymbolV2RenderContext& context, const QVariant& defaultVal, bool* ok ) const
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
    int attributeIndex = mFields.fieldNameIndex( dd->field() );
    if ( attributeIndex >= 0 )
    {
      if ( ok )
        *ok = true;
      return context.feature()->attribute( attributeIndex );
    }
  }
  return defaultVal;
}

bool QgsSymbolLayerV2::writeDxf( QgsDxfExport& e,
                                 double mmMapUnitScaleFactor,
                                 const QString& layerName,
                                 const QgsSymbolV2RenderContext* context,
                                 const QgsFeature* f,
                                 const QPointF& shift ) const
{
  Q_UNUSED( e );
  Q_UNUSED( mmMapUnitScaleFactor );
  Q_UNUSED( layerName );
  Q_UNUSED( context );
  Q_UNUSED( f );
  Q_UNUSED( shift );
  return false;
}

double QgsSymbolLayerV2::dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( e );
  Q_UNUSED( context );
  return 1.0;
}

double QgsSymbolLayerV2::dxfOffset( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( e );
  Q_UNUSED( context );
  return 0.0;
}

QColor QgsSymbolLayerV2::dxfColor( const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( context );
  return color();
}

QVector<qreal> QgsSymbolLayerV2::dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const
{
  Q_UNUSED( unit );
  return QVector<qreal>();
}

Qt::PenStyle QgsSymbolLayerV2::dxfPenStyle() const
{
  return Qt::SolidLine;
}

QColor QgsSymbolLayerV2::dxfBrushColor( const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( context );
  return color();
}

Qt::BrushStyle QgsSymbolLayerV2::dxfBrushStyle() const
{
  return Qt::NoBrush;
}

QgsPaintEffect *QgsSymbolLayerV2::paintEffect() const
{
  return mPaintEffect;
}

void QgsSymbolLayerV2::setPaintEffect( QgsPaintEffect *effect )
{
  delete mPaintEffect;
  mPaintEffect = effect;
}

QgsSymbolLayerV2::QgsSymbolLayerV2( QgsSymbolV2::SymbolType type, bool locked )
    : mType( type )
    , mLocked( locked )
    , mRenderingPass( 0 )
    , mPaintEffect( 0 )
{
  mPaintEffect = QgsPaintEffectRegistry::defaultStack();
  mPaintEffect->setEnabled( false );
}

void QgsSymbolLayerV2::prepareExpressions( const QgsFields* fields, double scale )
{
  QMap< QString, QgsDataDefined* >::iterator it = mDataDefinedProperties.begin();
  for ( ; it != mDataDefinedProperties.end(); ++it )
  {
    if ( it.value() )
    {
      QMap<QString, QVariant> params;
      if ( scale > 0 )
      {
        params.insert( "scale", scale );
      }
      it.value()->setExpressionParams( params );

      if ( fields )
      {
        it.value()->prepareExpression( QgsExpressionContextUtils::createFeatureBasedContext( QgsFeature(), *fields ) );
      }
      else
      {
        it.value()->prepareExpression();
      }
    }
  }

  if ( fields )
  {
    //QgsFields is implicitly shared, so it's cheap to make a copy
    mFields = *fields;
  }
}

void QgsSymbolLayerV2::prepareExpressions( const QgsSymbolV2RenderContext& context )
{
  QMap< QString, QgsDataDefined* >::iterator it = mDataDefinedProperties.begin();
  for ( ; it != mDataDefinedProperties.end(); ++it )
  {
    if ( it.value() )
    {
      QMap<QString, QVariant> params;
      if ( context.renderContext().rendererScale() > 0 )
      {
        params.insert( "scale", context.renderContext().rendererScale() );
      }
      it.value()->setExpressionParams( params );
      it.value()->prepareExpression( context.renderContext().expressionContext() );
    }
  }

  if ( context.fields() )
  {
    //QgsFields is implicitly shared, so it's cheap to make a copy
    mFields = *context.fields();
  }
}

QgsSymbolLayerV2::~QgsSymbolLayerV2()
{
  removeDataDefinedProperties();
  delete mPaintEffect;
}

QSet<QString> QgsSymbolLayerV2::usedAttributes() const
{
  QStringList columns;

  QMap< QString, QgsDataDefined* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() && ddIt.value()->isActive() )
    {
      columns.append( ddIt.value()->referencedColumns() );
    }
  }

  return columns.toSet();
}

void QgsSymbolLayerV2::saveDataDefinedProperties( QgsStringMap& stringMap ) const
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

void QgsSymbolLayerV2::restoreDataDefinedProperties( const QgsStringMap &stringMap )
{
  QgsStringMap::const_iterator propIt = stringMap.constBegin();
  for ( ; propIt != stringMap.constEnd(); ++propIt )
  {
    if ( propIt.key().endsWith( "_dd_expression" ) )
    {
      //found a data defined property

      //get data defined property name by stripping "_dd_expression" from property key
      QString propertyName = propIt.key().left( propIt.key().length() - 14 );

      QgsDataDefined* dd = QgsDataDefined::fromMap( stringMap, propertyName );
      if ( dd )
        setDataDefinedProperty( propertyName, dd );
    }
    else if ( propIt.key().endsWith( "_expression" ) )
    {
      //old style data defined property, upgrade

      //get data defined property name by stripping "_expression" from property key
      QString propertyName = propIt.key().left( propIt.key().length() - 11 );

      setDataDefinedProperty( propertyName, new QgsDataDefined( propIt.value() ) );
    }
  }
}

void QgsSymbolLayerV2::copyDataDefinedProperties( QgsSymbolLayerV2* destLayer ) const
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

void QgsSymbolLayerV2::copyPaintEffect( QgsSymbolLayerV2 *destLayer ) const
{
  if ( !destLayer || !mPaintEffect )
    return;

  destLayer->setPaintEffect( mPaintEffect->clone() );
}

QgsMarkerSymbolLayerV2::QgsMarkerSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Marker, locked )
    , mAngle( 0 )
    , mLineAngle( 0 )
    , mSize( 2.0 )
    , mSizeUnit( QgsSymbolV2::MM )
    , mOffsetUnit( QgsSymbolV2::MM )
    , mScaleMethod( QgsSymbolV2::ScaleArea )
    , mHorizontalAnchorPoint( HCenter )
    , mVerticalAnchorPoint( VCenter )
{

}

QgsLineSymbolLayerV2::QgsLineSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Line, locked )
    , mWidth( 0 )
    , mWidthUnit( QgsSymbolV2::MM )
    , mOffset( 0 )
    , mOffsetUnit( QgsSymbolV2::MM )
{
}

QgsFillSymbolLayerV2::QgsFillSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Fill, locked )
    , mAngle( 0.0 )
{
}

void QgsMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsMarkerSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  startRender( context );
  renderPoint( QPointF( size.width() / 2, size.height() / 2 ), context );
  stopRender( context );
}

void QgsMarkerSymbolLayerV2::markerOffset( const QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY ) const
{
  markerOffset( context, mSize, mSize, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayerV2::markerOffset( const QgsSymbolV2RenderContext& context, double width, double height, double& offsetX, double& offsetY ) const
{
  markerOffset( context, width, height, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayerV2::markerOffset( const QgsSymbolV2RenderContext& context, double width, double height,
    QgsSymbolV2::OutputUnit widthUnit, QgsSymbolV2::OutputUnit heightUnit,
    double& offsetX, double& offsetY, const QgsMapUnitScale& widthMapUnitScale, const QgsMapUnitScale& heightMapUnitScale ) const
{
  offsetX = mOffset.x();
  offsetY = mOffset.y();

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET ) )
  {
    QPointF offset = QgsSymbolLayerV2Utils::decodePoint( evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET, context ).toString() );
    offsetX = offset.x();
    offsetY = offset.y();
  }

  offsetX *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale );
  offsetY *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale );

  HorizontalAnchorPoint horizontalAnchorPoint = mHorizontalAnchorPoint;
  VerticalAnchorPoint verticalAnchorPoint = mVerticalAnchorPoint;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_HORIZONTAL_ANCHOR_POINT ) )
  {
    horizontalAnchorPoint = decodeHorizontalAnchorPoint( evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_HORIZONTAL_ANCHOR_POINT , context ).toString() );
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_VERTICAL_ANCHOR_POINT ) )
  {
    verticalAnchorPoint = decodeVerticalAnchorPoint( evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_VERTICAL_ANCHOR_POINT, context ).toString() );
  }

  //correct horizontal position according to anchor point
  if ( horizontalAnchorPoint == HCenter && verticalAnchorPoint == VCenter )
  {
    return;
  }

  double anchorPointCorrectionX = width * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), widthUnit, widthMapUnitScale ) / 2.0;
  double anchorPointCorrectionY = height * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), heightUnit, heightMapUnitScale ) / 2.0;
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

QPointF QgsMarkerSymbolLayerV2::_rotatedOffset( const QPointF& offset, double angle )
{
  angle = DEG2RAD( angle );
  double c = cos( angle ), s = sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

QgsMarkerSymbolLayerV2::HorizontalAnchorPoint QgsMarkerSymbolLayerV2::decodeHorizontalAnchorPoint( const QString& str )
{
  if ( str.compare( "left", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Left;
  }
  else if ( str.compare( "right", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Right;
  }
  else
  {
    return QgsMarkerSymbolLayerV2::HCenter;
  }
}

QgsMarkerSymbolLayerV2::VerticalAnchorPoint QgsMarkerSymbolLayerV2::decodeVerticalAnchorPoint( const QString& str )
{
  if ( str.compare( "top", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Top;
  }
  else if ( str.compare( "bottom", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Bottom;
  }
  else
  {
    return QgsMarkerSymbolLayerV2::VCenter;
  }
}

void QgsMarkerSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mSizeUnit = unit;
  mOffsetUnit = unit;
}

QgsSymbolV2::OutputUnit QgsMarkerSymbolLayerV2::outputUnit() const
{
  if ( mOffsetUnit != mSizeUnit )
  {
    return QgsSymbolV2::Mixed;
  }
  return mOffsetUnit;
}

void QgsMarkerSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mSizeMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsMarkerSymbolLayerV2::mapUnitScale() const
{
  if ( mSizeMapUnitScale == mOffsetMapUnitScale )
  {
    return mSizeMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsLineSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsLineSymbolLayerV2::outputUnit() const
{
  return mWidthUnit;
}

void QgsLineSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale& scale )
{
  mWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsLineSymbolLayerV2::mapUnitScale() const
{
  return mWidthMapUnitScale;
}


void QgsLineSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  QPolygonF points;
  // we're adding 0.5 to get rid of blurred preview:
  // drawing antialiased lines of width 1 at (x,0)-(x,100) creates 2px line
  points << QPointF( 0, int( size.height() / 2 ) + 0.5 ) << QPointF( size.width(), int( size.height() / 2 ) + 0.5 );

  startRender( context );
  renderPolyline( points, context );
  stopRender( context );
}

void QgsLineSymbolLayerV2::renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  renderPolyline( points, context );
  if ( rings )
  {
    foreach ( const QPolygonF& ring, *rings )
      renderPolyline( ring, context );
  }
}

double QgsLineSymbolLayerV2::dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( context );
  return width() * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), widthUnit(), e.mapUnits() );
}


void QgsFillSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width(), size.height() ) );
  startRender( context );
  renderPolygon( poly, NULL, context );
  stopRender( context );
}

void QgsFillSymbolLayerV2::_renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
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

  if ( rings == NULL )
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

    QList<QPolygonF>::const_iterator it = rings->constBegin();
    for ( ; it != rings->constEnd(); ++it )
    {
      QPolygonF ring = *it;
      path.addPolygon( ring );
    }

    p->drawPath( path );
  }
}

void QgsMarkerSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  QDomElement symbolizerElem = doc.createElement( "se:PointSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  writeSldMarker( doc, symbolizerElem, props );
}


