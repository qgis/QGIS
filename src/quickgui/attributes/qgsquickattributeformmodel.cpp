/***************************************************************************
 qgsquickattributeformmodel.cpp
  --------------------------------------
  Date                 : 22.9.2016
  Copyright            : (C) 2016 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickattributeformmodel.h"
#include "qgsquickattributeformmodelbase.h"

QgsQuickAttributeFormModel::QgsQuickAttributeFormModel( QObject *parent )
  : QSortFilterProxyModel( parent )
  , mSourceModel( new QgsQuickAttributeFormModelBase( this ) )
{
  setSourceModel( mSourceModel );
  connect( mSourceModel, &QgsQuickAttributeFormModelBase::hasTabsChanged, this, &QgsQuickAttributeFormModel::hasTabsChanged );
  connect( mSourceModel, &QgsQuickAttributeFormModelBase::attributeModelChanged, this, &QgsQuickAttributeFormModel::attributeModelChanged );
  connect( mSourceModel, &QgsQuickAttributeFormModelBase::constraintsHardValidChanged, this, &QgsQuickAttributeFormModel::constraintsHardValidChanged );
  connect( mSourceModel, &QgsQuickAttributeFormModelBase::constraintsSoftValidChanged, this, &QgsQuickAttributeFormModel::constraintsSoftValidChanged );
}

bool QgsQuickAttributeFormModel::hasTabs() const
{
  return mSourceModel->hasTabs();
}

void QgsQuickAttributeFormModel::setHasTabs( bool hasTabs )
{
  mSourceModel->setHasTabs( hasTabs );
}

QgsQuickAttributeModel *QgsQuickAttributeFormModel::attributeModel() const
{
  return mSourceModel->attributeModel();
}

void QgsQuickAttributeFormModel::setAttributeModel( QgsQuickAttributeModel *attributeModel )
{
  mSourceModel->setAttributeModel( attributeModel );
}

bool QgsQuickAttributeFormModel::constraintsHardValid() const
{
  return mSourceModel->constraintsHardValid();
}

bool QgsQuickAttributeFormModel::constraintsSoftValid() const
{
  return mSourceModel->constraintsSoftValid();
}

bool QgsQuickAttributeFormModel::rememberValuesAllowed() const
{
  return mSourceModel->rememberValuesAllowed();
}

void QgsQuickAttributeFormModel::save()
{
  mSourceModel->save();
}

void QgsQuickAttributeFormModel::create()
{
  mSourceModel->create();
}

QVariant QgsQuickAttributeFormModel::attribute( const QString &name ) const
{
  return mSourceModel->attribute( name );
}

void QgsQuickAttributeFormModel::forceClean()
{
  mSourceModel->forceClean();
}

void QgsQuickAttributeFormModel::setRememberValuesAllowed( bool rememberValuesAllowed )
{
  mSourceModel->setRememberValuesAllowed( rememberValuesAllowed );
}

bool QgsQuickAttributeFormModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  return mSourceModel->data( mSourceModel->index( source_row, 0, source_parent ), CurrentlyVisible ).toBool();
}
