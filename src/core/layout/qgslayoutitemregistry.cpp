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
#include "qgslayoutitemmap.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutitemtexttable.h"
#include "qgslayoutframe.h"
#include "qgsgloweffect.h"
#include "qgseffectstack.h"
#include "qgsvectorlayer.h"

#include <QPainter>

QgsLayoutItemRegistry::QgsLayoutItemRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsLayoutItemRegistry::~QgsLayoutItemRegistry()
{
  qDeleteAll( mMetadata );
  qDeleteAll( mMultiFrameMetadata );
}

bool QgsLayoutItemRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

#if 0
  // add temporary item to register
  auto createTemporaryItem = []( QgsLayout * layout )->QgsLayoutItem *
  {
    return new TestLayoutItem( layout );
  };

  addLayoutItemType( new QgsLayoutItemMetadata( QgsLayoutItemRegistry::LayoutItem + 1002, QStringLiteral( "temp type" ), createTemporaryItem ) );
#endif

  addLayoutItemType( new QgsLayoutItemMetadata( LayoutGroup, QObject::tr( "Group" ), QObject::tr( "Groups" ), QgsLayoutItemGroup::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutFrame, QObject::tr( "Frame" ), QObject::tr( "Frames" ), QgsLayoutFrame::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutPage, QObject::tr( "Page" ), QObject::tr( "Pages" ), QgsLayoutItemPage::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutMap, QObject::tr( "Map" ), QObject::tr( "Maps" ), QgsLayoutItemMap::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutPicture, QObject::tr( "Picture" ), QObject::tr( "Pictures" ), QgsLayoutItemPicture::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutLabel, QObject::tr( "Label" ), QObject::tr( "Labels" ), QgsLayoutItemLabel::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutLegend, QObject::tr( "Legend" ), QObject::tr( "Legends" ), QgsLayoutItemLegend::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutScaleBar, QObject::tr( "Scalebar" ), QObject::tr( "Scalebars" ), QgsLayoutItemScaleBar::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutShape, QObject::tr( "Shape" ), QObject::tr( "Shapes" ), []( QgsLayout * layout )
  {
    QgsLayoutItemShape *shape = new QgsLayoutItemShape( layout );
    shape->setShapeType( QgsLayoutItemShape::Rectangle );
    return shape;
  } ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutPolygon, QObject::tr( "Polygon" ), QObject::tr( "Polygons" ), QgsLayoutItemPolygon::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutPolyline, QObject::tr( "Polyline" ), QObject::tr( "Polylines" ), QgsLayoutItemPolyline::create ) );

  addLayoutMultiFrameType( new QgsLayoutMultiFrameMetadata( LayoutHtml, QObject::tr( "HTML" ), QgsLayoutItemHtml::create ) );
  addLayoutMultiFrameType( new QgsLayoutMultiFrameMetadata( LayoutAttributeTable, QObject::tr( "Attribute Table" ), QgsLayoutItemAttributeTable::create ) );
  addLayoutMultiFrameType( new QgsLayoutMultiFrameMetadata( LayoutTextTable, QObject::tr( "Text Table" ), QgsLayoutItemTextTable::create ) );

  return true;
}

QgsLayoutItemAbstractMetadata *QgsLayoutItemRegistry::itemMetadata( int type ) const
{
  return mMetadata.value( type );
}

QgsLayoutMultiFrameAbstractMetadata *QgsLayoutItemRegistry::multiFrameMetadata( int type ) const
{
  return mMultiFrameMetadata.value( type );
}

bool QgsLayoutItemRegistry::addLayoutItemType( QgsLayoutItemAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit typeAdded( metadata->type(), metadata->visibleName() );
  return true;
}

bool QgsLayoutItemRegistry::addLayoutMultiFrameType( QgsLayoutMultiFrameAbstractMetadata *metadata )
{
  if ( !metadata || mMultiFrameMetadata.contains( metadata->type() ) )
    return false;

  mMultiFrameMetadata[metadata->type()] = metadata;
  emit multiFrameTypeAdded( metadata->type(), metadata->visibleName() );
  return true;
}

QgsLayoutItem *QgsLayoutItemRegistry::createItem( int type, QgsLayout *layout ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createItem( layout );
}

QgsLayoutMultiFrame *QgsLayoutItemRegistry::createMultiFrame( int type, QgsLayout *layout ) const
{
  if ( !mMultiFrameMetadata.contains( type ) )
    return nullptr;

  return mMultiFrameMetadata[type]->createMultiFrame( layout );
}

void QgsLayoutItemRegistry::resolvePaths( int type, QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) const
{
  if ( mMetadata.contains( type ) )
  {
    mMetadata[type]->resolvePaths( properties, pathResolver, saving );
  }
  else if ( mMultiFrameMetadata.contains( type ) )
  {
    mMultiFrameMetadata[type]->resolvePaths( properties, pathResolver, saving );
  }
}

QMap<int, QString> QgsLayoutItemRegistry::itemTypes() const
{
  QMap<int, QString> types;
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }
  for ( auto it = mMultiFrameMetadata.constBegin(); it != mMultiFrameMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }

  return types;
}

///@cond TEMPORARY
#if 0
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
  Q_UNUSED( itemStyle )

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
#endif
///@endcond
