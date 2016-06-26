/***************************************************************************
 qgsmarkersymbollayerv2.cpp
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

#include "qgsmarkersymbollayerv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsdxfexport.h"
#include "qgsdxfpaintdevice.h"
#include "qgsexpression.h"
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

static void _fixQPictureDPI( QPainter* p )
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

QgsSimpleMarkerSymbolLayerBase::QgsSimpleMarkerSymbolLayerBase( QgsSimpleMarkerSymbolLayerBase::Shape shape, double size, double angle, QgsSymbolV2::ScaleMethod scaleMethod )
    : mShape( shape )
{
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  mSizeUnit = QgsSymbolV2::MM;
  mOffsetUnit = QgsSymbolV2::MM;
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

void QgsSimpleMarkerSymbolLayerBase::startRender( QgsSymbolV2RenderContext &context )
{
  bool hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE );
  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );

  // use either QPolygonF or QPainterPath for drawing
  if ( !prepareMarkerShape( mShape ) ) // drawing as a polygon
  {
    prepareMarkerPath( mShape ); // drawing as a painter path
  }

  QTransform transform;

  // scale the shape (if the size is not going to be modified)
  if ( !hasDataDefinedSize )
  {
    double scaledSize = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mSize, mSizeUnit, mSizeMapUnitScale );
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

  prepareExpressions( context );

  QgsMarkerSymbolLayerV2::startRender( context );
}

void QgsSimpleMarkerSymbolLayerBase::stopRender( QgsSymbolV2RenderContext &context )
{
  Q_UNUSED( context );
}

void QgsSimpleMarkerSymbolLayerBase::renderPoint( QPointF point, QgsSymbolV2RenderContext &context )
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
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME ) )
  {
    context.setOriginalValueVariable( encodeShape( symbol ) );
    QString name = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      Shape decoded = decodeShape( name, &ok );
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
    double s = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), scaledSize, mSizeUnit, mSizeMapUnitScale );
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

QRectF QgsSimpleMarkerSymbolLayerBase::bounds( QPointF point, QgsSymbolV2RenderContext &context )
{
  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, scaledSize, hasDataDefinedRotation, offset, angle );

  scaledSize = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), scaledSize, mSizeUnit, mSizeMapUnitScale );

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

QgsSimpleMarkerSymbolLayerBase::Shape QgsSimpleMarkerSymbolLayerBase::decodeShape( const QString &name, bool* ok )
{
  if ( ok )
    *ok = true;
  QString cleaned = name.toLower().trimmed();

  if ( cleaned ==  "square" || cleaned ==  "rectangle" )
    return Square;
  else if ( cleaned ==  "diamond" )
    return Diamond;
  else if ( cleaned ==  "pentagon" )
    return Pentagon;
  else if ( cleaned ==  "hexagon" )
    return Hexagon;
  else if ( cleaned ==  "triangle" )
    return Triangle;
  else if ( cleaned ==  "equilateral_triangle" )
    return EquilateralTriangle;
  else if ( cleaned ==  "star" || cleaned == "regular_star" )
    return Star;
  else if ( cleaned ==  "arrow" )
    return Arrow;
  else if ( cleaned ==  "circle" )
    return Circle;
  else if ( cleaned ==  "cross" )
    return Cross;
  else if ( cleaned ==  "cross_fill" )
    return CrossFill;
  else if ( cleaned ==  "cross2" || cleaned == "x" )
    return Cross2;
  else if ( cleaned ==  "line" )
    return Line;
  else if ( cleaned ==  "arrowhead" )
    return ArrowHead;
  else if ( cleaned ==  "filled_arrowhead" )
    return ArrowHeadFilled;
  else if ( cleaned ==  "semi_circle" )
    return SemiCircle;
  else if ( cleaned ==  "third_circle" )
    return ThirdCircle;
  else if ( cleaned ==  "quarter_circle" )
    return QuarterCircle;
  else if ( cleaned ==  "quarter_square" )
    return QuarterSquare;
  else if ( cleaned ==  "half_square" )
    return HalfSquare;
  else if ( cleaned ==  "diagonal_half_square" )
    return DiagonalHalfSquare;
  else if ( cleaned ==  "right_half_triangle" )
    return RightHalfTriangle;
  else if ( cleaned ==  "left_half_triangle" )
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
      return "square";
    case QuarterSquare:
      return "quarter_square";
    case HalfSquare:
      return "half_square";
    case DiagonalHalfSquare:
      return "diagonal_half_square";
    case Diamond:
      return "diamond";
    case Pentagon:
      return "pentagon";
    case Hexagon:
      return "hexagon";
    case Triangle:
      return "triangle";
    case EquilateralTriangle:
      return "equilateral_triangle";
    case LeftHalfTriangle:
      return "left_half_triangle";
    case RightHalfTriangle:
      return "right_half_triangle";
    case Star:
      return "star";
    case Arrow:
      return "arrow";
    case ArrowHeadFilled:
      return "filled_arrowhead";
    case CrossFill:
      return "cross_fill";
    case Circle:
      return "circle";
    case Cross:
      return "cross";
    case Cross2:
      return "cross2";
    case Line:
      return "line";
    case ArrowHead:
      return "arrowhead";
    case SemiCircle:
      return "semi_circle";
    case ThirdCircle:
      return "third_circle";
    case QuarterCircle:
      return "quarter_circle";
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
      polygon << QPointF( sin( DEG2RAD( 288.0 ) ), - cos( DEG2RAD( 288.0 ) ) )
      << QPointF( sin( DEG2RAD( 216.0 ) ), - cos( DEG2RAD( 216.0 ) ) )
      << QPointF( sin( DEG2RAD( 144.0 ) ), - cos( DEG2RAD( 144.0 ) ) )
      << QPointF( sin( DEG2RAD( 72.0 ) ), - cos( DEG2RAD( 72.0 ) ) )
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
      polygon << QPointF( sin( DEG2RAD( 300.0 ) ), - cos( DEG2RAD( 300.0 ) ) )
      << QPointF( sin( DEG2RAD( 240.0 ) ), - cos( DEG2RAD( 240.0 ) ) )
      << QPointF( sin( DEG2RAD( 180.0 ) ), - cos( DEG2RAD( 180.0 ) ) )
      << QPointF( sin( DEG2RAD( 120.0 ) ), - cos( DEG2RAD( 120.0 ) ) )
      << QPointF( sin( DEG2RAD( 60.0 ) ), - cos( DEG2RAD( 60.0 ) ) )
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
      polygon << QPointF( sin( DEG2RAD( 240.0 ) ), - cos( DEG2RAD( 240.0 ) ) )
      << QPointF( sin( DEG2RAD( 120.0 ) ), - cos( DEG2RAD( 120.0 ) ) )
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
      double inner_r = cos( DEG2RAD( 72.0 ) ) / cos( DEG2RAD( 36.0 ) );

      polygon << QPointF( inner_r * sin( DEG2RAD( 324.0 ) ), - inner_r * cos( DEG2RAD( 324.0 ) ) )  // 324
      << QPointF( sin( DEG2RAD( 288.0 ) ), - cos( DEG2RAD( 288 ) ) )    // 288
      << QPointF( inner_r * sin( DEG2RAD( 252.0 ) ), - inner_r * cos( DEG2RAD( 252.0 ) ) )   // 252
      << QPointF( sin( DEG2RAD( 216.0 ) ), - cos( DEG2RAD( 216.0 ) ) )   // 216
      << QPointF( 0, inner_r )         // 180
      << QPointF( sin( DEG2RAD( 144.0 ) ), - cos( DEG2RAD( 144.0 ) ) )   // 144
      << QPointF( inner_r * sin( DEG2RAD( 108.0 ) ), - inner_r * cos( DEG2RAD( 108.0 ) ) )   // 108
      << QPointF( sin( DEG2RAD( 72.0 ) ), - cos( DEG2RAD( 72.0 ) ) )    //  72
      << QPointF( inner_r * sin( DEG2RAD( 36.0 ) ), - inner_r * cos( DEG2RAD( 36.0 ) ) )   //  36
      << QPointF( 0, -1 )
      << QPointF( inner_r * sin( DEG2RAD( 324.0 ) ), - inner_r * cos( DEG2RAD( 324.0 ) ) );  // 324;          //   0
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

double QgsSimpleMarkerSymbolLayerBase::calculateSize( QgsSymbolV2RenderContext &context, bool &hasDataDefinedSize ) const
{
  double scaledSize = mSize;

  hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );
  bool ok = true;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE ) )
  {
    context.setOriginalValueVariable( mSize );
    scaledSize = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE, context, mSize, &ok ).toDouble();
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbolV2::ScaleArea:
        scaledSize = sqrt( scaledSize );
        break;
      case QgsSymbolV2::ScaleDiameter:
        break;
    }
  }

  return scaledSize;
}

void QgsSimpleMarkerSymbolLayerBase::calculateOffsetAndRotation( QgsSymbolV2RenderContext &context, double scaledSize, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const
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
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE ) )
  {
    context.setOriginalValueVariable( angle );
    angle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE, context, mAngle, &ok ).toDouble() + mLineAngle;
    usingDataDefinedRotation = ok;
  }

  hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation || usingDataDefinedRotation;
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature* f = context.feature();
    if ( f )
    {
      const QgsGeometry *g = f->constGeometry();
      if ( g && g->type() == QGis::Point )
      {
        const QgsMapToPixel& m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}


//
// QgsSimpleMarkerSymbolLayerV2
//

QgsSimpleMarkerSymbolLayerV2::QgsSimpleMarkerSymbolLayerV2( const QString& name, const QColor& color, const QColor& borderColor, double size, double angle, QgsSymbolV2::ScaleMethod scaleMethod,
    Qt::PenJoinStyle penJoinStyle )
    : QgsSimpleMarkerSymbolLayerBase( decodeShape( name ), size, angle, scaleMethod )
    , mBorderColor( borderColor )
    , mOutlineStyle( Qt::SolidLine )
    , mOutlineWidth( 0 )
    , mOutlineWidthUnit( QgsSymbolV2::MM )
    , mPenJoinStyle( penJoinStyle )
    , mName( name )
    , mUsingCache( false )
{
  mColor = color;
}

QgsSimpleMarkerSymbolLayerV2::QgsSimpleMarkerSymbolLayerV2( QgsSimpleMarkerSymbolLayerBase::Shape shape, double size, double angle, QgsSymbolV2::ScaleMethod scaleMethod, const QColor& color, const QColor& borderColor, Qt::PenJoinStyle penJoinStyle )
    : QgsSimpleMarkerSymbolLayerBase( shape, size, angle, scaleMethod )
    , mBorderColor( borderColor )
    , mOutlineStyle( Qt::SolidLine )
    , mOutlineWidth( 0 )
    , mOutlineWidthUnit( QgsSymbolV2::MM )
    , mPenJoinStyle( penJoinStyle )
    , mName( encodeShape( shape ) )
    , mUsingCache( false )
{
  mColor = color;
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::create( const QgsStringMap& props )
{
  Shape shape = Circle;
  QColor color = DEFAULT_SIMPLEMARKER_COLOR;
  QColor borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR;
  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;
  QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( "name" ) )
  {
    shape = decodeShape( props["name"] );
  }
  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "color_border" ) )
  {
    //pre 2.5 projects use "color_border"
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["color_border"] );
  }
  else if ( props.contains( "outline_color" ) )
  {
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["outline_color"] );
  }
  else if ( props.contains( "line_color" ) )
  {
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["line_color"] );
  }
  if ( props.contains( "joinstyle" ) )
  {
    penJoinStyle = QgsSymbolLayerV2Utils::decodePenJoinStyle( props["joinstyle"] );
  }
  if ( props.contains( "size" ) )
    size = props["size"].toDouble();
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();
  if ( props.contains( "scale_method" ) )
    scaleMethod = QgsSymbolLayerV2Utils::decodeScaleMethod( props["scale_method"] );

  QgsSimpleMarkerSymbolLayerV2* m = new QgsSimpleMarkerSymbolLayerV2( shape, size, angle, scaleMethod, color, borderColor, penJoinStyle );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  if ( props.contains( "offset_unit" ) )
    m->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  if ( props.contains( "size_unit" ) )
    m->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["size_unit"] ) );
  if ( props.contains( "size_map_unit_scale" ) )
    m->setSizeMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["size_map_unit_scale"] ) );

  if ( props.contains( "outline_style" ) )
  {
    m->setOutlineStyle( QgsSymbolLayerV2Utils::decodePenStyle( props["outline_style"] ) );
  }
  else if ( props.contains( "line_style" ) )
  {
    m->setOutlineStyle( QgsSymbolLayerV2Utils::decodePenStyle( props["line_style"] ) );
  }
  if ( props.contains( "outline_width" ) )
  {
    m->setOutlineWidth( props["outline_width"].toDouble() );
  }
  else if ( props.contains( "line_width" ) )
  {
    m->setOutlineWidth( props["line_width"].toDouble() );
  }
  if ( props.contains( "outline_width_unit" ) )
  {
    m->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["outline_width_unit"] ) );
  }
  if ( props.contains( "line_width_unit" ) )
  {
    m->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["line_width_unit"] ) );
  }
  if ( props.contains( "outline_width_map_unit_scale" ) )
  {
    m->setOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["outline_width_map_unit_scale"] ) );
  }

  if ( props.contains( "horizontal_anchor_point" ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint( props[ "horizontal_anchor_point" ].toInt() ) );
  }
  if ( props.contains( "vertical_anchor_point" ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayerV2::VerticalAnchorPoint( props[ "vertical_anchor_point" ].toInt() ) );
  }

  m->restoreDataDefinedProperties( props );

  return m;
}


QString QgsSimpleMarkerSymbolLayerV2::layerType() const
{
  return "SimpleMarker";
}

void QgsSimpleMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QgsSimpleMarkerSymbolLayerBase::startRender( context );

  QColor brushColor = mColor;
  QColor penColor = mBorderColor;

  brushColor.setAlphaF( mColor.alphaF() * context.alpha() );
  penColor.setAlphaF( mBorderColor.alphaF() * context.alpha() );

  mBrush = QBrush( brushColor );
  mPen = QPen( penColor );
  mPen.setStyle( mOutlineStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidthF( QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mOutlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale ) );

  QColor selBrushColor = context.renderContext().selectionColor();
  QColor selPenColor = selBrushColor == mColor ? selBrushColor : mBorderColor;
  if ( context.alpha() < 1 )
  {
    selBrushColor.setAlphaF( context.alpha() );
    selPenColor.setAlphaF( context.alpha() );
  }
  mSelBrush = QBrush( selBrushColor );
  mSelPen = QPen( selPenColor );
  mSelPen.setStyle( mOutlineStyle );
  mSelPen.setWidthF( QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mOutlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale ) );

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE );
  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );

  // use caching only when:
  // - size, rotation, shape, color, border color is not data-defined
  // - drawing to screen (not printer)
  mUsingCache = !hasDataDefinedRotation && !hasDataDefinedSize && !context.renderContext().forceVectorOutput()
                && !hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME ) && !hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR ) && !hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR_BORDER )
                && !hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) && !hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE )
                && !hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOIN_STYLE ) && !hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );

  if ( !shapeIsFilled( mShape ) )
  {
    // some markers can't be drawn as a polygon (circle, cross)
    // For these set the selected border color to the selected color
    mSelPen.setColor( selBrushColor );
  }


  if ( mUsingCache )
  {
    if ( !qgsDoubleNear( context.renderContext().rasterScaleFactor(), 1.0 ) )
    {
      QTransform transform;
      transform.scale( context.renderContext().rasterScaleFactor(), context.renderContext().rasterScaleFactor() );
      if ( !mPolygon.isEmpty() )
        mPolygon = transform.map( mPolygon );
      else
        mPath = transform.map( mPath );
    }

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


bool QgsSimpleMarkerSymbolLayerV2::prepareCache( QgsSymbolV2RenderContext& context )
{
  double scaledSize = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mSize, mSizeUnit, mSizeMapUnitScale );

  // calculate necessary image size for the cache
  double pw = qRound((( qgsDoubleNear( mPen.widthF(), 0.0 ) ? 1 : mPen.widthF() * 4 ) + 1 ) ) / 2 * 2; // make even (round up); handle cosmetic pen
  int imageSize = ( static_cast< int >( scaledSize ) + pw ) / 2 * 2 + 1; //  make image width, height odd; account for pen width
  double center = imageSize / 2.0;

  if ( imageSize > mMaximumCacheWidth )
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

bool QgsSimpleMarkerSymbolLayerV2::prepareShape( const QString& name )
{
  return shapeToPolygon( name.isNull() ? mShape : decodeShape( name ), mPolygon );
}

bool QgsSimpleMarkerSymbolLayerV2::prepareShape( const QString& name, QPolygonF &polygon ) const
{
  return shapeToPolygon( decodeShape( name ), polygon );
}

bool QgsSimpleMarkerSymbolLayerV2::preparePath( QString name )
{
  return prepareMarkerPath( decodeShape( name ) );
}

void QgsSimpleMarkerSymbolLayerV2::draw( QgsSymbolV2RenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path )
{
  //making changes here? Don't forget to also update ::bounds if the changes affect the bounding box
  //of the rendered point!

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  bool ok = true;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      mBrush.setColor( QgsSymbolLayerV2Utils::decodeColor( colorString ) );
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR_BORDER ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mBorderColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR_BORDER, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      mPen.setColor( QgsSymbolLayerV2Utils::decodeColor( colorString ) );
      mSelPen.setColor( QgsSymbolLayerV2Utils::decodeColor( colorString ) );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    double outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      mPen.setWidthF( QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale ) );
      mSelPen.setWidthF( QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale ) );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePenStyle( mOutlineStyle ) );
    QString outlineStyle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      mPen.setStyle( QgsSymbolLayerV2Utils::decodePenStyle( outlineStyle ) );
      mSelPen.setStyle( QgsSymbolLayerV2Utils::decodePenStyle( outlineStyle ) );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOIN_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle ) );
    QString style = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOIN_STYLE, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      mPen.setJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( style ) );
      mSelPen.setJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( style ) );
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

void QgsSimpleMarkerSymbolLayerV2::renderPoint( QPointF point, QgsSymbolV2RenderContext& context )
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
    double s = img.width() / context.renderContext().rasterScaleFactor();

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

QgsStringMap QgsSimpleMarkerSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["name"] = encodeShape( mShape );
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mBorderColor );
  map["size"] = QString::number( mSize );
  map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  map["size_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSizeMapUnitScale );
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["scale_method"] = QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod );
  map["outline_style"] = QgsSymbolLayerV2Utils::encodePenStyle( mOutlineStyle );
  map["outline_width"] = QString::number( mOutlineWidth );
  map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit );
  map["outline_width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOutlineWidthMapUnitScale );
  map["joinstyle"] = QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle );
  map["horizontal_anchor_point"] = QString::number( mHorizontalAnchorPoint );
  map["vertical_anchor_point"] = QString::number( mVerticalAnchorPoint );


  //data define properties
  saveDataDefinedProperties( map );
  return map;
}

QgsSimpleMarkerSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::clone() const
{
  QgsSimpleMarkerSymbolLayerV2* m = new QgsSimpleMarkerSymbolLayerV2( mShape, mSize, mAngle, mScaleMethod, mColor, mBorderColor, mPenJoinStyle );
  m->setOffset( mOffset );
  m->setSizeUnit( mSizeUnit );
  m->setSizeMapUnitScale( mSizeMapUnitScale );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setOutlineStyle( mOutlineStyle );
  m->setOutlineWidth( mOutlineWidth );
  m->setOutlineWidthUnit( mOutlineWidthUnit );
  m->setOutlineWidthMapUnitScale( mOutlineWidthMapUnitScale );
  m->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  m->setVerticalAnchorPoint( mVerticalAnchorPoint );
  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsSimpleMarkerSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QgsSymbolLayerV2Utils::wellKnownMarkerToSld( doc, graphicElem, encodeShape( mShape ), mColor, mBorderColor, mOutlineStyle, mOutlineWidth, mSize );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, graphicElem, mOffset );
}

QString QgsSimpleMarkerSymbolLayerV2::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
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
  ogrString.append( mBorderColor.name() );
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

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return nullptr;

  QString name = "square";
  QColor color, borderColor;
  double borderWidth, size;
  Qt::PenStyle borderStyle;

  if ( !QgsSymbolLayerV2Utils::wellKnownMarkerFromSld( graphicElem, name, color, borderColor, borderStyle, borderWidth, size ) )
    return nullptr;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerV2Utils::displacementFromSldElement( graphicElem, offset );

  Shape shape = decodeShape( name );

  QgsSimpleMarkerSymbolLayerV2 *m = new QgsSimpleMarkerSymbolLayerV2( shape, size );
  m->setColor( color );
  m->setBorderColor( borderColor );
  m->setAngle( angle );
  m->setOffset( offset );
  m->setOutlineStyle( borderStyle );
  return m;
}

void QgsSimpleMarkerSymbolLayerV2::drawMarker( QPainter* p, QgsSymbolV2RenderContext& context )
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

bool QgsSimpleMarkerSymbolLayerV2::writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, QgsSymbolV2RenderContext &context, QPointF shift ) const
{
  //data defined size?
  double size = mSize;

  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );

  //data defined size
  bool ok = true;
  if ( hasDataDefinedSize )
  {
    if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE ) )
    {
      context.setOriginalValueVariable( mSize );
      size = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE, context, mSize, &ok ).toDouble();
    }

    if ( ok )
    {
      switch ( mScaleMethod )
      {
        case QgsSymbolV2::ScaleArea:
          size = sqrt( size );
          break;
        case QgsSymbolV2::ScaleDiameter:
          break;
      }
    }

    size = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), size, mSizeUnit, mSizeMapUnitScale );
  }
  if ( mSizeUnit == QgsSymbolV2::MM )
  {
    size *= mmMapUnitScaleFactor;
  }
  double halfSize = size / 2.0;

  //outlineWidth
  double outlineWidth = mOutlineWidth;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, mOutlineWidth ).toDouble();
  }
  if ( mSizeUnit == QgsSymbolV2::MM )
  {
    outlineWidth *= mmMapUnitScaleFactor;
  }

  //color
  QColor pc = mPen.color();
  QColor bc = mBrush.color();
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      bc = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR_BORDER ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mBorderColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR_BORDER, context, QVariant(), &ok ).toString();
    if ( ok )
      pc = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );

  QPointF off( offsetX, offsetY );

  //angle
  double angle = mAngle + mLineAngle;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE, context, mAngle ).toDouble() + mLineAngle;
  }

  Shape shape = mShape;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME ) )
  {
    context.setOriginalValueVariable( encodeShape( shape ) );
    QString shapeName = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      shape = decodeShape( shapeName, &ok );
      if ( !ok )
        shape = mShape;
    }
  }

  angle = -angle; //rotation in Qt is counterclockwise
  if ( angle )
    off = _rotatedOffset( off, angle );

  if ( mSizeUnit == QgsSymbolV2::MM )
  {
    off *= mmMapUnitScaleFactor;
  }

  QTransform t;
  t.translate( shift.x() + offsetX, shift.y() + offsetY );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    t.rotate( angle );

  QPolygonF polygon;
  if ( shapeToPolygon( shape, polygon ) )
  {
    t.scale( halfSize, -halfSize );

    polygon = t.map( polygon );

    QgsPointSequenceV2 p;
    for ( int i = 0; i < polygon.size(); i++ )
      p << QgsPointV2( polygon[i] );
    p << p[0];

    if ( mBrush.style() != Qt::NoBrush )
      e.writePolygon( QgsRingSequenceV2() << p, layerName, "SOLID", bc );
    if ( mPen.style() != Qt::NoPen )
      e.writePolyline( p, layerName, "CONTINUOUS", pc, outlineWidth );
  }
  else if ( shape == Circle )
  {
    if ( mBrush.style() != Qt::NoBrush )
      e.writeFilledCircle( layerName, bc, QgsPointV2( shift ), halfSize );
    if ( mPen.style() != Qt::NoPen )
      e.writeCircle( layerName, pc, QgsPointV2( shift ), halfSize, "CONTINUOUS", outlineWidth );
  }
  else if ( shape == Line )
  {
    QPointF pt1 = t.map( QPointF( 0, -halfSize ) );
    QPointF pt2 = t.map( QPointF( 0, halfSize ) );

    if ( mPen.style() != Qt::NoPen )
      e.writeLine( QgsPointV2( pt1 ), QgsPointV2( pt2 ), layerName, "CONTINUOUS", pc, outlineWidth );
  }
  else if ( shape == Cross )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      QPointF pt1 = t.map( QPointF( -halfSize, 0 ) );
      QPointF pt2 = t.map( QPointF( halfSize, 0 ) );
      QPointF pt3 = t.map( QPointF( 0, -halfSize ) );
      QPointF pt4 = t.map( QPointF( 0, halfSize ) );

      e.writeLine( QgsPointV2( pt1 ), QgsPointV2( pt2 ), layerName, "CONTINUOUS", pc, outlineWidth );
      e.writeLine( QgsPointV2( pt3 ), QgsPointV2( pt4 ), layerName, "CONTINUOUS", pc, outlineWidth );
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

      e.writeLine( QgsPointV2( pt1 ), QgsPointV2( pt2 ), layerName, "CONTINUOUS", pc, outlineWidth );
      e.writeLine( QgsPointV2( pt3 ), QgsPointV2( pt4 ), layerName, "CONTINUOUS", pc, outlineWidth );
    }
  }
  else if ( shape == ArrowHead )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      QPointF pt1 = t.map( QPointF( -halfSize, halfSize ) );
      QPointF pt2 = t.map( QPointF( 0, 0 ) );
      QPointF pt3 = t.map( QPointF( -halfSize, -halfSize ) );

      e.writeLine( QgsPointV2( pt1 ), QgsPointV2( pt2 ), layerName, "CONTINUOUS", pc, outlineWidth );
      e.writeLine( QgsPointV2( pt3 ), QgsPointV2( pt2 ), layerName, "CONTINUOUS", pc, outlineWidth );
    }
  }
  else
  {
    QgsDebugMsg( QString( "Unsupported dxf marker name %1" ).arg( encodeShape( shape ) ) );
    return false;
  }

  return true;
}


void QgsSimpleMarkerSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsMarkerSymbolLayerV2::setOutputUnit( unit );
  mOutlineWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsSimpleMarkerSymbolLayerV2::outputUnit() const
{
  if ( QgsMarkerSymbolLayerV2::outputUnit() == mOutlineWidthUnit )
  {
    return mOutlineWidthUnit;
  }
  return QgsSymbolV2::Mixed;
}

void QgsSimpleMarkerSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale& scale )
{
  QgsMarkerSymbolLayerV2::setMapUnitScale( scale );
  mOutlineWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsSimpleMarkerSymbolLayerV2::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayerV2::mapUnitScale() == mOutlineWidthMapUnitScale )
  {
    return mOutlineWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QRectF QgsSimpleMarkerSymbolLayerV2::bounds( QPointF point, QgsSymbolV2RenderContext& context )
{
  QRectF symbolBounds = QgsSimpleMarkerSymbolLayerBase::bounds( point, context );

  // need to account for outline width
  double pixelSize = 1.0 / context.renderContext().rasterScaleFactor();
  double penWidth = 0.0;
  bool ok = true;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    double outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      penWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePenStyle( mOutlineStyle ) );
    QString outlineStyle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE, context, QVariant(), &ok ).toString();
    if ( ok && outlineStyle == "no" )
    {
      penWidth = 0.0;
    }
  }
  //antialiasing
  penWidth += pixelSize;

  //extend bounds by pen width / 2.0
  symbolBounds.adjust( -penWidth / 2.0, -penWidth / 2.0,
                       penWidth / 2.0, penWidth / 2.0 );

  return symbolBounds;
}

void QgsSimpleMarkerSymbolLayerV2::setColor( const QColor& color )
{
  if ( shapeIsFilled( mShape ) )
  {
    setFillColor( color );
  }
  else
  {
    setOutlineColor( color );
  }
}

QColor QgsSimpleMarkerSymbolLayerV2::color() const
{
  if ( shapeIsFilled( mShape ) )
  {
    return fillColor();
  }
  else
  {
    return outlineColor();
  }
}




//
// QgsFilledMarkerSymbolLayer
//

QgsFilledMarkerSymbolLayer::QgsFilledMarkerSymbolLayer( QgsSimpleMarkerSymbolLayerBase::Shape shape, double size, double angle, QgsSymbolV2::ScaleMethod scaleMethod )
    : QgsSimpleMarkerSymbolLayerBase( shape, size, angle, scaleMethod )
{
  mFill.reset( static_cast<QgsFillSymbolV2*>( QgsFillSymbolV2::createSimple( QgsStringMap() ) ) );
}

QgsSymbolLayerV2 *QgsFilledMarkerSymbolLayer::create( const QgsStringMap &props )
{
  QString name = DEFAULT_SIMPLEMARKER_NAME;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;
  QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( "name" ) )
    name = props["name"];
  if ( props.contains( "size" ) )
    size = props["size"].toDouble();
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();
  if ( props.contains( "scale_method" ) )
    scaleMethod = QgsSymbolLayerV2Utils::decodeScaleMethod( props["scale_method"] );

  QgsFilledMarkerSymbolLayer* m = new QgsFilledMarkerSymbolLayer( decodeShape( name ), size, angle, scaleMethod );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  if ( props.contains( "offset_unit" ) )
    m->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  if ( props.contains( "size_unit" ) )
    m->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["size_unit"] ) );
  if ( props.contains( "size_map_unit_scale" ) )
    m->setSizeMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["size_map_unit_scale"] ) );
  if ( props.contains( "horizontal_anchor_point" ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint( props[ "horizontal_anchor_point" ].toInt() ) );
  }
  if ( props.contains( "vertical_anchor_point" ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayerV2::VerticalAnchorPoint( props[ "vertical_anchor_point" ].toInt() ) );
  }

  m->setSubSymbol( QgsFillSymbolV2::createSimple( props ) );

  m->restoreDataDefinedProperties( props );

  return m;
}

QString QgsFilledMarkerSymbolLayer::layerType() const
{
  return "FilledMarker";
}

void QgsFilledMarkerSymbolLayer::startRender( QgsSymbolV2RenderContext &context )
{
  if ( mFill.data() )
  {
    mFill->startRender( context.renderContext(), context.fields() );
  }

  QgsSimpleMarkerSymbolLayerBase::startRender( context );
}

void QgsFilledMarkerSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mFill.data() )
  {
    mFill->stopRender( context.renderContext() );
  }
}

QgsStringMap QgsFilledMarkerSymbolLayer::properties() const
{
  QgsStringMap map;
  map["name"] = encodeShape( mShape );
  map["size"] = QString::number( mSize );
  map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  map["size_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSizeMapUnitScale );
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["scale_method"] = QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod );
  map["horizontal_anchor_point"] = QString::number( mHorizontalAnchorPoint );
  map["vertical_anchor_point"] = QString::number( mVerticalAnchorPoint );

  if ( mFill.data() )
  {
    map["color"] = QgsSymbolLayerV2Utils::encodeColor( mFill->color() );
  }

  //data define properties
  saveDataDefinedProperties( map );
  return map;
}

QgsFilledMarkerSymbolLayer *QgsFilledMarkerSymbolLayer::clone() const
{
  QgsFilledMarkerSymbolLayer* m = static_cast< QgsFilledMarkerSymbolLayer* >( QgsFilledMarkerSymbolLayer::create( properties() ) );
  copyPaintEffect( m );
  m->setSubSymbol( mFill->clone() );
  return m;
}

QgsSymbolV2* QgsFilledMarkerSymbolLayer::subSymbol()
{
  return mFill.data();
}

bool QgsFilledMarkerSymbolLayer::setSubSymbol( QgsSymbolV2 *symbol )
{
  if ( symbol && symbol->type() == QgsSymbolV2::Fill )
  {
    mFill.reset( static_cast<QgsFillSymbolV2*>( symbol ) );
    return true;
  }
  else
  {
    delete symbol;
    return false;
  }
}

double QgsFilledMarkerSymbolLayer::estimateMaxBleed() const
{
  if ( mFill.data() )
  {
    return QgsSymbolLayerV2Utils::estimateMaxSymbolBleed( mFill.data() );
  }
  return 0;
}

QSet<QString> QgsFilledMarkerSymbolLayer::usedAttributes() const
{
  QSet<QString> attr = QgsSimpleMarkerSymbolLayerBase::usedAttributes();
  if ( mFill.data() )
    attr.unite( mFill->usedAttributes() );
  return attr;
}

void QgsFilledMarkerSymbolLayer::setColor( const QColor& c )
{
  mColor = c;
  if ( mFill.data() )
    mFill->setColor( c );
}

QColor QgsFilledMarkerSymbolLayer::color() const
{
  return mFill.data() ?  mFill->color() : mColor;
}

void QgsFilledMarkerSymbolLayer::draw( QgsSymbolV2RenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path )
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
    mFill->renderPolygon( polygon, /* rings */ nullptr, context.feature(), context.renderContext() );
  }
  else
  {
    QPolygonF poly = path.toFillPolygon();
    mFill->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext() );
  }


}


