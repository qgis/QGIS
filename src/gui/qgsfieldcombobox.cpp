/***************************************************************************
   qgsfieldcombobox.cpp
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsfieldcombobox.h"
#include "qgsfieldproxymodel.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsfieldmodel.h"

QgsFieldComboBox::QgsFieldComboBox( QWidget *parent )
  : QComboBox( parent )
{
  mFieldProxyModel = new QgsFieldProxyModel( this );
  setModel( mFieldProxyModel );

  connect( this, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::activated ), this, &QgsFieldComboBox::indexChanged );
}

void QgsFieldComboBox::setFilters( QgsFieldProxyModel::Filters filters )
{
  mFieldProxyModel->setFilters( filters );
}

void QgsFieldComboBox::setAllowEmptyFieldName( bool allowEmpty )
{
  mFieldProxyModel->sourceFieldModel()->setAllowEmptyFieldName( allowEmpty );
}

bool QgsFieldComboBox::allowEmptyFieldName() const
{
  return mFieldProxyModel->sourceFieldModel()->allowEmptyFieldName();
}

void QgsFieldComboBox::setLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  mFieldProxyModel->sourceFieldModel()->setLayer( vl );
}

QgsVectorLayer *QgsFieldComboBox::layer() const
{
  return mFieldProxyModel->sourceFieldModel()->layer();
}

void QgsFieldComboBox::setFields( const QgsFields &fields )
{
  mFieldProxyModel->sourceFieldModel()->setFields( fields );
}

QgsFields QgsFieldComboBox::fields() const
{
  return mFieldProxyModel->sourceFieldModel()->fields();
}

void QgsFieldComboBox::setField( const QString &fieldName )
{
  const QString prevField = currentField();
  const QModelIndex idx = mFieldProxyModel->sourceFieldModel()->indexFromName( fieldName );
  if ( idx.isValid() )
  {
    const QModelIndex proxyIdx = mFieldProxyModel->mapFromSource( idx );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( proxyIdx.row() );
    }
    else
    {
      setCurrentIndex( -1 );
    }
  }
  else
  {
    setCurrentIndex( -1 );
  }

  if ( prevField != currentField() )
    emit fieldChanged( currentField() );
}

QString QgsFieldComboBox::currentField() const
{
  const int i = currentIndex();

  const QModelIndex proxyIndex = mFieldProxyModel->index( i, 0 );
  if ( !proxyIndex.isValid() )
  {
    return QString();
  }

  QString name = mFieldProxyModel->data( proxyIndex, QgsFieldModel::FieldNameRole ).toString();
  return name;
}

void QgsFieldComboBox::indexChanged( int i )
{
  Q_UNUSED( i )
  const QString name = currentField();
  emit fieldChanged( name );
}
