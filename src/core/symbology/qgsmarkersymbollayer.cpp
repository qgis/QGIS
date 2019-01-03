/***************************************************************************
 qgsmarkersymbollayer.cpp
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

#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"

#include "qgsdxfexport.h"
#include "qgsdxfpaintdevice.h"
#include "qgsexpression.h"
#include "qgsimagecache.h"
#include "qgsimageoperation.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgssvgcache.h"
#include "qgsunittypes.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

static void _fixQPictureDPI( QPainter *p )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  p->scale( static_cast< double >( qt_defaultDpiX() ) / p->device()->logicalDpiX(),
            static_cast< double >( qt_defaultDpiY() ) / p->device()->logicalDpiY() );
}


//////


//
// QgsSimpleMarkerSymbolLayerBase
//

QList<QgsSimpleMarkerSymbolLayerBase::Shape> QgsSimpleMarkerSymbolLayerBase::availableShapes()
{
  QList< Shape > shapes;
  shapes << Square
         << Diamond
         << Pentagon
         << Hexagon
         << Triangle
         << EquilateralTriangle
         << Star
         << Arrow
         << Circle
         << Cross
         << CrossFill
         << Cross2
         << Line
         << ArrowHead
         << ArrowHeadFilled
         << SemiCircle
         << ThirdCircle
         << QuarterCircle
         << QuarterSquare
         << HalfSquare
         << DiagonalHalfSquare
         << RightHalfTriangle
         << LeftHalfTriangle;
  return shapes;
}

QgsSimpleMarkerSymbolLayerBase::QgsSimpleMarkerSymbolLayerBase( QgsSimpleMarkerSymbolLayerBase::Shape shape, double size, double angle, QgsSymbol::ScaleMethod scaleMethod )
  : mShape( shape )
{
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  mSizeUnit = QgsUnitTypes::RenderMillimeters;
  mOffsetUnit = QgsUnitTypes::RenderMillimeters;
}

bool QgsSimpleMarkerSymbolLayerBase::shapeIsFilled( QgsSimpleMarkerSymbolLayerBase::Shape shape )
{
  switch ( shape )
  {
    case Square:
    case Diamond:
    case Pentagon:
    case Hexagon:
    case Triangle:
    case EquilateralTriangle:
    case Star:
    case Arrow:
    case Circle:
    case CrossFill:
    case ArrowHeadFilled:
    case SemiCircle:
    case ThirdCircle:
    case QuarterCircle:
    case QuarterSquare:
    case HalfSquare:
    case DiagonalHalfSquare:
    case RightHalfTriangle:
    case LeftHalfTriangle:
      return true;

    case Cross:
    case Cross2:
    case Line:
    case ArrowHead:
      return false;
  }
  return true;
}

void QgsSimpleMarkerSymbolLayerBase::startRender( QgsSymbolRenderContext &context )
{
  bool hasDataDefinedRotation = context.renderHints() & QgsSymbol::DynamicRotation
                                || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
  bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  // use either QPolygonF or QPainterPath for drawing
  if ( !prepareMarkerShape( mShape ) ) // drawing as a polygon
  {
    prepareMarkerPath( mShape ); // drawing as a painter path
  }

  QTransform transform;

  // scale the shape (if the size is not going to be modified)
  if ( !hasDataDefinedSize )
  {
    double scaledSize = context.renderContext().convertToPainterUnits( mSize, mSizeUnit, mSizeMapUnitScale );
    double half = scaledSize / 2.0;
    transform.scale( half, half );
  }

  // rotate if the rotation is not going to be changed during the rendering
  if ( !hasDataDefinedRotation && !qgsDoubleNear( mAngle, 0.0 ) )
  {
    transform.rotate( mAngle );
  }

  if ( !mPolygon.isEmpty() )
    mPolygon = transform.map( mPolygon );
  else
    mPath = transform.map( mPath );

  QgsMarkerSymbolLayer::startRender( context );
}

void QgsSimpleMarkerSymbolLayerBase::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );
}

void QgsSimpleMarkerSymbolLayerBase::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  //making changes here? Don't forget to also update ::bounds if the changes affect the bounding box
  //of the rendered point!

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, scaledSize, hasDataDefinedRotation, offset, angle );

  //data defined shape?
  bool createdNewPath = false;
  bool ok = true;
  Shape symbol = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( encodeShape( symbol ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      Shape decoded = decodeShape( exprVal.toString(), &ok );
      if ( ok )
      {
        symbol = decoded;

        if ( !prepareMarkerShape( symbol ) ) // drawing as a polygon
        {
          prepareMarkerPath( symbol ); // drawing as a painter path
        }
        createdNewPath = true;
      }
    }
    else
    {
      symbol = mShape;
    }
  }

  QTransform transform;

  // move to the desired position
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );

  // resize if necessary
  if ( hasDataDefinedSize || createdNewPath )
  {
    double s = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );
    double half = s / 2.0;
    transform.scale( half, half );
  }

  if ( !qgsDoubleNear( angle, 0.0 ) && ( hasDataDefinedRotation || createdNewPath ) )
    transform.rotate( angle );

  //need to pass: symbol, polygon, path

  QPolygonF polygon;
  QPainterPath path;
  if ( !mPolygon.isEmpty() )
  {
    polygon = transform.map( mPolygon );
  }
  else
  {
    path = transform.map( mPath );
  }
  draw( context, symbol, polygon, path );
}

QRectF QgsSimpleMarkerSymbolLayerBase::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, scaledSize, hasDataDefinedRotation, offset, angle );

  scaledSize = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );

  QTransform transform;

  // move to the desired position
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  return transform.mapRect( QRectF( -scaledSize / 2.0,
                                    -scaledSize / 2.0,
                                    scaledSize,
                                    scaledSize ) );
}

QgsSimpleMarkerSymbolLayerBase::Shape QgsSimpleMarkerSymbolLayerBase::decodeShape( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  QString cleaned = name.toLower().trimmed();

  if ( cleaned == QLatin1String( "square" ) || cleaned == QLatin1String( "rectangle" ) )
    return Square;
  else if ( cleaned == QLatin1String( "diamond" ) )
    return Diamond;
  else if ( cleaned == QLatin1String( "pentagon" ) )
    return Pentagon;
  else if ( cleaned == QLatin1String( "hexagon" ) )
    return Hexagon;
  else if ( cleaned == QLatin1String( "triangle" ) )
    return Triangle;
  else if ( cleaned == QLatin1String( "equilateral_triangle" ) )
    return EquilateralTriangle;
  else if ( cleaned == QLatin1String( "star" ) || cleaned == QLatin1String( "regular_star" ) )
    return Star;
  else if ( cleaned == QLatin1String( "arrow" ) )
    return Arrow;
  else if ( cleaned == QLatin1String( "circle" ) )
    return Circle;
  else if ( cleaned == QLatin1String( "cross" ) )
    return Cross;
  else if ( cleaned == QLatin1String( "cross_fill" ) )
    return CrossFill;
  else if ( cleaned == QLatin1String( "cross2" ) || cleaned == QLatin1String( "x" ) )
    return Cross2;
  else if ( cleaned == QLatin1String( "line" ) )
    return Line;
  else if ( cleaned == QLatin1String( "arrowhead" ) )
    return ArrowHead;
  else if ( cleaned == QLatin1String( "filled_arrowhead" ) )
    return ArrowHeadFilled;
  else if ( cleaned == QLatin1String( "semi_circle" ) )
    return SemiCircle;
  else if ( cleaned == QLatin1String( "third_circle" ) )
    return ThirdCircle;
  else if ( cleaned == QLatin1String( "quarter_circle" ) )
    return QuarterCircle;
  else if ( cleaned == QLatin1String( "quarter_square" ) )
    return QuarterSquare;
  else if ( cleaned == QLatin1String( "half_square" ) )
    return HalfSquare;
  else if ( cleaned == QLatin1String( "diagonal_half_square" ) )
    return DiagonalHalfSquare;
  else if ( cleaned == QLatin1String( "right_half_triangle" ) )
    return RightHalfTriangle;
  else if ( cleaned == QLatin1String( "left_half_triangle" ) )
    return LeftHalfTriangle;

  if ( ok )
    *ok = false;
  return Circle;
}

QString QgsSimpleMarkerSymbolLayerBase::encodeShape( QgsSimpleMarkerSymbolLayerBase::Shape shape )
{
  switch ( shape )
  {
    case Square:
      return QStringLiteral( "square" );
    case QuarterSquare:
      return QStringLiteral( "quarter_square" );
    case HalfSquare:
      return QStringLiteral( "half_square" );
    case DiagonalHalfSquare:
      return QStringLiteral( "diagonal_half_square" );
    case Diamond:
      return QStringLiteral( "diamond" );
    case Pentagon:
      return QStringLiteral( "pentagon" );
    case Hexagon:
      return QStringLiteral( "hexagon" );
    case Triangle:
      return QStringLiteral( "triangle" );
    case EquilateralTriangle:
      return QStringLiteral( "equilateral_triangle" );
    case LeftHalfTriangle:
      return QStringLiteral( "left_half_triangle" );
    case RightHalfTriangle:
      return QStringLiteral( "right_half_triangle" );
    case Star:
      return QStringLiteral( "star" );
    case Arrow:
      return QStringLiteral( "arrow" );
    case ArrowHeadFilled:
      return QStringLiteral( "filled_arrowhead" );
    case CrossFill:
      return QStringLiteral( "cross_fill" );
    case Circle:
      return QStringLiteral( "circle" );
    case Cross:
      return QStringLiteral( "cross" );
    case Cross2:
      return QStringLiteral( "cross2" );
    case Line:
      return QStringLiteral( "line" );
    case ArrowHead:
      return QStringLiteral( "arrowhead" );
    case SemiCircle:
      return QStringLiteral( "semi_circle" );
    case ThirdCircle:
      return QStringLiteral( "third_circle" );
    case QuarterCircle:
      return QStringLiteral( "quarter_circle" );
  }
  return QString();
}

bool QgsSimpleMarkerSymbolLayerBase::prepareMarkerShape( QgsSimpleMarkerSymbolLayerBase::Shape shape )
{
  return shapeToPolygon( shape, mPolygon );
}

bool QgsSimpleMarkerSymbolLayerBase::shapeToPolygon( QgsSimpleMarkerSymbolLayerBase::Shape shape, QPolygonF &polygon ) const
{
  polygon.clear();

  switch ( shape )
  {
    case Square:
      polygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 1, 1 ) ) );
      return true;

    case QuarterSquare:
      polygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 0, 0 ) ) );
      return true;

    case HalfSquare:
      polygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 0, 1 ) ) );
      return true;

    case DiagonalHalfSquare:
      polygon << QPointF( -1, -1 ) << QPointF( 1, 1 ) << QPointF( -1, 1 ) << QPointF( -1, -1 );
      return true;

    case Diamond:
      polygon << QPointF( -1, 0 ) << QPointF( 0, 1 )
              << QPointF( 1, 0 ) << QPointF( 0, -1 ) << QPointF( -1, 0 );
      return true;

    case Pentagon:
      /* angular-representation of hardcoded values used
      polygon << QPointF( std::sin( DEG2RAD( 288.0 ) ), - std::cos( DEG2RAD( 288.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 216.0 ) ), - std::cos( DEG2RAD( 216.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 144.0 ) ), - std::cos( DEG2RAD( 144.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 72.0 ) ), - std::cos( DEG2RAD( 72.0 ) ) )
      << QPointF( 0, -1 ); */
      polygon << QPointF( -0.9511, -0.3090 )
              << QPointF( -0.5878, 0.8090 )
              << QPointF( 0.5878, 0.8090 )
              << QPointF( 0.9511, -0.3090 )
              << QPointF( 0, -1 )
              << QPointF( -0.9511, -0.3090 );
      return true;

    case Hexagon:
      /* angular-representation of hardcoded values used
      polygon << QPointF( std::sin( DEG2RAD( 300.0 ) ), - std::cos( DEG2RAD( 300.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 240.0 ) ), - std::cos( DEG2RAD( 240.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 180.0 ) ), - std::cos( DEG2RAD( 180.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 120.0 ) ), - std::cos( DEG2RAD( 120.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 60.0 ) ), - std::cos( DEG2RAD( 60.0 ) ) )
      << QPointF( 0, -1 ); */
      polygon << QPointF( -0.8660, -0.5 )
              << QPointF( -0.8660, 0.5 )
              << QPointF( 0, 1 )
              << QPointF( 0.8660, 0.5 )
              << QPointF( 0.8660, -0.5 )
              << QPointF( 0, -1 )
              << QPointF( -0.8660, -0.5 );
      return true;

    case Triangle:
      polygon << QPointF( -1, 1 ) << QPointF( 1, 1 ) << QPointF( 0, -1 ) << QPointF( -1, 1 );
      return true;

    case EquilateralTriangle:
      /* angular-representation of hardcoded values used
      polygon << QPointF( std::sin( DEG2RAD( 240.0 ) ), - std::cos( DEG2RAD( 240.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 120.0 ) ), - std::cos( DEG2RAD( 120.0 ) ) )
      << QPointF( 0, -1 ); */
      polygon << QPointF( -0.8660, 0.5 )
              << QPointF( 0.8660, 0.5 )
              << QPointF( 0, -1 )
              << QPointF( -0.8660, 0.5 );
      return true;

    case LeftHalfTriangle:
      polygon << QPointF( 0, 1 ) << QPointF( 1, 1 ) << QPointF( 0, -1 ) << QPointF( 0, 1 );
      return true;

    case RightHalfTriangle:
      polygon << QPointF( -1, 1 ) << QPointF( 0, 1 ) << QPointF( 0, -1 ) << QPointF( -1, 1 );
      return true;

    case Star:
    {
      double inner_r = std::cos( DEG2RAD( 72.0 ) ) / std::cos( DEG2RAD( 36.0 ) );

      polygon << QPointF( inner_r * std::sin( DEG2RAD( 324.0 ) ), - inner_r * std::cos( DEG2RAD( 324.0 ) ) )  // 324
              << QPointF( std::sin( DEG2RAD( 288.0 ) ), - std::cos( DEG2RAD( 288 ) ) )    // 288
              << QPointF( inner_r * std::sin( DEG2RAD( 252.0 ) ), - inner_r * std::cos( DEG2RAD( 252.0 ) ) )   // 252
              << QPointF( std::sin( DEG2RAD( 216.0 ) ), - std::cos( DEG2RAD( 216.0 ) ) )   // 216
              << QPointF( 0, inner_r )         // 180
              << QPointF( std::sin( DEG2RAD( 144.0 ) ), - std::cos( DEG2RAD( 144.0 ) ) )   // 144
              << QPointF( inner_r * std::sin( DEG2RAD( 108.0 ) ), - inner_r * std::cos( DEG2RAD( 108.0 ) ) )   // 108
              << QPointF( std::sin( DEG2RAD( 72.0 ) ), - std::cos( DEG2RAD( 72.0 ) ) )    //  72
              << QPointF( inner_r * std::sin( DEG2RAD( 36.0 ) ), - inner_r * std::cos( DEG2RAD( 36.0 ) ) )   //  36
              << QPointF( 0, -1 )
              << QPointF( inner_r * std::sin( DEG2RAD( 324.0 ) ), - inner_r * std::cos( DEG2RAD( 324.0 ) ) );  // 324;          //   0
      return true;
    }

    case Arrow:
      polygon << QPointF( 0, -1 )
              << QPointF( 0.5,  -0.5 )
              << QPointF( 0.25, -0.5 )
              << QPointF( 0.25,  1 )
              << QPointF( -0.25,  1 )
              << QPointF( -0.25, -0.5 )
              << QPointF( -0.5,  -0.5 )
              << QPointF( 0, -1 );
      return true;

    case ArrowHeadFilled:
      polygon << QPointF( 0, 0 ) << QPointF( -1, 1 ) << QPointF( -1, -1 ) << QPointF( 0, 0 );
      return true;

    case CrossFill:
      polygon << QPointF( -1, -0.2 )
              << QPointF( -1, -0.2 )
              << QPointF( -1, 0.2 )
              << QPointF( -0.2, 0.2 )
              << QPointF( -0.2, 1 )
              << QPointF( 0.2, 1 )
              << QPointF( 0.2, 0.2 )
              << QPointF( 1, 0.2 )
              << QPointF( 1, -0.2 )
              << QPointF( 0.2, -0.2 )
              << QPointF( 0.2, -1 )
              << QPointF( -0.2, -1 )
              << QPointF( -0.2, -0.2 )
              << QPointF( -1, -0.2 );
      return true;

    case Circle:
    case Cross:
    case Cross2:
    case Line:
    case ArrowHead:
    case SemiCircle:
    case ThirdCircle:
    case QuarterCircle:
      return false;
  }

  return false;
}

