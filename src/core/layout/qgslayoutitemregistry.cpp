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
}

void TestLayoutItem::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle )
{
  Q_UNUSED( itemStyle );
  QPainter *painter = context.painter();

  painter->save();
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->setPen( Qt::NoPen );
  painter->setBrush( mColor );
  painter->drawRect( rect() );
  painter->restore();
}
///@endcond
