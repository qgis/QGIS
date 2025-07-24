/***************************************************************************
    qgsattributesformtreeviewitemdelegate.cpp
    ---------------------
    begin                : June 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributesformtreeviewitemdelegate.h"
#include "qgsattributesformview.h"
#include "qgsattributesformtreeviewindicator.h"
#include "moc_qgsattributesformtreeviewitemdelegate.cpp"

#include <QHelpEvent>
#include <QToolTip>

/// @cond PRIVATE

QgsAttributesFormTreeViewProxyStyle::QgsAttributesFormTreeViewProxyStyle( QgsAttributesFormBaseView *treeView )
  : QgsProxyStyle( treeView )
  , mAttributesFormTreeView( treeView )
{
}

QRect QgsAttributesFormTreeViewProxyStyle::subElementRect( QStyle::SubElement element, const QStyleOption *option, const QWidget *widget ) const
{
  if ( element == SE_AttributesFormTreeItemIndicator )
  {
    if ( const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>( option ) )
    {
      if ( vopt->index.isValid() )
      {
        const int count = mAttributesFormTreeView->indicators( vopt->index ).count();
        if ( count )
        {
          const QRect vpr = mAttributesFormTreeView->viewport()->rect();
          const QRect r = QProxyStyle::subElementRect( SE_ItemViewItemText, option, widget );
          const int indiWidth = r.height() * count;
          const int spacing = r.height() / 10;
          const int vpIndiWidth = vpr.width() - indiWidth - spacing;
          return QRect( vpIndiWidth, r.top(), indiWidth, r.height() );
        }
      }
    }
  }
  return QProxyStyle::subElementRect( element, option, widget );
}


QgsAttributesFormTreeViewItemDelegate::QgsAttributesFormTreeViewItemDelegate( QgsAttributesFormBaseView *parent )
  : QStyledItemDelegate( parent )
  , mAttributesFormTreeView( parent )
{
}

void QgsAttributesFormTreeViewItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyledItemDelegate::paint( painter, option, index );

  if ( !index.isValid() )
    return;

  QStyleOptionViewItem opt = option;
  initStyleOption( &opt, index );

  const QColor baseColor = opt.palette.base().color();
  const QList<QgsAttributesFormTreeViewIndicator *> indicators = mAttributesFormTreeView->indicators( index );
  if ( indicators.isEmpty() )
    return;

  const QRect indRect = mAttributesFormTreeView->style()->subElementRect( static_cast<QStyle::SubElement>( QgsAttributesFormTreeViewProxyStyle::SE_AttributesFormTreeItemIndicator ), &opt, mAttributesFormTreeView );
  const int spacing = indRect.height() / 10;
  const int h = indRect.height();
  int x = indRect.left();

  for ( QgsAttributesFormTreeViewIndicator *indicator : indicators )
  {
    const QRect rect( x + spacing, indRect.top() + spacing, h - spacing * 2, h - spacing * 2 );
    // Add a little more padding so the icon does not look misaligned to background
    const QRect iconRect( x + spacing * 2, indRect.top() + spacing * 2, h - spacing * 4, h - spacing * 4 );
    x += h;

    QIcon::Mode mode = QIcon::Normal;
    if ( !( opt.state & QStyle::State_Enabled ) )
      mode = QIcon::Disabled;
    else if ( opt.state & QStyle::State_Selected )
      mode = QIcon::Selected;

    // Draw indicator background, for when floating over text content
    const qreal bradius = spacing;
    const QBrush pb = painter->brush();
    const QPen pp = painter->pen();
    QBrush b = QBrush( opt.palette.midlight() );
    QColor bc = b.color();
    bc.setRed( static_cast<int>( bc.red() * 0.3 + baseColor.red() * 0.7 ) );
    bc.setGreen( static_cast<int>( bc.green() * 0.3 + baseColor.green() * 0.7 ) );
    bc.setBlue( static_cast<int>( bc.blue() * 0.3 + baseColor.blue() * 0.7 ) );
    b.setColor( bc );
    painter->setBrush( b );
    painter->setPen( QPen( QBrush( opt.palette.mid() ), 0.25 ) );
    painter->drawRoundedRect( rect, bradius, bradius );
    painter->setBrush( pb );
    painter->setPen( pp );

    indicator->icon().paint( painter, iconRect, Qt::AlignCenter, mode );
  }
}

bool QgsAttributesFormTreeViewItemDelegate::helpEvent( QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index )
{
  if ( event && event->type() == QEvent::ToolTip )
  {
    if ( index.isValid() )
    {
      const QList<QgsAttributesFormTreeViewIndicator *> indicators = mAttributesFormTreeView->indicators( index );
      if ( !indicators.isEmpty() )
      {
        QStyleOptionViewItem opt = option;
        initStyleOption( &opt, index );
        opt.showDecorationSelected = true; // See https://github.com/qgis/QGIS/pull/7853 for the rationale of this fix

        const QRect indRect = mAttributesFormTreeView->style()->subElementRect( static_cast<QStyle::SubElement>( QgsAttributesFormTreeViewProxyStyle::SE_AttributesFormTreeItemIndicator ), &opt, mAttributesFormTreeView );

        if ( indRect.contains( event->pos() ) )
        {
          const int indicatorIndex = ( event->pos().x() - indRect.left() ) / indRect.height();
          if ( indicatorIndex >= 0 && indicatorIndex < indicators.count() )
          {
            const QString tooltip = indicators[indicatorIndex]->toolTip();
            if ( !tooltip.isEmpty() )
            {
              QToolTip::showText( event->globalPos(), tooltip, view );
              return true;
            }
          }
        }
      }
    }
  }
  return QStyledItemDelegate::helpEvent( event, view, option, index );
}

/// @endcond
