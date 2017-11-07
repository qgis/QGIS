/***************************************************************************
                         testqgslayoutcontext.cpp
                         ------------------------
    begin                : November 2014
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

#include "qgslayoutcontext.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include <QObject>
#include "qgstest.h"
#include <QtTest/QSignalSpy>

class TestQgsLayoutContext: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsLayout
    void flags(); //test QgsLayout flags
    void feature();
    void layer();
    void dpi();
    void renderContextFlags();
    void boundingBoxes();
    void exportLayer();

  private:
    QString mReport;

};

void TestQgsLayoutContext::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Context Tests</h1>\n" );
}

void TestQgsLayoutContext::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLayoutContext::init()
{

}

void TestQgsLayoutContext::cleanup()
{

}

void TestQgsLayoutContext::creation()
{
  QgsLayoutContext *context = new QgsLayoutContext();
  QVERIFY( context );
  delete context;
}

void TestQgsLayoutContext::flags()
{
  QgsLayoutContext context;
  QSignalSpy spyFlagsChanged( &context, &QgsLayoutContext::flagsChanged );

  //test getting and setting flags
  context.setFlags( QgsLayoutContext::Flags( QgsLayoutContext::FlagAntialiasing | QgsLayoutContext::FlagUseAdvancedEffects ) );
  // default flags, so should be no signal
  QCOMPARE( spyFlagsChanged.count(), 0 );

  QVERIFY( context.flags() == ( QgsLayoutContext::FlagAntialiasing | QgsLayoutContext::FlagUseAdvancedEffects ) );
  QVERIFY( context.testFlag( QgsLayoutContext::FlagAntialiasing ) );
  QVERIFY( context.testFlag( QgsLayoutContext::FlagUseAdvancedEffects ) );
  QVERIFY( ! context.testFlag( QgsLayoutContext::FlagDebug ) );
  context.setFlag( QgsLayoutContext::FlagDebug );
  QCOMPARE( spyFlagsChanged.count(), 1 );
  QVERIFY( context.testFlag( QgsLayoutContext::FlagDebug ) );
  context.setFlag( QgsLayoutContext::FlagDebug, false );
  QCOMPARE( spyFlagsChanged.count(), 2 );
  QVERIFY( ! context.testFlag( QgsLayoutContext::FlagDebug ) );
  context.setFlag( QgsLayoutContext::FlagDebug, false ); //no change
  QCOMPARE( spyFlagsChanged.count(), 2 );
  context.setFlags( QgsLayoutContext::FlagDebug );
  QCOMPARE( spyFlagsChanged.count(), 3 );
}

void TestQgsLayoutContext::feature()
{
  QgsLayoutContext context;

  //test removing feature
  context.setFeature( QgsFeature() );
  QVERIFY( !context.feature().isValid() );

  //test setting/getting feature
  QgsFeature testFeature;
  testFeature.initAttributes( 1 );
  testFeature.setAttribute( 0, "Test" );
  context.setFeature( testFeature );
  QCOMPARE( context.feature().attribute( 0 ), testFeature.attribute( 0 ) );
}

void TestQgsLayoutContext::layer()
{
  QgsLayoutContext context;

  //test clearing layer
  context.setLayer( nullptr );
  QVERIFY( !context.layer() );

  //test setting/getting layer
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  context.setLayer( layer );
  QCOMPARE( context.layer(), layer );

  //clear layer
  context.setLayer( nullptr );
  QVERIFY( !context.layer() );

  delete layer;
}

void TestQgsLayoutContext::dpi()
{
  QgsLayoutContext context;
  context.setDpi( 600 );
  QCOMPARE( context.dpi(), 600.0 );
  QCOMPARE( context.measurementConverter().dpi(), 600.0 );
}

void TestQgsLayoutContext::renderContextFlags()
{
  QgsLayoutContext context;
  context.setFlags( 0 );
  QgsRenderContext::Flags flags = context.renderContextFlags();
  QVERIFY( !( flags & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( flags & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( flags & QgsRenderContext::ForceVectorOutput ) );

  context.setFlag( QgsLayoutContext::FlagAntialiasing );
  flags = context.renderContextFlags();
  QVERIFY( ( flags & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( flags & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( flags & QgsRenderContext::ForceVectorOutput ) );

  context.setFlag( QgsLayoutContext::FlagUseAdvancedEffects );
  flags = context.renderContextFlags();
  QVERIFY( ( flags & QgsRenderContext::Antialiasing ) );
  QVERIFY( ( flags & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( flags & QgsRenderContext::ForceVectorOutput ) );
}

void TestQgsLayoutContext::boundingBoxes()
{
  QgsLayoutContext context;
  context.setBoundingBoxesVisible( false );
  QVERIFY( !context.boundingBoxesVisible() );
  context.setBoundingBoxesVisible( true );
  QVERIFY( context.boundingBoxesVisible() );
}

void TestQgsLayoutContext::exportLayer()
{
  QgsLayoutContext context;
  // must default to -1
  QCOMPARE( context.currentExportLayer(), -1 );
  context.setCurrentExportLayer( 1 );
  QCOMPARE( context.currentExportLayer(), 1 );
}

QGSTEST_MAIN( TestQgsLayoutContext )
#include "testqgslayoutcontext.moc"
