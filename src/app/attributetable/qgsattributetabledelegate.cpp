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
#include <QComboBox>
#include <QPainter>

#include "qgsattributetableview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeeditor.h"
#include "qgslogger.h"

QgsVectorLayer *QgsAttributeTableDelegate::layer( const QAbstractItemModel *model ) const
{
  const QgsAttributeTableModel *tm = qobject_cast<const QgsAttributeTableModel *>( model );
  if ( tm )
    return tm->layer();

  const QgsAttributeTableFilterModel *fm = dynamic_cast<const QgsAttributeTableFilterModel *>( model );
  if ( fm )
    return fm->layer();

  return NULL;
}

int QgsAttributeTableDelegate::fieldIdx( const QModelIndex &index ) const
{
  const QgsAttributeTableModel *tm = qobject_cast<const QgsAttributeTableModel *>( index.model() );
  if ( tm )
    return tm->fieldIdx( index.column() );

  const QgsAttributeTableFilterModel *fm = dynamic_cast<const QgsAttributeTableFilterModel *>( index.model() );
  if ( fm )
    return fm->tableModel()->fieldIdx( index.column() );

  return -1;
}


QWidget *QgsAttributeTableDelegate::createEditor(
  QWidget *parent,
  const QStyleOptionViewItem &option,
  const QModelIndex &index ) const
{
  QgsVectorLayer *vl = layer( index.model() );
  if ( vl == NULL )
    return NULL;

  return QgsAttributeEditor::createAttributeEditor( parent, 0, vl, fieldIdx( index ), index.model()->data( index, Qt::EditRole ) );
}

void QgsAttributeTableDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = layer( model );
  if ( vl == NULL )
    return;

  QVariant value;
  if ( !QgsAttributeEditor::retrieveValue( editor, vl, fieldIdx( index ), value ) )
    return;

  model->setData( index, value );
}

void QgsAttributeTableDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = layer( index.model() );
  if ( vl == NULL )
    return;

  QgsAttributeEditor::setValue( editor, vl, fieldIdx( index ), index.model()->data( index, Qt::EditRole ) );
  editor->adjustSize();
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
