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
#include "qgspropertycollection.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgscolorrampimpl.h"
#include "qgssymbollayerutils.h"
#include "qgspropertytransformer.h"
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

    Type transformerType() const override { return SizeScaleTransformer; }
    TestTransformer *clone() const override
    {
      return new TestTransformer( mMinValue, mMaxValue );
    }
    QString toExpression( const QString & ) const override { return QString(); }

  private:

    QVariant transform( const QgsExpressionContext &context, const QVariant &value ) const override
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
    void conversions(); //test QgsProperty static conversion methods
    void invalid(); //test invalid properties
    void staticProperty(); //test for QgsStaticProperty
    void fieldBasedProperty(); //test for QgsFieldBasedProperty
    void expressionBasedProperty(); //test for QgsExpressionBasedProperty
    void equality();
    void isStaticValueInContext();
    void propertyTransformer(); //test for QgsPropertyTransformer
    void propertyTransformerFromExpression(); // text converting expression into QgsPropertyTransformer
    void genericNumericTransformer();
    void genericNumericTransformerFromExpression(); // text converting expression to QgsGenericNumericTransformer
    void sizeScaleTransformer(); //test for QgsSizeScaleTransformer
    void sizeScaleTransformerFromExpression(); // text converting expression to QgsSizeScaleTransformer
    void colorRampTransformer(); //test for QgsColorRampTransformer
    void propertyToTransformer(); //test converting expression based property to transformer/expression pair
    void asExpression(); //test converting property to expression
    void propertyCollection(); //test for QgsPropertyCollection
    void collectionStack(); //test for QgsPropertyCollectionStack
    void curveTransform();
    void asVariant();
    void isProjectColor();
    void referencedFieldsIgnoreContext();
    void mapToMap();

  private:

    QgsPropertiesDefinition mDefinitions;
    void checkCurveResult( const QList< QgsPointXY > &controlPoints, const QVector<double> &x, const QVector<double> &y );

};

void TestQgsProperty::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  mDefinitions.insert( Property1, QgsPropertyDefinition( QStringLiteral( "p1" ), QgsPropertyDefinition::DataTypeString, QString(), QString() ) );
  mDefinitions.insert( Property2, QgsPropertyDefinition( QStringLiteral( "p2" ), QgsPropertyDefinition::DataTypeString, QString(), QString() ) );
  mDefinitions.insert( Property3, QgsPropertyDefinition( QStringLiteral( "p3" ), QgsPropertyDefinition::DataTypeString, QString(), QString() ) );
  mDefinitions.insert( Property4, QgsPropertyDefinition( QStringLiteral( "p4" ), QgsPropertyDefinition::DataTypeString, QString(), QString() ) );
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

void TestQgsProperty::conversions()
{
  const QgsExpressionContext context;

  //all these tests are done for both a property and a collection
  QgsPropertyCollection collection;

  bool ok = false;

  //test color conversions

  //no color, should return defaultColor
  QgsProperty c1 = QgsProperty::fromValue( QVariant(), true );
  collection.setProperty( 0, c1 );
  QCOMPARE( c1.valueAsColor( context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  QCOMPARE( collection.valueAsColor( 0, context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  c1.setStaticValue( QColor( 255, 200, 100, 50 ) ); //color in qvariant
  collection.property( 0 ).setStaticValue( QColor( 255, 200, 100, 50 ) ); //color in qvariant
  QCOMPARE( c1.valueAsColor( context, QColor( 200, 210, 220 ), &ok ), QColor( 255, 200, 100, 50 ) );
  QVERIFY( ok );
  QCOMPARE( collection.valueAsColor( 0, context, QColor( 200, 210, 220 ) ), QColor( 255, 200, 100, 50 ) );
  c1.setStaticValue( QColor() );  //invalid color in qvariant, should return default color
  collection.property( 0 ).setStaticValue( QColor() );  //invalid color in qvariant, should return default color
  QCOMPARE( c1.valueAsColor( context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  QCOMPARE( collection.valueAsColor( 0, context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  c1.setStaticValue( QgsSymbolLayerUtils::encodeColor( QColor( 255, 200, 100, 50 ) ) ); //encoded color
  collection.property( 0 ).setStaticValue( QgsSymbolLayerUtils::encodeColor( QColor( 255, 200, 100, 50 ) ) ); //encoded color
  QCOMPARE( c1.valueAsColor( context, QColor( 200, 210, 220 ) ), QColor( 255, 200, 100, 50 ) );
  QCOMPARE( collection.valueAsColor( 0, context, QColor( 200, 210, 220 ) ), QColor( 255, 200, 100, 50 ) );
  c1.setStaticValue( "i am not a color" ); //badly encoded color, should return default color
  collection.property( 0 ).setStaticValue( "i am not a color" ); //badly encoded color, should return default color
  QCOMPARE( c1.valueAsColor( context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  QCOMPARE( collection.valueAsColor( 0, context, QColor( 200, 210, 220 ) ), QColor( 200, 210, 220 ) );
  collection.property( 0 ).setStaticValue( QVariant( QVariant::String ) ); //null value
  QCOMPARE( c1.valueAsColor( context, QColor( 200, 210, 220 ), &ok ), QColor( 200, 210, 220 ) );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsColor( 0, context, QColor( 200, 210, 220 ), &ok ), QColor( 200, 210, 220 ) );
  QVERIFY( !ok );

  // test double conversions
  QgsProperty d1 = QgsProperty::fromValue( QVariant(), true );
  collection.setProperty( 1, d1 );
  QCOMPARE( d1.valueAsDouble( context, -1.2, &ok ), -1.2 );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsDouble( 1, context, -1.2 ), -1.2 );
  d1.setStaticValue( 12.3 ); //double in qvariant
  collection.property( 1 ).setStaticValue( 12.3 ); //double in qvariant
  QCOMPARE( d1.valueAsDouble( context, -1.2, &ok ), 12.3 );
  QVERIFY( ok );
  QCOMPARE( collection.valueAsDouble( 1, context, -1.2 ), 12.3 );
  d1.setStaticValue( "15.6" ); //double as string
  collection.property( 1 ).setStaticValue( "15.6" ); //double as string
  QCOMPARE( d1.valueAsDouble( context, -1.2 ), 15.6 );
  QCOMPARE( collection.valueAsDouble( 1, context, -1.2 ), 15.6 );
  d1.setStaticValue( "i am not a double" ); //not a double, should return default value
  collection.property( 1 ).setStaticValue( "i am not a double" ); //not a double, should return default value
  QCOMPARE( d1.valueAsDouble( context, -1.2 ), -1.2 );
  QCOMPARE( collection.valueAsDouble( 1, context, -1.2 ), -1.2 );
  d1.setStaticValue( QVariant( QVariant::Double ) ); //null value
  collection.property( 1 ).setStaticValue( QVariant( QVariant::Double ) ); //null value
  QCOMPARE( d1.valueAsDouble( context, -1.2, &ok ), -1.2 );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsDouble( 1, context, -1.2, &ok ), -1.2 );
  QVERIFY( !ok );

  // test integer conversions
  QgsProperty i1 = QgsProperty::fromValue( QVariant(), true );
  collection.setProperty( 2, i1 );
  QCOMPARE( i1.valueAsInt( context, -11, &ok ), -11 );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), -11 );
  i1.setStaticValue( 13 ); //integer in qvariant
  collection.property( 2 ).setStaticValue( 13 ); //integer in qvariant
  QCOMPARE( i1.valueAsInt( context, -11, &ok ), 13 );
  QVERIFY( ok );
  QCOMPARE( collection.valueAsInt( 2, context, -11, &ok ), 13 );
  QVERIFY( ok );
  i1.setStaticValue( 13.9 ); //double in qvariant, should be rounded
  collection.property( 2 ).setStaticValue( 13.9 ); //double in qvariant, should be rounded
  QCOMPARE( i1.valueAsInt( context, -11 ), 14 );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), 14 );
  i1.setStaticValue( "15" ); //integer as string
  collection.property( 2 ).setStaticValue( "15" ); //integer as string
  QCOMPARE( i1.valueAsInt( context, -11 ), 15 );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), 15 );
  i1.setStaticValue( "15.9" ); //double as string, should be rounded
  collection.property( 2 ).setStaticValue( "15.9" ); //double as string, should be rounded
  QCOMPARE( i1.valueAsInt( context, -11 ), 16 );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), 16 );
  i1.setStaticValue( "i am not a int" ); //not a int, should return default value
  collection.property( 2 ).setStaticValue( "i am not a int" ); //not a int, should return default value
  QCOMPARE( i1.valueAsInt( context, -11 ), -11 );
  QCOMPARE( collection.valueAsInt( 2, context, -11 ), -11 );
  i1.setStaticValue( QVariant( QVariant::Int ) ); // null value
  collection.property( 2 ).setStaticValue( QVariant( QVariant::Int ) ); // null value
  QCOMPARE( i1.valueAsInt( context, -11, &ok ), -11 );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsInt( 2, context, -11, &ok ), -11 );
  QVERIFY( !ok );

  // test boolean conversions
  QgsProperty b1 = QgsProperty::fromValue( QVariant(), true );
  collection.setProperty( 3, b1 );
  QCOMPARE( b1.valueAsBool( context, false, &ok ), false );
  QVERIFY( !ok );
  QCOMPARE( b1.valueAsBool( context, true, &ok ), true );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsBool( 3, context, false ), false );
  QCOMPARE( collection.valueAsBool( 3, context, true ), true );
  b1.setStaticValue( true );
  collection.property( 3 ).setStaticValue( true );
  QCOMPARE( b1.valueAsBool( context, false, &ok ), true );
  QVERIFY( ok );
  QCOMPARE( b1.valueAsBool( context, true, &ok ), true );
  QVERIFY( ok );
  QCOMPARE( collection.valueAsBool( 3, context, false ), true );
  QCOMPARE( collection.valueAsBool( 3, context, true ), true );
  b1.setStaticValue( false );
  collection.property( 3 ).setStaticValue( false );
  QCOMPARE( b1.valueAsBool( context, false ), false );
  QCOMPARE( b1.valueAsBool( context, true ), false );
  QCOMPARE( collection.valueAsBool( 3, context, false ), false );
  QCOMPARE( collection.valueAsBool( 3, context, true ), false );
  b1.setStaticValue( 1 );
  collection.property( 3 ).setStaticValue( 1 );
  QCOMPARE( b1.valueAsBool( context, false ), true );
  QCOMPARE( b1.valueAsBool( context, true ), true );
  QCOMPARE( collection.valueAsBool( 3, context, false ), true );
  QCOMPARE( collection.valueAsBool( 3, context, true ), true );
  b1.setStaticValue( 0 );
  collection.property( 3 ).setStaticValue( 0 );
  QCOMPARE( b1.valueAsBool( context, false ), false );
  QCOMPARE( b1.valueAsBool( context, true ), false );
  QCOMPARE( collection.valueAsBool( 3, context, false ), false );
  QCOMPARE( collection.valueAsBool( 3, context, true ), false );
  b1.setStaticValue( "true" );
  collection.property( 3 ).setStaticValue( "true" );
  QCOMPARE( b1.valueAsBool( context, false ), true );
  QCOMPARE( b1.valueAsBool( context, true ), true );
  QCOMPARE( collection.valueAsBool( 3, context, false ), true );
  QCOMPARE( collection.valueAsBool( 3, context, true ), true );
  b1.setStaticValue( "" );
  collection.property( 3 ).setStaticValue( "" );
  QCOMPARE( b1.valueAsBool( context, false ), false );
  QCOMPARE( b1.valueAsBool( context, true ), false );
  QCOMPARE( collection.valueAsBool( 3, context, false ), false );
  QCOMPARE( collection.valueAsBool( 3, context, true ), false );
  b1.setStaticValue( QVariant( QVariant::Bool ) ); // null value
  collection.property( 3 ).setStaticValue( QVariant( QVariant::Bool ) );
  QCOMPARE( b1.valueAsBool( context, false ), false );
  QCOMPARE( b1.valueAsBool( context, true ), true );
  QCOMPARE( collection.valueAsBool( 3, context, false, &ok ), false );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsBool( 3, context, true, &ok ), true );
  QVERIFY( !ok );

  // test string conversions
  QgsProperty s1 = QgsProperty::fromValue( QVariant(), true );
  collection.setProperty( 4, s1 );
  QCOMPARE( s1.valueAsString( context, "n", &ok ), QStringLiteral( "n" ) );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsString( 4, context, "y", &ok ), QStringLiteral( "y" ) );
  QVERIFY( !ok );
  s1.setStaticValue( "s" );
  collection.property( 4 ).setStaticValue( "s" );
  QCOMPARE( s1.valueAsString( context, "n", &ok ), QStringLiteral( "s" ) );
  QVERIFY( ok );
  QCOMPARE( collection.valueAsString( 4, context, "y", &ok ), QStringLiteral( "s" ) );
  QVERIFY( ok );
  s1.setStaticValue( QVariant( QVariant::String ) );
  collection.property( 4 ).setStaticValue( QVariant( QVariant::String ) );
  QCOMPARE( s1.valueAsString( context, "n", &ok ), QStringLiteral( "n" ) );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsString( 4, context, "y", &ok ), QStringLiteral( "y" ) );
  QVERIFY( !ok );

  // test datetime conversions
  const QDateTime dt = QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) );
  const QDateTime dt2 = QDateTime( QDate( 2010, 1, 1 ), QTime( 0, 0, 0 ) );
  const QgsProperty dt1 = QgsProperty::fromValue( QVariant(), true );
  collection.setProperty( 5, dt1 );
  QCOMPARE( d1.valueAsDateTime( context, dt, &ok ), dt );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsDateTime( 5, context, dt, &ok ), dt );
  QVERIFY( !ok );
  d1.setStaticValue( dt2 ); //datetime in qvariant
  collection.property( 5 ).setStaticValue( dt2 ); //datetime in qvariant
  QCOMPARE( d1.valueAsDateTime( context, dt, &ok ), dt2 );
  QVERIFY( ok );
  QCOMPARE( collection.valueAsDateTime( 5, context,  dt, &ok ), dt2 );
  QVERIFY( ok );
  d1.setStaticValue( "2010-01-01" ); //datetime as string
  collection.property( 5 ).setStaticValue( "2010-01-01" ); //datetime as string
  QCOMPARE( d1.valueAsDateTime( context, dt ), dt2 );
  QCOMPARE( collection.valueAsDateTime( 5, context, dt ), dt2 );
  d1.setStaticValue( "i am not a datetime" ); //not a datetime, should return default value
  collection.property( 5 ).setStaticValue( "i am not a datetime" ); //not a double, should return default value
  QCOMPARE( d1.valueAsDateTime( context, dt ), dt );
  QCOMPARE( collection.valueAsDateTime( 5, context, dt ), dt );
  d1.setStaticValue( QVariant( QVariant::DateTime ) ); // null value
  collection.property( 5 ).setStaticValue( QVariant( QVariant::DateTime ) ); // null value
  QCOMPARE( d1.valueAsDateTime( context, dt, &ok ), dt );
  QVERIFY( !ok );
  QCOMPARE( collection.valueAsDateTime( 5, context, dt, &ok ), dt );
  QVERIFY( !ok );
}

