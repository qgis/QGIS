/***************************************************************************
    testqgsdatetimeedit.cpp
     --------------------------------------
    Date                 : September 2019
    Copyright            : (C) 2019 Etienne Trimaille
    Email                : etienne dot trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include "qdatetime.h"

#include <qgsdatetimeedit.h>

class TestQgsDateTimeEdit: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void nullValues();

};

void TestQgsDateTimeEdit::initTestCase()
{
}

void TestQgsDateTimeEdit::cleanupTestCase()
{
}

void TestQgsDateTimeEdit::init()
{
}

void TestQgsDateTimeEdit::cleanup()
{
}

void TestQgsDateTimeEdit::nullValues()
{
  QgsDateTimeEdit *timeEdit = new QgsDateTimeEdit();

  // Allow null with a null datetime
  QVERIFY( timeEdit->allowNull() );
  timeEdit->setDateTime( QDateTime() );
  QCOMPARE( timeEdit->dateTime(), QDateTime() );
  QCOMPARE( timeEdit->time(), QTime() );
  QCOMPARE( timeEdit->date(), QDate() );

  // Not null with not null datetime
  QDateTime date( QDate( 2019, 7, 6 ), QTime( 8, 30, 0 ) );
  timeEdit->setAllowNull( false );
  QVERIFY( !timeEdit->allowNull() );
  timeEdit->setDateTime( date );
  QCOMPARE( timeEdit->dateTime(), date );
  QCOMPARE( timeEdit->time(), date.time() );
  QCOMPARE( timeEdit->date(), date.date() );

  // Not null with null date
  timeEdit->setAllowNull( false );
  QVERIFY( !timeEdit->allowNull() );
  timeEdit->setDateTime( QDateTime() );
  QCOMPARE( timeEdit->dateTime(), date );
  QCOMPARE( timeEdit->time(), date.time() );
  QCOMPARE( timeEdit->date(), date.date() );

  delete timeEdit;
}

QGSTEST_MAIN( TestQgsDateTimeEdit )
#include "testqgsdatetimeedit.moc"
