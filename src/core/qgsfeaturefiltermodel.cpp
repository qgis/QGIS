/***************************************************************************
  qgsfeaturefiltermodel.cpp - QgsFeatureFilterModel
 ---------------------
 begin                : 10.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturefiltermodel.h"
#include "qgsfeatureexpressionvaluesgatherer.h"

#include "qgsvectorlayer.h"
#include "qgsconditionalstyle.h"
#include "qgsapplication.h"
#include "qgssettings.h"


QgsFeatureFilterModel::QgsFeatureFilterModel( QObject *parent )
  : QgsFeaturePickerModelBase( parent )
{
  setExtraIdentifierValueUnguarded( QVariantList() );
}

QString QgsFeatureFilterModel::identifierField() const
{
  return mIdentifierFields.value( 0 );
}

void QgsFeatureFilterModel::requestToReloadCurrentFeature( QgsFeatureRequest &request )
{
  QStringList conditions;
  for ( int i = 0; i < mIdentifierFields.count(); i++ )
  {
    if ( i >= mExtraIdentifierValue.count() )
    {
      conditions << QgsExpression::createFieldEqualityExpression( mIdentifierFields.at( i ), QVariant() );
    }
    else
    {
      conditions << QgsExpression::createFieldEqualityExpression( mIdentifierFields.at( i ), mExtraIdentifierValue.at( i ) );
    }
  }
  request.setFilterExpression( conditions.join( QStringLiteral( " AND " ) ) );
}

QSet<QString> QgsFeatureFilterModel::requestedAttributes() const
{
  return QSet<QString>( mIdentifierFields.begin(), mIdentifierFields.end() );
}

QStringList QgsFeatureFilterModel::identifierFields() const
{
  return mIdentifierFields;
}


void QgsFeatureFilterModel::setIdentifierFields( const QStringList &identifierFields )
{
  if ( mIdentifierFields == identifierFields )
    return;

  mIdentifierFields = identifierFields;
  emit identifierFieldsChanged();
  setExtraIdentifierValueToNull();
}


QVariantList QgsFeatureFilterModel::extraIdentifierValue() const
{
  if ( mExtraIdentifierValue.count() != mIdentifierFields.count() )
  {
    QVariantList nullValues;
    for ( int i = 0; i < mIdentifierFields.count(); i++ )
      nullValues << QVariant( QVariant::Int );
    return nullValues;
  }
  return mExtraIdentifierValue;
}

void QgsFeatureFilterModel::setExtraIdentifierValueToNull()
{
  setExtraIdentifierValue( QVariantList() );
}

QgsFeatureByIdentifierFieldsExpressionValuesGatherer QgsFeatureFilterModel::createValuesGatherer( const QgsFeatureRequest &request ) const
{
  new QgsFeatureByIdentifierFieldsExpressionValuesGatherer( mSourceLayer, mDisplayExpression, request, mIdentifierFields );
}