void TestQgsProperty::invalid()
{
  const QgsProperty p; //invalid property
  QCOMPARE( p.propertyType(), QgsProperty::InvalidProperty );
  const QgsProperty p2( p );
  QCOMPARE( p2.propertyType(), QgsProperty::InvalidProperty );
  QgsProperty p3 = QgsProperty::fromValue( 5 );
  p3 = p;
  QCOMPARE( p3.propertyType(), QgsProperty::InvalidProperty );

}

void TestQgsProperty::staticProperty()
{
  const QgsExpressionContext context;
  QgsProperty property = QgsProperty::fromValue( QStringLiteral( "test" ), true );
  QCOMPARE( property.propertyType(), QgsProperty::StaticProperty );
  QVERIFY( property.isActive() );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  QCOMPARE( property.value( context, QStringLiteral( "default" ) ).toString(), QStringLiteral( "test" ) );
  property.setActive( false );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  QVERIFY( !property.isActive() );
  QCOMPARE( property.value( context, QStringLiteral( "default" ) ).toString(), QStringLiteral( "default" ) );
  property.setStaticValue( 5 );
  property.setActive( true );
  QCOMPARE( property.value( context ).toInt(), 5 );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );

  QgsProperty p1;
  p1.setActive( true );
  p1.setStaticValue( "test" );
  p1.setTransformer( new TestTransformer( 10, 20 ) );

  QVariant element = p1.toVariant();

  QgsProperty r1;
  r1.loadVariant( element );
  QVERIFY( r1.isActive() );
  QVERIFY( r1.transformer() );
  QCOMPARE( r1.staticValue(), QVariant( "test" ) );

  p1.setActive( false );
  element = p1.toVariant();
  r1.loadVariant( element );
  QVERIFY( !r1.isActive() );

  //saving/restoring different types
  p1.setStaticValue( QVariant( 5 ) ); //int
  element = p1.toVariant();
  r1.loadVariant( element );
  QCOMPARE( r1.staticValue(), p1.staticValue() );
  p1.setStaticValue( QVariant( 5.7 ) ); //double
  element = p1.toVariant();
  r1.loadVariant( element );
  QCOMPARE( r1.staticValue(), p1.staticValue() );
  p1.setStaticValue( QVariant( true ) ); //bool
  element = p1.toVariant();
  r1.loadVariant( element );
  QCOMPARE( r1.staticValue(), p1.staticValue() );
  p1.setStaticValue( QVariant( 5LL ) ); //longlong
  element = p1.toVariant();
  r1.loadVariant( element );
  QCOMPARE( r1.staticValue(), p1.staticValue() );

  // test copying a static property
  QgsProperty p2;
  p2.setActive( true );
  p2.setStaticValue( "test" );
  p2.setTransformer( new TestTransformer( 10, 20 ) );
  // copy assign
  QgsProperty p3;
  p3 = p2;
  QVERIFY( p3.isActive() );
  QCOMPARE( p3.staticValue().toString(), QStringLiteral( "test" ) );
  QVERIFY( p3.transformer() );
  p2.setActive( false );
  p2.setStaticValue( 5.9 );
  p3 = p2;
  QVERIFY( !p3.isActive() );
  QCOMPARE( p3.staticValue().toDouble(), 5.9 );

  // copy constructor
  const QgsProperty p4( p2 );
  QVERIFY( !p4.isActive() );
  QCOMPARE( p4.staticValue().toDouble(), 5.9 );
  QVERIFY( p4.transformer() );
}

void TestQgsProperty::fieldBasedProperty()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "field1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "field2" ), QVariant::Int ) );
  ft.setFields( fields );
  QgsAttributes attr;
  attr << QVariant( 5 ) << QVariant( 7 );
  ft.setAttributes( attr );
  ft.setValid( true );

  // throw it in an expression context
  QgsExpressionContext context;
  context.setFeature( ft );
  context.setFields( fields );

  QgsProperty property = QgsProperty::fromField( QStringLiteral( "field1" ), true );
  QCOMPARE( property.propertyType(), QgsProperty::FieldBasedProperty );
  QVERIFY( property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), 5 );
  QCOMPARE( property.referencedFields( context ), QSet< QString >() << "field1" );
  property.setActive( false );
  QVERIFY( !property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  property.setField( QStringLiteral( "field2" ) );
  property.setActive( true );
  QCOMPARE( property.value( context, -1 ).toInt(), 7 );
  QCOMPARE( property.referencedFields( context ), QSet< QString >() << "field2" );
  //bad field reference
  property.setField( QStringLiteral( "bad_field" ) );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  // unset field name
  QgsProperty defaultProperty = QgsProperty::fromField( QString() );
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );
  defaultProperty.setActive( true );
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );

  //test preparation
  const QgsProperty property3 = QgsProperty::fromField( QStringLiteral( "field1" ), true );
  QVERIFY( property3.prepare( context ) );
  QCOMPARE( property3.value( context, -1 ).toInt(), 5 );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );

  QgsProperty p1;
  p1.setActive( true );
  p1.setField( QStringLiteral( "test_field" ) );

  QVariant element;
  QgsProperty r1;
  //try reading from an empty element
  r1.loadVariant( element );
  QVERIFY( !r1.isActive() );
  QVERIFY( r1.field().isEmpty() );

  // now populate element and re-read
  element = p1.toVariant();
  r1.loadVariant( element );
  QVERIFY( r1.isActive() );
  QCOMPARE( r1.field(), QStringLiteral( "test_field" ) );

  p1.setActive( false );
  element = p1.toVariant();
  r1.loadVariant( element );
  QVERIFY( !r1.isActive() );

  // test copying a field based property
  QgsProperty p2;
  p2.setActive( true );
  p2.setField( QStringLiteral( "test" ) );
  p2.setTransformer( new TestTransformer( 10, 20 ) );

  // copy constructor
  const QgsProperty p3( p2 );
  QVERIFY( p3.isActive() );
  QCOMPARE( p3.field(), QStringLiteral( "test" ) );
  QVERIFY( p3.transformer() );
  p2.setActive( false );

  // assignment operator
  QgsProperty p4;
  p4 = p2;
  QVERIFY( !p4.isActive() );
  QCOMPARE( p4.field(), QStringLiteral( "test" ) );
  QVERIFY( p4.transformer() );
}

