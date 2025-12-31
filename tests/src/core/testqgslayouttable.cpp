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
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionutils.h"
#include "qgsfeature.h"
#include "qgsfontutils.h"
#include "qgslayout.h"
#include "qgslayoutatlas.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmanager.h"
#include "qgslayoutreportcontext.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoututils.h"
#include "qgspallabeling.h"
#include "qgsprintlayout.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsrelationmanager.h"
#include "qgstest.h"
#include "qgstextrenderer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QObject>

class TestQgsLayoutTable : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutTable()
      : QgsTest( u"Layout Table Tests"_s, u"composer_table"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.

    void attributeTableHeadings();       //test retrieving attribute table headers
    void attributeTableRows();           //test retrieving attribute table rows
    void attributeTableFormattedRows();  //test retrieving attribute formatted table rows
    void attributeTableRowsLocalized();  //test retrieving attribute table rows with locale
    void attributeTableFilterFeatures(); //test filtering attribute table rows
    void attributeTableSetAttributes();  //test subset of attributes in table
    void attributeTableVisibleOnly();    //test displaying only visible attributes
    void attributeTableInsideAtlasOnly();
    void attributeTableRender(); //test rendering attribute table
    void manualColumnWidth();    //test setting manual column widths
    void attributeTableEmpty();  //test empty modes for attribute table
    void showEmptyRows();        //test showing empty rows
    void attributeTableExtend();
    void attributeTableRepeat();
    void attributeTableAtlasSource(); //test attribute table in atlas feature mode
    void attributeTableRestoreAtlasSource();
    void attributeTableRelationSource();            //test attribute table in relation mode
    void contentsContainsRow();                     //test the contentsContainsRow function
    void removeDuplicates();                        //test removing duplicate rows
    void multiLineText();                           //test rendering a table with multiline text
    void horizontalGrid();                          //test rendering a table with horizontal-only grid
    void verticalGrid();                            //test rendering a table with vertical-only grid
    void align();                                   //test alignment of table cells
    void wrapChar();                                //test setting wrap character
    void autoWrap();                                //test auto word wrap
    void cellStyles();                              //test cell styles
    void cellStylesRender();                        //test rendering cell styles
    void conditionalFormatting();                   //test rendering with conditional formatting
    void conditionalFormattingWithTextFormatting(); //test rendering with conditional formatting with text formatting
    void dataDefinedSource();
    void wrappedText();
    void testBaseSort();
    void testExpressionSort();
    void testScopeForCell();
    void testDataDefinedTextFormatForCell();
    void testIntegerNullCell();

  private:
    QgsVectorLayer *mVectorLayer = nullptr;

    //compares rows in table to expected rows
    void compareTable( QgsLayoutItemAttributeTable *table, const QVector<QStringList> &expectedRows, bool expectedResult = true );
};

void TestQgsLayoutTable::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  const QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), u"ogr"_s );
  QgsProject::instance()->addMapLayer( mVectorLayer );

  QgsFontUtils::loadStandardTestFonts( QStringList() << u"Bold"_s );
}

void TestQgsLayoutTable::cleanupTestCase()
{
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );
}

void TestQgsLayoutTable::attributeTableHeadings()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  //test retrieving attribute table headers
  QStringList expectedHeaders;
  expectedHeaders << u"Class"_s << u"Heading"_s << u"Importance"_s << u"Pilots"_s << u"Cabin Crew"_s << u"Staff"_s;

  //get header labels and compare
  const QMap<int, QString> headerMap = table->headerLabels();
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

