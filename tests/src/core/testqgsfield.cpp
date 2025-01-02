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
#include "qgsreferencedgeometry.h"
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QLocale>

#include <memory>

#include "qgsfield.h"
#include "qgsapplication.h"
#include "qgstest.h"
#include "qgsreferencedgeometry.h"

class TestQgsField : public QObject
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
    void gettersSetters(); //test getters and setters
    void isNumeric();      //test isNumeric
    void isDateTime();     //test isNumeric
    void equality();       //test equality operators
    void asVariant();      //test conversion to and from a QVariant
    void displayString();
    void convertCompatible();
    void dataStream();
    void displayName();
    void displayNameWithAlias();
    void displayType();
    void friendlyTypeString();
    void editorWidgetSetup();
    void collection();

  private:
};

void TestQgsField::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
}

void TestQgsField::cleanupTestCase()
{
}

void TestQgsField::init()
{
  QLocale::setDefault( QLocale::English );
}

void TestQgsField::cleanup()
{
  QLocale::setDefault( QLocale::English );
}

void TestQgsField::create()
{
  std::unique_ptr<QgsField> field( new QgsField( QStringLiteral( "name" ), QMetaType::Type::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) ) );
  QCOMPARE( field->name(), QString( "name" ) );
  QCOMPARE( field->type(), QMetaType::Type::Double );
  QCOMPARE( field->typeName(), QString( "double" ) );
  QCOMPARE( field->length(), 5 );
  QCOMPARE( field->precision(), 2 );
  QCOMPARE( field->comment(), QString( "comment" ) );
  QCOMPARE( field->isReadOnly(), false );
  QCOMPARE( field->splitPolicy(), Qgis::FieldDomainSplitPolicy::Duplicate );
}

void TestQgsField::copy()
{
  QgsField original( QStringLiteral( "original" ), QMetaType::Type::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  original.setConstraints( constraints );
  original.setReadOnly( true );
  original.setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
  original.setDuplicatePolicy( Qgis::FieldDuplicatePolicy::UnsetField );
  original.setMetadata( { { 1, QStringLiteral( "abc" ) }, { 2, 5 } } );

  QVariantMap config;
  config.insert( QStringLiteral( "a" ), "value_a" );
  const QgsEditorWidgetSetup setup( QStringLiteral( "test" ), config );
  original.setEditorWidgetSetup( setup );

  QgsField copy( original );
  QVERIFY( copy == original );
  QCOMPARE( copy.editorWidgetSetup().type(), original.editorWidgetSetup().type() );
  QCOMPARE( copy.editorWidgetSetup().config(), original.editorWidgetSetup().config() );

  copy.setName( QStringLiteral( "copy" ) );
  QCOMPARE( original.name(), QString( "original" ) );
  QVERIFY( copy != original );
}

void TestQgsField::assignment()
{
  QgsField original( QStringLiteral( "original" ), QMetaType::Type::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  original.setConstraints( constraints );
  original.setReadOnly( true );
  original.setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
  original.setDuplicatePolicy( Qgis::FieldDuplicatePolicy::UnsetField );
  original.setMetadata( { { 1, QStringLiteral( "abc" ) }, { 2, 5 } } );
  QgsField copy;
  copy = original;
  QVERIFY( copy == original );

  copy.setName( QStringLiteral( "copy" ) );
  QCOMPARE( original.name(), QString( "original" ) );
  QVERIFY( copy != original );
}

void TestQgsField::gettersSetters()
{
  QgsField field;
  field.setName( QStringLiteral( "name" ) );
  QCOMPARE( field.name(), QString( "name" ) );
  field.setType( QMetaType::Type::Int );
  QCOMPARE( field.type(), QMetaType::Type::Int );
  field.setTypeName( QStringLiteral( "typeName" ) );
  QCOMPARE( field.typeName(), QString( "typeName" ) );
  field.setLength( 5 );
  QCOMPARE( field.length(), 5 );
  field.setPrecision( 2 );
  QCOMPARE( field.precision(), 2 );
  field.setComment( QStringLiteral( "comment" ) );
  QCOMPARE( field.comment(), QString( "comment" ) );
  field.setAlias( QStringLiteral( "alias" ) );
  QCOMPARE( field.alias(), QString( "alias" ) );
  field.setDefaultValueDefinition( QgsDefaultValue( QStringLiteral( "1+2" ) ) );
  QCOMPARE( field.defaultValueDefinition().expression(), QString( "1+2" ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setDomainName( QStringLiteral( "domain" ) );
  field.setConstraints( constraints );
  QCOMPARE( field.constraints().constraints(), QgsFieldConstraints::ConstraintNotNull );
  QCOMPARE( field.constraints().constraintOrigin( QgsFieldConstraints::ConstraintNotNull ), QgsFieldConstraints::ConstraintOriginProvider );
  QCOMPARE( field.constraints().constraintOrigin( QgsFieldConstraints::ConstraintUnique ), QgsFieldConstraints::ConstraintOriginNotSet );
  QCOMPARE( field.constraints().domainName(), QStringLiteral( "domain" ) );
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginNotSet );
  field.setConstraints( constraints );
  QCOMPARE( field.constraints().constraints(), 0 );
  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique );
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull );
  field.setConstraints( constraints );
  QCOMPARE( field.constraints().constraints(), QgsFieldConstraints::ConstraintUnique | QgsFieldConstraints::ConstraintNotNull );
  constraints.removeConstraint( QgsFieldConstraints::ConstraintNotNull );
  field.setConstraints( constraints );
  QCOMPARE( field.constraints().constraints(), QgsFieldConstraints::ConstraintUnique );

  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );
  field.setConstraints( constraints );
  QCOMPARE( field.constraints().constraintExpression(), QStringLiteral( "constraint expression" ) );
  QCOMPARE( field.constraints().constraintDescription(), QStringLiteral( "description" ) );
  QCOMPARE( field.constraints().constraints(), QgsFieldConstraints::ConstraintUnique | QgsFieldConstraints::ConstraintExpression ); //setting constraint expression should add constraint
  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );

  // check a constraint strength which hasn't been set
  QCOMPARE( field.constraints().constraintStrength( QgsFieldConstraints::ConstraintNotNull ), QgsFieldConstraints::ConstraintStrengthNotSet );
  // check a constraint strength which has not been explicitly set
  QCOMPARE( field.constraints().constraintStrength( QgsFieldConstraints::ConstraintUnique ), QgsFieldConstraints::ConstraintStrengthHard );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthSoft );
  field.setConstraints( constraints );
  QCOMPARE( field.constraints().constraintStrength( QgsFieldConstraints::ConstraintUnique ), QgsFieldConstraints::ConstraintStrengthSoft );
  // try overwriting a provider constraint's strength
  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthSoft );
  field.setConstraints( constraints );
  QCOMPARE( field.constraints().constraintStrength( QgsFieldConstraints::ConstraintUnique ), QgsFieldConstraints::ConstraintStrengthSoft );

  field.setReadOnly( true );
  QCOMPARE( field.isReadOnly(), true );

  field.setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
  QCOMPARE( field.splitPolicy(), Qgis::FieldDomainSplitPolicy::GeometryRatio );

  field.setDuplicatePolicy( Qgis::FieldDuplicatePolicy::UnsetField );
  QCOMPARE( field.duplicatePolicy(), Qgis::FieldDuplicatePolicy::UnsetField );

  field.setMetadata( { { static_cast<int>( Qgis::FieldMetadataProperty::GeometryCrs ), QStringLiteral( "abc" ) }, { 2, 5 } } );
  QMap<int, QVariant> expected { { static_cast<int>( Qgis::FieldMetadataProperty::GeometryCrs ), QStringLiteral( "abc" ) }, { 2, 5 } };
  QCOMPARE( field.metadata(), expected );
  QVERIFY( !field.metadata( Qgis::FieldMetadataProperty::GeometryWkbType ).isValid() );
  QCOMPARE( field.metadata( Qgis::FieldMetadataProperty::GeometryCrs ).toString(), QStringLiteral( "abc" ) );
  field.setMetadata( Qgis::FieldMetadataProperty::GeometryWkbType, QStringLiteral( "def" ) );
  QCOMPARE( field.metadata( Qgis::FieldMetadataProperty::GeometryWkbType ).toString(), QStringLiteral( "def" ) );

  expected = QMap<int, QVariant> { { static_cast<int>( Qgis::FieldMetadataProperty::GeometryCrs ), QStringLiteral( "abc" ) }, { 2, 5 }, { static_cast<int>( Qgis::FieldMetadataProperty::GeometryWkbType ), QStringLiteral( "def" ) } };
  QCOMPARE( field.metadata(), expected );
}

