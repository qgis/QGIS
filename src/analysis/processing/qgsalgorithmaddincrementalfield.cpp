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

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsAddIncrementalFieldAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

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
                      "Optionally, grouping fields can be specified. If group fields are present, then the field value will "
                      "be reset for each combination of these group field values." );
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
  return QList<int>() << QgsProcessing::TypeVector;
}

QgsAddIncrementalFieldAlgorithm *QgsAddIncrementalFieldAlgorithm::createInstance() const
{
  return new QgsAddIncrementalFieldAlgorithm();
}

void QgsAddIncrementalFieldAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ), QObject::tr( "Field name" ), QStringLiteral( "AUTO" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "START" ), QObject::tr( "Start values at" ),
                QgsProcessingParameterNumber::Integer, 0, true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "GROUP_FIELDS" ), QObject::tr( "Group values by" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true ) );
}

QgsFields QgsAddIncrementalFieldAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outFields = inputFields;
  outFields.append( QgsField( mFieldName, QVariant::LongLong ) );
  mFields = outFields;
  return outFields;
}

bool QgsAddIncrementalFieldAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mStartValue = parameterAsInt( parameters, QStringLiteral( "START" ), context );
  mValue = mStartValue;
  mFieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  mGroupedFieldNames = parameterAsFields( parameters, QStringLiteral( "GROUP_FIELDS" ), context );
  return true;
}

QgsFeature QgsAddIncrementalFieldAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !mGroupedFieldNames.empty() && mGroupedFields.empty() )
  {
    for ( const QString &field : mGroupedFieldNames )
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
  }
  else
  {
    QgsAttributes groupAttributes;
    for ( int index : qgis::as_const( mGroupedFields ) )
    {
      groupAttributes << f.attribute( index );
    }
    long long value = mGroupedValues.value( groupAttributes, mStartValue );
    attributes.append( value );
    value++;
    mGroupedValues[ groupAttributes ] = value;
  }
  f.setAttributes( attributes );
  return f;
}

///@endcond
