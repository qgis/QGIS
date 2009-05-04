/***************************************************************************
     QgsAttributeTableDelegate.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QItemDelegate>
#include <QLineEdit>
#include <QPainter>

#include "qgsattributetableview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsvectordataprovider.h"

QWidget * QgsAttributeTableDelegate::createEditor( 
    QWidget *parent, 
    const QStyleOptionViewItem &option, 
    const QModelIndex &index ) const
{
  QWidget *editor = QItemDelegate::createEditor( parent, option, index );

  QLineEdit *le = dynamic_cast<QLineEdit*>( editor );
  if ( !le ) return editor;

  const QgsAttributeTableModel* m = dynamic_cast<const QgsAttributeTableModel*>( index.model() );
  if ( !m ) return editor;

  int col = index.column();
  QVariant::Type type = m->layer()->dataProvider()->fields()[col].type();

  if ( type == QVariant::Int )
  {
    le->setValidator( new QIntValidator( le ) );
  }
  else if ( type == QVariant::Double )
  {
    le->setValidator( new QDoubleValidator( le ) );
  }

  return editor;
}


void QgsAttributeTableDelegate::paint( QPainter * painter, 
    const QStyleOptionViewItem & option, 
    const QModelIndex & index ) const
{
  QItemDelegate::paint( painter, option, index );

  if ( option.state & QStyle::State_HasFocus )
  {
    QRect r = option.rect.adjusted( 1, 1, -1, -1 );
    QPen p( QBrush( QColor( 0, 255, 127 ) ), 2 );
    painter->save();
    painter->setPen( p );
    painter->drawRect( r );
    painter->restore();
  }
}

