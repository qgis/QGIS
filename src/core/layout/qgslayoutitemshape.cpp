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
#include "qgssymbollayerutils.h"
#include "qgslayoutmodel.h"
#include "qgsstyleentityvisitor.h"

#include <QPainter>

QgsLayoutItemShape::QgsLayoutItemShape( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mCornerRadius( 0 )
{
  setBackgroundEnabled( false );
  setFrameEnabled( false );
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mShapeStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );
  refreshSymbol();

  connect( this, &QgsLayoutItemShape::sizePositionChanged, this, [ = ]
  {
    updateBoundingRect();
    update();
  } );
}

QgsLayoutItemShape *QgsLayoutItemShape::create( QgsLayout *layout )
{
  return new QgsLayoutItemShape( layout );
}

int QgsLayoutItemShape::type() const
{
  return QgsLayoutItemRegistry::LayoutShape;
}

QIcon QgsLayoutItemShape::icon() const
{
  switch ( mShape )
  {
    case Ellipse:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemShapeEllipse.svg" ) );
    case Rectangle:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemShapeRectangle.svg" ) );
    case Triangle:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemShapeTriangle.svg" ) );
  }

  return QIcon();
}

QString QgsLayoutItemShape::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  switch ( mShape )
  {
    case Ellipse:
      return tr( "<Ellipse>" );
    case Rectangle:
      return tr( "<Rectangle>" );
    case Triangle:
      return tr( "<Triangle>" );
  }

  return tr( "<Shape>" );
}

void QgsLayoutItemShape::setShapeType( QgsLayoutItemShape::Shape type )
{
  if ( type == mShape )
  {
    return;
  }

  mShape = type;

  if ( mLayout && id().isEmpty() )
  {
    //notify the model that the display name has changed
    mLayout->itemsModel()->updateItemDisplayName( this );
  }
}

void QgsLayoutItemShape::refreshSymbol()
{
  if ( layout() )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( layout(), nullptr, layout()->renderContext().dpi() );
    mMaxSymbolBleed = ( 25.4 / layout()->renderContext().dpi() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mShapeStyleSymbol.get(), rc );
  }

  updateBoundingRect();

  update();
  emit frameChanged();
}

void QgsLayoutItemShape::updateBoundingRect()
{
  QRectF rectangle = rect();
  rectangle.adjust( -mMaxSymbolBleed, -mMaxSymbolBleed, mMaxSymbolBleed, mMaxSymbolBleed );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

void QgsLayoutItemShape::setSymbol( QgsFillSymbol *symbol )
{
  if ( !symbol )
    return;

  mShapeStyleSymbol.reset( symbol->clone() );
  refreshSymbol();
}

QRectF QgsLayoutItemShape::boundingRect() const
{
  return mCurrentRectangle;
}

double QgsLayoutItemShape::estimatedFrameBleed() const
{
  return mMaxSymbolBleed;
}

bool QgsLayoutItemShape::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mShapeStyleSymbol )
  {
    QgsStyleSymbolEntity entity( mShapeStyleSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, uuid(), displayName() ) ) )
      return false;
  }

  return true;
}

void QgsLayoutItemShape::draw( QgsLayoutItemRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  double scale = context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  QPolygonF shapePolygon;

  //shapes with curves must be enlarged before conversion to QPolygonF, or
  //the curves are approximated too much and appear jaggy
  QTransform t = QTransform::fromScale( 100, 100 );
  //inverse transform used to scale created polygons back to expected size
  QTransform ti = t.inverted();

  switch ( mShape )
  {
    case Ellipse:
    {
      //create an ellipse
      QPainterPath ellipsePath;
      ellipsePath.addEllipse( QRectF( 0, 0, rect().width() * scale, rect().height() * scale ) );
      QPolygonF ellipsePoly = ellipsePath.toFillPolygon( t );
      shapePolygon = ti.map( ellipsePoly );
      break;
    }
    case Rectangle:
    {
      //if corner radius set, then draw a rounded rectangle
      if ( mCornerRadius.length() > 0 )
      {
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
      break;
    }
    case Triangle:
    {
      shapePolygon << QPointF( 0, rect().height() * scale );
      shapePolygon << QPointF( rect().width() * scale, rect().height() * scale );
      shapePolygon << QPointF( rect().width() / 2.0 * scale, 0 );
      shapePolygon << QPointF( 0, rect().height() * scale );
      break;
    }
  }

  QList<QPolygonF> rings; //empty list

  symbol()->startRender( context.renderContext() );
  symbol()->renderPolygon( shapePolygon, &rings, nullptr, context.renderContext() );
  symbol()->stopRender( context.renderContext() );
}

bool QgsLayoutItemShape::writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "shapeType" ), mShape );
  element.setAttribute( QStringLiteral( "cornerRadiusMeasure" ), mCornerRadius.encodeMeasurement() );

  QDomElement shapeStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mShapeStyleSymbol.get(), document, context );
  element.appendChild( shapeStyleElem );

  return true;
}

bool QgsLayoutItemShape::readPropertiesFromElement( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  mShape = static_cast< Shape >( element.attribute( QStringLiteral( "shapeType" ), QStringLiteral( "0" ) ).toInt() );
  if ( element.hasAttribute( QStringLiteral( "cornerRadiusMeasure" ) ) )
    mCornerRadius = QgsLayoutMeasurement::decodeMeasurement( element.attribute( QStringLiteral( "cornerRadiusMeasure" ), QStringLiteral( "0" ) ) );
  else
    mCornerRadius = QgsLayoutMeasurement( element.attribute( QStringLiteral( "cornerRadius" ), QStringLiteral( "0" ) ).toDouble() );

  QDomElement shapeStyleSymbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    mShapeStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( shapeStyleSymbolElem, context ) );
  }

  return true;
}