void TestQgsLayoutTable::compareTable( QgsLayoutItemAttributeTable *table, const QVector<QStringList> &expectedRows, bool expectedResult )
{
  //retrieve rows and check
  QgsLayoutTableContents tableContents;
  const bool result = table->getTableContents( tableContents );
  QCOMPARE( result, expectedResult );

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
      QCOMPARE( QgsExpressionUtils::toLocalizedString( *cellIt ), expectedRows.at( rowNumber ).at( colNumber ) );
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
  row << u"Jet"_s << u"90"_s << u"3"_s << u"2"_s << u"0"_s << u"2"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"0"_s << u"1"_s << u"3"_s << u"3"_s << u"6"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Jet"_s << u"85"_s << u"3"_s << u"1"_s << u"1"_s << u"2"_s;
  expectedRows.append( row );

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  //retrieve rows and check
  table->setMaximumNumberOfFeatures( 3 );
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::attributeTableFormattedRows()
{
  QgsVectorLayer vl { u"Point?field=int:int"_s, u"test"_s, u"memory"_s };
  QVariantList valueConfig;
  QVariantMap config;
  config[u"one"_s] = u"1"_s;
  config[u"two"_s] = u"2"_s;
  valueConfig.append( config );
  QVariantMap editorConfig;
  editorConfig.insert( u"map"_s, valueConfig );
  vl.setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"ValueMap"_s, editorConfig ) );
  QgsFeature f { vl.fields() };
  f.setGeometry( QgsGeometry::fromWkt( u"point(9 45)"_s ) );
  f.setAttribute( u"int"_s, 2 );
  QgsFeature f2 { vl.fields() };
  f2.setGeometry( QgsGeometry::fromWkt( u"point(10 46)"_s ) );
  f2.setAttribute( u"int"_s, 1 );
  vl.dataProvider()->addFeatures( QgsFeatureList() << f << f2 );

  QVector<QStringList> expectedRows;
  QStringList row;
  row << u"two"_s;
  expectedRows.append( row );
  QStringList row2;
  row2 << u"one"_s;
  expectedRows.append( row2 );

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( &vl );

  //retrieve rows and check
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::attributeTableRowsLocalized()
{
  //test retrieving attribute table rows

  QgsVectorLayer vl { u"Point?field=int:int&field=double:double&"_s, u"test"_s, u"memory"_s };
  QgsFeature f { vl.fields() };
  f.setGeometry( QgsGeometry::fromWkt( u"point(9 45)"_s ) );
  f.setAttribute( u"int"_s, 12346 );
  f.setAttribute( u"double"_s, 123456.801 );
  vl.dataProvider()->addFeatures( QgsFeatureList() << f );

  QVector<QStringList> expectedRows;
  QStringList row;
  row << u"12,346"_s << u"123,456.801"_s;
  expectedRows.append( row );

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( &vl );

  //retrieve rows and check
  QLocale::setDefault( QLocale::English );
  compareTable( table, expectedRows );

  expectedRows.clear();
  row.clear();
  row << u"12.346"_s << u"123.456,801"_s;
  expectedRows.append( row );
  QLocale::setDefault( QLocale::Italian );
  compareTable( table, expectedRows );

  QLocale::setDefault( QLocale::English );
}


