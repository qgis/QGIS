/***************************************************************************
  qgslayertreeviewitemdelegate.cpp
  --------------------------------------
  Date                 : January 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewitemdelegate.h"

#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"
#include "qgslayertreeviewindicator.h"

#include <QHelpEvent>
#include <QToolTip>

/// @cond PRIVATE

QgsLayerTreeViewProxyStyle::QgsLayerTreeViewProxyStyle( QgsLayerTreeView *treeView )
  : QgsProxyStyle( treeView )
  , mLayerTreeView( treeView )
{
}


QRect QgsLayerTreeViewProxyStyle::subElementRect( QStyle::SubElement element, const QStyleOption *option, const QWidget *widget ) const
{
  if ( element == SE_ItemViewItemText || element == SE_LayerTreeItemIndicator )
  {
    if ( const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>( option ) )
    {
      if ( QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( vopt->index ) )
      {
        int count = mLayerTreeView->indicators( node ).count();
        if ( count )
        {
          QRect r = QProxyStyle::subElementRect( SE_ItemViewItemText, option, widget );
          int indiWidth = r.height() * count;
          int textWidth = r.width() - indiWidth;
          if ( element == SE_LayerTreeItemIndicator )
          {
            return QRect( r.left() + textWidth, r.top(), indiWidth, r.height() );
          }
          else if ( element == SE_ItemViewItemText )
          {
            return QRect( r.left(), r.top(), textWidth, r.height() );
          }
        }
      }
    }
  }
  return QProxyStyle::subElementRect( element, option, widget );
}


// -----


QgsLayerTreeViewItemDelegate::QgsLayerTreeViewItemDelegate( QgsLayerTreeView *parent )
  : QStyledItemDelegate( parent )
  , mLayerTreeView( parent )
{
  connect( mLayerTreeView, &QgsLayerTreeView::clicked, this, &QgsLayerTreeViewItemDelegate::onClicked );
}


void QgsLayerTreeViewItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyledItemDelegate::paint( painter, option, index );

  QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
  if ( !node )
    return;

  const QList<QgsLayerTreeViewIndicator *> indicators = mLayerTreeView->indicators( node );
  if ( indicators.isEmpty() )
    return;

  QStyleOptionViewItem opt = option;
  initStyleOption( &opt, index );

  QRect indRect = mLayerTreeView->style()->subElementRect( static_cast<QStyle::SubElement>( QgsLayerTreeViewProxyStyle::SE_LayerTreeItemIndicator ), &opt, mLayerTreeView );
  int spacing = indRect.height() / 10;
  int h = indRect.height();
  int x = indRect.left();

  for ( QgsLayerTreeViewIndicator *indicator : indicators )
  {
    QRect rect( x + spacing, indRect.top() + spacing, h - spacing * 2, h - spacing * 2 );
    x += h;

    QIcon::Mode mode = QIcon::Normal;
    if ( !( opt.state & QStyle::State_Enabled ) )
      mode = QIcon::Disabled;
    else if ( opt.state & QStyle::State_Selected )
      mode = QIcon::Selected;

    indicator->icon().paint( painter, rect, Qt::AlignCenter, mode );
  }
}

static void _fixStyleOption( QStyleOptionViewItem &opt )
{
  // This makes sure our delegate behaves correctly across different styles. Unfortunately there is inconsistency
  // in how QStyleOptionViewItem::showDecorationSelected is prepared for paint() vs what is returned from view's viewOptions():
  // - viewOptions() returns it based on style's SH_ItemView_ShowDecorationSelected hint
  // - for paint() there is extra call to QTreeViewPrivate::adjustViewOptionsForIndex() which makes it
  //   always true if view's selection behavior is SelectRows (which is the default and our case with layer tree view)
  // So for consistency between different calls we override it to what we get in paint() method ... phew!
  opt.showDecorationSelected = true;
}

bool QgsLayerTreeViewItemDelegate::helpEvent( QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index )
{
  if ( event && event->type() == QEvent::ToolTip )
  {
    QHelpEvent *he = static_cast<QHelpEvent *>( event );

    QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
    if ( node )
    {
      const QList<QgsLayerTreeViewIndicator *> indicators = mLayerTreeView->indicators( node );
      if ( !indicators.isEmpty() )
      {
        QStyleOptionViewItem opt = option;
        initStyleOption( &opt, index );
        _fixStyleOption( opt );

        QRect indRect = mLayerTreeView->style()->subElementRect( static_cast<QStyle::SubElement>( QgsLayerTreeViewProxyStyle::SE_LayerTreeItemIndicator ), &opt, mLayerTreeView );

        if ( indRect.contains( he->pos() ) )
        {
          int indicatorIndex = ( he->pos().x() - indRect.left() ) / indRect.height();
          if ( indicatorIndex >= 0 && indicatorIndex < indicators.count() )
          {
            const QString tooltip = indicators[indicatorIndex]->toolTip();
            if ( !tooltip.isEmpty() )
            {
              QToolTip::showText( he->globalPos(), tooltip, view );
              return true;
            }
          }
        }
      }
    }
  }
  return QStyledItemDelegate::helpEvent( event, view, option, index );
}


void QgsLayerTreeViewItemDelegate::onClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
  if ( !node )
    return;

  const QList<QgsLayerTreeViewIndicator *> indicators = mLayerTreeView->indicators( node );
  if ( indicators.isEmpty() )
    return;

  QStyleOptionViewItem opt( mLayerTreeView->viewOptions() );
  opt.rect = mLayerTreeView->visualRect( index );
  initStyleOption( &opt, index );
  _fixStyleOption( opt );

  QRect indRect = mLayerTreeView->style()->subElementRect( static_cast<QStyle::SubElement>( QgsLayerTreeViewProxyStyle::SE_LayerTreeItemIndicator ), &opt, mLayerTreeView );

  QPoint pos = mLayerTreeView->mLastReleaseMousePos;
  if ( indRect.contains( pos ) )
  {
    int indicatorIndex = ( pos.x() - indRect.left() ) / indRect.height();
    if ( indicatorIndex >= 0 && indicatorIndex < indicators.count() )
      emit indicators[indicatorIndex]->clicked( index );
  }
}

/// @endcond
