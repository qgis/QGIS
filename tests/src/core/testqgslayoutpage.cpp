/***************************************************************************
                         testqgslayoutpage.cpp
                         ---------------------
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

#include "qgslayout.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitemregistry.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmultirenderchecker.h"
#include "qgslayoutpagecollection.h"
#include <QObject>
#include "qgstest.h"
#include "qgsfillsymbol.h"

class TestQgsLayoutPage : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void itemType();
    void pageSize();
    void decodePageOrientation();
    void grid();
    void defaultPaper();
    void transparentPaper(); //test totally transparent paper style
    void borderedPaper(); //test page with border
    void markerLinePaper(); //test page with marker line borde

    void hiddenPages(); //test hidden page boundaries

    void pageLayout(); //test page layout

  private:
    QString mReport;

};

void TestQgsLayoutPage::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Page Tests</h1>\n" );
}

void TestQgsLayoutPage::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLayoutPage::init()
{

}

void TestQgsLayoutPage::cleanup()
{

}

void TestQgsLayoutPage::itemType()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  QCOMPARE( page->type(), static_cast< int >( QgsLayoutItemRegistry::LayoutPage ) );
}

void TestQgsLayoutPage::pageSize()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 270, 297, QgsUnitTypes::LayoutMeters ) );
  QCOMPARE( page->pageSize().width(), 270.0 );
  QCOMPARE( page->pageSize().height(), 297.0 );
  QCOMPARE( page->pageSize().units(), QgsUnitTypes::LayoutMeters );
  QCOMPARE( page->orientation(), QgsLayoutItemPage::Portrait );
  page->setPageSize( QgsLayoutSize( 297, 270, QgsUnitTypes::LayoutMeters ) );
  QCOMPARE( page->orientation(), QgsLayoutItemPage::Landscape );

  // from registry
  QVERIFY( !page->setPageSize( "hoooyeah" ) );
  // should be unchanged
  QCOMPARE( page->pageSize().width(), 297.0 );
  QCOMPARE( page->pageSize().height(), 270.0 );
  QCOMPARE( page->pageSize().units(), QgsUnitTypes::LayoutMeters );

  // good size
  QVERIFY( page->setPageSize( "A5" ) );
  QCOMPARE( page->pageSize().width(), 148.0 );
  QCOMPARE( page->pageSize().height(), 210.0 );
  QCOMPARE( page->pageSize().units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( page->orientation(), QgsLayoutItemPage::Portrait );

  QVERIFY( page->setPageSize( "A5", QgsLayoutItemPage::Landscape ) );
  QCOMPARE( page->pageSize().width(), 210.0 );
  QCOMPARE( page->pageSize().height(), 148.0 );
  QCOMPARE( page->pageSize().units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( page->orientation(), QgsLayoutItemPage::Landscape );

}

void TestQgsLayoutPage::decodePageOrientation()
{
  //test good string
  bool ok = false;
  QCOMPARE( QgsLayoutItemPage::decodePageOrientation( QString( " porTrait " ), &ok ), QgsLayoutItemPage::Portrait );
  QVERIFY( ok );
  QCOMPARE( QgsLayoutItemPage::decodePageOrientation( QString( "landscape" ), &ok ), QgsLayoutItemPage::Landscape );
  QVERIFY( ok );

  //test bad string
  QgsLayoutItemPage::decodePageOrientation( QString(), &ok );
  QVERIFY( !ok );
}

void TestQgsLayoutPage::grid()
{
  // test that grid follows page around
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );

  // should have a grid
  QVERIFY( page->mGrid.get() );

  // grid is parented to page, so while the grid should resize to match
  // page size, it should always report pos() of 0,0 (origin of page)
  page->attemptMove( QgsLayoutPoint( 5, 15 ) );
  page->attemptResize( QgsLayoutSize( 100, 200 ) );
  QCOMPARE( page->mGrid->rect().width(), 100.0 );
  QCOMPARE( page->mGrid->rect().height(), 200.0 );
  QCOMPARE( page->mGrid->pos().x(), 0.0 );
  QCOMPARE( page->mGrid->pos().y(), 0.0 );

  page->attemptMove( QgsLayoutPoint( 25, 35 ) );
  QCOMPARE( page->mGrid->rect().width(), 100.0 );
  QCOMPARE( page->mGrid->rect().height(), 200.0 );
  QCOMPARE( page->mGrid->pos().x(), 0.0 );
  QCOMPARE( page->mGrid->pos().y(), 0.0 );

  page->attemptResize( QgsLayoutSize( 150, 250 ) );
  QCOMPARE( page->mGrid->rect().width(), 150.0 );
  QCOMPARE( page->mGrid->rect().height(), 250.0 );
  QCOMPARE( page->mGrid->pos().x(), 0.0 );
  QCOMPARE( page->mGrid->pos().y(), 0.0 );

}

void TestQgsLayoutPage::defaultPaper()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );

  QgsLayoutChecker checker( QStringLiteral( "composerpaper_default" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutPage::transparentPaper()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol( new QgsFillSymbol() );
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::transparent );
  simpleFill->setStrokeColor( Qt::transparent );
  l.pageCollection()->setPageStyleSymbol( fillSymbol.get() );

  QgsLayoutChecker checker( QStringLiteral( "composerpaper_transparent" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutPage::borderedPaper()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol = std::make_unique< QgsFillSymbol >();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::white );
  simpleFill->setStrokeColor( Qt::black );
  simpleFill->setStrokeWidth( 6 );
  l.pageCollection()->setPageStyleSymbol( fillSymbol.get() );

  QgsLayoutChecker checker( QStringLiteral( "composerpaper_bordered" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutPage::markerLinePaper()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );

  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  static_cast< QgsSimpleMarkerSymbolLayer * >( markerLine->subSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  std::unique_ptr< QgsFillSymbol > markerLineSymbol = std::make_unique< QgsFillSymbol >();
  markerLineSymbol->changeSymbolLayer( 0, markerLine );
  l.pageCollection()->setPageStyleSymbol( markerLineSymbol.get() );

  QgsLayoutChecker checker( QStringLiteral( "composerpaper_markerborder" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutPage::hiddenPages()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol = std::make_unique< QgsFillSymbol >();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::blue );
  simpleFill->setStrokeColor( Qt::transparent );
  l.pageCollection()->setPageStyleSymbol( fillSymbol.get() );

  l.renderContext().setPagesVisible( false );

  QgsLayoutChecker checker( QStringLiteral( "composerpaper_hidden" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_paper" ) );
  const bool result = checker.testLayout( mReport );
  QVERIFY( result );

}

void TestQgsLayoutPage::pageLayout()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemPage > page1( new QgsLayoutItemPage( &l ) );
  page1->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );

  const QPageLayout layout1 = page1->pageLayout();

  QCOMPARE( layout1.orientation(), QPageLayout::Landscape );
  QCOMPARE( layout1.units(), QPageLayout::Millimeter );
  QCOMPARE( layout1.pageSize().size( QPageSize::Millimeter ).width(), 210 );
  QCOMPARE( layout1.pageSize().size( QPageSize::Millimeter ).height(), 297 );

  std::unique_ptr< QgsLayoutItemPage > page2( new QgsLayoutItemPage( &l ) );
  page2->setPageSize( QgsLayoutSize( 210, 297, QgsUnitTypes::LayoutMillimeters ) );

  const QPageLayout layout2 = page2->pageLayout();

  QCOMPARE( layout2.orientation(), QPageLayout::Portrait );
  QCOMPARE( layout2.units(), QPageLayout::Millimeter );
  QCOMPARE( layout2.pageSize().size( QPageSize::Millimeter ).width(), 210 );
  QCOMPARE( layout2.pageSize().size( QPageSize::Millimeter ).height(), 297 );
}

QGSTEST_MAIN( TestQgsLayoutPage )
#include "testqgslayoutpage.moc"
