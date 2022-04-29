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
#include "qgsfontutils.h"
#include "qgsimagecache.h"
#include "qgsimageoperation.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgssvgcache.h"
#include "qgsunittypes.h"
#include "qgssymbol.h"
#include "qgsfillsymbol.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

static constexpr int MAX_FONT_CHARACTER_SIZE_IN_PIXELS = 500;

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

QList<Qgis::MarkerShape> QgsSimpleMarkerSymbolLayerBase::availableShapes()
{
  QList< Qgis::MarkerShape > shapes;
  shapes << Qgis::MarkerShape::Square
         << Qgis::MarkerShape::Diamond
         << Qgis::MarkerShape::Pentagon
         << Qgis::MarkerShape::Hexagon
         << Qgis::MarkerShape::Octagon
         << Qgis::MarkerShape::SquareWithCorners
         << Qgis::MarkerShape::Triangle
         << Qgis::MarkerShape::EquilateralTriangle
         << Qgis::MarkerShape::Star
         << Qgis::MarkerShape::Arrow
         << Qgis::MarkerShape::Circle
         << Qgis::MarkerShape::Cross
         << Qgis::MarkerShape::CrossFill
         << Qgis::MarkerShape::Cross2
         << Qgis::MarkerShape::Line
         << Qgis::MarkerShape::HalfArc
         << Qgis::MarkerShape::ThirdArc
         << Qgis::MarkerShape::QuarterArc
         << Qgis::MarkerShape::ArrowHead
         << Qgis::MarkerShape::ArrowHeadFilled
         << Qgis::MarkerShape::SemiCircle
         << Qgis::MarkerShape::ThirdCircle
         << Qgis::MarkerShape::QuarterCircle
         << Qgis::MarkerShape::QuarterSquare
         << Qgis::MarkerShape::HalfSquare
         << Qgis::MarkerShape::DiagonalHalfSquare
         << Qgis::MarkerShape::RightHalfTriangle
         << Qgis::MarkerShape::LeftHalfTriangle
         << Qgis::MarkerShape::AsteriskFill;

  return shapes;
}

QgsSimpleMarkerSymbolLayerBase::QgsSimpleMarkerSymbolLayerBase( Qgis::MarkerShape shape, double size, double angle, Qgis::ScaleMethod scaleMethod )
  : mShape( shape )
{
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  mSizeUnit = QgsUnitTypes::RenderMillimeters;
  mOffsetUnit = QgsUnitTypes::RenderMillimeters;
}

QgsSimpleMarkerSymbolLayerBase::~QgsSimpleMarkerSymbolLayerBase() = default;

bool QgsSimpleMarkerSymbolLayerBase::shapeIsFilled( Qgis::MarkerShape shape )
{
  switch ( shape )
  {
    case Qgis::MarkerShape::Square:
    case Qgis::MarkerShape::Diamond:
    case Qgis::MarkerShape::Pentagon:
    case Qgis::MarkerShape::Hexagon:
    case Qgis::MarkerShape::Octagon:
    case Qgis::MarkerShape::SquareWithCorners:
    case Qgis::MarkerShape::Triangle:
    case Qgis::MarkerShape::EquilateralTriangle:
    case Qgis::MarkerShape::Star:
    case Qgis::MarkerShape::Arrow:
    case Qgis::MarkerShape::Circle:
    case Qgis::MarkerShape::CrossFill:
    case Qgis::MarkerShape::ArrowHeadFilled:
    case Qgis::MarkerShape::SemiCircle:
    case Qgis::MarkerShape::ThirdCircle:
    case Qgis::MarkerShape::QuarterCircle:
    case Qgis::MarkerShape::QuarterSquare:
    case Qgis::MarkerShape::HalfSquare:
    case Qgis::MarkerShape::DiagonalHalfSquare:
    case Qgis::MarkerShape::RightHalfTriangle:
    case Qgis::MarkerShape::LeftHalfTriangle:
    case Qgis::MarkerShape::AsteriskFill:
      return true;

    case Qgis::MarkerShape::Cross:
    case Qgis::MarkerShape::Cross2:
    case Qgis::MarkerShape::Line:
    case Qgis::MarkerShape::ArrowHead:
    case Qgis::MarkerShape::HalfArc:
    case Qgis::MarkerShape::ThirdArc:
    case Qgis::MarkerShape::QuarterArc:
      return false;
  }
  return true;
}

