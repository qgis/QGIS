/***************************************************************************
                         qgsalgorithmcalculateexpression.cpp
                         ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsalgorithmcalculateexpression.h"

///@cond PRIVATE

QString QgsCalculateExpressionAlgorithm::name() const
{
  return QStringLiteral( "calculateexpression" );
}

Qgis::ProcessingAlgorithmFlags QgsCalculateExpressionAlgorithm::flags() const
{
  return Qgis::ProcessingAlgorithmFlag::HideFromToolbox | Qgis::ProcessingAlgorithmFlag::SkipGenericModelLogging;
}

QString QgsCalculateExpressionAlgorithm::displayName() const
{
  return QObject::tr( "Calculate expression" );
}

QStringList QgsCalculateExpressionAlgorithm::tags() const
{
  return QObject::tr( "evaluate,variable,store" ).split( ',' );
}

QString QgsCalculateExpressionAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsCalculateExpressionAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsCalculateExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the result of a QGIS expression and makes it available for use in other parts of the model." );
}

QgsCalculateExpressionAlgorithm *QgsCalculateExpressionAlgorithm::createInstance() const
{
  return new QgsCalculateExpressionAlgorithm();
}

void QgsCalculateExpressionAlgorithm::initAlgorithm( const QVariantMap & )
{
  // possibly this should be a new dedicated parameter type for "QgsProcessingParameterVariant", as the values specified for the parameter will
  // be whatever the model calculates as the result of the expression. But this works for now...
  std::unique_ptr<QgsProcessingParameterString> inputParameter = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "INPUT" ), QObject::tr( "Input" ), QVariant(), false, false );
  // we limit the available sources for this parameter to just precalculated expressions -- otherwise it's confusing if we allow users
  // to enter a literal value for this parameter, as they could enter an expression in there and expect it to be evaluated.
  inputParameter->setMetadata(
    { QVariantMap( { { QStringLiteral( "model_widget" ), QVariantMap( { { QStringLiteral( "accepted_sources" ), QVariantList { static_cast<int>( Qgis::ProcessingModelChildParameterSource::Expression ) } } } ) } } )
    }
  );
  addParameter( inputParameter.release() );

  addOutput( new QgsProcessingOutputVariant( QStringLiteral( "OUTPUT" ), QObject::tr( "Value" ) ) );
}

QVariantMap QgsCalculateExpressionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &, QgsProcessingFeedback * )
{
  const QVariant res = parameters.value( QStringLiteral( "INPUT" ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), res );
  return outputs;
}

///@endcond