void TestQgsLayoutTable::attributeTableFilterFeatures()
{
  //test filtering attribute table rows
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  table->setMaximumNumberOfFeatures( 10 );
  table->setFeatureFilter( u"\"Class\"='B52'"_s );
  table->setFilterFeatures( true );

  QVector<QStringList> expectedRows;
  QStringList row;
  row << u"B52"_s << u"0"_s << u"10"_s << u"2"_s << u"1"_s << u"3"_s;
  expectedRows.append( row );
  row.clear();
  row << u"B52"_s << u"12"_s << u"10"_s << u"1"_s << u"1"_s << u"2"_s;
  expectedRows.append( row );
  row.clear();
  row << u"B52"_s << u"34"_s << u"10"_s << u"2"_s << u"1"_s << u"3"_s;
  expectedRows.append( row );
  row.clear();
  row << u"B52"_s << u"80"_s << u"10"_s << u"2"_s << u"1"_s << u"3"_s;
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( table, expectedRows );

  table->setFeatureFilter( u"\"Class\"=@airplane_class"_s );
  table->setFilterFeatures( true );
  expectedRows.clear();
  compareTable( table, expectedRows );

  QgsExpressionContextUtils::setLayoutVariable( &l, u"airplane_class"_s, u"Biplane"_s );

  row.clear();
  row << u"Biplane"_s << u"0"_s << u"1"_s << u"3"_s << u"3"_s << u"6"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"340"_s << u"1"_s << u"3"_s << u"3"_s << u"6"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"300"_s << u"1"_s << u"3"_s << u"2"_s << u"5"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"270"_s << u"1"_s << u"3"_s << u"4"_s << u"7"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"240"_s << u"1"_s << u"3"_s << u"2"_s << u"5"_s;
  expectedRows.append( row );
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
  attributes << u"Class"_s << u"Pilots"_s << u"Cabin Crew"_s;
  table->setDisplayedFields( attributes );
  table->setMaximumNumberOfFeatures( 3 );

  //check headers
  QStringList expectedHeaders;
  expectedHeaders << u"Class"_s << u"Pilots"_s << u"Cabin Crew"_s;

  //get header labels and compare
  const QMap<int, QString> headerMap = table->headerLabels();
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
  row << u"Jet"_s << u"2"_s << u"0"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"3"_s << u"3"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Jet"_s << u"1"_s << u"1"_s;
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
  row << u"Jet"_s << u"90"_s << u"3"_s << u"2"_s << u"0"_s << u"2"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"240"_s << u"1"_s << u"3"_s << u"2"_s << u"5"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Jet"_s << u"180"_s << u"3"_s << u"1"_s << u"0"_s << u"1"_s;
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( table, expectedRows );

  // with rotation
  map->setMapRotation( 90 );
  expectedRows.clear();
  row.clear();
  row << u"Jet"_s << u"90"_s << u"3"_s << u"2"_s << u"0"_s << u"2"_s;
  expectedRows.append( row );
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::attributeTableInsideAtlasOnly()
{
  //test displaying only visible attributes inside the atlas feature
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setExtent( QgsRectangle( -95.537, 32.736, -84.389, 42.2920 ) );
  l.addLayoutItem( map );

  table->setMap( map );
  table->setFilterToAtlasFeature( true );

  // no atlas feature
  QVector<QStringList> expectedRows;
  compareTable( table, expectedRows, false );

  //setup atlas
  auto atlasLayer = std::make_unique<QgsVectorLayer>( u"Polygon?crs=EPSG:3857"_s, u"atlas"_s, u"memory"_s );
  QVERIFY( atlasLayer->isValid() );
  const QgsGeometry atlasGeom( QgsGeometry::fromWkt( u"Polygon ((-8863916.31126776337623596 4621257.48816855065524578, -9664269.45078738406300545 5097056.938785120844841, -10049249.44194872118532658 3765399.75924854446202517, -8985488.94005555473268032 3458599.17133777122944593, -8863916.31126776337623596 4621257.48816855065524578))"_s ) );
  QgsFeature f;
  f.setGeometry( atlasGeom );
  atlasLayer->dataProvider()->addFeature( f );
  l.reportContext().setLayer( atlasLayer.get() );

  QgsFeatureIterator it = atlasLayer->getFeatures();
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  QStringList row;
  row << u"Biplane"_s << u"0"_s << u"1"_s << u"3"_s << u"3"_s << u"6"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Jet"_s << u"90"_s << u"3"_s << u"1"_s << u"0"_s << u"1"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"340"_s << u"1"_s << u"3"_s << u"3"_s << u"6"_s;
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( table, expectedRows );

  // combination of atlas and map extent visibility
  table->setDisplayOnlyVisibleFeatures( true );
  expectedRows.clear();
  row.clear();
  row << u"Jet"_s << u"90"_s << u"3"_s << u"1"_s << u"0"_s << u"1"_s;
  expectedRows.append( row );
  row.clear();
  row << u"Biplane"_s << u"340"_s << u"1"_s << u"3"_s << u"3"_s << u"6"_s;
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 20 );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_render"_s, &l );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 20 );
  table->columns()[0].setWidth( 5 );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_columnwidth"_s, &l );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 20 );
  //hide all features from table
  table->setFeatureFilter( u"1=2"_s );
  table->setFilterFeatures( true );

  table->setEmptyTableBehavior( QgsLayoutTable::HeadersOnly );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_headersonly"_s, &l );

  table->setEmptyTableBehavior( QgsLayoutTable::HideTable );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_hidetable"_s, &l );

  table->setEmptyTableBehavior( QgsLayoutTable::ShowMessage );
  table->setEmptyTableMessage( u"no rows"_s );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_showmessage"_s, &l );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 3 );
  table->setShowEmptyRows( true );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_drawempty"_s, &l );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
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
  const QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  vectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), u"ogr"_s );
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

