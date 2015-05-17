/***************************************************************************
     testqgsdatadefined.cpp
     ----------------------
    Date                 : November 2014
    Copyright            : (C) 2014 Nyall Dawson
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

#include <qgsdatadefined.h>

/** \ingroup UnitTests
 * Unit tests for QgsDataDefined
 */
class TestQgsDataDefined: public QObject
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
    void defaultValues(); //test hasDefaultValues method
    void equality(); //test equality operators
    void xmlMethods(); //test saving and reading from xml
    void mapMethods(); //test saving and reading from a string map
    void referencedColumns(); //test referenced columns method

  private:
};

void TestQgsDataDefined::initTestCase()
{

}

void TestQgsDataDefined::cleanupTestCase()
{

}

void TestQgsDataDefined::init()
{

}

void TestQgsDataDefined::cleanup()
{

}

void TestQgsDataDefined::create()
{
  QSharedPointer<QgsDataDefined> dd( new QgsDataDefined( true, true, QString( "exp" ), QString( "field" ) ) );
  QVERIFY( dd->isActive() );
  QVERIFY( dd->useExpression() );
  QCOMPARE( dd->expressionString(), QString( "exp" ) );
  QCOMPARE( dd->field(), QString( "field" ) );

  //test with string constructor
  QScopedPointer<QgsDataDefined> stringConstructorField( new QgsDataDefined( QString( "\"col1\"" ) ) );
  QVERIFY( stringConstructorField->isActive() );
  QVERIFY( ! stringConstructorField->useExpression() );
  QVERIFY( stringConstructorField->expressionString().isEmpty() );
  QCOMPARE( stringConstructorField->field(), QString( "col1" ) );

  QScopedPointer<QgsDataDefined> stringConstructorExp( new QgsDataDefined( QString( "1 + 2" ) ) );
  QVERIFY( stringConstructorExp->isActive() );
  QVERIFY( stringConstructorExp->useExpression() );
  QCOMPARE( stringConstructorExp->expressionString(), QString( "1 + 2" ) );
  QVERIFY( stringConstructorExp->field().isEmpty() );

  QScopedPointer<QgsDataDefined> stringConstructorEmpty( new QgsDataDefined( QString( "" ) ) );
  QVERIFY( ! stringConstructorEmpty->isActive() );
}

void TestQgsDataDefined::copy()
{
  QgsDataDefined original( true, true, QString( "sqrt(2)" ), QString( "field" ) );
  original.prepareExpression( NULL );
  QgsDataDefined copy( original );
  QVERIFY( copy == original );

  copy.setActive( false );
  QVERIFY( original.isActive() );
  QVERIFY( copy != original );
}

void TestQgsDataDefined::assignment()
{
  QgsDataDefined original( true, true, QString( "sqrt(2)" ), QString( "field" ) );
  QgsDataDefined copy;
  copy = original;
  QVERIFY( copy == original );

  copy.setActive( false );
  QVERIFY( original.isActive() );
  QVERIFY( copy != original );
}

void TestQgsDataDefined::gettersSetters()
{
  QgsDataDefined dd;
  dd.setActive( false );
  QVERIFY( !dd.isActive() );
  dd.setActive( true );
  QVERIFY( dd.isActive() );

  dd.setUseExpression( false );
  QVERIFY( !dd.useExpression() );
  dd.setUseExpression( true );
  QVERIFY( dd.useExpression() );

  dd.setExpressionString( QString( "expression" ) );
  QCOMPARE( dd.expressionString(), QString( "expression" ) );

  dd.setField( QString( "field" ) );
  QCOMPARE( dd.field(), QString( "field" ) );
}

void TestQgsDataDefined::defaultValues()
{
  QgsDataDefined* dd = new QgsDataDefined();
  QVERIFY( dd->hasDefaultValues() );
  dd->setActive( true );
  QVERIFY( !dd->hasDefaultValues() );
  delete dd;
  dd = new QgsDataDefined();
  dd->setUseExpression( true );
  QVERIFY( !dd->hasDefaultValues() );
  delete dd;
  dd = new QgsDataDefined();
  dd->setExpressionString( QString( "expression" ) );
  QVERIFY( !dd->hasDefaultValues() );
  delete dd;
  dd = new QgsDataDefined();
  dd->setField( QString( "field" ) );
  QVERIFY( !dd->hasDefaultValues() );
  delete dd;
}

