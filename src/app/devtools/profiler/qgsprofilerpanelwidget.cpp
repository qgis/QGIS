/***************************************************************************
    qgsprofilerpanelwidget.cpp
    -------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprofilerpanelwidget.h"
#include "qgsruntimeprofiler.h"
#include "qgslogger.h"
#include <QPainter>
#include <cmath>

//
// QgsProfilerPanelWidget
//

QgsProfilerPanelWidget::QgsProfilerPanelWidget( QgsRuntimeProfiler *profiler, QWidget *parent )
  : QgsDevToolWidget( parent )
  , mProfiler( profiler )
{
  setupUi( this );

  mTreeWidget->setColumnCount( 2 );
  mTreeWidget->setHeaderLabels( QStringList() << tr( "Task" ) << tr( "Time (seconds)" ) );
  mTreeWidget->setSortingEnabled( true );

  std::function< void( const QString &topLevel, QTreeWidgetItem *parentItem, double parentCost ) > addGroup;
  addGroup = [&]( const QString & topLevel, QTreeWidgetItem * parentItem, double parentCost )
  {
    const QStringList children = mProfiler->childGroups( topLevel );
    for ( const QString &child : children )
    {
      double profileTime = mProfiler->profileTime( topLevel.isEmpty() ? child : topLevel + '/' + child );
      QTreeWidgetItem *item = new QTreeWidgetItem( QStringList() << child << QString::number( profileTime ) );
      item->setData( 1, Qt::UserRole + 1, parentCost * 1000 );
      item->setData( 1, Qt::InitialSortOrderRole, profileTime * 1000 );
      if ( !parentItem )
        mTreeWidget->addTopLevelItem( item );
      else
        parentItem->addChild( item );

      addGroup( topLevel.isEmpty() ? child : topLevel + '/' + child, item, profileTime );
    }
  };
  const double totalTime = mProfiler->profileTime( QString() );
  addGroup( QString(), nullptr, totalTime );

  mTreeWidget->resizeColumnToContents( 0 );
  mTreeWidget->resizeColumnToContents( 1 );

  mTreeWidget->setItemDelegateForColumn( 1, new CostDelegate( Qt::InitialSortOrderRole, Qt::UserRole + 1 ) );
}


//
// CostDelegate
//

// adapted from KDAB's excellent "hotspot" application!

CostDelegate::CostDelegate( quint32 sortRole, quint32 totalCostRole, QObject *parent )
  : QStyledItemDelegate( parent )
  , m_sortRole( sortRole )
  , m_totalCostRole( totalCostRole )
{
}

CostDelegate::~CostDelegate() = default;

void CostDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  // TODO: handle negative values
  const auto cost = index.data( m_sortRole ).toULongLong();
  if ( cost == 0 )
  {
    QStyledItemDelegate::paint( painter, option, index );
    return;
  }

  const auto totalCost = index.data( m_totalCostRole ).toULongLong();
  const auto fraction = std::abs( float( cost ) / totalCost );

  auto rect = option.rect;
  rect.setWidth( rect.width() * fraction );

  const auto &brush = painter->brush();
  const auto &pen = painter->pen();

  painter->setPen( Qt::NoPen );

  if ( option.features & QStyleOptionViewItem::Alternate )
  {
    // we must handle this ourselves as otherwise the custom background
    // would get painted over with the alternate background color
    painter->setBrush( option.palette.alternateBase() );
    painter->drawRect( option.rect );
  }

  auto color = QColor::fromHsv( 120 - fraction * 120, 255, 255, ( -( ( fraction - 1 ) * ( fraction - 1 ) ) ) * 120 + 120 );
  painter->setBrush( color );
  painter->drawRect( rect );

  painter->setBrush( brush );
  painter->setPen( pen );

  if ( option.features & QStyleOptionViewItem::Alternate )
  {
    auto o = option;
    o.features &= ~QStyleOptionViewItem::Alternate;
    QStyledItemDelegate::paint( painter, o, index );
  }
  else
  {
    QStyledItemDelegate::paint( painter, option, index );
  }
}