//////////


QgsSvgMarkerSymbolLayerV2::QgsSvgMarkerSymbolLayerV2( const QString& name, double size, double angle, QgsSymbolV2::ScaleMethod scaleMethod )
{
  mPath = QgsSymbolLayerV2Utils::symbolNameToPath( name );
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  mOutlineWidth = 0.2;
  mOutlineWidthUnit = QgsSymbolV2::MM;
  mColor = QColor( Qt::black );
  mOutlineColor = QColor( Qt::black );
}


QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2::create( const QgsStringMap& props )
{
  QString name = DEFAULT_SVGMARKER_NAME;
  double size = DEFAULT_SVGMARKER_SIZE;
  double angle = DEFAULT_SVGMARKER_ANGLE;
  QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( "name" ) )
    name = props["name"];
  if ( props.contains( "size" ) )
    size = props["size"].toDouble();
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();
  if ( props.contains( "scale_method" ) )
    scaleMethod = QgsSymbolLayerV2Utils::decodeScaleMethod( props["scale_method"] );

  QgsSvgMarkerSymbolLayerV2* m = new QgsSvgMarkerSymbolLayerV2( name, size, angle, scaleMethod );

  //we only check the svg default parameters if necessary, since it could be expensive
  if ( !props.contains( "fill" ) && !props.contains( "color" ) && !props.contains( "outline" ) &&
       !props.contains( "outline_color" ) && !props.contains( "outline-width" ) && !props.contains( "outline_width" ) )
  {
    QColor fillColor, outlineColor;
    double fillOpacity = 1.0;
    double outlineOpacity = 1.0;
    double outlineWidth;
    bool hasFillParam = false, hasFillOpacityParam = false, hasOutlineParam = false, hasOutlineWidthParam = false, hasOutlineOpacityParam = false;
    bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultOutlineColor = false, hasDefaultOutlineWidth = false, hasDefaultOutlineOpacity = false;
    QgsSvgCache::instance()->containsParams( name, hasFillParam, hasDefaultFillColor, fillColor,
        hasFillOpacityParam, hasDefaultFillOpacity, fillOpacity,
        hasOutlineParam, hasDefaultOutlineColor, outlineColor,
        hasOutlineWidthParam, hasDefaultOutlineWidth, outlineWidth,
        hasOutlineOpacityParam, hasDefaultOutlineOpacity, outlineOpacity );
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
    if ( hasDefaultOutlineColor )
    {
      m->setOutlineColor( outlineColor );
    }
    if ( hasDefaultOutlineWidth )
    {
      m->setOutlineWidth( outlineWidth );
    }
    if ( hasDefaultOutlineOpacity )
    {
      QColor c = m->outlineColor();
      c.setAlphaF( outlineOpacity );
      m->setOutlineColor( c );
    }
  }

  if ( props.contains( "size_unit" ) )
    m->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["size_unit"] ) );
  if ( props.contains( "size_map_unit_scale" ) )
    m->setSizeMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["size_map_unit_scale"] ) );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  if ( props.contains( "offset_unit" ) )
    m->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  if ( props.contains( "fill" ) )
  {
    //pre 2.5 projects used "fill"
    m->setFillColor( QgsSymbolLayerV2Utils::decodeColor( props["fill"] ) );
  }
  else if ( props.contains( "color" ) )
  {
    m->setFillColor( QgsSymbolLayerV2Utils::decodeColor( props["color"] ) );
  }
  if ( props.contains( "outline" ) )
  {
    //pre 2.5 projects used "outline"
    m->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( props["outline"] ) );
  }
  else if ( props.contains( "outline_color" ) )
  {
    m->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( props["outline_color"] ) );
  }
  else if ( props.contains( "line_color" ) )
  {
    m->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( props["line_color"] ) );
  }

  if ( props.contains( "outline-width" ) )
  {
    //pre 2.5 projects used "outline-width"
    m->setOutlineWidth( props["outline-width"].toDouble() );
  }
  else if ( props.contains( "outline_width" ) )
  {
    m->setOutlineWidth( props["outline_width"].toDouble() );
  }
  else if ( props.contains( "line_width" ) )
  {
    m->setOutlineWidth( props["line_width"].toDouble() );
  }

  if ( props.contains( "outline_width_unit" ) )
  {
    m->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["outline_width_unit"] ) );
  }
  else if ( props.contains( "line_width_unit" ) )
  {
    m->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["line_width_unit"] ) );
  }
  if ( props.contains( "outline_width_map_unit_scale" ) )
    m->setOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["outline_width_map_unit_scale"] ) );

  if ( props.contains( "horizontal_anchor_point" ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint( props[ "horizontal_anchor_point" ].toInt() ) );
  }
  if ( props.contains( "vertical_anchor_point" ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayerV2::VerticalAnchorPoint( props[ "vertical_anchor_point" ].toInt() ) );
  }

  m->restoreDataDefinedProperties( props );

  return m;
}

