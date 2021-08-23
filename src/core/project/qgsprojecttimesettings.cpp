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
#include <QDomElement>

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
  const QDomElement temporalElement = element.firstChildElement( QStringLiteral( "TemporalRange" ) );
  if ( !temporalElement.isNull() )
  {
    const QDomNode begin = temporalElement.namedItem( QStringLiteral( "start" ) );
    const QDomNode end = temporalElement.namedItem( QStringLiteral( "end" ) );

    const QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
    const QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );

    setTemporalRange( QgsDateTimeRange( beginDate, endDate ) );

  }

  mTimeStepUnit = QgsUnitTypes::decodeTemporalUnit( element.attribute( QStringLiteral( "timeStepUnit" ), QgsUnitTypes::encodeUnit( QgsUnitTypes::TemporalHours ) ) );
  mTimeStep = element.attribute( QStringLiteral( "timeStep" ), "1" ).toDouble();
  mFrameRate = element.attribute( QStringLiteral( "frameRate" ), "1" ).toDouble();
  mCumulativeTemporalRange = element.attribute( QStringLiteral( "cumulativeTemporalRange" ), "0" ).toInt();

  return true;
}

QDomElement QgsProjectTimeSettings::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "ProjectTimeSettings" ) );

  if ( mRange.begin().isValid() && mRange.end().isValid() )
  {
    QDomElement temporalElement = document.createElement( QStringLiteral( "TemporalRange" ) );
    QDomElement startElement = document.createElement( QStringLiteral( "start" ) );
    QDomElement endElement = document.createElement( QStringLiteral( "end" ) );

    const QDomText startText = document.createTextNode( mRange.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
    const QDomText endText = document.createTextNode( mRange.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );

    startElement.appendChild( startText );
    endElement.appendChild( endText );

    temporalElement.appendChild( startElement );
    temporalElement.appendChild( endElement );

    element.appendChild( temporalElement );
  }

  element.setAttribute( QStringLiteral( "timeStepUnit" ), QgsUnitTypes::encodeUnit( mTimeStepUnit ) );
  element.setAttribute( QStringLiteral( "timeStep" ), qgsDoubleToString( mTimeStep ) );
  element.setAttribute( QStringLiteral( "frameRate" ), qgsDoubleToString( mFrameRate ) );
  element.setAttribute( QStringLiteral( "cumulativeTemporalRange" ),  mCumulativeTemporalRange ? 1 : 0 );

  return element;
}

QgsUnitTypes::TemporalUnit QgsProjectTimeSettings::timeStepUnit() const
{
  return mTimeStepUnit;
}

void QgsProjectTimeSettings::setTimeStepUnit( QgsUnitTypes::TemporalUnit unit )
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

