/***************************************************************************
                         qgsalgorithmconditionalbranch.cpp
                         ---------------------
    begin                : March 2020
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

#include "qgsalgorithmconditionalbranch.h"
#include "qgsapplication.h"

///@cond PRIVATE

QString QgsConditionalBranchAlgorithm::name() const
{
  return QStringLiteral( "condition" );
}

QString QgsConditionalBranchAlgorithm::displayName() const
{
  return QObject::tr( "Conditional branch" );
}

QStringList QgsConditionalBranchAlgorithm::tags() const
{
  return QObject::tr( "if,logic,test" ).split( ',' );
}

QString QgsConditionalBranchAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsConditionalBranchAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

Qgis::ProcessingAlgorithmFlags QgsConditionalBranchAlgorithm::flags() const
{
  return Qgis::ProcessingAlgorithmFlag::HideFromToolbox | Qgis::ProcessingAlgorithmFlag::SkipGenericModelLogging;
}

QString QgsConditionalBranchAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm adds a conditional branch into a model, allowing parts of the model to be executed based on the result of an expression evaluation." );
}

QString QgsConditionalBranchAlgorithm::shortDescription() const
{
  return QObject::tr( "Adds a conditional branch into a model, allowing parts of the model to be selectively executed." );
}

QgsConditionalBranchAlgorithm *QgsConditionalBranchAlgorithm::createInstance() const
{
  return new QgsConditionalBranchAlgorithm();
}

QgsConditionalBranchAlgorithm::~QgsConditionalBranchAlgorithm()
{
  qDeleteAll( mOutputs );
}

void QgsConditionalBranchAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  const QVariantList branches = configuration.value( QStringLiteral( "conditions" ) ).toList();
  for ( const QVariant &branch : branches )
  {
    const QVariantMap branchDef = branch.toMap();
    const QString name = branchDef.value( QStringLiteral( "name" ) ).toString();
    mOutputs.append( new Output( name, branchDef.value( QStringLiteral( "expression" ) ).toString() ) );
    addOutput( new QgsProcessingOutputConditionalBranch( name, name ) );
  }
}

QVariantMap QgsConditionalBranchAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );

  QVariantMap results;
  for ( Output *output : std::as_const( mOutputs ) )
  {
    output->expression.prepare( &expressionContext );
    const QVariant res = output->expression.evaluate( &expressionContext );
    results.insert( output->name, res );
    if ( res.toBool() )
    {
      feedback->pushInfo( QObject::tr( "Condition %1 passed" ).arg( output->name ) );
    }
    else
    {
      feedback->pushInfo( QObject::tr( "Condition %1 failed" ).arg( output->name ) );
    }
  }
  qDeleteAll( mOutputs );
  mOutputs.clear();
  return results;
}


///@endcond PRIVATE
