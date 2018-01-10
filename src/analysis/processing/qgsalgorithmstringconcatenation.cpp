/***************************************************************************
                         qgsalgorithmstringconcatenation.cpp
                         ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmstringconcatenation.h"

///@cond PRIVATE

QString QgsStringConcatenationAlgorithm::name() const
{
  return QStringLiteral( "stringconcatenation" );
}

QgsProcessingAlgorithm::Flags QgsStringConcatenationAlgorithm::flags() const
{
  return FlagHideFromToolbox | FlagCanRunInBackground;
}

QString QgsStringConcatenationAlgorithm::displayName() const
{
  return QObject::tr( "String concatenation" );
}

QStringList QgsStringConcatenationAlgorithm::tags() const
{
  return QObject::tr( "string,concatenation,merge" ).split( ',' );
}

QString QgsStringConcatenationAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsStringConcatenationAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsStringConcatenationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm concatenates two strings together." );
}

QgsStringConcatenationAlgorithm *QgsStringConcatenationAlgorithm::createInstance() const
{
  return new QgsStringConcatenationAlgorithm();
}

void QgsStringConcatenationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "INPUT_1" ), QObject::tr( "Input 1" ), QVariant(), false, false ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "INPUT_2" ), QObject::tr( "Input 2" ), QVariant(), false, false ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CONCATENATION" ), QObject::tr( "Concatenation" ) ) );
}

QVariantMap QgsStringConcatenationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QString input_1 = parameterAsString( parameters, QStringLiteral( "INPUT_1" ), context );
  QString input_2 = parameterAsString( parameters, QStringLiteral( "INPUT_2" ), context );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "CONCATENATION" ), input_1 + input_2 );
  return outputs;
}

///@endcond
