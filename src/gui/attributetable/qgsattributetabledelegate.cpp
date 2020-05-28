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
#include <QToolButton>

#include "qgsattributetabledelegate.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetableview.h"
#include "qgseditorwidgetregistry.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsfeatureselectionmodel.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgsactionmanager.h"
#include "qgsgui.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerjoinbuffer.h"

QgsVectorLayer *QgsAttributeTableDelegate::layer( const QAbstractItemModel *model )
{
  const QgsAttributeTableModel *tm = qobject_cast<const QgsAttributeTableModel *>( model );
  if ( tm )
    return tm->layer();

  const QgsAttributeTableFilterModel *fm = qobject_cast<const QgsAttributeTableFilterModel *>( model );
  if ( fm )
    return fm->layer();

  return nullptr;
}

const QgsAttributeTableModel *QgsAttributeTableDelegate::masterModel( const QAbstractItemModel *model )
{
  const QgsAttributeTableModel *tm = qobject_cast<const QgsAttributeTableModel *>( model );
  if ( tm )
    return tm;

  const QgsAttributeTableFilterModel *fm = qobject_cast<const QgsAttributeTableFilterModel *>( model );
  if ( fm )
    return fm->masterModel();

  return nullptr;
}

QWidget *QgsAttributeTableDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  QgsVectorLayer *vl = layer( index.model() );
  if ( !vl )
    return nullptr;

  int fieldIdx = index.model()->data( index, QgsAttributeTableModel::FieldIndexRole ).toInt();
  QgsAttributeEditorContext context( masterModel( index.model() )->editorContext(), QgsAttributeEditorContext::Popup );

  // Update the editor form context with the feature being edited
  QgsFeatureId fid( index.model()->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong() );
  context.setFormFeature( vl->getFeature( fid ) );

  QgsEditorWidgetWrapper *eww = QgsGui::editorWidgetRegistry()->create( vl, fieldIdx, nullptr, parent, context );
  QWidget *w = eww->widget();

  w->setAutoFillBackground( true );
  w->setFocusPolicy( Qt::StrongFocus ); // to make sure QMouseEvents are propagated to the editor widget

  const int fieldOrigin = vl->fields().fieldOrigin( fieldIdx );
  bool readOnly = true;
  if ( fieldOrigin == QgsFields::OriginJoin )
  {
    int srcFieldIndex;
    const QgsVectorLayerJoinInfo *info = vl->joinBuffer()->joinForFieldIndex( fieldIdx, vl->fields(), srcFieldIndex );

    if ( info && info->isEditable() )
      readOnly = info->joinLayer()->editFormConfig().readOnly( srcFieldIndex );
  }
  else
    readOnly = vl->editFormConfig().readOnly( fieldIdx );

  eww->setEnabled( !readOnly );

  return w;
}

void QgsAttributeTableDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = layer( model );
  if ( !vl )
    return;

  int fieldIdx = model->data( index, QgsAttributeTableModel::FieldIndexRole ).toInt();
  QgsFeatureId fid = model->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong();
  QVariant oldValue = model->data( index, Qt::EditRole );

  QgsEditorWidgetWrapper *eww = QgsEditorWidgetWrapper::fromWidget( editor );
  if ( !eww )
    return;

  QList<int> indexes = QList<int>() << fieldIdx;
  QVariantList newValues = QVariantList() << eww->value();
  const QStringList additionalFields = eww->additionalFields();
  for ( const QString &fieldName : additionalFields )
  {
    indexes << eww->layer()->fields().lookupField( fieldName );
  }
  newValues.append( eww->additionalFieldValues() );

  if ( ( oldValue != newValues.at( 0 ) && newValues.at( 0 ).isValid() )
       || oldValue.isNull() != newValues.at( 0 ).isNull()
       || newValues.count() > 1 )
  {
    // This fixes https://github.com/qgis/QGIS/issues/24398
    QgsFeatureRequest request( fid );
    request.setFlags( QgsFeatureRequest::NoGeometry );
    QgsFeature feature;
    vl->getFeatures( request ).nextFeature( feature );
    if ( feature.isValid() )
    {
      vl->beginEditCommand( tr( "Attribute changed" ) );
      for ( int i = 0; i < newValues.count(); i++ )
        vl->changeAttributeValue( fid, indexes.at( i ), newValues.at( i ), feature.attribute( indexes.at( i ) ) );
      vl->endEditCommand();
    }
  }
}

void QgsAttributeTableDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsEditorWidgetWrapper *eww = QgsEditorWidgetWrapper::fromWidget( editor );
  if ( !eww )
    return;

  QVariant value = index.model()->data( index, Qt::EditRole );
  const QStringList additionalFields = eww->additionalFields();

  if ( !additionalFields.empty() )
  {
    const QgsAttributeTableModel *model = masterModel( index.model() );
    if ( model )
    {
      QgsFeature feat = model->feature( index );
      QVariantList additionalFieldValues;
      for ( QString fieldName : additionalFields )
      {
        additionalFieldValues << feat.attribute( fieldName );
      }
      eww->setValues( value, additionalFieldValues );
    }
  }
  else
  {
    eww->setValues( value, QVariantList() );
  }
}

void QgsAttributeTableDelegate::setFeatureSelectionModel( QgsFeatureSelectionModel *featureSelectionModel )
{
  mFeatureSelectionModel = featureSelectionModel;
}

void QgsAttributeTableDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QgsAttributeTableFilterModel::ColumnType columnType = static_cast<QgsAttributeTableFilterModel::ColumnType>( index.model()->data( index, QgsAttributeTableFilterModel::TypeRole ).toInt() );

  if ( columnType == QgsAttributeTableFilterModel::ColumnTypeActionButton )
  {
    emit actionColumnItemPainted( index );
  }
  else
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
}
