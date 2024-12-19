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
#include "qgstest.h"
#include <QObject>
#include <qgsprovidermetadata.h>
#include <qgsproviderregistry.h>

class TestQgsProviderMetadata: public QObject
{
    Q_OBJECT

  public:
    TestQgsProviderMetadata() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

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

  mMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "raster" ) );
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
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testOne" ), QStringLiteral( "yes" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testTwo" ), QStringLiteral( "1" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testThree" ), 1 );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testFour" ), QStringLiteral( "true" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testFive" ), true );

  QVariantMap expected = { { QStringLiteral( "testOne" ), QVariant( true ) },
    { QStringLiteral( "testTwo" ), QVariant( true ) },
    { QStringLiteral( "testThree" ), QVariant( true ) },
    { QStringLiteral( "testFour" ), QVariant( true ) },
    { QStringLiteral( "testFive" ), QVariant( true ) }
  };

  QCOMPARE( uri, expected );

  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testOne" ), QStringLiteral( "YES" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testFour" ), QStringLiteral( "TRUE" ) );

  QCOMPARE( uri, expected );

  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testOne" ), QStringLiteral( "no" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testTwo" ), QStringLiteral( "0" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testThree" ), 0 );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testFour" ), QStringLiteral( "false" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testFive" ), false );

  expected = { { QStringLiteral( "testOne" ), QVariant( false ) },
    { QStringLiteral( "testTwo" ), QVariant( false ) },
    { QStringLiteral( "testThree" ), QVariant( false ) },
    { QStringLiteral( "testFour" ), QVariant( false ) },
    { QStringLiteral( "testFive" ), QVariant( false ) }
  };
  QCOMPARE( uri, expected );

  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testOne" ), QStringLiteral( "NO" ) );
  QgsProviderMetadata::setBoolParameter( uri, QStringLiteral( "testFour" ), QStringLiteral( "FALSE" ) );

  QCOMPARE( uri, expected );

  uri[ QStringLiteral( "testOne" ) ] = QStringLiteral( "yes" );
  uri[ QStringLiteral( "testTwo" ) ] = QStringLiteral( "1" );
  uri[ QStringLiteral( "testThree" ) ] = QStringLiteral( "true" );
  uri[ QStringLiteral( "testFour" ) ] = 1;
  uri[ QStringLiteral( "testFive" ) ] = true;
  uri[ QStringLiteral( "testSix" ) ] = QStringLiteral( "otherValue" );

  QVERIFY( mMetadata->boolParameter( uri, QStringLiteral( "testOne" ), false ) );
  QVERIFY( mMetadata->boolParameter( uri, QStringLiteral( "testTwo" ), false ) );
  QVERIFY( mMetadata->boolParameter( uri, QStringLiteral( "testThree" ), false ) );
  QVERIFY( mMetadata->boolParameter( uri, QStringLiteral( "testFour" ), false ) );
  QVERIFY( mMetadata->boolParameter( uri, QStringLiteral( "testFive" ), false ) );
  QVERIFY( !mMetadata->boolParameter( uri, QStringLiteral( "testSix" ), false ) );
}

QGSTEST_MAIN( TestQgsProviderMetadata )
#include "testqgsprovidermetadata.moc"
