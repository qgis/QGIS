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

#include "qgsfillsymbol.h"
#include "qgslayout.h"
#include "qgslayoutmodel.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoututils.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerutils.h"

#include <QPainter>

#include "moc_qgslayoutitemshape.cpp"

QgsLayoutItemShape::QgsLayoutItemShape( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mCornerRadius( 0 )
{
  setBackgroundEnabled( false );
  setFrameEnabled( false );
  QVariantMap properties;
  properties.insert( u"color"_s, u"white"_s );
  properties.insert( u"style"_s, u"solid"_s );
  properties.insert( u"style_border"_s, u"solid"_s );
  properties.insert( u"color_border"_s, u"black"_s );
  properties.insert( u"width_border"_s, u"0.3"_s );
  properties.insert( u"joinstyle"_s, u"miter"_s );
  mShapeStyleSymbol = QgsFillSymbol::createSimple( properties );
  refreshSymbol( false );

  connect( this, &QgsLayoutItemShape::sizePositionChanged, this, [this]
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
      return QgsApplication::getThemeIcon( u"/mLayoutItemShapeEllipse.svg"_s );
    case Rectangle:
      return QgsApplication::getThemeIcon( u"/mLayoutItemShapeRectangle.svg"_s );
    case Triangle:
      return QgsApplication::getThemeIcon( u"/mLayoutItemShapeTriangle.svg"_s );
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
  QgsRenderContext renderContext = context.renderContext();
  // symbol clipping messes with geometry generators used in the symbol for this item, and has no
  // valid use here. See https://github.com/qgis/QGIS/issues/58909
  renderContext.setFlag( Qgis::RenderContextFlag::DisableSymbolClippingToExtent );

  QPainter *painter = renderContext.painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  const double scale = renderContext.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );

  const QVector<QPolygonF> rings; //empty list

  symbol()->startRender( renderContext );
  symbol()->renderPolygon( calculatePolygon( scale ), &rings, nullptr, renderContext );
  symbol()->stopRender( renderContext );
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
  element.setAttribute( u"shapeType"_s, mShape );
  element.setAttribute( u"cornerRadiusMeasure"_s, mCornerRadius.encodeMeasurement() );

  const QDomElement shapeStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mShapeStyleSymbol.get(), document, context );
  element.appendChild( shapeStyleElem );

  return true;
}

bool QgsLayoutItemShape::readPropertiesFromElement( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  mShape = static_cast< Shape >( element.attribute( u"shapeType"_s, u"0"_s ).toInt() );
  if ( element.hasAttribute( u"cornerRadiusMeasure"_s ) )
    mCornerRadius = QgsLayoutMeasurement::decodeMeasurement( element.attribute( u"cornerRadiusMeasure"_s, u"0"_s ) );
  else
    mCornerRadius = QgsLayoutMeasurement( element.attribute( u"cornerRadius"_s, u"0"_s ).toDouble() );

  const QDomElement shapeStyleSymbolElem = element.firstChildElement( u"symbol"_s );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    mShapeStyleSymbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( shapeStyleSymbolElem, context );
    refreshSymbol( false );
  }

  return true;
}
