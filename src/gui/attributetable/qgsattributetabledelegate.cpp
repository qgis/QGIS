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
#include "qgsvectordataprovider.h"
#include "qgsactionmanager.h"
#include "qgsgui.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsrendercontext.h"

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

  const int fieldIdx = index.model()->data( index, static_cast< int >( QgsAttributeTableModel::CustomRole::FieldIndex ) ).toInt();
  QgsAttributeEditorContext context( masterModel( index.model() )->editorContext(), QgsAttributeEditorContext::Popup );

  // Update the editor form context with the feature being edited
  const QgsFeatureId fid( index.model()->data( index, static_cast< int >( QgsAttributeTableModel::CustomRole::FeatureId ) ).toLongLong() );
  context.setFormFeature( vl->getFeature( fid ) );

  QgsEditorWidgetWrapper *eww = QgsGui::editorWidgetRegistry()->create( vl, fieldIdx, nullptr, parent, context );
  QWidget *w = eww->widget();

  w->setAutoFillBackground( true );
  w->setFocusPolicy( Qt::StrongFocus ); // to make sure QMouseEvents are propagated to the editor widget

  const Qgis::FieldOrigin fieldOrigin = vl->fields().fieldOrigin( fieldIdx );
  bool readOnly = true;
  if ( fieldOrigin == Qgis::FieldOrigin::Join )
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

  const int fieldIdx = model->data( index, static_cast< int >( QgsAttributeTableModel::CustomRole::FieldIndex ) ).toInt();
  const QgsFeatureId fid = model->data( index, static_cast< int >( QgsAttributeTableModel::CustomRole::FeatureId ) ).toLongLong();
  const QVariant oldValue = model->data( index, Qt::EditRole );

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
       || QgsVariantUtils::isNull( oldValue ) != QgsVariantUtils::isNull( newValues.at( 0 ) )
       || newValues.count() > 1 )
  {
    // This fixes https://github.com/qgis/QGIS/issues/24398
    QgsFeatureRequest request( fid );
    request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
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

  const QVariant value = index.model()->data( index, Qt::EditRole );
  const QStringList additionalFields = eww->additionalFields();

  if ( !additionalFields.empty() )
  {
    const QgsAttributeTableModel *model = masterModel( index.model() );
    if ( model )
    {
      const QgsFeature feat = model->feature( index );
      QVariantList additionalFieldValues;
      for ( const QString &fieldName : additionalFields )
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
  const QgsAttributeTableFilterModel::ColumnType columnType = static_cast<QgsAttributeTableFilterModel::ColumnType>( index.model()->data( index, static_cast< int >( QgsAttributeTableFilterModel::CustomRole::Type ) ).toInt() );

  if ( columnType == QgsAttributeTableFilterModel::ColumnTypeActionButton )
  {
    emit actionColumnItemPainted( index );
  }
  else
  {
    const QgsFeatureId fid = index.model()->data( index, static_cast< int >( QgsAttributeTableModel::CustomRole::FeatureId ) ).toLongLong();

    QStyleOptionViewItem myOpt = option;

    if ( QgsVariantUtils::isNull( index.model()->data( index, Qt::EditRole ) ) )
    {
      myOpt.font.setItalic( true );
      myOpt.palette.setColor( QPalette::Text, QColor( "gray" ) );
    }

    if ( mFeatureSelectionModel && mFeatureSelectionModel->isSelected( fid ) )
      myOpt.state |= QStyle::State_Selected;

    QItemDelegate::paint( painter, myOpt, index );

    if ( option.state & QStyle::State_HasFocus )
    {
      const QRect r = option.rect.adjusted( 1, 1, -1, -1 );
      const QPen p( QBrush( QColor( 0, 255, 127 ) ), 2 );
      const QgsScopedQPainterState painterState( painter );
      painter->setPen( p );
      painter->drawRect( r );
    }
  }
}