void QgsSvgMarkerSymbolLayerV2::setPath( const QString& path )
{
  mPath = path;
  QColor defaultFillColor, defaultOutlineColor;
  double outlineWidth, fillOpacity, outlineOpacity;
  bool hasFillParam = false, hasFillOpacityParam = false, hasOutlineParam = false, hasOutlineWidthParam = false, hasOutlineOpacityParam = false;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultOutlineColor = false, hasDefaultOutlineWidth = false, hasDefaultOutlineOpacity = false;
  QgsSvgCache::instance()->containsParams( path, hasFillParam, hasDefaultFillColor, defaultFillColor,
      hasFillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      hasOutlineParam, hasDefaultOutlineColor, defaultOutlineColor,
      hasOutlineWidthParam, hasDefaultOutlineWidth, outlineWidth,
      hasOutlineOpacityParam, hasDefaultOutlineOpacity, outlineOpacity );

  double newFillOpacity = hasFillOpacityParam ? fillColor().alphaF() : 1.0;
  double newOutlineOpacity = hasOutlineOpacityParam ? outlineColor().alphaF() : 1.0;

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
  if ( hasDefaultOutlineColor )
  {
    defaultOutlineColor.setAlphaF( newOutlineOpacity );
    setOutlineColor( defaultOutlineColor );
  }
  if ( hasDefaultOutlineWidth )
  {
    setOutlineWidth( outlineWidth );
  }
  if ( hasDefaultOutlineOpacity )
  {
    QColor c = outlineColor();
    c.setAlphaF( outlineOpacity );
    setOutlineColor( c );
  }
}