void TestQgsLayoutTable::attributeTableRestoreAtlasSource()
{
  const QString projectPath = QStringLiteral( TEST_DATA_DIR ) + "/layout_atlas_table.qgs";
  QgsProject p;
  p.read( projectPath );

  QgsPrintLayout *l = dynamic_cast<QgsPrintLayout *>( p.layoutManager()->layouts().at( 0 ) );
  QgsLayoutItemAttributeTable *table = qobject_cast<QgsLayoutItemAttributeTable *>( l->multiFrames().at( 0 ) );
  QCOMPARE( table->source(), QgsLayoutItemAttributeTable::AtlasFeature );
  QVERIFY( l->atlas()->coverageLayer() );
  QVERIFY( l->atlas()->coverageLayer()->isValid() );
  QCOMPARE( table->sourceLayer(), l->atlas()->coverageLayer() );
  QCOMPARE( table->columns().count(), 2 );
  QCOMPARE( table->columns()[0].attribute(), u"Heading"_s );
  QCOMPARE( table->columns()[1].attribute(), u"Staff"_s );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  const QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points_relations.shp" );
  QgsVectorLayer *atlasLayer = new QgsVectorLayer( vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), u"ogr"_s );

  QgsProject::instance()->addMapLayer( atlasLayer );

  //setup atlas
  l.reportContext().setLayer( atlasLayer );

  QgsFeature f;
  QgsFeatureIterator it = atlasLayer->getFeatures();
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  //create a relation
  QgsRelation relation;
  relation.setId( u"testrelation"_s );
  relation.setReferencedLayer( atlasLayer->id() );
  relation.setReferencingLayer( mVectorLayer->id() );
  relation.addFieldPair( u"Class"_s, u"Class"_s );
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
  row1 << QVariant( u"string 1"_s ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( u"string 2"_s );
  QgsLayoutTableRow row2;
  row2 << QVariant( u"string 2"_s ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( u"string 2"_s );
  //same as row1
  QgsLayoutTableRow row3;
  row3 << QVariant( u"string 1"_s ) << QVariant( 2 ) << QVariant( 1.5 ) << QVariant( u"string 2"_s );
  QgsLayoutTableRow row4;
  row4 << QVariant( u"string 1"_s ) << QVariant( 2 ) << QVariant( 1.7 ) << QVariant( u"string 2"_s );

  testContents << row1;
  testContents << row2;

  QVERIFY( table->contentsContainsRow( testContents, row1 ) );
  QVERIFY( table->contentsContainsRow( testContents, row2 ) );
  QVERIFY( table->contentsContainsRow( testContents, row3 ) );
  QVERIFY( !table->contentsContainsRow( testContents, row4 ) );
}

void TestQgsLayoutTable::removeDuplicates()
{
  QgsVectorLayer *dupesLayer = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:integer&field=col3:integer"_s, u"dupes"_s, u"memory"_s );
  QVERIFY( dupesLayer->isValid() );
  QgsFeature f1( dupesLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, 1 );
  f1.setAttribute( u"col2"_s, 1 );
  f1.setAttribute( u"col3"_s, 1 );
  QgsFeature f2( dupesLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"col1"_s, 1 );
  f2.setAttribute( u"col2"_s, 2 );
  f2.setAttribute( u"col3"_s, 2 );
  QgsFeature f3( dupesLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"col1"_s, 1 );
  f3.setAttribute( u"col2"_s, 2 );
  f3.setAttribute( u"col3"_s, 3 );
  QgsFeature f4( dupesLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( u"col1"_s, 1 );
  f4.setAttribute( u"col2"_s, 1 );
  f4.setAttribute( u"col3"_s, 1 );
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
  table->columns().takeLast();
  table->refreshAttributes();
  QCOMPARE( table->contents().length(), 2 );
  table->columns().takeLast();
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( u"Point?field=col1:string&field=col2:string&field=col3:string"_s, u"multiline"_s, u"memory"_s );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, "multiline\nstring" );
  f1.setAttribute( u"col2"_s, "singleline string" );
  f1.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"col1"_s, "singleline string" );
  f2.setAttribute( u"col2"_s, "multiline\nstring" );
  f2.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"col1"_s, "singleline" );
  f3.setAttribute( u"col2"_s, "singleline" );
  f3.setAttribute( u"col3"_s, "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( u"col1"_s, "long triple\nline\nstring" );
  f4.setAttribute( u"col2"_s, "double\nlinestring" );
  f4.setAttribute( u"col3"_s, "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setVectorLayer( multiLineLayer );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_multiline"_s, &l );

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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( u"Point?field=col1:string&field=col2:string&field=col3:string"_s, u"multiline"_s, u"memory"_s );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, "multiline\nstring" );
  f1.setAttribute( u"col2"_s, "singleline string" );
  f1.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"col1"_s, "singleline string" );
  f2.setAttribute( u"col2"_s, "multiline\nstring" );
  f2.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"col1"_s, "singleline" );
  f3.setAttribute( u"col2"_s, "singleline" );
  f3.setAttribute( u"col3"_s, "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( u"col1"_s, "long triple\nline\nstring" );
  f4.setAttribute( u"col2"_s, "double\nlinestring" );
  f4.setAttribute( u"col3"_s, "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame1->setFrameEnabled( false );
  frame2->setFrameEnabled( false );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( false );
  table->setVectorLayer( multiLineLayer );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_horizontalgrid"_s, &l );

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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( u"Point?field=col1:string&field=col2:string&field=col3:string"_s, u"multiline"_s, u"memory"_s );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, "multiline\nstring" );
  f1.setAttribute( u"col2"_s, "singleline string" );
  f1.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"col1"_s, "singleline string" );
  f2.setAttribute( u"col2"_s, "multiline\nstring" );
  f2.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"col1"_s, "singleline" );
  f3.setAttribute( u"col2"_s, "singleline" );
  f3.setAttribute( u"col3"_s, "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( u"col1"_s, "long triple\nline\nstring" );
  f4.setAttribute( u"col2"_s, "double\nlinestring" );
  f4.setAttribute( u"col3"_s, "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame1->setFrameEnabled( false );
  frame2->setFrameEnabled( false );
  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setShowGrid( true );
  table->setHorizontalGrid( false );
  table->setVerticalGrid( true );
  table->setVectorLayer( multiLineLayer );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_verticalgrid"_s, &l );

  delete multiLineLayer;
}

void TestQgsLayoutTable::testDataDefinedTextFormatForCell()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );

  l.addMultiFrame( table );
  QgsLayoutFrame *frame = new QgsLayoutFrame( &l, table );
  frame->attemptSetSceneRect( QRectF( 5, 5, 150, 30 ) );
  frame->setFrameEnabled( true );
  l.addLayoutItem( frame );
  table->addFrame( frame );

  QgsTextFormat textFormat = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) );
  table->setHeaderTextFormat( textFormat );

  textFormat.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Size, QgsProperty::fromExpression( u"if(@column_number = 1,35,15)"_s ) );
  table->setContentTextFormat( textFormat );

  QGSVERIFYLAYOUTCHECK( u"composerattributetable_datadefinedtextformat"_s, &l );
}