bool QgsSimpleMarkerSymbolLayerBase::prepareMarkerPath( QgsSimpleMarkerSymbolLayerBase::Shape symbol )
{
  mPath = QPainterPath();

  switch ( symbol )
  {
    case Circle:

      mPath.addEllipse( QRectF( -1, -1, 2, 2 ) ); // x,y,w,h
      return true;

    case SemiCircle:
      mPath.arcTo( -1, -1, 2, 2, 0, 180 );
      mPath.lineTo( 0, 0 );
      return true;

    case ThirdCircle:
      mPath.arcTo( -1, -1, 2, 2, 90, 120 );
      mPath.lineTo( 0, 0 );
      return true;

    case QuarterCircle:
      mPath.arcTo( -1, -1, 2, 2, 90, 90 );
      mPath.lineTo( 0, 0 );
      return true;

    case Cross:
      mPath.moveTo( -1, 0 );
      mPath.lineTo( 1, 0 ); // horizontal
      mPath.moveTo( 0, -1 );
      mPath.lineTo( 0, 1 ); // vertical
      return true;

    case Cross2:
      mPath.moveTo( -1, -1 );
      mPath.lineTo( 1, 1 );
      mPath.moveTo( 1, -1 );
      mPath.lineTo( -1, 1 );
      return true;

    case Line:
      mPath.moveTo( 0, -1 );
      mPath.lineTo( 0, 1 ); // vertical line
      return true;

    case ArrowHead:
      mPath.moveTo( -1, -1 );
      mPath.lineTo( 0, 0 );
      mPath.lineTo( -1, 1 );
      return true;

    case Square:
    case QuarterSquare:
    case HalfSquare:
    case DiagonalHalfSquare:
    case Diamond:
    case Pentagon:
    case Hexagon:
    case Triangle:
    case EquilateralTriangle:
    case LeftHalfTriangle:
    case RightHalfTriangle:
    case Star:
    case Arrow:
    case ArrowHeadFilled:
    case CrossFill:
      return false;
  }
  return false;
}

double QgsSimpleMarkerSymbolLayerBase::calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const
{
  double scaledSize = mSize;

  hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );
  bool ok = true;
  if ( hasDataDefinedSize )
  {
    context.setOriginalValueVariable( mSize );
    scaledSize = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertySize, context.renderContext().expressionContext(),
                 mSize, &ok );
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbol::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case QgsSymbol::ScaleDiameter:
        break;
    }
  }

  return scaledSize;
}

