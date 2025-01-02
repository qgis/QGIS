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
  return QStringLiteral( "addautoincrementalfield" );
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
  return QStringLiteral( "vectortable" );
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
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ), QObject::tr( "Field name" ), QStringLiteral( "AUTO" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "START" ), QObject::tr( "Start values at" ), Qgis::ProcessingNumberParameterType::Integer, 0, true ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MODULUS" ), QObject::tr( "Modulus value" ), Qgis::ProcessingNumberParameterType::Integer, QVariant( 0 ), true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "GROUP_FIELDS" ), QObject::tr( "Group values by" ), QVariant(), QStringLiteral( "INPUT" ), Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  // sort params
  std::unique_ptr<QgsProcessingParameterExpression> sortExp = std::make_unique<QgsProcessingParameterExpression>( QStringLiteral( "SORT_EXPRESSION" ), QObject::tr( "Sort expression" ), QVariant(), QStringLiteral( "INPUT" ), true );
  sortExp->setFlags( sortExp->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( sortExp.release() );
  std::unique_ptr<QgsProcessingParameterBoolean> sortAscending = std::make_unique<QgsProcessingParameterBoolean>( QStringLiteral( "SORT_ASCENDING" ), QObject::tr( "Sort ascending" ), true );
  sortAscending->setFlags( sortAscending->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( sortAscending.release() );
  std::unique_ptr<QgsProcessingParameterBoolean> sortNullsFirst = std::make_unique<QgsProcessingParameterBoolean>( QStringLiteral( "SORT_NULLS_FIRST" ), QObject::tr( "Sort nulls first" ), false );
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
  mStartValue = parameterAsInt( parameters, QStringLiteral( "START" ), context );
  mValue = mStartValue;
  mModulusValue = parameterAsInt( parameters, QStringLiteral( "MODULUS" ), context );
  mFieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  mGroupedFieldNames = parameterAsStrings( parameters, QStringLiteral( "GROUP_FIELDS" ), context );

  mSortExpressionString = parameterAsExpression( parameters, QStringLiteral( "SORT_EXPRESSION" ), context );
  mSortAscending = parameterAsBoolean( parameters, QStringLiteral( "SORT_ASCENDING" ), context );
  mSortNullsFirst = parameterAsBoolean( parameters, QStringLiteral( "SORT_NULLS_FIRST" ), context );

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