QString QgsSvgMarkerSymbolLayerV2::layerType() const
{
  return "SvgMarker";
}

void QgsSvgMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QgsMarkerSymbolLayerV2::startRender( context ); // get anchor point expressions
  Q_UNUSED( context );
  prepareExpressions( context );
}

void QgsSvgMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsSvgMarkerSymbolLayerV2::renderPoint( QPointF point, QgsSymbolV2RenderContext& context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
    return;

  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );
  double size = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), scaledSize, mSizeUnit, mSizeMapUnitScale );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( size ) < 1 || 10000.0 < size )
  {
    return;
  }

  p->save();

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledSize, outputOffset, angle );

  p->translate( point + outputOffset );

  bool rotated = !qgsDoubleNear( angle, 0 );
  if ( rotated )
    p->rotate( angle );

  QString path = mPath;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME ) )
  {
    context.setOriginalValueVariable( mPath );
    path = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME, context, mPath ).toString();
  }

  double outlineWidth = mOutlineWidth;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, mOutlineWidth ).toDouble();
  }
  outlineWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );

  QColor fillColor = mColor;
  bool ok = false;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL, context, QVariant(), &ok ).toString();
    if ( ok )
      fillColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  QColor outlineColor = mOutlineColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mOutlineColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE, context, QVariant(), &ok ).toString();
    if ( ok )
      outlineColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  bool fitsInCache = true;
  bool usePict = true;
  double hwRatio = 1.0;
  if ( !context.renderContext().forceVectorOutput() && !rotated )
  {
    usePict = false;
    const QImage& img = QgsSvgCache::instance()->svgAsImage( path, size, fillColor, outlineColor, outlineWidth,
                        context.renderContext().scaleFactor(), context.renderContext().rasterScaleFactor(), fitsInCache );
    if ( fitsInCache && img.width() > 1 )
    {
      //consider transparency
      if ( !qgsDoubleNear( context.alpha(), 1.0 ) )
      {
        QImage transparentImage = img.copy();
        QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
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
    p->setOpacity( context.alpha() );
    const QPicture& pct = QgsSvgCache::instance()->svgAsPicture( path, size, fillColor, outlineColor, outlineWidth,
                          context.renderContext().scaleFactor(), context.renderContext().rasterScaleFactor(), context.renderContext().forceVectorOutput() );

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
    double penWidth = QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), QgsSymbolV2::MM );
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
    // workaround issue with nested QPictures forgetting antialiasing flag - see http://hub.qgis.org/issues/14960
    p->setRenderHint( QPainter::Antialiasing );
  }

}