void QgsSimpleMarkerSymbolLayerBase::startRender( QgsSymbolRenderContext &context )
{
  const bool hasDataDefinedRotation = context.renderHints() & Qgis::SymbolRenderHint::DynamicRotation
                                      || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
  const bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

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
    if ( mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits && context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
    {
      // rendering for symbol previews -- an size in meters in map units can't be calculated, so treat the size as millimeters
      // and clamp it to a reasonable range. It's the best we can do in this situation!
      scaledSize = std::min( std::max( context.renderContext().convertToPainterUnits( mSize, QgsUnitTypes::RenderMillimeters ), 3.0 ), 100.0 );
    }

    const double half = scaledSize / 2.0;
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
  Q_UNUSED( context )
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
  const double scaledSize = calculateSize( context, hasDataDefinedSize );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, scaledSize, hasDataDefinedRotation, offset, angle );

  //data defined shape?
  bool createdNewPath = false;
  bool ok = true;
  Qgis::MarkerShape symbol = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( encodeShape( symbol ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      const Qgis::MarkerShape decoded = decodeShape( exprVal.toString(), &ok );
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
    if ( mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits && context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
    {
      // rendering for symbol previews -- a size in meters in map units can't be calculated, so treat the size as millimeters
      // and clamp it to a reasonable range. It's the best we can do in this situation!
      s = std::min( std::max( context.renderContext().convertToPainterUnits( mSize, QgsUnitTypes::RenderMillimeters ), 3.0 ), 100.0 );
    }
    const double half = s / 2.0;
    transform.scale( half, half );
  }

  if ( !qgsDoubleNear( angle, 0.0 ) && ( hasDataDefinedRotation || createdNewPath ) )
  {
    transform.rotate( angle );
  }

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

Qgis::MarkerShape QgsSimpleMarkerSymbolLayerBase::decodeShape( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == QLatin1String( "square" ) || cleaned == QLatin1String( "rectangle" ) )
    return Qgis::MarkerShape::Square;
  else if ( cleaned == QLatin1String( "square_with_corners" ) )
    return Qgis::MarkerShape::SquareWithCorners;
  else if ( cleaned == QLatin1String( "diamond" ) )
    return Qgis::MarkerShape::Diamond;
  else if ( cleaned == QLatin1String( "pentagon" ) )
    return Qgis::MarkerShape::Pentagon;
  else if ( cleaned == QLatin1String( "hexagon" ) )
    return Qgis::MarkerShape::Hexagon;
  else if ( cleaned == QLatin1String( "octagon" ) )
    return Qgis::MarkerShape::Octagon;
  else if ( cleaned == QLatin1String( "triangle" ) )
    return Qgis::MarkerShape::Triangle;
  else if ( cleaned == QLatin1String( "equilateral_triangle" ) )
    return Qgis::MarkerShape::EquilateralTriangle;
  else if ( cleaned == QLatin1String( "star" ) || cleaned == QLatin1String( "regular_star" ) )
    return Qgis::MarkerShape::Star;
  else if ( cleaned == QLatin1String( "arrow" ) )
    return Qgis::MarkerShape::Arrow;
  else if ( cleaned == QLatin1String( "circle" ) )
    return Qgis::MarkerShape::Circle;
  else if ( cleaned == QLatin1String( "cross" ) )
    return Qgis::MarkerShape::Cross;
  else if ( cleaned == QLatin1String( "cross_fill" ) )
    return Qgis::MarkerShape::CrossFill;
  else if ( cleaned == QLatin1String( "cross2" ) || cleaned == QLatin1String( "x" ) )
    return Qgis::MarkerShape::Cross2;
  else if ( cleaned == QLatin1String( "line" ) )
    return Qgis::MarkerShape::Line;
  else if ( cleaned == QLatin1String( "arrowhead" ) )
    return Qgis::MarkerShape::ArrowHead;
  else if ( cleaned == QLatin1String( "filled_arrowhead" ) )
    return Qgis::MarkerShape::ArrowHeadFilled;
  else if ( cleaned == QLatin1String( "semi_circle" ) )
    return Qgis::MarkerShape::SemiCircle;
  else if ( cleaned == QLatin1String( "third_circle" ) )
    return Qgis::MarkerShape::ThirdCircle;
  else if ( cleaned == QLatin1String( "quarter_circle" ) )
    return Qgis::MarkerShape::QuarterCircle;
  else if ( cleaned == QLatin1String( "quarter_square" ) )
    return Qgis::MarkerShape::QuarterSquare;
  else if ( cleaned == QLatin1String( "half_square" ) )
    return Qgis::MarkerShape::HalfSquare;
  else if ( cleaned == QLatin1String( "diagonal_half_square" ) )
    return Qgis::MarkerShape::DiagonalHalfSquare;
  else if ( cleaned == QLatin1String( "right_half_triangle" ) )
    return Qgis::MarkerShape::RightHalfTriangle;
  else if ( cleaned == QLatin1String( "left_half_triangle" ) )
    return Qgis::MarkerShape::LeftHalfTriangle;
  else if ( cleaned == QLatin1String( "asterisk_fill" ) )
    return Qgis::MarkerShape::AsteriskFill;
  else if ( cleaned == QLatin1String( "half_arc" ) )
    return Qgis::MarkerShape::HalfArc;
  else if ( cleaned == QLatin1String( "third_arc" ) )
    return Qgis::MarkerShape::ThirdArc;
  else if ( cleaned == QLatin1String( "quarter_arc" ) )
    return Qgis::MarkerShape::QuarterArc;

  if ( ok )
    *ok = false;
  return Qgis::MarkerShape::Circle;
}

QString QgsSimpleMarkerSymbolLayerBase::encodeShape( Qgis::MarkerShape shape )
{
  switch ( shape )
  {
    case Qgis::MarkerShape::Square:
      return QStringLiteral( "square" );
    case Qgis::MarkerShape::QuarterSquare:
      return QStringLiteral( "quarter_square" );
    case Qgis::MarkerShape::HalfSquare:
      return QStringLiteral( "half_square" );
    case Qgis::MarkerShape::DiagonalHalfSquare:
      return QStringLiteral( "diagonal_half_square" );
    case Qgis::MarkerShape::Diamond:
      return QStringLiteral( "diamond" );
    case Qgis::MarkerShape::Pentagon:
      return QStringLiteral( "pentagon" );
    case Qgis::MarkerShape::Hexagon:
      return QStringLiteral( "hexagon" );
    case Qgis::MarkerShape::Octagon:
      return QStringLiteral( "octagon" );
    case Qgis::MarkerShape::SquareWithCorners:
      return QStringLiteral( "square_with_corners" );
    case Qgis::MarkerShape::Triangle:
      return QStringLiteral( "triangle" );
    case Qgis::MarkerShape::EquilateralTriangle:
      return QStringLiteral( "equilateral_triangle" );
    case Qgis::MarkerShape::LeftHalfTriangle:
      return QStringLiteral( "left_half_triangle" );
    case Qgis::MarkerShape::RightHalfTriangle:
      return QStringLiteral( "right_half_triangle" );
    case Qgis::MarkerShape::Star:
      return QStringLiteral( "star" );
    case Qgis::MarkerShape::Arrow:
      return QStringLiteral( "arrow" );
    case Qgis::MarkerShape::ArrowHeadFilled:
      return QStringLiteral( "filled_arrowhead" );
    case Qgis::MarkerShape::CrossFill:
      return QStringLiteral( "cross_fill" );
    case Qgis::MarkerShape::Circle:
      return QStringLiteral( "circle" );
    case Qgis::MarkerShape::Cross:
      return QStringLiteral( "cross" );
    case Qgis::MarkerShape::Cross2:
      return QStringLiteral( "cross2" );
    case Qgis::MarkerShape::Line:
      return QStringLiteral( "line" );
    case Qgis::MarkerShape::ArrowHead:
      return QStringLiteral( "arrowhead" );
    case Qgis::MarkerShape::SemiCircle:
      return QStringLiteral( "semi_circle" );
    case Qgis::MarkerShape::ThirdCircle:
      return QStringLiteral( "third_circle" );
    case Qgis::MarkerShape::QuarterCircle:
      return QStringLiteral( "quarter_circle" );
    case Qgis::MarkerShape::AsteriskFill:
      return QStringLiteral( "asterisk_fill" );
    case Qgis::MarkerShape::HalfArc:
      return QStringLiteral( "half_arc" );
    case Qgis::MarkerShape::ThirdArc:
      return QStringLiteral( "third_arc" );
    case Qgis::MarkerShape::QuarterArc:
      return QStringLiteral( "quarter_arc" );
  }
  return QString();
}

bool QgsSimpleMarkerSymbolLayerBase::prepareMarkerShape( Qgis::MarkerShape shape )
{
  return shapeToPolygon( shape, mPolygon );
}

bool QgsSimpleMarkerSymbolLayerBase::shapeToPolygon( Qgis::MarkerShape shape, QPolygonF &polygon ) const
{
  polygon.clear();

  switch ( shape )
  {
    case Qgis::MarkerShape::Square:
      polygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 1, 1 ) ) );
      return true;

    case Qgis::MarkerShape::SquareWithCorners:
    {
      static constexpr double VERTEX_OFFSET_FROM_ORIGIN = 0.6072;

      polygon << QPointF( - VERTEX_OFFSET_FROM_ORIGIN, 1 )
              << QPointF( VERTEX_OFFSET_FROM_ORIGIN, 1 )
              << QPointF( 1, VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( 1, -VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( VERTEX_OFFSET_FROM_ORIGIN, -1 )
              << QPointF( -VERTEX_OFFSET_FROM_ORIGIN, -1 )
              << QPointF( -1, -VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( -1, VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( -VERTEX_OFFSET_FROM_ORIGIN, 1 );
      return true;
    }

    case Qgis::MarkerShape::QuarterSquare:
      polygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 0, 0 ) ) );
      return true;

    case Qgis::MarkerShape::HalfSquare:
      polygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 0, 1 ) ) );
      return true;

    case Qgis::MarkerShape::DiagonalHalfSquare:
      polygon << QPointF( -1, -1 ) << QPointF( 1, 1 ) << QPointF( -1, 1 ) << QPointF( -1, -1 );
      return true;

    case Qgis::MarkerShape::Diamond:
      polygon << QPointF( -1, 0 ) << QPointF( 0, 1 )
              << QPointF( 1, 0 ) << QPointF( 0, -1 ) << QPointF( -1, 0 );
      return true;

    case Qgis::MarkerShape::Pentagon:
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

    case Qgis::MarkerShape::Hexagon:
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

    case Qgis::MarkerShape::Octagon:
    {
      static constexpr double VERTEX_OFFSET_FROM_ORIGIN = 1.0 / ( 1 + M_SQRT2 );

      polygon << QPointF( - VERTEX_OFFSET_FROM_ORIGIN, 1 )
              << QPointF( VERTEX_OFFSET_FROM_ORIGIN, 1 )
              << QPointF( 1, VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( 1, -VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( VERTEX_OFFSET_FROM_ORIGIN, -1 )
              << QPointF( -VERTEX_OFFSET_FROM_ORIGIN, -1 )
              << QPointF( -1, -VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( -1, VERTEX_OFFSET_FROM_ORIGIN )
              << QPointF( -VERTEX_OFFSET_FROM_ORIGIN, 1 );
      return true;
    }

    case Qgis::MarkerShape::Triangle:
      polygon << QPointF( -1, 1 ) << QPointF( 1, 1 ) << QPointF( 0, -1 ) << QPointF( -1, 1 );
      return true;

    case Qgis::MarkerShape::EquilateralTriangle:
      /* angular-representation of hardcoded values used
      polygon << QPointF( std::sin( DEG2RAD( 240.0 ) ), - std::cos( DEG2RAD( 240.0 ) ) )
      << QPointF( std::sin( DEG2RAD( 120.0 ) ), - std::cos( DEG2RAD( 120.0 ) ) )
      << QPointF( 0, -1 ); */
      polygon << QPointF( -0.8660, 0.5 )
              << QPointF( 0.8660, 0.5 )
              << QPointF( 0, -1 )
              << QPointF( -0.8660, 0.5 );
      return true;

    case Qgis::MarkerShape::LeftHalfTriangle:
      polygon << QPointF( 0, 1 ) << QPointF( 1, 1 ) << QPointF( 0, -1 ) << QPointF( 0, 1 );
      return true;

    case Qgis::MarkerShape::RightHalfTriangle:
      polygon << QPointF( -1, 1 ) << QPointF( 0, 1 ) << QPointF( 0, -1 ) << QPointF( -1, 1 );
      return true;

    case Qgis::MarkerShape::Star:
    {
      const double inner_r = std::cos( DEG2RAD( 72.0 ) ) / std::cos( DEG2RAD( 36.0 ) );

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

    case Qgis::MarkerShape::Arrow:
      polygon << QPointF( 0, -1 )
              << QPointF( 0.5,  -0.5 )
              << QPointF( 0.25, -0.5 )
              << QPointF( 0.25,  1 )
              << QPointF( -0.25,  1 )
              << QPointF( -0.25, -0.5 )
              << QPointF( -0.5,  -0.5 )
              << QPointF( 0, -1 );
      return true;

    case Qgis::MarkerShape::ArrowHeadFilled:
      polygon << QPointF( 0, 0 ) << QPointF( -1, 1 ) << QPointF( -1, -1 ) << QPointF( 0, 0 );
      return true;

    case Qgis::MarkerShape::CrossFill:
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

    case Qgis::MarkerShape::AsteriskFill:
    {
      static constexpr double THICKNESS = 0.3;
      static constexpr double HALF_THICKNESS = THICKNESS / 2.0;
      static constexpr double INTERSECTION_POINT = THICKNESS / M_SQRT2;
      static constexpr double DIAGONAL1 = M_SQRT1_2 - INTERSECTION_POINT * 0.5;
      static constexpr double DIAGONAL2 = M_SQRT1_2 + INTERSECTION_POINT * 0.5;

      polygon << QPointF( -HALF_THICKNESS, -1 )
              << QPointF( HALF_THICKNESS, -1 )
              << QPointF( HALF_THICKNESS, -HALF_THICKNESS - INTERSECTION_POINT )
              << QPointF( DIAGONAL1, -DIAGONAL2 )
              << QPointF( DIAGONAL2, -DIAGONAL1 )
              << QPointF( HALF_THICKNESS + INTERSECTION_POINT, -HALF_THICKNESS )
              << QPointF( 1, -HALF_THICKNESS )
              << QPointF( 1, HALF_THICKNESS )
              << QPointF( HALF_THICKNESS + INTERSECTION_POINT, HALF_THICKNESS )
              << QPointF( DIAGONAL2, DIAGONAL1 )
              << QPointF( DIAGONAL1, DIAGONAL2 )
              << QPointF( HALF_THICKNESS, HALF_THICKNESS + INTERSECTION_POINT )
              << QPointF( HALF_THICKNESS, 1 )
              << QPointF( -HALF_THICKNESS, 1 )
              << QPointF( -HALF_THICKNESS, HALF_THICKNESS + INTERSECTION_POINT )
              << QPointF( -DIAGONAL1, DIAGONAL2 )
              << QPointF( -DIAGONAL2, DIAGONAL1 )
              << QPointF( -HALF_THICKNESS - INTERSECTION_POINT, HALF_THICKNESS )
              << QPointF( -1, HALF_THICKNESS )
              << QPointF( -1, -HALF_THICKNESS )
              << QPointF( -HALF_THICKNESS - INTERSECTION_POINT, -HALF_THICKNESS )
              << QPointF( -DIAGONAL2, -DIAGONAL1 )
              << QPointF( -DIAGONAL1, -DIAGONAL2 )
              << QPointF( -HALF_THICKNESS, -HALF_THICKNESS - INTERSECTION_POINT )
              << QPointF( -HALF_THICKNESS, -1 );
      return true;
    }

    case Qgis::MarkerShape::Circle:
    case Qgis::MarkerShape::Cross:
    case Qgis::MarkerShape::Cross2:
    case Qgis::MarkerShape::Line:
    case Qgis::MarkerShape::ArrowHead:
    case Qgis::MarkerShape::SemiCircle:
    case Qgis::MarkerShape::ThirdCircle:
    case Qgis::MarkerShape::QuarterCircle:
    case Qgis::MarkerShape::HalfArc:
    case Qgis::MarkerShape::ThirdArc:
    case Qgis::MarkerShape::QuarterArc:
      return false;
  }

  return false;
}

bool QgsSimpleMarkerSymbolLayerBase::prepareMarkerPath( Qgis::MarkerShape symbol )
{
  mPath = QPainterPath();

  switch ( symbol )
  {
    case Qgis::MarkerShape::Circle:

      mPath.addEllipse( QRectF( -1, -1, 2, 2 ) ); // x,y,w,h
      return true;

    case Qgis::MarkerShape::SemiCircle:
      mPath.arcTo( -1, -1, 2, 2, 0, 180 );
      mPath.lineTo( 0, 0 );
      return true;

    case Qgis::MarkerShape::ThirdCircle:
      mPath.arcTo( -1, -1, 2, 2, 90, 120 );
      mPath.lineTo( 0, 0 );
      return true;

    case Qgis::MarkerShape::QuarterCircle:
      mPath.arcTo( -1, -1, 2, 2, 90, 90 );
      mPath.lineTo( 0, 0 );
      return true;

    case Qgis::MarkerShape::HalfArc:
      mPath.moveTo( 1, 0 );
      mPath.arcTo( -1, -1, 2, 2, 0, 180 );
      return true;

    case Qgis::MarkerShape::ThirdArc:
      mPath.moveTo( 0, -1 );
      mPath.arcTo( -1, -1, 2, 2, 90, 120 );
      return true;

    case Qgis::MarkerShape::QuarterArc:
      mPath.moveTo( 0, -1 );
      mPath.arcTo( -1, -1, 2, 2, 90, 90 );
      return true;

    case Qgis::MarkerShape::Cross:
      mPath.moveTo( -1, 0 );
      mPath.lineTo( 1, 0 ); // horizontal
      mPath.moveTo( 0, -1 );
      mPath.lineTo( 0, 1 ); // vertical
      return true;

    case Qgis::MarkerShape::Cross2:
      mPath.moveTo( -1, -1 );
      mPath.lineTo( 1, 1 );
      mPath.moveTo( 1, -1 );
      mPath.lineTo( -1, 1 );
      return true;

    case Qgis::MarkerShape::Line:
      mPath.moveTo( 0, -1 );
      mPath.lineTo( 0, 1 ); // vertical line
      return true;

    case Qgis::MarkerShape::ArrowHead:
      mPath.moveTo( -1, -1 );
      mPath.lineTo( 0, 0 );
      mPath.lineTo( -1, 1 );
      return true;

    case Qgis::MarkerShape::Square:
    case Qgis::MarkerShape::SquareWithCorners:
    case Qgis::MarkerShape::QuarterSquare:
    case Qgis::MarkerShape::HalfSquare:
    case Qgis::MarkerShape::DiagonalHalfSquare:
    case Qgis::MarkerShape::Diamond:
    case Qgis::MarkerShape::Pentagon:
    case Qgis::MarkerShape::Hexagon:
    case Qgis::MarkerShape::Octagon:
    case Qgis::MarkerShape::Triangle:
    case Qgis::MarkerShape::EquilateralTriangle:
    case Qgis::MarkerShape::LeftHalfTriangle:
    case Qgis::MarkerShape::RightHalfTriangle:
    case Qgis::MarkerShape::Star:
    case Qgis::MarkerShape::Arrow:
    case Qgis::MarkerShape::ArrowHeadFilled:
    case Qgis::MarkerShape::CrossFill:
    case Qgis::MarkerShape::AsteriskFill:
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
      case Qgis::ScaleMethod::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case Qgis::ScaleMethod::ScaleDiameter:
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

  hasDataDefinedRotation = false;
  //angle
  bool ok = true;
  angle = mAngle + mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), 0, &ok ) + mLineAngle;

    // If the expression evaluation was not successful, fallback to static value
    if ( !ok )
      angle = mAngle + mLineAngle;

    hasDataDefinedRotation = true;
  }

  hasDataDefinedRotation = context.renderHints() & Qgis::SymbolRenderHint::DynamicRotation || hasDataDefinedRotation;

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

QgsSimpleMarkerSymbolLayer::QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape shape, double size, double angle, Qgis::ScaleMethod scaleMethod, const QColor &color, const QColor &strokeColor, Qt::PenJoinStyle penJoinStyle )
  : QgsSimpleMarkerSymbolLayerBase( shape, size, angle, scaleMethod )
  , mStrokeColor( strokeColor )
  , mPenJoinStyle( penJoinStyle )
{
  mColor = color;
}

