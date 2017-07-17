/***************************************************************************
                            qgslayoutitemregistry.cpp
                            -------------------------
    begin                : June 2017
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

#include "qgslayoutitemregistry.h"
#include "qgslayoutitemshape.h"
#include "qgsgloweffect.h"
#include "qgseffectstack.h"
#include <QPainter>

QgsLayoutItemRegistry::QgsLayoutItemRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsLayoutItemRegistry::~QgsLayoutItemRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsLayoutItemRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  // add temporary item to register
  auto createTemporaryItem = []( QgsLayout * layout, const QVariantMap & )->QgsLayoutItem*
  {
    return new TestLayoutItem( layout );
  };

  addLayoutItemType( new QgsLayoutItemMetadata( 101, QStringLiteral( "temp type" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLabel.svg" ) ), createTemporaryItem ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutRectangle, QStringLiteral( "Rectangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ), QgsLayoutItemRectangularShape::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutEllipse, QStringLiteral( "Ellipse" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicCircle.svg" ) ), QgsLayoutItemEllipseShape::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutTriangle, QStringLiteral( "Triangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicTriangle.svg" ) ), QgsLayoutItemTriangleShape::create ) );

  return true;
}

QgsLayoutItemAbstractMetadata *QgsLayoutItemRegistry::itemMetadata( int type ) const
{
  return mMetadata.value( type );
}

bool QgsLayoutItemRegistry::addLayoutItemType( QgsLayoutItemAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit typeAdded( metadata->type(), metadata->visibleName() );
  return true;
}

QgsLayoutItem *QgsLayoutItemRegistry::createItem( int type, QgsLayout *layout, const QVariantMap &properties ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createItem( layout, properties );
}

void QgsLayoutItemRegistry::resolvePaths( int type, QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) const
{
  if ( !mMetadata.contains( type ) )
    return;

  mMetadata[type]->resolvePaths( properties, pathResolver, saving );

}

QMap<int, QString> QgsLayoutItemRegistry::itemTypes() const
{
  QMap<int, QString> types;
  QMap<int, QgsLayoutItemAbstractMetadata *>::ConstIterator it = mMetadata.constBegin();
  for ( ; it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }
  return types;
}

///@cond TEMPORARY
TestLayoutItem::TestLayoutItem( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  int h = static_cast< int >( 360.0 * qrand() / ( RAND_MAX + 1.0 ) );
  int s = ( qrand() % ( 200 - 100 + 1 ) ) + 100;
  int v = ( qrand() % ( 130 - 255 + 1 ) ) + 130;
  mColor = QColor::fromHsv( h, s, v );

  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), mColor.name() );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mShapeStyleSymbol = QgsFillSymbol::createSimple( properties );

}

void TestLayoutItem::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle )
{
  Q_UNUSED( itemStyle );

  QgsEffectStack stack;
  stack.appendEffect( new QgsDrawSourceEffect() );
  stack.appendEffect( new QgsInnerGlowEffect() );
  stack.begin( context );

  QPainter *painter = context.painter();

  painter->save();
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->setPen( Qt::NoPen );
  painter->setBrush( mColor );

  double scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  QPolygonF shapePolygon = QPolygonF( QRectF( 0, 0, rect().width() * scale, rect().height() * scale ) );
  QList<QPolygonF> rings; //empty list

  mShapeStyleSymbol->startRender( context );
  mShapeStyleSymbol->renderPolygon( shapePolygon, &rings, nullptr, context );
  mShapeStyleSymbol->stopRender( context );

// painter->drawRect( r );
  painter->restore();
  stack.end( context );
}
///@endcond
