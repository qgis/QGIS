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
#include "qgsfillsymbol.h"

#include <QPainter>

QgsLayoutItemShape::QgsLayoutItemShape( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mCornerRadius( 0 )
{
  setBackgroundEnabled( false );
  setFrameEnabled( false );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mShapeStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );
  refreshSymbol( false );

  connect( this, &QgsLayoutItemShape::sizePositionChanged, this, [ = ]
  {
    updateBoundingRect();
    update();
    emit clipPathChanged();
  } );
}

QgsLayoutItemShape::~QgsLayoutItemShape() = default;

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

QgsLayoutItem::Flags QgsLayoutItemShape::itemFlags() const
{
  QgsLayoutItem::Flags flags = QgsLayoutItem::itemFlags();
  flags |= QgsLayoutItem::FlagProvidesClipPath;
  return flags;
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

  emit clipPathChanged();
}

void QgsLayoutItemShape::refreshSymbol( bool redraw )
{
  if ( auto *lLayout = layout() )
  {
    const QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( lLayout, nullptr, lLayout->renderContext().dpi() );
    mMaxSymbolBleed = ( 25.4 / lLayout->renderContext().dpi() ) * QgsSymbolLayerUtils::estimateMaxSymbolBleed( mShapeStyleSymbol.get(), rc );
  }

  updateBoundingRect();

  if ( redraw )
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
  refreshSymbol( true );
}

void QgsLayoutItemShape::setCornerRadius( QgsLayoutMeasurement radius )
{
  mCornerRadius = radius;
  emit clipPathChanged();
}

QgsGeometry QgsLayoutItemShape::clipPath() const
{
  QPolygonF shapePolygon = mapToScene( calculatePolygon( 1.0 ) );
  // ensure polygon is closed
  if ( shapePolygon.at( 0 ) != shapePolygon.constLast() )
    shapePolygon << shapePolygon.at( 0 );
  return QgsGeometry::fromQPolygonF( shapePolygon );
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

  const double scale = context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  const QVector<QPolygonF> rings; //empty list

  symbol()->startRender( context.renderContext() );
  symbol()->renderPolygon( calculatePolygon( scale ), &rings, nullptr, context.renderContext() );
  symbol()->stopRender( context.renderContext() );
}

QPolygonF QgsLayoutItemShape::calculatePolygon( double scale ) const
{
  QPolygonF shapePolygon;

  //shapes with curves must be enlarged before conversion to QPolygonF, or
  //the curves are approximated too much and appear jaggy
  const QTransform t = QTransform::fromScale( 100, 100 );
  //inverse transform used to scale created polygons back to expected size
  const QTransform ti = t.inverted();

  switch ( mShape )
  {
    case Ellipse:
    {
      //create an ellipse
      QPainterPath ellipsePath;
      ellipsePath.addEllipse( QRectF( 0, 0, rect().width() * scale, rect().height() * scale ) );
      const QPolygonF ellipsePoly = ellipsePath.toFillPolygon( t );
      shapePolygon = ti.map( ellipsePoly );
      break;
    }
    case Rectangle:
    {
      //if corner radius set, then draw a rounded rectangle
      if ( mCornerRadius.length() > 0 )
      {
        QPainterPath roundedRectPath;
        const double radius = mLayout->convertToLayoutUnits( mCornerRadius ) * scale;
        roundedRectPath.addRoundedRect( QRectF( 0, 0, rect().width() * scale, rect().height() * scale ), radius, radius );
        const QPolygonF roundedPoly = roundedRectPath.toFillPolygon( t );
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
  return shapePolygon;
}

bool QgsLayoutItemShape::writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "shapeType" ), mShape );
  element.setAttribute( QStringLiteral( "cornerRadiusMeasure" ), mCornerRadius.encodeMeasurement() );

  const QDomElement shapeStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mShapeStyleSymbol.get(), document, context );
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

  const QDomElement shapeStyleSymbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    mShapeStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( shapeStyleSymbolElem, context ) );
    refreshSymbol( false );
  }

  return true;
}
