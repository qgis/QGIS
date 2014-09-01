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

QgsFieldComboBox::QgsFieldComboBox( QWidget *parent ) :
    QComboBox( parent )
{
  mFieldProxyModel = new QgsFieldProxyModel( this );
  setModel( mFieldProxyModel );

  connect( this, SIGNAL( activated( int ) ), this, SLOT( indexChanged( int ) ) );
}

void QgsFieldComboBox::setFilters( QgsFieldProxyModel::Filters filters )
{
  mFieldProxyModel->setFilters( filters );
}

void QgsFieldComboBox::setLayer( QgsMapLayer *layer )
{
  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
  if ( vl )
  {
    setLayer( vl );
  }
}

void QgsFieldComboBox::setLayer( QgsVectorLayer *layer )
{
  mFieldProxyModel->sourceFieldModel()->setLayer( layer );
}

QgsVectorLayer *QgsFieldComboBox::layer() const
{
  return mFieldProxyModel->sourceFieldModel()->layer();
}

void QgsFieldComboBox::setField( QString fieldName )
{
  QModelIndex idx = mFieldProxyModel->sourceFieldModel()->indexFromName( fieldName );
  if ( idx.isValid() )
  {
    QModelIndex proxyIdx = mFieldProxyModel->mapFromSource( idx );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( idx.row() );
      emit fieldChanged( currentField() );
      return;
    }
  }
  setCurrentIndex( -1 );
}

QString QgsFieldComboBox::currentField() const
{
  int i = currentIndex();

  const QModelIndex proxyIndex = mFieldProxyModel->index( i, 0 );
  if ( !proxyIndex.isValid() )
  {
    return "";
  }

  QString name = mFieldProxyModel->data( proxyIndex, QgsFieldModel::FieldNameRole ).toString();
  return name;
}

void QgsFieldComboBox::indexChanged( int i )
{
  Q_UNUSED( i );
  QString name = currentField();
  emit fieldChanged( name );
}
