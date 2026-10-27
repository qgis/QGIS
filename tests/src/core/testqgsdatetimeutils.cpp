/***************************************************************************
     testqgsdatetimeutils.cpp
     --------------------
    begin                : October 2026
    copyright            : (C) 2026 Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatetimeutils.h"
#include "qgstest.h"

#include <QDirIterator>
#include <QObject>
#include <QString>
#include <QStringList>

using namespace Qt::StringLiterals;

class TestQgsDateTimeUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void readWriteToXml();

  private:
};

void TestQgsDateTimeUtils::initTestCase()
{}

void TestQgsDateTimeUtils::readWriteToXml()
{
  QList<QDateTime> dateTimes;
  dateTimes
    << QDateTime()
    << QDateTime( QDate( 1950, 2, 3 ), QTime( 4, 5, 6 ), QTimeZone::LocalTime )
    << QDateTime( QDate( 2025, 2, 3 ), QTime( 4, 5, 6 ), QTimeZone::LocalTime )
    << QDateTime( QDate( 1950, 12, 3 ), QTime( 4, 5, 6 ), QTimeZone( u"Europe/Paris"_s.toUtf8() ) )
    << QDateTime( QDate( 2026, 10, 25 ), QTime( 2, 30, 6 ), QTimeZone( u"Europe/Paris"_s.toUtf8() ) )
    << QDateTime( QDate( 2025, 2, 3 ), QTime( 4, 5, 6 ), QTimeZone( u"Europe/Paris"_s.toUtf8() ) )
    << QDateTime( QDate( 1950, 1, 1 ), QTime( 0, 0, 0 ), QTimeZone( u"Asia/Tokyo"_s.toUtf8() ) )
    << QDateTime( QDate( 2020, 1, 12 ), QTime( 0, 0, 0 ), QTimeZone( u"Asia/Tokyo"_s.toUtf8() ) )
    << QDateTime( QDate( 1950, 2, 3 ), QTime( 4, 5, 6 ), QTimeZone( QTimeZone::UTC ) )
    << QDateTime( QDate( 2025, 10, 3 ), QTime( 4, 5, 6 ), QTimeZone( QTimeZone::UTC ) )
    << QDateTime( QDate( 2025, 2, 27 ), QTime( 4, 5, 6 ), QTimeZone::fromSecondsAheadOfUtc( 10 * 3600 ) )
    << QDateTime( QDate( 1960, 4, 27 ), QTime( 4, 5, 6 ), QTimeZone::fromSecondsAheadOfUtc( -5 * 3600 ) )
    << QDateTime( QDate( 2025, 9, 27 ), QTime( 4, 5, 6 ), QTimeZone::fromSecondsAheadOfUtc( 20 * 3600 ) ); //invalid time

  QDomDocument doc;
  QgsReadWriteContext context;
  QDomElement rootElement = doc.createElement( "root" );

  for ( qsizetype i = 0; i < dateTimes.size(); ++i )
  {
    const QDateTime &dt = dateTimes.at( i );
    rootElement.appendChild( QgsDateTimeUtils::writeDateTimeToXml( dt, u"my datetime %1"_s.arg( i ), doc, context ) );
  }

  for ( qsizetype i = 0; i < dateTimes.size(); ++i )
  {
    QDomElement dateTimeElement = rootElement.firstChildElement( u"my datetime %1"_s.arg( i ) );
    const QDateTime readDateTime = QgsDateTimeUtils::readDateTimeToXml( dateTimeElement, context );

    const QDateTime &dt = dateTimes.at( i );
    if ( dt.isValid() )
    {
      QVERIFY( readDateTime.isValid() );
      QCOMPARE( readDateTime, dt );
      QCOMPARE( readDateTime.timeZone(), dt.timeZone() );
      QCOMPARE( readDateTime.timeSpec(), dt.timeSpec() );
    }
    else
    {
      QVERIFY( !readDateTime.isValid() );
    }
  }
}

QGSTEST_MAIN( TestQgsDateTimeUtils )
#include "testqgsdatetimeutils.moc"