void TestQgsProperty::expressionBasedProperty()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "field1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "field2" ), QVariant::Int ) );
  ft.setFields( fields );
  QgsAttributes attr;
  attr << QVariant( 5 ) << QVariant( 7 );
  ft.setAttributes( attr );
  ft.setValid( true );

  // throw it in an expression context
  QgsExpressionContext context;
  context.setFeature( ft );
  context.setFields( fields );

  QgsProperty property = QgsProperty::fromExpression( QStringLiteral( "\"field1\" + \"field2\"" ), true );
  QCOMPARE( property.propertyType(), QgsProperty::ExpressionBasedProperty );
  QVERIFY( property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), 12 );
  QCOMPARE( property.referencedFields( context ).count(), 2 );
  QVERIFY( property.referencedFields( context ).contains( "field1" ) );
  QVERIFY( property.referencedFields( context ).contains( "field2" ) );
  property.setExpressionString( QStringLiteral( "\"field2\"*2" ) );
  QCOMPARE( property.value( context, -1 ).toInt(), 14 );
  QCOMPARE( property.referencedFields( context ), QSet< QString >() << "field2" );
  property.setActive( false );
  QVERIFY( !property.isActive() );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  property.setExpressionString( QStringLiteral( "'a'||'b'" ) );
  property.setActive( true );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  QCOMPARE( property.value( context, "bb" ).toString(), QStringLiteral( "ab" ) );
  //bad expression
  property.setExpressionString( QStringLiteral( "bad_ 5" ) );
  QCOMPARE( property.value( context, -1 ).toInt(), -1 );
  QVERIFY( property.referencedFields( context ).isEmpty() );
  // unset expression
  QgsProperty defaultProperty = QgsProperty::fromExpression( QString() );
  // an invalid expression (empty string) should return an invalid property
  QCOMPARE( defaultProperty.propertyType(), QgsProperty::InvalidProperty );
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );
  defaultProperty.setActive( true );
  QCOMPARE( defaultProperty.value( context, -1 ).toInt(), -1 );
  QVERIFY( defaultProperty.referencedFields( context ).isEmpty() );

  //preparation
  const QgsProperty property3 = QgsProperty::fromExpression( QStringLiteral( "\"field1\" + \"field2\"" ), true );
  QVERIFY( property3.prepare( context ) );
  QCOMPARE( property3.value( context, -1 ).toInt(), 12 );
  const QgsProperty property4 = QgsProperty::fromExpression( QStringLiteral( "\"field1\" + " ), true );
  QVERIFY( !property4.prepare( context ) );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );

  QgsProperty p1;
  p1.setActive( true );
  p1.setExpressionString( QStringLiteral( "4+5" ) );

  QVariant element;
  QgsProperty r1;
  //try reading from an empty element
  r1.loadVariant( element );
  QVERIFY( !r1.isActive() );
  QVERIFY( r1.expressionString().isEmpty() );
  QCOMPARE( r1.value( context, -1 ).toInt(), -1 );

  // now populate element and re-read
  element = p1.toVariant();
  r1.loadVariant( element );
  QVERIFY( r1.isActive() );
  QCOMPARE( r1.expressionString(), QStringLiteral( "4+5" ) );
  QCOMPARE( r1.value( context, -1 ).toInt(), 9 );

  p1.setActive( false );
  element = p1.toVariant();
  r1.loadVariant( element );
  QVERIFY( !r1.isActive() );
  QCOMPARE( r1.value( context, -1 ).toInt(), -1 );

  // test copying an expression based property
  QgsProperty p2;
  p2.setActive( true );
  p2.setExpressionString( QStringLiteral( "1+6" ) );

  // copy constructor
  const QgsProperty p3( p2 );
  QVERIFY( p3.isActive() );
  QCOMPARE( p3.expressionString(), QStringLiteral( "1+6" ) );
  QCOMPARE( p3.value( context, -1 ).toInt(), 7 );

  // assignment operator
  QgsProperty p4;
  p2.setActive( false );
  p4 = p2;
  QVERIFY( !p4.isActive() );
  QCOMPARE( p4.value( context, -1 ).toInt(), -1 );
  p2.setTransformer( new TestTransformer( 10, 20 ) );
  p4 = p2;
  QVERIFY( p4.transformer() );
}

void TestQgsProperty::equality()
{
  QgsProperty dd1;
  dd1.setActive( true );
  dd1.setField( QStringLiteral( "field" ) );
  QgsProperty dd2;
  dd2.setActive( true );
  dd2.setField( QStringLiteral( "field" ) );
  QVERIFY( dd1 == dd2 );
  QVERIFY( !( dd1 != dd2 ) );

  dd1.setExpressionString( QStringLiteral( "expression" ) );
  dd2.setExpressionString( QStringLiteral( "expression" ) );
  QVERIFY( dd1 == dd2 );
  QVERIFY( !( dd1 != dd2 ) );

  //test that all applicable components contribute to equality
  dd2.setActive( false );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd2.setActive( true );
  dd2.setExpressionString( QStringLiteral( "a" ) );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd2.setField( QStringLiteral( "field" ) );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd1.setField( QStringLiteral( "fieldb" ) );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd1.setField( QStringLiteral( "field" ) );
  QVERIFY( dd1 == dd2 );
  QVERIFY( !( dd1 != dd2 ) );

  // with transformer
  dd1.setTransformer( new QgsGenericNumericTransformer( 1, 2, 3, 4 ) );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd2.setTransformer( new QgsGenericNumericTransformer( 1, 2, 3, 4 ) );
  QVERIFY( dd1 == dd2 );
  QVERIFY( !( dd1 != dd2 ) );
}

void TestQgsProperty::isStaticValueInContext()
{
  // test the QgsProperty::isStaticValueInContext logic
  QgsExpressionContext context;
  QgsProperty p;
  QVariant v;
  v = 5; // set an initial value so we can be sure it's cleared
  // an invalid property is static -- its value won't change
  QVERIFY( p.isStaticValueInContext( context, v ) );
  QVERIFY( !v.isValid() );

  // a static value IS static (duh)
  p = QgsProperty::fromValue( 55 );
  QVERIFY( p.isStaticValueInContext( context, v ) );
  QCOMPARE( v.toInt(), 55 );

  // a field based property is NOT static
  p = QgsProperty::fromField( QStringLiteral( "xxx" ) );
  QVERIFY( !p.isStaticValueInContext( context, v ) );
  QVERIFY( !v.isValid() );

  // an expression based property may or may not be static
  // start with a non-static expression
  p = QgsProperty::fromExpression( QStringLiteral( "\"xxx\"" ) );
  v = 5;
  QVERIFY( !p.isStaticValueInContext( context, v ) );
  QVERIFY( !v.isValid() );

  // should still be non-static, even with valid fields
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "xxx" ), QVariant::Int ) );
  context.setFields( fields );
  v = 5;
  QVERIFY( !p.isStaticValueInContext( context, v ) );
  QVERIFY( !v.isValid() );

  // an expression which IS static
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "my_var" ), 123, true, true ) );
  context.appendScope( scope );
  p = QgsProperty::fromExpression( QStringLiteral( "@my_var * 2" ) );
  v = 5;
  QVERIFY( p.isStaticValueInContext( context, v ) );
  QCOMPARE( v.toInt(), 246 );
}

void TestQgsProperty::propertyTransformer()
{
  const QgsExpressionContext context;
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
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );

  const TestTransformer t1( -5, 6 );
  QVariant element;
  TestTransformer r1( -99, -98 );
  element = t1.toVariant();
  QVERIFY( r1.loadVariant( element ) );
  QCOMPARE( r1.minValue(), -5.0 );
  QCOMPARE( r1.maxValue(), 6.0 );

  //install into property and test evaluation
  QgsProperty p1;
  p1.setTransformer( new TestTransformer( 10, 20 ) );
  QVERIFY( dynamic_cast< const TestTransformer * >( p1.transformer() ) );
  QCOMPARE( static_cast< const TestTransformer * >( p1.transformer() )->minValue(), 10.0 );
  QCOMPARE( static_cast< const TestTransformer * >( p1.transformer() )->maxValue(), 20.0 );
  p1.setStaticValue( QVariant( QVariant::Double ) );
  QCOMPARE( p1.value( context, -99 ).toDouble(), -1.0 );
  p1.setStaticValue( 11.0 );
  QCOMPARE( p1.value( context, -99 ).toDouble(), 22.0 );

  //test that transform is saved/restored with property
  QVariant propElement;
  QgsProperty p2;
  QVERIFY( !p2.transformer() );
  propElement = p1.toVariant();
  p2.loadVariant( propElement );
  QVERIFY( p2.transformer() );
  QCOMPARE( p2.transformer()->minValue(), 10.0 );
  QCOMPARE( p2.transformer()->maxValue(), 20.0 );

  //test that copy constructor copies transformer
  const QgsProperty p4( p1 );
  QVERIFY( p4.transformer() );
  QCOMPARE( p4.transformer()->minValue(), 10.0 );
  QCOMPARE( p4.transformer()->maxValue(), 20.0 );

  //test that assignment operator copies transformer
  QgsProperty p5;
  p5 = p1;
  QVERIFY( p5.transformer() );
  QCOMPARE( p5.transformer()->minValue(), 10.0 );
  QCOMPARE( p5.transformer()->maxValue(), 20.0 );
}