void QgsSimpleMarkerSymbolLayerBase::calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const
{
  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, scaledSize, scaledSize, offsetX, offsetY );
  offset = QPointF( offsetX, offsetY );

  //angle
  bool ok = true;
  angle = mAngle + mLineAngle;
  bool usingDataDefinedRotation = false;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle, &ok ) + mLineAngle;
    usingDataDefinedRotation = ok;
  }

  hasDataDefinedRotation = context.renderHints() & QgsSymbol::DynamicRotation || usingDataDefinedRotation;
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature *f = context.feature();
    if ( f )
    {
      if ( f->hasGeometry() && f->geometry().type() == QgsWkbTypes::PointGeometry )
      {
        const QgsMapToPixel &m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}


//
// QgsSimpleMarkerSymbolLayer
//

QgsSimpleMarkerSymbolLayer::QgsSimpleMarkerSymbolLayer( QgsSimpleMarkerSymbolLayerBase::Shape shape, double size, double angle, QgsSymbol::ScaleMethod scaleMethod, const QColor &color, const QColor &strokeColor, Qt::PenJoinStyle penJoinStyle )
  : QgsSimpleMarkerSymbolLayerBase( shape, size, angle, scaleMethod )
  , mStrokeColor( strokeColor )
  , mPenJoinStyle( penJoinStyle )
{
  mColor = color;
}

QgsSymbolLayer *QgsSimpleMarkerSymbolLayer::create( const QgsStringMap &props )
{
  Shape shape = Circle;
  QColor color = DEFAULT_SIMPLEMARKER_COLOR;
  QColor strokeColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR;
  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;
  QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "name" ) ) )
  {
    shape = decodeShape( props[QStringLiteral( "name" )] );
  }
  if ( props.contains( QStringLiteral( "color" ) ) )
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )] );
  if ( props.contains( QStringLiteral( "color_border" ) ) )
  {
    //pre 2.5 projects use "color_border"
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color_border" )] );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )] );
  }
  else if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )] );
  }
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
  {
    penJoinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )] );
  }
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )] );

  QgsSimpleMarkerSymbolLayer *m = new QgsSimpleMarkerSymbolLayer( shape, size, angle, scaleMethod, color, strokeColor, penJoinStyle );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )] ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )] ) );

  if ( props.contains( QStringLiteral( "outline_style" ) ) )
  {
    m->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "outline_style" )] ) );
  }
  else if ( props.contains( QStringLiteral( "line_style" ) ) )
  {
    m->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "line_style" )] ) );
  }
  if ( props.contains( QStringLiteral( "outline_width" ) ) )
  {
    m->setStrokeWidth( props[QStringLiteral( "outline_width" )].toDouble() );
  }
  else if ( props.contains( QStringLiteral( "line_width" ) ) )
  {
    m->setStrokeWidth( props[QStringLiteral( "line_width" )].toDouble() );
  }
  if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    m->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "outline_width_map_unit_scale" )] ) );
  }

  if ( props.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( props[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  }
  if ( props.contains( QStringLiteral( "vertical_anchor_point" ) ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( props[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );
  }

  m->restoreOldDataDefinedProperties( props );

  return m;
}


QString QgsSimpleMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "SimpleMarker" );
}

void QgsSimpleMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QgsSimpleMarkerSymbolLayerBase::startRender( context );

  QColor brushColor = mColor;
  QColor penColor = mStrokeColor;

  brushColor.setAlphaF( mColor.alphaF() * context.opacity() );
  penColor.setAlphaF( mStrokeColor.alphaF() * context.opacity() );

  mBrush = QBrush( brushColor );
  mPen = QPen( penColor );
  mPen.setStyle( mStrokeStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );

  QColor selBrushColor = context.renderContext().selectionColor();
  QColor selPenColor = selBrushColor == mColor ? selBrushColor : mStrokeColor;
  if ( context.opacity() < 1 )
  {
    selBrushColor.setAlphaF( context.opacity() );
    selPenColor.setAlphaF( context.opacity() );
  }
  mSelBrush = QBrush( selBrushColor );
  mSelPen = QPen( selPenColor );
  mSelPen.setStyle( mStrokeStyle );
  mSelPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbol::DynamicRotation || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
  bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  // use caching only when:
  // - size, rotation, shape, color, stroke color is not data-defined
  // - drawing to screen (not printer)
  mUsingCache = !hasDataDefinedRotation && !hasDataDefinedSize && !context.renderContext().forceVectorOutput()
                && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor )
                && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle )
                && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle );

  if ( !shapeIsFilled( mShape ) )
  {
    // some markers can't be drawn as a polygon (circle, cross)
    // For these set the selected stroke color to the selected color
    mSelPen.setColor( selBrushColor );
  }


  if ( mUsingCache )
  {
    if ( !prepareCache( context ) )
    {
      mUsingCache = false;
    }
  }
  else
  {
    mCache = QImage();
    mSelCache = QImage();
  }
}


bool QgsSimpleMarkerSymbolLayer::prepareCache( QgsSymbolRenderContext &context )
{
  double scaledSize = context.renderContext().convertToPainterUnits( mSize, mSizeUnit, mSizeMapUnitScale );

  // calculate necessary image size for the cache
  double pw = static_cast< int >( std::round( ( ( qgsDoubleNear( mPen.widthF(), 0.0 ) ? 1 : mPen.widthF() * 4 ) + 1 ) ) ) / 2 * 2; // make even (round up); handle cosmetic pen
  int imageSize = ( static_cast< int >( scaledSize ) + pw ) / 2 * 2 + 1; //  make image width, height odd; account for pen width
  double center = imageSize / 2.0;

  if ( imageSize > MAXIMUM_CACHE_WIDTH )
  {
    return false;
  }

  mCache = QImage( QSize( imageSize, imageSize ), QImage::Format_ARGB32_Premultiplied );
  mCache.fill( 0 );

  bool needsBrush = shapeIsFilled( mShape );

  QPainter p;
  p.begin( &mCache );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( needsBrush ? mBrush : Qt::NoBrush );
  p.setPen( mPen );
  p.translate( QPointF( center, center ) );
  drawMarker( &p, context );
  p.end();

  // Construct the selected version of the Cache

  QColor selColor = context.renderContext().selectionColor();

  mSelCache = QImage( QSize( imageSize, imageSize ), QImage::Format_ARGB32_Premultiplied );
  mSelCache.fill( 0 );

  p.begin( &mSelCache );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( needsBrush ? mSelBrush : Qt::NoBrush );
  p.setPen( mSelPen );
  p.translate( QPointF( center, center ) );
  drawMarker( &p, context );
  p.end();

  // Check that the selected version is different.  If not, then re-render,
  // filling the background with the selection color and using the normal
  // colors for the symbol .. could be ugly!

  if ( mSelCache == mCache )
  {
    p.begin( &mSelCache );
    p.setRenderHint( QPainter::Antialiasing );
    p.fillRect( 0, 0, imageSize, imageSize, selColor );
    p.setBrush( needsBrush ? mBrush : Qt::NoBrush );
    p.setPen( mPen );
    p.translate( QPointF( center, center ) );
    drawMarker( &p, context );
    p.end();
  }

  return true;
}

void QgsSimpleMarkerSymbolLayer::draw( QgsSymbolRenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path )
{
  //making changes here? Don't forget to also update ::bounds if the changes affect the bounding box
  //of the rendered point!

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  bool ok = true;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    QColor c = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor, &ok );
    if ( ok )
      mBrush.setColor( c );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    QColor c = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mColor, &ok );
    if ( ok )
    {
      mPen.setColor( c );
      mSelPen.setColor( c );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), 0, &ok );
    if ( ok )
    {
      mPen.setWidthF( context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );
      mSelPen.setWidthF( context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    QString strokeStyle = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( strokeStyle ) );
      mSelPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( strokeStyle ) );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
      mSelPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
    }
  }

  if ( shapeIsFilled( shape ) )
  {
    p->setBrush( context.selected() ? mSelBrush : mBrush );
  }
  else
  {
    p->setBrush( Qt::NoBrush );
  }
  p->setPen( context.selected() ? mSelPen : mPen );

  if ( !polygon.isEmpty() )
    p->drawPolygon( polygon );
  else
    p->drawPath( path );
}

void QgsSimpleMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  //making changes here? Don't forget to also update ::bounds if the changes affect the bounding box
  //of the rendered point!

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  if ( mUsingCache )
  {
    QImage &img = context.selected() ? mSelCache : mCache;
    double s = img.width();

    bool hasDataDefinedSize = false;
    double scaledSize = calculateSize( context, hasDataDefinedSize );

    bool hasDataDefinedRotation = false;
    QPointF offset;
    double angle = 0;
    calculateOffsetAndRotation( context, scaledSize, hasDataDefinedRotation, offset, angle );

    p->drawImage( QRectF( point.x() - s / 2.0 + offset.x(),
                          point.y() - s / 2.0 + offset.y(),
                          s, s ), img );
  }
  else
  {
    QgsSimpleMarkerSymbolLayerBase::renderPoint( point, context );
  }
}

QgsStringMap QgsSimpleMarkerSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "name" )] = encodeShape( mShape );
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "outline_color" )] = QgsSymbolLayerUtils::encodeColor( mStrokeColor );
  map[QStringLiteral( "size" )] = QString::number( mSize );
  map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( mSizeUnit );
  map[QStringLiteral( "size_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "scale_method" )] = QgsSymbolLayerUtils::encodeScaleMethod( mScaleMethod );
  map[QStringLiteral( "outline_style" )] = QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle );
  map[QStringLiteral( "outline_width" )] = QString::number( mStrokeWidth );
  map[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( mStrokeWidthUnit );
  map[QStringLiteral( "outline_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale );
  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "horizontal_anchor_point" )] = QString::number( mHorizontalAnchorPoint );
  map[QStringLiteral( "vertical_anchor_point" )] = QString::number( mVerticalAnchorPoint );
  return map;
}

QgsSimpleMarkerSymbolLayer *QgsSimpleMarkerSymbolLayer::clone() const
{
  QgsSimpleMarkerSymbolLayer *m = new QgsSimpleMarkerSymbolLayer( mShape, mSize, mAngle, mScaleMethod, mColor, mStrokeColor, mPenJoinStyle );
  m->setOffset( mOffset );
  m->setSizeUnit( mSizeUnit );
  m->setSizeMapUnitScale( mSizeMapUnitScale );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setStrokeStyle( mStrokeStyle );
  m->setStrokeWidth( mStrokeWidth );
  m->setStrokeWidthUnit( mStrokeWidthUnit );
  m->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );
  m->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  m->setVerticalAnchorPoint( mVerticalAnchorPoint );
  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsSimpleMarkerSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mStrokeWidth, mStrokeWidthUnit, props );
  double size = QgsSymbolLayerUtils::rescaleUom( mSize, mSizeUnit, props );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, encodeShape( mShape ), mColor, mStrokeColor, mStrokeStyle, strokeWidth, size );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ) ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );
}

QString QgsSimpleMarkerSymbolLayer::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  Q_UNUSED( mmScaleFactor );
  Q_UNUSED( mapUnitScaleFactor );
#if 0
  QString ogrType = "3"; //default is circle
  if ( mName == "square" )
  {
    ogrType = "5";
  }
  else if ( mName == "triangle" )
  {
    ogrType = "7";
  }
  else if ( mName == "star" )
  {
    ogrType = "9";
  }
  else if ( mName == "circle" )
  {
    ogrType = "3";
  }
  else if ( mName == "cross" )
  {
    ogrType = "0";
  }
  else if ( mName == "x" || mName == "cross2" )
  {
    ogrType = "1";
  }
  else if ( mName == "line" )
  {
    ogrType = "10";
  }

  QString ogrString;
  ogrString.append( "SYMBOL(" );
  ogrString.append( "id:" );
  ogrString.append( '\"' );
  ogrString.append( "ogr-sym-" );
  ogrString.append( ogrType );
  ogrString.append( '\"' );
  ogrString.append( ",c:" );
  ogrString.append( mColor.name() );
  ogrString.append( ",o:" );
  ogrString.append( mStrokeColor.name() );
  ogrString.append( QString( ",s:%1mm" ).arg( mSize ) );
  ogrString.append( ')' );
  return ogrString;
#endif //0

  QString ogrString;
  ogrString.append( "PEN(" );
  ogrString.append( "c:" );
  ogrString.append( mColor.name() );
  ogrString.append( ",w:" );
  ogrString.append( QString::number( mSize ) );
  ogrString.append( "mm" );
  ogrString.append( ")" );
  return ogrString;
}

