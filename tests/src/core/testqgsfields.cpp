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
    void asVariant(); //test conversion to and from a QVariant
    void clear();
    void exists();
    void count();
    void isEmpty();
    void remove();
    void extend();
    void byIndex();
    void byName();
    void fieldOrigin();
    void fieldOriginIndex();

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

void TestQgsFields::asVariant()
{
  QgsField field1;
  field1.setName( "name" );
  QgsField field2;
  field2.setName( "name" );
  QgsFields original;
  original.append( field1 );
  original.append( field2 );

  //convert to and from a QVariant
  QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  QgsFields fromVar = qvariant_cast<QgsFields>( var );
  QCOMPARE( fromVar, original );
}

void TestQgsFields::clear()
{
  QgsFields original;
  QgsField field( "testfield" );
  original.append( field );
  QCOMPARE( original.count(), 1 );
  QgsFields copy( original );

  copy.clear();
  QCOMPARE( copy.count(), 0 );
  QCOMPARE( original.count(), 1 );
}

void TestQgsFields::exists()
{
  QgsFields fields;
  QgsField field( "testfield" );
  fields.append( field );

  QVERIFY( !fields.exists( -1 ) );
  QVERIFY( !fields.exists( 1 ) );
  QVERIFY( fields.exists( 0 ) );
}

void TestQgsFields::count()
{
  QgsFields fields;
  QCOMPARE( fields.count(), 0 );
  QCOMPARE( fields.size(), 0 );

  QgsField field( "testfield" );
  fields.append( field );
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.size(), 1 );

  QgsField field2( "testfield2" );
  fields.append( field2 );
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.size(), 2 );
}

void TestQgsFields::isEmpty()
{
  QgsFields fields;
  QVERIFY( fields.isEmpty() );

  QgsField field( "testfield" );
  fields.append( field );
  QVERIFY( !fields.isEmpty() );
}

void TestQgsFields::remove()
{
  QgsFields fields;

  //test for no crash
  fields.remove( 1 );

  QgsField field( "testfield" );
  fields.append( field );
  QgsField field2( "testfield2" );
  fields.append( field2 );

  //test for no crash
  fields.remove( -1 );
  fields.remove( 5 );

  //remove valid field
  fields.remove( 0 );
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.at( 0 ).name(), QString( "testfield2" ) );
}

void TestQgsFields::extend()
{
  QgsFields destination;
  QgsField field( "testfield" );
  destination.append( field );
  QgsField field2( "testfield2" );
  destination.append( field2 );

  QgsFields source;
  QgsField field3( "testfield3" );
  source.append( field3, QgsFields::OriginJoin, 5 );
  QgsField field4( "testfield4" );
  source.append( field4 );

  QCOMPARE( destination.count(), 2 );
  destination.extend( source );
  QCOMPARE( destination.count(), 4 );
  QCOMPARE( destination.at( 2 ), field3 );
  QCOMPARE( destination.at( 3 ), field4 );
}

void TestQgsFields::byIndex()
{
  QgsFields fields;
  QgsField field( "testfield" );
  fields.append( field );
  QgsField field2( "testfield2" );
  fields.append( field2 );

  QCOMPARE( fields[0], field );
  QCOMPARE( fields[1], field2 );

  const QgsFields& constFields = fields;
  QCOMPARE( constFields[0], field );
  QCOMPARE( constFields[1], field2 );
  QCOMPARE( constFields.at( 0 ), field );
  QCOMPARE( constFields.at( 1 ), field2 );
  QCOMPARE( constFields.field( 0 ), field );
  QCOMPARE( constFields.field( 1 ), field2 );
}

void TestQgsFields::byName()
{
  QgsFields fields;
  QgsField field( "testfield" );
  fields.append( field );
  QgsField field2( "testfield2" );
  fields.append( field2 );

  QCOMPARE( fields.field( "testfield" ), field );
  QCOMPARE( fields.field( "testfield2" ), field2 );
}

void TestQgsFields::fieldOrigin()
{
  QgsFields fields;
  QgsField field( QString( "testfield" ) );
  fields.append( field , QgsFields::OriginJoin );
  QgsField field2( QString( "testfield2" ) );
  fields.append( field2, QgsFields::OriginExpression );

  QCOMPARE( fields.fieldOrigin( 0 ), QgsFields::OriginJoin );
  QCOMPARE( fields.fieldOrigin( 1 ), QgsFields::OriginExpression );
}

void TestQgsFields::fieldOriginIndex()
{

}

QTEST_MAIN( TestQgsFields )
#include "testqgsfields.moc"