void TestQgsProperty::propertyTransformerFromExpression()
{
  QString baseExpression;
  QString fieldName;
  // not convertible to a transformer
  std::unique_ptr< QgsPropertyTransformer > exp( QgsPropertyTransformer::fromExpression( QStringLiteral( "1 * 2" ), baseExpression, fieldName ) );
  QVERIFY( !exp.get() );
  QVERIFY( baseExpression.isEmpty() );
  QVERIFY( fieldName.isEmpty() );

  // convertible to a size scale transformer
  exp.reset( QgsPropertyTransformer::fromExpression( QStringLiteral( "coalesce(scale_linear(column, 1, 7, 2, 10), 0)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->transformerType(), QgsPropertyTransformer::SizeScaleTransformer );
  QCOMPARE( fieldName, QStringLiteral( "column" ) );
  QVERIFY( baseExpression.isEmpty() );

  exp.reset( QgsPropertyTransformer::fromExpression( QStringLiteral( "coalesce(scale_linear(column * 2, 1, 7, 2, 10), 0)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->transformerType(), QgsPropertyTransformer::SizeScaleTransformer );
  QCOMPARE( baseExpression, QStringLiteral( "column * 2" ) );
  QVERIFY( fieldName.isEmpty() );
}

void TestQgsProperty::genericNumericTransformer()
{
  const QgsExpressionContext context;
  QgsGenericNumericTransformer t1( 10,
                                   20,
                                   100,
                                   200,
                                   -10,
                                   1.0 );
  QCOMPARE( t1.transformerType(), QgsPropertyTransformer::GenericNumericTransformer );
  QCOMPARE( t1.minValue(), 10.0 );
  QCOMPARE( t1.maxValue(), 20.0 );
  QCOMPARE( t1.minOutputValue(), 100.0 );
  QCOMPARE( t1.maxOutputValue(), 200.0 );
  QCOMPARE( t1.nullOutputValue(), -10.0 );
  QCOMPARE( t1.exponent(), 1.0 );

  //transform
  QCOMPARE( t1.transform( context, 10 ).toInt(), 100 );
  QCOMPARE( t1.transform( context, 20 ).toInt(), 200 );
  //null value
  QCOMPARE( t1.transform( context, QVariant( QVariant::Double ) ).toInt(), -10 );
  //non numeric value
  QCOMPARE( t1.transform( context, QVariant( "ffff" ) ), QVariant( "ffff" ) );

  // add a curve
  QVERIFY( !t1.curveTransform() );
  t1.setCurveTransform( new QgsCurveTransform( QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) ) );
  QVERIFY( t1.curveTransform() );
  QCOMPARE( t1.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  QCOMPARE( t1.transform( context, 10 ).toInt(), 180 );
  QCOMPARE( t1.transform( context, 20 ).toInt(), 120 );

  // copy
  const QgsGenericNumericTransformer s1( t1 );
  QVERIFY( s1.curveTransform() );
  QCOMPARE( s1.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  // assignment
  QgsGenericNumericTransformer s2;
  s2 = t1;
  QVERIFY( s2.curveTransform() );
  QCOMPARE( s2.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );

  QgsGenericNumericTransformer t2( 15,
                                   25,
                                   150,
                                   250,
                                   -10,
                                   99 );
  t2.setCurveTransform( new QgsCurveTransform( QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) ) );

  QVariant element;
  element = t2.toVariant();
  QgsGenericNumericTransformer r1;
  QVERIFY( r1.loadVariant( element ) );
  QCOMPARE( r1.minValue(), 15.0 );
  QCOMPARE( r1.maxValue(), 25.0 );
  QCOMPARE( r1.minOutputValue(), 150.0 );
  QCOMPARE( r1.maxOutputValue(), 250.0 );
  QCOMPARE( r1.nullOutputValue(), -10.0 );
  QCOMPARE( r1.exponent(), 99.0 );
  QVERIFY( r1.curveTransform() );
  QCOMPARE( r1.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  // test cloning
  std::unique_ptr< QgsGenericNumericTransformer > r2( t2.clone() );
  QCOMPARE( r2->minValue(), 15.0 );
  QCOMPARE( r2->maxValue(), 25.0 );
  QCOMPARE( r2->minOutputValue(), 150.0 );
  QCOMPARE( r2->maxOutputValue(), 250.0 );
  QCOMPARE( r2->nullOutputValue(), -10.0 );
  QCOMPARE( r2->exponent(), 99.0 );
  QVERIFY( r2->curveTransform() );
  QCOMPARE( r2->curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  //test various min/max value/size and scaling methods

  //getters and setters
  QgsGenericNumericTransformer t;
  t.setMinValue( 100 );
  QCOMPARE( t.minValue(), 100.0 );
  t.setMaxValue( 200 );
  QCOMPARE( t.maxValue(), 200.0 );
  t.setMinOutputValue( 10.0 );
  QCOMPARE( t.minOutputValue(), 10.0 );
  t.setMaxOutputValue( 20.0 );
  QCOMPARE( t.maxOutputValue(), 20.0 );
  t.setNullOutputValue( 1 );
  QCOMPARE( t.nullOutputValue(), 1.0 );
  t.setExponent( 2.5 );
  QCOMPARE( t.exponent(), 2.5 );

  //test linear scaling
  t.setExponent( 1.0 );
  QCOMPARE( t.value( 100 ), 10.0 );
  QCOMPARE( t.value( 150 ), 15.0 );
  QCOMPARE( t.value( 200 ), 20.0 );
  //test exponential scaling
  t.setExponent( 1.5 );
  QCOMPARE( t.value( 100 ), 10.0 );
  QGSCOMPARENEAR( t.value( 150 ), 13.5355, 0.001 );
  QCOMPARE( t.value( 200 ), 20.0 );

  // invalid settings, where minValue = maxValue
  const QgsGenericNumericTransformer invalid( 1.0, 1.0, 0, 1.0 );
  QCOMPARE( invalid.value( -1 ), 0.0 );
  QCOMPARE( invalid.value( 0 ), 0.0 );
  QCOMPARE( invalid.value( 1.0 ), 1.0 );
  QCOMPARE( invalid.value( 2.0 ), 1.0 );

  //as expression
  QgsGenericNumericTransformer t3( 15,
                                   25,
                                   150,
                                   250,
                                   -10,
                                   1.0 );
  QCOMPARE( t3.toExpression( "5+6" ), QStringLiteral( "coalesce(scale_linear(5+6, 15, 25, 150, 250), -10)" ) );
  t3.setExponent( 1.6 );
  QCOMPARE( t3.toExpression( "5+6" ), QStringLiteral( "coalesce(scale_exp(5+6, 15, 25, 150, 250, 1.6), -10)" ) );

  // test size scale transformer inside property
  QgsProperty p;
  p.setTransformer( new QgsGenericNumericTransformer( 15,
                    25,
                    150,
                    250,
                    -10,
                    99 ) );
  p.setStaticValue( QVariant() );
  bool ok = false;
  QCOMPARE( p.valueAsDouble( context, 100, &ok ), -10.0 );
  QVERIFY( ok );
  p.setExpressionString( QStringLiteral( "NULL" ) );
  QCOMPARE( p.valueAsDouble( context, 100, &ok ), -10.0 );
  QVERIFY( ok );
  p.setExpressionString( QStringLiteral( "no field" ) );
  QCOMPARE( p.valueAsDouble( context, 100, &ok ), -10.0 );
  QVERIFY( ok );
}

void TestQgsProperty::genericNumericTransformerFromExpression()
{
  QString baseExpression;
  QString fieldName;
  std::unique_ptr< QgsGenericNumericTransformer > exp( QgsGenericNumericTransformer::fromExpression( QStringLiteral( "coalesce(scale_linear(column, 1, 7, 2, 10), 0)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( fieldName, QStringLiteral( "column" ) );
  QVERIFY( baseExpression.isEmpty() );
  QCOMPARE( exp->minValue(), 1. );
  QCOMPARE( exp->maxValue(), 7. );
  QCOMPARE( exp->minOutputValue(), 2. );
  QCOMPARE( exp->maxOutputValue(), 10. );
  QCOMPARE( exp->nullOutputValue(), 0.0 );

  exp.reset( QgsGenericNumericTransformer::fromExpression( QStringLiteral( "scale_linear(column, 1, 7, 2, 10)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( fieldName, QStringLiteral( "column" ) );
  QVERIFY( baseExpression.isEmpty() );
  QCOMPARE( exp->minValue(), 1. );
  QCOMPARE( exp->maxValue(), 7. );
  QCOMPARE( exp->minOutputValue(), 2. );
  QCOMPARE( exp->maxOutputValue(), 10. );

  exp.reset( QgsGenericNumericTransformer::fromExpression( QStringLiteral( "scale_linear(column * 2, 1, 7, 2, 10)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( baseExpression, QStringLiteral( "column * 2" ) );
  QVERIFY( fieldName.isEmpty() );
  QCOMPARE( exp->minValue(), 1. );
  QCOMPARE( exp->maxValue(), 7. );
  QCOMPARE( exp->minOutputValue(), 2. );
  QCOMPARE( exp->maxOutputValue(), 10. );

  exp.reset( QgsGenericNumericTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7, 2, 10, 0.51), 1)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->minValue(), 1. );
  QCOMPARE( exp->maxValue(), 7. );
  QCOMPARE( exp->minOutputValue(), 2. );
  QCOMPARE( exp->maxOutputValue(), 10. );
  QCOMPARE( exp->exponent(), 0.51 );
  QCOMPARE( exp->nullOutputValue(), 1.0 );

  QVERIFY( !QgsGenericNumericTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7, a, 10, 0.5), 0)" ), baseExpression, fieldName ) );
  QVERIFY( !QgsGenericNumericTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7), 0)" ), baseExpression, fieldName ) );
  QVERIFY( !QgsGenericNumericTransformer::fromExpression( QStringLiteral( "1+2" ), baseExpression, fieldName ) );
  QVERIFY( !QgsGenericNumericTransformer::fromExpression( QString(), baseExpression, fieldName ) );
}

void TestQgsProperty::sizeScaleTransformer()
{
  const QgsExpressionContext context;
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

  // add a curve
  QVERIFY( !scale.curveTransform() );
  scale.setCurveTransform( new QgsCurveTransform( QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) ) );
  QVERIFY( scale.curveTransform() );
  QCOMPARE( scale.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) );
  QCOMPARE( scale.transform( context, 10 ).toInt(), 120 );
  QCOMPARE( scale.transform( context, 20 ).toInt(), 180 );

  // copy
  const QgsSizeScaleTransformer s1( scale );
  QVERIFY( s1.curveTransform() );
  QCOMPARE( s1.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) );

  // assignment
  QgsSizeScaleTransformer s2;
  s2 = scale;
  QVERIFY( s2.curveTransform() );
  QCOMPARE( s2.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );

  QgsSizeScaleTransformer t1( QgsSizeScaleTransformer::Exponential,
                              15,
                              25,
                              150,
                              250,
                              -10,
                              99 );
  t1.setCurveTransform( new QgsCurveTransform( QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) ) );

  QVariant element;
  element = t1.toVariant();
  QgsSizeScaleTransformer r1;
  QVERIFY( r1.loadVariant( element ) );
  QCOMPARE( r1.minValue(), 15.0 );
  QCOMPARE( r1.maxValue(), 25.0 );
  QCOMPARE( r1.minSize(), 150.0 );
  QCOMPARE( r1.maxSize(), 250.0 );
  QCOMPARE( r1.nullSize(), -10.0 );
  QCOMPARE( r1.exponent(), 99.0 );
  QCOMPARE( r1.type(), QgsSizeScaleTransformer::Exponential );
  QVERIFY( r1.curveTransform() );
  QCOMPARE( r1.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  // test cloning
  std::unique_ptr< QgsSizeScaleTransformer > r2( t1.clone() );
  QCOMPARE( r2->minValue(), 15.0 );
  QCOMPARE( r2->maxValue(), 25.0 );
  QCOMPARE( r2->minSize(), 150.0 );
  QCOMPARE( r2->maxSize(), 250.0 );
  QCOMPARE( r2->nullSize(), -10.0 );
  QCOMPARE( r2->exponent(), 99.0 );
  QCOMPARE( r2->type(), QgsSizeScaleTransformer::Exponential );
  QVERIFY( r2->curveTransform() );
  QCOMPARE( r2->curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

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
  QGSCOMPARENEAR( t.size( 150 ), 17.0711, 0.001 );
  QCOMPARE( t.size( 200 ), 20.0 );
  //test flannery scaling
  t.setType( QgsSizeScaleTransformer::Flannery );
  QCOMPARE( t.size( 100 ), 10.0 );
  QGSCOMPARENEAR( t.size( 150 ), 16.7362, 0.001 );
  QCOMPARE( t.size( 200 ), 20.0 );
  //test exponential scaling
  t.setType( QgsSizeScaleTransformer::Exponential );
  t.setExponent( 1.5 );
  QCOMPARE( t.size( 100 ), 10.0 );
  QGSCOMPARENEAR( t.size( 150 ), 13.5355, 0.001 );
  QCOMPARE( t.size( 200 ), 20.0 );

  //as expression
  QgsSizeScaleTransformer t2( QgsSizeScaleTransformer::Linear,
                              15,
                              25,
                              150,
                              250,
                              -10,
                              1.6 );
  QCOMPARE( t2.toExpression( "5+6" ), QStringLiteral( "coalesce(scale_linear(5+6, 15, 25, 150, 250), -10)" ) );
  t2.setType( QgsSizeScaleTransformer::Exponential );
  t2.setExponent( 1.6 );
  QCOMPARE( t2.toExpression( "5+6" ), QStringLiteral( "coalesce(scale_exp(5+6, 15, 25, 150, 250, 1.6), -10)" ) );

  // test size scale transformer inside property
  QgsProperty p;
  p.setTransformer( new  QgsSizeScaleTransformer( QgsSizeScaleTransformer::Exponential,
                    15,
                    25,
                    150,
                    250,
                    -10,
                    99 ) );
  p.setStaticValue( QVariant() );
  bool ok = false;
  QCOMPARE( p.valueAsDouble( context, 100, &ok ), -10.0 );
  QVERIFY( ok );
  p.setExpressionString( QStringLiteral( "NULL" ) );
  QCOMPARE( p.valueAsDouble( context, 100, &ok ), -10.0 );
  QVERIFY( ok );
  p.setExpressionString( QStringLiteral( "no field" ) );
  QCOMPARE( p.valueAsDouble( context, 100, &ok ), -10.0 );
  QVERIFY( ok );
}

void TestQgsProperty::sizeScaleTransformerFromExpression()
{
  QString baseExpression;
  QString fieldName;
  std::unique_ptr< QgsSizeScaleTransformer > exp( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "coalesce(scale_linear(column, 1, 7, 2, 10), 0)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Linear );
  QCOMPARE( fieldName, QStringLiteral( "column" ) );
  QVERIFY( baseExpression.isEmpty() );
  QCOMPARE( exp->minValue(), 1. );
  QCOMPARE( exp->maxValue(), 7. );
  QCOMPARE( exp->minSize(), 2. );
  QCOMPARE( exp->maxSize(), 10. );
  QCOMPARE( exp->nullSize(), 0.0 );

  exp.reset( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7, 2, 10, 0.5), 0)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Area );

  exp.reset( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7, 2, 10, 0.57), 0)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Flannery );

  exp.reset( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "scale_linear(column, 1, 7, 2, 10)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Linear );
  QCOMPARE( fieldName, QStringLiteral( "column" ) );
  QVERIFY( baseExpression.isEmpty() );
  QCOMPARE( exp->minValue(), 1. );
  QCOMPARE( exp->maxValue(), 7. );
  QCOMPARE( exp->minSize(), 2. );
  QCOMPARE( exp->maxSize(), 10. );

  exp.reset( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "scale_linear(column * 2, 1, 7, 2, 10)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Linear );
  QCOMPARE( baseExpression, QStringLiteral( "column * 2" ) );
  QVERIFY( fieldName.isEmpty() );
  QCOMPARE( exp->minValue(), 1. );
  QCOMPARE( exp->maxValue(), 7. );
  QCOMPARE( exp->minSize(), 2. );
  QCOMPARE( exp->maxSize(), 10. );

  exp.reset( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "scale_exp(column, 1, 7, 2, 10, 0.5)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Area );

  exp.reset( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "scale_exp(column, 1, 7, 2, 10, 0.57)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Flannery );

  exp.reset( QgsSizeScaleTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7, 2, 10, 0.51), 22)" ), baseExpression, fieldName ) );
  QVERIFY( exp.get() );
  QCOMPARE( exp->type(), QgsSizeScaleTransformer::Exponential );
  QCOMPARE( exp->nullSize(), 22.0 );

  QVERIFY( !QgsSizeScaleTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7, a, 10, 0.5), 0)" ), baseExpression, fieldName ) );
  QVERIFY( !QgsSizeScaleTransformer::fromExpression( QStringLiteral( "coalesce(scale_exp(column, 1, 7), 0)" ), baseExpression, fieldName ) );
  QVERIFY( !QgsSizeScaleTransformer::fromExpression( QStringLiteral( "1+2" ), baseExpression, fieldName ) );
  QVERIFY( !QgsSizeScaleTransformer::fromExpression( QString(), baseExpression, fieldName ) );
}

void TestQgsProperty::colorRampTransformer()
{
  const QgsExpressionContext context;
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

  // add a curve
  QVERIFY( !scale.curveTransform() );
  scale.setCurveTransform( new QgsCurveTransform( QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) ) );
  QVERIFY( scale.curveTransform() );
  QCOMPARE( scale.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) );

  QCOMPARE( scale.transform( context, 10 ).value<QColor>().name(), QString( "#333333" ) );
  QCOMPARE( scale.transform( context, 20 ).value<QColor>().name(), QString( "#cccccc" ) );

  // copy
  const QgsColorRampTransformer s1( scale );
  QVERIFY( s1.curveTransform() );
  QCOMPARE( s1.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) );

  // assignment
  QgsColorRampTransformer s2;
  s2 = scale;
  QVERIFY( s2.curveTransform() );
  QCOMPARE( s2.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1, 0.8 ) );

  //saving and restoring

  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );

  QgsColorRampTransformer t1( 15,
                              25,
                              new QgsGradientColorRamp( QColor( 10, 20, 30 ), QColor( 200, 190, 180 ) ),
                              QColor( 100, 150, 200 ) );
  t1.setRampName( QStringLiteral( "rampname " ) );
  t1.setCurveTransform( new QgsCurveTransform( QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) ) );

  QVariant element;
  element = t1.toVariant();
  QgsColorRampTransformer r1;
  QVERIFY( r1.loadVariant( element ) );
  QCOMPARE( r1.minValue(), 15.0 );
  QCOMPARE( r1.maxValue(), 25.0 );
  QCOMPARE( r1.nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( r1.rampName(), QStringLiteral( "rampname " ) );
  QVERIFY( dynamic_cast< QgsGradientColorRamp * >( r1.colorRamp() ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r1.colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r1.colorRamp() )->color2(), QColor( 200, 190, 180 ) );
  QVERIFY( r1.curveTransform() );
  QCOMPARE( r1.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  // test cloning
  std::unique_ptr< QgsColorRampTransformer > r2( t1.clone() );
  QCOMPARE( r2->minValue(), 15.0 );
  QCOMPARE( r2->maxValue(), 25.0 );
  QCOMPARE( r2->nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( r2->rampName(), QStringLiteral( "rampname " ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r2->colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r2->colorRamp() )->color2(), QColor( 200, 190, 180 ) );
  QVERIFY( r2->curveTransform() );
  QCOMPARE( r2->curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  // copy constructor
  const QgsColorRampTransformer r3( t1 );
  QCOMPARE( r3.minValue(), 15.0 );
  QCOMPARE( r3.maxValue(), 25.0 );
  QCOMPARE( r3.nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( r3.rampName(), QStringLiteral( "rampname " ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r3.colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r3.colorRamp() )->color2(), QColor( 200, 190, 180 ) );
  QVERIFY( r3.curveTransform() );
  QCOMPARE( r3.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

  // assignment operator
  QgsColorRampTransformer r4;
  r4 = t1;
  QCOMPARE( r4.minValue(), 15.0 );
  QCOMPARE( r4.maxValue(), 25.0 );
  QCOMPARE( r4.nullColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( r4.rampName(), QStringLiteral( "rampname " ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r4.colorRamp() )->color1(), QColor( 10, 20, 30 ) );
  QCOMPARE( static_cast< QgsGradientColorRamp * >( r4.colorRamp() )->color2(), QColor( 200, 190, 180 ) );
  QVERIFY( r4.curveTransform() );
  QCOMPARE( r4.curveTransform()->controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0, 0.8 ) << QgsPointXY( 1, 0.2 ) );

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
  QCOMPARE( static_cast< QgsGradientColorRamp * >( t.colorRamp() )->color1(), QColor( 10, 20, 100 ) );
  t.setRampName( QStringLiteral( "colorramp" ) );
  QCOMPARE( t.rampName(), QStringLiteral( "colorramp" ) );

  //test colors
  QCOMPARE( t.color( 50 ), QColor( 10, 20, 100 ) ); //out of range
  QCOMPARE( t.color( 100 ), QColor( 10, 20, 100 ) );
  QCOMPARE( t.color( 150 ), QColor( 55, 110, 150 ) );
  QCOMPARE( t.color( 200 ), QColor( 100, 200, 200 ) );
  QCOMPARE( t.color( 250 ), QColor( 100, 200, 200 ) ); //out of range

  //toExpression
  QgsColorRampTransformer t5( 15,
                              25,
                              new QgsGradientColorRamp( QColor( 10, 20, 30 ), QColor( 200, 190, 180 ) ),
                              QColor( 100, 150, 200 ) );
  QCOMPARE( t5.toExpression( "5+6" ), QStringLiteral( "coalesce(ramp_color('custom ramp',scale_linear(5+6, 15, 25, 0, 1)), '#6496c8')" ) );
  t5.setRampName( QStringLiteral( "my ramp" ) );
  QCOMPARE( t5.toExpression( "5+6" ), QStringLiteral( "coalesce(ramp_color('my ramp',scale_linear(5+6, 15, 25, 0, 1)), '#6496c8')" ) );
}

void TestQgsProperty::propertyToTransformer()
{
  // not convertible to a transformer:

  // fields cannot be converted
  QgsProperty p = QgsProperty::fromField( QStringLiteral( "a field" ) );
  QVERIFY( !p.convertToTransformer() );
  QVERIFY( !p.transformer() );
  QCOMPARE( p.field(), QStringLiteral( "a field" ) );

  // static values cannot be converted
  p = QgsProperty::fromValue( 5 );
  QVERIFY( !p.convertToTransformer() );
  QVERIFY( !p.transformer() );
  QCOMPARE( p.staticValue(), QVariant( 5 ) );

  // bad expression which cannot be converted
  p = QgsProperty::fromExpression( QStringLiteral( "5*5" ) );
  QVERIFY( !p.convertToTransformer() );
  QVERIFY( !p.transformer() );
  QCOMPARE( p.expressionString(), QStringLiteral( "5*5" ) );

  // expression which can be converted to size scale transformer with base expression
  p = QgsProperty::fromExpression( QStringLiteral( "coalesce(scale_linear(column * 2, 1, 7, 2, 10), 0)" ) );
  QVERIFY( p.convertToTransformer() );
  QVERIFY( p.transformer() );
  QCOMPARE( p.expressionString(), QStringLiteral( "column * 2" ) );

  // expression which can be converted to a size scale transformer with base column ref
  p = QgsProperty::fromExpression( QStringLiteral( "coalesce(scale_linear(column, 1, 7, 2, 10), 0)" ) );
  QVERIFY( p.convertToTransformer() );
  QVERIFY( p.transformer() );
  QCOMPARE( p.field(), QStringLiteral( "column" ) );
}

void TestQgsProperty::asExpression()
{
  // static property
  QgsProperty p = QgsProperty::fromValue( 5 );
  QCOMPARE( p.asExpression(), QStringLiteral( "5" ) );
  p = QgsProperty::fromValue( "value" );
  QCOMPARE( p.asExpression(), QStringLiteral( "'value'" ) );

  // field based property
  p = QgsProperty::fromField( QStringLiteral( "a field" ) );
  QCOMPARE( p.asExpression(), QStringLiteral( "\"a field\"" ) );

  // expression based property
  p = QgsProperty::fromExpression( QStringLiteral( "5 + 6" ) );
  QCOMPARE( p.asExpression(), QStringLiteral( "5 + 6" ) );

  // with transformer
  p.setTransformer( new QgsSizeScaleTransformer( QgsSizeScaleTransformer::Linear,
                    15,
                    25,
                    150,
                    250,
                    -10,
                    1 ) );
  QCOMPARE( p.asExpression(), QStringLiteral( "coalesce(scale_linear(5 + 6, 15, 25, 150, 250), -10)" ) );
}

void TestQgsProperty::propertyCollection()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "field1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "field2" ), QVariant::Int ) );
  ft.setFields( fields );
  QgsAttributes attr;
  attr << QVariant( 5 ) << QVariant( 7 );
  ft.setAttributes( attr );
  ft.setValid( true );

  // throw it in an expression context
  QgsExpressionContext context;
  context.setFeature( ft );
  context.setFields( fields );

  QgsPropertyCollection collection( QStringLiteral( "collection" ) );
  QCOMPARE( collection.name(), QStringLiteral( "collection" ) );
  QVERIFY( !collection.hasProperty( Property1 ) );
  QVERIFY( collection.referencedFields( context ).isEmpty() );
  QCOMPARE( collection.count(), 0 );
  QCOMPARE( collection.propertyKeys(), QSet< int >() );
  QVERIFY( !collection.hasDynamicProperties() );
  QVERIFY( !collection.hasActiveProperties() );

  QgsPropertyCollection collection2;
  QVERIFY( collection == collection2 );
  QVERIFY( !( collection != collection2 ) );

  const QgsProperty property = QgsProperty::fromValue( "value", true );
  collection.setProperty( Property1, property );
  QVERIFY( collection.hasProperty( Property1 ) );
  QCOMPARE( collection.count(), 1 );
  QCOMPARE( collection.propertyKeys(), QSet< int >() << Property1 );
  QCOMPARE( collection.property( Property1 ).value( context ), property.value( context ) );
  QCOMPARE( collection.value( Property1, context ), property.value( context ) );
  QVERIFY( collection.isActive( Property1 ) );
  QVERIFY( collection.hasActiveProperties() );
  QVERIFY( !collection.hasDynamicProperties() );

  QVERIFY( collection != collection2 );
  QVERIFY( !( collection == collection2 ) );
  collection2.setProperty( Property1, property );
  QVERIFY( collection == collection2 );
  QVERIFY( !( collection != collection2 ) );

  //preparation
  QVERIFY( collection.prepare( context ) );

  //test bad property
  QVERIFY( !const_cast< const QgsPropertyCollection * >( &collection )->property( Property2 ) );
  QVERIFY( !collection.value( Property2, context ).isValid() );
  QCOMPARE( collection.value( Property2, context, QStringLiteral( "default" ) ).toString(), QStringLiteral( "default" ) );
  QVERIFY( !collection.isActive( Property2 ) );

  //test replacing property
  const QgsProperty property2 = QgsProperty::fromValue( "value2", true );
  collection.setProperty( Property1, property2 );
  QCOMPARE( collection.count(), 1 );
  QCOMPARE( collection.propertyKeys(), QSet< int >() << Property1 );
  QCOMPARE( collection.property( Property1 ).value( context ), property2.value( context ) );
  QVERIFY( collection.hasActiveProperties() );
  QVERIFY( !collection.hasDynamicProperties() );
  QVERIFY( collection != collection2 );
  QVERIFY( !( collection == collection2 ) );

  //implicit conversion
  collection.setProperty( Property3, 5 );
  QCOMPARE( collection.property( Property3 ).value( context ).toInt(), 5 );
  QVERIFY( collection.property( Property3 ).isActive() );
  QCOMPARE( collection.count(), 2 );
  QCOMPARE( collection.propertyKeys(), QSet<int>() << Property1 << Property3 );

  //test removing a property
  collection.setProperty( Property1, QgsProperty() );
  QVERIFY( !const_cast< const QgsPropertyCollection * >( &collection )->property( Property1 ) );
  QVERIFY( !collection.hasProperty( Property1 ) );
  QCOMPARE( collection.propertyKeys(), QSet<int>() << Property3 );
  QVERIFY( !collection.property( Property1 ) ); // should insert a default created invalid property in internal hash
  QVERIFY( !collection.hasProperty( Property1 ) );

  //clear
  collection.clear();
  QCOMPARE( collection.count(), 0 );
  QCOMPARE( collection.propertyKeys(), QSet<int>() );
  QVERIFY( !collection.hasActiveProperties() );
  QVERIFY( !collection.hasDynamicProperties() );

  collection.setProperty( Property1, QgsProperty::fromValue( "v1", true ) );
  collection.setProperty( Property2, QgsProperty::fromValue( "v2", false ) );
  collection.setProperty( Property3, QgsProperty::fromField( QStringLiteral( "field1" ), true ) );
  collection.setProperty( Property4, QgsProperty::fromExpression( QStringLiteral( "\"field1\" + \"field2\"" ), true ) );
  QCOMPARE( collection.count(), 4 );

  collection2 = collection;
  QVERIFY( collection == collection2 );
  QVERIFY( !( collection != collection2 ) );
  collection2.setProperty( Property3, QgsProperty() );
  QVERIFY( collection != collection2 );
  QVERIFY( !( collection == collection2 ) );

  // test referenced fields
  QCOMPARE( collection.referencedFields( context ).count(), 2 );
  QVERIFY( collection.referencedFields( context ).contains( "field1" ) );
  QVERIFY( collection.referencedFields( context ).contains( "field2" ) );

  //saving and restoring

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  const QDomDocument doc( documentType );
  const QVariant collectionElement = collection.toVariant( mDefinitions );

  QgsPropertyCollection restoredCollection;
  restoredCollection.loadVariant( collectionElement, mDefinitions );
  QCOMPARE( restoredCollection.name(), QStringLiteral( "collection" ) );
  QCOMPARE( restoredCollection.count(), 4 );
  QCOMPARE( restoredCollection.property( Property1 ).propertyType(), QgsProperty::StaticProperty );
  QVERIFY( restoredCollection.property( Property1 ).isActive() );
  QCOMPARE( restoredCollection.property( Property1 ).staticValue(), QVariant( "v1" ) );
  QCOMPARE( restoredCollection.property( Property2 ).propertyType(), QgsProperty::StaticProperty );
  QVERIFY( !restoredCollection.property( Property2 ).isActive() );
  QCOMPARE( restoredCollection.property( Property2 ).staticValue(), QVariant( "v2" ) );
  QCOMPARE( restoredCollection.property( Property3 ).propertyType(), QgsProperty::FieldBasedProperty );
  QVERIFY( restoredCollection.property( Property3 ).isActive() );
  QCOMPARE( restoredCollection.property( Property3 ).field(), QStringLiteral( "field1" ) );
  QCOMPARE( restoredCollection.property( Property4 ).propertyType(), QgsProperty::ExpressionBasedProperty );
  QVERIFY( restoredCollection.property( Property4 ).isActive() );
  QCOMPARE( restoredCollection.property( Property4 ).expressionString(), QStringLiteral( "\"field1\" + \"field2\"" ) );
  QVERIFY( restoredCollection.hasActiveProperties() );
  QVERIFY( restoredCollection.hasDynamicProperties() );

  // copy constructor
  collection2 = QgsPropertyCollection( collection );
  QCOMPARE( collection2.name(), QStringLiteral( "collection" ) );
  QCOMPARE( collection2.count(), 4 );
  QCOMPARE( collection2.property( Property1 ).propertyType(), QgsProperty::StaticProperty );
  QVERIFY( collection2.property( Property1 ).isActive() );
  QCOMPARE( collection2.property( Property1 ).staticValue(), QVariant( "v1" ) );
  QCOMPARE( collection2.property( Property2 ).propertyType(), QgsProperty::StaticProperty );
  QVERIFY( !collection2.property( Property2 ).isActive() );
  QCOMPARE( collection2.property( Property2 ).staticValue(), QVariant( "v2" ) );
  QCOMPARE( collection2.property( Property3 ).propertyType(), QgsProperty::FieldBasedProperty );
  QVERIFY( collection2.property( Property3 ).isActive() );
  QCOMPARE( collection2.property( Property3 ).field(), QStringLiteral( "field1" ) );
  QCOMPARE( collection2.property( Property4 ).propertyType(), QgsProperty::ExpressionBasedProperty );
  QVERIFY( collection2.property( Property4 ).isActive() );
  QCOMPARE( collection2.property( Property4 ).expressionString(), QStringLiteral( "\"field1\" + \"field2\"" ) );
  QVERIFY( collection2.hasActiveProperties() );
  QVERIFY( collection2.hasDynamicProperties() );

  // assignment operator
  QgsPropertyCollection collection3;
  collection3.setProperty( Property1, QgsProperty::fromValue( "aaaa", false ) );
  collection3 = collection;
  QCOMPARE( collection3.name(), QStringLiteral( "collection" ) );
  QCOMPARE( collection3.count(), 4 );
  QCOMPARE( collection3.property( Property1 ).propertyType(), QgsProperty::StaticProperty );
  QVERIFY( collection3.property( Property1 ).isActive() );
  QCOMPARE( collection3.property( Property1 ).staticValue(), QVariant( "v1" ) );
  QCOMPARE( collection3.property( Property2 ).propertyType(), QgsProperty::StaticProperty );
  QVERIFY( !collection3.property( Property2 ).isActive() );
  QCOMPARE( collection3.property( Property2 ).staticValue(), QVariant( "v2" ) );
  QCOMPARE( collection3.property( Property3 ).propertyType(), QgsProperty::FieldBasedProperty );
  QVERIFY( collection3.property( Property3 ).isActive() );
  QCOMPARE( collection3.property( Property3 ).field(), QStringLiteral( "field1" ) );
  QCOMPARE( collection3.property( Property4 ).propertyType(), QgsProperty::ExpressionBasedProperty );
  QVERIFY( collection3.property( Property4 ).isActive() );
  QCOMPARE( collection3.property( Property4 ).expressionString(), QStringLiteral( "\"field1\" + \"field2\"" ) );
  QVERIFY( collection3.hasActiveProperties() );
  QVERIFY( collection3.hasDynamicProperties() );

  //test hasActiveProperties() and hasDynamicProperties()
  collection3.property( Property1 ).setActive( false );
  collection3.property( Property2 ).setActive( false );
  collection3.property( Property3 ).setActive( false );
  collection3.property( Property4 ).setActive( false );
  QVERIFY( !collection3.hasActiveProperties() );
  QVERIFY( !collection3.hasDynamicProperties() );
  collection3.property( Property4 ).setActive( true );
  QVERIFY( collection3.hasDynamicProperties() );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.property( Property4 ).setActive( false );
  collection3.property( Property2 ).setActive( true );
  QVERIFY( !collection3.hasDynamicProperties() );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.property( Property2 ).setActive( false );
  QVERIFY( !collection3.hasActiveProperties() );
  collection3.setProperty( Property1, "5" );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.setProperty( Property1, QgsProperty::fromValue( "6", true ) );
  QVERIFY( collection3.hasActiveProperties() );
  collection3.setProperty( Property1, QgsProperty::fromValue( "7", false ) );
  QVERIFY( !collection3.hasActiveProperties() );
  collection3.setProperty( Property3, QVariant( "val" ) );
  QVERIFY( collection3.hasActiveProperties() );
}

void TestQgsProperty::collectionStack()
{
  //make a feature
  QgsFeature ft;
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "field1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "field2" ), QVariant::Int ) );
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
  QVERIFY( !const_cast< const QgsPropertyCollectionStack * >( &stack )->at( 0 ) );
  QVERIFY( !stack.collection( "nothing" ) );
  QVERIFY( !stack.value( Property1, context ).isValid() );
  QCOMPARE( stack.value( Property1, context, "default" ).toString(), QStringLiteral( "default" ) );
  QVERIFY( !stack.isActive( Property1 ) );
  QVERIFY( stack.referencedFields( context ).isEmpty() );
  QCOMPARE( stack.count(), 0 );
  QVERIFY( !stack.hasDynamicProperties() );
  QVERIFY( !stack.hasActiveProperties() );

  //add a collection to the stack
  QgsPropertyCollection *collection = new QgsPropertyCollection( QStringLiteral( "collection" ) );
  stack.appendCollection( collection );
  QCOMPARE( stack.count(), 1 );
  QCOMPARE( stack.at( 0 ), collection );
  QCOMPARE( const_cast< const QgsPropertyCollectionStack * >( &stack )->at( 0 ), collection );
  QVERIFY( !stack.collection( "nothing" ) );
  QCOMPARE( stack.collection( "collection" ), collection );
  QVERIFY( !stack.property( Property1 ) );
  QVERIFY( !stack.value( Property1, context ).isValid() );
  QCOMPARE( stack.value( Property1, context, "default" ).toString(), QStringLiteral( "default" ) );
  QVERIFY( !stack.isActive( Property1 ) );
  QVERIFY( !stack.hasDynamicProperties() );
  QVERIFY( !stack.hasActiveProperties() );
  QVERIFY( stack.referencedFields( context ).isEmpty() );

  //now add a property to the collection
  const QgsProperty property = QgsProperty::fromValue( "value", true );
  stack.at( 0 )->setProperty( Property1, property );
  QVERIFY( stack.isActive( Property1 ) );
  QCOMPARE( stack.property( Property1 ).value( context ), property.value( context ) );
  QCOMPARE( stack.value( Property1, context ), property.value( context ) );
  QVERIFY( !stack.hasDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );
  QVERIFY( !stack.isActive( Property2 ) );
  collection->setProperty( Property2, QgsProperty::fromValue( "value1", true ) );
  QVERIFY( stack.isActive( Property2 ) );
  QVERIFY( !stack.hasDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );

  //add a second collection
  QgsPropertyCollection *collection2 = new QgsPropertyCollection( QStringLiteral( "collection2" ) );
  stack.appendCollection( collection2 );
  QCOMPARE( stack.count(), 2 );
  QCOMPARE( stack.at( 1 ), collection2 );
  QCOMPARE( const_cast< const QgsPropertyCollectionStack * >( &stack )->at( 1 ), collection2 );
  QCOMPARE( stack.collection( "collection2" ), collection2 );
  QVERIFY( !stack.hasDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );
  const QgsProperty property2 = QgsProperty::fromValue( "value2", true );
  collection2->setProperty( Property2, property2 );
  QVERIFY( stack.isActive( Property2 ) );
  QCOMPARE( stack.property( Property2 ).value( context ), property2.value( context ) );
  QCOMPARE( stack.value( Property2, context ), property2.value( context ) );
  QVERIFY( !stack.hasDynamicProperties() );
  QVERIFY( stack.hasActiveProperties() );

  //preparation
  QVERIFY( stack.prepare( context ) );

  //test adding active property later in the stack
  const QgsProperty property3 = QgsProperty::fromValue( "value3", true );
  collection2->setProperty( Property1, property3 );
  QVERIFY( stack.isActive( Property1 ) );
  QCOMPARE( stack.property( Property1 ).value( context, "default" ), property3.value( context ) );
  QCOMPARE( stack.value( Property1, context ), property3.value( context ) );
  collection2->property( Property1 ).setActive( false );
  QCOMPARE( stack.value( Property1, context ), property.value( context ) );

  //test overriding a property
  const QgsProperty property4 = QgsProperty::fromValue( "value4", true );
  collection2->setProperty( Property2, property4 );
  QVERIFY( stack.isActive( Property2 ) );
  QCOMPARE( stack.property( Property2 ).value( context ), property4.value( context ) );
  QCOMPARE( stack.value( Property2, context ), property4.value( context ) );
  collection2->property( Property2 ).setActive( false );
  QCOMPARE( stack.property( Property2 ).value( context ), QVariant( "value1" ) );
  QCOMPARE( stack.value( Property2, context ), QVariant( "value1" ) );

  //clearing
  stack.clear();
  QCOMPARE( stack.count(), 0 );
  QVERIFY( !stack.hasDynamicProperties() );
  QVERIFY( !stack.hasActiveProperties() );

  // test copying a stack
  QgsPropertyCollectionStack stack2;
  stack2.appendCollection( new QgsPropertyCollection( QStringLiteral( "collection1" ) ) );
  stack2.at( 0 )->setProperty( Property1, "val1" );
  stack2.at( 0 )->setProperty( Property2, "val2" );
  stack2.appendCollection( new QgsPropertyCollection( QStringLiteral( "collection2" ) ) );
  stack2.at( 1 )->setProperty( Property3, "val3" );
  //copy constructor
  QgsPropertyCollectionStack stack3( stack2 );
  QCOMPARE( stack3.count(), 2 );
  QCOMPARE( stack3.at( 0 )->name(), QStringLiteral( "collection1" ) );
  QCOMPARE( stack3.at( 1 )->name(), QStringLiteral( "collection2" ) );
  QCOMPARE( stack3.at( 0 )->property( Property1 ).staticValue(), QVariant( "val1" ) );
  QCOMPARE( stack3.at( 0 )->property( Property2 ).staticValue(), QVariant( "val2" ) );
  QCOMPARE( stack3.at( 1 )->property( Property3 ).staticValue(), QVariant( "val3" ) );
  QVERIFY( !stack3.hasDynamicProperties() );
  QVERIFY( stack3.hasActiveProperties() );
  //assignment operator
  stack3.clear();
  stack3.appendCollection( new QgsPropertyCollection( QStringLiteral( "temp" ) ) );
  stack3 = stack2;
  QCOMPARE( stack3.count(), 2 );
  QCOMPARE( stack3.at( 0 )->name(), QStringLiteral( "collection1" ) );
  QCOMPARE( stack3.at( 1 )->name(), QStringLiteral( "collection2" ) );
  QCOMPARE( stack3.at( 0 )->property( Property1 ).staticValue(), QVariant( "val1" ) );
  QCOMPARE( stack3.at( 0 )->property( Property2 ).staticValue(), QVariant( "val2" ) );
  QCOMPARE( stack3.at( 1 )->property( Property3 ).staticValue(), QVariant( "val3" ) );
  QVERIFY( !stack3.hasDynamicProperties() );
  QVERIFY( stack3.hasActiveProperties() );

  //check hasDynamicProperties() and hasActiveProperties()
  QgsPropertyCollectionStack stack4;
  stack4.appendCollection( new QgsPropertyCollection( QStringLiteral( "collection1" ) ) );
  stack4.at( 0 )->setProperty( Property1, "val1" );
  QVERIFY( !stack4.hasDynamicProperties() );
  QVERIFY( stack4.hasActiveProperties() );
  stack4.at( 0 )->property( Property1 ).setActive( false );
  QVERIFY( !stack4.hasActiveProperties() );
  stack4.at( 0 )->setProperty( Property1, "6" );
  QVERIFY( stack4.hasActiveProperties() );
  stack4.at( 0 )->setProperty( Property2, QgsProperty::fromExpression( QStringLiteral( "\"field1\" + \"field2\"" ), true ) );
  QVERIFY( stack4.hasActiveProperties() );
  QVERIFY( stack4.hasDynamicProperties() );
  QCOMPARE( stack4.referencedFields( context ), QSet< QString>() << "field1" << "field2" );
  stack4.at( 0 )->property( Property1 ).setActive( false );
  QVERIFY( stack4.hasActiveProperties() );
  QVERIFY( stack4.hasDynamicProperties() );
  stack4.at( 0 )->property( Property2 ).setActive( false );
  QVERIFY( !stack4.hasActiveProperties() );
  QVERIFY( !stack4.hasDynamicProperties() );
}

void TestQgsProperty::curveTransform()
{
  const QgsCurveTransform t;
  // linear transform
  QCOMPARE( t.y( -1 ), 0.0 );
  QCOMPARE( t.y( 0 ), 0.0 );
  QCOMPARE( t.y( 0.2 ), 0.2 );
  QCOMPARE( t.y( 0.5 ), 0.5 );
  QCOMPARE( t.y( 0.8 ), 0.8 );
  QCOMPARE( t.y( 1 ), 1.0 );
  QCOMPARE( t.y( 2 ), 1.0 );

  QVector< double > x;
  x << -1 << 0 << 0.2 << 0.5 << 0.8 << 1 << 2;
  QVector< double > y = t.y( x );
  QCOMPARE( y[0], 0.0 );
  QCOMPARE( y[1], 0.0 );
  QCOMPARE( y[2], 0.2 );
  QCOMPARE( y[3], 0.5 );
  QCOMPARE( y[4], 0.8 );
  QCOMPARE( y[5], 1.0 );
  QCOMPARE( y[6], 1.0 );

  // linear transform with y =/= x
  checkCurveResult( QList< QgsPointXY >() << QgsPointXY( 0, 0.2 ) << QgsPointXY( 1.0, 0.8 ),
                    QVector< double >() << -1 << 0 << 0.2 << 0.5 << 0.8 << 1 << 2,
                    QVector< double >() << 0.2 << 0.2 << 0.32 << 0.5 << 0.68 << 0.8 << 0.8 );

  // reverse linear transform with y = -x
  checkCurveResult( QList< QgsPointXY >() << QgsPointXY( 0.0, 1.0 ) << QgsPointXY( 1.0, 0 ),
                    QVector< double >() << -1 << 0 << 0.2 << 0.5 << 0.8 << 1 << 2,
                    QVector< double >() << 1.0 << 1.0 << 0.8 << 0.5 << 0.2 << 0.0 << 0.0 );

  // OK, time for some more complex tests...

  // 3 control points, but linear
  checkCurveResult( QList< QgsPointXY >() << QgsPointXY( 0, 0.0 ) << QgsPointXY( 0.2, 0.2 ) << QgsPointXY( 1.0, 1.0 ),
                    QVector< double >() << -1 << 0 << 0.2 << 0.5 << 0.8 << 1 << 2,
                    QVector< double >() << 0.0 << 0.0 << 0.2 << 0.5 << 0.8 << 1.0 << 1.0 );

  // test for "flat" response for x outside of control point range
  checkCurveResult( QList< QgsPointXY >() << QgsPointXY( 0.2, 0.2 ) << QgsPointXY( 0.5, 0.5 ) << QgsPointXY( 0.8, 0.8 ),
                    QVector< double >() << -1 << 0 << 0.1 << 0.2 << 0.5 << 0.8 << 0.9 << 1 << 2,
                    QVector< double >() << 0.2 << 0.2 << 0.2 << 0.2 << 0.5 << 0.8 << 0.8 << 0.8 << 0.8 );

  //curves!
  checkCurveResult( QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.4, 0.6 ) << QgsPointXY( 0.6, 0.8 ) << QgsPointXY( 1.0, 1.0 ),
                    QVector< double >() << -1 << 0 << 0.2 << 0.4 << 0.5 << 0.6 << 0.8 << 0.9 << 1.0 << 2.0,
                    QVector< double >() << 0.0 << 0.0 << 0.321429 << 0.6 << 0.710714 << 0.8 << 0.921429 << 0.963393 << 1.0 << 1.0 );

  //curves with more control points
  checkCurveResult( QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.2, 0.6 ) << QgsPointXY( 0.4, 0.6 ) << QgsPointXY( 0.6, 0.8 ) << QgsPointXY( 0.8, 0.3 ) << QgsPointXY( 1.0, 1.0 ),
                    QVector< double >() << -1 << 0 << 0.2 << 0.4 << 0.5 << 0.6 << 0.8 << 0.9 << 1.0 << 2.0,
                    QVector< double >() << 0.0 << 0.0 << 0.6 << 0.6 << 0.751316 << 0.8 << 0.3 << 0.508074 << 1.0 << 1.0 );

  // general tests
  QList< QgsPointXY > points = QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.4, 0.6 ) << QgsPointXY( 0.6, 0.8 ) << QgsPointXY( 1.0, 1.0 );
  QgsCurveTransform src( points );
  QCOMPARE( src.controlPoints(), points );
  points = QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.5, 0.6 ) << QgsPointXY( 0.6, 0.8 ) << QgsPointXY( 1.0, 1.0 );
  src.setControlPoints( points );
  QCOMPARE( src.controlPoints(), points );

  src.setControlPoints( QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 1.0, 1.0 ) );
  src.addControlPoint( 0.2, 0.3 );
  src.addControlPoint( 0.1, 0.4 );
  QCOMPARE( src.controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.1, 0.4 ) << QgsPointXY( 0.2, 0.3 ) << QgsPointXY( 1.0, 1.0 ) );

  // remove non-existent point
  src.removeControlPoint( 0.6, 0.7 );
  QCOMPARE( src.controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.1, 0.4 ) << QgsPointXY( 0.2, 0.3 ) << QgsPointXY( 1.0, 1.0 ) );

  // remove valid point
  src.removeControlPoint( 0.1, 0.4 );
  QCOMPARE( src.controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.2, 0.3 ) << QgsPointXY( 1.0, 1.0 ) );

  // copy constructor
  const QgsCurveTransform dest( src );
  QCOMPARE( dest.controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.2, 0.3 ) << QgsPointXY( 1.0, 1.0 ) );
  // check a value to ensure that derivative matrix was copied OK
  QGSCOMPARENEAR( dest.y( 0.5 ), 0.1, 0.638672 );

  // assignment operator
  QgsCurveTransform dest2;
  dest2 = src;
  QCOMPARE( dest2.controlPoints(), QList< QgsPointXY >() << QgsPointXY( 0.0, 0.0 ) << QgsPointXY( 0.2, 0.3 ) << QgsPointXY( 1.0, 1.0 ) );
  QGSCOMPARENEAR( dest2.y( 0.5 ), 0.1, 0.638672 );

  // writing and reading from xml
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  QDomElement element = doc.createElement( QStringLiteral( "xform" ) );
  QVERIFY( src.writeXml( element, doc ) );

  QgsCurveTransform r1;
  QVERIFY( r1.readXml( element, doc ) );
  QCOMPARE( r1.controlPoints(), src.controlPoints() );
  QGSCOMPARENEAR( dest2.y( 0.5 ), 0.1, 0.638672 );
}

