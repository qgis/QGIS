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
    void indexFromName();
    void toList();
    void allAttrsList();
    void appendExpressionField();
    void dataStream();
    void field(); //test QgsFields::Field

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
  QCOMPARE( fields.indexFromName( "testfield2" ), 0 );
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
  QCOMPARE( fields.fieldOrigin( 2 ), QgsFields::OriginUnknown );
}

void TestQgsFields::fieldOriginIndex()
{
  QgsFields fields;
  QgsField field( QString( "testfield" ) );
  fields.append( field , QgsFields::OriginProvider, 5 );
  QCOMPARE( fields.fieldOriginIndex( 0 ), 5 );

  QgsField field2( QString( "testfield2" ) );
  fields.append( field2, QgsFields::OriginProvider, 10 );
  QCOMPARE( fields.fieldOriginIndex( 1 ), 10 );

  QgsField field3( QString( "testfield3" ) );
  //field origin index not specified with OriginProvider, should be automatic
  fields.append( field3, QgsFields::OriginProvider );
  QCOMPARE( fields.fieldOriginIndex( 2 ), 2 );

  QgsField field4( QString( "testfield4" ) );
  //field origin index not specified with other than OriginProvider, should remain -1
  fields.append( field4, QgsFields::OriginEdit );
  QCOMPARE( fields.fieldOriginIndex( 3 ), -1 );
}

void TestQgsFields::indexFromName()
{
  QgsFields fields;
  QgsField field( QString( "testfield" ) );
  fields.append( field );
  QgsField field2( QString( "testfield2" ) );
  fields.append( field2 );
  QgsField field3( QString( "testfield3" ) );
  fields.append( field3 );

  QCOMPARE( fields.indexFromName( QString( "bad" ) ), -1 );
  QCOMPARE( fields.fieldNameIndex( QString( "bad" ) ), -1 );
  QCOMPARE( fields.indexFromName( QString( "testfield" ) ), 0 );
  QCOMPARE( fields.fieldNameIndex( QString( "testfield" ) ), 0 );
  QCOMPARE( fields.indexFromName( QString( "testfield3" ) ), 2 );
  QCOMPARE( fields.fieldNameIndex( QString( "testfield3" ) ), 2 );

  //indexFromName is case sensitive, fieldNameIndex isn't
  QCOMPARE( fields.indexFromName( QString( "teStFiEld2" ) ), -1 );
  QCOMPARE( fields.fieldNameIndex( QString( "teStFiEld2" ) ), 1 );
}

void TestQgsFields::toList()
{
  QgsFields fields;
  QList<QgsField> list = fields.toList();
  QVERIFY( list.isEmpty() );

  QgsField field( QString( "testfield" ) );
  fields.append( field );
  QgsField field2( QString( "testfield2" ) );
  fields.append( field2 );
  QgsField field3( QString( "testfield3" ) );
  fields.append( field3 );

  list = fields.toList();
  QCOMPARE( list.at( 0 ), field );
  QCOMPARE( list.at( 1 ), field2 );
  QCOMPARE( list.at( 2 ), field3 );
}

void TestQgsFields::allAttrsList()
{
  QgsFields fields;
  QgsAttributeList attrList = fields.allAttributesList();
  QVERIFY( attrList.isEmpty() );

  QgsField field( QString( "testfield" ) );
  fields.append( field );
  QgsField field2( QString( "testfield2" ) );
  fields.append( field2 );
  QgsField field3( QString( "testfield3" ) );
  fields.append( field3 );

  attrList = fields.allAttributesList();
  QCOMPARE( attrList.at( 0 ), 0 );
  QCOMPARE( attrList.at( 1 ), 1 );
  QCOMPARE( attrList.at( 2 ), 2 );
}

void TestQgsFields::appendExpressionField()
{
  QgsFields fields;
  QgsField field( QString( "testfield" ) );
  fields.append( field );
  QgsField field2( QString( "testfield2" ) );
  fields.append( field2 );

  QgsField dupeName( QString( "testfield" ) );
  QVERIFY( !fields.appendExpressionField( dupeName, 1 ) );

  //good name
  QgsField exprField( QString( "expression" ) );
  QVERIFY( fields.appendExpressionField( exprField, 5 ) );
  QCOMPARE( fields.count(), 3 );
  QCOMPARE( fields.fieldOrigin( 2 ), QgsFields::OriginExpression );
  QCOMPARE( fields.fieldOriginIndex( 2 ), 5 );
}

void TestQgsFields::dataStream()
{
  QgsField original1;
  original1.setName( "name" );
  original1.setType( QVariant::Int );
  original1.setLength( 5 );
  original1.setPrecision( 2 );
  original1.setTypeName( "typename1" );
  original1.setComment( "comment1" );

  QgsField original2;
  original2.setName( "next name" );
  original2.setType( QVariant::Double );
  original2.setLength( 15 );
  original2.setPrecision( 3 );
  original2.setTypeName( "double" );
  original2.setComment( "comment for field 2" );

  QgsFields originalFields;
  originalFields.append( original1 );
  originalFields.append( original2 );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );;
  ds << originalFields;

  QgsFields resultFields;
  ds.device()->seek( 0 );
  ds >> resultFields;

  QCOMPARE( resultFields, originalFields );
  QCOMPARE( resultFields.field( 0 ).typeName(), originalFields.field( 0 ).typeName() ); //typename is NOT required for equality
  QCOMPARE( resultFields.field( 0 ).comment(), originalFields.field( 0 ).comment() ); //comment is NOT required for equality
  QCOMPARE( resultFields.field( 1 ).typeName(), originalFields.field( 1 ).typeName() );
  QCOMPARE( resultFields.field( 1 ).comment(), originalFields.field( 1 ).comment() );
}

void TestQgsFields::field()
{
  QgsField original;
  original.setName( "name" );
  original.setType( QVariant::Int );
  original.setLength( 5 );
  original.setPrecision( 2 );

  //test constructors for QgsFields::Field
  QgsFields::Field fieldConstructor1( original, QgsFields::OriginJoin, 5 );
  QCOMPARE( fieldConstructor1.field, original );
  QCOMPARE( fieldConstructor1.origin, QgsFields::OriginJoin );
  QCOMPARE( fieldConstructor1.originIndex, 5 );

  QgsFields::Field fieldConstructor2;
  QCOMPARE( fieldConstructor2.origin, QgsFields::OriginUnknown );
  QCOMPARE( fieldConstructor2.originIndex, -1 );

  //test equality operators
  QgsFields::Field field1( original, QgsFields::OriginJoin, 5 );
  QgsFields::Field field2( original, QgsFields::OriginJoin, 5 );
  QVERIFY( field1 == field2 );
  QgsFields::Field field3( original, QgsFields::OriginEdit, 5 );
  QVERIFY( field1 != field3 );
  QgsFields::Field field4( original, QgsFields::OriginJoin, 6 );
  QVERIFY( field1 != field4 );
}

QTEST_MAIN( TestQgsFields )
#include "testqgsfields.moc"
