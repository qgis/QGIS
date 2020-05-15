/***************************************************************************
  qgstemporalutils.cpp
  -----------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporalutils.h"
#include "qgsproject.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgstemporalnavigationobject.h"
#include "qgsmapsettings.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsexpressioncontextutils.h"

QgsDateTimeRange QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject *project )
{
  const QMap<QString, QgsMapLayer *> &mapLayers = project->mapLayers();
  QgsMapLayer *currentLayer = nullptr;

  QDateTime minDate;
  QDateTime maxDate;

  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    currentLayer = it.value();

    if ( !currentLayer->temporalProperties() || !currentLayer->temporalProperties()->isActive() )
      continue;
    QgsDateTimeRange layerRange = currentLayer->temporalProperties()->calculateTemporalExtent( currentLayer );

    if ( layerRange.begin().isValid() && ( !minDate.isValid() ||  layerRange.begin() < minDate ) )
      minDate = layerRange.begin();
    if ( layerRange.end().isValid() && ( !maxDate.isValid() ||  layerRange.end() > maxDate ) )
      maxDate = layerRange.end();
  }

  return QgsDateTimeRange( minDate, maxDate );
}

bool QgsTemporalUtils::exportAnimation( const QgsMapSettings &mapSettings, const QgsDateTimeRange &animationRange, QgsInterval frameDuration, const QString &outputDirectory, const QString &fileNameTemplate, QString &error, QgsFeedback *feedback )
{
  if ( fileNameTemplate.isEmpty() )
  {
    error = QObject::tr( "Filename template is empty" );
    return false;
  }
  int numberOfDigits = fileNameTemplate.count( QLatin1Char( '#' ) );
  if ( numberOfDigits < 0 )
  {
    error = QObject::tr( "Wrong filename template format (must contain #)" );
    return false;
  }
  const QString token( numberOfDigits, QLatin1Char( '#' ) );
  if ( !fileNameTemplate.contains( token ) )
  {
    error = QObject::tr( "Filename template must contain all # placeholders in one continuous group." );
    return false;
  }

  QgsTemporalNavigationObject navigator;
  navigator.setTemporalExtents( animationRange );
  navigator.setFrameDuration( frameDuration );
  QgsMapSettings settings = mapSettings;
  const QgsExpressionContext context = settings.expressionContext();

  const long long totalFrames = navigator.totalFrameCount();
  long long currentFrame = 0;

  while ( currentFrame < totalFrames )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
      {
        error = QObject::tr( "Export canceled" );
        return false;
      }
      feedback->setProgress( currentFrame / static_cast<double>( totalFrames ) * 100 );
    }
    ++currentFrame;

    navigator.setCurrentFrameNumber( currentFrame );

    settings.setIsTemporal( true );
    settings.setTemporalRange( navigator.dateTimeRangeForFrameNumber( currentFrame ) );

    QgsExpressionContext frameContext = context;
    frameContext.appendScope( navigator.createExpressionContextScope() );
    frameContext.appendScope( QgsExpressionContextUtils::mapSettingsScope( settings ) );
    settings.setExpressionContext( frameContext );

    QString fileName( fileNameTemplate );
    const QString frameNoPaddedLeft( QStringLiteral( "%1" ).arg( currentFrame, numberOfDigits, 10, QChar( '0' ) ) ); // e.g. 0001
    fileName.replace( token, frameNoPaddedLeft );
    const QString path = QDir( outputDirectory ).filePath( fileName );

    QImage img = QImage( settings.outputSize(), settings.outputImageFormat() );
    img.setDotsPerMeterX( 1000 * settings.outputDpi() / 25.4 );
    img.setDotsPerMeterY( 1000 * settings.outputDpi() / 25.4 );
    img.fill( settings.backgroundColor().rgb() );

    QPainter p( &img );
    QgsMapRendererCustomPainterJob job( settings, &p );
    job.start();
    job.waitForFinished();
    p.end();

    img.save( path );
  }

  return true;
}
