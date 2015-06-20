/***************************************************************************
     testqgsfontutils.cpp
     --------------------
    Date                 : June 2015
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

#include "qgsapplication.h"
#include "qgsfontutils.h"

class TestQgsFontUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void xmlMethods(); //test saving and reading from xml
    void fromChildNode(); //test reading from child node

  private:

};

void TestQgsFontUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsFontUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFontUtils::init()
{

}

void TestQgsFontUtils::cleanup()
{

}

void TestQgsFontUtils::xmlMethods()
{
  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QFont f1 = QgsFontUtils::getStandardTestFont();
  f1.setPointSize( 48 );
  QDomElement fontElem = QgsFontUtils::toXmlElement( f1, doc, "test" );

  //test reading
  QFont f2;
  QVERIFY( f2.family() != f1.family() );
  QVERIFY( QgsFontUtils::setFromXmlElement( f2, fontElem ) );
  QCOMPARE( f2.family(), f1.family() );
  QCOMPARE( f2.pointSize(), f1.pointSize() );
  QCOMPARE( f2.italic(), f1.italic() );
  QCOMPARE( f2.weight(), f1.weight() );
  QCOMPARE( f2.styleName(), f1.styleName() );

  //test writing/reading with styles
  f1 = QgsFontUtils::getStandardTestFont( "Bold" );
  fontElem = QgsFontUtils::toXmlElement( f1, doc, "test" );
  QVERIFY( f2.styleName() != f1.styleName() );
  QVERIFY( QgsFontUtils::setFromXmlElement( f2, fontElem ) );
  QCOMPARE( f2.family(), f1.family() );
  QCOMPARE( f2.pointSize(), f1.pointSize() );
  QCOMPARE( f2.italic(), f1.italic() );
  QCOMPARE( f2.weight(), f1.weight() );
  QCOMPARE( f2.styleName(), QString( "Bold" ) );

  //test numeric weight
  f1.setWeight( 5 );
  fontElem = QgsFontUtils::toXmlElement( f1, doc, "test" );
  QVERIFY( f2.weight() != f1.weight() );
  QVERIFY( QgsFontUtils::setFromXmlElement( f2, fontElem ) );
  QCOMPARE( f2.weight(), f1.weight() );

  //test reading from null element
  QDomElement badElem;
  QVERIFY( !QgsFontUtils::setFromXmlElement( f2, badElem ) );
}

void TestQgsFontUtils::fromChildNode()
{
  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  QFont f1 = QgsFontUtils::getStandardTestFont();
  f1.setPointSize( 48 );
  QDomElement fontElem = QgsFontUtils::toXmlElement( f1, doc, "testNode" );
  QDomElement parentElem = doc.createElement( "parent" );

  //first try with no child element
  QFont f2;
  QVERIFY( f2.family() != f1.family() );
  QVERIFY( !QgsFontUtils::setFromXmlChildNode( f2, parentElem, "testNode" ) );

  //now append child
  parentElem.appendChild( fontElem );

  //test with invalid child node name
  QVERIFY( !QgsFontUtils::setFromXmlChildNode( f2, parentElem, "badNodeName" ) );

  //test good child node
  QVERIFY( QgsFontUtils::setFromXmlChildNode( f2, parentElem, "testNode" ) );
  QCOMPARE( f2.family(), f1.family() );
  QCOMPARE( f2.pointSize(), f1.pointSize() );
  QCOMPARE( f2.italic(), f1.italic() );
  QCOMPARE( f2.weight(), f1.weight() );
  QCOMPARE( f2.styleName(), f1.styleName() );
}

QTEST_MAIN( TestQgsFontUtils )
#include "testqgsfontutils.moc"
