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
#include "qgscompositionchecker.h"
#include "qgscomposershape.h"
#include "qgssymbolv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgslinesymbollayerv2.h"

#include <QObject>
#include <QtTest>
#include <QColor>
#include <QPainter>

class TestQgsComposerPaper: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void defaultPaper(); //test default paper style
    void transparentPaper(); //test totally transparent paper style
    void borderedPaper(); //test page with border
    void markerLinePaper(); //test page with marker line border

  private:
    QgsComposition* mComposition;
    QString mReport;
    QgsSimpleFillSymbolLayerV2* mSimpleFill;
    QgsMarkerLineSymbolLayerV2* mMarkerLine;
    QgsFillSymbolV2* mFillSymbol;
    QgsFillSymbolV2* mMarkerLineSymbol;
    QgsMapSettings mMapSettings;
    // QgsSingleSymbolRendererV2* mSymbolRenderer;

};

void TestQgsComposerPaper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create empty composition
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  //setup simple fill
  mSimpleFill = new QgsSimpleFillSymbolLayerV2();
  mFillSymbol = new QgsFillSymbolV2();
  mFillSymbol->changeSymbolLayer( 0, mSimpleFill );

  //setup marker line fill
  mMarkerLine = new QgsMarkerLineSymbolLayerV2();
  mMarkerLineSymbol = new QgsFillSymbolV2();
  mMarkerLineSymbol->changeSymbolLayer( 0, mMarkerLine );

  mReport = "<h1>Composer Paper Tests</h1>\n";
}

void TestQgsComposerPaper::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
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
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::transparentPaper()
{
  mSimpleFill->setColor( Qt::transparent );
  mSimpleFill->setBorderColor( Qt::transparent );
  mComposition->setPageStyleSymbol( mFillSymbol );
  QgsCompositionChecker checker( "composerpaper_transparent", mComposition );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::borderedPaper()
{
  mSimpleFill->setColor( Qt::white );
  mSimpleFill->setBorderColor( Qt::black );
  mSimpleFill->setBorderWidth( 6 );
  QgsCompositionChecker checker( "composerpaper_bordered", mComposition );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::markerLinePaper()
{
  mComposition->setPageStyleSymbol( mMarkerLineSymbol );
  QgsCompositionChecker checker( "composerpaper_markerborder", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

QTEST_MAIN( TestQgsComposerPaper )
#include "moc_testqgscomposerpaper.cxx"