QgsSymbolLayer *QgsSimpleMarkerSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  QString name = QStringLiteral( "square" );
  QColor color, strokeColor;
  double strokeWidth, size;
  Qt::PenStyle strokeStyle;

  if ( !QgsSymbolLayerUtils::wellKnownMarkerFromSld( graphicElem, name, color, strokeColor, strokeStyle, strokeWidth, size ) )
    return nullptr;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerUtils::displacementFromSldElement( graphicElem, offset );

  Shape shape = decodeShape( name );

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );
  offset.setX( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.x() ) );
  offset.setY( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.y() ) );

  QgsSimpleMarkerSymbolLayer *m = new QgsSimpleMarkerSymbolLayer( shape, size );
  m->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  m->setColor( color );
  m->setStrokeColor( strokeColor );
  m->setAngle( angle );
  m->setOffset( offset );
  m->setStrokeStyle( strokeStyle );
  m->setStrokeWidth( strokeWidth );
  return m;
}

void QgsSimpleMarkerSymbolLayer::drawMarker( QPainter *p, QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );

  if ( mPolygon.count() != 0 )
  {
    p->drawPolygon( mPolygon );
  }
  else
  {
    p->drawPath( mPath );
  }
}

bool QgsSimpleMarkerSymbolLayer::writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift ) const
{
  Q_UNUSED( mmMapUnitScaleFactor );

  //data defined size?
  double size = mSize;

  bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  //data defined size
  bool ok = true;
  if ( hasDataDefinedSize )
  {
    size = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertySize, context.renderContext().expressionContext(), mSize, &ok );

    if ( ok )
    {
      switch ( mScaleMethod )
      {
        case QgsSymbol::ScaleArea:
          size = std::sqrt( size );
          break;
        case QgsSymbol::ScaleDiameter:
          break;
      }
    }

    size *= e.mapUnitScaleFactor( e.symbologyScale(), mSizeUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  }
  if ( mSizeUnit == QgsUnitTypes::RenderMapUnits )
  {
    e.clipValueToMapUnitScale( size, mSizeMapUnitScale, context.renderContext().scaleFactor() );
  }
  double halfSize = size / 2.0;

  //strokeWidth
  double strokeWidth = mStrokeWidth;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  strokeWidth *= e.mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  if ( mSizeUnit == QgsUnitTypes::RenderMapUnits )
  {
    e.clipValueToMapUnitScale( strokeWidth, mStrokeWidthMapUnitScale, context.renderContext().scaleFactor() );
  }

  //color
  QColor pc = mPen.color();
  QColor bc = mBrush.color();
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    bc = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), bc );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    pc = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), pc );
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );
  offsetX *= context.renderContext().mapToPixel().mapUnitsPerPixel();
  offsetY *= context.renderContext().mapToPixel().mapUnitsPerPixel();


  QPointF off( offsetX, offsetY );

  //angle
  double angle = mAngle + mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
  }

  Shape shape = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( encodeShape( shape ) );
    QString shapeName = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      shape = decodeShape( shapeName, &ok );
      if ( !ok )
        shape = mShape;
    }
  }

  if ( angle )
    off = _rotatedOffset( off, angle );

  off *= e.mapUnitScaleFactor( e.symbologyScale(), mSizeUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );

  QTransform t;
  t.translate( shift.x() + off.x(), shift.y() - off.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    t.rotate( angle );

  QPolygonF polygon;
  if ( shapeToPolygon( shape, polygon ) )
  {
    t.scale( halfSize, -halfSize );

    polygon = t.map( polygon );

    QgsPointSequence p;
    p.reserve( polygon.size() );
    for ( int i = 0; i < polygon.size(); i++ )
    {
      p << QgsPoint( polygon[i] );
    }

    if ( mBrush.style() != Qt::NoBrush )
      e.writePolygon( QgsRingSequence() << p, layerName, QStringLiteral( "SOLID" ), bc );
    if ( mPen.style() != Qt::NoPen )
      e.writePolyline( p, layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
  }
  else if ( shape == Circle )
  {
    shift += QPointF( off.x(), -off.y() );
    if ( mBrush.style() != Qt::NoBrush )
      e.writeFilledCircle( layerName, bc, QgsPoint( shift ), halfSize );
    if ( mPen.style() != Qt::NoPen )
      e.writeCircle( layerName, pc, QgsPoint( shift ), halfSize, QStringLiteral( "CONTINUOUS" ), strokeWidth );
  }
  else if ( shape == Line )
  {
    QPointF pt1 = t.map( QPointF( 0, -halfSize ) );
    QPointF pt2 = t.map( QPointF( 0, halfSize ) );

    if ( mPen.style() != Qt::NoPen )
      e.writeLine( QgsPoint( pt1 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
  }
  else if ( shape == Cross )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      QPointF pt1 = t.map( QPointF( -halfSize, 0 ) );
      QPointF pt2 = t.map( QPointF( halfSize, 0 ) );
      QPointF pt3 = t.map( QPointF( 0, -halfSize ) );
      QPointF pt4 = t.map( QPointF( 0, halfSize ) );

      e.writeLine( QgsPoint( pt1 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
      e.writeLine( QgsPoint( pt3 ), QgsPoint( pt4 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
    }
  }
  else if ( shape == Cross2 )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      QPointF pt1 = t.map( QPointF( -halfSize, -halfSize ) );
      QPointF pt2 = t.map( QPointF( halfSize, halfSize ) );
      QPointF pt3 = t.map( QPointF( halfSize, -halfSize ) );
      QPointF pt4 = t.map( QPointF( -halfSize, halfSize ) );

      e.writeLine( QgsPoint( pt1 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
      e.writeLine( QgsPoint( pt3 ), QgsPoint( pt4 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
    }
  }
  else if ( shape == ArrowHead )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      QPointF pt1 = t.map( QPointF( -halfSize, halfSize ) );
      QPointF pt2 = t.map( QPointF( 0, 0 ) );
      QPointF pt3 = t.map( QPointF( -halfSize, -halfSize ) );

      e.writeLine( QgsPoint( pt1 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
      e.writeLine( QgsPoint( pt3 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Unsupported dxf marker name %1" ).arg( encodeShape( shape ) ) );
    return false;
  }

  return true;
}


void QgsSimpleMarkerSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  mStrokeWidthUnit = unit;
}

QgsUnitTypes::RenderUnit QgsSimpleMarkerSymbolLayer::outputUnit() const
{
  if ( QgsMarkerSymbolLayer::outputUnit() == mStrokeWidthUnit )
  {
    return mStrokeWidthUnit;
  }
  return QgsUnitTypes::RenderUnknownUnit;
}

void QgsSimpleMarkerSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayer::setMapUnitScale( scale );
  mStrokeWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsSimpleMarkerSymbolLayer::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayer::mapUnitScale() == mStrokeWidthMapUnitScale )
  {
    return mStrokeWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QRectF QgsSimpleMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  QRectF symbolBounds = QgsSimpleMarkerSymbolLayerBase::bounds( point, context );

  // need to account for stroke width
  double penWidth = 0.0;
  bool ok = true;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth, &ok );
    if ( ok )
    {
      penWidth = context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    QString strokeStyle = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok && strokeStyle == QLatin1String( "no" ) )
    {
      penWidth = 0.0;
    }
  }
  //antialiasing, add 1 pixel
  penWidth += 1;

  //extend bounds by pen width / 2.0
  symbolBounds.adjust( -penWidth / 2.0, -penWidth / 2.0,
                       penWidth / 2.0, penWidth / 2.0 );

  return symbolBounds;
}

void QgsSimpleMarkerSymbolLayer::setColor( const QColor &color )
{
  if ( shapeIsFilled( mShape ) )
  {
    setFillColor( color );
  }
  else
  {
    setStrokeColor( color );
  }
}

QColor QgsSimpleMarkerSymbolLayer::color() const
{
  if ( shapeIsFilled( mShape ) )
  {
    return fillColor();
  }
  else
  {
    return strokeColor();
  }
}




//
// QgsFilledMarkerSymbolLayer
//

QgsFilledMarkerSymbolLayer::QgsFilledMarkerSymbolLayer( QgsSimpleMarkerSymbolLayerBase::Shape shape, double size, double angle, QgsSymbol::ScaleMethod scaleMethod )
  : QgsSimpleMarkerSymbolLayerBase( shape, size, angle, scaleMethod )
{
  mFill.reset( static_cast<QgsFillSymbol *>( QgsFillSymbol::createSimple( QgsStringMap() ) ) );
}

QgsSymbolLayer *QgsFilledMarkerSymbolLayer::create( const QgsStringMap &props )
{
  QString name = DEFAULT_SIMPLEMARKER_NAME;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;
  QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "name" ) ) )
    name = props[QStringLiteral( "name" )];
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )] );

  QgsFilledMarkerSymbolLayer *m = new QgsFilledMarkerSymbolLayer( decodeShape( name ), size, angle, scaleMethod );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )] ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( props[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  }
  if ( props.contains( QStringLiteral( "vertical_anchor_point" ) ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( props[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );
  }

  m->setSubSymbol( QgsFillSymbol::createSimple( props ) );

  m->restoreOldDataDefinedProperties( props );

  return m;
}

QString QgsFilledMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "FilledMarker" );
}

void QgsFilledMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  if ( mFill )
  {
    mFill->startRender( context.renderContext(), context.fields() );
  }

  QgsSimpleMarkerSymbolLayerBase::startRender( context );
}

void QgsFilledMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mFill )
  {
    mFill->stopRender( context.renderContext() );
  }
}

QgsStringMap QgsFilledMarkerSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "name" )] = encodeShape( mShape );
  map[QStringLiteral( "size" )] = QString::number( mSize );
  map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( mSizeUnit );
  map[QStringLiteral( "size_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "scale_method" )] = QgsSymbolLayerUtils::encodeScaleMethod( mScaleMethod );
  map[QStringLiteral( "horizontal_anchor_point" )] = QString::number( mHorizontalAnchorPoint );
  map[QStringLiteral( "vertical_anchor_point" )] = QString::number( mVerticalAnchorPoint );

  if ( mFill )
  {
    map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mFill->color() );
  }
  return map;
}

QgsFilledMarkerSymbolLayer *QgsFilledMarkerSymbolLayer::clone() const
{
  QgsFilledMarkerSymbolLayer *m = static_cast< QgsFilledMarkerSymbolLayer * >( QgsFilledMarkerSymbolLayer::create( properties() ) );
  copyPaintEffect( m );
  copyDataDefinedProperties( m );
  m->setSubSymbol( mFill->clone() );
  return m;
}

QgsSymbol *QgsFilledMarkerSymbolLayer::subSymbol()
{
  return mFill.get();
}

