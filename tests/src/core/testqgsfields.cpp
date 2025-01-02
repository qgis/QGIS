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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include "qgsfields.h"

class TestQgsFields : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void create();          //test creating a data defined container
    void copy();            // test cpy destruction (double delete)
    void assignment();
    void equality();  //test equality operators
    void asVariant(); //test conversion to and from a QVariant
    void construct();
    void clear();
    void exists();
    void count();
    void isEmpty();
    void remove();
    void rename();
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
    void qforeach();
    void iterator();
    void constIterator();
    void appendList();
    void appendQgsFields();

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
  const QgsFields fields;
  QCOMPARE( fields.count(), 0 );
}

void TestQgsFields::copy()
{
  QgsFields original;
  //add field
  const QgsField field( QStringLiteral( "testfield" ) );
  original.append( field );
  QCOMPARE( original.count(), 1 );
  QgsFields copy( original );
  QCOMPARE( copy.count(), 1 );
  QVERIFY( copy == original );

  const QgsField copyfield( QStringLiteral( "copyfield" ) );
  copy.append( copyfield );
  QCOMPARE( copy.count(), 2 );
  QCOMPARE( original.count(), 1 );
  QVERIFY( copy != original );
}

void TestQgsFields::assignment()
{
  QgsFields original;
  //add field
  const QgsField field( QStringLiteral( "testfield" ) );
  original.append( field );

  QgsFields copy;
  copy = original;
  QVERIFY( copy == original );

  const QgsField copyfield( QStringLiteral( "copyfield" ) );
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
  field1.setName( QStringLiteral( "name" ) );
  QgsField field2;
  field2.setName( QStringLiteral( "name" ) );
  QCOMPARE( field1, field2 );
  fields1.append( field1 );
  fields2.append( field2 );
  QVERIFY( fields1 == fields2 );
  QVERIFY( !( fields1 != fields2 ) );

  //make a change and retest
  const QgsField field3;
  fields2.append( field3 );
  QVERIFY( !( fields1 == fields2 ) );
  QVERIFY( fields1 != fields2 );
}

void TestQgsFields::asVariant()
{
  QgsField field1;
  field1.setName( QStringLiteral( "name" ) );
  QgsField field2;
  field2.setName( QStringLiteral( "name" ) );
  QgsFields original;
  original.append( field1 );
  original.append( field2 );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  const QgsFields fromVar = qvariant_cast<QgsFields>( var );
  QCOMPARE( fromVar, original );
}

void TestQgsFields::construct()
{
  // construct using a list of fields
  QgsFields fields(
    QList<QgsField> {
      QgsField( QStringLiteral( "field1" ), QMetaType::Type::QString ),
      QgsField( QStringLiteral( "field2" ), QMetaType::Type::Int ),
      QgsField( QStringLiteral( "field3" ), QMetaType::Type::Double ),
    }
  );

  QCOMPARE( fields.size(), 3 );
  QCOMPARE( fields.at( 0 ).name(), QStringLiteral( "field1" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field1" ) ), 0 );
  QCOMPARE( fields.at( 1 ).name(), QStringLiteral( "field2" ) );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field2" ) ), 1 );
  QCOMPARE( fields.at( 2 ).name(), QStringLiteral( "field3" ) );
  QCOMPARE( fields.at( 2 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field3" ) ), 2 );
}

void TestQgsFields::clear()
{
  QgsFields original;
  const QgsField field( QStringLiteral( "testfield" ) );
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
  const QgsField field( QStringLiteral( "testfield" ) );
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

  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.size(), 1 );

  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.size(), 2 );
}

void TestQgsFields::isEmpty()
{
  QgsFields fields;
  QVERIFY( fields.isEmpty() );

  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  QVERIFY( !fields.isEmpty() );
}

void TestQgsFields::remove()
{
  QgsFields fields;

  //test for no crash
  fields.remove( 1 );

  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
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

void TestQgsFields::rename()
{
  QgsFields fields;

  QVERIFY( !fields.rename( 1, "name" ) );

  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  QVERIFY( !fields.rename( 0, "" ) );

  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );
  QVERIFY( !fields.rename( 0, "testfield2" ) );

  QVERIFY( fields.rename( 0, "newname" ) );
  QCOMPARE( fields.at( 0 ).name(), QString( "newname" ) );
  QCOMPARE( fields.at( 1 ).name(), QString( "testfield2" ) );
}

void TestQgsFields::extend()
{
  QgsFields destination;
  const QgsField field( QStringLiteral( "testfield" ) );
  destination.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  destination.append( field2 );

  QgsFields source;
  const QgsField field3( QStringLiteral( "testfield3" ) );
  source.append( field3, Qgis::FieldOrigin::Join, 5 );
  const QgsField field4( QStringLiteral( "testfield4" ) );
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
  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );

  QCOMPARE( fields[0], field );
  QCOMPARE( fields[1], field2 );

  const QgsFields &constFields = fields;
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
  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );

  QCOMPARE( fields.field( "testfield" ), field );
  QCOMPARE( fields.field( "testfield2" ), field2 );
}

