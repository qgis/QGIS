/***************************************************************************
     testqgsfield.cpp
     ----------------
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

class TestQgsField: public QObject
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
    void gettersSetters(); //test getters and setters
    void equality(); //test equality operators
  private:
};

void TestQgsField::initTestCase()
{

}

void TestQgsField::cleanupTestCase()
{

}

void TestQgsField::init()
{

}

void TestQgsField::cleanup()
{

}

void TestQgsField::create()
{
  QScopedPointer<QgsField> field( new QgsField( "name", QVariant::Double, "double", 5, 2, "comment" ) );
  QCOMPARE( field->name(), QString( "name" ) );
  QCOMPARE( field->type(), QVariant::Double );
  QCOMPARE( field->typeName(), QString( "double" ) );
  QCOMPARE( field->length(), 5 );
  QCOMPARE( field->precision(), 2 );
  QCOMPARE( field->comment(), QString( "comment" ) );
}

void TestQgsField::copy()
{
  QgsField original( "original", QVariant::Double, "double", 5, 2, "comment" );
  QgsField copy( original );
  QVERIFY( copy == original );

  copy.setName( "copy" );
  QCOMPARE( original.name(), QString( "original" ) );
  QVERIFY( copy != original );
}

void TestQgsField::assignment()
{
  QgsField original( "original", QVariant::Double, "double", 5, 2, "comment" );
  QgsField copy;
  copy = original;
  QVERIFY( copy == original );

  copy.setName( "copy" );
  QCOMPARE( original.name(), QString( "original" ) );
  QVERIFY( copy != original );
}

void TestQgsField::gettersSetters()
{
  QgsField field;
  field.setName( "name" );
  QCOMPARE( field.name(), QString( "name" ) );
  field.setType( QVariant::Int );
  QCOMPARE( field.type(), QVariant::Int );
  field.setTypeName( "typeName" );
  QCOMPARE( field.typeName(), QString( "typeName" ) );
  field.setLength( 5 );
  QCOMPARE( field.length(), 5 );
  field.setPrecision( 2 );
  QCOMPARE( field.precision(), 2 );
  field.setComment( "comment" );
  QCOMPARE( field.comment(), QString( "comment" ) );
}

void TestQgsField::equality()
{
  QgsField field1;
  field1.setName( "name" );
  field1.setType( QVariant::Int );
  field1.setLength( 5 );
  field1.setPrecision( 2 );
  field1.setTypeName( "typename1" ); //typename is NOT required for equality
  field1.setComment( "comment1" ); //comment is NOT required for equality
  QgsField field2;
  field2.setName( "name" );
  field2.setType( QVariant::Int );
  field2.setLength( 5 );
  field2.setPrecision( 2 );
  field2.setTypeName( "typename2" ); //typename is NOT required for equality
  field2.setComment( "comment2" ); //comment is NOT required for equality
  QVERIFY( field1 == field2 );
  QVERIFY( !( field1 != field2 ) );

  //test that all applicable components contribute to equality
  field2.setName( "name2" );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setName( "name" );
  field2.setType( QVariant::Double );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setType( QVariant::Int );
  field2.setLength( 9 );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setLength( 5 );
  field2.setPrecision( 9 );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setPrecision( 2 );
}

QTEST_MAIN( TestQgsField )
#include "testqgsfield.moc"
