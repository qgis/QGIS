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

#include "qgslayoutframe.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutitemchart.h"
#include "qgslayoutitemelevationprofile.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemmanualtable.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmarker.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitemtexttable.h"

#include <QPainter>

#include "moc_qgslayoutitemregistry.cpp"

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

  addLayoutItemType( new QgsLayoutItemMetadata( QgsLayoutItemRegistry::LayoutItem + 1002, u"temp type"_s, createTemporaryItem ) );
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
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutMarker, QObject::tr( "Marker" ), QObject::tr( "Markers" ), QgsLayoutItemMarker::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutPolygon, QObject::tr( "Polygon" ), QObject::tr( "Polygons" ), QgsLayoutItemPolygon::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutPolyline, QObject::tr( "Polyline" ), QObject::tr( "Polylines" ), QgsLayoutItemPolyline::create ) );

  addLayoutItemType( new QgsLayoutItemMetadata( LayoutElevationProfile, QObject::tr( "Elevation Profile" ), QObject::tr( "Elevation Profiles" ), QgsLayoutItemElevationProfile::create ) );
  addLayoutItemType( new QgsLayoutItemMetadata( LayoutChart, QObject::tr( "Chart" ), QObject::tr( "Charts" ), QgsLayoutItemChart::create ) );

  addLayoutMultiFrameType( new QgsLayoutMultiFrameMetadata( LayoutHtml, QObject::tr( "HTML" ), QgsLayoutItemHtml::create ) );
  addLayoutMultiFrameType( new QgsLayoutMultiFrameMetadata( LayoutAttributeTable, QObject::tr( "Attribute Table" ), QgsLayoutItemAttributeTable::create ) );
  addLayoutMultiFrameType( new QgsLayoutMultiFrameMetadata( LayoutTextTable, QObject::tr( "Text Table" ), QgsLayoutItemTextTable::create ) );
  addLayoutMultiFrameType( new QgsLayoutMultiFrameMetadata( LayoutManualTable, QObject::tr( "Fixed Table" ), QgsLayoutItemManualTable::create ) );

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

bool QgsLayoutItemRegistry::removeLayoutItemType( int typeId )
{
  if ( !mMetadata.contains( typeId ) )
    return false;
  mMetadata.remove( typeId );
  emit typeRemoved( typeId );
  return true;
}

bool QgsLayoutItemRegistry::removeLayoutItemType( QgsLayoutItemAbstractMetadata *metadata )
{
  return removeLayoutItemType( metadata->type() );
}

bool QgsLayoutItemRegistry::addLayoutMultiFrameType( QgsLayoutMultiFrameAbstractMetadata *metadata )
{
  if ( !metadata || mMultiFrameMetadata.contains( metadata->type() ) )
    return false;

  mMultiFrameMetadata[metadata->type()] = metadata;
  emit multiFrameTypeAdded( metadata->type(), metadata->visibleName() );
  return true;
}

bool QgsLayoutItemRegistry::removeLayoutMultiFrameType( int typeId )
{
  if ( !mMultiFrameMetadata.contains( typeId ) )
    return false;
  mMultiFrameMetadata.remove( typeId );
  emit multiFrameTypeRemoved( typeId );
  return true;
}

bool QgsLayoutItemRegistry::removeLayoutMultiFrameType( QgsLayoutMultiFrameAbstractMetadata *metadata )
{
  return removeLayoutMultiFrameType( metadata->type() );
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
  properties.insert( u"color"_s, mColor.name() );
  properties.insert( u"style"_s, u"solid"_s );
  properties.insert( u"style_border"_s, u"solid"_s );
  properties.insert( u"color_border"_s, u"black"_s );
  properties.insert( u"width_border"_s, u"0.3"_s );
  properties.insert( u"joinstyle"_s, u"miter"_s );
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
