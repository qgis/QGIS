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

#include <cstdlib>
#include <optional>

#include "qgsexpressioncontextutils.h"
#include "qgsmapdecoration.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmapsettings.h"
#include "qgsproject.h"
#include "qgstemporalnavigationobject.h"

#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

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

    if ( layerRange.begin().isValid() && ( !minDate.isValid() || layerRange.begin() < minDate ) )
      minDate = layerRange.begin();
    if ( layerRange.end().isValid() && ( !maxDate.isValid() || layerRange.end() > maxDate ) )
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
  const int numberOfDigits = settings.fileNameTemplate.count( '#'_L1 );
  if ( numberOfDigits < 0 )
  {
    error = QObject::tr( "Wrong filename template format (must contain #)" );
    return false;
  }
  const QString token( numberOfDigits, '#'_L1 );
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
  navigator.setNavigationMode( settings.animationMode );
  navigator.setTotalMovieFrames( settings.totalMovieFrames );
  if ( settings.frameDuration.originalUnit() == Qgis::TemporalUnit::IrregularStep )
  {
    navigator.setAvailableTemporalRanges( settings.availableTemporalRanges );
  }
  navigator.setTemporalRangeCumulative( settings.temporalRangeCumulative );

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
    const QString frameNoPaddedLeft( u"%1"_s.arg( currentFrame, numberOfDigits, 10, QChar( '0' ) ) ); // e.g. 0001
    fileName.replace( token, frameNoPaddedLeft );
    const QString path = QDir( settings.outputDirectory ).filePath( fileName );

    QImage img = QImage( ms.outputSize() * ms.devicePixelRatio(), ms.outputImageFormat() );
    img.setDevicePixelRatio( ms.devicePixelRatio() );
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

  if ( isFractional || interval.originalUnit() == Qgis::TemporalUnit::Unknown )
  {
    const double duration = interval.seconds();
    return start.addMSecs( frame * duration * 1000 );
  }
  else
  {
    switch ( interval.originalUnit() )
    {
      case Qgis::TemporalUnit::Milliseconds:
        return start.addMSecs( frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Seconds:
        return start.addSecs( frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Minutes:
        return start.addSecs( 60 * frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Hours:
        return start.addSecs( 3600 * frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Days:
        return start.addDays( frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Weeks:
        return start.addDays( 7 * frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Months:
        return start.addMonths( frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Years:
        return start.addYears( frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Decades:
        return start.addYears( 10 * frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Centuries:
        return start.addYears( 100 * frame * interval.originalDuration() );
      case Qgis::TemporalUnit::Unknown:
        // handled above
        return QDateTime();
      case Qgis::TemporalUnit::IrregularStep:
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

  if ( timeDuration.years == 0 && timeDuration.months == 0 && timeDuration.weeks == 0 && timeDuration.days == 0 && timeDuration.hours == 0 && timeDuration.minutes == 0 && timeDuration.seconds == 0 )
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
    secs += years * static_cast< long long >( QgsInterval::YEARS );
  if ( months )
    secs += months * static_cast< long long >( QgsInterval::MONTHS );
  if ( days )
    secs += days * static_cast< long long >( QgsInterval::DAY );
  if ( hours )
    secs += hours * static_cast< long long >( QgsInterval::HOUR );
  if ( minutes )
    secs += minutes * static_cast< long long >( QgsInterval::MINUTE );
  if ( seconds )
    secs += seconds;

  return secs;
}

QDateTime QgsTimeDuration::addToDateTime( const QDateTime &dateTime ) const
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

QStringList QgsNetCdfGribTimeParser::cfSplit( const QString &str, QChar delimiter )
{
  return str.split( delimiter, Qt::SkipEmptyParts );
}

int QgsNetCdfGribTimeParser::cfToInt( const QString &str )
{
  return std::atoi( str.toStdString().c_str() );
}

double QgsNetCdfGribTimeParser::cfToDouble( const QString &str )
{
  return std::atof( str.toStdString().c_str() );
}

QgsNetCdfGribTimeParser::CFTimeUnit QgsNetCdfGribTimeParser::cfParseDurationTimeUnit( const QString &timeUnit )
{
  CFTimeUnit unit = CFTimeUnit::Hours; //default unit

  if ( timeUnit == "millisec"_L1 || timeUnit == "msec"_L1 || timeUnit == "millisecs"_L1 || timeUnit == "msecs"_L1 )
  {
    unit = CFTimeUnit::Milliseconds;
  }
  else if ( timeUnit == "second"_L1
            || timeUnit == "seconds"_L1
            || timeUnit == "Seconds"_L1
            || timeUnit == "sec"_L1
            || timeUnit == "secs"_L1
            || timeUnit == "s"_L1
            || timeUnit == "se"_L1
            ||                   // ascii_dat format
            timeUnit == "2"_L1 ) // ascii_dat format
  {
    unit = CFTimeUnit::Seconds;
  }
  else if ( timeUnit == "minute"_L1
            || timeUnit == "minutes"_L1
            || timeUnit == "Minutes"_L1
            || timeUnit == "min"_L1
            || timeUnit == "mins"_L1
            || timeUnit == "mi"_L1
            ||                   // ascii_dat format
            timeUnit == "1"_L1 ) // ascii_dat format
  {
    unit = CFTimeUnit::Minutes;
  }
  else if ( timeUnit == "day"_L1 || timeUnit == "days"_L1 || timeUnit == "Days"_L1 )
  {
    unit = CFTimeUnit::Days;
  }
  else if ( timeUnit == "week"_L1 || timeUnit == "weeks"_L1 )
  {
    unit = CFTimeUnit::Weeks;
  }

  return unit;
}

QgsNetCdfGribTimeParser::CFTimeUnit QgsNetCdfGribTimeParser::cfParseTimeUnit( const QString &timeInformation )
{
  const QStringList strings = cfSplit( timeInformation, ' ' );
  if ( strings.size() < 3 )
    return CFTimeUnit::Hours; //default value

  if ( strings[1] == "since"_L1 )
  {
    const QString &timeUnit = strings[0];
    if ( timeUnit == "month"_L1 || timeUnit == "months"_L1 || timeUnit == "mon"_L1 || timeUnit == "mons"_L1 )
    {
      return CFTimeUnit::MonthsCF;
    }
    else if ( timeUnit == "year"_L1 || timeUnit == "years"_L1 || timeUnit == "yr"_L1 || timeUnit == "yrs"_L1 )
    {
      return CFTimeUnit::ExactYears;
    }

    return cfParseDurationTimeUnit( strings[0] );
  }

  return CFTimeUnit::Hours; //default value
}

int64_t QgsNetCdfGribTimeParser::cfJulianTimeFromGregorianCalendarDate( CFDateTimeValues values )
{
  // https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  if ( values.month <= 2 )
  {
    values.year--;
    values.month += 12;
  }

  const int A = values.year / 100;
  const int B = A / 4;
  const int C = 2 - A + B;
  const int E = int( 365.25 * ( values.year + 4716 ) );
  const int F = int( 30.6001 * ( values.month + 1 ) );
  const double julianDay = C + values.day + E + F - 1524.5;

  return int64_t( julianDay * MILLISECONDS_IN_DAY + ( values.hours ) * MILLISECONDS_IN_HOUR + values.minutes * MILLISECONDS_IN_MINUTE + values.seconds * MILLISECONDS_IN_SECOND );
}

int64_t QgsNetCdfGribTimeParser::cfJulianTimeFromJulianCalendarDate( CFDateTimeValues values )
{
  // https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  if ( values.month <= 2 )
  {
    values.year--;
    values.month += 12;
  }

  const int E = int( 365.25 * ( values.year + 4716 ) );
  const int F = int( 30.6001 * ( values.month + 1 ) );
  const double julianDay = values.day + E + F - 1524.5;

  return int64_t( julianDay * MILLISECONDS_IN_DAY + ( values.hours ) * MILLISECONDS_IN_HOUR + values.minutes * MILLISECONDS_IN_MINUTE + values.seconds * MILLISECONDS_IN_SECOND );
}

int64_t QgsNetCdfGribTimeParser::cfJulianTimeFromGregorianJulianCalendarDate( const CFDateTimeValues &values )
{
  if ( values.year > 1582 || ( values.year == 1582 && ( values.month > 10 || ( values.month == 10 && values.day >= 15 ) ) ) ) // gregorian calendar
  {
    return cfJulianTimeFromGregorianCalendarDate( values );
  }
  else
    return cfJulianTimeFromJulianCalendarDate( values );
}

std::optional<int64_t> QgsNetCdfGribTimeParser::cfParseReferenceTime( const QString &timeInformation, const QString &calendarString )
{
  const QStringList strings = cfSplit( timeInformation, ' ' );
  if ( strings.size() < 3 )
    return std::nullopt;

  if ( strings[1] != "since"_L1 )
    return std::nullopt;

  const QString &dateString = strings[2];

  const QStringList dateStringValues = cfSplit( dateString, '-' );
  if ( dateStringValues.size() != 3 )
    return std::nullopt;

  CFDateTimeValues values;
  values.year = cfToInt( dateStringValues[0] );
  values.month = cfToInt( dateStringValues[1] );
  values.day = cfToInt( dateStringValues[2] );
  values.hours = 0;
  values.minutes = 0;
  values.seconds = 0;

  if ( strings.size() > 3 )
  {
    const QString &timeString = strings[3];
    const QStringList timeStringsValue = cfSplit( timeString, ':' );
    if ( timeStringsValue.size() == 3 )
    {
      values.hours = cfToInt( timeStringsValue[0] );
      values.minutes = cfToInt( timeStringsValue[1] );
      values.seconds = cfToDouble( timeStringsValue[2] );
    }
  }

  CFCalendar calendar;
  if ( calendarString == "gregorian"_L1 || calendarString == "standard"_L1 || calendarString.isEmpty() )
    calendar = CFCalendar::Gregorian;
  else if ( calendarString == "proleptic_gregorian"_L1 )
    calendar = CFCalendar::ProlepticGregorian;
  else if ( calendarString == "julian"_L1 )
    calendar = CFCalendar::Julian;
  else
    return std::nullopt;

  // MDAL::DateTime( year, month, day, hours, minutes, seconds, calendar )
  switch ( calendar )
  {
    case CFCalendar::Gregorian:
      return cfJulianTimeFromGregorianJulianCalendarDate( values );
    case CFCalendar::ProlepticGregorian:
      return cfJulianTimeFromGregorianCalendarDate( values );
    case CFCalendar::Julian:
      return cfJulianTimeFromJulianCalendarDate( values );
  }
  return std::nullopt;
}

int64_t QgsNetCdfGribTimeParser::cfRelativeTimestamp( double duration, CFTimeUnit unit )
{
  double milliseconds = 0;
  switch ( unit )
  {
    case CFTimeUnit::Milliseconds:
      return static_cast<int64_t>( duration );
    case CFTimeUnit::Seconds:
      milliseconds = duration * MILLISECONDS_IN_SECOND;
      break;
    case CFTimeUnit::Minutes:
      milliseconds = duration * MILLISECONDS_IN_MINUTE;
      break;
    case CFTimeUnit::Hours:
      milliseconds = duration * MILLISECONDS_IN_HOUR;
      break;
    case CFTimeUnit::Days:
      milliseconds = duration * MILLISECONDS_IN_DAY;
      break;
    case CFTimeUnit::Weeks:
      milliseconds = duration * MILLISECONDS_IN_WEEK;
      break;
    case CFTimeUnit::MonthsCF:
      milliseconds = duration * MILLISECONDS_IN_MONTH_CF;
      break;
    case CFTimeUnit::ExactYears:
      milliseconds = duration * MILLISECONDS_IN_EXACT_YEAR;
      break;
  }

  // MDAL uses "int64_t( x + 0.5 )", i.e. truncation towards zero (not std::llround), keep it identical
  const double shiftedMilliseconds = milliseconds + 0.5;
  return static_cast<int64_t>( shiftedMilliseconds );
}

QgsNetCdfGribTimeParser::CFDateTimeValues QgsNetCdfGribTimeParser::cfDateTimeGregorianProleptic( int64_t julianTime )
{
  CFDateTimeValues values;
  // MDAL uses "int( x + 0.5 )", i.e. truncation towards zero, keep it identical
  const double shiftedJulianDays = julianTime / MILLISECONDS_IN_DAY + 0.5;
  const int Z = static_cast<int>( shiftedJulianDays );           // integer part of julian days count
  int F = int( julianTime - MILLISECONDS_IN_DAY * ( Z - 0.5 ) ); // fractional part of julian days count in ms;

  const int alpha = int( ( Z - 1867216.25 ) / 36524.25 );
  const int S = Z + 1 + alpha - int( alpha / 4 );

  const int B = S + 1524;
  const int C = int( ( B - 122.1 ) / 365.25 );
  const int D = int( 365.25 * C );
  const int E = int( ( B - D ) / 30.6001 );

  values.day = B - D - int( 30.6001 * E );
  if ( E < 14 )
    values.month = E - 1;
  else
    values.month = E - 13;

  if ( values.month > 2 )
    values.year = C - 4716;
  else
    values.year = C - 4715;

  values.hours = int( F / MILLISECONDS_IN_HOUR );
  F = int( F - values.hours * MILLISECONDS_IN_HOUR );
  values.minutes = int( F / MILLISECONDS_IN_MINUTE );
  F = int( F - values.minutes * MILLISECONDS_IN_MINUTE );
  values.seconds = int( F / MILLISECONDS_IN_SECOND );

  return values;
}

QDateTime QgsNetCdfGribTimeParser::cfJulianTimeToDateTime( int64_t julianTime )
{
  const CFDateTimeValues values = cfDateTimeGregorianProleptic( julianTime );
  // MDAL::DateTime::toStandardCalendarISO8601() does not support non-positive years
  if ( values.year <= 0 )
    return QDateTime();

  return QDateTime( QDate( values.year, values.month, values.day ), QTime( values.hours, values.minutes, static_cast<int>( values.seconds ) ), Qt::UTC );
}

std::optional<double> QgsNetCdfGribTimeParser::gribParseMetadataTime( const QString &timeString )
{
  const QString timeTrimmed = timeString.trimmed();
  const QStringList times = cfSplit( timeTrimmed, ' ' );
  if ( times.isEmpty() )
    return std::nullopt;
  return cfToDouble( times[0] );
}

QDateTime QgsNetCdfGribTimeParser::parseCfTime( double value, const QString &units, const QString &calendar )
{
  const std::optional<int64_t> referenceTime = cfParseReferenceTime( units, calendar );
  if ( !referenceTime.has_value() )
    return QDateTime();

  const CFTimeUnit unit = cfParseTimeUnit( units );
  return cfJulianTimeToDateTime( referenceTime.value() + cfRelativeTimestamp( value, unit ) );
}

QDateTime QgsNetCdfGribTimeParser::parseGribTime( const QString &metadataValue )
{
  const std::optional<double> value = gribParseMetadataTime( metadataValue );
  if ( !value.has_value() )
    return QDateTime();

  // MDAL::DateTime( double value, Epoch::Unix )
  const int64_t unixEpoch = cfJulianTimeFromGregorianJulianCalendarDate( CFDateTimeValues { 1970, 1, 1, 0, 0, 0 } );
  return cfJulianTimeToDateTime( unixEpoch + cfRelativeTimestamp( value.value(), CFTimeUnit::Seconds ) );
}