void TestQgsField::isNumeric()
{
  QgsField field;
  field.setType( QMetaType::Type::Int );
  QVERIFY( field.isNumeric() );
  field.setType( QMetaType::Type::UInt );
  QVERIFY( field.isNumeric() );
  field.setType( QMetaType::Type::Double );
  QVERIFY( field.isNumeric() );
  field.setType( QMetaType::Type::LongLong );
  QVERIFY( field.isNumeric() );
  field.setType( QMetaType::Type::ULongLong );
  QVERIFY( field.isNumeric() );
  field.setType( QMetaType::Type::QString );
  QVERIFY( !field.isNumeric() );
  field.setType( QMetaType::Type::QDateTime );
  QVERIFY( !field.isNumeric() );
  field.setType( QMetaType::Type::Bool );
  QVERIFY( !field.isNumeric() );
  field.setType( QMetaType::Type::UnknownType );
  QVERIFY( !field.isNumeric() );
}

void TestQgsField::isDateTime()
{
  QgsField field;
  field.setType( QMetaType::Type::Int );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QMetaType::Type::UInt );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QMetaType::Type::Double );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QMetaType::Type::LongLong );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QMetaType::Type::ULongLong );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QMetaType::Type::QString );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QMetaType::Type::QDateTime );
  QVERIFY( field.isDateOrTime() );
  field.setType( QMetaType::Type::QTime );
  QVERIFY( field.isDateOrTime() );
  field.setType( QMetaType::Type::QDate );
  QVERIFY( field.isDateOrTime() );
  field.setType( QMetaType::Type::Bool );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QMetaType::Type::UnknownType );
  QVERIFY( !field.isDateOrTime() );
}

