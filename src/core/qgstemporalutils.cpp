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
#include "qgsmapdecoration.h"
#include "qgsmapsettings.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsexpressioncontextutils.h"

#include <QRegularExpression>

QgsDateTimeRange QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject *project )
{
  const QMap<QString, QgsMapLayer *> mapLayers = project->mapLayers();
  QDateTime minDate;
  QDateTime maxDate;

  for ( auto it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    QgsMapLayer *currentLayer = it.value();

    if ( !currentLayer->temporalProperties() || !currentLayer->temporalProperties()->isActive() )
      continue;
    const QgsDateTimeRange layerRange = currentLayer->temporalProperties()->calculateTemporalExtent( currentLayer );

    if ( layerRange.begin().isValid() && ( !minDate.isValid() ||  layerRange.begin() < minDate ) )
      minDate = layerRange.begin();
    if ( layerRange.end().isValid() && ( !maxDate.isValid() ||  layerRange.end() > maxDate ) )
      maxDate = layerRange.end();
  }

  return QgsDateTimeRange( minDate, maxDate );
}

QList< QgsDateTimeRange > QgsTemporalUtils::usedTemporalRangesForProject( QgsProject *project )
{
  const QMap<QString, QgsMapLayer *> mapLayers = project->mapLayers();

  QList< QgsDateTimeRange > ranges;
  for ( auto it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    QgsMapLayer *currentLayer = it.value();

    if ( !currentLayer->temporalProperties() || !currentLayer->temporalProperties()->isActive() )
      continue;

    ranges.append( currentLayer->temporalProperties()->allTemporalRanges( currentLayer ) );
  }

  return QgsDateTimeRange::mergeRanges( ranges );
}

bool QgsTemporalUtils::exportAnimation( const QgsMapSettings &mapSettings, const QgsTemporalUtils::AnimationExportSettings &settings, QString &error, QgsFeedback *feedback )
{
  if ( settings.fileNameTemplate.isEmpty() )
  {
    error = QObject::tr( "Filename template is empty" );
    return false;
  }
  const int numberOfDigits = settings.fileNameTemplate.count( QLatin1Char( '#' ) );
  if ( numberOfDigits < 0 )
  {
    error = QObject::tr( "Wrong filename template format (must contain #)" );
    return false;
  }
  const QString token( numberOfDigits, QLatin1Char( '#' ) );
  if ( !settings.fileNameTemplate.contains( token ) )
  {
    error = QObject::tr( "Filename template must contain all # placeholders in one continuous group." );
    return false;
  }
  if ( !QDir().mkpath( settings.outputDirectory ) )
  {
    error = QObject::tr( "Output directory creation failure." );
    return false;
  }

  QgsTemporalNavigationObject navigator;
  navigator.setTemporalExtents( settings.animationRange );
  navigator.setFrameDuration( settings.frameDuration );
  QgsMapSettings ms = mapSettings;
  const QgsExpressionContext context = ms.expressionContext();
  ms.setFrameRate( settings.frameRate );

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

    navigator.setCurrentFrameNumber( currentFrame );

    ms.setIsTemporal( true );
    ms.setTemporalRange( navigator.dateTimeRangeForFrameNumber( currentFrame ) );
    ms.setCurrentFrame( currentFrame );

    QgsExpressionContext frameContext = context;
    frameContext.appendScope( navigator.createExpressionContextScope() );
    frameContext.appendScope( QgsExpressionContextUtils::mapSettingsScope( ms ) );
    ms.setExpressionContext( frameContext );

    QString fileName( settings.fileNameTemplate );
    const QString frameNoPaddedLeft( QStringLiteral( "%1" ).arg( currentFrame, numberOfDigits, 10, QChar( '0' ) ) ); // e.g. 0001
    fileName.replace( token, frameNoPaddedLeft );
    const QString path = QDir( settings.outputDirectory ).filePath( fileName );

    QImage img = QImage( ms.outputSize(), ms.outputImageFormat() );
    img.setDotsPerMeterX( 1000 * ms.outputDpi() / 25.4 );
    img.setDotsPerMeterY( 1000 * ms.outputDpi() / 25.4 );
    img.fill( ms.backgroundColor().rgb() );

    QPainter p( &img );
    QgsMapRendererCustomPainterJob job( ms, &p );
    job.start();
    job.waitForFinished();

    QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
    context.setPainter( &p );

    const auto constMDecorations = settings.decorations;
    for ( QgsMapDecoration *decoration : constMDecorations )
    {
      decoration->render( ms, context );
    }

    p.end();

    img.save( path );

    ++currentFrame;
  }

  return true;
}


