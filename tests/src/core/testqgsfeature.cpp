/***************************************************************************
     testqgsfeature.cpp
     ------------------
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

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgssymbol.h"
#include "qgslinesymbol.h"
#include "qgsunsetattributevalue.h"

class TestQgsFeature: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void attributesTest(); //test QgsAttributes
    void constructorTest(); //test default constructors
    void attributesToMap();
    void unsetAttributes();
    void create();//test creating a feature
    void copy();// test cpy destruction (double delete)
    void assignment();
    void gettersSetters(); //test getters and setters
    void attributes();
    void geometry();
    void asVariant(); //test conversion to and from a QVariant
    void fields();
    void equality();
    void attributeUsingField();
    void dataStream();


  private:

    QgsFields mFields;
    QgsAttributes mAttrs;
    QgsGeometry mGeometry;
    QgsGeometry mGeometry2;
};

void TestQgsFeature::initTestCase()
{
  //add fields
  const QgsField field( QStringLiteral( "field1" ) );
  mFields.append( field );
  const QgsField field2( QStringLiteral( "field2" ) );
  mFields.append( field2 );
  const QgsField field3( QStringLiteral( "field3" ) );
  mFields.append( field3 );

  //test attributes
  mAttrs << QVariant( 5 ) << QVariant( 7 ) << QVariant( "val" );

  mGeometry = QgsGeometry::fromWkt( QStringLiteral( "MULTILINESTRING((0 0, 10 0, 10 10, 20 10),(30 30, 40 30, 40 40, 50 40))" ) );
  mGeometry2 = QgsGeometry::fromWkt( QStringLiteral( "MULTILINESTRING((0 5, 15 0, 15 10, 25 10))" ) );
}

void TestQgsFeature::cleanupTestCase()
{

}

void TestQgsFeature::init()
{

}

void TestQgsFeature::cleanup()
{

}

void TestQgsFeature::attributesTest()
{
  QgsAttributes attr1;
  attr1 << QVariant( 5 ) << QVariant( 7 ) << QVariant( "val" );
  QgsAttributes attr2;
  attr2 << QVariant( 5 ) << QVariant( 7 ) << QVariant( "val" );
  QCOMPARE( attr1, attr2 );

  //different size
  QgsAttributes attr3;
  attr3 << QVariant( 5 ) << QVariant( 7 );
  QVERIFY( attr1 != attr3 );

  //different value
  QgsAttributes attr4;
  attr4 << QVariant( 5 ) << QVariant( 10 ) << QVariant( "val" );
  QVERIFY( attr1 != attr4 );

  //null value
  const QVariant nullDouble( QVariant::Double );
  QgsAttributes attr5;
  attr5 << QVariant( 5 ) << nullDouble << QVariant( "val" );
  QVERIFY( attr1 != attr5 );

  //null compared to 0 (see note at QgsAttributes::operator== )
  QgsAttributes attr6;
  attr6 << QVariant( 5 ) << QVariant( 0.0 ) << QVariant( "val" );
  QVERIFY( attr5 != attr6 );

  //constructed with size
  const QgsAttributes attr7( 5 );
  QCOMPARE( attr7.size(), 5 );

  // datetime vs string -- default qt qvariant equality test considers these equal? ?? ?!
  QgsAttributes attrWithString;
  attrWithString << QVariant( 5 ) << QVariant( QStringLiteral( "val" ) ) << QVariant( QStringLiteral( "2021-02-10 00:00" ) );
  QgsAttributes attrWithDate;
  attrWithDate << QVariant( 5 ) << QVariant( QStringLiteral( "val" ) ) << QVariant( QDateTime( QDate( 2021, 2, 10 ), QTime( 0, 0 ) ) );
  QVERIFY( attrWithString != attrWithDate );
  QVERIFY( attrWithDate != attrWithString );
}

void TestQgsFeature::constructorTest()
{
  const QgsFeature f;
  QVERIFY( f.approximateMemoryUsage() > 0 );
  QVERIFY( FID_IS_NULL( f.id() ) );
  const QgsFeature f2 { QgsFields() };
  QVERIFY( FID_IS_NULL( f2.id() ) );
  const QgsFeature f3 { 1234 };
  QVERIFY( ! FID_IS_NULL( f3.id() ) );
  const QgsFeature f4 { QgsFields(), 1234 };
  QVERIFY( ! FID_IS_NULL( f4.id() ) );
}

void TestQgsFeature::attributesToMap()
{
  QgsAttributes attr1;
  attr1 << QVariant( 5 ) << QVariant() << QVariant( "val" );
  const QgsAttributeMap map1 = attr1.toMap();

  QCOMPARE( map1.count(), 2 );
  QCOMPARE( map1.value( 0 ), QVariant( 5 ) );
  QCOMPARE( map1.value( 2 ), QVariant( "val" ) );

  QgsAttributes attr2;
  attr2 << QVariant() << QVariant( 5 ) << QVariant();
  const QgsAttributeMap map2 = attr2.toMap();

  QCOMPARE( map2.count(), 1 );
  QCOMPARE( map2.value( 1 ), QVariant( 5 ) );

  const QgsAttributes attr3;
  const QgsAttributeMap map3 = attr3.toMap();
  QVERIFY( map3.isEmpty() );
}

void TestQgsFeature::unsetAttributes()
{
  QgsAttributes attr1;
  attr1 << QVariant( 5 ) << QVariant() << QVariant( "val" ) << QVariant( QgsUnsetAttributeValue() )  << QVariant( QgsUnsetAttributeValue() );

  QVERIFY( !attr1.isUnsetValue( 0 ) );
  QVERIFY( !attr1.isUnsetValue( 1 ) );
  QVERIFY( !attr1.isUnsetValue( 2 ) );
  QVERIFY( attr1.isUnsetValue( 3 ) );
  QVERIFY( attr1.isUnsetValue( 4 ) );
}

void TestQgsFeature::create()
{
  //test constructors

  const QgsFeature featureFromId( 1000LL );
  QCOMPARE( featureFromId.id(), 1000LL );
  QCOMPARE( featureFromId.isValid(), false );
  QVERIFY( featureFromId.attributes().isEmpty() );

  const QgsFeature featureFromFieldsId( mFields, 1001LL );
  QCOMPARE( featureFromFieldsId.id(), 1001LL );
  QCOMPARE( featureFromFieldsId.fields(), mFields );
  QCOMPARE( featureFromFieldsId.isValid(), false );
  //should be 3 invalid attributes
  QCOMPARE( featureFromFieldsId.attributes().count(), 3 );
  for ( const QVariant &a : featureFromFieldsId.attributes() )
  {
    QVERIFY( !a.isValid() );
  }
}

void TestQgsFeature::copy()
{
  QgsFeature original( 1000LL );
  original.setAttributes( mAttrs );
  original.setEmbeddedSymbol( QgsLineSymbol::createSimple( QVariantMap() ) );

  QgsFeature copy( original );
  QVERIFY( copy.id() == original.id() );
  QCOMPARE( copy.attributes(), original.attributes() );
  QVERIFY( copy.embeddedSymbol() );
  // should be shallow copy
  QCOMPARE( copy.embeddedSymbol(), original.embeddedSymbol() );

  copy.setId( 1001LL );
  QCOMPARE( original.id(), 1000LL );
  QVERIFY( copy.id() != original.id() );

  // symbol should be cloned on detach
  QVERIFY( copy.embeddedSymbol() );
  QVERIFY( copy.embeddedSymbol() != original.embeddedSymbol() );
}

void TestQgsFeature::assignment()
{
  QgsFeature original( 1000LL );
  original.setAttributes( mAttrs );
  original.setEmbeddedSymbol( QgsLineSymbol::createSimple( QVariantMap() ) );

  QgsFeature copy;
  copy = original;
  QCOMPARE( copy.id(), original.id() );
  QCOMPARE( copy.attributes(), original.attributes() );
  QVERIFY( copy.embeddedSymbol() );
  // should be shallow copy
  QCOMPARE( copy.embeddedSymbol(), original.embeddedSymbol() );

  copy.setId( 1001LL );
  QCOMPARE( original.id(), 1000LL );
  QVERIFY( copy.id() != original.id() );
  QCOMPARE( copy.attributes(), original.attributes() );

  // symbol should be cloned on detach
  QVERIFY( copy.embeddedSymbol() );
  QVERIFY( copy.embeddedSymbol() != original.embeddedSymbol() );
}

void TestQgsFeature::gettersSetters()
{
  QgsFeature feature;
  feature.setId( 1000LL );
  QCOMPARE( feature.id(), 1000LL );

  feature.setValid( true );
  QCOMPARE( feature.isValid(), true );

  QVERIFY( !feature.embeddedSymbol() );
  feature.setEmbeddedSymbol( QgsLineSymbol::createSimple( QVariantMap() ) );
  QVERIFY( feature.embeddedSymbol() );
}

void TestQgsFeature::attributes()
{
  QgsFeature feature;
  feature.setAttributes( mAttrs );
  QCOMPARE( feature.attributes(), mAttrs );
  QCOMPARE( feature.attributes(), mAttrs );
  QVERIFY( feature.approximateMemoryUsage() > QgsFeature().approximateMemoryUsage() );

  //test implicit sharing detachment
  QgsFeature copy( feature );
  QCOMPARE( copy.attributes(), feature.attributes() );
  copy.setAttributes( QgsAttributes() );
  QVERIFY( copy.attributes().isEmpty() );
  QCOMPARE( feature.attributes(), mAttrs );

  //always start with a copy so that we can test implicit sharing detachment is working
  copy = feature;
  QCOMPARE( copy.attributes(), mAttrs );
  QgsAttributes newAttrs;
  newAttrs << QVariant( 1 ) << QVariant( 3 );
  copy.setAttributes( newAttrs );
  QCOMPARE( copy.attributes(), newAttrs );
  QCOMPARE( feature.attributes(), mAttrs );

  //test setAttribute
  copy = feature;
  QCOMPARE( copy.attributes(), mAttrs );
  //invalid indexes
  QVERIFY( !copy.setAttribute( -1, 5 ) );
  QCOMPARE( copy.attributes(), mAttrs );
  QVERIFY( !copy.setAttribute( 10, 5 ) );
  QCOMPARE( copy.attributes(), mAttrs );
  //valid index
  QVERIFY( copy.setAttribute( 1, 15 ) );
  QCOMPARE( copy.attributes().at( 1 ).toInt(), 15 );
  QCOMPARE( feature.attributes(), mAttrs );

  //test attribute by index
  QVERIFY( !feature.attribute( -1 ).isValid() );
  QVERIFY( !feature.attribute( 15 ).isValid() );
  QCOMPARE( feature.attribute( 1 ), mAttrs.at( 1 ) );

  //test initAttributes
  copy = feature;
  QCOMPARE( copy.attributes(), mAttrs );
  copy.initAttributes( 5 );
  QCOMPARE( copy.attributes().count(), 5 );
  for ( const QVariant &a : copy.attributes() )
  {
    QVERIFY( !a.isValid() );
  }
  QCOMPARE( feature.attributes(), mAttrs );

  //test deleteAttribute
  copy = feature;
  QCOMPARE( copy.attributes(), mAttrs );
  copy.deleteAttribute( 1 );
  QCOMPARE( copy.attributes().count(), 2 );
  QCOMPARE( copy.attributes().at( 0 ).toInt(), 5 );
  QCOMPARE( copy.attributes().at( 1 ).toString(), QString( "val" ) );
  QCOMPARE( feature.attributes(), mAttrs );
}

void TestQgsFeature::geometry()
{
  QgsFeature feature;
  QVERIFY( !feature.hasGeometry() );

  //test no double delete of geometry when setting:
  feature.setGeometry( QgsGeometry( mGeometry2 ) );
  QVERIFY( feature.hasGeometry() );
  QVERIFY( feature.approximateMemoryUsage() > QgsFeature().approximateMemoryUsage() );
  feature.setGeometry( QgsGeometry( mGeometry ) );
  QCOMPARE( feature.geometry().asWkb(), mGeometry.asWkb() );

  //test implicit sharing detachment
  QgsFeature copy( feature );
  QCOMPARE( copy.geometry().asWkb(), feature.geometry().asWkb() );
  copy.clearGeometry();
  QVERIFY( ! copy.hasGeometry() );
  QCOMPARE( feature.geometry().asWkb(), mGeometry.asWkb() );

  //test no crash when setting an empty geometry and triggering a detach
  QgsFeature emptyGeomFeature;
  emptyGeomFeature.setGeometry( QgsGeometry() );
  QVERIFY( !emptyGeomFeature.hasGeometry() );
  copy = emptyGeomFeature;
  copy.setId( 5 ); //force detach

  //setGeometry
  //always start with a copy so that we can test implicit sharing detachment is working
  copy = feature;
  QCOMPARE( copy.geometry().asWkb(), mGeometry.asWkb() );
  copy.setGeometry( QgsGeometry( mGeometry2 ) );
  QCOMPARE( copy.geometry().asWkb(), mGeometry2.asWkb() );
  QCOMPARE( feature.geometry().asWkb(), mGeometry.asWkb() );

  //setGeometry using reference
  copy = feature;
  QCOMPARE( copy.geometry().asWkb(), mGeometry.asWkb() );
  const QgsGeometry geomByRef( mGeometry2 );
  copy.setGeometry( geomByRef );
  QCOMPARE( copy.geometry().asWkb(), geomByRef.asWkb() );
  QCOMPARE( feature.geometry().asWkb(), mGeometry.asWkb() );

  //setGeometry using abstract geom
  copy = feature;
  QCOMPARE( copy.geometry().asWkb(), mGeometry.asWkb() );
  copy.setGeometry( std::make_unique< QgsPoint >( 5, 6 ) );
  QCOMPARE( copy.geometry().asWkt(), QStringLiteral( "Point (5 6)" ) );
  QCOMPARE( feature.geometry().asWkb(), mGeometry.asWkb() );

  //clearGeometry
  QgsFeature geomFeature;
  geomFeature.setGeometry( QgsGeometry( mGeometry2 ) );
  QVERIFY( geomFeature.hasGeometry() );
  geomFeature.clearGeometry();
  QVERIFY( !geomFeature.hasGeometry() );
  QVERIFY( geomFeature.geometry().isNull() );
}

void TestQgsFeature::asVariant()
{
  const QgsFeature original( mFields, 1001LL );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  const QgsFeature fromVar = qvariant_cast<QgsFeature>( var );
  //QCOMPARE( fromVar, original );
  QCOMPARE( fromVar.id(), original.id() );
  QCOMPARE( fromVar.fields(), original.fields() );
}

void TestQgsFeature::fields()
{
  QgsFeature original;
  QVERIFY( original.fields().isEmpty() );
  original.setFields( mFields );
  QCOMPARE( original.fields(), mFields );
  QVERIFY( original.approximateMemoryUsage() > QgsFeature().approximateMemoryUsage() );

  QgsFeature copy( original );
  QCOMPARE( copy.fields(), original.fields() );

  //test detach
  QgsFields newFields( mFields );
  newFields.remove( 2 );
  copy.setFields( newFields );
  QCOMPARE( copy.fields(), newFields );
  QCOMPARE( original.fields(), mFields );

  //test that no init leaves attributes
  copy = original;
  copy.initAttributes( 3 );
  copy.setAttribute( 0, 1 );
  copy.setAttribute( 1, 2 );
  copy.setAttribute( 2, 3 );
  copy.setFields( mFields, false );
  QCOMPARE( copy.fields(), mFields );
  //should be 3 invalid attributes
  QCOMPARE( copy.attributes().count(), 3 );
  QCOMPARE( copy.attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( copy.attributes().at( 1 ).toInt(), 2 );
  QCOMPARE( copy.attributes().at( 2 ).toInt(), 3 );

  //test setting fields with init
  copy = original;
  copy.initAttributes( 3 );
  copy.setAttribute( 0, 1 );
  copy.setAttribute( 1, 2 );
  copy.setAttribute( 2, 3 );
  copy.setFields( mFields, true );
  QCOMPARE( copy.fields(), mFields );
  //should be 3 invalid attributes
  QCOMPARE( copy.attributes().count(), 3 );
  for ( const QVariant &a : copy.attributes() )
  {
    QVERIFY( !a.isValid() );
  }

  //test fieldNameIndex
  copy = original;
  copy.setFields( mFields );
  QCOMPARE( copy.fieldNameIndex( "bad" ), -1 );
  QCOMPARE( copy.fieldNameIndex( "field1" ), 0 );
  QCOMPARE( copy.fieldNameIndex( "field2" ), 1 );
}

void TestQgsFeature::equality()
{
  QgsFeature feature;
  feature.setFields( mFields, true );
  feature.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature.setAttribute( 2, QStringLiteral( "attr3" ) );
  feature.setValid( true );
  feature.setId( 1 );
  feature.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );

  QgsFeature feature2 = feature;
  QVERIFY( feature == feature2 );

  feature2.setAttribute( 0, "attr1" );
  QVERIFY( feature == feature2 );

  feature2.setAttribute( 1, 1 );
  QVERIFY( feature != feature2 );
  feature2.setAttribute( 1, QStringLiteral( "attr2" ) );
  QVERIFY( feature == feature2 );
  feature2.setEmbeddedSymbol( QgsLineSymbol::createSimple( QVariantMap() ) );
  QVERIFY( feature != feature2 );
  feature.setEmbeddedSymbol( QgsLineSymbol::createSimple( QVariantMap() ) );
  QVERIFY( feature != feature2 );
  feature2.setEmbeddedSymbol( nullptr );
  QVERIFY( feature != feature2 );
  feature.setEmbeddedSymbol( nullptr );
  QVERIFY( feature == feature2 );

  QgsFeature feature3;
  feature3.setFields( mFields, true );
  feature3.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature3.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature3.setAttribute( 2, QStringLiteral( "attr3" ) );
  feature3.setValid( true );
  feature3.setId( 1 );
  feature3.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QVERIFY( feature == feature3 );

  QgsFeature feature4;
  feature4.setFields( mFields, true );
  feature4.setAttribute( 0, 1 );
  feature4.setAttribute( 1, 2 );
  feature4.setAttribute( 2, 3 );
  feature4.setValid( true );
  feature4.setId( 1 );
  feature4.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QVERIFY( feature != feature4 );

  QgsFeature feature5;
  feature5.setFields( mFields, true );
  feature5.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature5.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature5.setAttribute( 2, QStringLiteral( "attr3" ) );
  feature5.setId( 1 );
  feature5.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  feature5.setValid( false );

  QVERIFY( feature != feature5 );

  QgsFeature feature6;
  feature6.setFields( mFields, true );
  feature6.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature6.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature6.setAttribute( 2, QStringLiteral( "attr3" ) );
  feature6.setValid( true );
  feature6.setId( 2 );
  feature6.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );

  QVERIFY( feature != feature6 );

  QgsFeature feature7;
  feature7.setFields( mFields, true );
  feature7.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature7.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature7.setAttribute( 2, QStringLiteral( "attr3" ) );
  feature7.setValid( true );
  feature7.setId( 1 );
  feature7.setGeometry( QgsGeometry( new QgsPoint( 1, 3 ) ) );

  QVERIFY( feature != feature7 );

  // features without geometry
  QgsFeature feature8;
  feature8.setFields( mFields, true );
  feature8.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature8.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature8.setAttribute( 2, QStringLiteral( "attr3" ) );
  feature8.setValid( true );
  feature8.setId( 1 );
  QgsFeature feature9;
  feature9.setFields( mFields, true );
  feature9.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature9.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature9.setAttribute( 2, QStringLiteral( "attr3" ) );
  feature9.setValid( true );
  feature9.setId( 1 );
  QVERIFY( feature8 == feature9 );
  feature8.setGeometry( QgsGeometry( new QgsPoint( 1, 3 ) ) );
  QVERIFY( feature8 != feature9 );
  feature8.clearGeometry();
  feature9.setGeometry( QgsGeometry( new QgsPoint( 1, 3 ) ) );
  QVERIFY( feature8 != feature9 );
  feature8.setGeometry( QgsGeometry( new QgsPoint( 1, 3 ) ) );
  QVERIFY( feature8 == feature9 );
}

void TestQgsFeature::attributeUsingField()
{
  QgsFeature feature;
  feature.setFields( mFields, true );
  feature.setAttribute( 0, QStringLiteral( "attr1" ) );
  feature.setAttribute( 1, QStringLiteral( "attr2" ) );
  feature.setAttribute( 2, QStringLiteral( "attr3" ) );

  QVERIFY( !feature.attribute( "bad" ).isValid() );
  QCOMPARE( feature.attribute( "field1" ).toString(), QString( "attr1" ) );
  QCOMPARE( feature.attribute( "field2" ).toString(), QString( "attr2" ) );

  //setAttribute using field name
  QVERIFY( !feature.setAttribute( "bad", QString( "test" ) ) );
  QgsFeature copy( feature );
  QVERIFY( copy.setAttribute( "field1", QString( "test" ) ) );
  QCOMPARE( copy.attribute( "field1" ).toString(), QString( "test" ) );
  //test that copy has been detached
  QCOMPARE( feature.attribute( "field1" ).toString(), QString( "attr1" ) );

  //deleteAttribute by name
  copy = feature;
  QVERIFY( !copy.deleteAttribute( "bad" ) );
  QVERIFY( copy.deleteAttribute( "field1" ) );
  QVERIFY( !copy.attribute( "field1" ).isValid() );
  QVERIFY( feature.attribute( "field1" ).isValid() );
}

void TestQgsFeature::dataStream()
{
  QgsFeature originalFeature;
  originalFeature.setGeometry( QgsGeometry( mGeometry ) );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << originalFeature;

  QgsFeature resultFeature;
  ds.device()->seek( 0 );
  ds >> resultFeature;

  QCOMPARE( resultFeature.id(), originalFeature.id() );
  QCOMPARE( resultFeature.attributes(), originalFeature.attributes() );
  QCOMPARE( resultFeature.geometry().asWkb(), originalFeature.geometry().asWkb() );
  QCOMPARE( resultFeature.isValid(), originalFeature.isValid() );

  //also test with feature empty geometry
  originalFeature.setGeometry( QgsGeometry() );
  QByteArray ba2;
  QDataStream ds2( &ba2, QIODevice::ReadWrite );
  ds2 << originalFeature;

  ds2.device()->seek( 0 );
  ds2 >> resultFeature;

  QCOMPARE( resultFeature.id(), originalFeature.id() );
  QCOMPARE( resultFeature.attributes(), originalFeature.attributes() );
  QVERIFY( !resultFeature.hasGeometry() );
  QCOMPARE( resultFeature.isValid(), originalFeature.isValid() );
}

QGSTEST_MAIN( TestQgsFeature )
#include "testqgsfeature.moc"