QgsSimpleMarkerSymbolLayer::~QgsSimpleMarkerSymbolLayer() = default;

QgsSymbolLayer *QgsSimpleMarkerSymbolLayer::create( const QVariantMap &props )
{
  Qgis::MarkerShape shape = Qgis::MarkerShape::Circle;
  QColor color = DEFAULT_SIMPLEMARKER_COLOR;
  QColor strokeColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR;
  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;
  Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "name" ) ) )
  {
    shape = decodeShape( props[QStringLiteral( "name" )].toString() );
  }
  if ( props.contains( QStringLiteral( "color" ) ) )
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )].toString() );
  if ( props.contains( QStringLiteral( "color_border" ) ) )
  {
    //pre 2.5 projects use "color_border"
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color_border" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )].toString() );
  }
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
  {
    penJoinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )].toString() );
  }
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )].toString() );

  QgsSimpleMarkerSymbolLayer *m = new QgsSimpleMarkerSymbolLayer( shape, size, angle, scaleMethod, color, strokeColor, penJoinStyle );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "outline_style" ) ) )
  {
    m->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "outline_style" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "line_style" ) ) )
  {
    m->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "line_style" )].toString() ) );
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
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    m->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "outline_width_map_unit_scale" )].toString() ) );
  }

  if ( props.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
  {
    m->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( props[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  }
  if ( props.contains( QStringLiteral( "vertical_anchor_point" ) ) )
  {
    m->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( props[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );
  }

  if ( props.contains( QStringLiteral( "cap_style" ) ) )
  {
    m->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( props[QStringLiteral( "cap_style" )].toString() ) );
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
  mPen.setCapStyle( mPenCapStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );

  QColor selBrushColor = context.renderContext().selectionColor();
  QColor selPenColor = selBrushColor == mColor ? selBrushColor : mStrokeColor;
  if ( context.opacity() < 1  && !SELECTION_IS_OPAQUE )
  {
    selBrushColor.setAlphaF( context.opacity() );
    selPenColor.setAlphaF( context.opacity() );
  }
  mSelBrush = QBrush( selBrushColor );
  mSelPen = QPen( selPenColor );
  mSelPen.setStyle( mStrokeStyle );
  mSelPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );

  const bool hasDataDefinedRotation = context.renderHints() & Qgis::SymbolRenderHint::DynamicRotation || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
  const bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  // use caching only when:
  // - size, rotation, shape, color, stroke color is not data-defined
  // - drawing to screen (not printer)
  mUsingCache = !hasDataDefinedRotation && !hasDataDefinedSize && !context.renderContext().forceVectorOutput()
                && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor )
                && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle )
                && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle );

  if ( mUsingCache )
    mCachedOpacity = context.opacity();

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
  if ( mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits && context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
  {
    // rendering for symbol previews -- a size in meters in map units can't be calculated, so treat the size as millimeters
    // and clamp it to a reasonable range. It's the best we can do in this situation!
    scaledSize = std::min( std::max( context.renderContext().convertToPainterUnits( mSize, QgsUnitTypes::RenderMillimeters ), 3.0 ), 100.0 );
  }

  // take into account angle (which is not data-defined otherwise cache wouldn't be used)
  if ( !qgsDoubleNear( mAngle, 0.0 ) )
  {
    scaledSize = ( std::abs( std::sin( mAngle * M_PI / 180 ) ) + std::abs( std::cos( mAngle * M_PI / 180 ) ) ) * scaledSize;
  }
  // calculate necessary image size for the cache
  const double pw = static_cast< int >( std::round( ( ( qgsDoubleNear( mPen.widthF(), 0.0 ) ? 1 : mPen.widthF() * 4 ) + 1 ) ) ) / 2 * 2; // make even (round up); handle cosmetic pen
  const int imageSize = ( static_cast< int >( scaledSize ) + pw ) / 2 * 2 + 1; //  make image width, height odd; account for pen width
  const double center = imageSize / 2.0;
  if ( imageSize > MAXIMUM_CACHE_WIDTH )
  {
    return false;
  }

  mCache = QImage( QSize( imageSize, imageSize ), QImage::Format_ARGB32_Premultiplied );
  mCache.fill( 0 );

  const bool needsBrush = shapeIsFilled( mShape );

  QPainter p;
  p.begin( &mCache );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( needsBrush ? mBrush : Qt::NoBrush );
  p.setPen( mPen );
  p.translate( QPointF( center, center ) );
  drawMarker( &p, context );
  p.end();

  // Construct the selected version of the Cache

  const QColor selColor = context.renderContext().selectionColor();

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

void QgsSimpleMarkerSymbolLayer::draw( QgsSymbolRenderContext &context, Qgis::MarkerShape shape, const QPolygonF &polygon, const QPainterPath &path )
{
  //making changes here? Don't forget to also update ::bounds if the changes affect the bounding box
  //of the rendered point!

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QColor brushColor = mColor;
  brushColor.setAlphaF( brushColor.alphaF() * context.opacity() );
  mBrush.setColor( brushColor );

  QColor penColor = mStrokeColor;
  penColor.setAlphaF( penColor.alphaF() * context.opacity() );
  mPen.setColor( penColor );

  bool ok = true;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    QColor c = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor, &ok );
    if ( ok )
    {
      c.setAlphaF( c.alphaF() * context.opacity() );
      mBrush.setColor( c );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    QColor c = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor, &ok );
    if ( ok )
    {
      c.setAlphaF( c.alphaF() * context.opacity() );
      mPen.setColor( c );
      mSelPen.setColor( c );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    const double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), 0, &ok );
    if ( ok )
    {
      mPen.setWidthF( context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );
      mSelPen.setWidthF( context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    const QString strokeStyle = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( strokeStyle ) );
      mSelPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( strokeStyle ) );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    const QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
      mSelPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle ) );
    const QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyCapStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( style ) );
      mSelPen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( style ) );
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

  if ( mUsingCache && qgsDoubleNear( mCachedOpacity, context.opacity() ) )
  {
    const QImage &img = context.selected() ? mSelCache : mCache;
    const double s = img.width();

    bool hasDataDefinedSize = false;
    const double scaledSize = calculateSize( context, hasDataDefinedSize );

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

QVariantMap QgsSimpleMarkerSymbolLayer::properties() const
{
  QVariantMap map;
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
  map[QStringLiteral( "cap_style" )] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
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
  m->setPenCapStyle( mPenCapStyle );
  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsSimpleMarkerSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  const double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mStrokeWidth, mStrokeWidthUnit, props );
  const double size = QgsSymbolLayerUtils::rescaleUom( mSize, mSizeUnit, props );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, encodeShape( mShape ), mColor, mStrokeColor, mStrokeStyle, strokeWidth, size );

  // <Rotation>
  QString angleFunc;
  bool ok;
  const double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toString() ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  const QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );
}

