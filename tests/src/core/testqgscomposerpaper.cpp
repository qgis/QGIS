/***************************************************************************
                         testqgscomposerpaper.cpp
                         ----------------------
    begin                : January 2014
    copyright            : (C) 2014 by Marco Hugentobler, Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgsmultirenderchecker.h"
#include "qgscomposershape.h"
#include "qgssymbolv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgslinesymbollayerv2.h"

#include <QObject>
#include <QtTest/QtTest>
#include <QColor>
#include <QPainter>

class TestQgsComposerPaper : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerPaper()
        : mComposition( 0 )
        , mMapSettings( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void defaultPaper(); //test default paper style
    void transparentPaper(); //test totally transparent paper style
    void borderedPaper(); //test page with border
    void markerLinePaper(); //test page with marker line border
    void hiddenPages(); //test hidden page boundaries

  private:
    QgsComposition* mComposition;
    QString mReport;
    QgsMapSettings *mMapSettings;
    // QgsSingleSymbolRendererV2* mSymbolRenderer;

};

void TestQgsComposerPaper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create empty composition
  mMapSettings = new QgsMapSettings();
  mComposition = new QgsComposition( *mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer Paper Tests</h1>\n";
}

void TestQgsComposerPaper::cleanupTestCase()
{
  delete mComposition;
  delete mMapSettings;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
  QgsApplication::exitQgis();
}

void TestQgsComposerPaper::init()
{

}

void TestQgsComposerPaper::cleanup()
{

}

void TestQgsComposerPaper::defaultPaper()
{
  QgsCompositionChecker checker( "composerpaper_default", mComposition );
  checker.setControlPathPrefix( "composer_paper" );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::transparentPaper()
{
  QgsSimpleFillSymbolLayerV2* simpleFill = new QgsSimpleFillSymbolLayerV2();
  QgsFillSymbolV2* fillSymbol = new QgsFillSymbolV2();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::transparent );
  simpleFill->setBorderColor( Qt::transparent );
  mComposition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  QgsCompositionChecker checker( "composerpaper_transparent", mComposition );
  checker.setControlPathPrefix( "composer_paper" );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::borderedPaper()
{
  QgsSimpleFillSymbolLayerV2* simpleFill = new QgsSimpleFillSymbolLayerV2();
  QgsFillSymbolV2* fillSymbol = new QgsFillSymbolV2();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::white );
  simpleFill->setBorderColor( Qt::black );
  simpleFill->setBorderWidth( 6 );
  mComposition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  QgsCompositionChecker checker( "composerpaper_bordered", mComposition );
  checker.setControlPathPrefix( "composer_paper" );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::markerLinePaper()
{
  QgsMarkerLineSymbolLayerV2* markerLine = new QgsMarkerLineSymbolLayerV2();
  QgsFillSymbolV2* markerLineSymbol = new QgsFillSymbolV2();
  markerLineSymbol->changeSymbolLayer( 0, markerLine );
  mComposition->setPageStyleSymbol( markerLineSymbol );
  delete markerLineSymbol;

  QgsCompositionChecker checker( "composerpaper_markerborder", mComposition );
  checker.setControlPathPrefix( "composer_paper" );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerPaper::hiddenPages()
{
  QgsSimpleFillSymbolLayerV2* simpleFill = new QgsSimpleFillSymbolLayerV2();
  QgsFillSymbolV2* fillSymbol = new QgsFillSymbolV2();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::blue );
  simpleFill->setBorderColor( Qt::transparent );
  mComposition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  mComposition->setPagesVisible( false );
  QgsCompositionChecker checker( "composerpaper_hidden", mComposition );
  checker.setControlPathPrefix( "composer_paper" );
  bool result = checker.testComposition( mReport );
  mComposition->setPagesVisible( true );
  QVERIFY( result );
}

QTEST_MAIN( TestQgsComposerPaper )
#include "testqgscomposerpaper.moc"