double QgsSvgMarkerSymbolLayerV2::calculateSize( QgsSymbolV2RenderContext& context, bool& hasDataDefinedSize ) const
{
  double scaledSize = mSize;
  hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );

  bool ok = true;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE ) )
  {
    context.setOriginalValueVariable( mSize );
    scaledSize = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE, context, mSize, &ok ).toDouble();
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbolV2::ScaleArea:
        scaledSize = sqrt( scaledSize );
        break;
      case QgsSymbolV2::ScaleDiameter:
        break;
    }
  }

  return scaledSize;
}

void QgsSvgMarkerSymbolLayerV2::calculateOffsetAndRotation( QgsSymbolV2RenderContext& context, double scaledSize, QPointF& offset, double& angle ) const
{
  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, scaledSize, scaledSize, offsetX, offsetY );
  offset = QPointF( offsetX, offsetY );

  angle = mAngle + mLineAngle;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE, context, mAngle ).toDouble() + mLineAngle;
  }

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE );
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature* f = context.feature();
    if ( f )
    {
      const QgsGeometry *g = f->constGeometry();
      if ( g && g->type() == QGis::Point )
      {
        const QgsMapToPixel& m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}


QgsStringMap QgsSvgMarkerSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["name"] = QgsSymbolLayerV2Utils::symbolPathToName( mPath );
  map["size"] = QString::number( mSize );
  map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  map["size_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSizeMapUnitScale );
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["scale_method"] = QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod );
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mOutlineColor );
  map["outline_width"] = QString::number( mOutlineWidth );
  map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit );
  map["outline_width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOutlineWidthMapUnitScale );
  map["horizontal_anchor_point"] = QString::number( mHorizontalAnchorPoint );
  map["vertical_anchor_point"] = QString::number( mVerticalAnchorPoint );

  saveDataDefinedProperties( map );
  return map;
}