bool QgsFilledMarkerSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol && symbol->type() == QgsSymbol::Fill )
  {
    mFill.reset( static_cast<QgsFillSymbol *>( symbol ) );
    return true;
  }
  else
  {
    delete symbol;
    return false;
  }
}

double QgsFilledMarkerSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  if ( mFill )
  {
    return QgsSymbolLayerUtils::estimateMaxSymbolBleed( mFill.get(), context );
  }
  return 0;
}

QSet<QString> QgsFilledMarkerSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsSimpleMarkerSymbolLayerBase::usedAttributes( context );
  if ( mFill )
    attr.unite( mFill->usedAttributes( context ) );
  return attr;
}

void QgsFilledMarkerSymbolLayer::setColor( const QColor &c )
{
  mColor = c;
  if ( mFill )
    mFill->setColor( c );
}

QColor QgsFilledMarkerSymbolLayer::color() const
{
  return mFill ?  mFill->color() : mColor;
}

void QgsFilledMarkerSymbolLayer::draw( QgsSymbolRenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path )
{
  //making changes here? Don't forget to also update ::bounds if the changes affect the bounding box
  //of the rendered point!

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  if ( shapeIsFilled( shape ) )
  {
    p->setBrush( Qt::red );
  }
  else
  {
    p->setBrush( Qt::NoBrush );
  }
  p->setPen( Qt::black );

  if ( !polygon.isEmpty() )
  {
    mFill->renderPolygon( polygon, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
  }
  else
  {
    QPolygonF poly = path.toFillPolygon();
    mFill->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
  }


}


//////////


QgsSvgMarkerSymbolLayer::QgsSvgMarkerSymbolLayer( const QString &path, double size, double angle, QgsSymbol::ScaleMethod scaleMethod )
{
  mPath = path;
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  mStrokeWidth = 0.2;
  mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
  mColor = QColor( 35, 35, 35 );
  mStrokeColor = QColor( 35, 35, 35 );
  updateDefaultAspectRatio();
}


QgsSymbolLayer *QgsSvgMarkerSymbolLayer::create( const QgsStringMap &props )
{
  QString name;
  double size = DEFAULT_SVGMARKER_SIZE;
  double angle = DEFAULT_SVGMARKER_ANGLE;
  QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "name" ) ) )
    name = props[QStringLiteral( "name" )];
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )] );

  QgsSvgMarkerSymbolLayer *m = new QgsSvgMarkerSymbolLayer( name, size, angle, scaleMethod );

  //we only check the svg default parameters if necessary, since it could be expensive
  if ( !props.contains( QStringLiteral( "fill" ) ) && !props.contains( QStringLiteral( "color" ) ) && !props.contains( QStringLiteral( "outline" ) ) &&
       !props.contains( QStringLiteral( "outline_color" ) ) && !props.contains( QStringLiteral( "outline-width" ) ) && !props.contains( QStringLiteral( "outline_width" ) ) )
  {
    QColor fillColor, strokeColor;
    double fillOpacity = 1.0;
    double strokeOpacity = 1.0;
    double strokeWidth;
    bool hasFillParam = false, hasFillOpacityParam = false, hasStrokeParam = false, hasStrokeWidthParam = false, hasStrokeOpacityParam = false;
    bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultStrokeColor = false, hasDefaultStrokeWidth = false, hasDefaultStrokeOpacity = false;
    QgsApplication::svgCache()->containsParams( name, hasFillParam, hasDefaultFillColor, fillColor,
        hasFillOpacityParam, hasDefaultFillOpacity, fillOpacity,
        hasStrokeParam, hasDefaultStrokeColor, strokeColor,
        hasStrokeWidthParam, hasDefaultStrokeWidth, strokeWidth,
        hasStrokeOpacityParam, hasDefaultStrokeOpacity, strokeOpacity );
    if ( hasDefaultFillColor )
    {
      m->setFillColor( fillColor );
    }
    if ( hasDefaultFillOpacity )
    {
      QColor c = m->fillColor();
      c.setAlphaF( fillOpacity );
      m->setFillColor( c );
    }
    if ( hasDefaultStrokeColor )
    {
      m->setStrokeColor( strokeColor );
    }
    if ( hasDefaultStrokeWidth )
    {
      m->setStrokeWidth( strokeWidth );
    }
    if ( hasDefaultStrokeOpacity )
    {
      QColor c = m->strokeColor();
      c.setAlphaF( strokeOpacity );
      m->setStrokeColor( c );
    }
  }

  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )] ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "fixedAspectRatio" ) ) )
    m->setFixedAspectRatio( props[QStringLiteral( "fixedAspectRatio" )].toDouble() );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "fill" ) ) )
  {
    //pre 2.5 projects used "fill"
    m->setFillColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "fill" )] ) );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    m->setFillColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )] ) );
  }
  if ( props.contains( QStringLiteral( "outline" ) ) )
  {
    //pre 2.5 projects used "outline"
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline" )] ) );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )] ) );
  }
  else if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )] ) );
  }

  if ( props.contains( QStringLiteral( "outline-width" ) ) )
  {
    //pre 2.5 projects used "outline-width"
    m->setStrokeWidth( props[QStringLiteral( "outline-width" )].toDouble() );
  }
  else if ( props.contains( QStringLiteral( "outline_width" ) ) )
  {
    m->setStrokeWidth( props[QStringLiteral( "outline_width" )].toDouble() );
  }
  else if ( props.contains( QStringLiteral( "line_width" ) ) )
  {
    m->setStrokeWidth( props[QStringLiteral( "line_width" )].toDouble() );
  }

  if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )] ) );
  }
  else if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
    m->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "outline_width_map_unit_scale" )] ) );

  if ( props.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( props[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  }
  if ( props.contains( QStringLiteral( "vertical_anchor_point" ) ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( props[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );
  }

  m->restoreOldDataDefinedProperties( props );

  m->updateDefaultAspectRatio();

  return m;
}

void QgsSvgMarkerSymbolLayer::resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  QgsStringMap::iterator it = properties.find( QStringLiteral( "name" ) );
  if ( it != properties.end() )
  {
    if ( saving )
      it.value() = QgsSymbolLayerUtils::svgSymbolPathToName( it.value(), pathResolver );
    else
      it.value() = QgsSymbolLayerUtils::svgSymbolNameToPath( it.value(), pathResolver );
  }
}

void QgsSvgMarkerSymbolLayer::setPath( const QString &path )
{
  mPath = path;
  QColor defaultFillColor, defaultStrokeColor;
  double strokeWidth, fillOpacity, strokeOpacity;
  bool hasFillParam = false, hasFillOpacityParam = false, hasStrokeParam = false, hasStrokeWidthParam = false, hasStrokeOpacityParam = false;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultStrokeColor = false, hasDefaultStrokeWidth = false, hasDefaultStrokeOpacity = false;
  QgsApplication::svgCache()->containsParams( path, hasFillParam, hasDefaultFillColor, defaultFillColor,
      hasFillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      hasStrokeParam, hasDefaultStrokeColor, defaultStrokeColor,
      hasStrokeWidthParam, hasDefaultStrokeWidth, strokeWidth,
      hasStrokeOpacityParam, hasDefaultStrokeOpacity, strokeOpacity );

  double newFillOpacity = hasFillOpacityParam ? fillColor().alphaF() : 1.0;
  double newStrokeOpacity = hasStrokeOpacityParam ? strokeColor().alphaF() : 1.0;

  if ( hasDefaultFillColor )
  {
    defaultFillColor.setAlphaF( newFillOpacity );
    setFillColor( defaultFillColor );
  }
  if ( hasDefaultFillOpacity )
  {
    QColor c = fillColor();
    c.setAlphaF( fillOpacity );
    setFillColor( c );
  }
  if ( hasDefaultStrokeColor )
  {
    defaultStrokeColor.setAlphaF( newStrokeOpacity );
    setStrokeColor( defaultStrokeColor );
  }
  if ( hasDefaultStrokeWidth )
  {
    setStrokeWidth( strokeWidth );
  }
  if ( hasDefaultStrokeOpacity )
  {
    QColor c = strokeColor();
    c.setAlphaF( strokeOpacity );
    setStrokeColor( c );
  }

  updateDefaultAspectRatio();
}

double QgsSvgMarkerSymbolLayer::updateDefaultAspectRatio()
{
  if ( mDefaultAspectRatio == 0.0 )
  {
    //size
    double size = mSize;
    //assume 88 dpi as standard value
    double widthScaleFactor = 3.465;
    QSizeF svgViewbox = QgsApplication::svgCache()->svgViewboxSize( mPath, size, mColor, mStrokeColor, mStrokeWidth, widthScaleFactor );
    // set default aspect ratio
    mDefaultAspectRatio = svgViewbox.isValid() ? svgViewbox.height() / svgViewbox.width() : 0.0;
  }
  return mDefaultAspectRatio;
}

bool QgsSvgMarkerSymbolLayer::setPreservedAspectRatio( bool par )
{
  bool aPreservedAspectRatio = preservedAspectRatio();
  if ( aPreservedAspectRatio && !par )
  {
    mFixedAspectRatio = mDefaultAspectRatio;
  }
  else if ( !aPreservedAspectRatio && par )
  {
    mFixedAspectRatio = 0.0;
  }
  return preservedAspectRatio();
}


QString QgsSvgMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "SvgMarker" );
}

void QgsSvgMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QgsMarkerSymbolLayer::startRender( context ); // get anchor point expressions
  Q_UNUSED( context );
}

void QgsSvgMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );
}

void QgsSvgMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
    return;

  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );
  double size = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( size ) < 1 || 10000.0 < size )
  {
    return;
  }

  p->save();

  bool hasDataDefinedAspectRatio = false;
  double aspectRatio = calculateAspectRatio( context, scaledSize, hasDataDefinedAspectRatio );

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledSize, outputOffset, angle );

  p->translate( point + outputOffset );

  bool rotated = !qgsDoubleNear( angle, 0 );
  if ( rotated )
    p->rotate( angle );

  QString path = mPath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mPath );
    path = QgsSymbolLayerUtils::svgSymbolNameToPath( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mPath ),
           context.renderContext().pathResolver() );
  }

  double strokeWidth = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  strokeWidth = context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );

  QColor fillColor = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    fillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
  }

  QColor strokeColor = mStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    strokeColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor );
  }

  bool fitsInCache = true;
  bool usePict = true;
  double hwRatio = 1.0;
  if ( !context.renderContext().forceVectorOutput() && !rotated )
  {
    QImage img = QgsApplication::svgCache()->svgAsImage( path, size, fillColor, strokeColor, strokeWidth,
                 context.renderContext().scaleFactor(), fitsInCache, aspectRatio );
    if ( fitsInCache && img.width() > 1 )
    {
      usePict = false;
      //consider transparency
      if ( !qgsDoubleNear( context.opacity(), 1.0 ) )
      {
        QImage transparentImage = img.copy();
        QgsSymbolLayerUtils::multiplyImageOpacity( &transparentImage, context.opacity() );
        p->drawImage( -transparentImage.width() / 2.0, -transparentImage.height() / 2.0, transparentImage );
        hwRatio = static_cast< double >( transparentImage.height() ) / static_cast< double >( transparentImage.width() );
      }
      else
      {
        p->drawImage( -img.width() / 2.0, -img.height() / 2.0, img );
        hwRatio = static_cast< double >( img.height() ) / static_cast< double >( img.width() );
      }
    }
  }

  if ( usePict || !fitsInCache )
  {
    p->setOpacity( context.opacity() );
    QPicture pct = QgsApplication::svgCache()->svgAsPicture( path, size, fillColor, strokeColor, strokeWidth,
                   context.renderContext().scaleFactor(), context.renderContext().forceVectorOutput(), aspectRatio );
    if ( pct.width() > 1 )
    {
      p->save();
      _fixQPictureDPI( p );
      p->drawPicture( 0, 0, pct );
      p->restore();
      hwRatio = static_cast< double >( pct.height() ) / static_cast< double >( pct.width() );
    }
  }

  if ( context.selected() )
  {
    QPen pen( context.renderContext().selectionColor() );
    double penWidth = context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
    if ( penWidth > size / 20 )
    {
      // keep the pen width from covering symbol
      penWidth = size / 20;
    }
    double penOffset = penWidth / 2;
    pen.setWidth( penWidth );
    p->setPen( pen );
    p->setBrush( Qt::NoBrush );
    double wSize = size + penOffset;
    double hSize = size * hwRatio + penOffset;
    p->drawRect( QRectF( -wSize / 2.0, -hSize / 2.0, wSize, hSize ) );
  }

  p->restore();

  if ( context.renderContext().flags() & QgsRenderContext::Antialiasing )
  {
    // workaround issue with nested QPictures forgetting antialiasing flag - see https://issues.qgis.org/issues/14960
    p->setRenderHint( QPainter::Antialiasing );
  }

}

double QgsSvgMarkerSymbolLayer::calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const
{
  double scaledSize = mSize;
  hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  bool ok = true;
  if ( hasDataDefinedSize )
  {
    context.setOriginalValueVariable( mSize );
    scaledSize = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertySize, context.renderContext().expressionContext(), mSize, &ok );
  }
  else
  {
    hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth );
    if ( hasDataDefinedSize )
    {
      context.setOriginalValueVariable( mSize );
      scaledSize = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mSize, &ok );
    }
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbol::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case QgsSymbol::ScaleDiameter:
        break;
    }
  }

  return scaledSize;
}

double QgsSvgMarkerSymbolLayer::calculateAspectRatio( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedAspectRatio ) const
{
  hasDataDefinedAspectRatio = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight );
  if ( !hasDataDefinedAspectRatio )
    return mFixedAspectRatio;

  if ( !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) && mFixedAspectRatio <= 0.0 )
    return 0.0;

  double scaledAspectRatio = mDefaultAspectRatio;
  if ( mFixedAspectRatio > 0.0 )
    scaledAspectRatio = mFixedAspectRatio;

  double defaultHeight = mSize * scaledAspectRatio;
  scaledAspectRatio = defaultHeight / scaledSize;

  bool ok = true;
  double scaledHeight = scaledSize * scaledAspectRatio;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) )
  {
    context.setOriginalValueVariable( defaultHeight );
    scaledHeight = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyHeight, context.renderContext().expressionContext(), defaultHeight, &ok );
  }

  if ( hasDataDefinedAspectRatio && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbol::ScaleArea:
        scaledHeight = sqrt( scaledHeight );
        break;
      case QgsSymbol::ScaleDiameter:
        break;
    }
  }

  scaledAspectRatio = scaledHeight / scaledSize;

  return scaledAspectRatio;
}

void QgsSvgMarkerSymbolLayer::calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledSize, QPointF &offset, double &angle ) const
{
  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, scaledSize, scaledSize, offsetX, offsetY );
  offset = QPointF( offsetX, offsetY );

  angle = mAngle + mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
  }

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbol::DynamicRotation || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature *f = context.feature();
    if ( f )
    {
      if ( f->hasGeometry() && f->geometry().type() == QgsWkbTypes::PointGeometry )
      {
        const QgsMapToPixel &m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}


QgsStringMap QgsSvgMarkerSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "name" )] = mPath;
  map[QStringLiteral( "size" )] = QString::number( mSize );
  map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( mSizeUnit );
  map[QStringLiteral( "size_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  map[QStringLiteral( "fixedAspectRatio" )] = QString::number( mFixedAspectRatio );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "scale_method" )] = QgsSymbolLayerUtils::encodeScaleMethod( mScaleMethod );
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "outline_color" )] = QgsSymbolLayerUtils::encodeColor( mStrokeColor );
  map[QStringLiteral( "outline_width" )] = QString::number( mStrokeWidth );
  map[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( mStrokeWidthUnit );
  map[QStringLiteral( "outline_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale );
  map[QStringLiteral( "horizontal_anchor_point" )] = QString::number( mHorizontalAnchorPoint );
  map[QStringLiteral( "vertical_anchor_point" )] = QString::number( mVerticalAnchorPoint );
  return map;
}

QgsSvgMarkerSymbolLayer *QgsSvgMarkerSymbolLayer::clone() const
{
  QgsSvgMarkerSymbolLayer *m = new QgsSvgMarkerSymbolLayer( mPath, mSize, mAngle );
  m->setFixedAspectRatio( mFixedAspectRatio );
  m->setColor( mColor );
  m->setStrokeColor( mStrokeColor );
  m->setStrokeWidth( mStrokeWidth );
  m->setStrokeWidthUnit( mStrokeWidthUnit );
  m->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );
  m->setOffset( mOffset );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setSizeUnit( mSizeUnit );
  m->setSizeMapUnitScale( mSizeMapUnitScale );
  m->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  m->setVerticalAnchorPoint( mVerticalAnchorPoint );
  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsSvgMarkerSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  mStrokeWidthUnit = unit;
}

QgsUnitTypes::RenderUnit QgsSvgMarkerSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsMarkerSymbolLayer::outputUnit();
  if ( unit != mStrokeWidthUnit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

void QgsSvgMarkerSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayer::setMapUnitScale( scale );
  mStrokeWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsSvgMarkerSymbolLayer::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayer::mapUnitScale() == mStrokeWidthMapUnitScale )
  {
    return mStrokeWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsSvgMarkerSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  // encode a parametric SVG reference
  double size = QgsSymbolLayerUtils::rescaleUom( mSize, mSizeUnit, props );
  double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mStrokeWidth, mStrokeWidthUnit, props );
  QgsSymbolLayerUtils::parametricSvgToSld( doc, graphicElem, mPath, mColor, size, mStrokeColor, strokeWidth );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ) ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }

  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );
}

QgsSymbolLayer *QgsSvgMarkerSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  QString path, mimeType;
  QColor fillColor;
  double size;

  if ( !QgsSymbolLayerUtils::externalGraphicFromSld( graphicElem, path, mimeType, fillColor, size ) )
    return nullptr;

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );

  if ( mimeType != QLatin1String( "image/svg+xml" ) )
    return nullptr;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerUtils::displacementFromSldElement( graphicElem, offset );

  QgsSvgMarkerSymbolLayer *m = new QgsSvgMarkerSymbolLayer( path, size );
  m->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  m->setFillColor( fillColor );
  //m->setStrokeColor( strokeColor );
  //m->setStrokeWidth( strokeWidth );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

bool QgsSvgMarkerSymbolLayer::writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift ) const
{
  Q_UNUSED( layerName );
  Q_UNUSED( shift ); //todo...

  //size
  double size = mSize;

  bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  bool ok = true;
  if ( hasDataDefinedSize )
  {
    context.setOriginalValueVariable( mSize );
    size = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertySize, context.renderContext().expressionContext(), mSize, &ok );
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbol::ScaleArea:
        size = std::sqrt( size );
        break;
      case QgsSymbol::ScaleDiameter:
        break;
    }
  }

  if ( mSizeUnit == QgsUnitTypes::RenderMillimeters )
  {
    size *= mmMapUnitScaleFactor;
  }

  double halfSize = size / 2.0;

  //offset, angle
  QPointF offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    QString offsetString = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
      offset = QgsSymbolLayerUtils::decodePoint( offsetString );
  }
  double offsetX = offset.x();
  double offsetY = offset.y();

  QPointF outputOffset( offsetX, offsetY );

  double angle = mAngle + mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
  }
  //angle = -angle; //rotation in Qt is counterclockwise
  if ( angle )
    outputOffset = _rotatedOffset( outputOffset, angle );

  outputOffset *= e.mapUnitScaleFactor( e.symbologyScale(), mOffsetUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );

  QString path = mPath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mPath );
    path = QgsSymbolLayerUtils::svgSymbolNameToPath( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mPath ),
           context.renderContext().pathResolver() );
  }

  double strokeWidth = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  strokeWidth  *= e.mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );

  QColor fillColor = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    fillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
  }

  QColor strokeColor = mStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    strokeColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor );
  }

  const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( path, size, fillColor, strokeColor, strokeWidth,
                                 context.renderContext().scaleFactor(), mFixedAspectRatio );

  //if current entry image is 0: cache image for entry
  // checks to see if image will fit into cache
  //update stats for memory usage
  QSvgRenderer r( svgContent );
  if ( !r.isValid() )
  {
    return false;
  }

  QgsDxfPaintDevice pd( &e );
  pd.setDrawingSize( QSizeF( r.defaultSize() ) );

  QPainter p;
  p.begin( &pd );
  if ( !qgsDoubleNear( angle, 0.0 ) )
  {
    p.translate( r.defaultSize().width() / 2.0, r.defaultSize().height() / 2.0 );
    p.rotate( angle );
    p.translate( -r.defaultSize().width() / 2.0, -r.defaultSize().height() / 2.0 );
  }
  pd.setShift( shift + QPointF( outputOffset.x(), -outputOffset.y() ) );
  pd.setOutputSize( QRectF( -halfSize, -halfSize, size, size ) );
  pd.setLayer( layerName );
  r.render( &p );
  p.end();
  return true;
}

