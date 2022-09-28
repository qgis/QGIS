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
#include <memory>
#include "qgstest.h"

//dummy color scheme for testing
class DummyColorScheme : public QgsColorScheme
{
  public:

    DummyColorScheme() = default;

    QString schemeName() const override { return QStringLiteral( "Dummy scheme" ); }

    QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() ) override
    {
      QList< QPair< QColor, QString> > colors;
      if ( context == QLatin1String( "testscheme" ) )
      {
        colors << qMakePair( QColor( 255, 255, 0 ), QStringLiteral( "schemetest" ) );
      }
      else if ( baseColor.isValid() )
      {
        colors << qMakePair( baseColor, QStringLiteral( "base" ) );
      }
      else
      {
        colors << qMakePair( QColor( 255, 0, 0 ), QStringLiteral( "red" ) ) << qMakePair( QColor( 0, 255, 0 ), QString() );
      }
      return colors;
    }

    QgsColorScheme *clone() const override
    {
      return new DummyColorScheme();
    }

};

class TestQgsColorScheme : public QObject
{
    Q_OBJECT

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
  QgsApplication::init();
}

void TestQgsColorScheme::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsColorScheme::init()
{

}

void TestQgsColorScheme::cleanup()
{

}

void TestQgsColorScheme::createScheme()
{
  const std::shared_ptr<DummyColorScheme> dummyScheme( new DummyColorScheme() );
  QVERIFY( dummyScheme.get() );
}

void TestQgsColorScheme::getName()
{
  const std::shared_ptr<DummyColorScheme> dummyScheme( new DummyColorScheme() );
  QCOMPARE( dummyScheme->schemeName(),  QString( "Dummy scheme" ) );
}

void TestQgsColorScheme::colorsNoBase()
{
  const std::shared_ptr<DummyColorScheme> dummyScheme( new DummyColorScheme() );
  const QgsNamedColorList colors = dummyScheme->fetchColors();
  QCOMPARE( colors.length(), 2 );
  QCOMPARE( colors.at( 0 ).first, QColor( 255, 0, 0 ) );
  QCOMPARE( colors.at( 0 ).second, QString( "red" ) );
  QCOMPARE( colors.at( 1 ).first, QColor( 0, 255, 0 ) );
  QCOMPARE( colors.at( 1 ).second, QString() );
}

void TestQgsColorScheme::colorsWithBase()
{
  const std::shared_ptr<DummyColorScheme> dummyScheme( new DummyColorScheme() );
  const QColor testColor = QColor( 0, 0, 255 );
  const QgsNamedColorList colors = dummyScheme->fetchColors( QString(), testColor );
  QCOMPARE( colors.length(), 1 );
  QCOMPARE( colors.at( 0 ).first, testColor );
  QCOMPARE( colors.at( 0 ).second, QString( "base" ) );
}

void TestQgsColorScheme::colorsWithScheme()
{
  const std::shared_ptr<DummyColorScheme> dummyScheme( new DummyColorScheme() );
  const QgsNamedColorList colors = dummyScheme->fetchColors( QStringLiteral( "testscheme" ) );
  QCOMPARE( colors.length(), 1 );
  QCOMPARE( colors.at( 0 ).first, QColor( 255, 255, 0 ) );
  QCOMPARE( colors.at( 0 ).second, QString( "schemetest" ) );
}

void TestQgsColorScheme::clone()
{
  const std::shared_ptr<DummyColorScheme> dummyScheme( new DummyColorScheme() );
  const QgsNamedColorList colors = dummyScheme->fetchColors();
  const std::shared_ptr<QgsColorScheme> dummyScheme2( dummyScheme->clone() );
  const QgsNamedColorList colors2 = dummyScheme2->fetchColors();
  QCOMPARE( colors, colors2 );
}

QGSTEST_MAIN( TestQgsColorScheme )
#include "testqgscolorscheme.moc"
