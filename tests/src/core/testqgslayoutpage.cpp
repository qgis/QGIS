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
#include <QObject>
#include "qgstest.h"

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

  private:
    QString mReport;

};

void TestQgsLayoutPage::initTestCase()
{
  mReport = "<h1>Layout Page Tests</h1>\n";
}

void TestQgsLayoutPage::cleanupTestCase()
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
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
  QCOMPARE( page->type(), static_cast< int >( QgsLayoutItemRegistry::LayoutPage ) );
}

void TestQgsLayoutPage::pageSize()
{
  QgsProject p;
  QgsLayout l( &p );
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
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
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );

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

QGSTEST_MAIN( TestQgsLayoutPage )
#include "testqgslayoutpage.moc"
