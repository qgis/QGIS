/***************************************************************************
                         qgsalgorithmfieldcalculator.h
                         ----------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmfieldcalculator.h"
#include "qgsprocessingparameterfielddefinition.h"
#include "qgsexpressioncontextutils.h"

///@cond PRIVATE

QString QgsFieldCalculatorAlgorithm::name() const
{
  return QStringLiteral( "fieldcalculator" );
}

QString QgsFieldCalculatorAlgorithm::displayName() const
{
  return QObject::tr( "Field calculator" );
}

QStringList QgsFieldCalculatorAlgorithm::tags() const
{
  return QObject::tr( "field,calculator,vector" ).split( ',' );
}

QString QgsFieldCalculatorAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsFieldCalculatorAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

QString QgsFieldCalculatorAlgorithm::outputName() const
{
  return QObject::tr( "Calculated" );
}

QList<int> QgsFieldCalculatorAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVector;
}

QgsProcessingFeatureSource::Flag QgsFieldCalculatorAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

void QgsFieldCalculatorAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  std::unique_ptr< QgsProcessingParameterFieldDefinition > field = qgis::make_unique< QgsProcessingParameterFieldDefinition> ( QStringLiteral( "FIELD" ), QObject::tr( "Result field" ), QStringLiteral( "INPUT" ) );
  std::unique_ptr< QgsProcessingParameterExpression > expression = qgis::make_unique< QgsProcessingParameterExpression> ( QStringLiteral( "FORMULA" ), QObject::tr( "Formula" ), QVariant(), QStringLiteral( "INPUT" ), false );

  expression->setMetadata( QVariantMap( {{"inlineEditor", true}} ) );

  expression->setMetadata( QVariantMap( {{"inlineEditor", true}} ) );

  addParameter( field.release() );
  addParameter( expression.release() );
}

QgsFields QgsFieldCalculatorAlgorithm::outputFields( const QgsFields & ) const
{
  return mFields;
}

QString QgsFieldCalculatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a new vector layer with the same features of the input layer, "
                      "but either overwriting an existing attribute or adding an additional attribute. The values of this field "
                      "are computed from each feature using an expression, based on the properties and attributes of the feature. "
                      "Note that selecting a value in \"Result in existing field\" will ignore all the rest of the "
                      "field settings." );
}

QgsFieldCalculatorAlgorithm *QgsFieldCalculatorAlgorithm::createInstance() const
{
  return new QgsFieldCalculatorAlgorithm();
}


bool QgsFieldCalculatorAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );

  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QMap<QString, QVariant::Type> fieldTypes(
  {
    {"Real", QVariant::Double},
    {"Integer", QVariant::Int},
    {"String", QVariant::String},
    {"Date", QVariant::Date},
  } );

  // prepare fields
  QVariantMap fieldDefinitionMap = parameters.value( QStringLiteral( "FIELD" ) ).toMap();

  if ( fieldDefinitionMap.isEmpty() )
  {
    fieldDefinitionMap.insert( QStringLiteral( "name" ), parameters.value( QStringLiteral( "FIELD_NAME" ) ).toString() );
    fieldDefinitionMap.insert( QStringLiteral( "type" ), parameters.value( QStringLiteral( "FIELD_TYPE" ) ).toString() );
    fieldDefinitionMap.insert( QStringLiteral( "length" ), parameters.value( QStringLiteral( "FIELD_LENGTH" ) ).toInt() );
    fieldDefinitionMap.insert( QStringLiteral( "precision" ), parameters.value( QStringLiteral( "FIELD_PRECISION" ) ).toInt() );
  }

  const QString fieldType = fieldDefinitionMap.value( QStringLiteral( "type" ) ).toString();
  const QgsField field(
    fieldDefinitionMap.value( QStringLiteral( "name" ) ).toString(),
    fieldTypes.value( fieldType ),
    fieldType,
    fieldDefinitionMap.value( QStringLiteral( "length" ) ).toInt(),
    fieldDefinitionMap.value( QStringLiteral( "precision" ) ).toInt()
  );

  mFields = source->fields();
  int fieldIdx = mFields.lookupField( field.name() );

  if ( fieldIdx < 0 )
    mFields.append( field );

  QString dest;

  mFieldIdx = mFields.lookupField( field.name() );

  // prepare expression
  QString expressionString = parameterAsString( parameters, QStringLiteral( "FORMULA" ), context );
  mExpressionContext = createExpressionContext( parameters, context, source.get() );
  mExpression = QgsExpression( expressionString );
  mDa.setSourceCrs( source->sourceCrs(), context.transformContext() );
  mDa.setEllipsoid( context.ellipsoid() );

  mExpression.setGeomCalculator( &mDa );
  mExpression.setDistanceUnits( context.distanceUnit() );
  mExpression.setAreaUnits( context.areaUnit() );

  if ( mExpression.hasParserError() )
    throw QgsProcessingException( QObject::tr( "Parser error with formula expression \"%2\": %3" )
                                  .arg( expressionString, mExpression.parserErrorString() ) );

  mExpression.prepare( &mExpressionContext );

  return true;
}

QgsFeatureList QgsFieldCalculatorAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsAttributes attributes( mFields.size() );
  const QStringList fieldNames = mFields.names();
  for ( const QString &fieldName : fieldNames )
  {
    const int attributeIndex = feature.fieldNameIndex( fieldName );

    if ( attributeIndex >= 0 )
      attributes[attributeIndex] = feature.attribute( fieldName );
  }

  if ( mExpression.isValid() )
  {
    mExpressionContext.setFeature( feature );
    mExpressionContext.lastScope()->setVariable( QStringLiteral( "row_number" ), mRowNumber );

    const QVariant value = mExpression.evaluate( &mExpressionContext );

    if ( mExpression.hasEvalError() )
    {
      throw QgsProcessingException( QObject::tr( "Evaluation error in expression \"%1\": %2" )
                                    .arg( mExpression.expression(), mExpression.evalErrorString() ) );
    }

    attributes[mFieldIdx] = value;
    attributes.append( value );
  }
  else
  {
    attributes.append( QVariant() );
  }

  QgsFeature f = feature;
  f.setAttributes( attributes );
  mRowNumber++;
  return QgsFeatureList() << f;
}

bool QgsFieldCalculatorAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond

