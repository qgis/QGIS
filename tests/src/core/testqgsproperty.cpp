/***************************************************************************
                         testqgsproperty.cpp
                         -------------------
    begin                : April 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsproperty.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgscolorramp.h"
#include "qgssymbollayerutils.h"
#include <QObject>

enum PropertyKeys
{
  Property1,
  Property2,
  Property3,
  Property4,
};

class TestTransformer : public QgsPropertyTransformer
{
  public:

    TestTransformer( double minValue, double maxValue )
        : QgsPropertyTransformer( minValue, maxValue )
    {

    }

    virtual Type transformerType() const override { return SizeScaleTransformer; }
    virtual TestTransformer* clone() override
    {
      return new TestTransformer( mMinValue, mMaxValue );
    }

  private:

    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const override
    {
      Q_UNUSED( context );

      if ( value.isNull() )
        return -1.0;

      return value.toDouble() * 2;
    }

};


class TestQgsProperty : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void factory();
    void conversions(); //test QgsAbstractProperty static conversion methods
    void staticProperty(); //test for QgsStaticProperty
    void fieldBasedProperty(); //test for QgsFieldBasedProperty
    void expressionBasedProperty(); //test for QgsExpressionBasedProperty
    void propertyTransformer(); //test for QgsPropertyTransformer
    void sizeScaleTransformer(); //test for QgsSizeScaleTransformer
    void colorRampTransformer(); //test for QgsColorRampTransformer
    void propertyCollection(); //test for QgsPropertyCollection
    void collectionStack(); //test for QgsPropertyCollectionStack

  private:

    QMap< int, QString > mPropertyNameMap;

};

void TestQgsProperty::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  mPropertyNameMap.insert( Property1, "p1" );
  mPropertyNameMap.insert( Property2, "p2" );
  mPropertyNameMap.insert( Property3, "p3" );
  mPropertyNameMap.insert( Property4, "p4" );
}

void TestQgsProperty::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProperty::init()
{

}

void TestQgsProperty::cleanup()
{

}

void TestQgsProperty::factory()
{
  //test creating properties using factory method
  QgsAbstractProperty* prop = QgsAbstractProperty::create( QgsAbstractProperty::StaticProperty );
  QCOMPARE( prop->propertyType(), QgsAbstractProperty::StaticProperty );
  delete prop;
  prop = QgsAbstractProperty::create( QgsAbstractProperty::ExpressionBasedProperty );
  QCOMPARE( prop->propertyType(), QgsAbstractProperty::ExpressionBasedProperty );
  delete prop;
  prop = QgsAbstractProperty::create( QgsAbstractProperty::FieldBasedProperty );
  QCOMPARE( prop->propertyType(), QgsAbstractProperty::FieldBasedProperty );
  delete prop;
}

void TestQgsProperty::conversions()
{
  QgsExpressionContext context;

  //all these tests are done for both a property and a collection
  QgsPropertyCollection collection;

  //test color conversions

  //no color, should return defaultColor
  QgsStaticProperty* c1 = new QgsStaticProperty( QVariant(), true );
  collection.setProperty( 0, c1 );
  QCOMPARE( c1->valueAsColor( context , QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  QCOMPARE( collection.valueAsColor( 0, context , QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  c1->setStaticValue( QColor( 255, 200, 100, 50 ) ); //color in qvariant
  QCOMPARE( c1->valueAsColor( context , QColor( 200, 210, 220 ) ), QColor( 255, 200, 100, 50 ) );
  QCOMPARE( collection.valueAsColor( 0, context , QColor( 200, 210, 220 ) ), QColor( 255, 200, 100, 50 ) );
  c1->setStaticValue( QColor( ) );  //invalid color in qvariant, should return default color
  QCOMPARE( c1->valueAsColor( context , QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  QCOMPARE( collection.valueAsColor( 0, context , QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  c1->setStaticValue( QgsSymbolLayerUtils::encodeColor( QColor( 255, 200, 100, 50 ) ) ); //encoded color
  QCOMPARE( c1->valueAsColor( context , QColor( 200, 210, 220 ) ), QColor( 255, 200, 100, 50 ) );
  QCOMPARE( collection.valueAsColor( 0, context , QColor( 200, 210, 220 ) ), QColor( 255, 200, 100, 50 ) );
  c1->setStaticValue( "i am not a color" ); //badly encoded color, should return default color
  QCOMPARE( c1->valueAsColor( context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  QCOMPARE( collection.valueAsColor( 0, context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );

  // test double conversions
  QgsStaticProperty* d1 = new QgsStaticProperty( QVariant(), true );
  collection.setProperty( 1, d1 );
  QCOMPARE( d1->valueAsDouble( context , -1.2 ), -1.2 );
  QCOMPARE( collection.valueAsDouble( 1, context , -1.2 ), -1.2 );
  d1->setStaticValue( 12.3 ); //double in qvariant
  QCOMPARE( d1->valueAsDouble( context , -1.2 ), 12.3 );
  QCOMPARE( collection.valueAsDouble( 1, context , -1.2 ), 12.3 );
  d1->setStaticValue( "15.6" ); //double as string
  QCOMPARE( d1->valueAsDouble( context , -1.2 ), 15.6 );
  QCOMPARE( collection.valueAsDouble( 1, context , -1.2 ), 15.6 );
  d1->setStaticValue( "i am not a double" ); //not a double, should return default value
  QCOMPARE( d1->valueAsDouble( context, -1.2 ), -1.2 );
  QCOMPARE( collection.valueAsDouble( 1, context, -1.2 ), -1.2 );

  // test integer conversions
  QgsStaticProperty* i1 = new QgsStaticProperty( QVariant(), true );
  collection.setProperty( 2, i1 );
  QCOMPARE( i1->valueAsInt( context , -11 ), -11 );
  QCOMPARE( collection.valueAsInt( 2, context , -11 ), -11 );
  i1->setStaticValue( 13 ); //integer in qvariant
  QCOMPARE( i1->valueAsInt( context , -11 ), 13 );
  QCOMPARE( collection.valueAsInt( 2, context , -11 ), 13 );
  i1->setStaticValue( 13.9 ); //double in qvariant, should be rounded
  QCOMPARE( i1->valueAsInt( context , -11 ), 14 );
  QCOMPARE( collection.valueAsInt( 2, context , -11 ), 14 );
  i1->setStaticValue( "15" ); //integer as string
  QCOMPARE( i1->valueAsInt( context, -11 ), 15 );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), 15 );
  i1->setStaticValue( "15.9" ); //double as string, should be rounded
  QCOMPARE( i1->valueAsInt( context, -11 ), 16 );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), 16 );
  i1->setStaticValue( "i am not a int" ); //not a int, should return default value
  QCOMPARE( i1->valueAsInt( context, -11 ), -11 );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), -11 );
}

void TestQgsProperty::staticProperty()
{
  QgsExpressionContext context;
  QgsStaticProperty property( QString( "test" ), true );
  QCOMPARE( property.propertyType(), QgsAbstractProperty::StaticProperty );
  QVERIFY( property.isActive() );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  QCOMPARE( property.value( context, QString( "default" ) ).toString(), QString( "test" ) );
  property.setActive( false );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  QVERIFY( !property.isActive() );
  QCOMPARE( property.value( context, QString( "default" ) ).toString(), QString( "default" ) );
  property.setStaticValue( 5 );
  property.setActive( true );
  QCOMPARE( property.value( context ).toInt(), 5 );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QgsStaticProperty p1;
  p1.setActive( true );
  p1.setStaticValue( "test" );
  p1.setTransformer( new TestTransformer( 10, 20 ) );

  QDomElement element = doc.createElement( "prop" );
  p1.writeXML( element, doc );

  QgsStaticProperty r1;
  r1.readXML( element, doc );
  QVERIFY( r1.isActive() );
  QVERIFY( r1.transformer() );
  QCOMPARE( r1.staticValue(), QVariant( "test" ) );

  p1.setActive( false );
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QVERIFY( !r1.isActive() );

  //saving/restoring different types
  p1.setStaticValue( QVariant( 5 ) ); //int
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QCOMPARE( r1.staticValue(), p1.staticValue() );
  p1.setStaticValue( QVariant( 5.7 ) ); //double
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QCOMPARE( r1.staticValue(), p1.staticValue() );
  p1.setStaticValue( QVariant( true ) ); //bool
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QCOMPARE( r1.staticValue(), p1.staticValue() );
  p1.setStaticValue( QVariant( 5LL ) ); //longlong
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QCOMPARE( r1.staticValue(), p1.staticValue() );

  // test cloning a static property
  QgsStaticProperty p2;
  p2.setActive( true );
  p2.setStaticValue( "test" );
  p2.setTransformer( new TestTransformer( 10, 20 ) );
  QScopedPointer< QgsStaticProperty > p3( p2.clone() );
  QVERIFY( p3->isActive() );
  QCOMPARE( p3->staticValue().toString(), QString( "test" ) );
  QVERIFY( p3->transformer() );
  p2.setActive( false );
  p2.setStaticValue( 5.9 );
  p3.reset( p2.clone() );
  QVERIFY( !p3->isActive() );
  QCOMPARE( p3->staticValue().toDouble(), 5.9 );

  // copy constructor
  QgsStaticProperty p4( p2 );
  QVERIFY( !p4.isActive() );
  QCOMPARE( p4.staticValue().toDouble(), 5.9 );
  QVERIFY( p4.transformer() );
}

void TestQgsProperty::fieldBasedProperty()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( "field1", QVariant::Int ) );
  fields.append( QgsField( "field2", QVariant::Int ) );
  ft.setFields( fields );
  QgsAttributes attr;
  attr << QVariant( 5 ) << QVariant( 7 );
  ft.setAttributes( attr );
  ft.setValid( true );

  // throw it in an expression context
  QgsExpressionContext context;
  context.setFeature( ft );
  context.setFields( fields );

  QgsFieldBasedProperty property( QString( "field1" ), true );
  QCOMPARE( property.propertyType(), QgsAbstractProperty::FieldBasedProperty );
  QVERIFY( property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), 5 );
  QCOMPARE( property.referencedFields( context ), QSet< QString >() << "field1" );
  property.setActive( false );
  QVERIFY( !property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  property.setField( "field2" );
  property.setActive( true );
  QCOMPARE( property.value( context, -1 ).toInt(), 7 );
  QCOMPARE( property.referencedFields( context ), QSet< QString >() << "field2" );
  //bad field reference
  property.setField( "bad_field" );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  // unset field name
  QgsFieldBasedProperty defaultProperty;
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );
  defaultProperty.setActive( true );
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QgsFieldBasedProperty p1;
  p1.setActive( true );
  p1.setField( "test_field" );

  QDomElement element = doc.createElement( "prop" );
  QgsFieldBasedProperty r1;
  //try reading from an empty element
  r1.readXML( element, doc );
  QVERIFY( !r1.isActive() );
  QVERIFY( r1.field().isEmpty() );

  // now populate element and re-read
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QVERIFY( r1.isActive() );
  QCOMPARE( r1.field(), QString( "test_field" ) );

  p1.setActive( false );
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QVERIFY( !r1.isActive() );

  // test cloning a field based property
  QgsFieldBasedProperty p2;
  p2.setActive( true );
  p2.setField( "test" );
  p2.setTransformer( new TestTransformer( 10, 20 ) );
  QScopedPointer< QgsFieldBasedProperty > p3( p2.clone() );
  QVERIFY( p3->isActive() );
  QCOMPARE( p3->field(), QString( "test" ) );
  QVERIFY( p3->transformer() );
  p2.setActive( false );
  p3.reset( p2.clone() );
  QVERIFY( !p3->isActive() );
}

void TestQgsProperty::expressionBasedProperty()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( "field1", QVariant::Int ) );
  fields.append( QgsField( "field2", QVariant::Int ) );
  ft.setFields( fields );
  QgsAttributes attr;
  attr << QVariant( 5 ) << QVariant( 7 );
  ft.setAttributes( attr );
  ft.setValid( true );

  // throw it in an expression context
  QgsExpressionContext context;
  context.setFeature( ft );
  context.setFields( fields );

  QgsExpressionBasedProperty property( QString( "\"field1\" + \"field2\"" ), true );
  QCOMPARE( property.propertyType(), QgsAbstractProperty::ExpressionBasedProperty );
  QVERIFY( property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), 12 );
  QCOMPARE( property.referencedFields( context ).count(), 2 );
  QVERIFY( property.referencedFields( context ).contains( "field1" ) );
  QVERIFY( property.referencedFields( context ).contains( "field2" ) );
  property.setExpressionString( "\"field2\"*2" );
  QCOMPARE( property.value( context, -1 ).toInt(), 14 );
  QCOMPARE( property.referencedFields( context ), QSet< QString >() << "field2" );
  property.setActive( false );
  QVERIFY( !property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  property.setExpressionString( "'a'||'b'" );
  property.setActive( true );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  QCOMPARE( property.value( context, "bb" ).toString(), QString( "ab" ) );
  //bad expression
  property.setExpressionString( "bad_ 5" );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  // unset expression
  QgsExpressionBasedProperty defaultProperty;
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );
  defaultProperty.setActive( true );
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QgsExpressionBasedProperty p1;
  p1.setActive( true );
  p1.setExpressionString( "4+5" );

  QDomElement element = doc.createElement( "prop" );
  QgsExpressionBasedProperty r1;
  //try reading from an empty element
  r1.readXML( element, doc );
  QVERIFY( !r1.isActive() );
  QVERIFY( r1.expressionString().isEmpty() );
  QCOMPARE( r1.value( context, -1 ).toInt(), -1 );

  // now populate element and re-read
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QVERIFY( r1.isActive() );
  QCOMPARE( r1.expressionString(), QString( "4+5" ) );
  QCOMPARE( r1.value( context, -1 ).toInt(), 9 );

  p1.setActive( false );
  p1.writeXML( element, doc );
  r1.readXML( element, doc );
  QVERIFY( !r1.isActive() );
  QCOMPARE( r1.value( context, -1 ).toInt(), -1 );

  // test cloning an expression based property
  QgsExpressionBasedProperty p2;
  p2.setActive( true );
  p2.setExpressionString( "1+6" );

  QScopedPointer< QgsExpressionBasedProperty > p3( p2.clone() );
  QVERIFY( p3->isActive() );
  QCOMPARE( p3->expressionString(), QString( "1+6" ) );
  QCOMPARE( p3->value( context, -1 ).toInt(), 7 );
  p2.setActive( false );
  p3.reset( p2.clone() );
  QVERIFY( !p3->isActive() );
  QCOMPARE( p3->value( context, -1 ).toInt(), -1 );
  p2.setTransformer( new TestTransformer( 10, 20 ) );
  p3.reset( p2.clone() );
  QVERIFY( p3->transformer() );
}

void TestQgsProperty::propertyTransformer()
{
  QgsExpressionContext context;
  TestTransformer transform( -5, 5 );
  QCOMPARE( transform.minValue(), -5.0 );
  transform.setMinValue( -1 );
  QCOMPARE( transform.minValue(), -1.0 );
  QCOMPARE( transform.maxValue(), 5.0 );
  transform.setMaxValue( 10.0 );
  QCOMPARE( transform.maxValue(), 10.0 );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  TestTransformer t1( -5, 6 );
  QDomElement element = doc.createElement( "transform" );
  TestTransformer r1( -99, -98 );
  QVERIFY( t1.writeXML( element, doc ) );
  QVERIFY( r1.readXML( element, doc ) );
  QCOMPARE( r1.minValue(), -5.0 );
  QCOMPARE( r1.maxValue(), 6.0 );

  //install into property and test evaluation
  QgsStaticProperty p1;
  p1.setTransformer( new TestTransformer( 10, 20 ) );
  QCOMPARE( dynamic_cast< const TestTransformer* >( p1.transformer() )->minValue(), 10.0 );
  QCOMPARE( dynamic_cast< const TestTransformer* >( p1.transformer() )->maxValue(), 20.0 );
  p1.setStaticValue( QVariant( QVariant::Double ) );
  QCOMPARE( p1.value( context, -99 ).toDouble(), -1.0 );
  p1.setStaticValue( 11.0 );
  QCOMPARE( p1.value( context, -99 ).toDouble(), 22.0 );

  //test that transform is saved/restored with property
  QDomElement propElement = doc.createElement( "property" );
  QgsStaticProperty p2;
  QVERIFY( !p2.transformer() );
  QVERIFY( p1.writeXML( propElement, doc ) );
  QVERIFY( p2.readXML( propElement, doc ) );
  QVERIFY( p2.transformer() );
  QCOMPARE( p2.transformer()->minValue(), 10.0 );
  QCOMPARE( p2.transformer()->maxValue(), 20.0 );

  //test that transform is cloned with property
  QScopedPointer< QgsStaticProperty > p3( p1.clone() );
  QVERIFY( p3->transformer() );
  QCOMPARE( p3->transformer()->minValue(), 10.0 );
  QCOMPARE( p3->transformer()->maxValue(), 20.0 );

  //test that copy constructor copies transformer
  QgsStaticProperty p4( p1 );
  QVERIFY( p4.transformer() );
  QCOMPARE( p4.transformer()->minValue(), 10.0 );
  QCOMPARE( p4.transformer()->maxValue(), 20.0 );

  //test that assignment operator copies transformer
  QgsStaticProperty p5;
  p5 = p1;
  QVERIFY( p5.transformer() );
  QCOMPARE( p5.transformer()->minValue(), 10.0 );
  QCOMPARE( p5.transformer()->maxValue(), 20.0 );
}

void TestQgsProperty::sizeScaleTransformer()
{
  QgsExpressionContext context;
  QgsSizeScaleTransformer scale( QgsSizeScaleTransformer::Linear,
                                 10,
                                 20,
                                 100,
                                 200,
                                 -10,
                                 1.0 );
  QCOMPARE( scale.transformerType(), QgsPropertyTransformer::SizeScaleTransformer );
  QCOMPARE( scale.minValue(), 10.0 );
  QCOMPARE( scale.maxValue(), 20.0 );
  QCOMPARE( scale.minSize(), 100.0 );
  QCOMPARE( scale.maxSize(), 200.0 );
  QCOMPARE( scale.nullSize(), -10.0 );
  QCOMPARE( scale.exponent(), 1.0 );
  QCOMPARE( scale.type(), QgsSizeScaleTransformer::Linear );

  //transform
  QCOMPARE( scale.transform( context, 10 ).toInt(), 100 );
  QCOMPARE( scale.transform( context, 20 ).toInt(), 200 );
  //null value
  QCOMPARE( scale.transform( context, QVariant( QVariant::Double ) ).toInt(), -10 );
  //non numeric value
  QCOMPARE( scale.transform( context, QVariant( "ffff" ) ), QVariant( "ffff" ) );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QgsSizeScaleTransformer t1( QgsSizeScaleTransformer::Exponential,
                              15,
                              25,
                              150,
                              250,
                              -10,
                              99 );

  QDomElement element = doc.createElement( "xform" );
  QVERIFY( t1.writeXML( element, doc ) );
  QgsSizeScaleTransformer r1;
  QVERIFY( r1.readXML( element, doc ) );
  QCOMPARE( r1.minValue(), 15.0 );
  QCOMPARE( r1.maxValue(), 25.0 );
  QCOMPARE( r1.minSize(), 150.0 );
  QCOMPARE( r1.maxSize(), 250.0 );
  QCOMPARE( r1.nullSize(), -10.0 );
  QCOMPARE( r1.exponent(), 99.0 );
  QCOMPARE( r1.type(), QgsSizeScaleTransformer::Exponential );

  // test cloning
  QScopedPointer< QgsSizeScaleTransformer > r2( t1.clone() );
  QCOMPARE( r2->minValue(), 15.0 );
  QCOMPARE( r2->maxValue(), 25.0 );
  QCOMPARE( r2->minSize(), 150.0 );
  QCOMPARE( r2->maxSize(), 250.0 );
  QCOMPARE( r2->nullSize(), -10.0 );
  QCOMPARE( r2->exponent(), 99.0 );
  QCOMPARE( r2->type(), QgsSizeScaleTransformer::Exponential );

  //test various min/max value/size and scaling methods

  //getters and setters
  QgsSizeScaleTransformer t;
  t.setMinValue( 100 );
  QCOMPARE( t.minValue(), 100.0 );
  t.setMaxValue( 200 );
  QCOMPARE( t.maxValue(), 200.0 );
  t.setMinSize( 10.0 );
  QCOMPARE( t.minSize(), 10.0 );
  t.setMaxSize( 20.0 );
  QCOMPARE( t.maxSize(), 20.0 );
  t.setNullSize( 1 );
  QCOMPARE( t.nullSize(), 1.0 );
  t.setType( QgsSizeScaleTransformer::Area );
  QCOMPARE( t.type(), QgsSizeScaleTransformer::Area );
  t.setExponent( 2.5 );
  QCOMPARE( t.exponent(), 2.5 );

  //test that setting type updates exponent
  t.setType( QgsSizeScaleTransformer::Linear );
  QCOMPARE( t.exponent(), 1.0 );
  t.setType( QgsSizeScaleTransformer::Area );
  QCOMPARE( t.exponent(), 0.5 );
  t.setType( QgsSizeScaleTransformer::Flannery );
  QCOMPARE( t.exponent(), 0.57 );

  //test linear scaling
  t.setType( QgsSizeScaleTransformer::Linear );
  QCOMPARE( t.size( 100 ), 10.0 );
  QCOMPARE( t.size( 150 ), 15.0 );
  QCOMPARE( t.size( 200 ), 20.0 );
  //test area scaling
  t.setType( QgsSizeScaleTransformer::Area );
  QCOMPARE( t.size( 100 ), 10.0 );
  QVERIFY( qgsDoubleNear( t.size( 150 ), 17.0711, 0.001 ) );
  QCOMPARE( t.size( 200 ), 20.0 );
  //test flannery scaling
  t.setType( QgsSizeScaleTransformer::Flannery );
  QCOMPARE( t.size( 100 ), 10.0 );
  QVERIFY( qgsDoubleNear( t.size( 150 ), 16.7362, 0.001 ) );
  QCOMPARE( t.size( 200 ), 20.0 );
  //test exponential scaling
  t.setType( QgsSizeScaleTransformer::Exponential );
  t.setExponent( 1.5 );
  QCOMPARE( t.size( 100 ), 10.0 );
  QVERIFY( qgsDoubleNear( t.size( 150 ), 13.5355, 0.001 ) );
  QCOMPARE( t.size( 200 ), 20.0 );
}

void TestQgsProperty::colorRampTransformer()
{
  QgsExpressionContext context;
  QgsColorRampTransformer scale( 10,
                                 20,
                                 new QgsGradientColorRamp( QColor( 0, 0, 0 ), QColor( 255, 255, 255 ) ),
                                 QColor( 100, 150, 200 ) );
  QCOMPARE( scale.transformerType(), QgsPropertyTransformer::ColorRampTransformer );
  QCOMPARE( scale.minValue(), 10.0 );
  QCOMPARE( scale.maxValue(), 20.0 );
  QVERIFY( scale.colorRamp() );
  QCOMPARE( scale.nullColor(), QColor( 100, 150, 200 ) );

  //transform
  QCOMPARE( scale.transform( context, 10 ).value<QColor>(), QColor( 0, 0, 0 ) );
  QCOMPARE( scale.transform( context, 20 ).value<QColor>(), QColor( 255, 255, 255 ) );
  //null value
  QCOMPARE( scale.transform( context, QVariant( QVariant::Double ) ).value<QColor>(), QColor( 100, 150, 200 ) );
  //non numeric value
  QCOMPARE( scale.transform( context, QVariant( "ffff" ) ), QVariant( "ffff" ) );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QgsColorRampTransformer t1( 15,
                              25,
                              new QgsGradientColorRamp( QColor( 10, 20, 30 ), QColor( 200, 190, 180 ) ),
                              QColor( 100, 150, 200 ) );

  QDomElement element = doc.createElement( "xform" );
  QVERIFY( t1.writeXML( element, doc ) );
  QgsColorRampTransformer r1;
  QVERIFY( r1.readXML( element, doc ) );
  QCOMPARE( r1.minValue(), 15.0 );
  QCOMPARE( r1.maxValue(), 25.0 );
  QCOMPARE( r1.nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r1.colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r1.colorRamp() )->color2(), QColor( 200, 190, 180 ) );

  // test cloning
  QScopedPointer< QgsColorRampTransformer > r2( t1.clone() );
  QCOMPARE( r2->minValue(), 15.0 );
  QCOMPARE( r2->maxValue(), 25.0 );
  QCOMPARE( r2->nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r2->colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r2->colorRamp() )->color2(), QColor( 200, 190, 180 ) );

  // copy constructor
  QgsColorRampTransformer r3( t1 );
  QCOMPARE( r3.minValue(), 15.0 );
  QCOMPARE( r3.maxValue(), 25.0 );
  QCOMPARE( r3.nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r3.colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r3.colorRamp() )->color2(), QColor( 200, 190, 180 ) );

  // assignment operator
  QgsColorRampTransformer r4;
  r4 = t1;
  QCOMPARE( r4.minValue(), 15.0 );
  QCOMPARE( r4.maxValue(), 25.0 );
  QCOMPARE( r4.nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r4.colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( r4.colorRamp() )->color2(), QColor( 200, 190, 180 ) );

  //test various min/max value/color and scaling methods

  //getters and setters
  QgsColorRampTransformer t;
  t.setMinValue( 100 );
  QCOMPARE( t.minValue(), 100.0 );
  t.setMaxValue( 200 );
  QCOMPARE( t.maxValue(), 200.0 );
  t.setNullColor( QColor( 1, 10, 11, 21 ) );
  QCOMPARE( t.nullColor(), QColor( 1, 10, 11, 21 ) );
  t.setColorRamp( new QgsGradientColorRamp( QColor( 10, 20, 100 ), QColor( 100, 200, 200 ) ) );
  QCOMPARE( dynamic_cast< QgsGradientColorRamp* >( t.colorRamp() )->color1(), QColor( 10, 20, 100 ) );

  //test colors
  QCOMPARE( t.color( 50 ), QColor( 10, 20, 100 ) ); //out of range
  QCOMPARE( t.color( 100 ), QColor( 10, 20, 100 ) );
  QCOMPARE( t.color( 150 ), QColor( 55, 110, 150 ) );
  QCOMPARE( t.color( 200 ), QColor( 100, 200, 200 ) );
  QCOMPARE( t.color( 250 ), QColor( 100, 200, 200 ) ); //out of range
}

void TestQgsProperty::propertyCollection()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( "field1", QVariant::Int ) );
  fields.append( QgsField( "field2", QVariant::Int ) );
  ft.setFields( fields );
  QgsAttributes attr;
  attr << QVariant( 5 ) << QVariant( 7 );
  ft.setAttributes( attr );
  ft.setValid( true );

  // throw it in an expression context
  QgsExpressionContext context;
  context.setFeature( ft );
  context.setFields( fields );

  QgsPropertyCollection collection( "collection" );
  QCOMPARE( collection.name(), QString( "collection" ) );
  QVERIFY( !collection.hasProperty( Property1 ) );
  QVERIFY( collection.referencedFields( context ).isEmpty() );
  QCOMPARE( collection.count(), 0 );
  QCOMPARE( collection.propertyKeys(), QList< int >() );
  QVERIFY( !collection.hasActiveDynamicProperties() );
  QVERIFY( !collection.hasActiveProperties() );

  QgsStaticProperty* property = new QgsStaticProperty( "value", true );
  collection.setProperty( Property1, property );
  QVERIFY( collection.hasProperty( Property1 ) );
  QCOMPARE( collection.count(), 1 );
  QCOMPARE( collection.propertyKeys(), QList< int >() << Property1 );
  QCOMPARE( collection.property( Property1 )->value( context ), property->value( context ) );
  QCOMPARE( collection.value( Property1, context ), property->value( context ) );
  QVERIFY( collection.isActive( Property1 ) );
  QVERIFY( collection.hasActiveProperties() );
  QVERIFY( !collection.hasActiveDynamicProperties() );

  //test bad property
  QVERIFY( !collection.property( Property2 ) );
  QVERIFY( !collection.value( Property2, context ).isValid() );
  QCOMPARE( collection.value( Property2, context, QString( "default" ) ).toString(), QString( "default" ) );
  QVERIFY( !collection.isActive( Property2 ) );

  //test replacing property
  QgsStaticProperty* property2 = new QgsStaticProperty( "value2", true );
  collection.setProperty( Property1, property2 );
  QCOMPARE( collection.count(), 1 );
  QCOMPARE( collection.propertyKeys(), QList< int >() << Property1 );
  QCOMPARE( collection.property( Property1 )->value( context ), property2->value( context ) );
  QVERIFY( collection.hasActiveProperties() );
  QVERIFY( !collection.hasActiveDynamicProperties() );

  //implicit conversion
  collection.setProperty( Property3, 5 );
  QCOMPARE( collection.property( Property3 )->value( context ).toInt(), 5 );
  QVERIFY( collection.property( Property3 )->isActive() );
  QCOMPARE( collection.count(), 2 );
  QCOMPARE( collection.propertyKeys().toSet(), QSet<int>() << Property1 << Property3 );

  //test removing a property
  collection.setProperty( Property1, nullptr );
  QVERIFY( !collection.property( Property1 ) );
  QVERIFY( !collection.hasProperty( Property1 ) );
  QCOMPARE( collection.propertyKeys(), QList<int>() << Property3 );

  //clear
  collection.clear();
  QCOMPARE( collection.count(), 0 );
  QCOMPARE( collection.propertyKeys(), QList<int>() );
  QVERIFY( !collection.hasActiveProperties() );
  QVERIFY( !collection.hasActiveDynamicProperties() );

  collection.setProperty( Property1, new QgsStaticProperty( "v1", true ) );
  collection.setProperty( Property2, new QgsStaticProperty( "v2", false ) );
  collection.setProperty( Property3, new QgsFieldBasedProperty( "field1", true ) );
  collection.setProperty( Property4, new QgsExpressionBasedProperty( "\"field1\" + \"field2\"", true ) );

  // test referenced fields
  QCOMPARE( collection.referencedFields( context ).count(), 2 );
  QVERIFY( collection.referencedFields( context ).contains( "field1" ) );
  QVERIFY( collection.referencedFields( context ).contains( "field2" ) );

  //saving and restoring

  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );
  QDomElement element = doc.createElement( "collection" );
  collection.writeXML( element, doc, mPropertyNameMap );

  QgsPropertyCollection restoredCollection;
  restoredCollection.readXML( element, doc, mPropertyNameMap );
  QCOMPARE( restoredCollection.name(), QString( "collection" ) );
  QCOMPARE( restoredCollection.count(), 4 );
  QCOMPARE( restoredCollection.property( Property1 )->propertyType(), QgsAbstractProperty::StaticProperty );
  QVERIFY( restoredCollection.property( Property1 )->isActive() );
  QCOMPARE( static_cast< QgsStaticProperty* >( restoredCollection.property( Property1 ) )->staticValue(), QVariant( "v1" ) );
  QCOMPARE( restoredCollection.property( Property2 )->propertyType(), QgsAbstractProperty::StaticProperty );
  QVERIFY( !restoredCollection.property( Property2 )->isActive() );
  QCOMPARE( static_cast< QgsStaticProperty* >( restoredCollection.property( Property2 ) )->staticValue(), QVariant( "v2" ) );
  QCOMPARE( restoredCollection.property( Property3 )->propertyType(), QgsAbstractProperty::FieldBasedProperty );
  QVERIFY( restoredCollection.property( Property3 )->isActive() );
  QCOMPARE( static_cast< QgsFieldBasedProperty* >( restoredCollection.property( Property3 ) )->field(), QString( "field1" ) );
  QCOMPARE( restoredCollection.property( Property4 )->propertyType(), QgsAbstractProperty::ExpressionBasedProperty );
  QVERIFY( restoredCollection.property( Property4 )->isActive() );
  QCOMPARE( static_cast< QgsExpressionBasedProperty* >( restoredCollection.property( Property4 ) )->expressionString(), QString( "\"field1\" + \"field2\"" ) );
  QVERIFY( restoredCollection.hasActiveProperties() );
  QVERIFY( restoredCollection.hasActiveDynamicProperties() );

  // copy constructor
  QgsPropertyCollection collection2( collection );
  QCOMPARE( collection2.name(), QString( "collection" ) );
  QCOMPARE( collection2.count(), 4 );
  QCOMPARE( collection2.property( Property1 )->propertyType(), QgsAbstractProperty::StaticProperty );
  QVERIFY( collection2.property( Property1 )->isActive() );
  QCOMPARE( static_cast< QgsStaticProperty* >( collection2.property( Property1 ) )->staticValue(), QVariant( "v1" ) );
  QCOMPARE( collection2.property( Property2 )->propertyType(), QgsAbstractProperty::StaticProperty );
  QVERIFY( !collection2.property( Property2 )->isActive() );
  QCOMPARE( static_cast< QgsStaticProperty* >( collection2.property( Property2 ) )->staticValue(), QVariant( "v2" ) );
  QCOMPARE( collection2.property( Property3 )->propertyType(), QgsAbstractProperty::FieldBasedProperty );
  QVERIFY( collection2.property( Property3 )->isActive() );
  QCOMPARE( static_cast< QgsFieldBasedProperty* >( collection2.property( Property3 ) )->field(), QString( "field1" ) );
  QCOMPARE( collection2.property( Property4 )->propertyType(), QgsAbstractProperty::ExpressionBasedProperty );
  QVERIFY( collection2.property( Property4 )->isActive() );
  QCOMPARE( static_cast< QgsExpressionBasedProperty* >( collection2.property( Property4 ) )->expressionString(), QString( "\"field1\" + \"field2\"" ) );
  QVERIFY( collection2.hasActiveProperties() );
  QVERIFY( collection2.hasActiveDynamicProperties() );

  // assignment operator
  QgsPropertyCollection collection3;
  collection3.setProperty( Property1, new QgsStaticProperty( "aaaa", false ) );
  collection3 = collection;
  QCOMPARE( collection3.name(), QString( "collection" ) );
  QCOMPARE( collection3.count(), 4 );
  QCOMPARE( collection3.property( Property1 )->propertyType(), QgsAbstractProperty::StaticProperty );
  QVERIFY( collection3.property( Property1 )->isActive() );
  QCOMPARE( static_cast< QgsStaticProperty* >( collection3.property( Property1 ) )->staticValue(), QVariant( "v1" ) );
  QCOMPARE( collection3.property( Property2 )->propertyType(), QgsAbstractProperty::StaticProperty );
  QVERIFY( !collection3.property( Property2 )->isActive() );
  QCOMPARE( static_cast< QgsStaticProperty* >( collection3.property( Property2 ) )->staticValue(), QVariant( "v2" ) );
  QCOMPARE( collection3.property( Property3 )->propertyType(), QgsAbstractProperty::FieldBasedProperty );
  QVERIFY( collection3.property( Property3 )->isActive() );
  QCOMPARE( static_cast< QgsFieldBasedProperty* >( collection3.property( Property3 ) )->field(), QString( "field1" ) );
  QCOMPARE( collection3.property( Property4 )->propertyType(), QgsAbstractProperty::ExpressionBasedProperty );
  QVERIFY( collection3.property( Property4 )->isActive() );
  QCOMPARE( static_cast< QgsExpressionBasedProperty* >( collection3.property( Property4 ) )->expressionString(), QString( "\"field1\" + \"field2\"" ) );
  QVERIFY( collection3.hasActiveProperties() );
  QVERIFY( collection3.hasActiveDynamicProperties() );

  //test hasActiveProperties() and hasActiveDynamicProperties()
  collection3.property( Property1 )->setActive( false );
  collection3.property( Property2 )->setActive( false );
  collection3.property( Property3 )->setActive( false );
  collection3.property( Property4 )->setActive( false );
  QVERIFY( !collection3.hasActiveProperties() );
  QVERIFY( !collection3.hasActiveDynamicProperties() );
  collection3.property( Property4 )->setActive( true );
  QVERIFY( collection3.hasActiveDynamicProperties() );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.property( Property4 )->setActive( false );
  collection3.property( Property2 )->setActive( true );
  QVERIFY( !collection3.hasActiveDynamicProperties() );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.property( Property2 )->setActive( false );
  QVERIFY( !collection3.hasActiveProperties() );
  collection3.setProperty( Property1, "5" );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.setProperty( Property1, new QgsStaticProperty( "6", true ) );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.setProperty( Property1, new QgsStaticProperty( "7", false ) );
  QVERIFY( !collection3.hasActiveProperties() );
  collection3.setProperty( Property3, QVariant( "val" ) );
  QVERIFY( collection3.hasActiveProperties() );
}

void TestQgsProperty::collectionStack()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( "field1", QVariant::Int ) );
  fields.append( QgsField( "field2", QVariant::Int ) );
  ft.setFields( fields );
  QgsAttributes attr;
  attr << QVariant( 5 ) << QVariant( 7 );
  ft.setAttributes( attr );
  ft.setValid( true );

  // throw it in an expression context
  QgsExpressionContext context;
  context.setFeature( ft );
  context.setFields( fields );

  QgsPropertyCollectionStack stack;
  //test retrieving from empty stack
  QVERIFY( !stack.property( Property1 ) );
  QVERIFY( !stack.at( 0 ) );
  QVERIFY( !const_cast< const QgsPropertyCollectionStack* >( &stack )->at( 0 ) );
  QVERIFY( !stack.collection( "nothing" ) );
  QVERIFY( !stack.value( Property1, context ).isValid() );
  QCOMPARE( stack.value( Property1, context, "default" ).toString(), QString( "default" ) );
  QVERIFY( !stack.hasActiveProperty( Property1 ) );
  QVERIFY( stack.referencedFields( context ).isEmpty() );
  QCOMPARE( stack.count(), 0 );
  QVERIFY( !stack.hasActiveDynamicProperties() );
  QVERIFY( !stack.hasActiveProperties() );

  //add a collection to the stack
  QgsPropertyCollection* collection = new QgsPropertyCollection( "collection" );
  stack.appendCollection( collection );
  QCOMPARE( stack.count(), 1 );
  QCOMPARE( stack.at( 0 ), collection );
  QCOMPARE( const_cast< const QgsPropertyCollectionStack* >( &stack )->at( 0 ), collection );
  QVERIFY( !stack.collection( "nothing" ) );
  QCOMPARE( stack.collection( "collection" ), collection );
  QVERIFY( !stack.property( Property1 ) );
  QVERIFY( !stack.value( Property1, context ).isValid() );
  QCOMPARE( stack.value( Property1, context, "default" ).toString(), QString( "default" ) );
  QVERIFY( !stack.hasActiveProperty( Property1 ) );
  QVERIFY( !stack.hasActiveDynamicProperties() );
  QVERIFY( !stack.hasActiveProperties() );
  QVERIFY( stack.referencedFields( context ).isEmpty() );

  //now add a property to the collection
  QgsStaticProperty* property = new QgsStaticProperty( "value", true );
  collection->setProperty( Property1, property );
  QVERIFY( stack.hasActiveProperty( Property1 ) );
  QCOMPARE( stack.property( Property1 )->value( context ), property->value( context ) );
  QCOMPARE( stack.value( Property1, context ), property->value( context ) );
  QVERIFY( !stack.hasActiveDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );
  QVERIFY( !stack.hasActiveProperty( Property2 ) );
  collection->setProperty( Property2, new QgsStaticProperty( "value1", true ) );
  QVERIFY( stack.hasActiveProperty( Property2 ) );
  QVERIFY( !stack.hasActiveDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );

  //add a second collection
  QgsPropertyCollection* collection2 = new QgsPropertyCollection( "collection2" );
  stack.appendCollection( collection2 );
  QCOMPARE( stack.count(), 2 );
  QCOMPARE( stack.at( 1 ), collection2 );
  QCOMPARE( const_cast< const QgsPropertyCollectionStack* >( &stack )->at( 1 ), collection2 );
  QCOMPARE( stack.collection( "collection2" ), collection2 );
  QVERIFY( !stack.hasActiveDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );
  QgsStaticProperty* property2 = new QgsStaticProperty( "value2", true );
  collection2->setProperty( Property2, property2 );
  QVERIFY( stack.hasActiveProperty( Property2 ) );
  QCOMPARE( stack.property( Property2 )->value( context ), property2->value( context ) );
  QCOMPARE( stack.value( Property2, context ), property2->value( context ) );
  QVERIFY( !stack.hasActiveDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );

  //test adding active property later in the stack
  QgsStaticProperty* property3 = new QgsStaticProperty( "value3", true );
  collection2->setProperty( Property1, property3 );
  QVERIFY( stack.hasActiveProperty( Property1 ) );
  QCOMPARE( stack.property( Property1 )->value( context, "default" ), property3->value( context ) );
  QCOMPARE( stack.value( Property1, context ), property3->value( context ) );
  property3->setActive( false );
  QCOMPARE( stack.value( Property1, context ), property->value( context ) );

  //test overriding a property
  QgsStaticProperty* property4 = new QgsStaticProperty( "value4", true );
  collection2->setProperty( Property2, property4 );
  QVERIFY( stack.hasActiveProperty( Property2 ) );
  QCOMPARE( stack.property( Property2 )->value( context ), property4->value( context ) );
  QCOMPARE( stack.value( Property2, context ), property4->value( context ) );
  property4->setActive( false );
  QCOMPARE( stack.property( Property2 )->value( context ), QVariant( "value1" ) );
  QCOMPARE( stack.value( Property2, context ), QVariant( "value1" ) );

  //clearing
  stack.clear();
  QCOMPARE( stack.count(), 0 );
  QVERIFY( !stack.hasActiveDynamicProperties() );
  QVERIFY( !stack.hasActiveProperties() );

  // test copying a stack
  QgsPropertyCollectionStack stack2;
  stack2.appendCollection( new QgsPropertyCollection( "collection1" ) );
  stack2.at( 0 )->setProperty( Property1, "val1" );
  stack2.at( 0 )->setProperty( Property2, "val2" );
  stack2.appendCollection( new QgsPropertyCollection( "collection2" ) );
  stack2.at( 1 )->setProperty( Property3, "val3" );
  //copy constructor
  QgsPropertyCollectionStack stack3( stack2 );
  QCOMPARE( stack3.count(), 2 );
  QCOMPARE( stack3.at( 0 )->name(), QString( "collection1" ) );
  QCOMPARE( stack3.at( 1 )->name(), QString( "collection2" ) );
  QCOMPARE( static_cast< QgsStaticProperty* >( stack3.at( 0 )->property( Property1 ) )->staticValue(), QVariant( "val1" ) );
  QCOMPARE( static_cast< QgsStaticProperty* >( stack3.at( 0 )->property( Property2 ) )->staticValue(), QVariant( "val2" ) );
  QCOMPARE( static_cast< QgsStaticProperty* >( stack3.at( 1 )->property( Property3 ) )->staticValue(), QVariant( "val3" ) );
  QVERIFY( !stack3.hasActiveDynamicProperties() );
  QVERIFY( stack3.hasActiveProperties() );
  //assignment operator
  stack3.clear();
  stack3.appendCollection( new QgsPropertyCollection( "temp" ) );
  stack3 = stack2;
  QCOMPARE( stack3.count(), 2 );
  QCOMPARE( stack3.at( 0 )->name(), QString( "collection1" ) );
  QCOMPARE( stack3.at( 1 )->name(), QString( "collection2" ) );
  QCOMPARE( static_cast< QgsStaticProperty* >( stack3.at( 0 )->property( Property1 ) )->staticValue(), QVariant( "val1" ) );
  QCOMPARE( static_cast< QgsStaticProperty* >( stack3.at( 0 )->property( Property2 ) )->staticValue(), QVariant( "val2" ) );
  QCOMPARE( static_cast< QgsStaticProperty* >( stack3.at( 1 )->property( Property3 ) )->staticValue(), QVariant( "val3" ) );
  QVERIFY( !stack3.hasActiveDynamicProperties() );
  QVERIFY( stack3.hasActiveProperties() );

  //check hasActiveDynamicProperties() and hasActiveProperties()
  QgsPropertyCollectionStack stack4;
  stack4.appendCollection( new QgsPropertyCollection( "collection1" ) );
  stack4.at( 0 )->setProperty( Property1, "val1" );
  QVERIFY( !stack4.hasActiveDynamicProperties() );
  QVERIFY( stack4.hasActiveProperties() );
  stack4.at( 0 )->property( Property1 )->setActive( false );
  QVERIFY( !stack4.hasActiveProperties() );
  stack4.at( 0 )->setProperty( Property1, "6" );
  QVERIFY( stack4.hasActiveProperties() );
  stack4.at( 0 )->setProperty( Property2, new QgsExpressionBasedProperty( "\"field1\" + \"field2\"", true ) );
  QVERIFY( stack4.hasActiveProperties() );
  QVERIFY( stack4.hasActiveDynamicProperties() );
  QCOMPARE( stack4.referencedFields( context ), QSet< QString>() << "field1" << "field2" );
  stack4.at( 0 )->property( Property1 )->setActive( false );
  QVERIFY( stack4.hasActiveProperties() );
  QVERIFY( stack4.hasActiveDynamicProperties() );
  stack4.at( 0 )->property( Property2 )->setActive( false );
  QVERIFY( !stack4.hasActiveProperties() );
  QVERIFY( !stack4.hasActiveDynamicProperties() );
}

QGSTEST_MAIN( TestQgsProperty )
#include "testqgsproperty.moc"