void TestQgsFields::fieldOrigin()
{
  QgsFields fields;
  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field, Qgis::FieldOrigin::Join );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2, Qgis::FieldOrigin::Expression );

  QCOMPARE( fields.fieldOrigin( 0 ), Qgis::FieldOrigin::Join );
  QCOMPARE( fields.fieldOrigin( 1 ), Qgis::FieldOrigin::Expression );
  QCOMPARE( fields.fieldOrigin( 2 ), Qgis::FieldOrigin::Unknown );
}

void TestQgsFields::fieldOriginIndex()
{
  QgsFields fields;
  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field, Qgis::FieldOrigin::Provider, 5 );
  QCOMPARE( fields.fieldOriginIndex( 0 ), 5 );

  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2, Qgis::FieldOrigin::Provider, 10 );
  QCOMPARE( fields.fieldOriginIndex( 1 ), 10 );

  const QgsField field3( QStringLiteral( "testfield3" ) );
  //field origin index not specified with OriginProvider, should be automatic
  fields.append( field3, Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.fieldOriginIndex( 2 ), 2 );

  const QgsField field4( QStringLiteral( "testfield4" ) );
  //field origin index not specified with other than OriginProvider, should remain -1
  fields.append( field4, Qgis::FieldOrigin::Edit );
  QCOMPARE( fields.fieldOriginIndex( 3 ), -1 );
}

void TestQgsFields::indexFromName()
{
  QgsFields fields;
  QgsField field( QStringLiteral( "testfield" ) );
  field.setAlias( QStringLiteral( "testfieldAlias" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );
  QgsField field3( QStringLiteral( "testfield3" ) );
  field3.setAlias( QString() );
  fields.append( field3 );
  QCOMPARE( fields.lookupField( QString() ), -1 );

  const QgsField field4 = QgsField( QString() );
  fields.append( field4 );
  QCOMPARE( fields.lookupField( QString() ), 3 );

  QCOMPARE( fields.indexFromName( QString( "bad" ) ), -1 );
  QCOMPARE( fields.lookupField( QString( "bad" ) ), -1 );
  QCOMPARE( fields.indexFromName( QString( "testfield" ) ), 0 );
  QCOMPARE( fields.lookupField( QString( "testfield" ) ), 0 );
  QCOMPARE( fields.indexFromName( QString( "testfield3" ) ), 2 );
  QCOMPARE( fields.lookupField( QString( "testfield3" ) ), 2 );

  //indexFromName is case sensitive, fieldNameIndex isn't
  QCOMPARE( fields.indexFromName( QString( "teStFiEld2" ) ), -1 );
  QCOMPARE( fields.lookupField( QString( "teStFiEld2" ) ), 1 );

  //test that fieldNameIndex prefers exact case matches over case insensitive matches
  const QgsField sameNameDifferentCase( QStringLiteral( "teStFielD" ) ); //#spellok
  fields.append( sameNameDifferentCase );
  QCOMPARE( fields.lookupField( QString( "teStFielD" ) ), 4 ); //#spellok

  //test that the alias is only matched with fieldNameIndex
  QCOMPARE( fields.indexFromName( "testfieldAlias" ), -1 );
  QCOMPARE( fields.lookupField( "testfieldAlias" ), 0 );
  QCOMPARE( fields.lookupField( "testfieldalias" ), 0 );
}

void TestQgsFields::toList()
{
  QgsFields fields;
  QList<QgsField> list = fields.toList();
  QVERIFY( list.isEmpty() );

  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );
  const QgsField field3( QStringLiteral( "testfield3" ) );
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

  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );
  const QgsField field3( QStringLiteral( "testfield3" ) );
  fields.append( field3 );

  attrList = fields.allAttributesList();
  QCOMPARE( attrList.at( 0 ), 0 );
  QCOMPARE( attrList.at( 1 ), 1 );
  QCOMPARE( attrList.at( 2 ), 2 );
}