QRectF QgsSvgMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );
  scaledSize = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( scaledSize ) < 1 || 10000.0 < scaledSize )
  {
    return QRectF();
  }

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledSize, outputOffset, angle );

  QString path = mPath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mPath );
    path = QgsSymbolLayerUtils::svgSymbolNameToPath( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mPath ),
           context.renderContext().pathResolver() );
  }

  double strokeWidth = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  strokeWidth = context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );

  //need to get colors to take advantage of cached SVGs
  QColor fillColor = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    fillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
  }

  QColor strokeColor = mStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    fillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor );
  }

  QSizeF svgViewbox = QgsApplication::svgCache()->svgViewboxSize( path, scaledSize, fillColor, strokeColor, strokeWidth,
                      context.renderContext().scaleFactor(), mFixedAspectRatio );

  double scaledHeight = svgViewbox.isValid() ? scaledSize * svgViewbox.height() / svgViewbox.width() : scaledSize;

  QMatrix transform;

  // move to the desired position
  transform.translate( point.x() + outputOffset.x(), point.y() + outputOffset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  //antialiasing
  strokeWidth += 1.0 / 2.0;

  QRectF symbolBounds = transform.mapRect( QRectF( -scaledSize / 2.0,
                        -scaledHeight / 2.0,
                        scaledSize,
                        scaledHeight ) );

  //extend bounds by pen width / 2.0
  symbolBounds.adjust( -strokeWidth / 2.0, -strokeWidth / 2.0,
                       strokeWidth / 2.0, strokeWidth / 2.0 );

  return symbolBounds;

}

//////////

QgsRasterMarkerSymbolLayer::QgsRasterMarkerSymbolLayer( const QString &path, double size, double angle, QgsSymbol::ScaleMethod scaleMethod )
  : mPath( path )
{
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  updateDefaultAspectRatio();
}


QgsSymbolLayer *QgsRasterMarkerSymbolLayer::create( const QgsStringMap &props )
{
  QString path;
  double size = DEFAULT_RASTERMARKER_SIZE;
  double angle = DEFAULT_RASTERMARKER_ANGLE;
  QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "imageFile" ) ) )
    path = props[QStringLiteral( "imageFile" )];
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )] );

  QgsRasterMarkerSymbolLayer *m = new QgsRasterMarkerSymbolLayer( path, size, angle, scaleMethod );

  if ( props.contains( QStringLiteral( "alpha" ) ) )
  {
    m->setOpacity( props[QStringLiteral( "alpha" )].toDouble() );
  }

  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )] ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "fixedAspectRatio" ) ) )
    m->setFixedAspectRatio( props[QStringLiteral( "fixedAspectRatio" )].toDouble() );

  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );

  if ( props.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( props[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  }
  if ( props.contains( QStringLiteral( "vertical_anchor_point" ) ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( props[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );
  }

  m->restoreOldDataDefinedProperties( props );
  m->updateDefaultAspectRatio();

  return m;
}

void QgsRasterMarkerSymbolLayer::resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  QgsStringMap::iterator it = properties.find( QStringLiteral( "name" ) );
  if ( it != properties.end() )
  {
    if ( saving )
      it.value() = QgsSymbolLayerUtils::svgSymbolPathToName( it.value(), pathResolver );
    else
      it.value() = QgsSymbolLayerUtils::svgSymbolNameToPath( it.value(), pathResolver );
  }
}

void QgsRasterMarkerSymbolLayer::setPath( const QString &path )
{
  mPath = path;
  updateDefaultAspectRatio();
}

bool QgsRasterMarkerSymbolLayer::setPreservedAspectRatio( bool par )
{
  bool aPreservedAspectRatio = preservedAspectRatio();
  if ( aPreservedAspectRatio && !par )
  {
    mFixedAspectRatio = mDefaultAspectRatio;
  }
  else if ( !aPreservedAspectRatio && par )
  {
    mFixedAspectRatio = 0.0;
  }
  return preservedAspectRatio();
}

double QgsRasterMarkerSymbolLayer::updateDefaultAspectRatio()
{
  if ( mDefaultAspectRatio == 0.0 )
  {
    QSize size = QgsApplication::imageCache()->originalSize( mPath );
    mDefaultAspectRatio = ( !size.isNull() && size.isValid() && size.width() > 0 ) ? static_cast< double >( size.height() ) / static_cast< double >( size.width() ) : 0.0;
  }
  return mDefaultAspectRatio;
}

QString QgsRasterMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "RasterMarker" );
}

void QgsRasterMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
    return;

  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );
  double width = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );
  bool hasDataDefinedAspectRatio = false;
  double aspectRatio = calculateAspectRatio( context, scaledSize, hasDataDefinedAspectRatio );
  double height = width * ( preservedAspectRatio() ? defaultAspectRatio() : aspectRatio );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( width ) < 1 || 10000.0 < width )
  {
    return;
  }

  QString path = mPath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mPath );
    path = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mPath );
    if ( preservedAspectRatio() && path != mPath )
    {
      QSize size = QgsApplication::imageCache()->originalSize( path );
      if ( !size.isNull() && size.isValid() && size.width() > 0 )
      {
        height = width * ( static_cast< double >( size.height() ) / static_cast< double >( size.width() ) );
      }
    }
  }

  if ( path.isEmpty() )
    return;

  p->save();

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledSize, scaledSize * ( height / width ), outputOffset, angle );

  p->translate( point + outputOffset );

  bool rotated = !qgsDoubleNear( angle, 0 );
  if ( rotated )
    p->rotate( angle );

  double opacity = mOpacity;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOpacity ) )
  {
    context.setOriginalValueVariable( mOpacity );
    opacity = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOpacity, context.renderContext().expressionContext(), opacity * 100 ) / 100.0;
  }
  opacity *= context.opacity();

  bool cached;
  QImage img = QgsApplication::imageCache()->pathAsImage( path, QSize( width, preservedAspectRatio() ? 0 : width * aspectRatio ), preservedAspectRatio(), opacity, cached );
  if ( !img.isNull() )
  {
    if ( context.selected() )
      QgsImageOperation::adjustHueSaturation( img, 1.0, context.renderContext().selectionColor(), 1.0 );

    p->drawImage( -img.width() / 2.0, -img.height() / 2.0, img );
  }

  p->restore();
}

double QgsRasterMarkerSymbolLayer::calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const
{
  double scaledSize = mSize;
  hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  bool ok = true;
  if ( hasDataDefinedSize )
  {
    context.setOriginalValueVariable( mSize );
    scaledSize = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertySize, context.renderContext().expressionContext(), mSize, &ok );
  }
  else
  {
    hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth );
    if ( hasDataDefinedSize )
    {
      context.setOriginalValueVariable( mSize );
      scaledSize = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mSize, &ok );
    }
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbol::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case QgsSymbol::ScaleDiameter:
        break;
    }
  }

  return scaledSize;
}

double QgsRasterMarkerSymbolLayer::calculateAspectRatio( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedAspectRatio ) const
{
  hasDataDefinedAspectRatio = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight );
  if ( !hasDataDefinedAspectRatio )
    return mFixedAspectRatio;

  if ( !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) && mFixedAspectRatio <= 0.0 )
    return 0.0;

  double scaledAspectRatio = mDefaultAspectRatio;
  if ( mFixedAspectRatio > 0.0 )
    scaledAspectRatio = mFixedAspectRatio;

  double defaultHeight = mSize * scaledAspectRatio;
  scaledAspectRatio = defaultHeight / scaledSize;

  bool ok = true;
  double scaledHeight = scaledSize * scaledAspectRatio;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) )
  {
    context.setOriginalValueVariable( defaultHeight );
    scaledHeight = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyHeight, context.renderContext().expressionContext(), defaultHeight, &ok );
  }

  if ( hasDataDefinedAspectRatio && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbol::ScaleArea:
        scaledHeight = sqrt( scaledHeight );
        break;
      case QgsSymbol::ScaleDiameter:
        break;
    }
  }

  scaledAspectRatio = scaledHeight / scaledSize;

  return scaledAspectRatio;
}

void QgsRasterMarkerSymbolLayer::calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledWidth, double scaledHeight, QPointF &offset, double &angle ) const
{
  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, scaledWidth, scaledHeight, offsetX, offsetY );
  offset = QPointF( offsetX, offsetY );

  angle = mAngle + mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
  }

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbol::DynamicRotation || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
  if ( hasDataDefinedRotation )
  {
    const QgsFeature *f = context.feature();
    if ( f )
    {
      if ( f->hasGeometry() && f->geometry().type() == QgsWkbTypes::PointGeometry )
      {
        const QgsMapToPixel &m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}


QgsStringMap QgsRasterMarkerSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "imageFile" )] = mPath;
  map[QStringLiteral( "size" )] = QString::number( mSize );
  map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( mSizeUnit );
  map[QStringLiteral( "size_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  map[QStringLiteral( "fixedAspectRatio" )] = QString::number( mFixedAspectRatio );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "alpha" )] = QString::number( mOpacity );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "scale_method" )] = QgsSymbolLayerUtils::encodeScaleMethod( mScaleMethod );
  map[QStringLiteral( "horizontal_anchor_point" )] = QString::number( mHorizontalAnchorPoint );
  map[QStringLiteral( "vertical_anchor_point" )] = QString::number( mVerticalAnchorPoint );
  return map;
}

QgsRasterMarkerSymbolLayer *QgsRasterMarkerSymbolLayer::clone() const
{
  QgsRasterMarkerSymbolLayer *m = new QgsRasterMarkerSymbolLayer( mPath, mSize, mAngle );
  m->setFixedAspectRatio( mFixedAspectRatio );
  m->setOpacity( mOpacity );
  m->setOffset( mOffset );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setSizeUnit( mSizeUnit );
  m->setSizeMapUnitScale( mSizeMapUnitScale );
  m->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  m->setVerticalAnchorPoint( mVerticalAnchorPoint );
  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsRasterMarkerSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayer::setMapUnitScale( scale );
}

QgsMapUnitScale QgsRasterMarkerSymbolLayer::mapUnitScale() const
{
  return QgsMarkerSymbolLayer::mapUnitScale();
}

QRectF QgsRasterMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );
  double width = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );
  bool hasDataDefinedAspectRatio = false;
  double aspectRatio = calculateAspectRatio( context, scaledSize, hasDataDefinedAspectRatio );
  double height = width * ( preservedAspectRatio() ? defaultAspectRatio() : aspectRatio );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( scaledSize ) < 1 || 10000.0 < scaledSize )
  {
    return QRectF();
  }

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledSize, scaledSize * ( height / width ), outputOffset, angle );

  QMatrix transform;

  // move to the desired position
  transform.translate( point.x() + outputOffset.x(), point.y() + outputOffset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  QRectF symbolBounds = transform.mapRect( QRectF( -width / 2.0,
                        -height / 2.0,
                        width,
                        height ) );

  return symbolBounds;
}

