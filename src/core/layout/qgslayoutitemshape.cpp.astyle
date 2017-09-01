/***************************************************************************
                              qgslayoutitemshape.cpp
                             -----------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemshape.h"
#include "qgslayout.h"
#include "qgslayoututils.h"

#include <QPainter>

QgsLayoutItemShape::QgsLayoutItemShape( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mShapeStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );
}

void QgsLayoutItemShape::setSymbol( QgsFillSymbol *symbol )
{
  if ( !symbol )
    return;

  mShapeStyleSymbol.reset( symbol->clone() );
}

//
// QgsLayoutItemRectangularShape
//

QgsLayoutItemRectangularShape::QgsLayoutItemRectangularShape( QgsLayout *layout )
  : QgsLayoutItemShape( layout )
  , mCornerRadius( 0.0 )
{

}

QgsLayoutItemRectangularShape *QgsLayoutItemRectangularShape::create( QgsLayout *layout, const QVariantMap &settings )
{
  Q_UNUSED( settings );
  return new QgsLayoutItemRectangularShape( layout );
}

void QgsLayoutItemRectangularShape::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * )
{
  QPainter *painter = context.painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  double scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  QPolygonF shapePolygon;
  if ( mCornerRadius.length() > 0 )
  {
    //shapes with curves must be enlarged before conversion to QPolygonF, or
    //the curves are approximated too much and appear jaggy
    QTransform t = QTransform::fromScale( 100, 100 );
    //inverse transform used to scale created polygons back to expected size
    QTransform ti = t.inverted();

    QPainterPath roundedRectPath;
    double radius = mLayout->convertToLayoutUnits( mCornerRadius ) * scale;
    roundedRectPath.addRoundedRect( QRectF( 0, 0, rect().width() * scale, rect().height() * scale ), radius, radius );
    QPolygonF roundedPoly = roundedRectPath.toFillPolygon( t );
    shapePolygon = ti.map( roundedPoly );
  }
  else
  {
    shapePolygon = QPolygonF( QRectF( 0, 0, rect().width() * scale, rect().height() * scale ) );
  }

  QList<QPolygonF> rings; //empty list

  symbol()->startRender( context );
  symbol()->renderPolygon( shapePolygon, &rings, nullptr, context );
  symbol()->stopRender( context );
}


//
// QgsLayoutItemEllipseShape
//

QgsLayoutItemEllipseShape::QgsLayoutItemEllipseShape( QgsLayout *layout )
  : QgsLayoutItemShape( layout )
{

}

QgsLayoutItemEllipseShape *QgsLayoutItemEllipseShape::create( QgsLayout *layout, const QVariantMap &settings )
{
  Q_UNUSED( settings );
  return new QgsLayoutItemEllipseShape( layout );
}

void QgsLayoutItemEllipseShape::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * )
{
  QPainter *painter = context.painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  double scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  //shapes with curves must be enlarged before conversion to QPolygonF, or
  //the curves are approximated too much and appear jaggy
  QTransform t = QTransform::fromScale( 100, 100 );
  //inverse transform used to scale created polygons back to expected size
  QTransform ti = t.inverted();

  //create an ellipse
  QPainterPath ellipsePath;
  ellipsePath.addEllipse( QRectF( 0, 0, rect().width() * scale, rect().height() * scale ) );
  QPolygonF ellipsePoly = ellipsePath.toFillPolygon( t );
  QPolygonF shapePolygon = ti.map( ellipsePoly );

  QList<QPolygonF> rings; //empty list

  symbol()->startRender( context );
  symbol()->renderPolygon( shapePolygon, &rings, nullptr, context );
  symbol()->stopRender( context );
}

//
// QgsLayoutItemTriangleShape
//

QgsLayoutItemTriangleShape::QgsLayoutItemTriangleShape( QgsLayout *layout )
  : QgsLayoutItemShape( layout )
{

}

QgsLayoutItemTriangleShape *QgsLayoutItemTriangleShape::create( QgsLayout *layout, const QVariantMap &settings )
{
  Q_UNUSED( settings );
  return new QgsLayoutItemTriangleShape( layout );
}

void QgsLayoutItemTriangleShape::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * )
{
  QPainter *painter = context.painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  double scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  QPolygonF shapePolygon = QPolygonF() << QPointF( 0, rect().height() * scale )
                           << QPointF( rect().width() * scale, rect().height() * scale )
                           << QPointF( rect().width() / 2.0 * scale, 0 )
                           << QPointF( 0, rect().height() * scale );

  QList<QPolygonF> rings; //empty list

  symbol()->startRender( context );
  symbol()->renderPolygon( shapePolygon, &rings, nullptr, context );
  symbol()->stopRender( context );
}
