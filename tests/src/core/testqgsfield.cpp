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
    void asVariant(); //test conversion to and from a QVariant
    void displayString();
    void convertCompatible();
    void dataStream();

  private:
};

void TestQgsField::initTestCase()
{
  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );
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

void TestQgsField::asVariant()
{
  QgsField original( "original", QVariant::Double, "double", 5, 2, "comment" );

  //convert to and from a QVariant
  QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  QgsField fromVar = qvariant_cast<QgsField>( var );
  QCOMPARE( fromVar, original );
}

void TestQgsField::displayString()
{
  QgsField stringField( "string", QVariant::String, "string" );

  //test string value
  QString test( "test string" );
  QCOMPARE( stringField.displayString( test ), test );

  //test NULL
  QSettings s;
  s.setValue( "qgis/nullValue", "TEST NULL" );
  QVariant nullString = QVariant( QVariant::String );
  QCOMPARE( stringField.displayString( nullString ), QString( "TEST NULL" ) );

  //test int value
  QgsField intField( "int", QVariant::String, "int" );
  QCOMPARE( intField.displayString( 5 ), QString( "5" ) );
  //test NULL int
  QVariant nullInt = QVariant( QVariant::Int );
  QCOMPARE( intField.displayString( nullInt ), QString( "TEST NULL" ) );

  //test double value
  QgsField doubleField( "double", QVariant::Double, "double", 10, 3 );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5.005" ) );
  //test NULL double
  QVariant nullDouble = QVariant( QVariant::Double );
  QCOMPARE( doubleField.displayString( nullDouble ), QString( "TEST NULL" ) );

}

void TestQgsField::convertCompatible()
{
  //test string field
  QgsField stringField( "string", QVariant::String, "string" );

  QVariant stringVar( "test string" );
  QVERIFY( stringField.convertCompatible( stringVar ) );
  QCOMPARE( stringVar.toString( ), QString( "test string" ) );
  QVariant nullString = QVariant( QVariant::String );
  QVERIFY( stringField.convertCompatible( nullString ) );
  QCOMPARE( nullString.type(), QVariant::String );
  QVERIFY( nullString.isNull() );
  QVariant intVar( 5 );
  QVERIFY( stringField.convertCompatible( intVar ) );
  QCOMPARE( intVar.type(), QVariant::String );
  QCOMPARE( intVar, QVariant( "5" ) );
  QVariant nullInt = QVariant( QVariant::Int );
  QVERIFY( stringField.convertCompatible( nullInt ) );
  QCOMPARE( nullInt.type(), QVariant::String );
  QVERIFY( nullInt.isNull() );
  QVariant doubleVar( 9.7 );
  QVERIFY( stringField.convertCompatible( doubleVar ) );
  QCOMPARE( doubleVar.type(), QVariant::String );
  QCOMPARE( doubleVar, QVariant( "9.7" ) );
  QVariant nullDouble = QVariant( QVariant::Double );
  QVERIFY( stringField.convertCompatible( nullDouble ) );
  QCOMPARE( nullDouble.type(), QVariant::String );
  QVERIFY( nullDouble.isNull() );

  //test double
  QgsField doubleField( "double", QVariant::Double, "double" );

  stringVar = QVariant( "test string" );
  QVERIFY( !doubleField.convertCompatible( stringVar ) );
  QCOMPARE( stringVar.type(), QVariant::Double );
  QVERIFY( stringVar.isNull( ) );
  nullString = QVariant( QVariant::String );
  QVERIFY( doubleField.convertCompatible( nullString ) );
  QCOMPARE( nullString.type(), QVariant::Double );
  QVERIFY( nullString.isNull() );
  intVar = QVariant( 5 );
  QVERIFY( doubleField.convertCompatible( intVar ) );
  QCOMPARE( intVar.type(), QVariant::Double );
  QCOMPARE( intVar, QVariant( 5.0 ) );
  nullInt = QVariant( QVariant::Int );
  QVERIFY( doubleField.convertCompatible( nullInt ) );
  QCOMPARE( nullInt.type(), QVariant::Double );
  QVERIFY( nullInt.isNull() );
  doubleVar = QVariant( 9.7 );
  QVERIFY( doubleField.convertCompatible( doubleVar ) );
  QCOMPARE( doubleVar.type(), QVariant::Double );
  QCOMPARE( doubleVar, QVariant( 9.7 ) );
  nullDouble = QVariant( QVariant::Double );
  QVERIFY( doubleField.convertCompatible( nullDouble ) );
  QCOMPARE( nullDouble.type(), QVariant::Double );
  QVERIFY( nullDouble.isNull() );

  //test special rules

  //conversion of longlong to int
  QgsField intField( "int", QVariant::Int, "int" );
  QVariant longlong( 99999999999999999LL );
  QVERIFY( !intField.convertCompatible( longlong ) );
  QCOMPARE( longlong.type(), QVariant::Int );
  QVERIFY( longlong.isNull( ) );
  //conversion of longlong to longlong field
  QgsField longlongField( "long", QVariant::LongLong, "longlong" );
  longlong = QVariant( 99999999999999999LL );
  QVERIFY( longlongField.convertCompatible( longlong ) );
  QCOMPARE( longlong.type(), QVariant::LongLong );
  QCOMPARE( longlong, QVariant( 99999999999999999LL ) );

  //double with precision
  QgsField doubleWithPrecField( "double", QVariant::Double, "double", 10, 3 );
  doubleVar = QVariant( 10.12345678 );
  //note - this returns true!
  QVERIFY( doubleWithPrecField.convertCompatible( doubleVar ) );
  QCOMPARE( doubleVar.type(), QVariant::Double );
  QCOMPARE( doubleVar.toDouble(), 10.123 );

  //truncating string length
  QgsField stringWithLen( "string", QVariant::String, "string", 3 );
  stringVar = QVariant( "longstring" );
  QVERIFY( !stringWithLen.convertCompatible( stringVar ) );
  QCOMPARE( stringVar.type(), QVariant::String );
  QCOMPARE( stringVar.toString(), QString( "lon" ) );
}

void TestQgsField::dataStream()
{
  QgsField original;
  original.setName( "name" );
  original.setType( QVariant::Int );
  original.setLength( 5 );
  original.setPrecision( 2 );
  original.setTypeName( "typename1" );
  original.setComment( "comment1" );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );;
  ds << original;

  QgsField result;
  ds.device()->seek( 0 );
  ds >> result;

  QCOMPARE( result, original );
  QCOMPARE( result.typeName(), original.typeName() ); //typename is NOT required for equality
  QCOMPARE( result.comment(), original.comment() ); //comment is NOT required for equality
}

QTEST_MAIN( TestQgsField )
#include "testqgsfield.moc"
