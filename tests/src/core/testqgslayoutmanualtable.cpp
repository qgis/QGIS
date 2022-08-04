/***************************************************************************
                         testqgslayoutmanualtable.cpp
                         ----------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
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
#include "qgslayoutitemmanualtable.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutframe.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsprintlayout.h"
#include "qgscurrencynumericformat.h"
#include "qgsmultirenderchecker.h"
#include "qgsfontutils.h"

#include <QObject>
#include "qgstest.h"

class TestQgsLayoutManualTable : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutManualTable() : QgsTest( QStringLiteral( "Layout Manual Table Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void setContents();
    void scopeForCell();
    void expressionContents();
    void cellStyles();
    void cellFormat();
    void rowHeight();
    void columnWidth();
    void headers();
    void cellTextFormat();
    void cellTextAlignment();

  private:

    //compares rows in table to expected rows
    void compareTable( QgsLayoutItemManualTable *table, const QVector<QStringList> &expectedRows );
};

void TestQgsLayoutManualTable::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

void TestQgsLayoutManualTable::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutManualTable::compareTable( QgsLayoutItemManualTable *table, const QVector<QStringList> &expectedRows )
{
  //retrieve rows and check
  QgsLayoutTableContents tableContents;
  const bool result = table->getTableContents( tableContents );
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

void TestQgsLayoutManualTable::setContents()
{
  QVector<QStringList> expectedRows;

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  // empty table
  compareTable( table, expectedRows );
  QVERIFY( table->tableContents().isEmpty() );

  // one empty row
  QStringList row;
  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << QgsTableRow() );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 1 );
  QVERIFY( table->tableContents().at( 0 ).isEmpty() );

  // 1x1
  row << QStringLiteral( "Jet" );
  expectedRows.clear();
  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );

  // 1 x 2
  row.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "Helicopter" );
  expectedRows.clear();
  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "Helicopter" ) );

  // 1 x 3
  row.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "Helicopter" ) << QStringLiteral( "Plane" );
  expectedRows.clear();
  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "Helicopter" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "Plane" ) );

  // unbalanced
  row.clear();
  expectedRows.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "Helicopter" ) << QStringLiteral( "Plane" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "A" ) << QString() << QString();

  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "Helicopter" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "Plane" ) );
  QCOMPARE( table->tableContents().at( 1 ).size(), 1 );
  QCOMPARE( table->tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "A" ) );

  // unbalanced
  row.clear();
  expectedRows.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "Helicopter" ) << QStringLiteral( "Plane" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "A" ) << QStringLiteral( "B" ) << QString();

  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) << QgsTableCell( QStringLiteral( "B" ) ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "Helicopter" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "Plane" ) );
  QCOMPARE( table->tableContents().at( 1 ).size(), 2 );
  QCOMPARE( table->tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "A" ) );
  QCOMPARE( table->tableContents().at( 1 ).at( 1 ).content().toString(), QStringLiteral( "B" ) );

  // 2 x 3
  row.clear();
  expectedRows.clear();
  row << QStringLiteral( "Jet" ) << QStringLiteral( "Helicopter" ) << QStringLiteral( "Plane" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "A" ) << QStringLiteral( "B" ) << QStringLiteral( "C" );

  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) << QgsTableCell( QStringLiteral( "B" ) ) << QgsTableCell( QStringLiteral( "C" ) ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "Helicopter" ) );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "Plane" ) );
  QCOMPARE( table->tableContents().at( 1 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "A" ) );
  QCOMPARE( table->tableContents().at( 1 ).at( 1 ).content().toString(), QStringLiteral( "B" ) );
  QCOMPARE( table->tableContents().at( 1 ).at( 2 ).content().toString(), QStringLiteral( "C" ) );

  table->setRowHeights( QList< double >() << 5.5 << 4.0 );
  table->setColumnWidths( QList< double >() << 15.5 << 14.0 << 13.4 );

  // save and restore

  //write to XML
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement tableElement = doc.createElement( QStringLiteral( "table" ) );
  QVERIFY( table->writeXml( tableElement, doc, QgsReadWriteContext(), true ) );

  //read from XML
  QgsLayoutItemManualTable *tableFromXml = new QgsLayoutItemManualTable( &l );
  QVERIFY( tableFromXml->readXml( tableElement.firstChildElement(), doc, QgsReadWriteContext(), true ) );

  QCOMPARE( tableFromXml->tableContents().size(), 2 );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "Helicopter" ) );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "Plane" ) );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).size(), 3 );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "A" ) );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).at( 1 ).content().toString(), QStringLiteral( "B" ) );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).at( 2 ).content().toString(), QStringLiteral( "C" ) );

  QCOMPARE( tableFromXml->rowHeights(), QList< double >() << 5.5 << 4.0 );
  QCOMPARE( tableFromXml->columnWidths(), QList< double >() << 15.5 << 14.0 << 13.4 );
}

void TestQgsLayoutManualTable::scopeForCell()
{
  QgsPrintLayout l( QgsProject::instance() );
  l.initializeDefaults();
  l.setName( QStringLiteral( "my layout" ) );
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  std::unique_ptr< QgsExpressionContextScope > scope( table->scopeForCell( 1, 2 ) );

  // variable values for row/col should start at 1, not 0!
  QCOMPARE( scope->variable( QStringLiteral( "row_number" ) ).toInt(), 2 );
  QCOMPARE( scope->variable( QStringLiteral( "column_number" ) ).toInt(), 3 );
}

void TestQgsLayoutManualTable::expressionContents()
{
  QVector<QStringList> expectedRows;

  QgsPrintLayout l( QgsProject::instance() );
  l.initializeDefaults();
  l.setName( QStringLiteral( "my layout" ) );
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  QStringList row;

  // 2 x 3
  row << QStringLiteral( "Jet" ) << QStringLiteral( "1,2" ) << QStringLiteral( "1,3" );
  expectedRows.append( row );
  row.clear();
  row << QStringLiteral( "my layout" ) << QStringLiteral( "Helicopter" ) << QStringLiteral( "Plane" );
  expectedRows.append( row );

  table->setTableContents(
    QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QgsProperty::fromExpression( QStringLiteral( "@row_number  || ',' || @column_number" ) ) ) << QgsTableCell( QgsProperty::fromExpression( QStringLiteral( "@row_number  || ',' || @column_number" ) ) ) )
    << ( QgsTableRow() << QgsTableCell( QgsProperty::fromExpression( QStringLiteral( "@layout_name" ) ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) ) );
  compareTable( table, expectedRows );

  // save and restore

  //write to XML
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement tableElement = doc.createElement( QStringLiteral( "table" ) );
  QVERIFY( table->writeXml( tableElement, doc, QgsReadWriteContext(), true ) );

  //read from XML
  QgsLayoutItemManualTable *tableFromXml = new QgsLayoutItemManualTable( &l );
  QVERIFY( tableFromXml->readXml( tableElement.firstChildElement(), doc, QgsReadWriteContext(), true ) );
  compareTable( tableFromXml, expectedRows );
}

void TestQgsLayoutManualTable::cellStyles()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  const QgsTableCell c11;
  QgsTableCell c12;
  c12.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTableCell c13;
  c13.setForegroundColor( QColor( 0, 255, 0 ) );
  const QgsTableCell c21;
  QgsTableCell c22;
  c22.setBackgroundColor( QColor( 255, 255, 0 ) );
  c22.setForegroundColor( QColor( 255, 0, 255 ) );
  const QgsTableCell c23;

  table->setBackgroundColor( QColor() );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << c11 << c12 << c13 )
                           << ( QgsTableRow() << c21 << c22 << c23 ) );
  QCOMPARE( table->backgroundColor( 0, 0 ), QColor() );
  QCOMPARE( table->backgroundColor( 0, 1 ), QColor( 255, 0, 0 ) );
  QCOMPARE( table->backgroundColor( 0, 2 ), QColor() );
  QCOMPARE( table->backgroundColor( 1, 0 ), QColor() );
  QCOMPARE( table->backgroundColor( 1, 1 ), QColor( 255, 255, 0 ) );
  QCOMPARE( table->backgroundColor( 1, 2 ), QColor() );
  QCOMPARE( table->conditionalCellStyle( 0, 0 ).textColor(), QColor() );
  QCOMPARE( table->conditionalCellStyle( 0, 1 ).textColor(), QColor() );
  QCOMPARE( table->conditionalCellStyle( 0, 2 ).textColor(), QColor( 0, 255, 0 ) );
  QCOMPARE( table->conditionalCellStyle( 1, 0 ).textColor(), QColor() );
  QCOMPARE( table->conditionalCellStyle( 1, 1 ).textColor(), QColor( 255, 0, 255 ) );
  QCOMPARE( table->conditionalCellStyle( 1, 2 ).textColor(), QColor() );
}

void TestQgsLayoutManualTable::cellFormat()
{
  QVector<QStringList> expectedRows;

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  // 1 x 3
  QStringList row;
  row << QStringLiteral( "Jet" ) << QStringLiteral( "76" ) << QStringLiteral( "$87.00" );
  expectedRows.clear();
  expectedRows.append( row );

  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( 76 ) << c3 ) );
  compareTable( table, expectedRows );
}

void TestQgsLayoutManualTable::rowHeight()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) << QgsTableCell( QStringLiteral( "B" ) ) << QgsTableCell( QStringLiteral( "C" ) ) ) );

  table->setRowHeights( QList< double >() << 0 << 40.0 );
  QgsLayoutChecker checker( QStringLiteral( "manualtable_rowheight" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_manual_table" ) );
  const bool result = checker.testLayout( mReport );
  QVERIFY( result );
}

void TestQgsLayoutManualTable::columnWidth()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) << QgsTableCell( QStringLiteral( "B" ) ) << QgsTableCell( QStringLiteral( "C" ) ) ) );

  table->setColumnWidths( QList< double >() << 0 << 10.0 << 30.0 );
  QgsLayoutChecker checker( QStringLiteral( "manualtable_columnwidth" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_manual_table" ) );
  const bool result = checker.testLayout( mReport );
  QVERIFY( result );
}

void TestQgsLayoutManualTable::headers()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  QgsTextFormat headerFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 16 ) ) );
  headerFormat.setColor( QColor( 255, 0, 255 ) );
  table->setHeaderTextFormat( headerFormat );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( QStringLiteral( "Helicopter" ) ) << QgsTableCell( QStringLiteral( "Plane" ) ) )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) << QgsTableCell( QStringLiteral( "B" ) ) << QgsTableCell( QStringLiteral( "C" ) ) ) );
  table->setIncludeTableHeader( true );
  table->setHeaders( QgsLayoutTableColumns() << QgsLayoutTableColumn( QStringLiteral( "header1" ) )
                     << QgsLayoutTableColumn( QStringLiteral( "h2" ) )
                     << QgsLayoutTableColumn( QStringLiteral( "header 3" ) ) );

  QgsLayoutChecker checker( QStringLiteral( "manualtable_headers" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_manual_table" ) );
  const bool result = checker.testLayout( mReport );
  QVERIFY( result );
}

void TestQgsLayoutManualTable::cellTextFormat()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  QgsTableCell c1( QStringLiteral( "Jet" ) );
  QgsTextFormat f1 = table->contentTextFormat();
  f1.setSize( 20 );
  f1.buffer().setEnabled( true );
  f1.buffer().setColor( QColor( 100, 130, 150 ) );
  f1.buffer().setSize( 1 );
  c1.setTextFormat( f1 );

  QgsTableCell c3( QStringLiteral( "Plane" ) );
  QgsTextFormat f2 = table->contentTextFormat();
  f2.setSize( 16 );
  f2.buffer().setEnabled( true );
  f2.buffer().setColor( QColor( 150, 110, 90 ) );
  f2.buffer().setSize( 1 );
  c3.setTextFormat( f2 );

  QgsTableCell c5( QStringLiteral( "B" ) );
  QgsTextFormat f3 = table->contentTextFormat();
  f3.setSize( 36 );
  f3.buffer().setEnabled( true );
  f3.buffer().setColor( QColor( 200, 110, 90 ) );
  f3.buffer().setSize( 1 );
  c5.setTextFormat( f3 );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << c1 << QgsTableCell( QStringLiteral( "Helicopter" ) ) << c3 )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) << c5 << QgsTableCell( QStringLiteral( "C" ) ) ) );

  table->setColumnWidths( QList< double >() << 0 << 0.0 << 30.0 );
  QgsLayoutChecker checker( QStringLiteral( "manualtable_textformat" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_manual_table" ) );
  const bool result = checker.testLayout( mReport );
  QVERIFY( result );
}

void TestQgsLayoutManualTable::cellTextAlignment()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  QgsTableCell c1( QStringLiteral( "Jet" ) );
  c1.setHorizontalAlignment( Qt::AlignRight );
  c1.setVerticalAlignment( Qt::AlignBottom );

  QgsTableCell c3( QStringLiteral( "Plane" ) );
  c3.setHorizontalAlignment( Qt::AlignCenter );
  c3.setVerticalAlignment( Qt::AlignTop );

  QgsTableCell c5( QStringLiteral( "B" ) );
  c5.setHorizontalAlignment( Qt::AlignRight );
  c5.setVerticalAlignment( Qt::AlignTop );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << c1 << QgsTableCell( QStringLiteral( "Helicopter\nHelicopter" ) ) << c3 )
                           << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A" ) ) << c5 << QgsTableCell( QStringLiteral( "C" ) ) ) );

  table->setColumnWidths( QList< double >() << 0 << 0.0 << 30.0 );
  QgsLayoutChecker checker( QStringLiteral( "manualtable_textalign" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_manual_table" ) );
  const bool result = checker.testLayout( mReport );
  QVERIFY( result );
}

QGSTEST_MAIN( TestQgsLayoutManualTable )
#include "testqgslayoutmanualtable.moc"