void TestQgsLayoutTable::testIntegerNullCell()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );

  l.addMultiFrame( table );
  QgsLayoutFrame *frame = new QgsLayoutFrame( &l, table );
  frame->attemptSetSceneRect( QRectF( 5, 5, 150, 30 ) );
  frame->setFrameEnabled( true );
  l.addLayoutItem( frame );
  table->addFrame( frame );

  auto layer = std::make_unique<QgsVectorLayer>( u"Point?field=intf:integer"_s, u"point"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  QgsFeature f1( layer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"intf"_s, 1 );
  QgsFeature f2( layer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"intf"_s, 2 );
  QgsFeature f3( layer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"intf"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );
  layer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 );

  table->setVectorLayer( layer.get() );
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  QGSVERIFYLAYOUTCHECK( u"composerattributetable_integernullcell"_s, &l );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  QgsVectorLayer *multiLineLayer = new QgsVectorLayer( u"Point?field=col1:string&field=col2:string&field=col3:string"_s, u"multiline"_s, u"memory"_s );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, "multiline\nstring" );
  f1.setAttribute( u"col2"_s, "singleline string" );
  f1.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"col1"_s, "singleline string" );
  f2.setAttribute( u"col2"_s, "multiline\nstring" );
  f2.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"col1"_s, "singleline" );
  f3.setAttribute( u"col2"_s, "singleline" );
  f3.setAttribute( u"col3"_s, "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( u"col1"_s, "long triple\nline\nstring" );
  f4.setAttribute( u"col2"_s, "double\nlinestring" );
  f4.setAttribute( u"col3"_s, "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setVectorLayer( multiLineLayer );

  table->columns()[0].setHAlignment( Qt::AlignLeft );
  table->columns()[0].setVAlignment( Qt::AlignTop );
  table->columns()[1].setHAlignment( Qt::AlignHCenter );
  table->columns()[1].setVAlignment( Qt::AlignVCenter );
  table->columns()[2].setHAlignment( Qt::AlignRight );
  table->columns()[2].setVAlignment( Qt::AlignBottom );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_align"_s, &l );

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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  auto multiLineLayer = std::make_unique<QgsVectorLayer>( u"Point?field=col1:string&field=col2:string&field=col3:string"_s, u"multiline"_s, u"memory"_s );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, "multiline\nstring" );
  f1.setAttribute( u"col2"_s, "singleline string" );
  f1.setAttribute( u"col3"_s, "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  table->setMaximumNumberOfFeatures( 1 );
  table->setVectorLayer( multiLineLayer.get() );
  table->setWrapString( u"in"_s );

  QVector<QStringList> expectedRows;
  QStringList row;
  row << u"multil\ne\nstr\ng"_s << u"s\nglel\ne str\ng"_s << u"s\nglel\ne"_s;
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  auto multiLineLayer = std::make_unique<QgsVectorLayer>( u"Point?field=col1:string&field=col2:string&field=col3:string"_s, u"multiline"_s, u"memory"_s );
  QVERIFY( multiLineLayer->isValid() );
  QgsFeature f1( multiLineLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, "long multiline\nstring" );
  f1.setAttribute( u"col2"_s, "singleline string" );
  f1.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f2( multiLineLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"col1"_s, "singleline string" );
  f2.setAttribute( u"col2"_s, "multiline\nstring" );
  f2.setAttribute( u"col3"_s, "singleline" );
  QgsFeature f3( multiLineLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"col1"_s, "singleline" );
  f3.setAttribute( u"col2"_s, "singleline" );
  f3.setAttribute( u"col3"_s, "multiline\nstring" );
  QgsFeature f4( multiLineLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( u"col1"_s, "a bit long triple line string" );
  f4.setAttribute( u"col2"_s, "double toolongtofitononeline string with some more lines on the end andanotherreallylongline" );
  f4.setAttribute( u"col3"_s, "singleline" );
  multiLineLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  frame2->attemptSetSceneRect( QRectF( 5, 40, 100, 90 ) );

  table->setMaximumNumberOfFeatures( 20 );
  table->setVectorLayer( multiLineLayer.get() );
  table->setWrapBehavior( QgsLayoutTable::WrapText );

  table->columns()[0].setWidth( 25 );
  table->columns()[1].setWidth( 25 );
  QGSVERIFYLAYOUTCHECK( u"composerattributetable_autowrap"_s, &l );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  QgsLayoutTableStyle original;
  original.enabled = true;
  original.cellBackgroundColor = QColor( 200, 100, 150, 90 );

  //write to xml
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s
  );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement node = doc.createElement( u"style"_s );
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
  const QgsLayoutTableStyle style2;
  style1.enabled = false;
  style1.cellBackgroundColor = QColor( 60, 62, 64, 68 );
  originalTable->setCellStyle( QgsLayoutTable::LastColumn, style2 );

  //write to XML
  QDomElement tableElement = doc.createElement( u"table"_s );
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
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

  QGSVERIFYLAYOUTCHECK( u"composerattributetable_cellstyle"_s, &l, 0, 0, QSize(), 10 );
}

void TestQgsLayoutTable::conditionalFormatting()
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 7 );
  table->setShowEmptyRows( true );


  QgsConditionalStyles rowStyles;
  QgsConditionalStyle style1;
  style1.setRule( u"\"Heading\" >= 300"_s );
  style1.setTextColor( QColor( 255, 255, 255 ) );
  style1.setBackgroundColor( QColor( 0, 0, 0 ) );
  rowStyles.append( style1 );
  mVectorLayer->conditionalStyles()->setRowStyles( rowStyles );
  QgsConditionalStyle style2;
  style2.setRule( u"@value > 5"_s );
  style2.setTextColor( QColor( 255, 0, 0 ) );
  style2.setBackgroundColor( QColor( 0, 0, 255 ) );
  mVectorLayer->conditionalStyles()->setFieldStyles( u"Staff"_s, QList<QgsConditionalStyle>() << style2 );

  table->setUseConditionalStyling( true );

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

  QGSVERIFYLAYOUTCHECK( u"composerattributetable_conditionalstyles"_s, &l, 0, 0, QSize(), 10 );
}