QString QgsSimpleMarkerSymbolLayer::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  Q_UNUSED( mmScaleFactor )
  Q_UNUSED( mapUnitScaleFactor )
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
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

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
    const double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerUtils::displacementFromSldElement( graphicElem, offset );

  const Qgis::MarkerShape shape = decodeShape( name );

  const QString uom = element.attribute( QStringLiteral( "uom" ) );
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
  Q_UNUSED( context )

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
  //data defined size?
  double size = mSize;

  const bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

  //data defined size
  bool ok = true;
  if ( hasDataDefinedSize )
  {
    size = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertySize, context.renderContext().expressionContext(), mSize, &ok );

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

    size *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mSizeUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  }

  if ( mSizeUnit == QgsUnitTypes::RenderMillimeters )
  {
    size *= mmMapUnitScaleFactor;
  }

  if ( mSizeUnit == QgsUnitTypes::RenderMapUnits )
  {
    e.clipValueToMapUnitScale( size, mSizeMapUnitScale, context.renderContext().scaleFactor() );
  }
  const double halfSize = size / 2.0;

  //strokeWidth
  double strokeWidth = mStrokeWidth;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  strokeWidth *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
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

  Qgis::MarkerShape shape = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( encodeShape( shape ) );
    const QString shapeName = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      shape = decodeShape( shapeName, &ok );
      if ( !ok )
        shape = mShape;
    }
  }

  if ( angle )
    off = _rotatedOffset( off, angle );

  off *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mSizeUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );

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
  else if ( shape == Qgis::MarkerShape::Circle )
  {
    shift += QPointF( off.x(), -off.y() );
    if ( mBrush.style() != Qt::NoBrush )
      e.writeFilledCircle( layerName, bc, QgsPoint( shift ), halfSize );
    if ( mPen.style() != Qt::NoPen )
      e.writeCircle( layerName, pc, QgsPoint( shift ), halfSize, QStringLiteral( "CONTINUOUS" ), strokeWidth );
  }
  else if ( shape == Qgis::MarkerShape::Line )
  {
    const QPointF pt1 = t.map( QPointF( 0, -halfSize ) );
    const QPointF pt2 = t.map( QPointF( 0, halfSize ) );

    if ( mPen.style() != Qt::NoPen )
      e.writeLine( QgsPoint( pt1 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
  }
  else if ( shape == Qgis::MarkerShape::Cross )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      const QPointF pt1 = t.map( QPointF( -halfSize, 0 ) );
      const QPointF pt2 = t.map( QPointF( halfSize, 0 ) );
      const QPointF pt3 = t.map( QPointF( 0, -halfSize ) );
      const QPointF pt4 = t.map( QPointF( 0, halfSize ) );

      e.writeLine( QgsPoint( pt1 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
      e.writeLine( QgsPoint( pt3 ), QgsPoint( pt4 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
    }
  }
  else if ( shape == Qgis::MarkerShape::Cross2 )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      const QPointF pt1 = t.map( QPointF( -halfSize, -halfSize ) );
      const QPointF pt2 = t.map( QPointF( halfSize, halfSize ) );
      const QPointF pt3 = t.map( QPointF( halfSize, -halfSize ) );
      const QPointF pt4 = t.map( QPointF( -halfSize, halfSize ) );

      e.writeLine( QgsPoint( pt1 ), QgsPoint( pt2 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
      e.writeLine( QgsPoint( pt3 ), QgsPoint( pt4 ), layerName, QStringLiteral( "CONTINUOUS" ), pc, strokeWidth );
    }
  }
  else if ( shape == Qgis::MarkerShape::ArrowHead )
  {
    if ( mPen.style() != Qt::NoPen )
    {
      const QPointF pt1 = t.map( QPointF( -halfSize, halfSize ) );
      const QPointF pt2 = t.map( QPointF( 0, 0 ) );
      const QPointF pt3 = t.map( QPointF( -halfSize, -halfSize ) );

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

bool QgsSimpleMarkerSymbolLayer::usesMapUnits() const
{
  return mSizeUnit == QgsUnitTypes::RenderMapUnits || mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mStrokeWidthUnit == QgsUnitTypes::RenderMapUnits || mStrokeWidthUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

QRectF QgsSimpleMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  QRectF symbolBounds = QgsSimpleMarkerSymbolLayerBase::bounds( point, context );

  // need to account for stroke width
  double penWidth = mStrokeWidth;
  bool ok = true;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    const double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth, &ok );
    if ( ok )
    {
      penWidth = strokeWidth;
    }
  }
  penWidth = context.renderContext().convertToPainterUnits( penWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    const QString strokeStyle = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok && strokeStyle == QLatin1String( "no" ) )
    {
      penWidth = 0.0;
    }
  }
  else if ( mStrokeStyle == Qt::NoPen )
    penWidth = 0;

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

QgsFilledMarkerSymbolLayer::QgsFilledMarkerSymbolLayer( Qgis::MarkerShape shape, double size, double angle, Qgis::ScaleMethod scaleMethod )
  : QgsSimpleMarkerSymbolLayerBase( shape, size, angle, scaleMethod )
{
  mFill.reset( static_cast<QgsFillSymbol *>( QgsFillSymbol::createSimple( QVariantMap() ) ) );
}

QgsFilledMarkerSymbolLayer::~QgsFilledMarkerSymbolLayer() = default;

QgsSymbolLayer *QgsFilledMarkerSymbolLayer::create( const QVariantMap &props )
{
  QString name = DEFAULT_SIMPLEMARKER_NAME;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;
  Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "name" ) ) )
    name = props[QStringLiteral( "name" )].toString();
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )].toString() );

  QgsFilledMarkerSymbolLayer *m = new QgsFilledMarkerSymbolLayer( decodeShape( name ), size, angle, scaleMethod );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )].toString() ) );
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

QVariantMap QgsFilledMarkerSymbolLayer::properties() const
{
  QVariantMap map;
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
  if ( symbol && symbol->type() == Qgis::SymbolType::Fill )
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

bool QgsFilledMarkerSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mFill && mFill->hasDataDefinedProperties() )
    return true;
  return false;
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

bool QgsFilledMarkerSymbolLayer::usesMapUnits() const
{
  return mSizeUnit == QgsUnitTypes::RenderMapUnits || mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits
         || ( mFill && mFill->usesMapUnits() );
}

void QgsFilledMarkerSymbolLayer::draw( QgsSymbolRenderContext &context, Qgis::MarkerShape shape, const QPolygonF &polygon, const QPainterPath &path )
{
  //making changes here? Don't forget to also update ::bounds if the changes affect the bounding box
  //of the rendered point!

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  const double prevOpacity = mFill->opacity();
  mFill->setOpacity( mFill->opacity() * context.opacity() );

  if ( shapeIsFilled( shape ) )
  {
    p->setBrush( Qt::red );
  }
  else
  {
    p->setBrush( Qt::NoBrush );
  }
  p->setPen( Qt::black );

  const bool prevIsSubsymbol = context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  if ( !polygon.isEmpty() )
  {
    mFill->renderPolygon( polygon, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
  }
  else
  {
    const QPolygonF poly = path.toFillPolygon();
    mFill->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
  }

  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );

  mFill->setOpacity( prevOpacity );
}


//////////


QgsSvgMarkerSymbolLayer::QgsSvgMarkerSymbolLayer( const QString &path, double size, double angle, Qgis::ScaleMethod scaleMethod )
{
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  mStrokeWidth = 0.2;
  mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
  mColor = QColor( 35, 35, 35 );
  mStrokeColor = QColor( 35, 35, 35 );
  setPath( path );
}

