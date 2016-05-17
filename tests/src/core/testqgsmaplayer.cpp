/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
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
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>

class TestSignalReceiver : public QObject
{
    Q_OBJECT

  public:
    TestSignalReceiver()
        : QObject( 0 )
        , blendMode( QPainter::CompositionMode_SourceOver )
    {}
    QPainter::CompositionMode blendMode;
  public slots:
    void onBlendModeChanged( const QPainter::CompositionMode blendMode )
    {
      this->blendMode = blendMode;
    }
};

/** \ingroup UnitTests
 * This is a unit test for the QgsMapLayer class.
 */
class TestQgsMapLayer : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapLayer()
        : mpLayer( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void isValid();

    void setBlendMode();

    void isInScaleRange_data();
    void isInScaleRange();


  private:
    QgsMapLayer * mpLayer;
};

void TestQgsMapLayer::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsMapLayer::init()
{
  //create some objects that will be used in all tests...
  //create a map layer that will be used in all tests...
  QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  myFileName = myFileName + "/points.shp";
  QFileInfo myMapFileInfo( myFileName );
  mpLayer = new QgsVectorLayer( myMapFileInfo.filePath(),
                                myMapFileInfo.completeBaseName(), "ogr" );
}

void TestQgsMapLayer::cleanup()
{
  delete mpLayer;
}

void TestQgsMapLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapLayer::isValid()
{
  QVERIFY( mpLayer->isValid() );
}

void TestQgsMapLayer::setBlendMode()
{
  TestSignalReceiver receiver;
  QObject::connect( mpLayer, SIGNAL( blendModeChanged( const QPainter::CompositionMode ) ),
                    &receiver, SLOT( onBlendModeChanged( const QPainter::CompositionMode ) ) );
  QCOMPARE( int( receiver.blendMode ), 0 );
  mpLayer->setBlendMode( QPainter::CompositionMode_Screen );
  // check the signal has been correctly emitted
  QCOMPARE( receiver.blendMode, QPainter::CompositionMode_Screen );
  // check accessor
  QCOMPARE( mpLayer->blendMode(), QPainter::CompositionMode_Screen );
}

void TestQgsMapLayer::isInScaleRange_data()
{
  QTest::addColumn<double>( "scale" );
  QTest::addColumn<bool>( "isInScale" );

  QTest::newRow( "in the middle" ) << 3000.0 << true;
  QTest::newRow( "too low" ) << 1000.0 << false;
  QTest::newRow( "too high" ) << 6000.0 << false;
  QTest::newRow( "max is not inclusive" ) << 5000.0 << false;
  QTest::newRow( "min is inclusive" ) << 2500.0 << true;
  QTest::newRow( "min is inclusive even with conversion errors" ) << static_cast< double >( 1.0f / (( float )1.0 / 2500.0 ) ) << true;
}

void TestQgsMapLayer::isInScaleRange()
{
  QFETCH( double, scale );
  QFETCH( bool, isInScale );

  mpLayer->setMinimumScale( 2500.0 );
  mpLayer->setMaximumScale( 5000.0 );
  mpLayer->setScaleBasedVisibility( true );
  QCOMPARE( mpLayer->isInScaleRange( scale ), isInScale );
  //always in scale range if scale based visibility is false
  mpLayer->setScaleBasedVisibility( false );
  QCOMPARE( mpLayer->isInScaleRange( scale ), true );

}

QTEST_MAIN( TestQgsMapLayer )
#include "testqgsmaplayer.moc"