void TestQgsLayoutTable::conditionalFormattingWithTextFormatting()
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
  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setHeaderTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  table->setBackgroundColor( Qt::yellow );

  table->setMaximumNumberOfFeatures( 7 );
  table->setShowEmptyRows( true );


  QgsConditionalStyles rowStyles;
  QgsConditionalStyle style1;
  style1.setRule( u"\"Heading\" >= 300"_s );
  style1.setTextColor( QColor( 255, 255, 255 ) );
  style1.setBackgroundColor( QColor( 0, 0, 0 ) );

  QFont conditionalFont1 = QgsFontUtils::getStandardTestFont( u"Bold Oblique"_s );
  conditionalFont1.setStrikeOut( true );
  conditionalFont1.setUnderline( true );
  style1.setFont( conditionalFont1 );
  rowStyles.append( style1 );
  mVectorLayer->conditionalStyles()->setRowStyles( rowStyles );
  QgsConditionalStyle style2;
  style2.setRule( u"@value > 5"_s );
  style2.setTextColor( QColor( 255, 0, 0 ) );
  style2.setBackgroundColor( QColor( 0, 0, 255 ) );
  QFont conditionalFont2 = QgsFontUtils::getStandardTestFont( u"Bold"_s );
  conditionalFont2.setUnderline( true );
  style2.setFont( conditionalFont2 );
  mVectorLayer->conditionalStyles()->setFieldStyles( u"Staff"_s, QList<QgsConditionalStyle>() << style2 );

  table->setUseConditionalStyling( true );

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

  QGSVERIFYLAYOUTCHECK( u"composerattributetable_conditionalstyles_text"_s, &l, 0, 0, QSize(), 10 );
}