void TestQgsFields::appendExpressionField()
{
  QgsFields fields;
  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "testfield2" ) );
  fields.append( field2 );

  const QgsField dupeName( QStringLiteral( "testfield" ) );
  QVERIFY( !fields.appendExpressionField( dupeName, 1 ) );

  //good name
  const QgsField exprField( QStringLiteral( "expression" ) );
  QVERIFY( fields.appendExpressionField( exprField, 5 ) );
  QCOMPARE( fields.count(), 3 );
  QCOMPARE( fields.fieldOrigin( 2 ), Qgis::FieldOrigin::Expression );
  QCOMPARE( fields.fieldOriginIndex( 2 ), 5 );
}

void TestQgsFields::dataStream()
{
  QgsField original1;
  original1.setName( QStringLiteral( "name" ) );
  original1.setType( QMetaType::Type::Int );
  original1.setLength( 5 );
  original1.setPrecision( 2 );
  original1.setTypeName( QStringLiteral( "typename1" ) );
  original1.setComment( QStringLiteral( "comment1" ) );

  QgsField original2;
  original2.setName( QStringLiteral( "next name" ) );
  original2.setType( QMetaType::Type::Double );
  original2.setLength( 15 );
  original2.setPrecision( 3 );
  original2.setTypeName( QStringLiteral( "double" ) );
  original2.setComment( QStringLiteral( "comment for field 2" ) );

  QgsFields originalFields;
  originalFields.append( original1 );
  originalFields.append( original2 );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << originalFields;

  QgsFields resultFields;
  ds.device()->seek( 0 );
  ds >> resultFields;

  QCOMPARE( resultFields, originalFields );
  QCOMPARE( resultFields.field( 0 ).typeName(), originalFields.field( 0 ).typeName() ); //typename is NOT required for equality
  QCOMPARE( resultFields.field( 0 ).comment(), originalFields.field( 0 ).comment() );   //comment is NOT required for equality
  QCOMPARE( resultFields.field( 1 ).typeName(), originalFields.field( 1 ).typeName() );
  QCOMPARE( resultFields.field( 1 ).comment(), originalFields.field( 1 ).comment() );
}

void TestQgsFields::field()
{
  QgsField original;
  original.setName( QStringLiteral( "name" ) );
  original.setType( QMetaType::Type::Int );
  original.setLength( 5 );
  original.setPrecision( 2 );

  //test constructors for QgsFields::Field
  const QgsFields::Field fieldConstructor1( original, Qgis::FieldOrigin::Join, 5 );
  QCOMPARE( fieldConstructor1.field, original );
  QCOMPARE( fieldConstructor1.origin, Qgis::FieldOrigin::Join );
  QCOMPARE( fieldConstructor1.originIndex, 5 );

  const QgsFields::Field fieldConstructor2;
  QCOMPARE( fieldConstructor2.origin, Qgis::FieldOrigin::Unknown );
  QCOMPARE( fieldConstructor2.originIndex, -1 );

  //test equality operators
  const QgsFields::Field field1( original, Qgis::FieldOrigin::Join, 5 );
  const QgsFields::Field field2( original, Qgis::FieldOrigin::Join, 5 );
  QVERIFY( field1 == field2 );
  const QgsFields::Field field3( original, Qgis::FieldOrigin::Edit, 5 );
  QVERIFY( field1 != field3 );
  const QgsFields::Field field4( original, Qgis::FieldOrigin::Join, 6 );
  QVERIFY( field1 != field4 );
}

