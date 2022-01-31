/***************************************************************************
                         qgsalgorithmrefactorfields.h
                         ---------------------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsalgorithmrefactorfields.h"
#include "qgsprocessingparameterfieldmap.h"

///@cond PRIVATE

QString QgsRefactorFieldsAlgorithm::name() const
{
  return QStringLiteral( "refactorfields" );
}

QString QgsRefactorFieldsAlgorithm::displayName() const
{
  return QObject::tr( "Refactor fields" );
}

QString QgsRefactorFieldsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm allows editing the structure of the attributes table of a vector layer. Fields can be modified "
                      "in their type and name, using a fields mapping.\n\n"
                      "The original layer is not modified. A new layer is generated, which contains a modified attribute table, according "
                      "to the provided fields mapping.\n\n"
                      "Rows in orange have constraints in the template layer from which these fields were loaded. Treat this information "
                      "as a hint during configuration. No constraints will be added on an output layer nor will they be checked or "
                      "enforced by the algorithm." );
}

QStringList QgsRefactorFieldsAlgorithm::tags() const
{
  return QObject::tr( "attributes,table" ).split( ',' );
}

QString QgsRefactorFieldsAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsRefactorFieldsAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

QString QgsRefactorFieldsAlgorithm::outputName() const
{
  return QObject::tr( "Refactored" );
}

QList<int> QgsRefactorFieldsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVector;
}

QgsProcessingFeatureSource::Flag QgsRefactorFieldsAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsRefactorFieldsAlgorithm *QgsRefactorFieldsAlgorithm::createInstance() const
{
  return new QgsRefactorFieldsAlgorithm();
}

void QgsRefactorFieldsAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterFieldMapping > param = std::make_unique< QgsProcessingParameterFieldMapping> ( QStringLiteral( "FIELDS_MAPPING" ), QObject::tr( "Fields mapping" ), QStringLiteral( "INPUT" ) );
  addParameter( param.release() );
}

QgsFields QgsRefactorFieldsAlgorithm::outputFields( const QgsFields & ) const
{
  return mFields;
}

bool QgsRefactorFieldsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  mDa.setSourceCrs( source->sourceCrs(), context.transformContext() );
  mDa.setEllipsoid( context.ellipsoid() );

  mExpressionContext = createExpressionContext( parameters, context, source.get() );

  const QVariantList mapping = parameters.value( QStringLiteral( "FIELDS_MAPPING" ) ).toList();
  for ( const QVariant &map : mapping )
  {
    const QVariantMap fieldDef = map.toMap();
    const QString name = fieldDef.value( QStringLiteral( "name" ) ).toString();
    if ( name.isEmpty() )
      throw QgsProcessingException( QObject::tr( "Field name cannot be empty" ) );

    const QVariant::Type type = static_cast< QVariant::Type >( fieldDef.value( QStringLiteral( "type" ) ).toInt() );
    const QString typeName = fieldDef.value( QStringLiteral( "sub_name" ) ).toString();
    const QVariant::Type subType = static_cast< QVariant::Type >( fieldDef.value( QStringLiteral( "sub_type" ) ).toInt() );

    const int length = fieldDef.value( QStringLiteral( "length" ), 0 ).toInt();
    const int precision = fieldDef.value( QStringLiteral( "precision" ), 0 ).toInt();

    mFields.append( QgsField( name, type, typeName, length, precision, QString(), subType ) );

    const QString expressionString = fieldDef.value( QStringLiteral( "expression" ) ).toString();
    if ( !expressionString.isEmpty() )
    {
      QgsExpression expression( expressionString );
      expression.setGeomCalculator( &mDa );
      expression.setDistanceUnits( context.distanceUnit() );
      expression.setAreaUnits( context.areaUnit() );
      if ( expression.hasParserError() )
      {
        throw QgsProcessingException( QObject::tr( "Parser error for field \"%1\" with expression \"%2\": %3" )
                                      .arg(
                                        name,
                                        expressionString,
                                        expression.parserErrorString() ) );
      }
      mExpressions.append( expression );
    }
    else
    {
      mExpressions.append( QgsExpression() );
    }
  }

  return true;
}

QgsFeatureList QgsRefactorFieldsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !mExpressionsPrepared )
  {
    for ( auto it = mExpressions.begin(); it != mExpressions.end(); ++it )
    {
      if ( it->isValid() )
        it->prepare( &mExpressionContext );
    }
  }

  QgsAttributes attributes;
  attributes.reserve( mExpressions.size() );
  for ( auto it = mExpressions.begin(); it != mExpressions.end(); ++it )
  {
    if ( it->isValid() )
    {
      mExpressionContext.setFeature( feature );
      mExpressionContext.lastScope()->setVariable( QStringLiteral( "row_number" ), mRowNumber );
      const QVariant value = it->evaluate( &mExpressionContext );
      if ( it->hasEvalError() )
      {
        throw QgsProcessingException( QObject::tr( "Evaluation error in expression \"%1\": %2" ).arg( it->expression(), it->evalErrorString() ) );
      }
      attributes.append( value );
    }
    else
    {
      attributes.append( QVariant() );
    }
  }

  QgsFeature f = feature;
  f.setAttributes( attributes );
  mRowNumber++;
  return QgsFeatureList() << f;
}

bool QgsRefactorFieldsAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond
