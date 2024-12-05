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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include "qgsapplication.h"
#include "qgsfontutils.h"

class TestQgsFontUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void xmlMethods();      //test saving and reading from xml
    void fromChildNode();   //test reading from child node
    void toCss();           //test converting font to CSS

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
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" )
  );
  QDomDocument doc( documentType );

  QFont f1 = QgsFontUtils::getStandardTestFont();
  f1.setPointSize( 48 );
  QDomElement fontElem = QgsFontUtils::toXmlElement( f1, doc, QStringLiteral( "test" ) );

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
  f1 = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  fontElem = QgsFontUtils::toXmlElement( f1, doc, QStringLiteral( "test" ) );
#ifndef Q_OS_WIN
  QVERIFY( f2.styleName() != f1.styleName() );
#else
  QVERIFY( f2.bold() != f1.bold() );
#endif
  QVERIFY( QgsFontUtils::setFromXmlElement( f2, fontElem ) );
  QCOMPARE( f2.family(), f1.family() );
  QCOMPARE( f2.pointSize(), f1.pointSize() );
  QCOMPARE( f2.italic(), f1.italic() );
  QCOMPARE( f2.weight(), f1.weight() );
#ifndef Q_OS_WIN
  QCOMPARE( f2.styleName(), QString( "Bold" ) );
#else
  QVERIFY( f2.bold() );
#endif

  //test numeric weight
  f1.setWeight( QFont::ExtraLight );
  fontElem = QgsFontUtils::toXmlElement( f1, doc, QStringLiteral( "test" ) );
  QVERIFY( f2.weight() != f1.weight() );
  QVERIFY( QgsFontUtils::setFromXmlElement( f2, fontElem ) );
  QCOMPARE( f2.weight(), f1.weight() );

  //test reading from null element
  const QDomElement badElem;
  QVERIFY( !QgsFontUtils::setFromXmlElement( f2, badElem ) );
}

void TestQgsFontUtils::fromChildNode()
{
  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" )
  );
  QDomDocument doc( documentType );

  QFont f1 = QgsFontUtils::getStandardTestFont();
  f1.setPointSize( 48 );
  const QDomElement fontElem = QgsFontUtils::toXmlElement( f1, doc, QStringLiteral( "testNode" ) );
  QDomElement parentElem = doc.createElement( QStringLiteral( "parent" ) );

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

void TestQgsFontUtils::toCss()
{
  QFont f1 = QgsFontUtils::getStandardTestFont();
  f1.setPixelSize( 48 );
  QCOMPARE( QgsFontUtils::asCSS( f1 ), QString( "font-family: QGIS Vera Sans;font-style: normal;font-weight: 400;font-size: 48px;" ) );
  f1.setItalic( true );
  QCOMPARE( QgsFontUtils::asCSS( f1 ), QString( "font-family: QGIS Vera Sans;font-style: italic;font-weight: 400;font-size: 48px;" ) );
  f1.setStyle( QFont::StyleOblique );
  QCOMPARE( QgsFontUtils::asCSS( f1 ), QString( "font-family: QGIS Vera Sans;font-style: oblique;font-weight: 400;font-size: 48px;" ) );
  f1.setBold( true );
  QCOMPARE( QgsFontUtils::asCSS( f1 ), QString( "font-family: QGIS Vera Sans;font-style: oblique;font-weight: 700;font-size: 48px;" ) );
  f1.setPointSizeF( 12.5 );
  QCOMPARE( QgsFontUtils::asCSS( f1 ), QString( "font-family: QGIS Vera Sans;font-style: oblique;font-weight: 700;font-size: 12.5px;" ) );
  QCOMPARE( QgsFontUtils::asCSS( f1, 10 ), QString( "font-family: QGIS Vera Sans;font-style: oblique;font-weight: 700;font-size: 125px;" ) );
}

QGSTEST_MAIN( TestQgsFontUtils )
#include "testqgsfontutils.moc"
