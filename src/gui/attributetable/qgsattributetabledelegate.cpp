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

#include "qgsattributeeditor.h"
#include "qgsattributetabledelegate.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetableview.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfeatureselectionmodel.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"


QgsVectorLayer* QgsAttributeTableDelegate::layer( const QAbstractItemModel *model )
{
  const QgsAttributeTableModel *tm = qobject_cast<const QgsAttributeTableModel *>( model );
  if ( tm )
    return tm->layer();

  const QgsAttributeTableFilterModel *fm = dynamic_cast<const QgsAttributeTableFilterModel *>( model );
  if ( fm )
    return fm->layer();

  return 0;
}

const QgsAttributeTableModel* QgsAttributeTableDelegate::masterModel( const QAbstractItemModel* model )
{
  const QgsAttributeTableModel *tm = qobject_cast<const QgsAttributeTableModel *>( model );
  if ( tm )
    return tm;

  const QgsAttributeTableFilterModel *fm = dynamic_cast<const QgsAttributeTableFilterModel *>( model );
  if ( fm )
    return fm->masterModel();

  return 0;
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

  QString widgetType = vl->editorWidgetV2( fieldIdx );
  QgsEditorWidgetConfig cfg = vl->editorWidgetV2Config( fieldIdx );
  QgsAttributeEditorContext context( masterModel( index.model() )->editorContext(), QgsAttributeEditorContext::Popup );
  QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, vl, fieldIdx, cfg, 0, parent, context );
  QWidget* w = eww->widget();

  w->setAutoFillBackground( true );

  eww->setEnabled( vl->fieldEditable( fieldIdx ) );

  return w;
}

void QgsAttributeTableDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = layer( model );
  if ( vl == NULL )
    return;

  int fieldIdx = model->data( index, QgsAttributeTableModel::FieldIndexRole ).toInt();
  QgsFeatureId fid = model->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong();
  QVariant oldValue = model->data( index, Qt::EditRole );

  QVariant newValue;
  QgsEditorWidgetWrapper* eww = QgsEditorWidgetWrapper::fromWidget( editor );
  if ( !eww )
    return;

  newValue = eww->value();

  if (( oldValue != newValue && newValue.isValid() ) || oldValue.isNull() != newValue.isNull() )
  {
    vl->beginEditCommand( tr( "Attribute changed" ) );
    vl->changeAttributeValue( fid, fieldIdx, newValue, oldValue );
    vl->endEditCommand();
  }
}

void QgsAttributeTableDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsEditorWidgetWrapper* eww =  QgsEditorWidgetWrapper::fromWidget( editor );
  if ( !eww )
    return;

  eww->setValue( index.model()->data( index, Qt::EditRole ) );
}

void QgsAttributeTableDelegate::setFeatureSelectionModel( QgsFeatureSelectionModel *featureSelectionModel )
{
  mFeatureSelectionModel = featureSelectionModel;
}

void QgsAttributeTableDelegate::paint( QPainter * painter,
                                       const QStyleOptionViewItem & option,
                                       const QModelIndex & index ) const
{
  QgsFeatureId fid = index.model()->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong();

  QStyleOptionViewItem myOpt = option;

  if ( index.model()->data( index, Qt::EditRole ).isNull() )
  {
    myOpt.font.setItalic( true );
    myOpt.palette.setColor( QPalette::Text, QColor( "gray" ) );
  }

  if ( mFeatureSelectionModel && mFeatureSelectionModel->isSelected( fid ) )
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