void TestQgsProperty::asVariant()
{
  const QgsProperty original = QgsProperty::fromField( QStringLiteral( "field1" ), true );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  const QgsProperty fromVar = qvariant_cast<QgsProperty>( var );
  QCOMPARE( fromVar.propertyType(), QgsProperty::FieldBasedProperty );
  QVERIFY( fromVar.isActive() );
  QCOMPARE( fromVar.field(), QStringLiteral( "field1" ) );
}

void TestQgsProperty::isProjectColor()
{
  QgsProperty p = QgsProperty::fromValue( 3, true );
  QVERIFY( !p.isProjectColor() );
  p = QgsProperty::fromField( QStringLiteral( "blah" ), true );
  QVERIFY( !p.isProjectColor() );
  p = QgsProperty::fromExpression( QStringLiteral( "1+2" ), true );
  QVERIFY( !p.isProjectColor() );
  p = QgsProperty::fromExpression( QStringLiteral( "project_color('mine')" ), true );
  QVERIFY( p.isProjectColor() );
  p = QgsProperty::fromExpression( QStringLiteral( "project_color('burnt pineapple Skin 76')" ), true );
  QVERIFY( p.isProjectColor() );
  p.setActive( false );
  QVERIFY( p.isProjectColor() );
}

void TestQgsProperty::referencedFieldsIgnoreContext()
{
  // Currently QgsProperty::referencedFields() for an expression will return field names
  // only if those field names are present in the context's fields. The ignoreContext
  // argument is a workaround for the case when we don't have fields yet.

  const QgsProperty p = QgsProperty::fromExpression( QStringLiteral( "foo + bar" ) );
  QCOMPARE( p.referencedFields( QgsExpressionContext() ), QSet<QString>() );
  QCOMPARE( p.referencedFields( QgsExpressionContext(), true ), QSet<QString>() << QStringLiteral( "foo" ) << QStringLiteral( "bar" ) );

  // if the property is from a field, the ignoreContext does not make a difference
  const QgsProperty p2 = QgsProperty::fromField( QStringLiteral( "boo" ) );
  QCOMPARE( p2.referencedFields( QgsExpressionContext() ), QSet<QString>() << QStringLiteral( "boo" ) );
  QCOMPARE( p2.referencedFields( QgsExpressionContext(), true ), QSet<QString>() << QStringLiteral( "boo" ) );

  QgsPropertyCollection collection;
  collection.setProperty( 0, p );
  collection.setProperty( 1, p2 );

  QCOMPARE( collection.referencedFields( QgsExpressionContext() ), QSet<QString>() << QStringLiteral( "boo" ) );
  QCOMPARE( collection.referencedFields( QgsExpressionContext(), true ), QSet<QString>() << QStringLiteral( "boo" ) << QStringLiteral( "foo" ) << QStringLiteral( "bar" ) );
}