QgsSvgMarkerSymbolLayerV2* QgsSvgMarkerSymbolLayerV2::clone() const
{
  QgsSvgMarkerSymbolLayerV2* m = new QgsSvgMarkerSymbolLayerV2( mPath, mSize, mAngle );
  m->setColor( mColor );
  m->setOutlineColor( mOutlineColor );
  m->setOutlineWidth( mOutlineWidth );
  m->setOutlineWidthUnit( mOutlineWidthUnit );
  m->setOutlineWidthMapUnitScale( mOutlineWidthMapUnitScale );
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

void QgsSvgMarkerSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsMarkerSymbolLayerV2::setOutputUnit( unit );
  mOutlineWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsSvgMarkerSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = QgsMarkerSymbolLayerV2::outputUnit();
  if ( unit != mOutlineWidthUnit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

void QgsSvgMarkerSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayerV2::setMapUnitScale( scale );
  mOutlineWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsSvgMarkerSymbolLayerV2::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayerV2::mapUnitScale() == mOutlineWidthMapUnitScale )
  {
    return mOutlineWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsSvgMarkerSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QgsSymbolLayerV2Utils::externalGraphicToSld( doc, graphicElem, mPath, "image/svg+xml", mColor, mSize );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }

  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, graphicElem, mOffset );
}

QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return nullptr;

  QString path, mimeType;
  QColor fillColor;
  double size;

  if ( !QgsSymbolLayerV2Utils::externalGraphicFromSld( graphicElem, path, mimeType, fillColor, size ) )
    return nullptr;

  if ( mimeType != "image/svg+xml" )
    return nullptr;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerV2Utils::displacementFromSldElement( graphicElem, offset );

  QgsSvgMarkerSymbolLayerV2* m = new QgsSvgMarkerSymbolLayerV2( path, size );
  m->setFillColor( fillColor );
  //m->setOutlineColor( outlineColor );
  //m->setOutlineWidth( outlineWidth );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

bool QgsSvgMarkerSymbolLayerV2::writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, QgsSymbolV2RenderContext &context, QPointF shift ) const
{
  Q_UNUSED( layerName );
  Q_UNUSED( shift ); //todo...

  //size
  double size = mSize;

  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );

  bool ok = true;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE ) )
  {
    context.setOriginalValueVariable( mSize );
    size = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE, context, mSize, &ok ).toDouble();
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbolV2::ScaleArea:
        size = sqrt( size );
        break;
      case QgsSymbolV2::ScaleDiameter:
        break;
    }
  }

  if ( mSizeUnit == QgsSymbolV2::MM )
  {
    size *= mmMapUnitScaleFactor;
  }

  double halfSize = size / 2.0;

  //offset, angle
  QPointF offset = mOffset;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePoint( mOffset ) );
    QString offsetString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET, context, QVariant(), &ok ).toString();
    if ( ok )
      offset = QgsSymbolLayerV2Utils::decodePoint( offsetString );
  }
  double offsetX = offset.x();
  double offsetY = offset.y();
  if ( mSizeUnit == QgsSymbolV2::MM )
  {
    offsetX *= mmMapUnitScaleFactor;
    offsetY *= mmMapUnitScaleFactor;
  }

  QPointF outputOffset( offsetX, offsetY );

  double angle = mAngle + mLineAngle;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE, context, mAngle ).toDouble() + mLineAngle;
  }
  //angle = -angle; //rotation in Qt is counterclockwise
  if ( angle )
    outputOffset = _rotatedOffset( outputOffset, angle );

  QString path = mPath;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME ) )
  {
    context.setOriginalValueVariable( mPath );
    path = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME, context, mPath ).toString();
  }

  double outlineWidth = mOutlineWidth;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, mOutlineWidth ).toDouble();
  }
  outlineWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );

  QColor fillColor = mColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL, context, QVariant(), &ok ).toString();
    if ( ok )
      fillColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  QColor outlineColor = mOutlineColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mOutlineColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE, context, QVariant(), &ok ).toString();
    if ( ok )
      outlineColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  const QByteArray &svgContent = QgsSvgCache::instance()->svgContent( path, size, fillColor, outlineColor, outlineWidth,
                                 context.renderContext().scaleFactor(),
                                 context.renderContext().rasterScaleFactor() );

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
  pd.setShift( shift );
  pd.setOutputSize( QRectF( -halfSize, -halfSize, size, size ) );
  pd.setLayer( layerName );
  r.render( &p );
  p.end();
  return true;
}

QRectF QgsSvgMarkerSymbolLayerV2::bounds( QPointF point, QgsSymbolV2RenderContext& context )
{
  bool hasDataDefinedSize = false;
  double scaledSize = calculateSize( context, hasDataDefinedSize );
  scaledSize = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), scaledSize, mSizeUnit, mSizeMapUnitScale );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( scaledSize ) < 1 || 10000.0 < scaledSize )
  {
    return QRectF();
  }

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledSize, outputOffset, angle );

  QString path = mPath;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME ) )
  {
    context.setOriginalValueVariable( mPath );
    path = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_NAME, context, mPath ).toString();
  }

  double outlineWidth = mOutlineWidth;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, mOutlineWidth ).toDouble();
  }
  outlineWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );

  //need to get colors to take advantage of cached SVGs
  QColor fillColor = mColor;
  bool ok = false;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL, context, QVariant(), &ok ).toString();
    if ( ok )
      fillColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  QColor outlineColor = mOutlineColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mOutlineColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE, context, QVariant(), &ok ).toString();
    if ( ok )
      outlineColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  QSizeF svgViewbox = QgsSvgCache::instance()->svgViewboxSize( path, scaledSize, fillColor, outlineColor, outlineWidth,
                      context.renderContext().scaleFactor(),
                      context.renderContext().rasterScaleFactor() );

  double scaledHeight = svgViewbox.isValid() ? scaledSize * svgViewbox.height() / svgViewbox.width() : scaledSize;
  double pixelSize = 1.0 / context.renderContext().rasterScaleFactor();

  QMatrix transform;

  // move to the desired position
  transform.translate( point.x() + outputOffset.x(), point.y() + outputOffset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  //antialiasing
  outlineWidth += pixelSize / 2.0;

  QRectF symbolBounds = transform.mapRect( QRectF( -scaledSize / 2.0,
                        -scaledHeight / 2.0,
                        scaledSize,
                        scaledHeight ) );

  //extend bounds by pen width / 2.0
  symbolBounds.adjust( -outlineWidth / 2.0, -outlineWidth / 2.0,
                       outlineWidth / 2.0, outlineWidth / 2.0 );

  return symbolBounds;

}

//////////

QgsFontMarkerSymbolLayerV2::QgsFontMarkerSymbolLayerV2( const QString& fontFamily, QChar chr, double pointSize, const QColor& color, double angle )
    : mFontMetrics( nullptr )
    , mChrWidth( 0 )
{
  mFontFamily = fontFamily;
  mChr = chr;
  mColor = color;
  mAngle = angle;
  mSize = pointSize;
  mOrigSize = pointSize;
  mSizeUnit = QgsSymbolV2::MM;
  mOffset = QPointF( 0, 0 );
  mOffsetUnit = QgsSymbolV2::MM;
  mOutlineColor = DEFAULT_FONTMARKER_BORDERCOLOR;
  mOutlineWidth = 0.0;
  mOutlineWidthUnit = QgsSymbolV2::MM;
  mPenJoinStyle = DEFAULT_FONTMARKER_JOINSTYLE;
}