void TestQgsField::equality()
{
  QgsField field1;
  field1.setName( QStringLiteral( "name" ) );
  field1.setType( QMetaType::Type::Int );
  field1.setLength( 5 );
  field1.setPrecision( 2 );
  field1.setTypeName( QStringLiteral( "typename1" ) ); //typename is NOT required for equality
  field1.setComment( QStringLiteral( "comment1" ) );   //comment is NOT required for equality
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  field1.setConstraints( constraints );
  QgsField field2;
  field2.setName( QStringLiteral( "name" ) );
  field2.setType( QMetaType::Type::Int );
  field2.setLength( 5 );
  field2.setPrecision( 2 );
  field2.setTypeName( QStringLiteral( "typename2" ) ); //typename is NOT required for equality
  field2.setComment( QStringLiteral( "comment2" ) );   //comment is NOT required for equality
  constraints = field2.constraints();
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  field2.setConstraints( constraints );
  QVERIFY( field1 == field2 );
  QVERIFY( !( field1 != field2 ) );

  //test that all applicable components contribute to equality
  field2.setName( QStringLiteral( "name2" ) );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setName( QStringLiteral( "name" ) );
  field2.setType( QMetaType::Type::Double );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setType( QMetaType::Type::Int );
  field2.setLength( 9 );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setLength( 5 );
  field2.setPrecision( 9 );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setPrecision( 2 );
  field2.setAlias( QStringLiteral( "alias " ) );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setAlias( QString() );
  field2.setReadOnly( true );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setReadOnly( false );
  field2.setDefaultValueDefinition( QgsDefaultValue( QStringLiteral( "1+2" ) ) );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setDefaultValueDefinition( QgsDefaultValue() );
  constraints = field2.constraints();
  constraints.removeConstraint( QgsFieldConstraints::ConstraintNotNull );
  field2.setConstraints( constraints );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  constraints = field2.constraints();
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginLayer );
  field2.setConstraints( constraints );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  constraints = field2.constraints();
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraintExpression( QStringLiteral( "exp" ) );
  field2.setConstraints( constraints );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  constraints = field2.constraints();
  constraints.setConstraintExpression( QStringLiteral( "exp" ), QStringLiteral( "desc" ) );
  field2.setConstraints( constraints );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  constraints = QgsFieldConstraints();
  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard );
  field2.setConstraints( constraints );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthSoft );
  field1.setConstraints( constraints );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setConstraints( constraints );

  field1.setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
  QVERIFY( field1 == field2 );

  field1.setDuplicatePolicy( Qgis::FieldDuplicatePolicy::UnsetField );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setDuplicatePolicy( Qgis::FieldDuplicatePolicy::UnsetField );
  QVERIFY( field1 == field2 );

  field1.setMetadata( { { static_cast<int>( Qgis::FieldMetadataProperty::GeometryCrs ), QStringLiteral( "abc" ) }, { 2, 5 } } );
  QVERIFY( !( field1 == field2 ) );
  QVERIFY( field1 != field2 );
  field2.setMetadata( { { static_cast<int>( Qgis::FieldMetadataProperty::GeometryCrs ), QStringLiteral( "abc" ) }, { 2, 5 } } );
  QVERIFY( field1 == field2 );
  QVERIFY( !( field1 != field2 ) );

  QgsFieldConstraints constraints1;
  QgsFieldConstraints constraints2;
  constraints1.setDomainName( QStringLiteral( "d" ) );
  QVERIFY( !( constraints1 == constraints2 ) );
  constraints2.setDomainName( QStringLiteral( "d" ) );
  QVERIFY( constraints1 == constraints2 );

  QgsEditorWidgetSetup setup1 { QStringLiteral( "TextEdit" ), QVariantMap() };
  QgsEditorWidgetSetup setup2 { QStringLiteral( "TextEdit" ), QVariantMap() };

  field1.setEditorWidgetSetup( setup1 );
  field2.setEditorWidgetSetup( setup2 );
  QVERIFY( field1 == field2 );

  setup2 = QgsEditorWidgetSetup { QStringLiteral( "Text" ), QVariantMap() };
  field2.setEditorWidgetSetup( setup2 );
  QVERIFY( field1 != field2 );
  setup1 = QgsEditorWidgetSetup { QStringLiteral( "Text" ), QVariantMap() };
  field1.setEditorWidgetSetup( setup1 );
  QVERIFY( field1 == field2 );

  setup1 = QgsEditorWidgetSetup { QStringLiteral( "TextEdit" ), QVariantMap { { QStringLiteral( "a" ), QStringLiteral( "b" ) } } };
  setup2 = QgsEditorWidgetSetup { QStringLiteral( "TextEdit" ), QVariantMap { { QStringLiteral( "a" ), QStringLiteral( "b" ) } } };
  field1.setEditorWidgetSetup( setup1 );
  field2.setEditorWidgetSetup( setup2 );
  QVERIFY( field1 == field2 );

  setup2 = QgsEditorWidgetSetup { QStringLiteral( "TextEdit" ), QVariantMap { { QStringLiteral( "a" ), QStringLiteral( "XXXXXX" ) } } };
  field2.setEditorWidgetSetup( setup2 );
  QVERIFY( field1 != field2 );
}

void TestQgsField::asVariant()
{
  QgsField original( QStringLiteral( "original" ), QMetaType::Type::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull );
  original.setConstraints( constraints );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  const QgsField fromVar = qvariant_cast<QgsField>( var );
  QCOMPARE( fromVar, original );
}

