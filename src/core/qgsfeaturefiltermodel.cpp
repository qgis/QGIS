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
#include "qgssettings.h"
#include "qgsvariantutils.h"

bool qVariantListCompare( const QVariantList &a, const QVariantList &b )
{
  if ( a.size() != b.size() )
    return false;

  for ( int i = 0; i < a.size(); ++i )
  {
    if ( !qgsVariantEqual( a.at( i ), b.at( i ) ) )
      return false;
  }
  return true;
}


QgsFeatureFilterModel::QgsFeatureFilterModel( QObject *parent )
  : QgsFeaturePickerModelBase( parent )
{
  setFetchGeometry( false );
  setFetchLimit( QgsSettings().value( QStringLiteral( "maxEntriesRelationWidget" ), 100, QgsSettings::Gui ).toInt() );
  setExtraIdentifierValueUnguarded( nullIdentifier() );
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
    if ( i >= mExtraIdentifierValue.toList().count() )
    {
      conditions << QgsExpression::createFieldEqualityExpression( mIdentifierFields.at( i ), QVariant() );
    }
    else
    {
      conditions << QgsExpression::createFieldEqualityExpression( mIdentifierFields.at( i ), mExtraIdentifierValue.toList().at( i ) );
    }
  }
  request.setFilterExpression( conditions.join( QLatin1String( " AND " ) ) );
}

QSet<QString> QgsFeatureFilterModel::requestedAttributes() const
{
  return QSet<QString>( mIdentifierFields.begin(), mIdentifierFields.end() );
}

QVariant QgsFeatureFilterModel::entryIdentifier( const QgsFeatureExpressionValuesGatherer::Entry &entry ) const
{
  return entry.featureId;
}

QgsFeatureExpressionValuesGatherer::Entry QgsFeatureFilterModel::createEntry( const QVariant &identifier ) const
{
  const QVariantList constValues = identifier.toList();

  QStringList values;
  for ( const QVariant &v : constValues )
    values << QStringLiteral( "(%1)" ).arg( v.toString() );

  return QgsFeatureExpressionValuesGatherer::Entry( constValues, values.join( QLatin1Char( ' ' ) ), QgsFeature( sourceLayer() ? sourceLayer()->fields() : QgsFields() ) );
}

bool QgsFeatureFilterModel::compareEntries( const QgsFeatureExpressionValuesGatherer::Entry &a, const QgsFeatureExpressionValuesGatherer::Entry &b ) const
{
  return qVariantListCompare( a.identifierFields, b.identifierFields );
}

bool QgsFeatureFilterModel::identifierIsNull( const QVariant &identifier ) const
{
  const QVariantList values = identifier.toList();
  for ( const QVariant &value : values )
  {
    if ( !QgsVariantUtils::isNull( value ) )
    {
      return false;
    }
  }
  return true;
}

QVariant QgsFeatureFilterModel::nullIdentifier() const
{
  QVariantList nullValues;
  for ( int i = 0; i < mIdentifierFields.count(); i++ )
    nullValues << QVariant( QVariant::Int );
  return nullValues;
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

QgsFeatureExpressionValuesGatherer *QgsFeatureFilterModel::createValuesGatherer( const QgsFeatureRequest &request ) const
{
  return new QgsFeatureExpressionValuesGatherer( sourceLayer(), displayExpression(), request, mIdentifierFields );
}


QVariantList QgsFeatureFilterModel::extraIdentifierValues() const
{
  QVariantList values = mExtraIdentifierValue.toList();
  if ( values.count() != mIdentifierFields.count() )
  {
    return nullIdentifier().toList();
  }
  return values;
}

void QgsFeatureFilterModel::setExtraIdentifierValues( const QVariantList &extraIdentifierValues )
{
  setExtraIdentifierValue( extraIdentifierValues );
}

void QgsFeatureFilterModel::setExtraIdentifierValueToNull()
{
  setExtraIdentifierValue( nullIdentifier() );
}

