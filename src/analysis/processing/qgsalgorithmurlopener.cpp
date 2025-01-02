/***************************************************************************
                         qgsalgorithmurlopener.cpp
                         ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Dave Signer
    email                : david at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmurlopener.h"
#include "moc_qgsalgorithmurlopener.cpp"
#include "qgsprocessingparameters.h"
#include "qgis.h"

#include <QUrl>
#include <QDesktopServices>

///@cond PRIVATE

QString QgsOpenUrlAlgorithm::name() const
{
  return QStringLiteral( "openurl" );
}

QString QgsOpenUrlAlgorithm::displayName() const
{
  return tr( "Open file or URL" );
}

QString QgsOpenUrlAlgorithm::shortDescription() const
{
  return tr( "Opens files in their default associated application, or URLs in the user's default web browser." );
}

QStringList QgsOpenUrlAlgorithm::tags() const
{
  return tr( "open,url,internet,url,fetch,get,request,https,pdf,file" ).split( ',' );
}

QString QgsOpenUrlAlgorithm::group() const
{
  return tr( "File tools" );
}

QString QgsOpenUrlAlgorithm::groupId() const
{
  return QStringLiteral( "filetools" );
}

QString QgsOpenUrlAlgorithm::shortHelpString() const
{
  return tr( "This algorithm opens files in their default associated application, or URLs in the user's default web browser." );
}

QgsOpenUrlAlgorithm *QgsOpenUrlAlgorithm::createInstance() const
{
  return new QgsOpenUrlAlgorithm();
}

void QgsOpenUrlAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "URL" ), tr( "URL or file path" ), QVariant(), false, false ) );
  addOutput( new QgsProcessingOutputBoolean( QStringLiteral( "SUCCESS" ), QObject::tr( "Successfully performed opening file or URL" ) ) );
}

QVariantMap QgsOpenUrlAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString url = parameterAsString( parameters, QStringLiteral( "URL" ), context );
  if ( url.isEmpty() )
    throw QgsProcessingException( tr( "No URL or file path specified" ) );
  const QUrl qurl = QUrl::fromUserInput( url );

  const bool result = QDesktopServices::openUrl( qurl );

  if ( result )
    feedback->pushInfo( QObject::tr( "Successfully opened %1" ).arg( url ) );
  else
    feedback->reportError( QObject::tr( "Failed opening %1" ).arg( url ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "SUCCESS" ), result );
  return outputs;
}

///@endcond