void TestQgsDataDefined::equality()
{
  QgsDataDefined dd1;
  dd1.setActive( true );
  dd1.setField( QString( "field" ) );
  dd1.setExpressionString( QString( "expression" ) );
  dd1.setUseExpression( true );
  QgsDataDefined dd2;
  dd2.setActive( true );
  dd2.setField( QString( "field" ) );
  dd2.setExpressionString( QString( "expression" ) );
  dd2.setUseExpression( true );
  QVERIFY( dd1 == dd2 );
  QVERIFY( !( dd1 != dd2 ) );

  //test that all applicable components contribute to equality
  dd2.setActive( false );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd2.setActive( true );
  dd2.setField( QString( "a" ) );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd2.setField( QString( "field" ) );
  dd2.setExpressionString( QString( "b" ) );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
  dd2.setExpressionString( QString( "expression" ) );
  dd2.setUseExpression( false );
  QVERIFY( !( dd1 == dd2 ) );
  QVERIFY( dd1 != dd2 );
}

void TestQgsDataDefined::xmlMethods()
{
  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QgsDataDefined dd1;
  dd1.setActive( true );
  dd1.setField( QString( "field" ) );
  dd1.setExpressionString( QString( "expression" ) );
  dd1.setUseExpression( true );
  QDomElement ddElem = dd1.toXmlElement( doc, "test" );

  //test reading
  QgsDataDefined dd2;
  QVERIFY( dd2 != dd1 );
  QVERIFY( dd2.setFromXmlElement( ddElem ) );
  QVERIFY( dd2 == dd1 );

  //test reading from null element
  QDomElement badElem;
  QVERIFY( !dd2.setFromXmlElement( badElem ) );
}

void TestQgsDataDefined::mapMethods()
{
  //test reading empty map
  QgsStringMap empty;
  QgsDataDefined* fromEmpty = QgsDataDefined::fromMap( empty );
  QVERIFY( !fromEmpty );

  //no base name
  QgsDataDefined dd1;
  dd1.setActive( true );
  dd1.setField( QString( "field" ) );
  dd1.setExpressionString( QString( "expression" ) );
  dd1.setUseExpression( true );
  QgsStringMap map1 = dd1.toMap();

  QgsDataDefined* dd2 = QgsDataDefined::fromMap( map1 );
  QCOMPARE( *dd2, dd1 );
  delete dd2;

  //base name
  QgsDataDefined dd3;
  dd3.setActive( false );
  dd3.setField( QString( "field2" ) );
  dd3.setExpressionString( QString( "expression2" ) );
  dd3.setUseExpression( false );
  QgsStringMap map2 = dd3.toMap( QString( "basename" ) );

  QgsDataDefined* dd4 = QgsDataDefined::fromMap( map2, QString( "basename" ) );
  QCOMPARE( *dd4, dd3 );
  delete dd4;

  // read with invalid basename
  dd4 = QgsDataDefined::fromMap( map2, QString( "xx" ) );
  QVERIFY( !dd4 );

  //test read map with only an expression
  QgsStringMap expMapOnly;
  expMapOnly.insert( QString( "expression" ), QString( "test_exp" ) );

  dd4 = QgsDataDefined::fromMap( expMapOnly );
  QVERIFY( dd4 );
  QCOMPARE( dd4->expressionString(), QString( "test_exp" ) );
  QVERIFY( dd4->isActive() );
  QVERIFY( dd4->useExpression() );

  delete dd4;
}

void TestQgsDataDefined::referencedColumns()
{
  QgsDataDefined dd;
  dd.setActive( true );
  dd.setUseExpression( true );

  QStringList cols = dd.referencedColumns();
  QVERIFY( cols.isEmpty() );

  //set as expression
  dd.setExpressionString( "1+col1+col2" );
  cols = dd.referencedColumns();
  QCOMPARE( cols.length(), 2 );
  QVERIFY( cols.contains( QString( "col1" ) ) );
  QVERIFY( cols.contains( QString( "col2" ) ) );

  //alter expression and check that referenced columns is updated
  dd.setExpressionString( "1+col1+col2+col3" );
  cols = dd.referencedColumns();
  QCOMPARE( cols.length(), 3 );
  QVERIFY( cols.contains( QString( "col1" ) ) );
  QVERIFY( cols.contains( QString( "col2" ) ) );
  QVERIFY( cols.contains( QString( "col3" ) ) );

  //switch to field
  dd.setUseExpression( false );
  cols = dd.referencedColumns();
  QVERIFY( cols.isEmpty() );

  dd.setField( "field" );
  cols = dd.referencedColumns();
  QCOMPARE( cols.length(), 1 );
  QVERIFY( cols.contains( QString( "field" ) ) );

  //switch back to expression
  dd.setUseExpression( true );
  cols = dd.referencedColumns();
  QCOMPARE( cols.length(), 3 );
  QVERIFY( cols.contains( QString( "col1" ) ) );
  QVERIFY( cols.contains( QString( "col2" ) ) );
  QVERIFY( cols.contains( QString( "col3" ) ) );
}

QTEST_MAIN( TestQgsDataDefined )
#include "testqgsdatadefined.moc"
