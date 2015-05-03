/***************************************************************************
     testqgsfields.cpp
     -----------------
    Date                 : May 2015
    Copyright            : (C) 2015 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QSharedPointer>

#include "qgsfield.h"

class TestQgsFields: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void create();//test creating a data defined container
    void copy();// test cpy destruction (double delete)
    void assignment();
    void equality(); //test equality operators
  private:
};

void TestQgsFields::initTestCase()
{

}

void TestQgsFields::cleanupTestCase()
{

}

void TestQgsFields::init()
{

}

void TestQgsFields::cleanup()
{

}

void TestQgsFields::create()
{
  QgsFields fields;
  QCOMPARE( fields.count(), 0 );
}

void TestQgsFields::copy()
{
  QgsFields original;
  //add field
  QgsField field( "testfield" );
  original.append( field );
  QCOMPARE( original.count(), 1 );
  QgsFields copy( original );
  QCOMPARE( copy.count(), 1 );
  QVERIFY( copy == original );

  QgsField copyfield( "copyfield" );
  copy.append( copyfield );
  QCOMPARE( copy.count(), 2 );
  QCOMPARE( original.count(), 1 );
  QVERIFY( copy != original );
}

void TestQgsFields::assignment()
{
  QgsFields original;
  //add field
  QgsField field( "testfield" );
  original.append( field );

  QgsFields copy;
  copy = original;
  QVERIFY( copy == original );

  QgsField copyfield( "copyfield" );
  copy.append( copyfield );
  QCOMPARE( original.count(), 1 );
  QCOMPARE( copy.count(), 2 );
  QVERIFY( copy != original );
}

void TestQgsFields::equality()
{
  //compare two empty QgsFields
  QgsFields fields1;
  QgsFields fields2;
  QVERIFY( fields1 == fields2 );
  QVERIFY( !( fields1 != fields2 ) );

  //append an identical fields to both and retest
  QgsField field1;
  field1.setName( "name" );
  QgsField field2;
  field2.setName( "name" );
  QCOMPARE( field1, field2 );
  fields1.append( field1 );
  fields2.append( field2 );
  QVERIFY( fields1 == fields2 );
  QVERIFY( !( fields1 != fields2 ) );

  //make a change and retest
  QgsField field3;
  fields2.append( field3 );
  QVERIFY( !( fields1 == fields2 ) );
  QVERIFY( fields1 != fields2 );
}

QTEST_MAIN( TestQgsFields )
#include "testqgsfields.moc"
