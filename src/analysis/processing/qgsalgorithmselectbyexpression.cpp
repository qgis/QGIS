/***************************************************************************
                         qgsalgorithmselectbyexpression.h
                         ---------------------
    begin                : MArch 2026
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

#include "qgsalgorithmselectbyexpression.h"

#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsSelectByExpressionAlgorithm::name() const
{
  return u"selectbyexpression"_s;
}

QString QgsSelectByExpressionAlgorithm::displayName() const
{
  return QObject::tr( "Select by expression" );
}

QStringList QgsSelectByExpressionAlgorithm::tags() const
{
  return QObject::tr( "select,filter,expression,field" ).split( ',' );
}

QString QgsSelectByExpressionAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsSelectByExpressionAlgorithm::groupId() const
{
  return u"vectorselection"_s;
}

QString QgsSelectByExpressionAlgorithm::shortDescription() const
{
  return QObject::tr( "Selects features from a vector layer based on a QGIS expression." );
}

QString QgsSelectByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm creates a selection in a vector layer. The criteria for selecting "
    "features is based on a QGIS expression.\n\n"
    "For help with QGIS expression functions, see the inbuilt help for specific "
    "functions which is available in the expression builder."
  );
}

Qgis::ProcessingAlgorithmFlags QgsSelectByExpressionAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool;
}

QgsSelectByExpressionAlgorithm *QgsSelectByExpressionAlgorithm::createInstance() const
{
  return new QgsSelectByExpressionAlgorithm();
}

void QgsSelectByExpressionAlgorithm::initAlgorithm( const QVariantMap & )
{
  QStringList methods = QStringList()
                        << QObject::tr( "creating new selection" )
                        << QObject::tr( "adding to current selection" )
                        << QObject::tr( "selecting within current selection" )
                        << QObject::tr( "removing from current selection" );

  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterExpression( u"EXPRESSION"_s, QObject::tr( "Expression" ), QVariant(), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterEnum( u"SELECTION_METHOD"_s, QObject::tr( "Modify current selection by" ), methods, false, 0 ) );

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

bool QgsSelectByExpressionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
  {
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );
  }

  const QString expressionString = parameterAsExpression( parameters, u"EXPRESSION"_s, context );
  QgsExpression expression( expressionString );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }

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

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );
  expressionContext.appendScope( layer->createExpressionContextScope() );

  layer->selectByExpression( expression, method, &expressionContext );

  return true;
}

QVariantMap QgsSelectByExpressionAlgorithm::processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap results;
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///@endcond