void TestQgsProperty::checkCurveResult( const QList<QgsPointXY> &controlPoints, const QVector<double> &x, const QVector<double> &y )
{
  // build transform
  const QgsCurveTransform t( controlPoints );

  // we check two approaches
  for ( int i = 0; i < x.count(); ++i )
  {
    QGSCOMPARENEAR( t.y( x.at( i ) ), y.at( i ), 0.0001 );
  }

  const QVector< double > results = t.y( x );
  for ( int i = 0; i < y.count(); ++i )
  {
    QGSCOMPARENEAR( results.at( i ), y.at( i ), 0.0001 );
  }
}

void TestQgsProperty::mapToMap()
{
  const QgsProperty p1 = QgsProperty::fromExpression( "project_color('burnt marigold')" );
  const QgsProperty p2 = QgsProperty::fromValue( 1 );

  QMap<QString, QgsProperty> propertyMap;
  propertyMap.insert( "key1", p1 );
  propertyMap.insert( "key2", p2 );

  const QVariantMap variantMap = QgsProperty::propertyMapToVariantMap( propertyMap );
  QCOMPARE( variantMap.value( "key1" ).toMap().value( "expression" ).toString(), "project_color('burnt marigold')" );
  QCOMPARE( variantMap.value( "key2" ).toMap().value( "val" ).toInt(), 1 );

  QCOMPARE( QgsProperty::variantMapToPropertyMap( variantMap ), propertyMap );
}

QGSTEST_MAIN( TestQgsProperty )
#include "testqgsproperty.moc"
