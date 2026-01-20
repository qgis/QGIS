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
  return u"stringconcatenation"_s;
}

Qgis::ProcessingAlgorithmFlags QgsStringConcatenationAlgorithm::flags() const
{
  return Qgis::ProcessingAlgorithmFlag::HideFromToolbox | Qgis::ProcessingAlgorithmFlag::SkipGenericModelLogging;
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
  return u"modelertools"_s;
}

QString QgsStringConcatenationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm concatenates two strings together." );
}

QString QgsStringConcatenationAlgorithm::shortDescription() const
{
  return QObject::tr( "Concatenates two strings together." );
}

QgsStringConcatenationAlgorithm *QgsStringConcatenationAlgorithm::createInstance() const
{
  return new QgsStringConcatenationAlgorithm();
}

void QgsStringConcatenationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( u"INPUT_1"_s, QObject::tr( "Input 1" ), QVariant(), false, false ) );
  addParameter( new QgsProcessingParameterString( u"INPUT_2"_s, QObject::tr( "Input 2" ), QVariant(), false, false ) );
  addOutput( new QgsProcessingOutputString( u"CONCATENATION"_s, QObject::tr( "Concatenation" ) ) );
}

QVariantMap QgsStringConcatenationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QString input_1 = parameterAsString( parameters, u"INPUT_1"_s, context );
  const QString input_2 = parameterAsString( parameters, u"INPUT_2"_s, context );

  QVariantMap outputs;
  outputs.insert( u"CONCATENATION"_s, QString( input_1 + input_2 ) );
  return outputs;
}

///@endcond
