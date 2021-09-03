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
#include "qgis.h"
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

  mProxyModel = new QgsProfilerProxyModel( profiler, this );

  mTreeView->setModel( mProxyModel );
  mTreeView->setSortingEnabled( true );

  //mTreeView->resizeColumnToContents( 0 );
  //mTreeView->resizeColumnToContents( 1 );

  mTreeView->setItemDelegateForColumn( 1, new CostDelegate( QgsRuntimeProfilerNode::Elapsed, QgsRuntimeProfilerNode::ParentElapsed, mTreeView ) );

  connect( mProfiler, &QgsRuntimeProfiler::groupAdded, this, [ = ]( const QString & group )
  {
    mCategoryComboBox->addItem( QgsRuntimeProfiler::translateGroupName( group ).isEmpty() ? group : QgsRuntimeProfiler::translateGroupName( group ), group );
    if ( mCategoryComboBox->count() == 1 )
    {
      mCategoryComboBox->setCurrentIndex( 0 );
      mProxyModel->setGroup( mCategoryComboBox->currentData().toString() );
    }
  } );

  connect( mCategoryComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    mProxyModel->setGroup( mCategoryComboBox->currentData().toString() );
  } );

  const QSet<QString> groups = mProfiler->groups();
  for ( const QString &group : groups )
  {
    mCategoryComboBox->addItem( QgsRuntimeProfiler::translateGroupName( group ).isEmpty() ? group : QgsRuntimeProfiler::translateGroupName( group ), group );
  }

  if ( mCategoryComboBox->count() > 0 )
  {
    mCategoryComboBox->setCurrentIndex( 0 );
    mProxyModel->setGroup( mCategoryComboBox->currentData().toString() );
  }
}

//
// QgsProfilerProxyModel
//

QgsProfilerProxyModel::QgsProfilerProxyModel( QgsRuntimeProfiler *profiler, QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setSourceModel( profiler );
}

void QgsProfilerProxyModel::setGroup( const QString &group )
{
  mGroup = group;
  invalidateFilter();
}

bool QgsProfilerProxyModel::filterAcceptsRow( int row, const QModelIndex &source_parent ) const
{
  const QModelIndex index = sourceModel()->index( row, 0, source_parent );
  return sourceModel()->data( index, QgsRuntimeProfilerNode::Group ).toString() == mGroup;
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
  const auto cost = index.data( m_sortRole ).toDouble();
  if ( cost == 0 )
  {
    QStyledItemDelegate::paint( painter, option, index );
    return;
  }

  const auto totalCost = index.data( m_totalCostRole ).toDouble();
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

  const auto color = QColor::fromHsv( 120 - fraction * 120, 255, 255, ( -( ( fraction - 1 ) * ( fraction - 1 ) ) ) * 120 + 120 );
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

