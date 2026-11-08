/***************************************************************************
                            qgsdatetimeutils.cpp
                          ---------------------
    begin                : October 2026
    copyright            : (C) 2026 by Vincent Cloarec
    email                : vcloarec@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatetimeutils.h"

#include <QString>

using namespace Qt::StringLiterals;

void QgsDateTimeUtils::writeDateTimeToXml( const QDateTime &dateTime, const QString &tagName, QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  if ( dateTime.isNull() )
  {
    element.setAttribute( u"%1-null"_s.arg( tagName ), u"true"_s );
    return;
  }

  element.setAttribute( u"%1-epoch-ms"_s.arg( tagName ), QString::number( dateTime.toMSecsSinceEpoch() ) );

  const QString specAttributeName = u"%1-time-spec"_s.arg( tagName );
  switch ( dateTime.timeSpec() )
  {
    case Qt::TimeSpec::LocalTime:
      element.setAttribute( specAttributeName, u"local-time"_s );
      break;
    case Qt::TimeSpec::UTC:
      element.setAttribute( specAttributeName, u"utc"_s );
      break;
    case Qt::TimeSpec::OffsetFromUTC:
      element.setAttribute( specAttributeName, u"offset-from-utc"_s );
      element.setAttribute( u"%1-offset"_s.arg( tagName ), QString::number( dateTime.offsetFromUtc() ) );
      break;
    case Qt::TimeSpec::TimeZone:
      QByteArray ts = dateTime.timeZone().id();
      element.setAttribute( specAttributeName, u"time-zone"_s );
      element.setAttribute( u"%1-time-zone"_s.arg( tagName ), QString::fromUtf8( dateTime.timeZone().id() ) );
      break;
  }
}

QDateTime QgsDateTimeUtils::readDateTimeFromXml( const QDomElement &element, const QString &tagName, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  if ( element.attribute( u"%1-null"_s.arg( tagName ) ) == "true"_L1 )
    return QDateTime();

  bool ok;
  qint64 sinceEpoch = element.attribute( u"%1-epoch-ms"_s.arg( tagName ) ).toLongLong( &ok );
  if ( !ok )
    return QDateTime();

  const QString timeSpecStr = element.attribute( u"%1-time-spec"_s.arg( tagName ) );

  QTimeZone timeZone;

  if ( timeSpecStr == "local-time"_L1 )
    timeZone = QTimeZone::LocalTime;

  if ( timeSpecStr == "utc"_L1 )
    timeZone = QTimeZone::UTC;

  if ( timeSpecStr == "offset-from-utc"_L1 )
  {
    int offset = element.attribute( u"%1-offset"_s.arg( tagName ) ).toInt( &ok );
    if ( !ok )
      return QDateTime();

    timeZone = QTimeZone::fromSecondsAheadOfUtc( offset );
  }

  if ( timeSpecStr == "time-zone"_L1 )
  {
    const QByteArray id = element.attribute( u"%1-time-zone"_s.arg( tagName ) ).toUtf8();
    if ( id.isEmpty() )
      timeZone = QTimeZone();
    else
      timeZone = QTimeZone( id );
  }

  return QDateTime::fromMSecsSinceEpoch( sinceEpoch, timeZone );
}