void TestQgsFields::qforeach()
{
  QgsFields fields;
  const QgsField field( QStringLiteral( "1" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "2" ) );
  fields.append( field2 );

  int i = 0;
  for ( const QgsField &field : fields )
  {
    QCOMPARE( field, fields.at( i ) );
    ++i;
  }
}

void TestQgsFields::iterator()
{
  QgsFields fields;

  //test with empty fields
  QCOMPARE( fields.begin(), fields.end() );

  const QgsField field( QStringLiteral( "1" ) );
  fields.append( field );
  const QgsField field2( QStringLiteral( "2" ) );
  fields.append( field2 );

  QgsFields::iterator it = fields.begin();

  QCOMPARE( it->name(), QString( "1" ) );
  QCOMPARE( ( ++it )->name(), QString( "2" ) );
  QCOMPARE( ( --it )->name(), QString( "1" ) );
  QCOMPARE( ( it++ )->name(), QString( "1" ) );
  QCOMPARE( it->name(), QString( "2" ) );
  it->setName( QStringLiteral( "Test" ) );
  QCOMPARE( ( it-- )->name(), QString( "Test" ) );
  QCOMPARE( it->name(), QString( "1" ) );
  QCOMPARE( it[1].name(), QString( "Test" ) );
  it += 2;
  QCOMPARE( it, fields.end() );
  it -= 2;
  QCOMPARE( it->name(), QString( "1" ) );
  QgsFields::iterator it2( it );
  QVERIFY( it <= it2 );
  QVERIFY( it2 >= it );
  ++it2;
  QVERIFY( it < it2 );
  QVERIFY( it <= it2 );
  QVERIFY( it2 > it );
  QVERIFY( it2 >= it );
  QCOMPARE( it2, it + 1 );
  QCOMPARE( it, it2 - 1 );
  QCOMPARE( it2 - it, 1 );
}


void TestQgsFields::constIterator()
{
  QgsFields fields;

  //test with empty fields
  QCOMPARE( fields.constBegin(), fields.constEnd() );
  QCOMPARE( const_cast<const QgsFields *>( &fields )->begin(), const_cast<const QgsFields *>( &fields )->end() );
  for ( const QgsField &f : fields )
  {
    Q_UNUSED( f );
    //should not be called!
    QVERIFY( false );
  }

  const QgsField field( QString( QStringLiteral( "1" ) ) );
  fields.append( field );
  const QgsField field2( QString( QStringLiteral( "2" ) ) );
  fields.append( field2 );

  const QgsFields constFields( fields );

  QgsFields::const_iterator it = constFields.begin();

  QCOMPARE( it->name(), QString( "1" ) );
  QCOMPARE( ( ++it )->name(), QString( "2" ) );
  QCOMPARE( ( --it )->name(), QString( "1" ) );
  QCOMPARE( ( it++ )->name(), QString( "1" ) );
  QCOMPARE( it->name(), QString( "2" ) );
  QCOMPARE( ( it-- )->name(), QString( "2" ) );
  QCOMPARE( it->name(), QString( "1" ) );
  QCOMPARE( it[1].name(), QString( "2" ) );
  it += 2;
  QCOMPARE( it, constFields.end() );

  QgsFields::const_iterator it2 = fields.constBegin();

  QCOMPARE( it2->name(), QString( "1" ) );
  QCOMPARE( ( ++it2 )->name(), QString( "2" ) );
  QCOMPARE( ( --it2 )->name(), QString( "1" ) );
  QCOMPARE( ( it2++ )->name(), QString( "1" ) );
  QCOMPARE( it2->name(), QString( "2" ) );
  QCOMPARE( ( it2-- )->name(), QString( "2" ) );
  QCOMPARE( it2->name(), QString( "1" ) );
  QCOMPARE( it2[1].name(), QString( "2" ) );
  it2 += 2;
  QCOMPARE( it2, fields.constEnd() );

  QgsFields::const_iterator it3( it );
  QVERIFY( it <= it3 );
  QVERIFY( it3 >= it );
  ++it3;
  QVERIFY( it < it3 );
  QVERIFY( it <= it3 );
  QVERIFY( it3 > it );
  QVERIFY( it3 >= it );
  QCOMPARE( it3, it + 1 );
  QCOMPARE( it, it3 - 1 );
  QCOMPARE( it3 - it, 1 );
}