//////////

QgsFontMarkerSymbolLayer::QgsFontMarkerSymbolLayer( const QString &fontFamily, QChar chr, double pointSize, const QColor &color, double angle )
{
  mFontFamily = fontFamily;
  mChr = chr;
  mColor = color;
  mAngle = angle;
  mSize = pointSize;
  mOrigSize = pointSize;
  mSizeUnit = QgsUnitTypes::RenderMillimeters;
  mOffset = QPointF( 0, 0 );
  mOffsetUnit = QgsUnitTypes::RenderMillimeters;
  mStrokeColor = DEFAULT_FONTMARKER_BORDERCOLOR;
  mStrokeWidth = 0.0;
  mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
  mPenJoinStyle = DEFAULT_FONTMARKER_JOINSTYLE;
}

QgsFontMarkerSymbolLayer::~QgsFontMarkerSymbolLayer()
{
  delete mFontMetrics;
}

QgsSymbolLayer *QgsFontMarkerSymbolLayer::create( const QgsStringMap &props )
{
  QString fontFamily = DEFAULT_FONTMARKER_FONT;
  QChar chr = DEFAULT_FONTMARKER_CHR;
  double pointSize = DEFAULT_FONTMARKER_SIZE;
  QColor color = DEFAULT_FONTMARKER_COLOR;
  double angle = DEFAULT_FONTMARKER_ANGLE;

  if ( props.contains( QStringLiteral( "font" ) ) )
    fontFamily = props[QStringLiteral( "font" )];
  if ( props.contains( QStringLiteral( "chr" ) ) && props[QStringLiteral( "chr" )].length() > 0 )
    chr = props[QStringLiteral( "chr" )].at( 0 );
  if ( props.contains( QStringLiteral( "size" ) ) )
    pointSize = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "color" ) ) )
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )] );
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();

  QgsFontMarkerSymbolLayer *m = new QgsFontMarkerSymbolLayer( fontFamily, chr, pointSize, color, angle );

  if ( props.contains( QStringLiteral( "outline_color" ) ) )
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )] ) );
  if ( props.contains( QStringLiteral( "outline_width" ) ) )
    m->setStrokeWidth( props[QStringLiteral( "outline_width" )].toDouble() );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )] ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )] ) );
  if ( props.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
    m->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "outline_width_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
    m->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )] ) );
  if ( props.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( props[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  if ( props.contains( QStringLiteral( "vertical_anchor_point" ) ) )
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( props[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );

  m->restoreOldDataDefinedProperties( props );

  return m;
}

QString QgsFontMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "FontMarker" );
}

void QgsFontMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QColor brushColor = mColor;
  QColor penColor = mStrokeColor;

  brushColor.setAlphaF( mColor.alphaF() * context.opacity() );
  penColor.setAlphaF( mStrokeColor.alphaF() * context.opacity() );

  mBrush = QBrush( brushColor );
  mPen = QPen( penColor );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );

  mFont = QFont( mFontFamily );
  mFont.setPixelSize( context.renderContext().convertToPainterUnits( mSize, mSizeUnit, mSizeMapUnitScale ) );
  delete mFontMetrics;
  mFontMetrics = new QFontMetrics( mFont );
  mChrWidth = mFontMetrics->width( mChr );
  mChrOffset = QPointF( mChrWidth / 2.0, -mFontMetrics->ascent() / 2.0 );
  mOrigSize = mSize; // save in case the size would be data defined
}

void QgsFontMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );
}

QString QgsFontMarkerSymbolLayer::characterToRender( QgsSymbolRenderContext &context, QPointF &charOffset, double &charWidth )
{
  charOffset = mChrOffset;
  QString charToRender = mChr;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCharacter ) )
  {
    context.setOriginalValueVariable( mChr );
    charToRender = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyCharacter, context.renderContext().expressionContext(), mChr );
    if ( charToRender != mChr )
    {
      charWidth = mFontMetrics->width( charToRender );
      charOffset = QPointF( charWidth / 2.0, -mFontMetrics->ascent() / 2.0 );
    }
  }
  return charToRender;
}

void QgsFontMarkerSymbolLayer::calculateOffsetAndRotation( QgsSymbolRenderContext &context,
    double scaledSize,
    bool &hasDataDefinedRotation,
    QPointF &offset,
    double &angle ) const
{
  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, scaledSize, scaledSize, offsetX, offsetY );
  offset = QPointF( offsetX, offsetY );

  //angle
  bool ok = true;
  angle = mAngle + mLineAngle;
  bool usingDataDefinedRotation = false;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle, &ok ) + mLineAngle;
    usingDataDefinedRotation = ok;
  }

  hasDataDefinedRotation = context.renderHints() & QgsSymbol::DynamicRotation || usingDataDefinedRotation;
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature *f = context.feature();
    if ( f )
    {
      if ( f->hasGeometry() && f->geometry().type() == QgsWkbTypes::PointGeometry )
      {
        const QgsMapToPixel &m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}

double QgsFontMarkerSymbolLayer::calculateSize( QgsSymbolRenderContext &context )
{
  double scaledSize = mSize;
  bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  bool ok = true;
  if ( hasDataDefinedSize )
  {
    context.setOriginalValueVariable( mSize );
    scaledSize = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertySize, context.renderContext().expressionContext(), mSize, &ok );
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbol::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case QgsSymbol::ScaleDiameter:
        break;
    }
  }
  return scaledSize;
}

void QgsFontMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
    return;

  QTransform transform;

  bool ok;
  QColor brushColor = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    brushColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), brushColor );
  }
  brushColor = context.selected() ? context.renderContext().selectionColor() : brushColor;
  brushColor.setAlphaF( brushColor.alphaF() * context.opacity() );
  mBrush.setColor( brushColor );

  QColor penColor = mStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    penColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), penColor );
  }
  penColor.setAlphaF( penColor.alphaF() * context.opacity() );

  double penWidth = context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth, &ok );
    if ( ok )
    {
      penWidth = context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
    }
  }

  p->setBrush( mBrush );
  if ( !qgsDoubleNear( penWidth, 0.0 ) )
  {
    mPen.setColor( penColor );
    mPen.setWidthF( penWidth );
    p->setPen( mPen );
  }
  else
  {
    p->setPen( Qt::NoPen );
  }
  p->save();

  QPointF chrOffset = mChrOffset;
  double chrWidth;
  QString charToRender = characterToRender( context, chrOffset, chrWidth );

  double sizeToRender = calculateSize( context );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, sizeToRender, hasDataDefinedRotation, offset, angle );

  transform.translate( point.x() + offset.x(), point.y() + offset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  if ( !qgsDoubleNear( sizeToRender, mOrigSize ) )
  {
    double s = sizeToRender / mOrigSize;
    transform.scale( s, s );
  }

  QPainterPath path;
  path.addText( -chrOffset.x(), -chrOffset.y(), mFont, charToRender );
  p->drawPath( transform.map( path ) );
  p->restore();
}

QgsStringMap QgsFontMarkerSymbolLayer::properties() const
{
  QgsStringMap props;
  props[QStringLiteral( "font" )] = mFontFamily;
  props[QStringLiteral( "chr" )] = mChr;
  props[QStringLiteral( "size" )] = QString::number( mSize );
  props[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( mSizeUnit );
  props[QStringLiteral( "size_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  props[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  props[QStringLiteral( "outline_color" )] = QgsSymbolLayerUtils::encodeColor( mStrokeColor );
  props[QStringLiteral( "outline_width" )] = QString::number( mStrokeWidth );
  props[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( mStrokeWidthUnit );
  props[QStringLiteral( "outline_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale );
  props[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  props[QStringLiteral( "angle" )] = QString::number( mAngle );
  props[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  props[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  props[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  props[QStringLiteral( "horizontal_anchor_point" )] = QString::number( mHorizontalAnchorPoint );
  props[QStringLiteral( "vertical_anchor_point" )] = QString::number( mVerticalAnchorPoint );
  return props;
}

QgsFontMarkerSymbolLayer *QgsFontMarkerSymbolLayer::clone() const
{
  QgsFontMarkerSymbolLayer *m = new QgsFontMarkerSymbolLayer( mFontFamily, mChr, mSize, mColor, mAngle );
  m->setStrokeColor( mStrokeColor );
  m->setStrokeWidth( mStrokeWidth );
  m->setStrokeWidthUnit( mStrokeWidthUnit );
  m->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );
  m->setPenJoinStyle( mPenJoinStyle );
  m->setOffset( mOffset );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setSizeUnit( mSizeUnit );
  m->setSizeMapUnitScale( mSizeMapUnitScale );
  m->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  m->setVerticalAnchorPoint( mVerticalAnchorPoint );
  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsFontMarkerSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  QString fontPath = QStringLiteral( "ttf://%1" ).arg( mFontFamily );
  int markIndex = mChr.unicode();
  double size = QgsSymbolLayerUtils::rescaleUom( mSize, mSizeUnit, props );
  QgsSymbolLayerUtils::externalMarkerToSld( doc, graphicElem, fontPath, QStringLiteral( "ttf" ), &markIndex, mColor, size );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ) ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );
}

QRectF QgsFontMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  QPointF chrOffset = mChrOffset;
  double chrWidth = mChrWidth;
  //calculate width of rendered character
  ( void )characterToRender( context, chrOffset, chrWidth );

  if ( !mFontMetrics )
    mFontMetrics = new QFontMetrics( mFont );

  double scaledSize = calculateSize( context );
  if ( !qgsDoubleNear( scaledSize, mOrigSize ) )
  {
    chrWidth *= scaledSize / mOrigSize;
  }

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, scaledSize, hasDataDefinedRotation, offset, angle );
  scaledSize = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );

  QMatrix transform;

  // move to the desired position
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  QRectF symbolBounds = transform.mapRect( QRectF( -chrWidth / 2.0,
                        -scaledSize / 2.0,
                        chrWidth,
                        scaledSize ) );
  return symbolBounds;
}

QgsSymbolLayer *QgsFontMarkerSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  QString name, format;
  QColor color;
  double size;
  int chr;

  if ( !QgsSymbolLayerUtils::externalMarkerFromSld( graphicElem, name, format, chr, color, size ) )
    return nullptr;

  if ( !name.startsWith( QLatin1String( "ttf://" ) ) || format != QLatin1String( "ttf" ) )
    return nullptr;

  QString fontFamily = name.mid( 6 );

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerUtils::displacementFromSldElement( graphicElem, offset );

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  offset.setX( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.x() ) );
  offset.setY( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.y() ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );

  QgsMarkerSymbolLayer *m = new QgsFontMarkerSymbolLayer( fontFamily, chr, size, color );
  m->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

