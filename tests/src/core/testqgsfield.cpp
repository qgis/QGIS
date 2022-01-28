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
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QLocale>

#include <memory>

#include "qgssettings.h"
#include "qgsfield.h"
#include "qgsapplication.h"
#include "qgstest.h"

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
    void isNumeric(); //test isNumeric
    void isDateTime(); //test isNumeric
    void equality(); //test equality operators
    void asVariant(); //test conversion to and from a QVariant
    void displayString();
    void convertCompatible();
    void dataStream();
    void displayName();
    void displayNameWithAlias();
    void displayType();
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
  std::unique_ptr<QgsField> field( new QgsField( QStringLiteral( "name" ), QVariant::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) ) );
  QCOMPARE( field->name(), QString( "name" ) );
  QCOMPARE( field->type(), QVariant::Double );
  QCOMPARE( field->typeName(), QString( "double" ) );
  QCOMPARE( field->length(), 5 );
  QCOMPARE( field->precision(), 2 );
  QCOMPARE( field->comment(), QString( "comment" ) );
  QCOMPARE( field->isReadOnly(), false );
}

void TestQgsField::copy()
{
  QgsField original( QStringLiteral( "original" ), QVariant::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  original.setConstraints( constraints );
  original.setReadOnly( true );
  QgsField copy( original );
  QVERIFY( copy == original );

  copy.setName( QStringLiteral( "copy" ) );
  QCOMPARE( original.name(), QString( "original" ) );
  QVERIFY( copy != original );
}

void TestQgsField::assignment()
{
  QgsField original( QStringLiteral( "original" ), QVariant::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  original.setConstraints( constraints );
  original.setReadOnly( true );
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
  field.setType( QVariant::Int );
  QCOMPARE( field.type(), QVariant::Int );
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
}

void TestQgsField::isNumeric()
{
  QgsField field;
  field.setType( QVariant::Int );
  QVERIFY( field.isNumeric() );
  field.setType( QVariant::UInt );
  QVERIFY( field.isNumeric() );
  field.setType( QVariant::Double );
  QVERIFY( field.isNumeric() );
  field.setType( QVariant::LongLong );
  QVERIFY( field.isNumeric() );
  field.setType( QVariant::ULongLong );
  QVERIFY( field.isNumeric() );
  field.setType( QVariant::String );
  QVERIFY( !field.isNumeric() );
  field.setType( QVariant::DateTime );
  QVERIFY( !field.isNumeric() );
  field.setType( QVariant::Bool );
  QVERIFY( !field.isNumeric() );
  field.setType( QVariant::Invalid );
  QVERIFY( !field.isNumeric() );
}

void TestQgsField::isDateTime()
{
  QgsField field;
  field.setType( QVariant::Int );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QVariant::UInt );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QVariant::Double );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QVariant::LongLong );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QVariant::ULongLong );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QVariant::String );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QVariant::DateTime );
  QVERIFY( field.isDateOrTime() );
  field.setType( QVariant::Time );
  QVERIFY( field.isDateOrTime() );
  field.setType( QVariant::Date );
  QVERIFY( field.isDateOrTime() );
  field.setType( QVariant::Bool );
  QVERIFY( !field.isDateOrTime() );
  field.setType( QVariant::Invalid );
  QVERIFY( !field.isDateOrTime() );
}

void TestQgsField::equality()
{
  QgsField field1;
  field1.setName( QStringLiteral( "name" ) );
  field1.setType( QVariant::Int );
  field1.setLength( 5 );
  field1.setPrecision( 2 );
  field1.setTypeName( QStringLiteral( "typename1" ) ); //typename is NOT required for equality
  field1.setComment( QStringLiteral( "comment1" ) ); //comment is NOT required for equality
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  field1.setConstraints( constraints );
  QgsField field2;
  field2.setName( QStringLiteral( "name" ) );
  field2.setType( QVariant::Int );
  field2.setLength( 5 );
  field2.setPrecision( 2 );
  field2.setTypeName( QStringLiteral( "typename2" ) ); //typename is NOT required for equality
  field2.setComment( QStringLiteral( "comment2" ) ); //comment is NOT required for equality
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

  QgsFieldConstraints constraints1;
  QgsFieldConstraints constraints2;
  constraints1.setDomainName( QStringLiteral( "d" ) );
  QVERIFY( !( constraints1 == constraints2 ) );
  constraints2.setDomainName( QStringLiteral( "d" ) );
  QVERIFY( constraints1 == constraints2 );
}

