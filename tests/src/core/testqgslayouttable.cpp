/***************************************************************************
                         testqgslayouttable.cpp
                         ----------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsapplication.h"
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutframe.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfeature.h"
#include "qgsmultirenderchecker.h"
#include "qgsfontutils.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsreadwritecontext.h"

#include <QObject>
#include "qgstest.h"

class TestQgsLayoutTable : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutTable() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void attributeTableHeadings(); //test retrieving attribute table headers
    void attributeTableRows(); //test retrieving attribute table rows
    void attributeTableFilterFeatures(); //test filtering attribute table rows
    void attributeTableSetAttributes(); //test subset of attributes in table
    void attributeTableVisibleOnly(); //test displaying only visible attributes
    void attributeTableRender(); //test rendering attribute table
    void manualColumnWidth(); //test setting manual column widths
    void attributeTableEmpty(); //test empty modes for attribute table
    void showEmptyRows(); //test showing empty rows
    void attributeTableExtend();
    void attributeTableRepeat();
    void attributeTableAtlasSource(); //test attribute table in atlas feature mode
    void attributeTableRelationSource(); //test attribute table in relation mode
    void contentsContainsRow(); //test the contentsContainsRow function
    void removeDuplicates(); //test removing duplicate rows
    void multiLineText(); //test rendering a table with multiline text
    void horizontalGrid(); //test rendering a table with horizontal-only grid
    void verticalGrid(); //test rendering a table with vertical-only grid
    void align(); //test alignment of table cells
    void wrapChar(); //test setting wrap character
    void autoWrap(); //test auto word wrap
    void cellStyles(); //test cell styles
    void cellStylesRender(); //test rendering cell styles
    void dataDefinedSource();

  private:
    QgsVectorLayer *mVectorLayer = nullptr;
    QString mReport;

    //compares rows in table to expected rows
    void compareTable( QgsLayoutItemAttributeTable *table, const QVector<QStringList> &expectedRows );
};

void TestQgsLayoutTable::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mVectorLayer );

  mReport = QStringLiteral( "<h1>Layout Table Tests</h1>\n" );
}

void TestQgsLayoutTable::cleanupTestCase()
{
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

void TestQgsLayoutTable::init()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );
}

void TestQgsLayoutTable::cleanup()
{
}

void TestQgsLayoutTable::attributeTableHeadings()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  //test retrieving attribute table headers
  QStringList expectedHeaders;
  expectedHeaders << QStringLiteral( "Class" ) << QStringLiteral( "Heading" ) << QStringLiteral( "Importance" ) << QStringLiteral( "Pilots" ) << QStringLiteral( "Cabin Crew" ) << QStringLiteral( "Staff" );

  //get header labels and compare
  QMap<int, QString> headerMap = table->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    evaluated = headerIt.value();
    expected = expectedHeaders.at( headerIt.key() );
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsLayoutTable::compareTable( QgsLayoutItemAttributeTable *table, const QVector<QStringList> &expectedRows )
{
  //retrieve rows and check
  QgsLayoutTableContents tableContents;
  bool result = table->getTableContents( tableContents );
  QCOMPARE( result, true );

  QgsLayoutTableContents::const_iterator resultIt = tableContents.constBegin();
  int rowNumber = 0;
  int colNumber = 0;

  //check that number of rows matches expected
  QCOMPARE( tableContents.count(), expectedRows.count() );

  for ( ; resultIt != tableContents.constEnd(); ++resultIt )
  {
    colNumber = 0;
    QgsLayoutTableRow::const_iterator cellIt = ( *resultIt ).constBegin();
    for ( ; cellIt != ( *resultIt ).constEnd(); ++cellIt )
    {
      QCOMPARE( ( *cellIt ).toString(), expectedRows.at( rowNumber ).at( colNumber ) );
      colNumber++;
    }
    //also check that number of columns matches expected
    QCOMPARE( ( *resultIt ).count(), expectedRows.at( rowNumber ).count() );

    rowNumber++;
  }
}

void TestQgsLayoutTable::attributeTableRows()
{
  //test retrieving attribute table rows

  QVector<QStringList> expectedRows;
  QStringList row;
  row << QStringLiteral( "Jet" ) << QStringLiteral( "90" ) << QStringLiteral( "3" ) << QStringLiteral( "2" ) << QStringLiteral( "0" ) << QStringLiteral( "2" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "Biplane" ) << QStringLiteral( "0" ) << QStringLiteral( "1" ) << QStringLiteral( "3" ) << QStringLiteral( "3" ) << QStringLiteral( "6" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "85" ) << QStringLiteral( "3" ) << QStringLiteral( "1" ) << QStringLiteral( "1" ) << QStringLiteral( "2" );
  expectedRows.append( row );

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  //retrieve rows and check
  table->setMaximumNumberOfFeatures( 3 );
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::attributeTableFilterFeatures()
{
  //test filtering attribute table rows
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  table->setMaximumNumberOfFeatures( 10 );
  table->setFeatureFilter( QStringLiteral( "\"Class\"='B52'" ) );
  table->setFilterFeatures( true );

  QVector<QStringList> expectedRows;
  QStringList row;
  row << QStringLiteral( "B52" ) << QStringLiteral( "0" ) << QStringLiteral( "10" ) << QStringLiteral( "2" ) << QStringLiteral( "1" ) << QStringLiteral( "3" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "B52" ) << QStringLiteral( "12" ) << QStringLiteral( "10" ) << QStringLiteral( "1" ) << QStringLiteral( "1" ) << QStringLiteral( "2" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "B52" ) << QStringLiteral( "34" ) << QStringLiteral( "10" ) << QStringLiteral( "2" ) << QStringLiteral( "1" ) << QStringLiteral( "3" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "B52" ) << QStringLiteral( "80" ) << QStringLiteral( "10" ) << QStringLiteral( "2" ) << QStringLiteral( "1" ) << QStringLiteral( "3" );
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::attributeTableSetAttributes()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  //test subset of attributes in table
  QStringList attributes;
  attributes << QStringLiteral( "Class" ) << QStringLiteral( "Pilots" ) << QStringLiteral( "Cabin Crew" );
  table->setDisplayedFields( attributes );
  table->setMaximumNumberOfFeatures( 3 );

  //check headers
  QStringList expectedHeaders;
  expectedHeaders << QStringLiteral( "Class" ) << QStringLiteral( "Pilots" ) << QStringLiteral( "Cabin Crew" );

  //get header labels and compare
  QMap<int, QString> headerMap = table->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    evaluated = headerIt.value();
    expected = expectedHeaders.at( headerIt.key() );
    QCOMPARE( evaluated, expected );
  }

  QVector<QStringList> expectedRows;
  QStringList row;
  row << QStringLiteral( "Jet" ) << QStringLiteral( "2" ) << QStringLiteral( "0" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "Biplane" ) << QStringLiteral( "3" ) << QStringLiteral( "3" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "1" ) << QStringLiteral( "1" );
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::attributeTableVisibleOnly()
{
  //test displaying only visible attributes
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setExtent( QgsRectangle( -131.767, 30.558, -110.743, 41.070 ) );
  l.addLayoutItem( map );

  table->setMap( map );
  table->setDisplayOnlyVisibleFeatures( true );

  QVector<QStringList> expectedRows;
  QStringList row;
  row << QStringLiteral( "Jet" ) << QStringLiteral( "90" ) << QStringLiteral( "3" ) << QStringLiteral( "2" ) << QStringLiteral( "0" ) << QStringLiteral( "2" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "Biplane" ) << QStringLiteral( "240" ) << QStringLiteral( "1" ) << QStringLiteral( "3" ) << QStringLiteral( "2" ) << QStringLiteral( "5" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "180" ) << QStringLiteral( "3" ) << QStringLiteral( "1" ) << QStringLiteral( "0" ) << QStringLiteral( "1" );
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::attributeTableRender()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 20 );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_render" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  bool result = checker.testLayout( mReport );
  QVERIFY( result );
}

void TestQgsLayoutTable::manualColumnWidth()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 20 );
  table->columns().at( 0 )->setWidth( 5 );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_columnwidth" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  bool result = checker.testLayout( mReport, 0 );
  QVERIFY( result );
}

void TestQgsLayoutTable::attributeTableEmpty()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 20 );
  //hide all features from table
  table->setFeatureFilter( QStringLiteral( "1=2" ) );
  table->setFilterFeatures( true );

  table->setEmptyTableBehavior( QgsLayoutTable::HeadersOnly );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_headersonly" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  QVERIFY( checker.testLayout( mReport, 0 ) );

  table->setEmptyTableBehavior( QgsLayoutTable::HideTable );
  QgsLayoutChecker checker2( QStringLiteral( "composerattributetable_hidetable" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  QVERIFY( checker2.testLayout( mReport, 0 ) );

  table->setEmptyTableBehavior( QgsLayoutTable::ShowMessage );
  table->setEmptyTableMessage( QStringLiteral( "no rows" ) );
  QgsLayoutChecker checker3( QStringLiteral( "composerattributetable_showmessage" ), &l );
  checker3.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  QVERIFY( checker3.testLayout( mReport, 0 ) );
}

void TestQgsLayoutTable::showEmptyRows()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 3 );
  table->setShowEmptyRows( true );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_drawempty" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  QVERIFY( checker.testLayout( mReport, 0 ) );
}

void TestQgsLayoutTable::attributeTableExtend()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  //test that adding and removing frames automatically does not result in a crash
  table->removeFrame( 1 );

  //force auto creation of some new frames
  table->setResizeMode( QgsLayoutMultiFrame::ExtendToNextPage );

  l.setSelectedItem( table->frame( 1 ) );

  //now auto remove extra created frames
  table->setMaximumNumberOfFeatures( 1 );
}

void TestQgsLayoutTable::attributeTableRepeat()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  //test that creating and removing new frames in repeat mode does not crash

  table->setResizeMode( QgsLayoutMultiFrame::UseExistingFrames );
  //remove extra frames
  for ( int idx = table->frameCount(); idx > 0; --idx )
  {
    table->removeFrame( idx - 1 );
  }

  table->setMaximumNumberOfFeatures( 1 );

  //force auto creation of some new frames
  table->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );

  for ( int features = 0; features < 50; ++features )
  {
    table->setMaximumNumberOfFeatures( features );
  }

  //and then the reverse....
  for ( int features = 50; features > 1; --features )
  {
    table->setMaximumNumberOfFeatures( features );
  }
}

void TestQgsLayoutTable::attributeTableAtlasSource()
{
  QgsLayout l( QgsProject::instance() );
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );

  table->setSource( QgsLayoutItemAttributeTable::AtlasFeature );

  //setup atlas
  QgsVectorLayer *vectorLayer = nullptr;
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  vectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                    vectorFileInfo.completeBaseName(),
                                    QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( vectorLayer );
  l.reportContext().setLayer( vectorLayer );

  QgsFeature f;
  QgsFeatureIterator it = vectorLayer->getFeatures();
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  QCOMPARE( table->contents().length(), 1 );
  QgsLayoutTableRow row = table->contents().at( 0 );

  //check a couple of results
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 90 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 2 ) );
  QCOMPARE( row.at( 4 ), QVariant( 0 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );

  //next atlas feature
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  QCOMPARE( table->contents().length(), 1 );
  row = table->contents().at( 0 );
  QCOMPARE( row.at( 0 ), QVariant( "Biplane" ) );
  QCOMPARE( row.at( 1 ), QVariant( 0 ) );
  QCOMPARE( row.at( 2 ), QVariant( 1 ) );
  QCOMPARE( row.at( 3 ), QVariant( 3 ) );
  QCOMPARE( row.at( 4 ), QVariant( 3 ) );
  QCOMPARE( row.at( 5 ), QVariant( 6 ) );

  //next atlas feature
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  QCOMPARE( table->contents().length(), 1 );
  row = table->contents().at( 0 );
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 85 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 1 ) );
  QCOMPARE( row.at( 4 ), QVariant( 1 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );

  //try for a crash when removing current atlas layer
  QgsProject::instance()->removeMapLayer( vectorLayer->id() );
  table->refreshAttributes();

}


void TestQgsLayoutTable::attributeTableRelationSource()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points_relations.shp" );
  QgsVectorLayer *atlasLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );

  QgsProject::instance()->addMapLayer( atlasLayer );

  //setup atlas
  l.reportContext().setLayer( atlasLayer );

  QgsFeature f;
  QgsFeatureIterator it = atlasLayer->getFeatures();
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  //create a relation
  QgsRelation relation;
  relation.setId( QStringLiteral( "testrelation" ) );
  relation.setReferencedLayer( atlasLayer->id() );
  relation.setReferencingLayer( mVectorLayer->id() );
  relation.addFieldPair( QStringLiteral( "Class" ), QStringLiteral( "Class" ) );
  QgsProject::instance()->relationManager()->addRelation( relation );

  table = new QgsLayoutItemAttributeTable( &l );
  table->setMaximumNumberOfFeatures( 50 );
  table->setSource( QgsLayoutItemAttributeTable::RelationChildren );
  table->setRelationId( relation.id() );

  QCOMPARE( f.attribute( "Class" ).toString(), QString( "Jet" ) );
  QCOMPARE( table->contents().length(), 8 );

  QgsLayoutTableRow row = table->contents().at( 0 );

  //check a couple of results
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 90 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 2 ) );
  QCOMPARE( row.at( 4 ), QVariant( 0 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );
  row = table->contents().at( 1 );
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 85 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 1 ) );
  QCOMPARE( row.at( 4 ), QVariant( 1 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );
  row = table->contents().at( 2 );
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 95 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 1 ) );
  QCOMPARE( row.at( 4 ), QVariant( 1 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );

  //next atlas feature
  it.nextFeature( f );
  l.reportContext().setFeature( f );
  QCOMPARE( f.attribute( "Class" ).toString(), QString( "Biplane" ) );
  QCOMPARE( table->contents().length(), 5 );
  row = table->contents().at( 0 );
  QCOMPARE( row.at( 0 ), QVariant( "Biplane" ) );
  QCOMPARE( row.at( 1 ), QVariant( 0 ) );
  QCOMPARE( row.at( 2 ), QVariant( 1 ) );
  QCOMPARE( row.at( 3 ), QVariant( 3 ) );
  QCOMPARE( row.at( 4 ), QVariant( 3 ) );
  QCOMPARE( row.at( 5 ), QVariant( 6 ) );
  row = table->contents().at( 1 );
  QCOMPARE( row.at( 0 ), QVariant( "Biplane" ) );
  QCOMPARE( row.at( 1 ), QVariant( 340 ) );
  QCOMPARE( row.at( 2 ), QVariant( 1 ) );
  QCOMPARE( row.at( 3 ), QVariant( 3 ) );
  QCOMPARE( row.at( 4 ), QVariant( 3 ) );
  QCOMPARE( row.at( 5 ), QVariant( 6 ) );

  //try for a crash when removing current atlas layer
  QgsProject::instance()->removeMapLayer( atlasLayer->id() );

  table->refreshAttributes();
}

void TestQgsLayoutTable::contentsContainsRow()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );

  QgsLayoutTableContents testContents;
  QgsLayoutTableRow row1;
  row1 << QVariant( QStringLiteral( "string 1" ) ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( QStringLiteral( "string 2" ) );
  QgsLayoutTableRow row2;
  row2 << QVariant( QStringLiteral( "string 2" ) ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( QStringLiteral( "string 2" ) );
  //same as row1
  QgsLayoutTableRow row3;
  row3 << QVariant( QStringLiteral( "string 1" ) ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( QStringLiteral( "string 2" ) );
  QgsLayoutTableRow row4;
  row4 << QVariant( QStringLiteral( "string 1" ) ) << QVariant( 2 ) << QVariant( 1.7 ) << QVariant( QStringLiteral( "string 2" ) );

  testContents << row1;
  testContents << row2;

  QVERIFY( table->contentsContainsRow( testContents, row1 ) );
  QVERIFY( table->contentsContainsRow( testContents, row2 ) );
  QVERIFY( table->contentsContainsRow( testContents, row3 ) );
  QVERIFY( !table->contentsContainsRow( testContents, row4 ) );
}

void TestQgsLayoutTable::removeDuplicates()
{
  QgsVectorLayer *dupesLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "dupes" ), QStringLiteral( "memory" ) );
  QVERIFY( dupesLayer->isValid() );
  QgsFeature f1( dupesLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 1 );
  f1.setAttribute( QStringLiteral( "col2" ), 1 );
  f1.setAttribute( QStringLiteral( "col3" ), 1 );
  QgsFeature f2( dupesLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( QStringLiteral( "col1" ), 1 );
  f2.setAttribute( QStringLiteral( "col2" ), 2 );
  f2.setAttribute( QStringLiteral( "col3" ), 2 );
  QgsFeature f3( dupesLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( QStringLiteral( "col1" ), 1 );
  f3.setAttribute( QStringLiteral( "col2" ), 2 );
  f3.setAttribute( QStringLiteral( "col3" ), 3 );
  QgsFeature f4( dupesLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( QStringLiteral( "col1" ), 1 );
  f4.setAttribute( QStringLiteral( "col2" ), 1 );
  f4.setAttribute( QStringLiteral( "col3" ), 1 );
  dupesLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setSource( QgsLayoutItemAttributeTable::LayerAttributes );
  table->setVectorLayer( dupesLayer );
  table->setMaximumNumberOfFeatures( 50 );
  QCOMPARE( table->contents().length(), 4 );

  table->setUniqueRowsOnly( true );
  QCOMPARE( table->contents().length(), 3 );

  //check if removing attributes in unique mode works correctly (should result in duplicate rows,
  //which will be stripped out)
  delete table->columns().takeLast();
  table->refreshAttributes();
  QCOMPARE( table->contents().length(), 2 );
  delete table->columns().takeLast();
  table->refreshAttributes();
  QCOMPARE( table->contents().length(), 1 );
  table->setUniqueRowsOnly( false );
  QCOMPARE( table->contents().length(), 4 );

  delete dupesLayer;
}

void TestQgsLayoutTable::multiLineText()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), "multiline\nstring" );
  f1.setAttribute( QStringLiteral( "col2" ), "singleline string" );
  f1.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( QStringLiteral( "col1" ), "singleline string" );
  f2.setAttribute( QStringLiteral( "col2" ), "multiline\nstring" );
  f2.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( QStringLiteral( "col1" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col2" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col3" ), "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( QStringLiteral( "col1" ), "long triple\nline\nstring" );
  f4.setAttribute( QStringLiteral( "col2" ), "double\nlinestring" );
  f4.setAttribute( QStringLiteral( "col3" ), "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setVectorLayer( multiLineLayer );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_multiline" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  bool result = checker.testLayout( mReport );
  QVERIFY( result );

  delete multiLineLayer;
}

void TestQgsLayoutTable::horizontalGrid()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), "multiline\nstring" );
  f1.setAttribute( QStringLiteral( "col2" ), "singleline string" );
  f1.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( QStringLiteral( "col1" ), "singleline string" );
  f2.setAttribute( QStringLiteral( "col2" ), "multiline\nstring" );
  f2.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( QStringLiteral( "col1" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col2" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col3" ), "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( QStringLiteral( "col1" ), "long triple\nline\nstring" );
  f4.setAttribute( QStringLiteral( "col2" ), "double\nlinestring" );
  f4.setAttribute( QStringLiteral( "col3" ), "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame1->setFrameEnabled( false );
  frame2->setFrameEnabled( false );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( false );
  table->setVectorLayer( multiLineLayer );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_horizontalgrid" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  bool result = checker.testLayout( mReport );
  QVERIFY( result );

  delete multiLineLayer;
}

void TestQgsLayoutTable::verticalGrid()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), "multiline\nstring" );
  f1.setAttribute( QStringLiteral( "col2" ), "singleline string" );
  f1.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( QStringLiteral( "col1" ), "singleline string" );
  f2.setAttribute( QStringLiteral( "col2" ), "multiline\nstring" );
  f2.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( QStringLiteral( "col1" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col2" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col3" ), "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( QStringLiteral( "col1" ), "long triple\nline\nstring" );
  f4.setAttribute( QStringLiteral( "col2" ), "double\nlinestring" );
  f4.setAttribute( QStringLiteral( "col3" ), "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame1->setFrameEnabled( false );
  frame2->setFrameEnabled( false );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setShowGrid( true );
  table->setHorizontalGrid( false );
  table->setVerticalGrid( true );
  table->setVectorLayer( multiLineLayer );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_verticalgrid" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  bool result = checker.testLayout( mReport );
  QVERIFY( result );

  delete multiLineLayer;
}

void TestQgsLayoutTable::align()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), "multiline\nstring" );
  f1.setAttribute( QStringLiteral( "col2" ), "singleline string" );
  f1.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( QStringLiteral( "col1" ), "singleline string" );
  f2.setAttribute( QStringLiteral( "col2" ), "multiline\nstring" );
  f2.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( QStringLiteral( "col1" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col2" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col3" ), "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( QStringLiteral( "col1" ), "long triple\nline\nstring" );
  f4.setAttribute( QStringLiteral( "col2" ), "double\nlinestring" );
  f4.setAttribute( QStringLiteral( "col3" ), "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setVectorLayer( multiLineLayer );

  table->columns().at( 0 )->setHAlignment( Qt::AlignLeft );
  table->columns().at( 0 )->setVAlignment( Qt::AlignTop );
  table->columns().at( 1 )->setHAlignment( Qt::AlignHCenter );
  table->columns().at( 1 )->setVAlignment( Qt::AlignVCenter );
  table->columns().at( 2 )->setHAlignment( Qt::AlignRight );
  table->columns(). at( 2 )->setVAlignment( Qt::AlignBottom );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_align" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  bool result = checker.testLayout( mReport );
  QVERIFY( result );

  delete multiLineLayer;
}

void TestQgsLayoutTable::wrapChar()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  std::unique_ptr< QgsVectorLayer > multiLineLayer = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), "multiline\nstring" );
  f1.setAttribute( QStringLiteral( "col2" ), "singleline string" );
  f1.setAttribute( QStringLiteral( "col3" ), "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  table->setMaximumNumberOfFeatures( 1 );
  table->setVectorLayer( multiLineLayer.get() );
  table->setWrapString( QStringLiteral( "in" ) );

  QVector<QStringList> expectedRows;
  QStringList row;
  row << QStringLiteral( "multil\ne\nstr\ng" ) << QStringLiteral( "s\nglel\ne str\ng" ) << QStringLiteral( "s\nglel\ne" );
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::autoWrap()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  std::unique_ptr< QgsVectorLayer > multiLineLayer = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), "long multiline\nstring" );
  f1.setAttribute( QStringLiteral( "col2" ), "singleline string" );
  f1.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( QStringLiteral( "col1" ), "singleline string" );
  f2.setAttribute( QStringLiteral( "col2" ), "multiline\nstring" );
  f2.setAttribute( QStringLiteral( "col3" ), "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( QStringLiteral( "col1" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col2" ), "singleline" );
  f3.setAttribute( QStringLiteral( "col3" ), "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( QStringLiteral( "col1" ), "a bit long triple line string" );
  f4.setAttribute( QStringLiteral( "col2" ), "double toolongtofitononeline string with some more lines on the end andanotherreallylongline" );
  f4.setAttribute( QStringLiteral( "col3" ), "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setVectorLayer( multiLineLayer.get() );
  table->setWrapBehavior( QgsLayoutTable::WrapText );

  table->columns().at( 0 )->setWidth( 25 );
  table->columns().at( 1 )->setWidth( 25 );
  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_autowrap" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  bool result = checker.testLayout( mReport, 0 );
  QVERIFY( result );
}

void TestQgsLayoutTable::cellStyles()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  QgsLayoutTableStyle original;
  original.enabled = true;
  original.cellBackgroundColor = QColor( 200, 100, 150, 90 );

  //write to xml
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement node = doc.createElement( QStringLiteral( "style" ) );
  QVERIFY( original.writeXml( node, doc ) );

  //read from xml
  QgsLayoutTableStyle styleFromXml;
  QVERIFY( styleFromXml.readXml( node ) );

  //check
  QCOMPARE( original.enabled, styleFromXml.enabled );
  QCOMPARE( original.cellBackgroundColor, styleFromXml.cellBackgroundColor );


  // check writing/reading whole set of styles
  QgsLayoutItemAttributeTable *originalTable = new QgsLayoutItemAttributeTable( &l );

  QgsLayoutTableStyle style1;
  style1.enabled = true;
  style1.cellBackgroundColor = QColor( 25, 50, 75, 100 );
  originalTable->setCellStyle( QgsLayoutTable::FirstRow, style1 );
  QgsLayoutTableStyle style2;
  style1.enabled = false;
  style1.cellBackgroundColor = QColor( 60, 62, 64, 68 );
  originalTable->setCellStyle( QgsLayoutTable::LastColumn, style2 );

  //write to XML
  QDomElement tableElement = doc.createElement( QStringLiteral( "table" ) );
  QVERIFY( originalTable->writeXml( tableElement, doc, QgsReadWriteContext(), true ) );

  //read from XML
  QgsLayoutItemAttributeTable *tableFromXml = new QgsLayoutItemAttributeTable( &l );
  QVERIFY( tableFromXml->readXml( tableElement.firstChildElement(), doc, QgsReadWriteContext(), true ) );

  //check that styles were correctly read
  QCOMPARE( tableFromXml->cellStyle( QgsLayoutTable::FirstRow )->enabled, originalTable->cellStyle( QgsLayoutTable::FirstRow )->enabled );
  QCOMPARE( tableFromXml->cellStyle( QgsLayoutTable::FirstRow )->cellBackgroundColor, originalTable->cellStyle( QgsLayoutTable::FirstRow )->cellBackgroundColor );
  QCOMPARE( tableFromXml->cellStyle( QgsLayoutTable::LastColumn )->enabled, originalTable->cellStyle( QgsLayoutTable::LastColumn )->enabled );
  QCOMPARE( tableFromXml->cellStyle( QgsLayoutTable::LastColumn )->cellBackgroundColor, originalTable->cellStyle( QgsLayoutTable::LastColumn )->cellBackgroundColor );

  //check backgroundColor method
  //build up rules in descending order of precedence
  table->setBackgroundColor( QColor( 50, 50, 50, 50 ) );
  QgsLayoutTableStyle style;
  style.enabled = true;
  style.cellBackgroundColor = QColor( 25, 50, 75, 100 );
  table->setCellStyle( QgsLayoutTable::OddColumns, style );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 50, 50, 50, 50 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 0, 3 ), QColor( 50, 50, 50, 50 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 50, 50, 50, 50 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 1, 3 ), QColor( 50, 50, 50, 50 ) );
  style.cellBackgroundColor = QColor( 30, 80, 90, 23 );
  table->setCellStyle( QgsLayoutTable::EvenColumns, style );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 30, 80, 90, 23 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 0, 3 ), QColor( 30, 80, 90, 23 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 30, 80, 90, 23 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 1, 3 ), QColor( 30, 80, 90, 23 ) );
  style.cellBackgroundColor = QColor( 111, 112, 113, 114 );
  table->setCellStyle( QgsLayoutTable::OddRows, style );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 3 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 30, 80, 90, 23 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 25, 50, 75, 100 ) );
  QCOMPARE( table->backgroundColor( 1, 3 ), QColor( 30, 80, 90, 23 ) );
  QCOMPARE( table->backgroundColor( 2, 0 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 2, 1 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 2, 2 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 2, 3 ), QColor( 111, 112, 113, 114 ) );
  style.cellBackgroundColor = QColor( 222, 223, 224, 225 );
  table->setCellStyle( QgsLayoutTable::EvenRows, style );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 3 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 3 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 2, 0 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 2, 1 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 2, 2 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 2, 3 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 3, 0 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 3, 1 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 3, 2 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 3, 3 ), QColor( 222, 223, 224, 225 ) );
  style.cellBackgroundColor = QColor( 1, 2, 3, 4 );
  table->setCellStyle( QgsLayoutTable::FirstColumn, style );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 3 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 3 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 2, 0 ), QColor( 1, 2, 3, 4 ) );
  style.cellBackgroundColor = QColor( 7, 8, 9, 10 );
  table->setCellStyle( QgsLayoutTable::LastColumn, style );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 5 ), QColor( 7, 8, 9, 10 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 5 ), QColor( 7, 8, 9, 10 ) );
  QCOMPARE( table->backgroundColor( 2, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 2, 5 ), QColor( 7, 8, 9, 10 ) );
  style.cellBackgroundColor = QColor( 87, 88, 89, 90 );
  table->setCellStyle( QgsLayoutTable::HeaderRow, style );
  QCOMPARE( table->backgroundColor( -1, 0 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 1 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 2 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 5 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 111, 112, 113, 114 ) );
  QCOMPARE( table->backgroundColor( 0, 5 ), QColor( 7, 8, 9, 10 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 5 ), QColor( 7, 8, 9, 10 ) );
  QCOMPARE( table->backgroundColor( 2, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 2, 5 ), QColor( 7, 8, 9, 10 ) );
  style.cellBackgroundColor = QColor( 187, 188, 189, 190 );
  table->setCellStyle( QgsLayoutTable::FirstRow, style );
  QCOMPARE( table->backgroundColor( -1, 0 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 1 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 2 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 5 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 0, 5 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 5 ), QColor( 7, 8, 9, 10 ) );
  QCOMPARE( table->backgroundColor( 2, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 2, 5 ), QColor( 7, 8, 9, 10 ) );
  style.cellBackgroundColor = QColor( 147, 148, 149, 150 );
  table->setCellStyle( QgsLayoutTable::LastRow, style );
  QCOMPARE( table->backgroundColor( -1, 0 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 1 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 2 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( -1, 5 ), QColor( 87, 88, 89, 90 ) );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 0, 5 ), QColor( 187, 188, 189, 190 ) );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor( 222, 223, 224, 225 ) );
  QCOMPARE( table->backgroundColor( 1, 5 ), QColor( 7, 8, 9, 10 ) );
  QCOMPARE( table->backgroundColor( 2, 0 ), QColor( 1, 2, 3, 4 ) );
  QCOMPARE( table->backgroundColor( 2, 5 ), QColor( 7, 8, 9, 10 ) );
  QCOMPARE( table->backgroundColor( 9, 0 ), QColor( 147, 148, 149, 150 ) );
  QCOMPARE( table->backgroundColor( 9, 1 ), QColor( 147, 148, 149, 150 ) );
  QCOMPARE( table->backgroundColor( 9, 2 ), QColor( 147, 148, 149, 150 ) );
  QCOMPARE( table->backgroundColor( 9, 5 ), QColor( 147, 148, 149, 150 ) );
}

void TestQgsLayoutTable::cellStylesRender()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 30 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, table );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 30 ) );
  frame1->setFrameEnabled( true );
  frame2->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->addFrame( frame2 );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 10 );
  table->setContentFont( QgsFontUtils::getStandardTestFont() );
  table->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 3 );
  table->setShowEmptyRows( true );

  QgsLayoutTableStyle style;
  style.enabled = true;
  style.cellBackgroundColor = QColor( 25, 50, 75, 100 );
  table->setCellStyle( QgsLayoutTable::OddColumns, style );
  style.cellBackgroundColor = QColor( 90, 110, 150, 200 );
  table->setCellStyle( QgsLayoutTable::EvenRows, style );
  style.cellBackgroundColor = QColor( 150, 160, 210, 200 );
  table->setCellStyle( QgsLayoutTable::HeaderRow, style );
  style.cellBackgroundColor = QColor( 0, 200, 50, 200 );
  table->setCellStyle( QgsLayoutTable::FirstColumn, style );
  style.cellBackgroundColor = QColor( 200, 50, 0, 200 );
  table->setCellStyle( QgsLayoutTable::LastColumn, style );
  style.cellBackgroundColor = QColor( 200, 50, 200, 200 );
  table->setCellStyle( QgsLayoutTable::FirstRow, style );
  style.cellBackgroundColor = QColor( 50, 200, 200, 200 );
  table->setCellStyle( QgsLayoutTable::LastRow, style );

  QgsLayoutChecker checker( QStringLiteral( "composerattributetable_cellstyle" ), &l );
  checker.setColorTolerance( 10 );
  checker.setControlPathPrefix( QStringLiteral( "composer_table" ) );
  QVERIFY( checker.testLayout( mReport, 0 ) );
}

void TestQgsLayoutTable::dataDefinedSource()
{
  // add a couple of layers
  QgsVectorLayer *layer1 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QVERIFY( layer1->isValid() );
  QgsFeature f( layer1->fields() );
  f.setAttributes( QgsAttributes() << 1 << 2 << 3 );
  layer1->dataProvider()->addFeature( f );

  // different field order
  QgsVectorLayer *layer2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col3:integer&field=col2:integer" ), QStringLiteral( "l2" ), QStringLiteral( "memory" ) );
  QVERIFY( layer2->isValid() );
  QgsFeature f2( layer2->fields() );
  f2.setAttributes( QgsAttributes() << 11 << 13 << 12 );
  layer2->dataProvider()->addFeature( f2 );

  // missing fields
  QgsVectorLayer *layer3 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col3:integer" ), QStringLiteral( "l3" ), QStringLiteral( "memory" ) );
  QVERIFY( layer3->isValid() );
  QgsFeature f3( layer3->fields() );
  f3.setAttributes( QgsAttributes() << 21 << 23 );
  layer3->dataProvider()->addFeature( f3 );

  QgsProject p;
  p.addMapLayer( layer1 );
  p.addMapLayer( layer2 );
  p.addMapLayer( layer3 );

  QgsLayout l( &p );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setSource( QgsLayoutItemAttributeTable::LayerAttributes );
  table->setVectorLayer( layer1 );
  table->setMaximumNumberOfFeatures( 50 );
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector< QVariant >() << 1 << 2 << 3 );

  // data defined table name, by layer id
  table->dataDefinedProperties().setProperty( QgsLayoutObject::AttributeTableSourceLayer, layer1->id() );
  table->refresh();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector< QVariant >() << 1 << 2 << 3 );

  // by layer name
  table->dataDefinedProperties().setProperty( QgsLayoutObject::AttributeTableSourceLayer, QStringLiteral( "l2" ) );
  table->refresh();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector< QVariant >() << 11 << 12 << 13 );

  // by layer name (case insensitive)
  table->dataDefinedProperties().setProperty( QgsLayoutObject::AttributeTableSourceLayer, QStringLiteral( "L3" ) );
  table->refresh();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector< QVariant >() << 21 << QVariant() << 23 );

  // delete current data defined layer match
  p.removeMapLayer( layer3->id() );
  QApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  // expect table to return to preset layer
  table->refreshAttributes();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector< QVariant >() << 1 << 2 << 3 );
}

QGSTEST_MAIN( TestQgsLayoutTable )
#include "testqgslayouttable.moc"
