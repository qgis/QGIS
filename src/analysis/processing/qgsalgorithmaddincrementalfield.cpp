/***************************************************************************
                         qgsalgorithmaddincrementalfield.cpp
                         -----------------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmaddincrementalfield.h"

#include "qgsfeaturerequest.h"

///@cond PRIVATE

QString QgsAddIncrementalFieldAlgorithm::name() const
{
  return u"addautoincrementalfield"_s;
}

QString QgsAddIncrementalFieldAlgorithm::displayName() const
{
  return QObject::tr( "Add autoincremental field" );
}

QString QgsAddIncrementalFieldAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm adds a new integer field to a vector layer, with a sequential value for each feature.\n\n"
                      "This field can be used as a unique ID for features in the layer. The new attribute "
                      "is not added to the input layer but a new layer is generated instead.\n\n"
                      "The initial starting value for the incremental series can be specified.\n\n"
                      "Specifying an optional modulus value will restart the count to START whenever the field value reaches the modulus value.\n\n"
                      "Optionally, grouping fields can be specified. If group fields are present, then the field value will "
                      "be reset for each combination of these group field values.\n\n"
                      "The sort order for features may be specified, if so, then the incremental field will respect "
                      "this sort order." );
}

QString QgsAddIncrementalFieldAlgorithm::shortDescription() const
{
  return QObject::tr( "Adds a new integer field to a vector layer, with a sequential value for each feature, "
                      "usable as a unique ID for features in the layer." );
}

QStringList QgsAddIncrementalFieldAlgorithm::tags() const
{
  return QObject::tr( "add,create,serial,primary,key,unique,fields" ).split( ',' );
}

QString QgsAddIncrementalFieldAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsAddIncrementalFieldAlgorithm::groupId() const
{
  return u"vectortable"_s;
}

QString QgsAddIncrementalFieldAlgorithm::outputName() const
{
  return QObject::tr( "Incremented" );
}

QList<int> QgsAddIncrementalFieldAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector );
}

QgsAddIncrementalFieldAlgorithm *QgsAddIncrementalFieldAlgorithm::createInstance() const
{
  return new QgsAddIncrementalFieldAlgorithm();
}

Qgis::ProcessingFeatureSourceFlags QgsAddIncrementalFieldAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

void QgsAddIncrementalFieldAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( u"FIELD_NAME"_s, QObject::tr( "Field name" ), u"AUTO"_s ) );
  addParameter( new QgsProcessingParameterNumber( u"START"_s, QObject::tr( "Start values at" ), Qgis::ProcessingNumberParameterType::Integer, 0, true ) );
  addParameter( new QgsProcessingParameterNumber( u"MODULUS"_s, QObject::tr( "Modulus value" ), Qgis::ProcessingNumberParameterType::Integer, QVariant( 0 ), true ) );
  addParameter( new QgsProcessingParameterField( u"GROUP_FIELDS"_s, QObject::tr( "Group values by" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  // sort params
  auto sortExp = std::make_unique<QgsProcessingParameterExpression>( u"SORT_EXPRESSION"_s, QObject::tr( "Sort expression" ), QVariant(), u"INPUT"_s, true );
  sortExp->setFlags( sortExp->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( sortExp.release() );
  auto sortAscending = std::make_unique<QgsProcessingParameterBoolean>( u"SORT_ASCENDING"_s, QObject::tr( "Sort ascending" ), true );
  sortAscending->setFlags( sortAscending->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( sortAscending.release() );
  auto sortNullsFirst = std::make_unique<QgsProcessingParameterBoolean>( u"SORT_NULLS_FIRST"_s, QObject::tr( "Sort nulls first" ), false );
  sortNullsFirst->setFlags( sortNullsFirst->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( sortNullsFirst.release() );
}

QgsFields QgsAddIncrementalFieldAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outFields = inputFields;
  outFields.append( QgsField( mFieldName, QMetaType::Type::LongLong ) );
  mFields = outFields;
  return outFields;
}

bool QgsAddIncrementalFieldAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  mStartValue = parameterAsInt( parameters, u"START"_s, context );
  mValue = mStartValue;
  mModulusValue = parameterAsInt( parameters, u"MODULUS"_s, context );
  mFieldName = parameterAsString( parameters, u"FIELD_NAME"_s, context );
  mGroupedFieldNames = parameterAsStrings( parameters, u"GROUP_FIELDS"_s, context );

  mSortExpressionString = parameterAsExpression( parameters, u"SORT_EXPRESSION"_s, context );
  mSortAscending = parameterAsBoolean( parameters, u"SORT_ASCENDING"_s, context );
  mSortNullsFirst = parameterAsBoolean( parameters, u"SORT_NULLS_FIRST"_s, context );

  if ( source->fields().lookupField( mFieldName ) >= 0 )
  {
    throw QgsProcessingException( QObject::tr( "A field with the same name (%1) already exists" ).arg( mFieldName ) );
  }

  return true;
}

QgsFeatureRequest QgsAddIncrementalFieldAlgorithm::request() const
{
  if ( mSortExpressionString.isEmpty() )
    return QgsFeatureRequest();

  return QgsFeatureRequest().setOrderBy( QgsFeatureRequest::OrderBy() << QgsFeatureRequest::OrderByClause( mSortExpressionString, mSortAscending, mSortNullsFirst ) );
}

QgsFeatureList QgsAddIncrementalFieldAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !mGroupedFieldNames.empty() && mGroupedFields.empty() )
  {
    for ( const QString &field : std::as_const( mGroupedFieldNames ) )
    {
      int idx = mFields.lookupField( field );
      if ( idx >= 0 )
        mGroupedFields << idx;
    }
  }

  QgsFeature f = feature;
  QgsAttributes attributes = f.attributes();
  if ( mGroupedFields.empty() )
  {
    attributes.append( mValue );
    mValue++;
    if ( mModulusValue != 0 && ( mValue % mModulusValue ) == 0 )
      mValue = mStartValue;
  }
  else
  {
    QgsAttributes groupAttributes;
    groupAttributes.reserve( mGroupedFields.size() );
    for ( int index : std::as_const( mGroupedFields ) )
    {
      groupAttributes << f.attribute( index );
    }
    long long value = mGroupedValues.value( groupAttributes, mStartValue );
    attributes.append( value );
    value++;
    if ( mModulusValue != 0 && ( value % mModulusValue ) == 0 )
      value = mStartValue;
    mGroupedValues[groupAttributes] = value;
  }
  f.setAttributes( attributes );
  return QgsFeatureList() << f;
}

bool QgsAddIncrementalFieldAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond
