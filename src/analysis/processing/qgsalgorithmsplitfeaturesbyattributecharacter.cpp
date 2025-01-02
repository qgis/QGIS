/***************************************************************************
                         qgsalgorithmsplitfeaturesbyattributecharacter.cpp
                         ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmsplitfeaturesbyattributecharacter.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometrycollection.h"
#include <QRegularExpression>

///@cond PRIVATE

QString QgsSplitFeaturesByAttributeCharacterAlgorithm::name() const
{
  return QStringLiteral( "splitfeaturesbycharacter" );
}

QString QgsSplitFeaturesByAttributeCharacterAlgorithm::displayName() const
{
  return QObject::tr( "Split features by character" );
}

QStringList QgsSplitFeaturesByAttributeCharacterAlgorithm::tags() const
{
  return QObject::tr( "separate,attribute,value,string" ).split( ',' );
}

QString QgsSplitFeaturesByAttributeCharacterAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSplitFeaturesByAttributeCharacterAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsSplitFeaturesByAttributeCharacterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm splits features into multiple output features by splitting a field's value with a specified character.\n\n"
                      "For instance, if a layer contains features with multiple comma separated values contained in a single field, this "
                      "algorithm can be used to split these values up across multiple output features.\n\n"
                      "Geometries and other attributes remain unchanged in the output.\n\n"
                      "Optionally, the separator string can be a regular expression for added flexibility." );
}

QString QgsSplitFeaturesByAttributeCharacterAlgorithm::shortDescription() const
{
  return QObject::tr( "Splits features into multiple output features by splitting a field by a character." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsSplitFeaturesByAttributeCharacterAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QList<int> QgsSplitFeaturesByAttributeCharacterAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector );
}

void QgsSplitFeaturesByAttributeCharacterAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Split using values in field" ), QVariant(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "CHAR" ), QObject::tr( "Split values using character" ) ) );
  std::unique_ptr<QgsProcessingParameterDefinition> regexParam = std::make_unique<QgsProcessingParameterBoolean>( QStringLiteral( "REGEX" ), QObject::tr( "Use regular expression separator" ) );
  regexParam->setFlags( regexParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( regexParam.release() );
}

Qgis::ProcessingSourceType QgsSplitFeaturesByAttributeCharacterAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::Vector;
}

QgsSplitFeaturesByAttributeCharacterAlgorithm *QgsSplitFeaturesByAttributeCharacterAlgorithm::createInstance() const
{
  return new QgsSplitFeaturesByAttributeCharacterAlgorithm();
}

QgsFields QgsSplitFeaturesByAttributeCharacterAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  mFieldIndex = inputFields.lookupField( mFieldName );
  QgsFields outputFields;
  for ( int i = 0; i < inputFields.count(); ++i )
  {
    if ( i != mFieldIndex )
    {
      outputFields.append( inputFields.at( i ) );
    }
    else
    {
      // we need to convert the split field to a string field
      outputFields.append( QgsField( inputFields.at( i ).name(), QMetaType::Type::QString ) );
    }
  }
  return outputFields;
}

QString QgsSplitFeaturesByAttributeCharacterAlgorithm::outputName() const
{
  return QObject::tr( "Split" );
}

bool QgsSplitFeaturesByAttributeCharacterAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mChar = parameterAsString( parameters, QStringLiteral( "CHAR" ), context );
  mFieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  mUseRegex = parameterAsBoolean( parameters, QStringLiteral( "REGEX" ), context );
  if ( mUseRegex )
    mRegex = QRegularExpression( mChar );
  return true;
}

QgsFeatureList QgsSplitFeaturesByAttributeCharacterAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeatureList res;
  const QString val = f.attribute( mFieldIndex ).toString();
  const QStringList parts = mUseRegex ? val.split( mRegex ) : val.split( mChar );
  res.reserve( parts.size() );
  for ( const QString &p : parts )
  {
    QgsFeature out = f;
    out.setAttribute( mFieldIndex, p );
    res << out;
  }
  return res;
}

QgsFeatureSink::SinkFlags QgsSplitFeaturesByAttributeCharacterAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

///@endcond
