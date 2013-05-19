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

#include "qgsfeatureselectionmodel.h"
#include "qgsattributetableview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeeditor.h"
#include "qgslogger.h"



// TODO: Remove this casting orgy

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

QWidget *QgsAttributeTableDelegate::createEditor(
  QWidget *parent,
  const QStyleOptionViewItem &option,
  const QModelIndex &index ) const
{
  Q_UNUSED( option );
  QgsVectorLayer *vl = layer( index.model() );
  if ( !vl )
    return NULL;

  int fieldIdx = index.model()->data( index, QgsAttributeTableModel::FieldIndexRole ).toInt();

  QWidget *w = QgsAttributeEditor::createAttributeEditor( parent, 0, vl, fieldIdx, index.model()->data( index, Qt::EditRole ) );

  w->setAutoFillBackground( true );

  if ( parent )
  {
    QgsAttributeTableView *tv = dynamic_cast<QgsAttributeTableView *>( parent->parentWidget() );
    w->setMinimumWidth( tv->columnWidth( index.column() ) );

    if ( vl->editType( fieldIdx ) == QgsVectorLayer::FileName ||
         vl->editType( fieldIdx ) == QgsVectorLayer::Calendar )
    {
      QLineEdit *le = w->findChild<QLineEdit*>();
      le->adjustSize();
      w->setMinimumHeight( le->height()*2 ); // FIXME: there must be a better way to do this
    }
  }

  return w;
}

void QgsAttributeTableDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = layer( model );
  if ( vl == NULL )
    return;

  int fieldIdx = model->data( index, QgsAttributeTableModel::FieldIndexRole ).toInt();
  QgsFeatureId fid = model->data( index, QgsAttributeTableModel::FeatureIdRole ).toInt();

  QVariant value;
  if ( !QgsAttributeEditor::retrieveValue( editor, vl, fieldIdx, value ) )
    return;

  vl->beginEditCommand( tr( "Attribute changed" ) );
  vl->changeAttributeValue( fid, fieldIdx, value, true );
  vl->endEditCommand();
}

void QgsAttributeTableDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = layer( index.model() );
  if ( vl == NULL )
    return;

  int fieldIdx = index.model()->data( index, QgsAttributeTableModel::FieldIndexRole ).toInt();
  QgsAttributeEditor::setValue( editor, vl, fieldIdx, index.model()->data( index, Qt::EditRole ) );
}

void QgsAttributeTableDelegate::setFeatureSelectionModel( QgsFeatureSelectionModel *featureSelectionModel )
{
  mFeatureSelectionModel = featureSelectionModel;
}

void QgsAttributeTableDelegate::paint( QPainter * painter,
                                       const QStyleOptionViewItem & option,
                                       const QModelIndex & index ) const
{
  QgsFeatureId fid = index.model()->data( index, QgsAttributeTableModel::FeatureIdRole ).toInt();

  QStyleOptionViewItem myOpt = option;

  if ( mFeatureSelectionModel->isSelected( fid ) )
    myOpt.state |= QStyle::State_Selected;

  QItemDelegate::paint( painter, myOpt, index );

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