void TestQgsFields::appendList()
{
  // test appending a list of fields
  QgsFields fields;

  QVERIFY( fields.append(
    QList<QgsField> {
      QgsField( QStringLiteral( "field1" ), QMetaType::Type::QString ),
      QgsField( QStringLiteral( "field2" ), QMetaType::Type::Int ),
      QgsField( QStringLiteral( "field3" ), QMetaType::Type::Double ),
    }
  ) );

  QCOMPARE( fields.size(), 3 );
  QCOMPARE( fields.at( 0 ).name(), QStringLiteral( "field1" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.fieldOrigin( 0 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field1" ) ), 0 );
  QCOMPARE( fields.at( 1 ).name(), QStringLiteral( "field2" ) );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.fieldOrigin( 1 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field2" ) ), 1 );
  QCOMPARE( fields.at( 2 ).name(), QStringLiteral( "field3" ) );
  QCOMPARE( fields.at( 2 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.fieldOrigin( 2 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field3" ) ), 2 );

  // should be rejected, duplicate field name
  QVERIFY( !fields.append(
    QList<QgsField> {
      QgsField( QStringLiteral( "field1" ), QMetaType::Type::QString )
    }
  ) );

  QCOMPARE( fields.size(), 3 );

  QVERIFY( fields.append(
    QList<QgsField> {
      QgsField( QStringLiteral( "field4" ), QMetaType::Type::QString ),
      QgsField( QStringLiteral( "field5" ), QMetaType::Type::Int ),
      QgsField( QStringLiteral( "field6" ), QMetaType::Type::Double ),
    },
    Qgis::FieldOrigin::Join
  ) );

  QCOMPARE( fields.size(), 6 );
  QCOMPARE( fields.at( 0 ).name(), QStringLiteral( "field1" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.fieldOrigin( 0 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field1" ) ), 0 );
  QCOMPARE( fields.at( 1 ).name(), QStringLiteral( "field2" ) );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.fieldOrigin( 1 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field2" ) ), 1 );
  QCOMPARE( fields.at( 2 ).name(), QStringLiteral( "field3" ) );
  QCOMPARE( fields.at( 2 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.fieldOrigin( 2 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field3" ) ), 2 );
  QCOMPARE( fields.at( 3 ).name(), QStringLiteral( "field4" ) );
  QCOMPARE( fields.at( 3 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.fieldOrigin( 3 ), Qgis::FieldOrigin::Join );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field4" ) ), 3 );
  QCOMPARE( fields.at( 4 ).name(), QStringLiteral( "field5" ) );
  QCOMPARE( fields.at( 4 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.fieldOrigin( 4 ), Qgis::FieldOrigin::Join );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field5" ) ), 4 );
  QCOMPARE( fields.at( 5 ).name(), QStringLiteral( "field6" ) );
  QCOMPARE( fields.at( 5 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.fieldOrigin( 5 ), Qgis::FieldOrigin::Join );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field6" ) ), 5 );
}

void TestQgsFields::appendQgsFields()
{
  // test appending fields from QgsFields
  QgsFields fields;

  QgsFields fields2;
  fields2.append( QgsField( QStringLiteral( "field1" ), QMetaType::Type::QString ), Qgis::FieldOrigin::Edit, 2 );
  fields2.append( QgsField( QStringLiteral( "field2" ), QMetaType::Type::Int ), Qgis::FieldOrigin::Join, 4 );
  fields2.append( QgsField( QStringLiteral( "field3" ), QMetaType::Type::Double ), Qgis::FieldOrigin::Provider, 6 );

  QVERIFY( fields.append( fields2 ) );
  QCOMPARE( fields.size(), 3 );
  QCOMPARE( fields.at( 0 ).name(), QStringLiteral( "field1" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.fieldOrigin( 0 ), Qgis::FieldOrigin::Edit );
  QCOMPARE( fields.fieldOriginIndex( 0 ), 2 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field1" ) ), 0 );
  QCOMPARE( fields.at( 1 ).name(), QStringLiteral( "field2" ) );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.fieldOrigin( 1 ), Qgis::FieldOrigin::Join );
  QCOMPARE( fields.fieldOriginIndex( 1 ), 4 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field2" ) ), 1 );
  QCOMPARE( fields.at( 2 ).name(), QStringLiteral( "field3" ) );
  QCOMPARE( fields.at( 2 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.fieldOrigin( 2 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.fieldOriginIndex( 2 ), 6 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field3" ) ), 2 );

  // should be rejected, duplicate field name
  QVERIFY( !fields.append( fields2 ) );
  QCOMPARE( fields.size(), 3 );

  QgsFields fields3;
  fields3.append( QgsField( QStringLiteral( "field4" ), QMetaType::Type::QString ), Qgis::FieldOrigin::Expression, 3 );
  fields3.append( QgsField( QStringLiteral( "field5" ), QMetaType::Type::Int ), Qgis::FieldOrigin::Join, 5 );
  fields3.append( QgsField( QStringLiteral( "field6" ), QMetaType::Type::Double ), Qgis::FieldOrigin::Provider, 7 );

  QVERIFY( fields.append( fields3 ) );

  QCOMPARE( fields.size(), 6 );
  QCOMPARE( fields.at( 0 ).name(), QStringLiteral( "field1" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.fieldOrigin( 0 ), Qgis::FieldOrigin::Edit );
  QCOMPARE( fields.fieldOriginIndex( 0 ), 2 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field1" ) ), 0 );
  QCOMPARE( fields.at( 1 ).name(), QStringLiteral( "field2" ) );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.fieldOrigin( 1 ), Qgis::FieldOrigin::Join );
  QCOMPARE( fields.fieldOriginIndex( 1 ), 4 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field2" ) ), 1 );
  QCOMPARE( fields.at( 2 ).name(), QStringLiteral( "field3" ) );
  QCOMPARE( fields.at( 2 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.fieldOrigin( 2 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.fieldOriginIndex( 2 ), 6 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field3" ) ), 2 );
  QCOMPARE( fields.at( 3 ).name(), QStringLiteral( "field4" ) );
  QCOMPARE( fields.at( 3 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.fieldOrigin( 3 ), Qgis::FieldOrigin::Expression );
  QCOMPARE( fields.fieldOriginIndex( 3 ), 3 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field4" ) ), 3 );
  QCOMPARE( fields.at( 4 ).name(), QStringLiteral( "field5" ) );
  QCOMPARE( fields.at( 4 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.fieldOrigin( 4 ), Qgis::FieldOrigin::Join );
  QCOMPARE( fields.fieldOriginIndex( 4 ), 5 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field5" ) ), 4 );
  QCOMPARE( fields.at( 5 ).name(), QStringLiteral( "field6" ) );
  QCOMPARE( fields.at( 5 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.fieldOrigin( 5 ), Qgis::FieldOrigin::Provider );
  QCOMPARE( fields.fieldOriginIndex( 5 ), 7 );
  QCOMPARE( fields.indexFromName( QStringLiteral( "field6" ) ), 5 );
}

QGSTEST_MAIN( TestQgsFields )
#include "testqgsfields.moc"