void TestQgsLayoutTable::dataDefinedSource()
{
  // add a couple of layers
  QgsVectorLayer *layer1 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:integer&field=col3:integer"_s, u"l1"_s, u"memory"_s );
  QVERIFY( layer1->isValid() );
  QgsFeature f( layer1->fields() );
  f.setAttributes( QgsAttributes() << 1 << 2 << 3 );
  layer1->dataProvider()->addFeature( f );

  // different field order
  QgsVectorLayer *layer2 = new QgsVectorLayer( u"Point?field=col1:integer&field=col3:integer&field=col2:integer"_s, u"l2"_s, u"memory"_s );
  QVERIFY( layer2->isValid() );
  QgsFeature f2( layer2->fields() );
  f2.setAttributes( QgsAttributes() << 11 << 13 << 12 );
  layer2->dataProvider()->addFeature( f2 );

  // missing fields
  QgsVectorLayer *layer3 = new QgsVectorLayer( u"Point?field=col1:integer&field=col3:integer"_s, u"l3"_s, u"memory"_s );
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
  QCOMPARE( table->contents().at( 0 ), QVector<QVariant>() << 1 << 2 << 3 );

  // data defined table name, by layer id
  table->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::AttributeTableSourceLayer, layer1->id() );
  table->refresh();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector<QVariant>() << 1 << 2 << 3 );

  // by layer name
  table->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::AttributeTableSourceLayer, u"l2"_s );
  table->refresh();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector<QVariant>() << 11 << 12 << 13 );

  // by layer name (case insensitive)
  table->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::AttributeTableSourceLayer, u"L3"_s );
  table->refresh();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector<QVariant>() << 21 << QVariant() << 23 );

  // delete current data defined layer match
  p.removeMapLayer( layer3->id() );
  QApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  // expect table to return to preset layer
  table->refreshAttributes();
  QCOMPARE( table->contents().length(), 1 );
  QCOMPARE( table->contents().at( 0 ), QVector<QVariant>() << 1 << 2 << 3 );
}

void TestQgsLayoutTable::wrappedText()
{
  QgsProject p;
  QgsLayout l( &p );

  const QFont f;
  const QString sourceText( "Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna aliqua" );
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  const QString wrapText = QgsTextRenderer::wrappedText( context, sourceText, context.convertToPainterUnits( 101, Qgis::RenderUnit::Millimeters ) /*columnWidth*/, QgsTextFormat::fromQFont( f ) ).join( '\n' );
  //there should be no line break before the last word (bug #20546)
  QVERIFY( !wrapText.endsWith( "\naliqua" ) );
}

void TestQgsLayoutTable::testBaseSort()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 1 );
  QgsLayoutTableColumn col;
  col.setAttribute( table->columns()[2].attribute() );
  col.setSortOrder( Qt::DescendingOrder );
  table->sortColumns() = { col };
  table->refresh();

  QVector<QStringList> expectedRows;
  QStringList row;
  row << u"Jet"_s << u"100"_s << u"20"_s << u"3"_s << u"0"_s << u"3"_s;
  expectedRows.append( row );
  row.clear();

  //retrieve rows and check
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::testExpressionSort()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );
  table->setDisplayOnlyVisibleFeatures( false );
  table->setMaximumNumberOfFeatures( 1 );
  QgsLayoutTableColumn col;
  col.setAttribute( "Heading * -1" );
  col.setHeading( "exp" );
  col.setSortOrder( Qt::AscendingOrder );
  table->sortColumns() = { col };
  table->columns()[0] = col;
  table->refresh();

  QVector<QStringList> expectedRows;
  QStringList row;
  row << u"-340"_s << u"340"_s << u"1"_s << u"3"_s << u"3"_s << u"6"_s;
  expectedRows.append( row );
  row.clear();

  //retrieve rows and check
  compareTable( table, expectedRows );
}

void TestQgsLayoutTable::testScopeForCell()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemAttributeTable *table = new QgsLayoutItemAttributeTable( &l );
  table->setVectorLayer( mVectorLayer );
  table->refresh();

  std::unique_ptr<QgsExpressionContextScope> scope( table->scopeForCell( 0, 0 ) );

  // variable values for row/col should start at 1, not 0!
  QCOMPARE( scope->variable( u"row_number"_s ).toInt(), 1 );
  QCOMPARE( scope->variable( u"column_number"_s ).toInt(), 1 );
  QCOMPARE( scope->feature().attribute( 0 ).toString(), u"Jet"_s );
  scope.reset( table->scopeForCell( 0, 1 ) );
  QCOMPARE( scope->variable( u"row_number"_s ).toInt(), 1 );
  QCOMPARE( scope->variable( u"column_number"_s ).toInt(), 2 );
  QCOMPARE( scope->feature().attribute( 0 ).toString(), u"Jet"_s );
  scope.reset( table->scopeForCell( 1, 2 ) );
  QCOMPARE( scope->variable( u"row_number"_s ).toInt(), 2 );
  QCOMPARE( scope->variable( u"column_number"_s ).toInt(), 3 );
  QCOMPARE( scope->feature().attribute( 0 ).toString(), u"Biplane"_s );

  // make sure fields are set
  QgsExpressionContext context;
  context.appendScope( scope.release() );
  QCOMPARE( context.fields().size(), 6 );
  QCOMPARE( context.fields().at( 0 ).name(), u"Class"_s );
}

QGSTEST_MAIN( TestQgsLayoutTable )
#include "testqgslayouttable.moc"
