/***************************************************************************
   qgspointcloudattributecombobox.cpp
    --------------------------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgspointcloudattributecombobox.h"
#include "qgsfieldproxymodel.h"
#include "qgsmaplayer.h"
#include "qgspointcloudlayer.h"

QgsPointCloudAttributeComboBox::QgsPointCloudAttributeComboBox( QWidget *parent )
  : QComboBox( parent )
{
  mAttributeModel = new QgsPointCloudAttributeModel( this );
  mProxyModel = new QgsPointCloudAttributeProxyModel( mAttributeModel, this );
  setModel( mProxyModel );

  connect( this, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::activated ), this, &QgsPointCloudAttributeComboBox::indexChanged );
}

void QgsPointCloudAttributeComboBox::setFilters( QgsPointCloudAttributeProxyModel::Filters filters )
{
  mProxyModel->setFilters( filters );
}

void QgsPointCloudAttributeComboBox::setAllowEmptyAttributeName( bool allowEmpty )
{
  mAttributeModel->setAllowEmptyAttributeName( allowEmpty );
}

bool QgsPointCloudAttributeComboBox::allowEmptyAttributeName() const
{
  return mAttributeModel->allowEmptyAttributeName();
}

void QgsPointCloudAttributeComboBox::setLayer( QgsMapLayer *layer )
{
  QgsPointCloudLayer *pcl = qobject_cast<QgsPointCloudLayer *>( layer );
  mAttributeModel->setLayer( pcl );
}

QgsPointCloudLayer *QgsPointCloudAttributeComboBox::layer() const
{
  return mAttributeModel->layer();
}

void QgsPointCloudAttributeComboBox::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributeModel->setAttributes( attributes );
}

QgsPointCloudAttributeCollection QgsPointCloudAttributeComboBox::attributes() const
{
  return mAttributeModel->attributes();
}

void QgsPointCloudAttributeComboBox::setAttribute( const QString &name )
{
  const QString prevAttribute = currentAttribute();
  const QModelIndex idx = mAttributeModel->indexFromName( name );
  if ( idx.isValid() )
  {
    const QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
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

  if ( prevAttribute != currentAttribute() )
    emit attributeChanged( currentAttribute() );
}

QString QgsPointCloudAttributeComboBox::currentAttribute() const
{
  const int i = currentIndex();

  const QModelIndex proxyIndex = mProxyModel->index( i, 0 );
  if ( !proxyIndex.isValid() )
  {
    return QString();
  }

  return mProxyModel->data( proxyIndex, QgsPointCloudAttributeModel::AttributeNameRole ).toString();
}

void QgsPointCloudAttributeComboBox::indexChanged( int i )
{
  Q_UNUSED( i )
  const QString name = currentAttribute();
  emit attributeChanged( name );
}