QgsSvgMarkerSymbolLayer::~QgsSvgMarkerSymbolLayer() = default;

QgsSymbolLayer *QgsSvgMarkerSymbolLayer::create( const QVariantMap &props )
{
  QString name;
  double size = DEFAULT_SVGMARKER_SIZE;
  double angle = DEFAULT_SVGMARKER_ANGLE;
  Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "name" ) ) )
    name = props[QStringLiteral( "name" )].toString();
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )].toString() );

  QgsSvgMarkerSymbolLayer *m = new QgsSvgMarkerSymbolLayer( name, size, angle, scaleMethod );

  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "fixedAspectRatio" ) ) )
    m->setFixedAspectRatio( props[QStringLiteral( "fixedAspectRatio" )].toDouble() );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "fill" ) ) )
  {
    //pre 2.5 projects used "fill"
    m->setFillColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "fill" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    m->setFillColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "outline" ) ) )
  {
    //pre 2.5 projects used "outline"
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )].toString() ) );
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
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
    m->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "outline_width_map_unit_scale" )].toString() ) );

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

  if ( props.contains( QStringLiteral( "parameters" ) ) )
  {
    const QVariantMap parameters = props[QStringLiteral( "parameters" )].toMap();
    m->setParameters( QgsProperty::variantMapToPropertyMap( parameters ) );
  }

  return m;
}

void QgsSvgMarkerSymbolLayer::resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  const QVariantMap::iterator it = properties.find( QStringLiteral( "name" ) );
  if ( it != properties.end() )
  {
    if ( saving )
    {
      it.value() = QgsSymbolLayerUtils::svgSymbolPathToName( it.value().toString(), pathResolver );
    }
    else
    {
      it.value() = QgsSymbolLayerUtils::svgSymbolNameToPath( it.value().toString(), pathResolver );
    }
  }
}

void QgsSvgMarkerSymbolLayer::setPath( const QString &path )
{
  mDefaultAspectRatio = 0;
  mHasFillParam = false;
  mPath = path;
  QColor defaultFillColor, defaultStrokeColor;
  double strokeWidth, fillOpacity, strokeOpacity;
  bool hasFillOpacityParam = false, hasStrokeParam = false, hasStrokeWidthParam = false, hasStrokeOpacityParam = false;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultStrokeColor = false, hasDefaultStrokeWidth = false, hasDefaultStrokeOpacity = false;
  QgsApplication::svgCache()->containsParams( path, mHasFillParam, hasDefaultFillColor, defaultFillColor,
      hasFillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      hasStrokeParam, hasDefaultStrokeColor, defaultStrokeColor,
      hasStrokeWidthParam, hasDefaultStrokeWidth, strokeWidth,
      hasStrokeOpacityParam, hasDefaultStrokeOpacity, strokeOpacity );

  const double newFillOpacity = hasFillOpacityParam ? fillColor().alphaF() : 1.0;
  const double newStrokeOpacity = hasStrokeOpacityParam ? strokeColor().alphaF() : 1.0;

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
    const double size = mSize;
    //assume 88 dpi as standard value
    const double widthScaleFactor = 3.465;
    const QSizeF svgViewbox = QgsApplication::svgCache()->svgViewboxSize( mPath, size, mColor, mStrokeColor, mStrokeWidth, widthScaleFactor );
    // set default aspect ratio
    mDefaultAspectRatio = svgViewbox.isValid() ? svgViewbox.height() / svgViewbox.width() : 0.0;
  }
  return mDefaultAspectRatio;
}

bool QgsSvgMarkerSymbolLayer::setPreservedAspectRatio( bool par )
{
  const bool aPreservedAspectRatio = preservedAspectRatio();
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

void QgsSvgMarkerSymbolLayer::setParameters( const QMap<QString, QgsProperty> &parameters )
{
  mParameters = parameters;
}


QString QgsSvgMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "SvgMarker" );
}

void QgsSvgMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QgsMarkerSymbolLayer::startRender( context ); // get anchor point expressions
  Q_UNUSED( context )
}

void QgsSvgMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsSvgMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
    return;

  bool hasDataDefinedSize = false;
  const double scaledWidth = calculateSize( context, hasDataDefinedSize );
  const double width = context.renderContext().convertToPainterUnits( scaledWidth, mSizeUnit, mSizeMapUnitScale );

  //don't render symbols with a width below one or above 10,000 pixels
  if ( static_cast< int >( width ) < 1 || 10000.0 < width )
  {
    return;
  }

  const QgsScopedQPainterState painterState( p );

  bool hasDataDefinedAspectRatio = false;
  const double aspectRatio = calculateAspectRatio( context, scaledWidth, hasDataDefinedAspectRatio );
  double scaledHeight = scaledWidth * ( !qgsDoubleNear( aspectRatio, 0.0 ) ? aspectRatio : mDefaultAspectRatio );

  const QgsStringMap evaluatedParameters = QgsSymbolLayerUtils::evaluatePropertiesMap( mParameters, context.renderContext().expressionContext() );

  double strokeWidth = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  strokeWidth = context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );

  QColor fillColor = mColor;
  if ( context.selected() && mHasFillParam )
  {
    fillColor = context.renderContext().selectionColor();
  }
  else if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
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

  QString path = mPath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mPath );
    path = QgsSymbolLayerUtils::svgSymbolNameToPath( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mPath ),
           context.renderContext().pathResolver() );
    if ( path != mPath && qgsDoubleNear( aspectRatio, 0.0 ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) )
    {
      // adjust height of data defined path
      const QSizeF svgViewbox = QgsApplication::svgCache()->svgViewboxSize( path, scaledWidth, fillColor, strokeColor, strokeWidth,
                                context.renderContext().scaleFactor(), aspectRatio,
                                ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ), evaluatedParameters );
      scaledHeight = svgViewbox.isValid() ? scaledWidth * svgViewbox.height() / svgViewbox.width() : scaledWidth;
    }
  }

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledWidth, scaledHeight, outputOffset, angle );

  p->translate( point + outputOffset );

  const bool rotated = !qgsDoubleNear( angle, 0 );
  if ( rotated )
    p->rotate( angle );

  bool fitsInCache = true;
  bool usePict = true;
  const bool rasterizeSelected = !mHasFillParam || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName );
  if ( ( !context.renderContext().forceVectorOutput() && !rotated ) || ( context.selected() && rasterizeSelected ) )
  {
    QImage img = QgsApplication::svgCache()->svgAsImage( path, width, fillColor, strokeColor, strokeWidth,
                 context.renderContext().scaleFactor(), fitsInCache, aspectRatio,
                 ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ), evaluatedParameters );
    if ( fitsInCache && img.width() > 1 )
    {
      usePict = false;

      if ( context.selected() )
        QgsImageOperation::adjustHueSaturation( img, 1.0, context.renderContext().selectionColor(), 1.0, context.renderContext().feedback() );

      //consider transparency
      if ( !qgsDoubleNear( context.opacity(), 1.0 ) )
      {
        QImage transparentImage = img.copy();
        QgsSymbolLayerUtils::multiplyImageOpacity( &transparentImage, context.opacity() );
        p->drawImage( -transparentImage.width() / 2.0, -transparentImage.height() / 2.0, transparentImage );
      }
      else
      {
        p->drawImage( -img.width() / 2.0, -img.height() / 2.0, img );
      }
    }
  }

  if ( usePict || !fitsInCache )
  {
    p->setOpacity( context.opacity() );
    const QPicture pct = QgsApplication::svgCache()->svgAsPicture( path, width, fillColor, strokeColor, strokeWidth,
                         context.renderContext().scaleFactor(), context.renderContext().forceVectorOutput(), aspectRatio,
                         ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ), evaluatedParameters );
    if ( pct.width() > 1 )
    {
      const QgsScopedQPainterState painterPictureState( p );
      _fixQPictureDPI( p );
      p->drawPicture( 0, 0, pct );
    }
  }

  // workaround issue with nested QPictures forgetting antialiasing flag - see https://github.com/qgis/QGIS/issues/22909
  context.renderContext().setPainterFlagsUsingContext( p );
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
      case Qgis::ScaleMethod::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case Qgis::ScaleMethod::ScaleDiameter:
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

  const double defaultHeight = mSize * scaledAspectRatio;
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
      case Qgis::ScaleMethod::ScaleArea:
        scaledHeight = sqrt( scaledHeight );
        break;
      case Qgis::ScaleMethod::ScaleDiameter:
        break;
    }
  }

  scaledAspectRatio = scaledHeight / scaledSize;

  return scaledAspectRatio;
}

void QgsSvgMarkerSymbolLayer::calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledWidth, double scaledHeight, QPointF &offset, double &angle ) const
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

  const bool hasDataDefinedRotation = context.renderHints() & Qgis::SymbolRenderHint::DynamicRotation || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
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


QVariantMap QgsSvgMarkerSymbolLayer::properties() const
{
  QVariantMap map;
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

  map[QStringLiteral( "parameters" )] = QgsProperty::propertyMapToVariantMap( mParameters );

  return map;
}

bool QgsSvgMarkerSymbolLayer::usesMapUnits() const
{
  return mSizeUnit == QgsUnitTypes::RenderMapUnits || mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mStrokeWidthUnit == QgsUnitTypes::RenderMapUnits || mStrokeWidthUnit == QgsUnitTypes::RenderMetersInMapUnits;
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
  m->setParameters( mParameters );

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
  const QgsUnitTypes::RenderUnit unit = QgsMarkerSymbolLayer::outputUnit();
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

void QgsSvgMarkerSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  // encode a parametric SVG reference
  const double size = QgsSymbolLayerUtils::rescaleUom( mSize, mSizeUnit, props );
  const double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mStrokeWidth, mStrokeWidthUnit, props );
  QgsSymbolLayerUtils::parametricSvgToSld( doc, graphicElem, mPath, mColor, size, mStrokeColor, strokeWidth );

  // <Rotation>
  QString angleFunc;
  bool ok;
  const double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toString() ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }

  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  const QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );
}