void TestQgsField::displayString()
{
  const QgsField stringField( QStringLiteral( "string" ), QMetaType::Type::QString, QStringLiteral( "string" ) );

  //test string value
  const QString test( QStringLiteral( "test string" ) );
  QCOMPARE( stringField.displayString( test ), test );

  //test NULL
  QgsApplication::setNullRepresentation( QStringLiteral( "TEST NULL" ) );
  const QVariant nullString = QgsVariantUtils::createNullVariant( QMetaType::Type::QString );
  QCOMPARE( stringField.displayString( nullString ), QString( "TEST NULL" ) );

  //test int value in string type
  const QgsField intField( QStringLiteral( "int" ), QMetaType::Type::QString, QStringLiteral( "int" ) );
  QCOMPARE( intField.displayString( 5 ), QString( "5" ) );
  QCOMPARE( intField.displayString( 599999898999LL ), QString( "599999898999" ) );

  //test int value in int type
  const QgsField intField2( QStringLiteral( "int" ), QMetaType::Type::Int, QStringLiteral( "int" ) );
  QCOMPARE( intField2.displayString( 5 ), QString( "5" ) );
  QCOMPARE( intField2.displayString( 599999898999LL ), QString( "599,999,898,999" ) );

  //test long type
  const QgsField longField( QStringLiteral( "long" ), QMetaType::Type::LongLong, QStringLiteral( "longlong" ) );
  QCOMPARE( longField.displayString( 5 ), QString( "5" ) );
  QCOMPARE( longField.displayString( 599999898999LL ), QString( "599,999,898,999" ) );

  //test NULL int
  const QVariant nullInt = QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  QCOMPARE( intField.displayString( nullInt ), QString( "TEST NULL" ) );

  //test double value
  const QgsField doubleField( QStringLiteral( "double" ), QMetaType::Type::Double, QStringLiteral( "double" ), 10, 3 );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5.005" ) );
  QCOMPARE( doubleField.displayString( 4.5e-09 ).toLower(), QString( "4.5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0.0001" ) );
  QCOMPARE( doubleField.displayString( -5.005005 ), QString( "-5.005" ) );
  QCOMPARE( doubleField.displayString( -4.5e-09 ).toLower(), QString( "-4.5e-09" ) );
  QCOMPARE( doubleField.displayString( -1e-04 ), QString( "-0.0001" ) );
  const QgsField doubleFieldNoPrec( QStringLiteral( "double" ), QMetaType::Type::Double, QStringLiteral( "double" ), 10 );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5.005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5.005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ).toLower(), QString( "4.5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-08 ).toLower(), QString( "4.5e-08" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-07 ).toLower(), QString( "4.5e-07" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-06 ).toLower(), QString( "4.5e-06" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-05 ).toLower(), QString( "4.5e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-05 ).toLower(), QString( "1e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5e-05 ).toLower(), QString( "5e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-04 ), QString( "0.0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-03 ), QString( "0.001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-02 ), QString( "0.01" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-01 ), QString( "0.1" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -5.005005 ), QString( "-5.005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -5.005005005 ), QString( "-5.005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -4.5e-09 ).toLower(), QString( "-4.5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -4.5e-08 ).toLower(), QString( "-4.5e-08" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -4.5e-07 ).toLower(), QString( "-4.5e-07" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -4.5e-06 ).toLower(), QString( "-4.5e-06" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -4.5e-05 ).toLower(), QString( "-4.5e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -1e-05 ).toLower(), QString( "-1e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -5e-05 ).toLower(), QString( "-5e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -1e-04 ), QString( "-0.0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -1e-03 ), QString( "-0.001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -1e-02 ), QString( "-0.01" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( -1e-01 ), QString( "-0.1" ) );
  QCOMPARE( QLocale().numberOptions() & QLocale::NumberOption::OmitGroupSeparator, QLocale::NumberOption::DefaultNumberOptions );
  QCOMPARE( doubleFieldNoPrec.displayString( 599999898999.0 ), QString( "599,999,898,999" ) );

  // no conversion when this is default value expression
  QCOMPARE( doubleFieldNoPrec.displayString( ( "(1+2)" ) ), QString( "(1+2)" ) );

  //test NULL double
  const QVariant nullDouble = QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
  QCOMPARE( doubleField.displayString( nullDouble ), QString( "TEST NULL" ) );

  //test double value with German locale
  QLocale::setDefault( QLocale::German );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5,005" ) );
  QCOMPARE( doubleField.displayString( 4.5e-09 ).toLower(), QString( "4,5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0,0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5,005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5,005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ).toLower(), QString( "4,5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-04 ), QString( "0,0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 599999898999.0 ), QString( "599.999.898.999" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5999.123456 ), QString( "5.999,123456" ) );

  //test value with custom German locale (OmitGroupSeparator)
  QLocale customGerman( QLocale::German );
  customGerman.setNumberOptions( QLocale::NumberOption::OmitGroupSeparator );
  QLocale::setDefault( customGerman );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5,005" ) );
  QCOMPARE( doubleField.displayString( 4.5e-09 ).toLower(), QString( "4,5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0,0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5,005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5,005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ).toLower(), QString( "4,5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-04 ), QString( "0,0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 599999898999.0 ), QString( "599999898999" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5999.123456 ), QString( "5999,123456" ) );

  //test int value in int type with custom German locale (OmitGroupSeparator)
  QCOMPARE( intField2.displayString( 5 ), QString( "5" ) );
  QCOMPARE( intField2.displayString( 599999898999LL ), QString( "599999898999" ) );

  //test long type with custom German locale (OmitGroupSeparator)
  QCOMPARE( longField.displayString( 5 ), QString( "5" ) );
  QCOMPARE( longField.displayString( 599999898999LL ), QString( "599999898999" ) );

  //test value with custom english locale (OmitGroupSeparator)
  QLocale customEnglish( QLocale::English );
  customEnglish.setNumberOptions( QLocale::NumberOption::OmitGroupSeparator );
  QLocale::setDefault( customEnglish );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5.005" ) );
  QCOMPARE( doubleField.displayString( 4.5e-09 ).toLower(), QString( "4.5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0.0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5.005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5.005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ).toLower(), QString( "4.5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-08 ).toLower(), QString( "4.5e-08" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-07 ).toLower(), QString( "4.5e-07" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-06 ).toLower(), QString( "4.5e-06" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-05 ).toLower(), QString( "4.5e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-05 ).toLower(), QString( "1e-05" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-04 ), QString( "0.0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-03 ), QString( "0.001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-02 ), QString( "0.01" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-01 ), QString( "0.1" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 599999898999.0 ), QString( "599999898999" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5999.123456 ), QString( "5999.123456" ) );

  //test int value in int type with custom english locale (OmitGroupSeparator)
  QCOMPARE( intField2.displayString( 5 ), QString( "5" ) );
  QCOMPARE( intField2.displayString( 599999898999LL ), QString( "599999898999" ) );

  //test long type with custom english locale (OmitGroupSeparator)
  QCOMPARE( longField.displayString( 5 ), QString( "5" ) );
  QCOMPARE( longField.displayString( 599999898999LL ), QString( "599999898999" ) );

  // binary field
  const QgsField binaryField( QStringLiteral( "binary" ), QMetaType::Type::QByteArray, QStringLiteral( "Binary" ) );
  const QString testBAString( QStringLiteral( "test string" ) );
  const QByteArray testBA( testBAString.toLocal8Bit() );
  QCOMPARE( binaryField.displayString( testBA ), QStringLiteral( "BLOB" ) );

  // array field
  const QgsField stringArrayField( QStringLiteral( "stringArray" ), QMetaType::Type::QStringList, QStringLiteral( "StringArray" ) );
  QCOMPARE( stringArrayField.displayString( QStringList() << "A" << "B" << "C" ), QStringLiteral( "A, B, C" ) );
  const QgsField intArrayField( QStringLiteral( "intArray" ), QMetaType::Type::QVariantList, QStringLiteral( "IntArray" ) );
  QCOMPARE( intArrayField.displayString( QVariantList() << 1 << 2 << 3 ), QStringLiteral( "1, 2, 3" ) );
}

void TestQgsField::convertCompatible()
{
  //test string field
  const QgsField stringField( QStringLiteral( "string" ), QMetaType::Type::QString, QStringLiteral( "string" ) );

  QVariant stringVar( "test string" );
  QVERIFY( stringField.convertCompatible( stringVar ) );
  QCOMPARE( stringVar.toString(), QString( "test string" ) );
  QVariant nullString = QgsVariantUtils::createNullVariant( QMetaType::Type::QString );
  QVERIFY( stringField.convertCompatible( nullString ) );
  QCOMPARE( static_cast<QMetaType::Type>( nullString.userType() ), QMetaType::Type::QString );
  QVERIFY( nullString.isNull() );
  QVariant intVar( 5 );
  QVERIFY( stringField.convertCompatible( intVar ) );
  QCOMPARE( static_cast<QMetaType::Type>( intVar.userType() ), QMetaType::Type::QString );
  QCOMPARE( intVar, QVariant( "5" ) );
  QVariant nullInt = QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  QVERIFY( stringField.convertCompatible( nullInt ) );
  QCOMPARE( static_cast<QMetaType::Type>( nullInt.userType() ), QMetaType::Type::QString );
  QVERIFY( nullInt.isNull() );
  QVariant doubleVar( 1.25 );
  QVERIFY( stringField.convertCompatible( doubleVar ) );
  QCOMPARE( static_cast<QMetaType::Type>( doubleVar.userType() ), QMetaType::Type::QString );
  QCOMPARE( doubleVar, QVariant( "1.25" ) );
  QVariant nullDouble = QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
  QVERIFY( stringField.convertCompatible( nullDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( nullDouble.userType() ), QMetaType::Type::QString );
  QVERIFY( nullDouble.isNull() );

  //test double
  const QgsField doubleField( QStringLiteral( "double" ), QMetaType::Type::Double, QStringLiteral( "double" ) );

  stringVar = QVariant( "test string" );
  QString error;
  QVERIFY( !doubleField.convertCompatible( stringVar, &error ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringVar.userType() ), QMetaType::Type::Double );
  QCOMPARE( error, QStringLiteral( "Could not convert value \"test string\" to target type \"double\"" ) );
  stringVar = QVariant( "test string" );
  QVERIFY( !doubleField.convertCompatible( stringVar ) );
  QVERIFY( stringVar.isNull() );
  nullString = QgsVariantUtils::createNullVariant( QMetaType::Type::QString );
  QVERIFY( doubleField.convertCompatible( nullString, &error ) );
  QVERIFY( error.isEmpty() );
  QCOMPARE( static_cast<QMetaType::Type>( nullString.userType() ), QMetaType::Type::Double );
  QVERIFY( nullString.isNull() );
  intVar = QVariant( 5 );
  QVERIFY( doubleField.convertCompatible( intVar ) );
  QCOMPARE( static_cast<QMetaType::Type>( intVar.userType() ), QMetaType::Type::Double );
  QCOMPARE( intVar, QVariant( 5.0 ) );
  nullInt = QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  QVERIFY( doubleField.convertCompatible( nullInt ) );
  QCOMPARE( static_cast<QMetaType::Type>( nullInt.userType() ), QMetaType::Type::Double );
  QVERIFY( nullInt.isNull() );
  doubleVar = QVariant( 1.25 );
  QVERIFY( doubleField.convertCompatible( doubleVar ) );
  QCOMPARE( static_cast<QMetaType::Type>( doubleVar.userType() ), QMetaType::Type::Double );
  QCOMPARE( doubleVar, QVariant( 1.25 ) );
  nullDouble = QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
  QVERIFY( doubleField.convertCompatible( nullDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( nullDouble.userType() ), QMetaType::Type::Double );
  QVERIFY( nullDouble.isNull() );

  //test special rules

  //conversion of double to int
  QgsField intField( QStringLiteral( "int" ), QMetaType::Type::Int, QStringLiteral( "int" ) );
  //small double, should be rounded
  QVariant smallDouble( 45.7 );
  QVERIFY( intField.convertCompatible( smallDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( smallDouble.userType() ), QMetaType::Type::Int );
  QCOMPARE( smallDouble, QVariant( 46 ) );
  QVariant negativeSmallDouble( -9345.754534525235235 );
  QVERIFY( intField.convertCompatible( negativeSmallDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( negativeSmallDouble.userType() ), QMetaType::Type::Int );
  QCOMPARE( negativeSmallDouble, QVariant( -9346 ) );
  //large double, cannot be converted
  QVariant largeDouble( 9999999999.99 );
  QVERIFY( !intField.convertCompatible( largeDouble, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"10000000000\" is too large for integer field" ) );
  largeDouble = QVariant( 9999999999.99 );
  QVERIFY( !intField.convertCompatible( largeDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( largeDouble.userType() ), QMetaType::Type::Int );
  QVERIFY( largeDouble.isNull() );

  //conversion of string double value to int
  QVariant notNumberString( "notanumber" );
  QVERIFY( !intField.convertCompatible( notNumberString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"notanumber\" is not a number" ) );
  notNumberString = QVariant( "notanumber" );
  QVERIFY( !intField.convertCompatible( notNumberString ) );
  QCOMPARE( static_cast<QMetaType::Type>( notNumberString.userType() ), QMetaType::Type::Int );
  QVERIFY( notNumberString.isNull() );
  //small double, should be rounded
  QVariant smallDoubleString( "45.7" );
  QVERIFY( intField.convertCompatible( smallDoubleString ) );
  QCOMPARE( static_cast<QMetaType::Type>( smallDoubleString.userType() ), QMetaType::Type::Int );
  QCOMPARE( smallDoubleString, QVariant( 46 ) );
  QVariant negativeSmallDoubleString( "-9345.754534525235235" );
  QVERIFY( intField.convertCompatible( negativeSmallDoubleString ) );
  QCOMPARE( static_cast<QMetaType::Type>( negativeSmallDoubleString.userType() ), QMetaType::Type::Int );
  QCOMPARE( negativeSmallDoubleString, QVariant( -9346 ) );
  //large double, cannot be converted
  QVariant largeDoubleString( "9999999999.99" );
  QVERIFY( !intField.convertCompatible( largeDoubleString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"1e+10\" is too large for integer field" ) );
  largeDoubleString = QVariant( "9999999999.99" );
  QVERIFY( !intField.convertCompatible( largeDoubleString ) );
  QCOMPARE( static_cast<QMetaType::Type>( largeDoubleString.userType() ), QMetaType::Type::Int );
  QVERIFY( largeDoubleString.isNull() );

  //conversion of longlong to int
  QVariant longlong( 99999999999999999LL );
  QVERIFY( !intField.convertCompatible( longlong, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"99999999999999999\" is too large for integer field" ) );
  QCOMPARE( static_cast<QMetaType::Type>( longlong.userType() ), QMetaType::Type::Int );
  QVERIFY( longlong.isNull() );
  QVariant smallLonglong( 99LL );
  QVERIFY( intField.convertCompatible( smallLonglong ) );
  QCOMPARE( static_cast<QMetaType::Type>( smallLonglong.userType() ), QMetaType::Type::Int );
  QCOMPARE( smallLonglong, QVariant( 99 ) );
  // negative longlong to int
  QVariant negativeLonglong( -99999999999999999LL );
  QVERIFY( !intField.convertCompatible( negativeLonglong, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"-99999999999999999\" is too large for integer field" ) );
  QCOMPARE( static_cast<QMetaType::Type>( negativeLonglong.userType() ), QMetaType::Type::Int );
  QVERIFY( negativeLonglong.isNull() );
  // small negative longlong to int
  QVariant smallNegativeLonglong( -99LL );
  QVERIFY( intField.convertCompatible( smallNegativeLonglong ) );
  QCOMPARE( static_cast<QMetaType::Type>( smallNegativeLonglong.userType() ), QMetaType::Type::Int );
  QCOMPARE( smallNegativeLonglong, QVariant( -99 ) );

  //string representation of an int
  QVariant stringInt( "123456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringInt.userType() ), QMetaType::Type::Int );
  QCOMPARE( stringInt, QVariant( 123456 ) );
  // now with group separator for english locale
  stringInt = QVariant( "123,456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringInt.userType() ), QMetaType::Type::Int );
  QCOMPARE( stringInt, QVariant( "123456" ) );

  //conversion of longlong to longlong field
  const QgsField longlongField( QStringLiteral( "long" ), QMetaType::Type::LongLong, QStringLiteral( "longlong" ) );
  longlong = QVariant( 99999999999999999LL );
  QVERIFY( longlongField.convertCompatible( longlong ) );
  QCOMPARE( static_cast<QMetaType::Type>( longlong.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( longlong, QVariant( 99999999999999999LL ) );

  //string representation of a longlong
  QVariant stringLong( "99999999999999999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringLong.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );
  // now with group separator for english locale
  stringLong = QVariant( "99,999,999,999,999,999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringLong.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );

  //conversion of string double value to longlong
  notNumberString = QVariant( "notanumber" );
  QVERIFY( !longlongField.convertCompatible( notNumberString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"notanumber\" is not a number" ) );
  QCOMPARE( static_cast<QMetaType::Type>( notNumberString.userType() ), QMetaType::Type::LongLong );
  QVERIFY( notNumberString.isNull() );
  //small double, should be rounded
  smallDoubleString = QVariant( "45.7" );
  QVERIFY( longlongField.convertCompatible( smallDoubleString ) );
  QCOMPARE( static_cast<QMetaType::Type>( smallDoubleString.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( smallDoubleString, QVariant( 46 ) );
  negativeSmallDoubleString = QVariant( "-9345.754534525235235" );
  QVERIFY( longlongField.convertCompatible( negativeSmallDoubleString ) );
  QCOMPARE( static_cast<QMetaType::Type>( negativeSmallDoubleString.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( negativeSmallDoubleString, QVariant( -9346 ) );
  //large double, can be converted
  largeDoubleString = QVariant( "9999999999.99" );
  QVERIFY( longlongField.convertCompatible( largeDoubleString ) );
  QCOMPARE( static_cast<QMetaType::Type>( largeDoubleString.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( largeDoubleString, QVariant( 10000000000LL ) );
  //extra large double, cannot be converted
  largeDoubleString = QVariant( "999999999999999999999.99" );
  QVERIFY( !longlongField.convertCompatible( largeDoubleString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"1e+21\" is too large for long long field" ) );
  QCOMPARE( static_cast<QMetaType::Type>( largeDoubleString.userType() ), QMetaType::Type::LongLong );
  QVERIFY( largeDoubleString.isNull() );

  //string representation of a double
  QVariant stringDouble( "123456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringDouble.userType() ), QMetaType::Type::Double );
  QCOMPARE( stringDouble, QVariant( 123456.012345 ) );
  // now with group separator for english locale
  stringDouble = QVariant( "1,223,456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringDouble.userType() ), QMetaType::Type::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );
  // This should not convert
  stringDouble = QVariant( "1.223.456,012345" );
  QVERIFY( !doubleField.convertCompatible( stringDouble, &error ) );
  QCOMPARE( error, QStringLiteral( "Could not convert value \"1.223.456,012345\" to target type \"double\"" ) );

  //double with precision
  const QgsField doubleWithPrecField( QStringLiteral( "double" ), QMetaType::Type::Double, QStringLiteral( "double" ), 10, 3 );
  doubleVar = QVariant( 10.12345678 );
  //note - this returns true!
  QVERIFY( doubleWithPrecField.convertCompatible( doubleVar ) );
  QCOMPARE( static_cast<QMetaType::Type>( doubleVar.userType() ), QMetaType::Type::Double );
  QCOMPARE( doubleVar.toDouble(), 10.123 );

  //truncating string length
  const QgsField stringWithLen( QStringLiteral( "string" ), QMetaType::Type::QString, QStringLiteral( "string" ), 3 );
  stringVar = QVariant( "longstring" );
  QVERIFY( !stringWithLen.convertCompatible( stringVar, &error ) );
  QCOMPARE( error, QStringLiteral( "String of length 10 exceeds maximum field length (3)" ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringVar.userType() ), QMetaType::Type::QString );
  QCOMPARE( stringVar.toString(), QString( "lon" ) );

  // Referenced geometries
  const QgsField stringGeomRef( QStringLiteral( "string" ), QMetaType::Type::QString, QStringLiteral( "string" ) );
  QgsGeometry geom { QgsGeometry::fromWkt( "POINT( 1 1 )" ) };
  QgsReferencedGeometry geomRef { geom, QgsCoordinateReferenceSystem() };
  QVariant geomVar = QVariant::fromValue( geomRef );
  QVERIFY( stringGeomRef.convertCompatible( geomVar, &error ) );
  QCOMPARE( static_cast<QMetaType::Type>( geomVar.userType() ), QMetaType::Type::QString );
  QCOMPARE( geomVar.toString().toUpper(), QString( "POINT (1 1)" ) );

  /////////////////////////////////////////////////////////
  // German locale tests

  //double with ',' as decimal separator for German locale
  QLocale::setDefault( QLocale::German );
  QVariant doubleCommaVar( "1,2345" );
  QVERIFY( doubleField.convertCompatible( doubleCommaVar ) );
  QCOMPARE( static_cast<QMetaType::Type>( doubleCommaVar.userType() ), QMetaType::Type::Double );
  QCOMPARE( doubleCommaVar.toString(), QString( "1.2345" ) );

  //string representation of an int
  stringInt = QVariant( "123456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringInt.userType() ), QMetaType::Type::Int );
  QCOMPARE( stringInt, QVariant( 123456 ) );
  // now with group separator for german locale
  stringInt = QVariant( "123.456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringInt.userType() ), QMetaType::Type::Int );
  QCOMPARE( stringInt, QVariant( "123456" ) );

  //string representation of a longlong
  stringLong = QVariant( "99999999999999999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringLong.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );
  // now with group separator for german locale
  stringLong = QVariant( "99.999.999.999.999.999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringLong.userType() ), QMetaType::Type::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );

  //string representation of a double
  stringDouble = QVariant( "123456,012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringDouble.userType() ), QMetaType::Type::Double );
  QCOMPARE( stringDouble, QVariant( 123456.012345 ) );
  // For doubles we also want to accept dot as a decimal point
  stringDouble = QVariant( "123456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringDouble.userType() ), QMetaType::Type::Double );
  QCOMPARE( stringDouble, QVariant( 123456.012345 ) );
  // now with group separator for german locale
  stringDouble = QVariant( "1.223.456,012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringDouble.userType() ), QMetaType::Type::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );
  // Be are good citizens and we also accept english locale
  stringDouble = QVariant( "1,223,456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringDouble.userType() ), QMetaType::Type::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );

  // Test that wrongly formatted decimal separator are also accepted
  QLocale::setDefault( QLocale::German );
  stringDouble = QVariant( "12.23.456,012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( static_cast<QMetaType::Type>( stringDouble.userType() ), QMetaType::Type::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );

  // Test 0 on int fields
  intField = QgsField( QStringLiteral( "int" ), QMetaType::Type::Int, QStringLiteral( "Integer" ), 10 );
  QVariant vZero = 0;
  QVERIFY( intField.convertCompatible( vZero ) );

  // Test string-based json field conversion
  {
    const QgsField jsonField( QStringLiteral( "json" ), QMetaType::Type::QString, QStringLiteral( "json" ) );
    QVariant jsonValue = QVariant::fromValue( QVariantList() << 1 << 5 << 8 );
    QVERIFY( jsonField.convertCompatible( jsonValue ) );
    QCOMPARE( static_cast<QMetaType::Type>( jsonValue.userType() ), QMetaType::Type::QString );
    QCOMPARE( jsonValue, QString( "[1,5,8]" ) );
    QVariantMap variantMap;
    variantMap.insert( QStringLiteral( "a" ), 1 );
    variantMap.insert( QStringLiteral( "c" ), 3 );
    jsonValue = QVariant::fromValue( variantMap );
    QVERIFY( jsonField.convertCompatible( jsonValue ) );
    QCOMPARE( static_cast<QMetaType::Type>( jsonValue.userType() ), QMetaType::Type::QString );
    QCOMPARE( jsonValue, QString( "{\"a\":1,\"c\":3}" ) );
  }

  // Test map-based json field (i.e. OGR geopackage JSON fields) conversion
  {
    const QgsField jsonField( QStringLiteral( "json" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ) );
    QVariant jsonValue = QVariant::fromValue( QVariantList() << 1 << 5 << 8 );
    QVERIFY( jsonField.convertCompatible( jsonValue ) );
    QCOMPARE( static_cast<QMetaType::Type>( jsonValue.userType() ), QMetaType::Type::QVariantList );
    QCOMPARE( jsonValue, QVariantList() << 1 << 5 << 8 );
    QVariantMap variantMap;
    variantMap.insert( QStringLiteral( "a" ), 1 );
    variantMap.insert( QStringLiteral( "c" ), 3 );
    jsonValue = QVariant::fromValue( variantMap );
    QVERIFY( jsonField.convertCompatible( jsonValue ) );
    QCOMPARE( static_cast<QMetaType::Type>( jsonValue.userType() ), QMetaType::Type::QVariantMap );
    QCOMPARE( jsonValue, variantMap );
  }

  // geometry field conversion
  const QgsField geometryField( QStringLiteral( "geometry" ), QMetaType::Type::User, QStringLiteral( "geometry" ) );
  QVariant geometryValue;
  QVERIFY( geometryField.convertCompatible( geometryValue ) );
  QVERIFY( geometryValue.isNull() );
  geometryValue = QVariant::fromValue( QgsGeometry::fromWkt( QStringLiteral( "Point( 1 2 )" ) ) );
  QVERIFY( geometryField.convertCompatible( geometryValue ) );
  QCOMPARE( geometryValue.userType(), qMetaTypeId<QgsGeometry>() );

  geometryValue = QVariant::fromValue( QgsReferencedGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point( 1 2 )" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ) );
  QVERIFY( geometryField.convertCompatible( geometryValue ) );
  QCOMPARE( geometryValue.userType(), qMetaTypeId<QgsReferencedGeometry>() );

  geometryValue = QStringLiteral( "LineString( 1 2, 3 4 )" );
  QVERIFY( geometryField.convertCompatible( geometryValue ) );
  QCOMPARE( geometryValue.userType(), qMetaTypeId<QgsGeometry>() );
}

void TestQgsField::dataStream()
{
  QgsField original;
  original.setName( QStringLiteral( "name" ) );
  original.setType( QMetaType::Type::Int );
  original.setLength( 5 );
  original.setPrecision( 2 );
  original.setTypeName( QStringLiteral( "typename1" ) );
  original.setComment( QStringLiteral( "comment1" ) );
  original.setAlias( QStringLiteral( "alias" ) );
  original.setDefaultValueDefinition( QgsDefaultValue( QStringLiteral( "default" ) ) );
  original.setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
  original.setDuplicatePolicy( Qgis::FieldDuplicatePolicy::DefaultValue );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginLayer );
  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  original.setConstraints( constraints );
  original.setMetadata( { { static_cast<int>( Qgis::FieldMetadataProperty::GeometryCrs ), QStringLiteral( "abc" ) }, { 2, 5 } } );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << original;

  QgsField result;
  ds.device()->seek( 0 );
  ds >> result;

  QCOMPARE( result, original );
  QCOMPARE( result.typeName(), original.typeName() ); //typename is NOT required for equality
  QCOMPARE( result.comment(), original.comment() );   //comment is NOT required for equality
}

void TestQgsField::displayName()
{
  QgsField field;
  field.setName( QStringLiteral( "name" ) );
  QCOMPARE( field.displayName(), QString( "name" ) );
  field.setAlias( QStringLiteral( "alias" ) );
  QCOMPARE( field.displayName(), QString( "alias" ) );
  field.setAlias( QString() );
  QCOMPARE( field.displayName(), QString( "name" ) );
}


void TestQgsField::displayNameWithAlias()
{
  QgsField field;
  field.setName( QStringLiteral( "name" ) );
  QCOMPARE( field.displayNameWithAlias(), QString( "name" ) );
  field.setAlias( QStringLiteral( "alias" ) );
  QCOMPARE( field.displayNameWithAlias(), QString( "name (alias)" ) );
  field.setAlias( QString() );
  QCOMPARE( field.displayNameWithAlias(), QString( "name" ) );
}


void TestQgsField::displayType()
{
  QgsField field;
  field.setTypeName( QStringLiteral( "numeric" ) );
  QCOMPARE( field.displayType(), QString( "numeric" ) );
  field.setLength( 20 );
  QCOMPARE( field.displayType(), QString( "numeric(20)" ) );
  field.setPrecision( 10 );
  field.setPrecision( 10 );
  QCOMPARE( field.displayType(), QString( "numeric(20, 10)" ) );
  QCOMPARE( field.displayType( true ), QString( "numeric(20, 10) NULL" ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique );
  field.setConstraints( constraints );
  QCOMPARE( field.displayType( true ), QString( "numeric(20, 10) NULL UNIQUE" ) );

  // test field without an explicit type name, we should use the field type
  QgsField field2;
  field2.setType( QMetaType::Type::QString );
  QCOMPARE( field2.displayType( false ), QString( "Text (string)" ) );
  QCOMPARE( field2.displayType( true ), QString( "Text (string) NULL" ) );
  field2.setType( QMetaType::Type::Int );
  QCOMPARE( field2.displayType( false ), QString( "Integer (32 bit)" ) );
}

void TestQgsField::friendlyTypeString()
{
  QgsField field;
  field.setType( QMetaType::Type::QString );
  QCOMPARE( field.friendlyTypeString(), QStringLiteral( "Text (string)" ) );
  field.setType( QMetaType::Type::Double );
  field.setLength( 20 );
  QCOMPARE( field.friendlyTypeString(), QStringLiteral( "Decimal (double)" ) );
  field.setType( QMetaType::Type::QVariantList );
  field.setSubType( QMetaType::Type::QString );
  QCOMPARE( field.friendlyTypeString(), QStringLiteral( "List" ) );
  field.setType( QMetaType::Type::User );
  field.setTypeName( QStringLiteral( "geometry" ) );
  QCOMPARE( field.friendlyTypeString(), QStringLiteral( "Geometry" ) );
}

void TestQgsField::editorWidgetSetup()
{
  QgsField field;
  QVariantMap config;
  config.insert( QStringLiteral( "a" ), "value_a" );
  const QgsEditorWidgetSetup setup( QStringLiteral( "test" ), config );
  field.setEditorWidgetSetup( setup );

  const QgsField otherField = field;

  QCOMPARE( field.editorWidgetSetup().type(), setup.type() );
  QCOMPARE( field.editorWidgetSetup().config(), setup.config() );
  // trigger copy-on-write with unrelated method call when private pointer is referenced more than once
  field.setName( QStringLiteral( "original" ) );
  // verify that editorWidgetSetup still remains
  QCOMPARE( field.editorWidgetSetup().type(), setup.type() );
  QCOMPARE( field.editorWidgetSetup().config(), setup.config() );
  QCOMPARE( otherField.editorWidgetSetup().type(), setup.type() );
  QCOMPARE( otherField.editorWidgetSetup().config(), setup.config() );
}

void TestQgsField::collection()
{
  QgsField field( QStringLiteral( "collection" ), QMetaType::Type::QVariantList, QStringLiteral( "_int32" ), 0, 0, QString(), QMetaType::Type::Int );
  QCOMPARE( field.subType(), QMetaType::Type::Int );
  field.setSubType( QMetaType::Type::Double );
  QCOMPARE( field.subType(), QMetaType::Type::Double );

  QVariant str( "hello" );
  QVERIFY( !field.convertCompatible( str ) );

  QVariant intList = QVariantList( { 1, 2, 3 } );
  QVERIFY( field.convertCompatible( intList ) );
  QCOMPARE( intList.toList(), QVariantList( { 1, 2, 3 } ) );

  QVariant doubleList = QVariantList( { 1.1, 2.2, 3.3 } );
  QVERIFY( field.convertCompatible( doubleList ) );
  QCOMPARE( doubleList.toList(), QVariantList( { 1.1, 2.2, 3.3 } ) );

  QgsField stringListField( QStringLiteral( "collection" ), QMetaType::Type::QStringList );
  str = QVariant( "hello" );
  QVERIFY( stringListField.convertCompatible( str ) );
  QCOMPARE( str, QStringList { QStringLiteral( "hello" ) } );

  QVariant strList = QVariant( QStringList( { "hello", "there" } ) );
  QVERIFY( stringListField.convertCompatible( strList ) );
  QCOMPARE( strList, QVariant( QStringList( { "hello", "there" } ) ) );

  QVariant strInVariantList = QVariant( QVariantList( { "hello", "there" } ) );
  QVERIFY( stringListField.convertCompatible( strInVariantList ) );
  QCOMPARE( strInVariantList, QVariant( QStringList( { "hello", "there" } ) ) );
}

QGSTEST_MAIN( TestQgsField )
#include "testqgsfield.moc"