QgsFontMarkerSymbolLayerV2::~QgsFontMarkerSymbolLayerV2()
{
  delete mFontMetrics;
}

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2::create( const QgsStringMap& props )
{
  QString fontFamily = DEFAULT_FONTMARKER_FONT;
  QChar chr = DEFAULT_FONTMARKER_CHR;
  double pointSize = DEFAULT_FONTMARKER_SIZE;
  QColor color = DEFAULT_FONTMARKER_COLOR;
  double angle = DEFAULT_FONTMARKER_ANGLE;

  if ( props.contains( "font" ) )
    fontFamily = props["font"];
  if ( props.contains( "chr" ) && props["chr"].length() > 0 )
    chr = props["chr"].at( 0 );
  if ( props.contains( "size" ) )
    pointSize = props["size"].toDouble();
  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();

  QgsFontMarkerSymbolLayerV2* m = new QgsFontMarkerSymbolLayerV2( fontFamily, chr, pointSize, color, angle );

  if ( props.contains( "outline_color" ) )
    m->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( props["outline_color"] ) );
  if ( props.contains( "outline_width" ) )
    m->setOutlineWidth( props["outline_width"].toDouble() );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  if ( props.contains( "offset_unit" ) )
    m->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit" ] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale" ] ) );
  if ( props.contains( "size_unit" ) )
    m->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["size_unit"] ) );
  if ( props.contains( "size_map_unit_scale" ) )
    m->setSizeMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["size_map_unit_scale"] ) );
  if ( props.contains( "outline_width_unit" ) )
    m->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["outline_width_unit"] ) );
  if ( props.contains( "outline_width_map_unit_scale" ) )
    m->setOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["outline_width_map_unit_scale"] ) );
  if ( props.contains( "joinstyle" ) )
    m->setPenJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( props["joinstyle"] ) );
  if ( props.contains( "horizontal_anchor_point" ) )
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint( props[ "horizontal_anchor_point" ].toInt() ) );
  if ( props.contains( "vertical_anchor_point" ) )
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayerV2::VerticalAnchorPoint( props[ "vertical_anchor_point" ].toInt() ) );

  m->restoreDataDefinedProperties( props );

  return m;
}

QString QgsFontMarkerSymbolLayerV2::layerType() const
{
  return "FontMarker";
}

void QgsFontMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QColor brushColor = mColor;
  QColor penColor = mOutlineColor;

  brushColor.setAlphaF( mColor.alphaF() * context.alpha() );
  penColor.setAlphaF( mOutlineColor.alphaF() * context.alpha() );

  mBrush = QBrush( brushColor );
  mPen = QPen( penColor );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidthF( QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mOutlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale ) );

  mFont = QFont( mFontFamily );
  mFont.setPixelSize( QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mSize, mSizeUnit, mSizeMapUnitScale ) );
  delete mFontMetrics;
  mFontMetrics = new QFontMetrics( mFont );
  mChrWidth =  mFontMetrics->width( mChr );
  mChrOffset = QPointF( mChrWidth / 2.0, -mFontMetrics->ascent() / 2.0 );
  mOrigSize = mSize; // save in case the size would be data defined
  prepareExpressions( context );
}

void QgsFontMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

QString QgsFontMarkerSymbolLayerV2::characterToRender( QgsSymbolV2RenderContext& context, QPointF& charOffset, double& charWidth )
{
  charOffset = mChrOffset;
  QString charToRender = mChr;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_CHAR ) )
  {
    context.setOriginalValueVariable( mChr );
    charToRender = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_CHAR, context, mChr ).toString();
    if ( charToRender != mChr )
    {
      charWidth = mFontMetrics->width( charToRender );
      charOffset = QPointF( charWidth / 2.0, -mFontMetrics->ascent() / 2.0 );
    }
  }
  return charToRender;
}

void QgsFontMarkerSymbolLayerV2::calculateOffsetAndRotation( QgsSymbolV2RenderContext& context,
    double scaledSize,
    bool& hasDataDefinedRotation,
    QPointF& offset,
    double& angle ) const
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
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE ) )
  {
    context.setOriginalValueVariable( angle );
    angle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_ANGLE, context, mAngle, &ok ).toDouble() + mLineAngle;
    usingDataDefinedRotation = ok;
  }

  hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation || usingDataDefinedRotation;
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature* f = context.feature();
    if ( f )
    {
      const QgsGeometry *g = f->constGeometry();
      if ( g && g->type() == QGis::Point )
      {
        const QgsMapToPixel& m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}

double QgsFontMarkerSymbolLayerV2::calculateSize( QgsSymbolV2RenderContext& context )
{
  double scaledSize = mSize;
  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE );

  bool ok = true;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE ) )
  {
    context.setOriginalValueVariable( mSize );
    scaledSize = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_SIZE, context, mSize, &ok ).toDouble();
  }

  if ( hasDataDefinedSize && ok )
  {
    switch ( mScaleMethod )
    {
      case QgsSymbolV2::ScaleArea:
        scaledSize = sqrt( scaledSize );
        break;
      case QgsSymbolV2::ScaleDiameter:
        break;
    }
  }
  return scaledSize;
}

void QgsFontMarkerSymbolLayerV2::renderPoint( QPointF point, QgsSymbolV2RenderContext& context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
    return;

  QTransform transform;

  bool ok;
  QColor brushColor = mColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      brushColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }
  brushColor = context.selected() ? context.renderContext().selectionColor() : brushColor;
  brushColor.setAlphaF( brushColor.alphaF() * context.alpha() );
  mBrush.setColor( brushColor );

  QColor penColor = mOutlineColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR_BORDER ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mOutlineColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR_BORDER, context, QVariant(), &ok ).toString();
    if ( ok )
      penColor = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }
  penColor.setAlphaF( penColor.alphaF() * context.alpha() );

  double penWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mOutlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    double outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      penWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );
    }
  }

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOIN_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle ) );
    QString style = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOIN_STYLE, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      mPen.setJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( style ) );
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

QgsStringMap QgsFontMarkerSymbolLayerV2::properties() const
{
  QgsStringMap props;
  props["font"] = mFontFamily;
  props["chr"] = mChr;
  props["size"] = QString::number( mSize );
  props["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  props["size_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSizeMapUnitScale );
  props["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  props["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mOutlineColor );
  props["outline_width"] = QString::number( mOutlineWidth );
  props["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit );
  props["outline_width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOutlineWidthMapUnitScale );
  props["joinstyle"] = QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle );
  props["angle"] = QString::number( mAngle );
  props["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  props["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  props["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  props["horizontal_anchor_point"] = QString::number( mHorizontalAnchorPoint );
  props["vertical_anchor_point"] = QString::number( mVerticalAnchorPoint );

  //data define properties
  saveDataDefinedProperties( props );

  return props;
}

QgsFontMarkerSymbolLayerV2* QgsFontMarkerSymbolLayerV2::clone() const
{
  QgsFontMarkerSymbolLayerV2* m = new QgsFontMarkerSymbolLayerV2( mFontFamily, mChr, mSize, mColor, mAngle );
  m->setOutlineColor( mOutlineColor );
  m->setOutlineWidth( mOutlineWidth );
  m->setOutlineWidthUnit( mOutlineWidthUnit );
  m->setOutlineWidthMapUnitScale( mOutlineWidthMapUnitScale );
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

void QgsFontMarkerSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QString fontPath = QString( "ttf://%1" ).arg( mFontFamily );
  int markIndex = mChr.unicode();
  QgsSymbolLayerV2Utils::externalMarkerToSld( doc, graphicElem, fontPath, "ttf", &markIndex, mColor, mSize );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, graphicElem, mOffset );
}

QRectF QgsFontMarkerSymbolLayerV2::bounds( QPointF point, QgsSymbolV2RenderContext& context )
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
  scaledSize = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), scaledSize, mSizeUnit, mSizeMapUnitScale );

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

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return nullptr;

  QString name, format;
  QColor color;
  double size;
  int chr;

  if ( !QgsSymbolLayerV2Utils::externalMarkerFromSld( graphicElem, name, format, chr, color, size ) )
    return nullptr;

  if ( !name.startsWith( "ttf://" ) || format != "ttf" )
    return nullptr;

  QString fontFamily = name.mid( 6 );

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerV2Utils::displacementFromSldElement( graphicElem, offset );

  QgsMarkerSymbolLayerV2 *m = new QgsFontMarkerSymbolLayerV2( fontFamily, chr, size, color );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

