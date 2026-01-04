/***************************************************************************
                         qgsprojecttimesettings.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojecttimesettings.h"

#include "qgis.h"
#include "qgsunittypes.h"

#include <QDomElement>

#include "moc_qgsprojecttimesettings.cpp"

QgsProjectTimeSettings::QgsProjectTimeSettings( QObject *parent )
  : QObject( parent )
{

}

void QgsProjectTimeSettings::reset()
{
  mRange = QgsDateTimeRange();
  emit temporalRangeChanged();
}

QgsDateTimeRange QgsProjectTimeSettings::temporalRange() const
{
  return mRange;
}

void QgsProjectTimeSettings::setTemporalRange( const QgsDateTimeRange &range )
{
  if ( range == mRange )
    return;
  mRange = range;

  emit temporalRangeChanged();
}

bool QgsProjectTimeSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomElement temporalElement = element.firstChildElement( u"TemporalRange"_s );
  if ( !temporalElement.isNull() )
  {
    const QDomNode begin = temporalElement.namedItem( u"start"_s );
    const QDomNode end = temporalElement.namedItem( u"end"_s );

    const QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
    const QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );

    setTemporalRange( QgsDateTimeRange( beginDate, endDate ) );

  }

  mTimeStepUnit = QgsUnitTypes::decodeTemporalUnit( element.attribute( u"timeStepUnit"_s, QgsUnitTypes::encodeUnit( Qgis::TemporalUnit::Hours ) ) );
  mTimeStep = element.attribute( u"timeStep"_s, "1" ).toDouble();
  mFrameRate = element.attribute( u"frameRate"_s, "1" ).toDouble();
  mCumulativeTemporalRange = element.attribute( u"cumulativeTemporalRange"_s, "0" ).toInt();

  mTotalMovieFrames = element.attribute( u"totalMovieFrames"_s, "100" ).toLongLong();

  return true;
}

QDomElement QgsProjectTimeSettings::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( u"ProjectTimeSettings"_s );

  if ( mRange.begin().isValid() && mRange.end().isValid() )
  {
    QDomElement temporalElement = document.createElement( u"TemporalRange"_s );
    QDomElement startElement = document.createElement( u"start"_s );
    QDomElement endElement = document.createElement( u"end"_s );

    const QDomText startText = document.createTextNode( mRange.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
    const QDomText endText = document.createTextNode( mRange.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );

    startElement.appendChild( startText );
    endElement.appendChild( endText );

    temporalElement.appendChild( startElement );
    temporalElement.appendChild( endElement );

    element.appendChild( temporalElement );
  }

  element.setAttribute( u"timeStepUnit"_s, QgsUnitTypes::encodeUnit( mTimeStepUnit ) );
  element.setAttribute( u"timeStep"_s, qgsDoubleToString( mTimeStep ) );
  element.setAttribute( u"frameRate"_s, qgsDoubleToString( mFrameRate ) );
  element.setAttribute( u"cumulativeTemporalRange"_s,  mCumulativeTemporalRange ? 1 : 0 );
  element.setAttribute( u"totalMovieFrames"_s,  mTotalMovieFrames );

  return element;
}

Qgis::TemporalUnit QgsProjectTimeSettings::timeStepUnit() const
{
  return mTimeStepUnit;
}

void QgsProjectTimeSettings::setTimeStepUnit( Qgis::TemporalUnit unit )
{
  mTimeStepUnit = unit;
}

double QgsProjectTimeSettings::timeStep() const
{
  return mTimeStep;
}

void QgsProjectTimeSettings::setTimeStep( double timeStep )
{
  mTimeStep = timeStep;
}

void QgsProjectTimeSettings::setFramesPerSecond( double rate )
{
  mFrameRate = rate;
}

double QgsProjectTimeSettings::framesPerSecond() const
{
  return mFrameRate;
}

void QgsProjectTimeSettings::setIsTemporalRangeCumulative( bool state )
{
  mCumulativeTemporalRange = state;
}

bool QgsProjectTimeSettings::isTemporalRangeCumulative() const
{
  return mCumulativeTemporalRange;
}

long long QgsProjectTimeSettings::totalMovieFrames() const
{
  return mTotalMovieFrames;
}

void QgsProjectTimeSettings::setTotalMovieFrames( long long frames )
{
  mTotalMovieFrames = frames;
}

