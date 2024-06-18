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
#include "qgsexpressioncontextutils.h"
#include "qgsvariantutils.h"

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
  return QList<int>() << static_cast< int >( Qgis::ProcessingSourceType::Vector );
}

Qgis::ProcessingFeatureSourceFlags QgsFieldCalculatorAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

void QgsFieldCalculatorAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  QStringList fieldTypes;
  QVariantList icons;
  fieldTypes.reserve( 11 );
  icons.reserve( 11 );
  for ( const auto &type :
        std::vector < std::pair< QMetaType::Type, QMetaType::Type > >
{
  {QMetaType::Type::Double, QMetaType::Type::UnknownType },
  {QMetaType::Type::Int, QMetaType::Type::UnknownType },
  {QMetaType::Type::QString, QMetaType::Type::UnknownType },
  {QMetaType::Type::QDate, QMetaType::Type::UnknownType },
  {QMetaType::Type::QTime, QMetaType::Type::UnknownType },
  {QMetaType::Type::QDateTime, QMetaType::Type::UnknownType },
  {QMetaType::Type::Bool, QMetaType::Type::UnknownType },
  {QMetaType::Type::QByteArray, QMetaType::Type::UnknownType },
  {QMetaType::Type::QStringList, QMetaType::Type::UnknownType },
  {QMetaType::Type::QVariantList, QMetaType::Type::Int },
  {QMetaType::Type::QVariantList, QMetaType::Type::Double }
} )
  {
    fieldTypes << QgsVariantUtils::typeToDisplayString( type.first, type.second );
    icons << QgsFields::iconForFieldType( type.first, type.second );
  }

  std::unique_ptr< QgsProcessingParameterString > fieldName = std::make_unique< QgsProcessingParameterString > ( QStringLiteral( "FIELD_NAME" ), QObject::tr( "Field name" ), QVariant(), false );
  std::unique_ptr< QgsProcessingParameterEnum > fieldType = std::make_unique< QgsProcessingParameterEnum > ( QStringLiteral( "FIELD_TYPE" ), QObject::tr( "Result field type" ), fieldTypes, false, 0 );
  fieldType->setMetadata(
  {
    QVariantMap( {{
        QStringLiteral( "widget_wrapper" ),
        QVariantMap(
        { {
            QStringLiteral( "icons" ), icons
          }}
        )
      }} )
  } );

  std::unique_ptr< QgsProcessingParameterNumber > fieldLength = std::make_unique< QgsProcessingParameterNumber > ( QStringLiteral( "FIELD_LENGTH" ), QObject::tr( "Result field length" ), Qgis::ProcessingNumberParameterType::Integer, QVariant( 0 ), false, 0 );
  std::unique_ptr< QgsProcessingParameterNumber > fieldPrecision = std::make_unique< QgsProcessingParameterNumber > ( QStringLiteral( "FIELD_PRECISION" ), QObject::tr( "Result field precision" ), Qgis::ProcessingNumberParameterType::Integer, QVariant( 0 ), false, 0 );
  std::unique_ptr< QgsProcessingParameterExpression > expression = std::make_unique< QgsProcessingParameterExpression> ( QStringLiteral( "FORMULA" ), QObject::tr( "Formula" ), QVariant(), QStringLiteral( "INPUT" ), false );

  expression->setMetadata( QVariantMap( {{"inlineEditor", true}} ) );

  addParameter( fieldName.release() );
  addParameter( fieldType.release() );
  addParameter( fieldLength.release() );
  addParameter( fieldPrecision.release() );
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
                      "Note that if \"Field name\" is an existing field in the layer then all the rest of the field settings are ignored." );
}

QgsFieldCalculatorAlgorithm *QgsFieldCalculatorAlgorithm::createInstance() const
{
  return new QgsFieldCalculatorAlgorithm();
}


bool QgsFieldCalculatorAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );

  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  // prepare fields
  const int fieldTypeIdx = parameterAsInt( parameters, QStringLiteral( "FIELD_TYPE" ), context );
  const int fieldLength = parameterAsInt( parameters, QStringLiteral( "FIELD_LENGTH" ), context );
  const int fieldPrecision = parameterAsInt( parameters, QStringLiteral( "FIELD_PRECISION" ), context );
  const QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );

  QMetaType::Type fieldType = QMetaType::Type::QString;
  QMetaType::Type fieldSubType = QMetaType::Type::UnknownType;
  switch ( fieldTypeIdx )
  {
    case 0: // Float
      fieldType = QMetaType::Type::Double;
      break;
    case 1: // Integer
      fieldType = QMetaType::Type::Int;
      break;
    case 2: // String
      fieldType = QMetaType::Type::QString;
      break;
    case 3: // Date
      fieldType = QMetaType::Type::QDate;
      break;
    case 4: // Time
      fieldType = QMetaType::Type::QTime;
      break;
    case 5: // DateTime
      fieldType = QMetaType::Type::QDateTime;
      break;
    case 6: // Boolean
      fieldType = QMetaType::Type::Bool;
      break;
    case 7: // Binary
      fieldType = QMetaType::Type::QByteArray;
      break;
    case 8: // StringList
      fieldType = QMetaType::Type::QStringList;
      fieldSubType = QMetaType::Type::QString;
      break;
    case 9: // IntegerList
      fieldType = QMetaType::Type::QVariantList;
      fieldSubType = QMetaType::Type::Int;
      break;
    case 10: // DoubleList
      fieldType = QMetaType::Type::QVariantList;
      fieldSubType = QMetaType::Type::Double;
      break;
  }

  if ( fieldName.isEmpty() )
    throw QgsProcessingException( QObject::tr( "Field name must not be an empty string" ) );

  const QgsField field(
    fieldName,
    fieldType,
    QString(),
    fieldLength,
    fieldPrecision,
    QString(),
    fieldSubType
  );

  mFields = source->fields();

  const int fieldIdx = mFields.indexFromName( field.name() );

  if ( fieldIdx < 0 )
  {
    mFields.append( field );
  }
  else
  {
    feedback->pushWarning( QObject::tr( "Field name %1 already exists and will be replaced" ).arg( field.name() ) );
  }

  mFieldIdx = mFields.lookupField( field.name() );

  // prepare expression
  const QString expressionString = parameterAsString( parameters, QStringLiteral( "FORMULA" ), context );
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
  }
  else
  {
    attributes[mFieldIdx] = QVariant();
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
