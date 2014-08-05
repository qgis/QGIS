/***************************************************************************
                         testqgscomposertablev2.cpp
                         ----------------------
    begin                : September 2014
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

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgscomposerattributetablev2.h"
#include "qgscomposertablecolumn.h"
#include "qgscomposerframe.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfeature.h"
#include "qgscompositionchecker.h"
#include "qgsfontutils.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"

#include <QObject>
#include <QtTest>

class TestQgsComposerTableV2: public QObject
{
    Q_OBJECT;
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

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsMapSettings mMapSettings;
    QgsVectorLayer* mVectorLayer;
    QgsVectorLayer* mVectorLayer2;
    QgsComposerAttributeTableV2* mComposerAttributeTable;
    QgsComposerFrame* mFrame1;
    QgsComposerFrame* mFrame2;
    QString mReport;

    //compares rows in mComposerAttributeTable to expected rows
    void compareTable( QList<QStringList> &expectedRows );
};

void TestQgsComposerTableV2::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "points.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     "ogr" );
  QgsMapLayerRegistry::instance()->addMapLayer( mVectorLayer );
  mVectorLayer2 = new QgsVectorLayer( vectorFileInfo.filePath(),
                                      vectorFileInfo.completeBaseName(),
                                      "ogr" );

  //create composition with composer map
  mMapSettings.setLayers( QStringList() << mVectorLayer->id() << mVectorLayer2->id() );
  mMapSettings.setCrsTransformEnabled( false );
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 portrait

  mComposerAttributeTable = new QgsComposerAttributeTableV2( mComposition, false );

  mFrame1 = new QgsComposerFrame( mComposition, mComposerAttributeTable, 5, 5, 100, 30 );
  mFrame2 = new QgsComposerFrame( mComposition, mComposerAttributeTable, 5, 40, 100, 30 );

  mFrame1->setFrameEnabled( true );
  mFrame2->setFrameEnabled( true );
  mComposerAttributeTable->addFrame( mFrame1 );
  mComposerAttributeTable->addFrame( mFrame2 );

  mComposition->addComposerTableFrame( mComposerAttributeTable, mFrame1 );
  mComposition->addComposerTableFrame( mComposerAttributeTable, mFrame2 );
  mComposerAttributeTable->setVectorLayer( mVectorLayer );
  mComposerAttributeTable->setDisplayOnlyVisibleFeatures( false );
  mComposerAttributeTable->setMaximumNumberOfFeatures( 10 );
  mComposerAttributeTable->setContentFont( QgsFontUtils::getStandardTestFont() );
  mComposerAttributeTable->setHeaderFont( QgsFontUtils::getStandardTestFont() );
  mComposerAttributeTable->setBackgroundColor( Qt::yellow );

  mReport = "<h1>Composer TableV2 Tests</h1>\n";
}

void TestQgsComposerTableV2::cleanupTestCase()
{
  delete mComposition;
  delete mVectorLayer;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerTableV2::init()
{
}

void TestQgsComposerTableV2::cleanup()
{
}

void TestQgsComposerTableV2::attributeTableHeadings()
{
  //test retrieving attribute table headers
  QStringList expectedHeaders;
  expectedHeaders << "Class" << "Heading" << "Importance" << "Pilots" << "Cabin Crew" << "Staff";

  //get header labels and compare
  QMap<int, QString> headerMap = mComposerAttributeTable->headerLabels();
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

void TestQgsComposerTableV2::compareTable( QList<QStringList> &expectedRows )
{
  //retrieve rows and check
  QgsComposerTableContents tableContents;
  bool result = mComposerAttributeTable->getTableContents( tableContents );
  QCOMPARE( result, true );

  QgsComposerTableContents::const_iterator resultIt = tableContents.constBegin();
  int rowNumber = 0;
  int colNumber = 0;

  //check that number of rows matches expected
  QCOMPARE( tableContents.count(), expectedRows.count() );

  for ( ; resultIt != tableContents.constEnd(); ++resultIt )
  {
    colNumber = 0;
    QgsComposerTableRow::const_iterator cellIt = ( *resultIt ).constBegin();
    for ( ; cellIt != ( *resultIt ).constEnd(); ++cellIt )
    {
      QCOMPARE(( *cellIt ).toString(), expectedRows.at( rowNumber ).at( colNumber ) );
      colNumber++;
    }
    //also check that number of columns matches expected
    QCOMPARE(( *resultIt ).count(), expectedRows.at( rowNumber ).count() );

    rowNumber++;
  }
}

void TestQgsComposerTableV2::attributeTableRows()
{
  //test retrieving attribute table rows

  QList<QStringList> expectedRows;
  QStringList row;
  row << "Jet" << "90" << "3" << "2" << "0" << "2";
  expectedRows.append( row );
  row.clear();
  row << "Biplane" << "0" << "1" << "3" << "3" << "6";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "85" << "3" << "1" << "1" << "2";
  expectedRows.append( row );

  //retrieve rows and check
  mComposerAttributeTable->setMaximumNumberOfFeatures( 3 );
  compareTable( expectedRows );
}

void TestQgsComposerTableV2::attributeTableFilterFeatures()
{
  //test filtering attribute table rows
  mComposerAttributeTable->setMaximumNumberOfFeatures( 10 );
  mComposerAttributeTable->setFeatureFilter( QString( "\"Class\"='B52'" ) );
  mComposerAttributeTable->setFilterFeatures( true );

  QList<QStringList> expectedRows;
  QStringList row;
  row << "B52" << "0" << "10" << "2" << "1" << "3";
  expectedRows.append( row );
  row.clear();
  row << "B52" << "12" << "10" << "1" << "1" << "2";
  expectedRows.append( row );
  row.clear();
  row << "B52" << "34" << "10" << "2" << "1" << "3";
  expectedRows.append( row );
  row.clear();
  row << "B52" << "80" << "10" << "2" << "1" << "3";
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( expectedRows );

  mComposerAttributeTable->setFilterFeatures( false );
}

void TestQgsComposerTableV2::attributeTableSetAttributes()
{
  //test subset of attributes in table
  QSet<int> attributes;
  attributes << 0 << 3 << 4;
  mComposerAttributeTable->setDisplayAttributes( attributes );
  mComposerAttributeTable->setMaximumNumberOfFeatures( 3 );

  //check headers
  QStringList expectedHeaders;
  expectedHeaders << "Class" << "Pilots" << "Cabin Crew";

  //get header labels and compare
  QMap<int, QString> headerMap = mComposerAttributeTable->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    evaluated = headerIt.value();
    expected = expectedHeaders.at( headerIt.key() );
    QCOMPARE( evaluated, expected );
  }

  QList<QStringList> expectedRows;
  QStringList row;
  row << "Jet" << "2" << "0";
  expectedRows.append( row );
  row.clear();
  row << "Biplane" << "3" << "3";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "1" << "1";
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( expectedRows );

  attributes.clear();
  mComposerAttributeTable->setDisplayAttributes( attributes );
}

void TestQgsComposerTableV2::attributeTableVisibleOnly()
{
  //test displaying only visible attributes

  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );
  mComposerMap->setNewExtent( QgsRectangle( -131.767, 30.558, -110.743, 41.070 ) );

  mComposerAttributeTable->setComposerMap( mComposerMap );
  mComposerAttributeTable->setDisplayOnlyVisibleFeatures( true );

  QList<QStringList> expectedRows;
  QStringList row;
  row << "Jet" << "90" << "3" << "2" << "0" << "2";
  expectedRows.append( row );
  row.clear();
  row << "Biplane" << "240" << "1" << "3" << "2" << "5";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "180" << "3" << "1" << "0" << "1";
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( expectedRows );

  mComposerAttributeTable->setDisplayOnlyVisibleFeatures( false );
  mComposerAttributeTable->setComposerMap( 0 );
  mComposition->removeItem( mComposerMap );
}

void TestQgsComposerTableV2::attributeTableRender()
{
  mComposerAttributeTable->setMaximumNumberOfFeatures( 20 );
  QgsCompositionChecker checker( "composerattributetable_render", mComposition );
  bool result = checker.testComposition( mReport );
  QVERIFY( result );
}

void TestQgsComposerTableV2::manualColumnWidth()
{
  mComposerAttributeTable->setMaximumNumberOfFeatures( 20 );
  mComposerAttributeTable->columns()->at( 0 )->setWidth( 5 );
  QgsCompositionChecker checker( "composerattributetable_columnwidth", mComposition );
  bool result = checker.testComposition( mReport, 0 );
  mComposerAttributeTable->columns()->at( 0 )->setWidth( 0 );
  QVERIFY( result );
}

void TestQgsComposerTableV2::attributeTableEmpty()
{
  mComposerAttributeTable->setMaximumNumberOfFeatures( 20 );
  //hide all features from table
  mComposerAttributeTable->setFeatureFilter( QString( "1=2" ) );
  mComposerAttributeTable->setFilterFeatures( true );

  mComposerAttributeTable->setEmptyTableBehaviour( QgsComposerTableV2::HeadersOnly );
  QgsCompositionChecker checker( "composerattributetable_headersonly", mComposition );
  QVERIFY( checker.testComposition( mReport, 0 ) );

  mComposerAttributeTable->setEmptyTableBehaviour( QgsComposerTableV2::HideTable );
  QgsCompositionChecker checker2( "composerattributetable_hidetable", mComposition );
  QVERIFY( checker2.testComposition( mReport, 0 ) );

  mComposerAttributeTable->setEmptyTableBehaviour( QgsComposerTableV2::ShowMessage );
  mComposerAttributeTable->setEmptyTableMessage( "no rows" );
  QgsCompositionChecker checker3( "composerattributetable_showmessage", mComposition );
  QVERIFY( checker3.testComposition( mReport, 0 ) );

  mComposerAttributeTable->setFilterFeatures( false );
}

void TestQgsComposerTableV2::showEmptyRows()
{
  mComposerAttributeTable->setMaximumNumberOfFeatures( 3 );
  mComposerAttributeTable->setShowEmptyRows( true );
  QgsCompositionChecker checker( "composerattributetable_drawempty", mComposition );
  QVERIFY( checker.testComposition( mReport, 0 ) );
  mComposerAttributeTable->setMaximumNumberOfFeatures( 20 );
  mComposerAttributeTable->setShowEmptyRows( false );
}

void TestQgsComposerTableV2::attributeTableExtend()
{
  //test that adding and removing frames automatically does not result in a crash
  mComposerAttributeTable->removeFrame( 1 );

  //force auto creation of some new frames
  mComposerAttributeTable->setResizeMode( QgsComposerMultiFrame::ExtendToNextPage );

  mComposition->setSelectedItem( mComposerAttributeTable->frame( 1 ) );

  //now auto remove extra created frames
  mComposerAttributeTable->setMaximumNumberOfFeatures( 1 );
}

void TestQgsComposerTableV2::attributeTableRepeat()
{
  //test that creating and removing new frames in repeat mode does not crash

  mComposerAttributeTable->setResizeMode( QgsComposerMultiFrame::UseExistingFrames );
  //remove extra frames
  for ( int idx = mComposerAttributeTable->frameCount(); idx > 0; --idx )
  {
    mComposerAttributeTable->removeFrame( idx - 1 );
  }

  mComposerAttributeTable->setMaximumNumberOfFeatures( 1 );

  //force auto creation of some new frames
  mComposerAttributeTable->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );

  for ( int features = 0; features < 50; ++features )
  {
    mComposerAttributeTable->setMaximumNumberOfFeatures( features );
  }

  //and then the reverse....
  for ( int features = 50; features > 1; --features )
  {
    mComposerAttributeTable->setMaximumNumberOfFeatures( features );
  }
}

void TestQgsComposerTableV2::attributeTableAtlasSource()
{
  QgsComposerAttributeTableV2* table = new QgsComposerAttributeTableV2( mComposition, false );


  table->setSource( QgsComposerAttributeTableV2::AtlasFeature );

  //setup atlas
  mComposition->atlasComposition().setCoverageLayer( mVectorLayer2 );
  mComposition->atlasComposition().setEnabled( true );
  QVERIFY( mComposition->atlasComposition().beginRender() );

  QVERIFY( mComposition->atlasComposition().prepareForFeature( 0 ) );
  QCOMPARE( table->contents()->length(), 1 );
  QgsComposerTableRow row = table->contents()->at( 0 );

  //check a couple of results
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 90 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 2 ) );
  QCOMPARE( row.at( 4 ), QVariant( 0 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );

  //next atlas feature
  QVERIFY( mComposition->atlasComposition().prepareForFeature( 1 ) );
  QCOMPARE( table->contents()->length(), 1 );
  row = table->contents()->at( 0 );
  QCOMPARE( row.at( 0 ), QVariant( "Biplane" ) );
  QCOMPARE( row.at( 1 ), QVariant( 0 ) );
  QCOMPARE( row.at( 2 ), QVariant( 1 ) );
  QCOMPARE( row.at( 3 ), QVariant( 3 ) );
  QCOMPARE( row.at( 4 ), QVariant( 3 ) );
  QCOMPARE( row.at( 5 ), QVariant( 6 ) );

  //next atlas feature
  QVERIFY( mComposition->atlasComposition().prepareForFeature( 2 ) );
  QCOMPARE( table->contents()->length(), 1 );
  row = table->contents()->at( 0 );
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 85 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 1 ) );
  QCOMPARE( row.at( 4 ), QVariant( 1 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );

  mComposition->atlasComposition().endRender();

  //try for a crash when removing current atlas layer
  QgsMapLayerRegistry::instance()->removeMapLayer( mVectorLayer2->id() );
  table->refreshAttributes();

  mComposition->removeMultiFrame( table );
  delete table;
}


void TestQgsComposerTableV2::attributeTableRelationSource()
{
  QFileInfo vectorFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "points_relations.shp" );
  QgsVectorLayer* atlasLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      "ogr" );

  QgsMapLayerRegistry::instance()->addMapLayer( atlasLayer );

  //setup atlas
  mComposition->atlasComposition().setCoverageLayer( atlasLayer );
  mComposition->atlasComposition().setEnabled( true );

  //create a relation
  QgsRelation relation;
  relation.setRelationId( "testrelation" );
  relation.setReferencedLayer( atlasLayer->id() );
  relation.setReferencingLayer( mVectorLayer->id() );
  relation.addFieldPair( "Class", "Class" );
  QgsProject::instance()->relationManager()->addRelation( relation );

  QgsComposerAttributeTableV2* table = new QgsComposerAttributeTableV2( mComposition, false );
  table->setMaximumNumberOfFeatures( 50 );
  table->setSource( QgsComposerAttributeTableV2::RelationChildren );
  table->setRelationId( relation.id() );

  QVERIFY( mComposition->atlasComposition().beginRender() );
  QVERIFY( mComposition->atlasComposition().prepareForFeature( 0 ) );

  QCOMPARE( mComposition->atlasComposition().currentFeature()->attribute( "Class" ).toString(), QString( "Jet" ) );
  QCOMPARE( table->contents()->length(), 8 );

  QgsComposerTableRow row = table->contents()->at( 0 );

  //check a couple of results
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 90 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 2 ) );
  QCOMPARE( row.at( 4 ), QVariant( 0 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );
  row = table->contents()->at( 1 );
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 85 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 1 ) );
  QCOMPARE( row.at( 4 ), QVariant( 1 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );
  row = table->contents()->at( 2 );
  QCOMPARE( row.at( 0 ), QVariant( "Jet" ) );
  QCOMPARE( row.at( 1 ), QVariant( 95 ) );
  QCOMPARE( row.at( 2 ), QVariant( 3 ) );
  QCOMPARE( row.at( 3 ), QVariant( 1 ) );
  QCOMPARE( row.at( 4 ), QVariant( 1 ) );
  QCOMPARE( row.at( 5 ), QVariant( 2 ) );

  //next atlas feature
  QVERIFY( mComposition->atlasComposition().prepareForFeature( 1 ) );
  QCOMPARE( mComposition->atlasComposition().currentFeature()->attribute( "Class" ).toString(), QString( "Biplane" ) );
  QCOMPARE( table->contents()->length(), 5 );
  row = table->contents()->at( 0 );
  QCOMPARE( row.at( 0 ), QVariant( "Biplane" ) );
  QCOMPARE( row.at( 1 ), QVariant( 0 ) );
  QCOMPARE( row.at( 2 ), QVariant( 1 ) );
  QCOMPARE( row.at( 3 ), QVariant( 3 ) );
  QCOMPARE( row.at( 4 ), QVariant( 3 ) );
  QCOMPARE( row.at( 5 ), QVariant( 6 ) );
  row = table->contents()->at( 1 );
  QCOMPARE( row.at( 0 ), QVariant( "Biplane" ) );
  QCOMPARE( row.at( 1 ), QVariant( 340 ) );
  QCOMPARE( row.at( 2 ), QVariant( 1 ) );
  QCOMPARE( row.at( 3 ), QVariant( 3 ) );
  QCOMPARE( row.at( 4 ), QVariant( 3 ) );
  QCOMPARE( row.at( 5 ), QVariant( 6 ) );

  mComposition->atlasComposition().endRender();

  //try for a crash when removing current atlas layer
  QgsMapLayerRegistry::instance()->removeMapLayer( atlasLayer->id() );

  table->refreshAttributes();

  mComposition->removeMultiFrame( table );
  delete table;
}

void TestQgsComposerTableV2::contentsContainsRow()
{
  QgsComposerTableContents testContents;
  QgsComposerTableRow row1;
  row1 << QVariant( QString( "string 1" ) ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( QString( "string 2" ) );
  QgsComposerTableRow row2;
  row2 << QVariant( QString( "string 2" ) ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( QString( "string 2" ) );
  //same as row1
  QgsComposerTableRow row3;
  row3 << QVariant( QString( "string 1" ) ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( QString( "string 2" ) );
  QgsComposerTableRow row4;
  row4 << QVariant( QString( "string 1" ) ) << QVariant( 2 ) << QVariant( 1.7 ) << QVariant( QString( "string 2" ) );

  testContents << row1;
  testContents << row2;

  QVERIFY( mComposerAttributeTable->contentsContainsRow( testContents, row1 ) );
  QVERIFY( mComposerAttributeTable->contentsContainsRow( testContents, row2 ) );
  QVERIFY( mComposerAttributeTable->contentsContainsRow( testContents, row3 ) );
  QVERIFY( !mComposerAttributeTable->contentsContainsRow( testContents, row4 ) );
}

void TestQgsComposerTableV2::removeDuplicates()
{
  QgsVectorLayer* dupesLayer = new QgsVectorLayer( "Point?field=col1:integer&field=col2:integer&field=col3:integer", "dupes", "memory" );
  QVERIFY( dupesLayer->isValid() );
  QgsFeature f1( dupesLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( "col1", 1 );
  f1.setAttribute( "col2", 1 );
  f1.setAttribute( "col3", 1 );
  QgsFeature f2( dupesLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( "col1", 1 );
  f2.setAttribute( "col2", 2 );
  f2.setAttribute( "col3", 2 );
  QgsFeature f3( dupesLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( "col1", 1 );
  f3.setAttribute( "col2", 2 );
  f3.setAttribute( "col3", 3 );
  QgsFeature f4( dupesLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( "col1", 1 );
  f4.setAttribute( "col2", 1 );
  f4.setAttribute( "col3", 1 );
  dupesLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  QgsComposerAttributeTableV2* table = new QgsComposerAttributeTableV2( mComposition, false );
  table->setSource( QgsComposerAttributeTableV2::LayerAttributes );
  table->setVectorLayer( dupesLayer );
  table->setMaximumNumberOfFeatures( 50 );
  QCOMPARE( table->contents()->length(), 4 );

  table->setUniqueRowsOnly( true );
  QCOMPARE( table->contents()->length(), 3 );

  //check if removing attributes in unique mode works correctly (should result in duplicate rows,
  //which will be stripped out)
  table->columns()->removeLast();
  table->refreshAttributes();
  QCOMPARE( table->contents()->length(), 2 );
  table->columns()->removeLast();
  table->refreshAttributes();
  QCOMPARE( table->contents()->length(), 1 );
  table->setUniqueRowsOnly( false );
  QCOMPARE( table->contents()->length(), 4 );

  mComposition->removeMultiFrame( table );
  delete dupesLayer;
}

QTEST_MAIN( TestQgsComposerTableV2 )
#include "testqgscomposertablev2.moc"