QgsSymbolLayer *QgsSvgMarkerSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  QString path, mimeType;
  QColor fillColor;
  double size;

  if ( !QgsSymbolLayerUtils::externalGraphicFromSld( graphicElem, path, mimeType, fillColor, size ) )
    return nullptr;

  const QString uom = element.attribute( QStringLiteral( "uom" ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );

  if ( mimeType != QLatin1String( "image/svg+xml" ) )
    return nullptr;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    const double d = angleFunc.toDouble( &ok );
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
  //size
  double size = mSize;

  const bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

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
      case Qgis::ScaleMethod::ScaleArea:
        size = std::sqrt( size );
        break;
      case Qgis::ScaleMethod::ScaleDiameter:
        break;
    }
  }

  if ( mSizeUnit == QgsUnitTypes::RenderMillimeters )
  {
    size *= mmMapUnitScaleFactor;
  }

  //offset, angle
  QPointF offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    const QVariant val = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), QString() );
    const QPointF res = QgsSymbolLayerUtils::toPoint( val, &ok );
    if ( ok )
      offset = res;
  }
  const double offsetX = offset.x();
  const double offsetY = offset.y();

  QPointF outputOffset( offsetX, offsetY );

  double angle = mAngle + mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
  }

  if ( angle )
    outputOffset = _rotatedOffset( outputOffset, angle );

  outputOffset *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mOffsetUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );

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
  strokeWidth  *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );

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

  const QgsStringMap evaluatedParameters = QgsSymbolLayerUtils::evaluatePropertiesMap( mParameters, context.renderContext().expressionContext() );

  const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( path, size, fillColor, strokeColor, strokeWidth,
                                 context.renderContext().scaleFactor(), mFixedAspectRatio,
                                 ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ), evaluatedParameters );

  QSvgRenderer r( svgContent );
  if ( !r.isValid() )
    return false;

  QgsDxfPaintDevice pd( &e );
  pd.setDrawingSize( QSizeF( r.defaultSize() ) );

  QSizeF outSize( r.defaultSize() );
  outSize.scale( size, size, Qt::KeepAspectRatio );

  QPainter p;
  p.begin( &pd );
  if ( !qgsDoubleNear( angle, 0.0 ) )
  {
    p.translate( r.defaultSize().width() / 2.0, r.defaultSize().height() / 2.0 );
    p.rotate( angle );
    p.translate( -r.defaultSize().width() / 2.0, -r.defaultSize().height() / 2.0 );
  }
  pd.setShift( shift + QPointF( outputOffset.x(), -outputOffset.y() ) );
  pd.setOutputSize( QRectF( -outSize.width() / 2.0, -outSize.height() / 2.0, outSize.width(), outSize.height() ) );
  pd.setLayer( layerName );
  r.render( &p );
  p.end();
  return true;
}

QRectF QgsSvgMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  bool hasDataDefinedSize = false;
  double scaledWidth = calculateSize( context, hasDataDefinedSize );

  bool hasDataDefinedAspectRatio = false;
  const double aspectRatio = calculateAspectRatio( context, scaledWidth, hasDataDefinedAspectRatio );
  double scaledHeight = scaledWidth * ( !qgsDoubleNear( aspectRatio, 0.0 ) ? aspectRatio : mDefaultAspectRatio );

  scaledWidth = context.renderContext().convertToPainterUnits( scaledWidth, mSizeUnit, mSizeMapUnitScale );
  scaledHeight = context.renderContext().convertToPainterUnits( scaledHeight, mSizeUnit, mSizeMapUnitScale );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( scaledWidth ) < 1 || 10000.0 < scaledWidth )
  {
    return QRectF();
  }

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledWidth, scaledHeight, outputOffset, angle );

  double strokeWidth = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  strokeWidth = context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );

  QString path = mPath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mPath );
    path = QgsSymbolLayerUtils::svgSymbolNameToPath( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mPath ),
           context.renderContext().pathResolver() );
    if ( path != mPath && qgsDoubleNear( aspectRatio, 0.0 ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) )
    {
      // need to get colors to take advantage of cached SVGs
      QColor fillColor = mColor;
      if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
      {
        context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
        fillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
      }

      const QColor strokeColor = mStrokeColor;
      if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
      {
        context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
        fillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor );
      }

      const QgsStringMap evaluatedParameters = QgsSymbolLayerUtils::evaluatePropertiesMap( mParameters, context.renderContext().expressionContext() );

      // adjust height of data defined path
      const QSizeF svgViewbox = QgsApplication::svgCache()->svgViewboxSize( path, scaledWidth, fillColor, strokeColor, strokeWidth,
                                context.renderContext().scaleFactor(), aspectRatio,
                                ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ), evaluatedParameters );
      scaledHeight = svgViewbox.isValid() ? scaledWidth * svgViewbox.height() / svgViewbox.width() : scaledWidth;
    }
  }

  QTransform transform;
  // move to the desired position
  transform.translate( point.x() + outputOffset.x(), point.y() + outputOffset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  //antialiasing
  strokeWidth += 1.0 / 2.0;

  QRectF symbolBounds = transform.mapRect( QRectF( -scaledWidth / 2.0,
                        -scaledHeight / 2.0,
                        scaledWidth,
                        scaledHeight ) );

  //extend bounds by pen width / 2.0
  symbolBounds.adjust( -strokeWidth / 2.0, -strokeWidth / 2.0,
                       strokeWidth / 2.0, strokeWidth / 2.0 );

  return symbolBounds;
}

//////////

QgsRasterMarkerSymbolLayer::QgsRasterMarkerSymbolLayer( const QString &path, double size, double angle, Qgis::ScaleMethod scaleMethod )
  : mPath( path )
{
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  updateDefaultAspectRatio();
}

QgsRasterMarkerSymbolLayer::~QgsRasterMarkerSymbolLayer() = default;

QgsSymbolLayer *QgsRasterMarkerSymbolLayer::create( const QVariantMap &props )
{
  QString path;
  double size = DEFAULT_RASTERMARKER_SIZE;
  double angle = DEFAULT_RASTERMARKER_ANGLE;
  Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( QStringLiteral( "imageFile" ) ) )
    path = props[QStringLiteral( "imageFile" )].toString();
  if ( props.contains( QStringLiteral( "size" ) ) )
    size = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();
  if ( props.contains( QStringLiteral( "scale_method" ) ) )
    scaleMethod = QgsSymbolLayerUtils::decodeScaleMethod( props[QStringLiteral( "scale_method" )].toString() );

  std::unique_ptr< QgsRasterMarkerSymbolLayer > m = std::make_unique< QgsRasterMarkerSymbolLayer >( path, size, angle, scaleMethod );
  m->setCommonProperties( props );
  return m.release();
}

void QgsRasterMarkerSymbolLayer::setCommonProperties( const QVariantMap &properties )
{
  if ( properties.contains( QStringLiteral( "alpha" ) ) )
  {
    setOpacity( properties[QStringLiteral( "alpha" )].toDouble() );
  }

  if ( properties.contains( QStringLiteral( "size_unit" ) ) )
    setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "size_unit" )].toString() ) );
  if ( properties.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "size_map_unit_scale" )].toString() ) );
  if ( properties.contains( QStringLiteral( "fixedAspectRatio" ) ) )
    setFixedAspectRatio( properties[QStringLiteral( "fixedAspectRatio" )].toDouble() );

  if ( properties.contains( QStringLiteral( "offset" ) ) )
    setOffset( QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "offset" )].toString() ) );
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
    setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );

  if ( properties.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
  {
    setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( properties[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  }
  if ( properties.contains( QStringLiteral( "vertical_anchor_point" ) ) )
  {
    setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( properties[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );
  }

  restoreOldDataDefinedProperties( properties );
  updateDefaultAspectRatio();
}

void QgsRasterMarkerSymbolLayer::resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  const QVariantMap::iterator it = properties.find( QStringLiteral( "name" ) );
  if ( it != properties.end() && it.value().type() == QVariant::String )
  {
    if ( saving )
      it.value() = QgsSymbolLayerUtils::svgSymbolPathToName( it.value().toString(), pathResolver );
    else
      it.value() =  QgsSymbolLayerUtils::svgSymbolNameToPath( it.value().toString(), pathResolver );
  }
}

void QgsRasterMarkerSymbolLayer::setPath( const QString &path )
{
  mPath = path;
  updateDefaultAspectRatio();
}

bool QgsRasterMarkerSymbolLayer::setPreservedAspectRatio( bool par )
{
  const bool aPreservedAspectRatio = preservedAspectRatio();
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
    const QSize size = QgsApplication::imageCache()->originalSize( mPath );
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

  QString path = mPath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mPath );
    path = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mPath );
  }

  if ( path.isEmpty() )
    return;

  double width = 0.0;
  double height = 0.0;

  bool hasDataDefinedSize = false;
  const double scaledSize = calculateSize( context, hasDataDefinedSize );

  bool hasDataDefinedAspectRatio = false;
  const double aspectRatio = calculateAspectRatio( context, scaledSize, hasDataDefinedAspectRatio );

  QPointF outputOffset;
  double angle = 0.0;

  // RenderPercentage Unit Type takes original image size
  if ( mSizeUnit == QgsUnitTypes::RenderPercentage )
  {
    const QSize size = QgsApplication::imageCache()->originalSize( path );
    if ( size.isEmpty() )
      return;

    width = ( scaledSize * static_cast< double >( size.width() ) ) / 100.0;
    height = ( scaledSize * static_cast< double >( size.height() ) ) / 100.0;

    // don't render symbols with size below one or above 10,000 pixels
    if ( static_cast< int >( width ) < 1 || 10000.0 < width || static_cast< int >( height ) < 1 || 10000.0 < height )
      return;

    calculateOffsetAndRotation( context, width, height, outputOffset, angle );
  }
  else
  {
    width = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );
    height = width * ( preservedAspectRatio() ? defaultAspectRatio() : aspectRatio );

    if ( preservedAspectRatio() && path != mPath )
    {
      const QSize size = QgsApplication::imageCache()->originalSize( path );
      if ( !size.isNull() && size.isValid() && size.width() > 0 )
      {
        height = width * ( static_cast< double >( size.height() ) / static_cast< double >( size.width() ) );
      }
    }

    // don't render symbols with size below one or above 10,000 pixels
    if ( static_cast< int >( width ) < 1 || 10000.0 < width )
      return;

    calculateOffsetAndRotation( context, scaledSize, scaledSize * ( height / width ), outputOffset, angle );
  }

  const QgsScopedQPainterState painterState( p );
  p->translate( point + outputOffset );

  const bool rotated = !qgsDoubleNear( angle, 0 );
  if ( rotated )
    p->rotate( angle );

  double opacity = mOpacity;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOpacity ) )
  {
    context.setOriginalValueVariable( mOpacity );
    opacity = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOpacity, context.renderContext().expressionContext(), opacity * 100 ) / 100.0;
  }
  opacity *= context.opacity();

  QImage img = fetchImage( context.renderContext(), path, QSize( width, preservedAspectRatio() ? 0 : width * aspectRatio ), preservedAspectRatio(), opacity );
  if ( !img.isNull() )
  {
    if ( context.selected() )
    {
      QgsImageOperation::adjustHueSaturation( img, 1.0, context.renderContext().selectionColor(), 1.0, context.renderContext().feedback() );
    }

    p->drawImage( -img.width() / 2.0, -img.height() / 2.0, img );
  }
}