QDateTime QgsTemporalUtils::calculateFrameTime( const QDateTime &start, const long long frame, const QgsInterval &interval )
{

  double unused;
  const bool isFractional = !qgsDoubleNear( fabs( modf( interval.originalDuration(), &unused ) ), 0.0 );

  if ( isFractional || interval.originalUnit() == QgsUnitTypes::TemporalUnit::TemporalUnknownUnit )
  {
    const double duration = interval.seconds();
    return start.addMSecs( frame * duration * 1000 );
  }
  else
  {
    switch ( interval.originalUnit() )
    {
      case QgsUnitTypes::TemporalUnit::TemporalMilliseconds:
        return start.addMSecs( frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalSeconds:
        return start.addSecs( frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalMinutes:
        return start.addSecs( 60 * frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalHours:
        return start.addSecs( 3600 * frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalDays:
        return start.addDays( frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalWeeks:
        return start.addDays( 7 * frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalMonths:
        return start.addMonths( frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalYears:
        return start.addYears( frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalDecades:
        return start.addYears( 10 * frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalCenturies:
        return start.addYears( 100 * frame * interval.originalDuration() );
      case QgsUnitTypes::TemporalUnit::TemporalUnknownUnit:
        // handled above
        return QDateTime();
      case QgsUnitTypes::TemporalUnit::TemporalIrregularStep:
        // not supported by this method
        return QDateTime();
    }
  }
  return QDateTime();
}

QList<QDateTime> QgsTemporalUtils::calculateDateTimesUsingDuration( const QDateTime &start, const QDateTime &end, const QString &duration, bool &ok, bool &maxValuesExceeded, int maxValues )
{
  ok = false;
  const QgsTimeDuration timeDuration( QgsTimeDuration::fromString( duration, ok ) );
  if ( !ok )
    return {};

  if ( timeDuration.years == 0 && timeDuration.months == 0 && timeDuration.weeks == 0 && timeDuration.days == 0
       && timeDuration.hours == 0 && timeDuration.minutes == 0 && timeDuration.seconds == 0 )
  {
    ok = false;
    return {};
  }
  return calculateDateTimesUsingDuration( start, end, timeDuration, maxValuesExceeded, maxValues );
}

QList<QDateTime> QgsTemporalUtils::calculateDateTimesUsingDuration( const QDateTime &start, const QDateTime &end, const QgsTimeDuration &timeDuration, bool &maxValuesExceeded, int maxValues )
{
  QList<QDateTime> res;
  QDateTime current = start;
  maxValuesExceeded = false;
  while ( current <= end )
  {
    res << current;

    if ( maxValues >= 0 && res.size() > maxValues )
    {
      maxValuesExceeded = true;
      break;
    }

    if ( timeDuration.years )
      current = current.addYears( timeDuration.years );
    if ( timeDuration.months )
      current = current.addMonths( timeDuration.months );
    if ( timeDuration.weeks || timeDuration.days )
      current = current.addDays( timeDuration.weeks * 7 + timeDuration.days );
    if ( timeDuration.hours || timeDuration.minutes || timeDuration.seconds )
      current = current.addSecs( timeDuration.hours * 60LL * 60 + timeDuration.minutes * 60 + timeDuration.seconds );
  }
  return res;
}

QList<QDateTime> QgsTemporalUtils::calculateDateTimesFromISO8601( const QString &string, bool &ok, bool &maxValuesExceeded, int maxValues )
{
  ok = false;
  maxValuesExceeded = false;
  const QStringList parts = string.split( '/' );
  if ( parts.length() != 3 )
  {
    return {};
  }

  const QDateTime start = QDateTime::fromString( parts.at( 0 ), Qt::ISODate );
  if ( !start.isValid() )
    return {};
  const QDateTime end = QDateTime::fromString( parts.at( 1 ), Qt::ISODate );
  if ( !end.isValid() )
    return {};

  return calculateDateTimesUsingDuration( start, end, parts.at( 2 ), ok, maxValuesExceeded, maxValues );
}

//
// QgsTimeDuration
//

QgsInterval QgsTimeDuration::toInterval() const
{
  return QgsInterval( years, months, weeks, days, hours, minutes, seconds );
}

QString QgsTimeDuration::toString() const
{
  QString text( "P" );

  if ( years )
  {
    text.append( QString::number( years ) );
    text.append( 'Y' );
  }
  if ( months )
  {
    text.append( QString::number( months ) );
    text.append( 'M' );
  }
  if ( days )
  {
    text.append( QString::number( days ) );
    text.append( 'D' );
  }

  if ( hours )
  {
    if ( !text.contains( 'T' ) )
      text.append( 'T' );
    text.append( QString::number( hours ) );
    text.append( 'H' );
  }
  if ( minutes )
  {
    if ( !text.contains( 'T' ) )
      text.append( 'T' );
    text.append( QString::number( minutes ) );
    text.append( 'M' );
  }
  if ( seconds )
  {
    if ( !text.contains( 'T' ) )
      text.append( 'T' );
    text.append( QString::number( seconds ) );
    text.append( 'S' );
  }
  return text;
}

long long QgsTimeDuration::toSeconds() const
{
  long long secs = 0.0;

  if ( years )
    secs += years * QgsInterval::YEARS;
  if ( months )
    secs += months * QgsInterval::MONTHS;
  if ( days )
    secs += days * QgsInterval::DAY;
  if ( hours )
    secs += hours * QgsInterval::HOUR;
  if ( minutes )
    secs += minutes * QgsInterval::MINUTE;
  if ( seconds )
    secs += seconds;

  return secs;
}

QDateTime QgsTimeDuration::addToDateTime( const QDateTime &dateTime )
{
  QDateTime resultDateTime = dateTime;

  if ( years )
    resultDateTime = resultDateTime.addYears( years );
  if ( months )
    resultDateTime = resultDateTime.addMonths( months );
  if ( weeks || days )
    resultDateTime = resultDateTime.addDays( weeks * 7 + days );
  if ( hours || minutes || seconds )
    resultDateTime = resultDateTime.addSecs( hours * 60LL * 60 + minutes * 60 + seconds );

  return resultDateTime;
}

QgsTimeDuration QgsTimeDuration::fromString( const QString &string, bool &ok )
{
  ok = false;
  thread_local const QRegularExpression sRx( QStringLiteral( R"(P(?:([\d]+)Y)?(?:([\d]+)M)?(?:([\d]+)W)?(?:([\d]+)D)?(?:T(?:([\d]+)H)?(?:([\d]+)M)?(?:([\d\.]+)S)?)?$)" ) );

  const QRegularExpressionMatch match = sRx.match( string );
  QgsTimeDuration duration;
  if ( match.hasMatch() )
  {
    ok = true;
    duration.years = match.captured( 1 ).toInt();
    duration.months = match.captured( 2 ).toInt();
    duration.weeks = match.captured( 3 ).toInt();
    duration.days = match.captured( 4 ).toInt();
    duration.hours = match.captured( 5 ).toInt();
    duration.minutes = match.captured( 6 ).toInt();
    duration.seconds = match.captured( 7 ).toDouble();
  }
  return duration;
}
