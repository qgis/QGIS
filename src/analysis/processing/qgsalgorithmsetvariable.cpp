/***************************************************************************
                         qgsalgorithmsetvariable.cpp
                         ---------------------
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

#include "qgsalgorithmsetvariable.h"
#include "qgsexpressioncontextutils.h"

///@cond PRIVATE

QString QgsSetProjectVariableAlgorithm::name() const
{
  return QStringLiteral( "setprojectvariable" );
}

QgsProcessingAlgorithm::Flags QgsSetProjectVariableAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::FlagHideFromToolbox | QgsProcessingAlgorithm::FlagSkipGenericModelLogging;
}

QString QgsSetProjectVariableAlgorithm::displayName() const
{
  return QObject::tr( "Set project variable" );
}

QStringList QgsSetProjectVariableAlgorithm::tags() const
{
  return QObject::tr( "expression" ).split( ',' );
}

QString QgsSetProjectVariableAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsSetProjectVariableAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsSetProjectVariableAlgorithm::shortDescription() const
{
  return QObject::tr( "Sets an expression variable for the current project" );
}

QString QgsSetProjectVariableAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets an expression variable for the current project." );
}

QgsSetProjectVariableAlgorithm *QgsSetProjectVariableAlgorithm::createInstance() const
{
  return new QgsSetProjectVariableAlgorithm();
}

bool QgsSetProjectVariableAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // this is all nice and quick, we can (and should) do it in the main thread without issue
  const QString name = parameterAsString( parameters, QStringLiteral( "NAME" ), context );
  const QString value = parameterAsString( parameters, QStringLiteral( "VALUE" ), context );

  if ( name.isEmpty() )
    throw QgsProcessingException( QObject::tr( "Variable name cannot be empty" ) );

  QgsExpressionContextUtils::setProjectVariable( context.project(), name, value );
  feedback->pushInfo( QObject::tr( "Set variable \'%1\' to \'%2\'" ).arg( name, value ) );

  return true;
}

void QgsSetProjectVariableAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "NAME" ), QObject::tr( "Variable name" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "VALUE" ), QObject::tr( "Variable value" ) ) );
}

QVariantMap QgsSetProjectVariableAlgorithm::processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  return QVariantMap();
}

///@endcond
