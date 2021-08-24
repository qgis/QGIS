/***************************************************************************
  qgsfeaturepickermodel.cpp - QgsFeaturePickerModel
 ---------------------
 begin                : 03.04.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturepickermodel.h"
#include "qgsfeatureexpressionvaluesgatherer.h"

#include "qgsvectorlayer.h"
#include "qgsconditionalstyle.h"
#include "qgsapplication.h"
#include "qgssettings.h"


QgsFeaturePickerModel::QgsFeaturePickerModel( QObject *parent )
  :  QgsFeaturePickerModelBase( parent )
{
  setFetchGeometry( true );
  setExtraIdentifierValueUnguarded( nullIdentifier() );

  connect( this, &QgsFeaturePickerModelBase::extraIdentifierValueIndexChanged, this, [ = ]() {emit featureChanged( feature() );} );
}

void QgsFeaturePickerModel::requestToReloadCurrentFeature( QgsFeatureRequest &request )
{
  request.setFilterFid( mExtraIdentifierValue.value<QgsFeatureId>() );
}

QVariant QgsFeaturePickerModel::entryIdentifier( const QgsFeatureExpressionValuesGatherer::Entry &entry ) const
{
  return entry.identifierFields;
}

QgsFeatureExpressionValuesGatherer::Entry QgsFeaturePickerModel::createEntry( const QVariant &identifier ) const
{
  const QgsFeatureId fid = identifier.value<QgsFeatureId>();
  return QgsFeatureExpressionValuesGatherer::Entry( fid, QStringLiteral( "(%1)" ).arg( fid ), sourceLayer() );
}

bool QgsFeaturePickerModel::compareEntries( const QgsFeatureExpressionValuesGatherer::Entry &a, const QgsFeatureExpressionValuesGatherer::Entry &b ) const
{
  return a.featureId == b.featureId;
}

bool QgsFeaturePickerModel::identifierIsNull( const QVariant &identifier ) const
{
  return identifier.value<QgsFeatureId>() == nullIdentifier();
}

QVariant QgsFeaturePickerModel::nullIdentifier() const
{
  return FID_NULL;
}

void QgsFeaturePickerModel::setExtraIdentifierValueToNull()
{
  setExtraIdentifierValue( FID_NULL );
}

void QgsFeaturePickerModel::setFeature( const QgsFeatureId &fid )
{
  setExtraIdentifierValue( fid );
}

QgsFeature QgsFeaturePickerModel::feature() const
{
  return mEntries.value( mExtraValueIndex ).feature;
}

QgsFeatureExpressionValuesGatherer *QgsFeaturePickerModel::createValuesGatherer( const QgsFeatureRequest &request ) const
{
  return new QgsFeatureExpressionValuesGatherer( sourceLayer(), displayExpression(), request );
}

