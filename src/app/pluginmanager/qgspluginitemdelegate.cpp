/***************************************************************************
    qgspluginitemdelegate.cpp  - a QItemDelegate subclass for plugin manager
                             -------------------
    begin                : Fri Sep 13 2013, Brighton HF
    copyright            : (C) 2013 Borys Jurgiel
    email                : info@borysjurgiel.pl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspluginitemdelegate.h"
#include <QPainter>
#include <QFont>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QApplication>
#include "qgspluginsortfilterproxymodel.h"


QgsPluginItemDelegate::QgsPluginItemDelegate( QObject * parent ) : QStyledItemDelegate( parent ) {}


QSize QgsPluginItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );
  // Calculate row height, adds some 20% padding
  int pixelsHigh = QApplication::fontMetrics().height() * 1.4;
  return QSize( pixelsHigh, pixelsHigh );
}


void QgsPluginItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  painter->save();
  painter->setRenderHint( QPainter::SmoothPixmapTransform );
  QStyle *style = QApplication::style();
  int pixelsHigh = QApplication::fontMetrics().height();

  // Draw the background
  style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, NULL );

  // Draw the checkbox
  if ( index.flags() & Qt::ItemIsUserCheckable )
  {
    QStyleOptionButton checkBoxStyle;
    checkBoxStyle.rect = option.rect;
    if ( index.data( Qt::CheckStateRole ).toBool() )
    {
      checkBoxStyle.state = QStyle::State_On | QStyle::State_Enabled;
    }
    else
    {
      checkBoxStyle.state = QStyle::State_Off | QStyle::State_Enabled;
    }
    style->drawControl( QStyle::CE_CheckBox, &checkBoxStyle, painter );
  }

  // Draw the icon
  QPixmap iconPixmap = index.data( Qt::DecorationRole ).value<QPixmap>();

  if ( !iconPixmap.isNull() )
  {
    int iconSize = pixelsHigh;
    painter->drawPixmap( option.rect.left() + 1.2 * pixelsHigh, option.rect.top() + 0.2 * pixelsHigh, iconSize, iconSize, iconPixmap );
  }

  // Draw the text
  if ( option.state & QStyle::State_Selected )
  {
    painter->setPen( option.palette.highlightedText().color() );
  }
  else
  {
    painter->setPen( option.palette.text().color() );
  }

  if ( ! index.data( PLUGIN_ERROR_ROLE ).toString().isEmpty() )
  {
    painter->setPen( Qt::red );
  }

  if ( ! index.data( PLUGIN_ERROR_ROLE ).toString().isEmpty()
       || index.data( PLUGIN_STATUS_ROLE ).toString() == QString( "upgradeable" )
       || index.data( PLUGIN_STATUS_ROLE ).toString() == QString( "new" ) )
  {
    QFont font = painter->font();
    font.setBold( true );
    painter->setFont( font );
  }
  painter->drawText( option.rect.left() + pixelsHigh * 2.4, option.rect.bottom() - pixelsHigh * 0.4, index.data( Qt::DisplayRole ).toString() );

  painter->restore();
}