QImage QgsRasterMarkerSymbolLayer::fetchImage( QgsRenderContext &context, const QString &path, QSize size, bool preserveAspectRatio, double opacity ) const
{
  bool cached = false;
  return QgsApplication::imageCache()->pathAsImage( path, size, preserveAspectRatio, opacity, cached, context.flags() & Qgis::RenderContextFlag::RenderBlocking );
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
      case Qgis::ScaleMethod::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case Qgis::ScaleMethod::ScaleDiameter:
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

  const double defaultHeight = mSize * scaledAspectRatio;
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
      case Qgis::ScaleMethod::ScaleArea:
        scaledHeight = sqrt( scaledHeight );
        break;
      case Qgis::ScaleMethod::ScaleDiameter:
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

  const bool hasDataDefinedRotation = context.renderHints() & Qgis::SymbolRenderHint::DynamicRotation || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );
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


QVariantMap QgsRasterMarkerSymbolLayer::properties() const
{
  QVariantMap map;
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
  std::unique_ptr< QgsRasterMarkerSymbolLayer > m = std::make_unique< QgsRasterMarkerSymbolLayer >( mPath, mSize, mAngle );
  copyCommonProperties( m.get() );
  return m.release();
}


void QgsRasterMarkerSymbolLayer::copyCommonProperties( QgsRasterMarkerSymbolLayer *other ) const
{
  other->setFixedAspectRatio( mFixedAspectRatio );
  other->setOpacity( mOpacity );
  other->setOffset( mOffset );
  other->setOffsetUnit( mOffsetUnit );
  other->setOffsetMapUnitScale( mOffsetMapUnitScale );
  other->setSizeUnit( mSizeUnit );
  other->setSizeMapUnitScale( mSizeMapUnitScale );
  other->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  other->setVerticalAnchorPoint( mVerticalAnchorPoint );
  copyDataDefinedProperties( other );
  copyPaintEffect( other );
}

bool QgsRasterMarkerSymbolLayer::usesMapUnits() const
{
  return mSizeUnit == QgsUnitTypes::RenderMapUnits || mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

QColor QgsRasterMarkerSymbolLayer::color() const
{
  return QColor();
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
  const double scaledSize = calculateSize( context, hasDataDefinedSize );
  const double width = context.renderContext().convertToPainterUnits( scaledSize, mSizeUnit, mSizeMapUnitScale );
  bool hasDataDefinedAspectRatio = false;
  const double aspectRatio = calculateAspectRatio( context, scaledSize, hasDataDefinedAspectRatio );
  const double height = width * ( preservedAspectRatio() ? defaultAspectRatio() : aspectRatio );

  //don't render symbols with size below one or above 10,000 pixels
  if ( static_cast< int >( scaledSize ) < 1 || 10000.0 < scaledSize )
  {
    return QRectF();
  }

  QPointF outputOffset;
  double angle = 0.0;
  calculateOffsetAndRotation( context, scaledSize, scaledSize * ( height / width ), outputOffset, angle );

  QTransform transform;

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

QgsFontMarkerSymbolLayer::QgsFontMarkerSymbolLayer( const QString &fontFamily, QString chr, double pointSize, const QColor &color, double angle )
{
  mFontFamily = fontFamily;
  mString = chr;
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

QgsFontMarkerSymbolLayer::~QgsFontMarkerSymbolLayer() = default;

QgsSymbolLayer *QgsFontMarkerSymbolLayer::create( const QVariantMap &props )
{
  QString fontFamily = DEFAULT_FONTMARKER_FONT;
  QString string = DEFAULT_FONTMARKER_CHR;
  double pointSize = DEFAULT_FONTMARKER_SIZE;
  QColor color = DEFAULT_FONTMARKER_COLOR;
  double angle = DEFAULT_FONTMARKER_ANGLE;

  if ( props.contains( QStringLiteral( "font" ) ) )
    fontFamily = props[QStringLiteral( "font" )].toString();
  if ( props.contains( QStringLiteral( "chr" ) ) && props[QStringLiteral( "chr" )].toString().length() > 0 )
    string = props[QStringLiteral( "chr" )].toString();
  if ( props.contains( QStringLiteral( "size" ) ) )
    pointSize = props[QStringLiteral( "size" )].toDouble();
  if ( props.contains( QStringLiteral( "color" ) ) )
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )].toString() );
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();

  QgsFontMarkerSymbolLayer *m = new QgsFontMarkerSymbolLayer( fontFamily, string, pointSize, color, angle );

  if ( props.contains( QStringLiteral( "font_style" ) ) )
    m->setFontStyle( props[QStringLiteral( "font_style" )].toString() );
  if ( props.contains( QStringLiteral( "outline_color" ) ) )
    m->setStrokeColor( QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )].toString() ) );
  if ( props.contains( QStringLiteral( "outline_width" ) ) )
    m->setStrokeWidth( props[QStringLiteral( "outline_width" )].toDouble() );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    m->setOffset( QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    m->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    m->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "size_unit" ) ) )
    m->setSizeUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "size_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "size_map_unit_scale" ) ) )
    m->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "size_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
    m->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
    m->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "outline_width_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
    m->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )].toString() ) );
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
  if ( !mFontStyle.isEmpty() )
  {
    mFont.setStyleName( QgsFontUtils::translateNamedStyle( mFontStyle ) );
  }

  double sizePixels = context.renderContext().convertToPainterUnits( mSize, mSizeUnit, mSizeMapUnitScale );
  mNonZeroFontSize = !qgsDoubleNear( sizePixels, 0.0 );

  if ( mNonZeroFontSize && sizePixels > MAX_FONT_CHARACTER_SIZE_IN_PIXELS )
  {
    // if font is too large (e.g using map units and map is very zoomed in), then we limit
    // the font size and instead scale up the painter.
    // this avoids issues with massive font sizes (eg https://github.com/qgis/QGIS/issues/42270)
    mFontSizeScale = sizePixels / MAX_FONT_CHARACTER_SIZE_IN_PIXELS;
    sizePixels = MAX_FONT_CHARACTER_SIZE_IN_PIXELS;
  }
  else
    mFontSizeScale = 1.0;

  // if a non zero, but small pixel size results, round up to 2 pixels so that a "dot" is at least visible
  // (if we set a <=1 pixel size here Qt will reset the font to a default size, leading to much too large symbols)
  mFont.setPixelSize( std::max( 2, static_cast< int >( std::round( sizePixels ) ) ) );
  mFontMetrics.reset( new QFontMetrics( mFont ) );
  mChrWidth = mFontMetrics->horizontalAdvance( mString );
  mChrOffset = QPointF( mChrWidth / 2.0, -mFontMetrics->ascent() / 2.0 );
  mOrigSize = mSize; // save in case the size would be data defined

  // use caching only when not using a data defined character
  mUseCachedPath = !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFontFamily ) &&
                   !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFontStyle ) &&
                   !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCharacter );
  if ( mUseCachedPath )
  {
    QPointF chrOffset = mChrOffset;
    double chrWidth;
    const QString charToRender = characterToRender( context, chrOffset, chrWidth );
    mCachedPath = QPainterPath();
    mCachedPath.addText( -chrOffset.x(), -chrOffset.y(), mFont, charToRender );
  }
}

void QgsFontMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

QString QgsFontMarkerSymbolLayer::characterToRender( QgsSymbolRenderContext &context, QPointF &charOffset, double &charWidth )
{
  charOffset = mChrOffset;
  QString stringToRender = mString;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCharacter ) )
  {
    context.setOriginalValueVariable( mString );
    stringToRender = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyCharacter, context.renderContext().expressionContext(), mString );
    if ( stringToRender != mString )
    {
      charWidth = mFontMetrics->horizontalAdvance( stringToRender );
      charOffset = QPointF( charWidth / 2.0, -mFontMetrics->ascent() / 2.0 );
    }
  }
  return stringToRender;
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle, &ok ) + mLineAngle;

    // If the expression evaluation was not successful, fallback to static value
    if ( !ok )
      angle = mAngle + mLineAngle;
  }

  hasDataDefinedRotation = context.renderHints() & Qgis::SymbolRenderHint::DynamicRotation;
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
  const bool hasDataDefinedSize = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySize );

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
      case Qgis::ScaleMethod::ScaleArea:
        scaledSize = std::sqrt( scaledSize );
        break;
      case Qgis::ScaleMethod::ScaleDiameter:
        break;
    }
  }
  return scaledSize;
}

void QgsFontMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p || !mNonZeroFontSize )
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
  if ( !context.selected() || !SELECTION_IS_OPAQUE )
  {
    brushColor.setAlphaF( brushColor.alphaF() * context.opacity() );
  }
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
    const double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth, &ok );
    if ( ok )
    {
      penWidth = context.renderContext().convertToPainterUnits( strokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    const QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
    }
  }

  const QgsScopedQPainterState painterState( p );
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

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFontFamily ) )
  {
    context.setOriginalValueVariable( mFontFamily );
    const QString fontFamily = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyFontFamily, context.renderContext().expressionContext(), mFontFamily, &ok );
    mFont.setFamily( ok ? fontFamily : mFontFamily );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFontStyle ) )
  {
    context.setOriginalValueVariable( mFontStyle );
    const QString fontStyle = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyFontStyle, context.renderContext().expressionContext(), mFontStyle, &ok );
    QgsFontUtils::updateFontViaStyle( mFont, QgsFontUtils::translateNamedStyle( ok ? fontStyle : mFontStyle ) );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFontFamily ) || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFontStyle ) )
  {
    mFontMetrics.reset( new QFontMetrics( mFont ) );
  }

  QPointF chrOffset = mChrOffset;
  double chrWidth;
  const QString charToRender = characterToRender( context, chrOffset, chrWidth );

  const double sizeToRender = calculateSize( context );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, sizeToRender, hasDataDefinedRotation, offset, angle );

  p->translate( point.x() + offset.x(), point.y() + offset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  if ( !qgsDoubleNear( sizeToRender, mOrigSize ) )
  {
    const double s = sizeToRender / mOrigSize;
    transform.scale( s, s );
  }

  if ( !qgsDoubleNear( mFontSizeScale, 1.0 ) )
    transform.scale( mFontSizeScale, mFontSizeScale );

  if ( mUseCachedPath )
  {
    p->drawPath( transform.map( mCachedPath ) );
  }
  else
  {
    QPainterPath path;
    path.addText( -chrOffset.x(), -chrOffset.y(), mFont, charToRender );
    p->drawPath( transform.map( path ) );
  }
}

QVariantMap QgsFontMarkerSymbolLayer::properties() const
{
  QVariantMap props;
  props[QStringLiteral( "font" )] = mFontFamily;
  props[QStringLiteral( "font_style" )] = mFontStyle;
  props[QStringLiteral( "chr" )] = mString;
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
  QgsFontMarkerSymbolLayer *m = new QgsFontMarkerSymbolLayer( mFontFamily, mString, mSize, mColor, mAngle );
  m->setFontStyle( mFontStyle );
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

void QgsFontMarkerSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  const QString fontPath = QStringLiteral( "ttf://%1" ).arg( mFontFamily );
  int markIndex = !mString.isEmpty() ? mString.at( 0 ).unicode() : 0;
  const double size = QgsSymbolLayerUtils::rescaleUom( mSize, mSizeUnit, props );
  QgsSymbolLayerUtils::externalMarkerToSld( doc, graphicElem, fontPath, QStringLiteral( "ttf" ), &markIndex, mColor, size );

  // <Rotation>
  QString angleFunc;
  bool ok;
  const double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toString() ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  const QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );
}

bool QgsFontMarkerSymbolLayer::usesMapUnits() const
{
  return mSizeUnit == QgsUnitTypes::RenderMapUnits || mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mStrokeWidthUnit == QgsUnitTypes::RenderMapUnits || mStrokeWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

QRectF QgsFontMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  QPointF chrOffset = mChrOffset;
  double chrWidth = mChrWidth;
  //calculate width of rendered character
  ( void )characterToRender( context, chrOffset, chrWidth );

  if ( !mFontMetrics )
    mFontMetrics.reset( new QFontMetrics( mFont ) );

  double scaledSize = calculateSize( context );
  if ( !qgsDoubleNear( scaledSize, mOrigSize ) )
  {
    chrWidth *= scaledSize / mOrigSize;
  }
  chrWidth *= mFontSizeScale;

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

  QRectF symbolBounds = transform.mapRect( QRectF( -chrWidth / 2.0,
                        -scaledSize / 2.0,
                        chrWidth,
                        scaledSize ) );
  return symbolBounds;
}

QgsSymbolLayer *QgsFontMarkerSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

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

  const QString fontFamily = name.mid( 6 );

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    const double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerUtils::displacementFromSldElement( graphicElem, offset );

  const QString uom = element.attribute( QStringLiteral( "uom" ) );
  offset.setX( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.x() ) );
  offset.setY( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.y() ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );

  QgsMarkerSymbolLayer *m = new QgsFontMarkerSymbolLayer( fontFamily, QChar( chr ), size, color );
  m->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

void QgsFontMarkerSymbolLayer::resolveFonts( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  const QString fontFamily = properties.value( QStringLiteral( "font" ), DEFAULT_FONTMARKER_FONT ).toString();
  if ( !QgsFontUtils::fontFamilyMatchOnSystem( fontFamily ) )
  {
    context.pushMessage( QObject::tr( "Font “%1” not available on system" ).arg( fontFamily ) );
  }
}

void QgsSvgMarkerSymbolLayer::prepareExpressions( const QgsSymbolRenderContext &context )
{
  QMap<QString, QgsProperty>::iterator it = mParameters.begin();
  for ( ; it != mParameters.end(); ++it )
    it.value().prepare( context.renderContext().expressionContext() );

  QgsMarkerSymbolLayer::prepareExpressions( context );
}


QSet<QString> QgsSvgMarkerSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attrs = QgsMarkerSymbolLayer::usedAttributes( context );

  QMap<QString, QgsProperty>::const_iterator it = mParameters.constBegin();
  for ( ; it != mParameters.constEnd(); ++it )
  {
    attrs.unite( it.value().referencedFields( context.expressionContext(), true ) );
  }

  return attrs;
}

//
// QgsAnimatedMarkerSymbolLayer
//

QgsAnimatedMarkerSymbolLayer::QgsAnimatedMarkerSymbolLayer( const QString &path, double size, double angle )
  : QgsRasterMarkerSymbolLayer( path, size, angle )
{

}

QgsAnimatedMarkerSymbolLayer::~QgsAnimatedMarkerSymbolLayer() = default;

QgsSymbolLayer *QgsAnimatedMarkerSymbolLayer::create( const QVariantMap &properties )
{
  QString path;
  double size = DEFAULT_RASTERMARKER_SIZE;
  double angle = DEFAULT_RASTERMARKER_ANGLE;

  if ( properties.contains( QStringLiteral( "imageFile" ) ) )
    path = properties[QStringLiteral( "imageFile" )].toString();
  if ( properties.contains( QStringLiteral( "size" ) ) )
    size = properties[QStringLiteral( "size" )].toDouble();
  if ( properties.contains( QStringLiteral( "angle" ) ) )
    angle = properties[QStringLiteral( "angle" )].toDouble();

  std::unique_ptr< QgsAnimatedMarkerSymbolLayer > m = std::make_unique< QgsAnimatedMarkerSymbolLayer >( path, size, angle );
  m->setFrameRate( properties.value( QStringLiteral( "frameRate" ), QStringLiteral( "10" ) ).toDouble() );

  m->setCommonProperties( properties );
  return m.release();
}

QString QgsAnimatedMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "AnimatedMarker" );
}

QVariantMap QgsAnimatedMarkerSymbolLayer::properties() const
{
  QVariantMap res = QgsRasterMarkerSymbolLayer::properties();
  res.insert( QStringLiteral( "frameRate" ), mFrameRateFps );
  return res;
}

QgsAnimatedMarkerSymbolLayer *QgsAnimatedMarkerSymbolLayer::clone() const
{
  std::unique_ptr< QgsAnimatedMarkerSymbolLayer > m = std::make_unique< QgsAnimatedMarkerSymbolLayer >( mPath, mSize, mAngle );
  m->setFrameRate( mFrameRateFps );
  copyCommonProperties( m.get() );
  return m.release();
}

void QgsAnimatedMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QgsRasterMarkerSymbolLayer::startRender( context );

  mPreparedPaths.clear();
  if ( !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) && !mPath.isEmpty() )
  {
    QgsApplication::imageCache()->prepareAnimation( mPath );
    mStaticPath = true;
  }
  else
  {
    mStaticPath = false;
  }
}

QImage QgsAnimatedMarkerSymbolLayer::fetchImage( QgsRenderContext &context, const QString &path, QSize size, bool preserveAspectRatio, double opacity ) const
{
  if ( !mStaticPath && !mPreparedPaths.contains( path ) )
  {
    QgsApplication::imageCache()->prepareAnimation( path );
    mPreparedPaths.insert( path );
  }

  const long long mapFrameNumber = context.currentFrame();
  const int totalFrameCount = QgsApplication::imageCache()->totalFrameCount( path, context.flags() & Qgis::RenderContextFlag::RenderBlocking );
  const double markerAnimationDuration = totalFrameCount / mFrameRateFps;

  double animationTimeSeconds = 0;
  if ( mapFrameNumber >= 0 && context.frameRate() > 0 )
  {
    // render is part of an animation, so we base the calculated frame on that
    animationTimeSeconds = mapFrameNumber / context.frameRate();
  }
  else
  {
    // render is outside of animation, so base the calculated frame on the current epoch
    animationTimeSeconds = QDateTime::currentMSecsSinceEpoch() / 1000.0;
  }

  const double markerAnimationProgressSeconds = std::fmod( animationTimeSeconds, markerAnimationDuration );
  const int movieFrame = static_cast< int >( std::floor( markerAnimationProgressSeconds * mFrameRateFps ) );

  bool cached = false;
  return QgsApplication::imageCache()->pathAsImage( path, size, preserveAspectRatio, opacity, cached, context.flags() & Qgis::RenderContextFlag::RenderBlocking, 96, movieFrame );
}