void TestQgsField::asVariant()
{
  QgsField original( QStringLiteral( "original" ), QVariant::Double, QStringLiteral( "double" ), 5, 2, QStringLiteral( "comment" ) );
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
  const QgsField stringField( QStringLiteral( "string" ), QVariant::String, QStringLiteral( "string" ) );

  //test string value
  const QString test( QStringLiteral( "test string" ) );
  QCOMPARE( stringField.displayString( test ), test );

  //test NULL
  QgsApplication::setNullRepresentation( QStringLiteral( "TEST NULL" ) );
  const QVariant nullString = QVariant( QVariant::String );
  QCOMPARE( stringField.displayString( nullString ), QString( "TEST NULL" ) );

  //test int value in string type
  const QgsField intField( QStringLiteral( "int" ), QVariant::String, QStringLiteral( "int" ) );
  QCOMPARE( intField.displayString( 5 ), QString( "5" ) );
  QCOMPARE( intField.displayString( 599999898999LL ), QString( "599999898999" ) );

  //test int value in int type
  const QgsField intField2( QStringLiteral( "int" ), QVariant::Int, QStringLiteral( "int" ) );
  QCOMPARE( intField2.displayString( 5 ), QString( "5" ) );
  QCOMPARE( intField2.displayString( 599999898999LL ), QString( "599,999,898,999" ) );

  //test long type
  const QgsField longField( QStringLiteral( "long" ), QVariant::LongLong, QStringLiteral( "longlong" ) );
  QCOMPARE( longField.displayString( 5 ), QString( "5" ) );
  QCOMPARE( longField.displayString( 599999898999LL ), QString( "599,999,898,999" ) );

  //test NULL int
  const QVariant nullInt = QVariant( QVariant::Int );
  QCOMPARE( intField.displayString( nullInt ), QString( "TEST NULL" ) );

  //test double value
  const QgsField doubleField( QStringLiteral( "double" ), QVariant::Double, QStringLiteral( "double" ), 10, 3 );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5.005" ) );
  QCOMPARE( doubleField.displayString( 4.5e-09 ), QString( "4.5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0.0001" ) );
  const QgsField doubleFieldNoPrec( QStringLiteral( "double" ), QVariant::Double, QStringLiteral( "double" ), 10 );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5.005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5.005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ), QString( "4.5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-04 ), QString( "0.0001" ) );
  QCOMPARE( QLocale().numberOptions() & QLocale::NumberOption::OmitGroupSeparator, QLocale::NumberOption::DefaultNumberOptions );
  QCOMPARE( doubleFieldNoPrec.displayString( 599999898999.0 ), QString( "599,999,898,999" ) );

  // no conversion when this is default value expression
  QCOMPARE( doubleFieldNoPrec.displayString( ( "(1+2)" ) ), QString( "(1+2)" ) );

  //test NULL double
  const QVariant nullDouble = QVariant( QVariant::Double );
  QCOMPARE( doubleField.displayString( nullDouble ), QString( "TEST NULL" ) );

  //test double value with German locale
  QLocale::setDefault( QLocale::German );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5,005" ) );
  QCOMPARE( doubleField.displayString( 4.5e-09 ), QString( "4,5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0,0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5,005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5,005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ), QString( "4,5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-04 ), QString( "0,0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 599999898999.0 ), QString( "599.999.898.999" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5999.123456 ), QString( "5.999,123456" ) );

  //test value with custom German locale (OmitGroupSeparator)
  QLocale customGerman( QLocale::German );
  customGerman.setNumberOptions( QLocale::NumberOption::OmitGroupSeparator );
  QLocale::setDefault( customGerman );
  QCOMPARE( doubleField.displayString( 5.005005 ), QString( "5,005" ) );
  QCOMPARE( doubleField.displayString( 4.5e-09 ), QString( "4,5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0,0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5,005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5,005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ), QString( "4,5e-09" ) );
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
  QCOMPARE( doubleField.displayString( 4.5e-09 ), QString( "4.5e-09" ) );
  QCOMPARE( doubleField.displayString( 1e-04 ), QString( "0.0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005 ), QString( "5.005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5.005005005 ), QString( "5.005005005" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 4.5e-09 ), QString( "4.5e-09" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 1e-04 ), QString( "0.0001" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 599999898999.0 ), QString( "599999898999" ) );
  QCOMPARE( doubleFieldNoPrec.displayString( 5999.123456 ), QString( "5999.123456" ) );

  //test int value in int type with custom english locale (OmitGroupSeparator)
  QCOMPARE( intField2.displayString( 5 ), QString( "5" ) );
  QCOMPARE( intField2.displayString( 599999898999LL ), QString( "599999898999" ) );

  //test long type with custom english locale (OmitGroupSeparator)
  QCOMPARE( longField.displayString( 5 ), QString( "5" ) );
  QCOMPARE( longField.displayString( 599999898999LL ), QString( "599999898999" ) );

  // binary field
  const QgsField binaryField( QStringLiteral( "binary" ), QVariant::ByteArray, QStringLiteral( "Binary" ) );
  const QString testBAString( QStringLiteral( "test string" ) );
  const QByteArray testBA( testBAString.toLocal8Bit() );
  QCOMPARE( binaryField.displayString( testBA ), QStringLiteral( "BLOB" ) );

  // array field
  const QgsField stringArrayField( QStringLiteral( "stringArray" ), QVariant::StringList, QStringLiteral( "StringArray" ) );
  QCOMPARE( stringArrayField.displayString( QStringList() << "A" << "B" << "C" ), QStringLiteral( "A, B, C" ) );
  const QgsField intArrayField( QStringLiteral( "intArray" ), QVariant::List, QStringLiteral( "IntArray" ) );
  QCOMPARE( intArrayField.displayString( QVariantList() << 1 << 2 << 3 ), QStringLiteral( "1, 2, 3" ) );
}

void TestQgsField::convertCompatible()
{
  //test string field
  const QgsField stringField( QStringLiteral( "string" ), QVariant::String, QStringLiteral( "string" ) );

  QVariant stringVar( "test string" );
  QVERIFY( stringField.convertCompatible( stringVar ) );
  QCOMPARE( stringVar.toString(), QString( "test string" ) );
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
  QVariant doubleVar( 1.25 );
  QVERIFY( stringField.convertCompatible( doubleVar ) );
  QCOMPARE( doubleVar.type(), QVariant::String );
  QCOMPARE( doubleVar, QVariant( "1.25" ) );
  QVariant nullDouble = QVariant( QVariant::Double );
  QVERIFY( stringField.convertCompatible( nullDouble ) );
  QCOMPARE( nullDouble.type(), QVariant::String );
  QVERIFY( nullDouble.isNull() );

  //test double
  const QgsField doubleField( QStringLiteral( "double" ), QVariant::Double, QStringLiteral( "double" ) );

  stringVar = QVariant( "test string" );
  QString error;
  QVERIFY( !doubleField.convertCompatible( stringVar, &error ) );
  QCOMPARE( stringVar.type(), QVariant::Double );
  QCOMPARE( error, QStringLiteral( "Could not convert value \"test string\" to target type" ) );
  stringVar = QVariant( "test string" );
  QVERIFY( !doubleField.convertCompatible( stringVar ) );
  QVERIFY( stringVar.isNull() );
  nullString = QVariant( QVariant::String );
  QVERIFY( doubleField.convertCompatible( nullString, &error ) );
  QVERIFY( error.isEmpty() );
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
  doubleVar = QVariant( 1.25 );
  QVERIFY( doubleField.convertCompatible( doubleVar ) );
  QCOMPARE( doubleVar.type(), QVariant::Double );
  QCOMPARE( doubleVar, QVariant( 1.25 ) );
  nullDouble = QVariant( QVariant::Double );
  QVERIFY( doubleField.convertCompatible( nullDouble ) );
  QCOMPARE( nullDouble.type(), QVariant::Double );
  QVERIFY( nullDouble.isNull() );

  //test special rules

  //conversion of double to int
  QgsField intField( QStringLiteral( "int" ), QVariant::Int, QStringLiteral( "int" ) );
  //small double, should be rounded
  QVariant smallDouble( 45.7 );
  QVERIFY( intField.convertCompatible( smallDouble ) );
  QCOMPARE( smallDouble.type(), QVariant::Int );
  QCOMPARE( smallDouble, QVariant( 46 ) );
  QVariant negativeSmallDouble( -9345.754534525235235 );
  QVERIFY( intField.convertCompatible( negativeSmallDouble ) );
  QCOMPARE( negativeSmallDouble.type(), QVariant::Int );
  QCOMPARE( negativeSmallDouble, QVariant( -9346 ) );
  //large double, cannot be converted
  QVariant largeDouble( 9999999999.99 );
  QVERIFY( !intField.convertCompatible( largeDouble, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"10000000000\" is too large for integer field" ) );
  largeDouble = QVariant( 9999999999.99 );
  QVERIFY( !intField.convertCompatible( largeDouble ) );
  QCOMPARE( largeDouble.type(), QVariant::Int );
  QVERIFY( largeDouble.isNull() );

  //conversion of string double value to int
  QVariant notNumberString( "notanumber" );
  QVERIFY( !intField.convertCompatible( notNumberString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"notanumber\" is not a number" ) );
  notNumberString = QVariant( "notanumber" );
  QVERIFY( !intField.convertCompatible( notNumberString ) );
  QCOMPARE( notNumberString.type(), QVariant::Int );
  QVERIFY( notNumberString.isNull() );
  //small double, should be rounded
  QVariant smallDoubleString( "45.7" );
  QVERIFY( intField.convertCompatible( smallDoubleString ) );
  QCOMPARE( smallDoubleString.type(), QVariant::Int );
  QCOMPARE( smallDoubleString, QVariant( 46 ) );
  QVariant negativeSmallDoubleString( "-9345.754534525235235" );
  QVERIFY( intField.convertCompatible( negativeSmallDoubleString ) );
  QCOMPARE( negativeSmallDoubleString.type(), QVariant::Int );
  QCOMPARE( negativeSmallDoubleString, QVariant( -9346 ) );
  //large double, cannot be converted
  QVariant largeDoubleString( "9999999999.99" );
  QVERIFY( !intField.convertCompatible( largeDoubleString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"1e+10\" is too large for integer field" ) );
  largeDoubleString = QVariant( "9999999999.99" );
  QVERIFY( !intField.convertCompatible( largeDoubleString ) );
  QCOMPARE( largeDoubleString.type(), QVariant::Int );
  QVERIFY( largeDoubleString.isNull() );

  //conversion of longlong to int
  QVariant longlong( 99999999999999999LL );
  QVERIFY( !intField.convertCompatible( longlong, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"99999999999999999\" is too large for integer field" ) );
  QCOMPARE( longlong.type(), QVariant::Int );
  QVERIFY( longlong.isNull() );
  QVariant smallLonglong( 99LL );
  QVERIFY( intField.convertCompatible( smallLonglong ) );
  QCOMPARE( smallLonglong.type(), QVariant::Int );
  QCOMPARE( smallLonglong, QVariant( 99 ) );
  // negative longlong to int
  QVariant negativeLonglong( -99999999999999999LL );
  QVERIFY( !intField.convertCompatible( negativeLonglong, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"-99999999999999999\" is too large for integer field" ) );
  QCOMPARE( negativeLonglong.type(), QVariant::Int );
  QVERIFY( negativeLonglong.isNull() );
  // small negative longlong to int
  QVariant smallNegativeLonglong( -99LL );
  QVERIFY( intField.convertCompatible( smallNegativeLonglong ) );
  QCOMPARE( smallNegativeLonglong.type(), QVariant::Int );
  QCOMPARE( smallNegativeLonglong, QVariant( -99 ) );

  //string representation of an int
  QVariant stringInt( "123456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( stringInt.type(), QVariant::Int );
  QCOMPARE( stringInt, QVariant( 123456 ) );
  // now with group separator for english locale
  stringInt = QVariant( "123,456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( stringInt.type(), QVariant::Int );
  QCOMPARE( stringInt, QVariant( "123456" ) );

  //conversion of longlong to longlong field
  const QgsField longlongField( QStringLiteral( "long" ), QVariant::LongLong, QStringLiteral( "longlong" ) );
  longlong = QVariant( 99999999999999999LL );
  QVERIFY( longlongField.convertCompatible( longlong ) );
  QCOMPARE( longlong.type(), QVariant::LongLong );
  QCOMPARE( longlong, QVariant( 99999999999999999LL ) );

  //string representation of a longlong
  QVariant stringLong( "99999999999999999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( stringLong.type(), QVariant::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );
  // now with group separator for english locale
  stringLong = QVariant( "99,999,999,999,999,999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( stringLong.type(), QVariant::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );

  //conversion of string double value to longlong
  notNumberString = QVariant( "notanumber" );
  QVERIFY( !longlongField.convertCompatible( notNumberString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"notanumber\" is not a number" ) );
  QCOMPARE( notNumberString.type(), QVariant::LongLong );
  QVERIFY( notNumberString.isNull() );
  //small double, should be rounded
  smallDoubleString = QVariant( "45.7" );
  QVERIFY( longlongField.convertCompatible( smallDoubleString ) );
  QCOMPARE( smallDoubleString.type(), QVariant::LongLong );
  QCOMPARE( smallDoubleString, QVariant( 46 ) );
  negativeSmallDoubleString = QVariant( "-9345.754534525235235" );
  QVERIFY( longlongField.convertCompatible( negativeSmallDoubleString ) );
  QCOMPARE( negativeSmallDoubleString.type(), QVariant::LongLong );
  QCOMPARE( negativeSmallDoubleString, QVariant( -9346 ) );
  //large double, can be converted
  largeDoubleString = QVariant( "9999999999.99" );
  QVERIFY( longlongField.convertCompatible( largeDoubleString ) );
  QCOMPARE( largeDoubleString.type(), QVariant::LongLong );
  QCOMPARE( largeDoubleString, QVariant( 10000000000LL ) );
  //extra large double, cannot be converted
  largeDoubleString = QVariant( "999999999999999999999.99" );
  QVERIFY( !longlongField.convertCompatible( largeDoubleString, &error ) );
  QCOMPARE( error, QStringLiteral( "Value \"1e+21\" is too large for long long field" ) );
  QCOMPARE( largeDoubleString.type(), QVariant::LongLong );
  QVERIFY( largeDoubleString.isNull() );

  //string representation of a double
  QVariant stringDouble( "123456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( stringDouble.type(), QVariant::Double );
  QCOMPARE( stringDouble, QVariant( 123456.012345 ) );
  // now with group separator for english locale
  stringDouble = QVariant( "1,223,456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( stringDouble.type(), QVariant::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );
  // This should not convert
  stringDouble = QVariant( "1.223.456,012345" );
  QVERIFY( ! doubleField.convertCompatible( stringDouble, &error ) );
  QCOMPARE( error, QStringLiteral( "Could not convert value \"1.223.456,012345\" to target type" ) );

  //double with precision
  const QgsField doubleWithPrecField( QStringLiteral( "double" ), QVariant::Double, QStringLiteral( "double" ), 10, 3 );
  doubleVar = QVariant( 10.12345678 );
  //note - this returns true!
  QVERIFY( doubleWithPrecField.convertCompatible( doubleVar ) );
  QCOMPARE( doubleVar.type(), QVariant::Double );
  QCOMPARE( doubleVar.toDouble(), 10.123 );

  //truncating string length
  const QgsField stringWithLen( QStringLiteral( "string" ), QVariant::String, QStringLiteral( "string" ), 3 );
  stringVar = QVariant( "longstring" );
  QVERIFY( !stringWithLen.convertCompatible( stringVar, &error ) );
  QCOMPARE( error, QStringLiteral( "String of length 10 exceeds maximum field length (3)" ) );
  QCOMPARE( stringVar.type(), QVariant::String );
  QCOMPARE( stringVar.toString(), QString( "lon" ) );


  /////////////////////////////////////////////////////////
  // German locale tests

  //double with ',' as decimal separator for German locale
  QLocale::setDefault( QLocale::German );
  QVariant doubleCommaVar( "1,2345" );
  QVERIFY( doubleField.convertCompatible( doubleCommaVar ) );
  QCOMPARE( doubleCommaVar.type(), QVariant::Double );
  QCOMPARE( doubleCommaVar.toString(), QString( "1.2345" ) );

  //string representation of an int
  stringInt = QVariant( "123456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( stringInt.type(), QVariant::Int );
  QCOMPARE( stringInt, QVariant( 123456 ) );
  // now with group separator for german locale
  stringInt = QVariant( "123.456" );
  QVERIFY( intField.convertCompatible( stringInt ) );
  QCOMPARE( stringInt.type(), QVariant::Int );
  QCOMPARE( stringInt, QVariant( "123456" ) );

  //string representation of a longlong
  stringLong = QVariant( "99999999999999999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( stringLong.type(), QVariant::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );
  // now with group separator for german locale
  stringLong = QVariant( "99.999.999.999.999.999" );
  QVERIFY( longlongField.convertCompatible( stringLong ) );
  QCOMPARE( stringLong.type(), QVariant::LongLong );
  QCOMPARE( stringLong, QVariant( 99999999999999999LL ) );

  //string representation of a double
  stringDouble = QVariant( "123456,012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( stringDouble.type(), QVariant::Double );
  QCOMPARE( stringDouble, QVariant( 123456.012345 ) );
  // For doubles we also want to accept dot as a decimal point
  stringDouble = QVariant( "123456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( stringDouble.type(), QVariant::Double );
  QCOMPARE( stringDouble, QVariant( 123456.012345 ) );
  // now with group separator for german locale
  stringDouble = QVariant( "1.223.456,012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( stringDouble.type(), QVariant::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );
  // Be are good citizens and we also accept english locale
  stringDouble = QVariant( "1,223,456.012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( stringDouble.type(), QVariant::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );

  // Test that wrongly formatted decimal separator are also accepted
  QLocale::setDefault( QLocale::German );
  stringDouble = QVariant( "12.23.456,012345" );
  QVERIFY( doubleField.convertCompatible( stringDouble ) );
  QCOMPARE( stringDouble.type(), QVariant::Double );
  QCOMPARE( stringDouble, QVariant( 1223456.012345 ) );

  // Test 0 on int fields
  intField = QgsField( QStringLiteral( "int" ), QVariant::Int, QStringLiteral( "Integer" ), 10 );
  QVariant vZero = 0;
  QVERIFY( intField.convertCompatible( vZero ) );

  // Test json field conversion
  const QgsField jsonField( QStringLiteral( "json" ), QVariant::String, QStringLiteral( "json" ) );
  QVariant jsonValue = QVariant::fromValue( QVariantList() << 1 << 5 << 8 );
  QVERIFY( jsonField.convertCompatible( jsonValue ) );
  QCOMPARE( jsonValue.type(), QVariant::String );
  QCOMPARE( jsonValue, QString( "[1,5,8]" ) );
  QVariantMap variantMap;
  variantMap.insert( QStringLiteral( "a" ), 1 );
  variantMap.insert( QStringLiteral( "c" ), 3 );
  jsonValue = QVariant::fromValue( variantMap );
  QVERIFY( jsonField.convertCompatible( jsonValue ) );
  QCOMPARE( jsonValue.type(), QVariant::String );
  QCOMPARE( jsonValue, QString( "{\"a\":1,\"c\":3}" ) );
}

void TestQgsField::dataStream()
{
  QgsField original;
  original.setName( QStringLiteral( "name" ) );
  original.setType( QVariant::Int );
  original.setLength( 5 );
  original.setPrecision( 2 );
  original.setTypeName( QStringLiteral( "typename1" ) );
  original.setComment( QStringLiteral( "comment1" ) );
  original.setAlias( QStringLiteral( "alias" ) );
  original.setDefaultValueDefinition( QgsDefaultValue( QStringLiteral( "default" ) ) );
  QgsFieldConstraints constraints;
  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginLayer );
  constraints.setConstraintExpression( QStringLiteral( "constraint expression" ), QStringLiteral( "description" ) );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  original.setConstraints( constraints );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << original;

  QgsField result;
  ds.device()->seek( 0 );
  ds >> result;

  QCOMPARE( result, original );
  QCOMPARE( result.typeName(), original.typeName() ); //typename is NOT required for equality
  QCOMPARE( result.comment(), original.comment() ); //comment is NOT required for equality
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
}


void TestQgsField::editorWidgetSetup()
{
  QgsField field;
  QVariantMap config;
  config.insert( QStringLiteral( "a" ), "value_a" );
  const QgsEditorWidgetSetup setup( QStringLiteral( "test" ), config );
  field.setEditorWidgetSetup( setup );

  QCOMPARE( field.editorWidgetSetup().type(), setup.type() );
  QCOMPARE( field.editorWidgetSetup().config(), setup.config() );
}

void TestQgsField::collection()
{
  QgsField field( QStringLiteral( "collection" ), QVariant::List, QStringLiteral( "_int32" ), 0, 0, QString(), QVariant::Int );
  QCOMPARE( field.subType(), QVariant::Int );
  field.setSubType( QVariant::Double );
  QCOMPARE( field.subType(), QVariant::Double );

  QVariant str( "hello" );
  QVERIFY( !field.convertCompatible( str ) );
}

QGSTEST_MAIN( TestQgsField )
#include "testqgsfield.moc"
