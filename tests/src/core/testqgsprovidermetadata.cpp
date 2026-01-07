/***************************************************************************
                         testqgsprovidermetadata.cpp
                         ---------------
    begin                : April 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgstest.h"

#include <QObject>

class TestQgsProviderMetadata : public QObject
{
    Q_OBJECT

  public:
    TestQgsProviderMetadata() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void checkBoolParameterSetting();

  private:
    QgsProviderMetadata *mMetadata = nullptr;
};

void TestQgsProviderMetadata::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsProviderMetadata::init()
{
  //create some objects that will be used in all tests...
  //create a temporal object that will be used in all tests...

  mMetadata = QgsProviderRegistry::instance()->providerMetadata( u"raster"_s );
}

void TestQgsProviderMetadata::cleanup()
{
}

void TestQgsProviderMetadata::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProviderMetadata::checkBoolParameterSetting()
{
  QVariantMap uri;
  QgsProviderMetadata::setBoolParameter( uri, u"testOne"_s, u"yes"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testTwo"_s, u"1"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testThree"_s, 1 );
  QgsProviderMetadata::setBoolParameter( uri, u"testFour"_s, u"true"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testFive"_s, true );

  QVariantMap expected = { { u"testOne"_s, QVariant( true ) }, { u"testTwo"_s, QVariant( true ) }, { u"testThree"_s, QVariant( true ) }, { u"testFour"_s, QVariant( true ) }, { u"testFive"_s, QVariant( true ) } };

  QCOMPARE( uri, expected );

  QgsProviderMetadata::setBoolParameter( uri, u"testOne"_s, u"YES"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testFour"_s, u"TRUE"_s );

  QCOMPARE( uri, expected );

  QgsProviderMetadata::setBoolParameter( uri, u"testOne"_s, u"no"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testTwo"_s, u"0"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testThree"_s, 0 );
  QgsProviderMetadata::setBoolParameter( uri, u"testFour"_s, u"false"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testFive"_s, false );

  expected = { { u"testOne"_s, QVariant( false ) }, { u"testTwo"_s, QVariant( false ) }, { u"testThree"_s, QVariant( false ) }, { u"testFour"_s, QVariant( false ) }, { u"testFive"_s, QVariant( false ) } };
  QCOMPARE( uri, expected );

  QgsProviderMetadata::setBoolParameter( uri, u"testOne"_s, u"NO"_s );
  QgsProviderMetadata::setBoolParameter( uri, u"testFour"_s, u"FALSE"_s );

  QCOMPARE( uri, expected );

  uri[u"testOne"_s] = u"yes"_s;
  uri[u"testTwo"_s] = u"1"_s;
  uri[u"testThree"_s] = u"true"_s;
  uri[u"testFour"_s] = 1;
  uri[u"testFive"_s] = true;
  uri[u"testSix"_s] = u"otherValue"_s;

  QVERIFY( mMetadata->boolParameter( uri, u"testOne"_s, false ) );
  QVERIFY( mMetadata->boolParameter( uri, u"testTwo"_s, false ) );
  QVERIFY( mMetadata->boolParameter( uri, u"testThree"_s, false ) );
  QVERIFY( mMetadata->boolParameter( uri, u"testFour"_s, false ) );
  QVERIFY( mMetadata->boolParameter( uri, u"testFive"_s, false ) );
  QVERIFY( !mMetadata->boolParameter( uri, u"testSix"_s, false ) );
}

QGSTEST_MAIN( TestQgsProviderMetadata )
#include "testqgsprovidermetadata.moc"
