/***************************************************************************
                         qgsalgorithmraiseexception.cpp
                         ---------------------
    begin                : February 2020
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

#include "qgsalgorithmraiseexception.h"

///@cond PRIVATE

//
// QgsRaiseExceptionAlgorithm
//

QString QgsRaiseExceptionAlgorithm::name() const
{
  return QStringLiteral( "raiseexception" );
}

QgsProcessingAlgorithm::Flags QgsRaiseExceptionAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | FlagHideFromToolbox | FlagCustomException | FlagSkipGenericModelLogging;
}

QString QgsRaiseExceptionAlgorithm::displayName() const
{
  return QObject::tr( "Raise exception" );
}

QStringList QgsRaiseExceptionAlgorithm::tags() const
{
  return QObject::tr( "abort,warn,error,cancel" ).split( ',' );
}

QString QgsRaiseExceptionAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsRaiseExceptionAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsRaiseExceptionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm raises an exception and cancels a model's execution.\n\n"
                      "The exception message can be customized, and optionally an expression based condition "
                      "can be specified. If an expression condition is used, then the exception will only "
                      "be raised if the expression result is true. A false result indicates that no exception "
                      "will be raised, and the model execution can continue uninterrupted." );
}

QString QgsRaiseExceptionAlgorithm::shortDescription() const
{
  return QObject::tr( "Raises an exception and cancels a model's execution." );
}

QgsRaiseExceptionAlgorithm *QgsRaiseExceptionAlgorithm::createInstance() const
{
  return new QgsRaiseExceptionAlgorithm();
}

void QgsRaiseExceptionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "MESSAGE" ), QObject::tr( "Error message" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "CONDITION" ), QObject::tr( "Condition" ), QVariant(), QString(), true ) );
}

QVariantMap QgsRaiseExceptionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QString expression = parameterAsExpression( parameters, QStringLiteral( "CONDITION" ), context );
  if ( !expression.isEmpty() )
  {
    const QgsExpressionContext expContext = createExpressionContext( parameters, context );
    QgsExpression exp( expression );
    if ( exp.hasParserError() )
    {
      throw QgsProcessingException( QObject::tr( "Error parsing condition expression: %1" ).arg( exp.parserErrorString() ) );
    }
    if ( !exp.evaluate( &expContext ).toBool() )
      return QVariantMap();
  }

  const QString error = parameterAsString( parameters, QStringLiteral( "MESSAGE" ), context );
  throw QgsProcessingException( error );
}


//
// QgsRaiseWarningAlgorithm
//

QString QgsRaiseWarningAlgorithm::name() const
{
  return QStringLiteral( "raisewarning" );
}

QgsProcessingAlgorithm::Flags QgsRaiseWarningAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | FlagHideFromToolbox | FlagSkipGenericModelLogging;
}

QString QgsRaiseWarningAlgorithm::displayName() const
{
  return QObject::tr( "Raise warning" );
}

QStringList QgsRaiseWarningAlgorithm::tags() const
{
  return QObject::tr( "abort,warn,error,cancel" ).split( ',' );
}

QString QgsRaiseWarningAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsRaiseWarningAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsRaiseWarningAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm raises a warning message in the log.\n\n"
                      "The warning message can be customized, and optionally an expression based condition "
                      "can be specified. If an expression condition is used, then the warning will only "
                      "be logged if the expression result is true. A false result indicates that no warning "
                      "will be logged." );
}

QString QgsRaiseWarningAlgorithm::shortDescription() const
{
  return QObject::tr( "Raises an warning message." );
}

QgsRaiseWarningAlgorithm *QgsRaiseWarningAlgorithm::createInstance() const
{
  return new QgsRaiseWarningAlgorithm();
}

void QgsRaiseWarningAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "MESSAGE" ), QObject::tr( "Warning message" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "CONDITION" ), QObject::tr( "Condition" ), QVariant(), QString(), true ) );
}

QVariantMap QgsRaiseWarningAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString expression = parameterAsExpression( parameters, QStringLiteral( "CONDITION" ), context );
  if ( !expression.isEmpty() )
  {
    const QgsExpressionContext expContext = createExpressionContext( parameters, context );
    QgsExpression exp( expression );
    if ( exp.hasParserError() )
    {
      throw QgsProcessingException( QObject::tr( "Error parsing condition expression: %1" ).arg( exp.parserErrorString() ) );
    }
    if ( !exp.evaluate( &expContext ).toBool() )
      return QVariantMap();
  }

  const QString warning = parameterAsString( parameters, QStringLiteral( "MESSAGE" ), context );
  feedback->pushWarning( warning );
  return QVariantMap();
}


//
// QgsRaiseMessageAlgorithm
//

QString QgsRaiseMessageAlgorithm::name() const
{
  return QStringLiteral( "raisemessage" );
}

QgsProcessingAlgorithm::Flags QgsRaiseMessageAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | FlagHideFromToolbox | FlagSkipGenericModelLogging;
}

QString QgsRaiseMessageAlgorithm::displayName() const
{
  return QObject::tr( "Raise message" );
}

QStringList QgsRaiseMessageAlgorithm::tags() const
{
  return QObject::tr( "information" ).split( ',' );
}

QString QgsRaiseMessageAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsRaiseMessageAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsRaiseMessageAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm raises an information message in the log.\n\n"
                      "The message can be customized, and optionally an expression based condition "
                      "can be specified. If an expression condition is used, then the message will only "
                      "be logged if the expression result is true. A false result indicates that no message "
                      "will be logged." );
}

QString QgsRaiseMessageAlgorithm::shortDescription() const
{
  return QObject::tr( "Raises an information message." );
}

QgsRaiseMessageAlgorithm *QgsRaiseMessageAlgorithm::createInstance() const
{
  return new QgsRaiseMessageAlgorithm();
}

void QgsRaiseMessageAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "MESSAGE" ), QObject::tr( "Information message" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "CONDITION" ), QObject::tr( "Condition" ), QVariant(), QString(), true ) );
}

QVariantMap QgsRaiseMessageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString expression = parameterAsExpression( parameters, QStringLiteral( "CONDITION" ), context );
  if ( !expression.isEmpty() )
  {
    const QgsExpressionContext expContext = createExpressionContext( parameters, context );
    QgsExpression exp( expression );
    if ( exp.hasParserError() )
    {
      throw QgsProcessingException( QObject::tr( "Error parsing condition expression: %1" ).arg( exp.parserErrorString() ) );
    }
    if ( !exp.evaluate( &expContext ).toBool() )
      return QVariantMap();
  }

  const QString info = parameterAsString( parameters, QStringLiteral( "MESSAGE" ), context );
  feedback->pushInfo( info );
  return QVariantMap();
}

///@endcond
