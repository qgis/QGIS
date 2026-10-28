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


QDomElement QgsDateTimeUtils::writeDateTimeToXml( const QDateTime &dateTime, const QString &tagName, QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement element = doc.createElement( tagName );

  if ( !dateTime.isValid() )
  {
    element.setAttribute( u"valid"_s, u"false"_s );
    return element;
  }

  element.setAttribute( u"valid"_s, u"true"_s );
  element.setAttribute( u"epoch-ms"_s, QString::number( dateTime.toMSecsSinceEpoch() ) );

  switch ( dateTime.timeSpec() )
  {
    case Qt::TimeSpec::LocalTime:
      element.setAttribute( u"time-spec"_s, u"local-time"_s );
      break;
    case Qt::TimeSpec::UTC:
      element.setAttribute( u"time-spec"_s, u"utc"_s );
      break;
    case Qt::TimeSpec::OffsetFromUTC:
      element.setAttribute( u"time-spec"_s, u"offset-from-utc"_s );
      element.setAttribute( u"offset"_s, QString::number( dateTime.offsetFromUtc() ) );
      break;
    case Qt::TimeSpec::TimeZone:
      element.setAttribute( u"time-spec"_s, u"time-zone"_s );
      element.setAttribute( u"time-zone"_s, QString::fromUtf8( dateTime.timeZone().id() ) );
      break;
  }

  return element;
}

QDateTime QgsDateTimeUtils::readDateTimeFromXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  if ( element.attribute( u"valid"_s ) != "true"_L1 )
    return QDateTime();

  bool ok;
  qint64 sinceEpoch = element.attribute( u"epoch-ms"_s ).toLongLong( &ok );
  if ( !ok )
    return QDateTime();

  const QString timeSpecStr = element.attribute( u"time-spec"_s );

  QTimeZone timeZone;

  if ( timeSpecStr == "local-time"_L1 )
    timeZone = QTimeZone::LocalTime;

  if ( timeSpecStr == "utc"_L1 )
    timeZone = QTimeZone::UTC;

  if ( timeSpecStr == "offset-from-utc"_L1 )
  {
    int offset = element.attribute( u"offset"_s ).toInt( &ok );
    if ( !ok )
      return QDateTime();

    timeZone = QTimeZone::fromSecondsAheadOfUtc( offset );
  }

  if ( timeSpecStr == "time-zone"_L1 )
  {
    const QByteArray id = element.attribute( u"time-zone"_s ).toUtf8();
    timeZone = QTimeZone( id );
  }

  if ( timeZone.isValid() )
    return QDateTime::fromMSecsSinceEpoch( sinceEpoch, timeZone );

  return QDateTime();
}
