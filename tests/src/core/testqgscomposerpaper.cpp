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
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"

#include <QObject>
#include "qgstest.h"
#include <QColor>
#include <QPainter>

class TestQgsComposerPaper : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerPaper() = default;

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
    QgsComposition *mComposition = nullptr;
    QString mReport;
    // QgsSingleSymbolRenderer* mSymbolRenderer;

};

void TestQgsComposerPaper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create empty composition
  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = QStringLiteral( "<h1>Composer Paper Tests</h1>\n" );
}

void TestQgsComposerPaper::cleanupTestCase()
{
  delete mComposition;

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
  QgsCompositionChecker checker( QStringLiteral( "composerpaper_default" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::transparentPaper()
{
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::transparent );
  simpleFill->setStrokeColor( Qt::transparent );
  mComposition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  QgsCompositionChecker checker( QStringLiteral( "composerpaper_transparent" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::borderedPaper()
{
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::white );
  simpleFill->setStrokeColor( Qt::black );
  simpleFill->setStrokeWidth( 6 );
  mComposition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  QgsCompositionChecker checker( QStringLiteral( "composerpaper_bordered" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerPaper::markerLinePaper()
{
  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  QgsFillSymbol *markerLineSymbol = new QgsFillSymbol();
  markerLineSymbol->changeSymbolLayer( 0, markerLine );
  mComposition->setPageStyleSymbol( markerLineSymbol );
  delete markerLineSymbol;

  QgsCompositionChecker checker( QStringLiteral( "composerpaper_markerborder" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerPaper::hiddenPages()
{
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::blue );
  simpleFill->setStrokeColor( Qt::transparent );
  mComposition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  mComposition->setPagesVisible( false );
  QgsCompositionChecker checker( QStringLiteral( "composerpaper_hidden" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  bool result = checker.testComposition( mReport );
  mComposition->setPagesVisible( true );
  QVERIFY( result );
}

QGSTEST_MAIN( TestQgsComposerPaper )
#include "testqgscomposerpaper.moc"
