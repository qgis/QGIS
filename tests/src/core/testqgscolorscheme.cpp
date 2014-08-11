/***************************************************************************
                         testqgscolorscheme.cpp
                         -----------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscolorscheme.h"
#include <QObject>
#include <QtTest>

//dummy color scheme for testing
class DummyColorScheme : public QgsColorScheme
{
  public:

    DummyColorScheme() {}

    virtual ~DummyColorScheme() {}

    virtual QString schemeName() const { return QString( "Dummy scheme" ); }

    virtual QgsNamedColorList fetchColors( const QString context = QString(),
                                           const QColor baseColor = QColor() )
    {
      QList< QPair< QColor, QString> > colors;
      if ( context == QString( "testscheme" ) )
      {
        colors << qMakePair( QColor( 255, 255, 0 ), QString( "schemetest" ) );
      }
      else if ( baseColor.isValid() )
      {
        colors << qMakePair( baseColor, QString( "base" ) );
      }
      else
      {
        colors << qMakePair( QColor( 255, 0, 0 ), QString( "red" ) ) << qMakePair( QColor( 0, 255, 0 ), QString() );
      }
      return colors;
    }

    virtual QgsColorScheme* clone() const
    {
      return new DummyColorScheme();
    }

};

class TestQgsColorScheme : public QObject
{
    Q_OBJECT;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void createScheme(); // test creation of a scheme
    void getName(); //get scheme name
    void colorsNoBase(); //fetch colors with no base color
    void colorsWithBase(); //fetch colors using a base color
    void colorsWithScheme(); //fetch colors using a scheme
    void clone(); //test cloning color scheme

  private:

};

void TestQgsColorScheme::initTestCase()
{

}

void TestQgsColorScheme::cleanupTestCase()
{

}

void TestQgsColorScheme::init()
{

}

void TestQgsColorScheme::cleanup()
{

}

void TestQgsColorScheme::createScheme()
{
  DummyColorScheme* dummyScheme = new DummyColorScheme();
  QVERIFY( dummyScheme );
  delete dummyScheme;
}

void TestQgsColorScheme::getName()
{
  DummyColorScheme* dummyScheme = new DummyColorScheme();
  QCOMPARE( dummyScheme->schemeName(),  QString( "Dummy scheme" ) );
  delete dummyScheme;
}

void TestQgsColorScheme::colorsNoBase()
{
  DummyColorScheme* dummyScheme = new DummyColorScheme();
  QgsNamedColorList colors = dummyScheme->fetchColors();
  QCOMPARE( colors.length(), 2 );
  QCOMPARE( colors.at( 0 ).first, QColor( 255, 0, 0 ) );
  QCOMPARE( colors.at( 0 ).second, QString( "red" ) );
  QCOMPARE( colors.at( 1 ).first, QColor( 0, 255, 0 ) );
  QCOMPARE( colors.at( 1 ).second, QString() );
  delete dummyScheme;
}

void TestQgsColorScheme::colorsWithBase()
{
  DummyColorScheme* dummyScheme = new DummyColorScheme();
  QColor testColor = QColor( 0, 0, 255 );
  QgsNamedColorList colors = dummyScheme->fetchColors( QString(), testColor );
  QCOMPARE( colors.length(), 1 );
  QCOMPARE( colors.at( 0 ).first, testColor );
  QCOMPARE( colors.at( 0 ).second, QString( "base" ) );

  delete dummyScheme;
}

void TestQgsColorScheme::colorsWithScheme()
{
  DummyColorScheme* dummyScheme = new DummyColorScheme();
  QgsNamedColorList colors = dummyScheme->fetchColors( QString( "testscheme" ) );
  QCOMPARE( colors.length(), 1 );
  QCOMPARE( colors.at( 0 ).first, QColor( 255, 255, 0 ) );
  QCOMPARE( colors.at( 0 ).second, QString( "schemetest" ) );

  delete dummyScheme;
}

void TestQgsColorScheme::clone()
{
  DummyColorScheme* dummyScheme = new DummyColorScheme();
  QgsNamedColorList colors = dummyScheme->fetchColors();
  QgsColorScheme* dummyScheme2 = dummyScheme->clone();
  QgsNamedColorList colors2 = dummyScheme2->fetchColors();
  QCOMPARE( colors, colors2 );
}

QTEST_MAIN( TestQgsColorScheme )
#include "moc_testqgscolorscheme.cxx"
