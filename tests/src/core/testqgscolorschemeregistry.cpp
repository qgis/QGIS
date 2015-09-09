/***************************************************************************
                         testqgscolorschemeregistry.cpp
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

#include "qgscolorschemeregistry.h"
#include "qgscolorscheme.h"
#include <QObject>
#include <QSharedPointer>
#include <QtTest/QtTest>

//dummy color scheme for testing
class DummyColorScheme : public QgsColorScheme
{
  public:

    DummyColorScheme() {}

    virtual ~DummyColorScheme() {}

    virtual QString schemeName() const override { return QString( "Dummy scheme" ); }

    virtual QgsNamedColorList fetchColors( const QString &context = QString(),
                                           const QColor &baseColor = QColor() ) override
    {
      QList< QPair< QColor, QString> > colors;
      if ( context == "testscheme" )
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

    virtual QgsColorScheme* clone() const override
    {
      return new DummyColorScheme();
    }

};

class TestQgsColorSchemeRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void createInstance(); // create global instance of QgsColorSchemeRegistry
    void instanceHasDefaultSchemes(); // check that global instance is populated with default schemes
    void createEmpty(); // check that creating an empty registry works
    void addScheme(); // check adding a scheme to an empty registry
    void addDefaultSchemes(); // check adding a scheme to an empty registry
    void populateFromInstance(); // check populating an empty scheme from the registry
    void removeScheme(); // check removing a scheme from a registry
    void matchingSchemes(); //check fetching schemes of specific type

  private:

};

void TestQgsColorSchemeRegistry::initTestCase()
{

}

void TestQgsColorSchemeRegistry::cleanupTestCase()
{

}

void TestQgsColorSchemeRegistry::init()
{

}

void TestQgsColorSchemeRegistry::cleanup()
{

}

void TestQgsColorSchemeRegistry::createInstance()
{
  QgsColorSchemeRegistry* registry = QgsColorSchemeRegistry::instance();
  QVERIFY( registry );
}

void TestQgsColorSchemeRegistry::instanceHasDefaultSchemes()
{
  //check that scheme instance is initially populated with some schemes
  //(assumes that there is some default schemes)
  QgsColorSchemeRegistry* registry = QgsColorSchemeRegistry::instance();
  QVERIFY( registry->schemes().length() > 0 );
}

void TestQgsColorSchemeRegistry::createEmpty()
{
  //create an empty registry
  QSharedPointer<QgsColorSchemeRegistry> registry( new QgsColorSchemeRegistry() );
  QVERIFY( registry->schemes().length() == 0 );
}

void TestQgsColorSchemeRegistry::addScheme()
{
  //create an empty registry
  QSharedPointer<QgsColorSchemeRegistry> registry( new QgsColorSchemeRegistry() );
  QVERIFY( registry->schemes().length() == 0 );
  QgsColorScheme *recentScheme = new QgsRecentColorScheme();
  registry->addColorScheme( recentScheme );
  QVERIFY( registry->schemes().length() == 1 );
}

void TestQgsColorSchemeRegistry::addDefaultSchemes()
{
  //create an empty registry
  QSharedPointer<QgsColorSchemeRegistry> registry( new QgsColorSchemeRegistry() );
  QVERIFY( registry->schemes().length() == 0 );
  //add default schemes
  registry->addDefaultSchemes();
  QVERIFY( registry->schemes().length() > 0 );
}

void TestQgsColorSchemeRegistry::populateFromInstance()
{
  //create an empty registry
  QSharedPointer<QgsColorSchemeRegistry> registry( new QgsColorSchemeRegistry() );
  QVERIFY( registry->schemes().length() == 0 );
  //add schemes from instance
  registry->populateFromInstance();
  QCOMPARE( registry->schemes().length(), QgsColorSchemeRegistry::instance()->schemes().length() );
}

void TestQgsColorSchemeRegistry::removeScheme()
{
  //create an empty registry
  QSharedPointer<QgsColorSchemeRegistry> registry( new QgsColorSchemeRegistry() );
  QVERIFY( registry->schemes().length() == 0 );
  //add a scheme
  QgsColorScheme *recentScheme = new QgsRecentColorScheme();
  registry->addColorScheme( recentScheme );
  QVERIFY( registry->schemes().length() == 1 );
  //remove the scheme
  QVERIFY( registry->removeColorScheme( recentScheme ) );
  QVERIFY( registry->schemes().length() == 0 );
  //try removing a scheme not in the registry
  QVERIFY( !registry->removeColorScheme( recentScheme ) );
  delete recentScheme;
}

void TestQgsColorSchemeRegistry::matchingSchemes()
{
  QSharedPointer<QgsColorSchemeRegistry> registry( new QgsColorSchemeRegistry() );
  //add some schemes
  QgsColorScheme *recentScheme = new QgsRecentColorScheme();
  registry->addColorScheme( recentScheme );
  DummyColorScheme *dummyScheme = new DummyColorScheme();
  registry->addColorScheme( dummyScheme );
  QVERIFY( registry->schemes().length() == 2 );
  QList< QgsRecentColorScheme* > recentSchemes;
  QList< DummyColorScheme* > dummySchemes;
  registry->schemes( recentSchemes );
  QVERIFY( recentSchemes.length() == 1 );
  QCOMPARE( recentSchemes.at( 0 ), recentScheme );
  registry->schemes( dummySchemes );
  QVERIFY( dummySchemes.length() == 1 );
  QCOMPARE( dummySchemes.at( 0 ), dummyScheme );
}

QTEST_MAIN( TestQgsColorSchemeRegistry )
#include "testqgscolorschemeregistry.moc"
