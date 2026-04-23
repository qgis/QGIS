/***************************************************************************
                         qgsalgorithmselectbyattribute.h
                         ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmselectbyattribute.h"

#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsSelectByAttributeAlgorithm::name() const
{
  return u"selectbyattribute"_s;
}

QString QgsSelectByAttributeAlgorithm::displayName() const
{
  return QObject::tr( "Select by attribute" );
}

QStringList QgsSelectByAttributeAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,attribute,value,contains,null,field" ).split( ',' );
}

QString QgsSelectByAttributeAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsSelectByAttributeAlgorithm::groupId() const
{
  return u"vectorselection"_s;
}

QString QgsSelectByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm creates a selection in a vector layer.\n\n"
    "The criteria for selected features is defined based on the values of an attribute from the input layer."
  );
}

QString QgsSelectByAttributeAlgorithm::shortDescription() const
{
  return QObject::tr( "Selects features from a vector layer based on an attribute from the layer." );
}

QgsSelectByAttributeAlgorithm *QgsSelectByAttributeAlgorithm::createInstance() const
{
  return new QgsSelectByAttributeAlgorithm();
}

void QgsSelectByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Selection attribute" ), QVariant(), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterEnum(
    u"OPERATOR"_s,
    QObject::tr( "Operator" ),
    QStringList()
      << QObject::tr( "=" )
      << QObject::tr( "≠" )
      << QObject::tr( ">" )
      << QObject::tr( "≥" )
      << QObject::tr( "<" )
      << QObject::tr( "≤" )
      << QObject::tr( "begins with" )
      << QObject::tr( "contains" )
      << QObject::tr( "is null" )
      << QObject::tr( "is not null" )
      << QObject::tr( "does not contain" ),
    false,
    0
  ) );
  addParameter( new QgsProcessingParameterString( u"VALUE"_s, QObject::tr( "Value" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterEnum(
    u"SELECTION_METHOD"_s,
    QObject::tr( "Modify current selection by" ),
    QStringList() << QObject::tr( "creating new selection" ) << QObject::tr( "adding to current selection" ) << QObject::tr( "selecting within current selection" ) << QObject::tr( "removing from current selection" ),
    false,
    0
  ) );

  // backwards compatibility parameters
  // TODO QGIS 5: remove compatibility parameters and their logic
  auto methodParam = std::make_unique<QgsProcessingParameterEnum>(
    u"METHOD"_s,
    QObject::tr( "Modify current selection by" ),
    QStringList() << QObject::tr( "creating new selection" ) << QObject::tr( "adding to current selection" ) << QObject::tr( "removing from current selection" ) << QObject::tr( "selecting within current selection" ),
    false,
    0
  );
  methodParam->setFlags( methodParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( std::move( methodParam ) );

  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Selected (attribute)" ) ) );
}

bool QgsSelectByAttributeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
  {
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );
  }

  const QString fieldName = parameterAsString( parameters, u"FIELD"_s, context );
  const Operation op = static_cast<Operation>( parameterAsEnum( parameters, u"OPERATOR"_s, context ) );
  const QString value = parameterAsString( parameters, u"VALUE"_s, context );
  Qgis::SelectBehavior method = static_cast<Qgis::SelectBehavior>( parameterAsEnum( parameters, u"SELECTION_METHOD"_s, context ) );

  // handle backwards compatibility parameter
  if ( parameters.value( u"METHOD"_s ).isValid() )
  {
    method = static_cast<Qgis::SelectBehavior>( parameterAsEnum( parameters, u"METHOD"_s, context ) );
    if ( method == Qgis::SelectBehavior::IntersectSelection )
    {
      method = Qgis::SelectBehavior::RemoveFromSelection;
    }
    else if ( method == Qgis::SelectBehavior::RemoveFromSelection )
    {
      method = Qgis::SelectBehavior::IntersectSelection;
    }
  }

  mLayerId = layer->id();

  const int idx = layer->fields().lookupField( fieldName );
  if ( idx < 0 )
  {
    throw QgsProcessingException( QObject::tr( "Field '%1' was not found in INPUT layer." ).arg( fieldName ) );
  }

  const QMetaType::Type fieldType = layer->fields().at( idx ).type();
  if ( fieldType != QMetaType::Type::QString && ( op == BeginsWith || op == Contains || op == DoesNotContain ) )
  {
    QString method;
    switch ( op )
    {
      case BeginsWith:
        method = QObject::tr( "begins with" );
        break;
      case Contains:
        method = QObject::tr( "contains" );
        break;
      case DoesNotContain:
        method = QObject::tr( "does not contain" );
        break;

      default:
        break;
    }

    throw QgsProcessingException( QObject::tr( "Operator '%1' can be used only with string fields." ).arg( method ) );
  }

  const QString fieldRef = QgsExpression::quotedColumnRef( fieldName );
  const QString quotedVal = QgsExpression::quotedValue( value );
  QString expr;
  switch ( op )
  {
    case Equals:
      expr = u"%1 = %2"_s.arg( fieldRef, quotedVal );
      break;
    case NotEquals:
      expr = u"%1 <> %2"_s.arg( fieldRef, quotedVal );
      break;
    case GreaterThan:
      expr = u"%1 > %2"_s.arg( fieldRef, quotedVal );
      break;
    case GreaterThanEqualTo:
      expr = u"%1 >= %2"_s.arg( fieldRef, quotedVal );
      break;
    case LessThan:
      expr = u"%1 < %2"_s.arg( fieldRef, quotedVal );
      break;
    case LessThanEqualTo:
      expr = u"%1 <= %2"_s.arg( fieldRef, quotedVal );
      break;
    case BeginsWith:
      expr = u"%1 LIKE '%2%'"_s.arg( fieldRef, value );
      break;
    case Contains:
      expr = u"%1 LIKE '%%2%'"_s.arg( fieldRef, value );
      break;
    case IsNull:
      expr = u"%1 IS NULL"_s.arg( fieldRef );
      break;
    case IsNotNull:
      expr = u"%1 IS NOT NULL"_s.arg( fieldRef );
      break;
    case DoesNotContain:
      expr = u"%1 NOT LIKE '%%2%'"_s.arg( fieldRef, value );
      break;
  }

  QgsExpression expression( expr );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );
  expressionContext.appendScope( layer->createExpressionContextScope() );

  layer->selectByExpression( expression, method, &expressionContext );

  return true;
}

QVariantMap QgsSelectByAttributeAlgorithm::processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap results;
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///@endcond
