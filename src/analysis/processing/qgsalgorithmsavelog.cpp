/***************************************************************************
                         qgsalgorithmsavelog.cpp
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

#include "qgsalgorithmsavelog.h"

#include <QTextStream>

///@cond PRIVATE

QString QgsSaveLogToFileAlgorithm::name() const
{
  return u"savelog"_s;
}

Qgis::ProcessingAlgorithmFlags QgsSaveLogToFileAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::HideFromToolbox | Qgis::ProcessingAlgorithmFlag::SkipGenericModelLogging;
}

QString QgsSaveLogToFileAlgorithm::displayName() const
{
  return QObject::tr( "Save log to file" );
}

QStringList QgsSaveLogToFileAlgorithm::tags() const
{
  return QObject::tr( "record,messages,logged" ).split( ',' );
}

QString QgsSaveLogToFileAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsSaveLogToFileAlgorithm::groupId() const
{
  return u"modelertools"_s;
}

QString QgsSaveLogToFileAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm saves the model's execution log to a file.\n"
                      "Optionally, the log can be saved in a HTML formatted version." );
}

QString QgsSaveLogToFileAlgorithm::shortDescription() const
{
  return QObject::tr( "Saves the model's log contents to a file." );
}

QgsSaveLogToFileAlgorithm *QgsSaveLogToFileAlgorithm::createInstance() const
{
  return new QgsSaveLogToFileAlgorithm();
}

void QgsSaveLogToFileAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "Log file" ), QObject::tr( "Text files (*.txt);;HTML files (*.html *.HTML)" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"USE_HTML"_s, QObject::tr( "Use HTML formatting" ), false ) );
}

QVariantMap QgsSaveLogToFileAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString file = parameterAsFile( parameters, u"OUTPUT"_s, context );
  const bool useHtml = parameterAsBool( parameters, u"USE_HTML"_s, context );
  if ( !file.isEmpty() )
  {
    QFile exportFile( file );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not save log to file %1" ).arg( file ) );
    }
    QTextStream fout( &exportFile );
    fout << ( useHtml ? feedback->htmlLog() : feedback->textLog() );
  }
  QVariantMap res;
  res.insert( u"OUTPUT"_s, file );
  return res;
}

///@endcond
