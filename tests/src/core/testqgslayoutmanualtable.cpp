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
#include "qgscurrencynumericformat.h"
#include "qgsfontutils.h"
#include "qgslayout.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemmanualtable.h"
#include "qgslayouttablecolumn.h"
#include "qgsprintlayout.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgstest.h"

#include <QObject>

class TestQgsLayoutManualTable : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutManualTable()
      : QgsTest( u"Layout Manual Table Tests"_s, u"layout_manual_table"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

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
    void mergedCells();
    void mergedCellsVertOnly();
    void mergedCellsHozOnly();
    void mergedCellsBackgroundColor();

  private:
    //compares rows in table to expected rows
    void compareTable( QgsLayoutItemManualTable *table, const QVector<QStringList> &expectedRows );
};

void TestQgsLayoutManualTable::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsFontUtils::loadStandardTestFonts( QStringList() << u"Bold"_s );
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
  row << u"Jet"_s;
  expectedRows.clear();
  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), u"Jet"_s );

  // 1 x 2
  row.clear();
  row << u"Jet"_s << u"Helicopter"_s;
  expectedRows.clear();
  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), u"Jet"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), u"Helicopter"_s );

  // 1 x 3
  row.clear();
  row << u"Jet"_s << u"Helicopter"_s << u"Plane"_s;
  expectedRows.clear();
  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 1 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), u"Jet"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), u"Helicopter"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), u"Plane"_s );

  // unbalanced
  row.clear();
  expectedRows.clear();
  row << u"Jet"_s << u"Helicopter"_s << u"Plane"_s;
  expectedRows.append( row );
  row.clear();
  row << u"A"_s << QString() << QString();

  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), u"Jet"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), u"Helicopter"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), u"Plane"_s );
  QCOMPARE( table->tableContents().at( 1 ).size(), 1 );
  QCOMPARE( table->tableContents().at( 1 ).at( 0 ).content().toString(), u"A"_s );

  // unbalanced
  row.clear();
  expectedRows.clear();
  row << u"Jet"_s << u"Helicopter"_s << u"Plane"_s;
  expectedRows.append( row );
  row.clear();
  row << u"A"_s << u"B"_s << QString();

  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"B"_s ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), u"Jet"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), u"Helicopter"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), u"Plane"_s );
  QCOMPARE( table->tableContents().at( 1 ).size(), 2 );
  QCOMPARE( table->tableContents().at( 1 ).at( 0 ).content().toString(), u"A"_s );
  QCOMPARE( table->tableContents().at( 1 ).at( 1 ).content().toString(), u"B"_s );

  // 2 x 3
  row.clear();
  expectedRows.clear();
  row << u"Jet"_s << u"Helicopter"_s << u"Plane"_s;
  expectedRows.append( row );
  row.clear();
  row << u"A"_s << u"B"_s << u"C"_s;

  expectedRows.append( row );
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"B"_s ) << QgsTableCell( u"C"_s ) ) );
  compareTable( table, expectedRows );
  QCOMPARE( table->tableContents().size(), 2 );
  QCOMPARE( table->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 0 ).at( 0 ).content().toString(), u"Jet"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 1 ).content().toString(), u"Helicopter"_s );
  QCOMPARE( table->tableContents().at( 0 ).at( 2 ).content().toString(), u"Plane"_s );
  QCOMPARE( table->tableContents().at( 1 ).size(), 3 );
  QCOMPARE( table->tableContents().at( 1 ).at( 0 ).content().toString(), u"A"_s );
  QCOMPARE( table->tableContents().at( 1 ).at( 1 ).content().toString(), u"B"_s );
  QCOMPARE( table->tableContents().at( 1 ).at( 2 ).content().toString(), u"C"_s );

  table->setRowHeights( QList<double>() << 5.5 << 4.0 );
  table->setColumnWidths( QList<double>() << 15.5 << 14.0 << 13.4 );

  // save and restore

  //write to XML
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s
  );
  QDomDocument doc( documentType );
  QDomElement tableElement = doc.createElement( u"table"_s );
  QVERIFY( table->writeXml( tableElement, doc, QgsReadWriteContext(), true ) );

  //read from XML
  QgsLayoutItemManualTable *tableFromXml = new QgsLayoutItemManualTable( &l );
  QVERIFY( tableFromXml->readXml( tableElement.firstChildElement(), doc, QgsReadWriteContext(), true ) );

  QCOMPARE( tableFromXml->tableContents().size(), 2 );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).size(), 3 );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).at( 0 ).content().toString(), u"Jet"_s );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).at( 1 ).content().toString(), u"Helicopter"_s );
  QCOMPARE( tableFromXml->tableContents().at( 0 ).at( 2 ).content().toString(), u"Plane"_s );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).size(), 3 );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).at( 0 ).content().toString(), u"A"_s );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).at( 1 ).content().toString(), u"B"_s );
  QCOMPARE( tableFromXml->tableContents().at( 1 ).at( 2 ).content().toString(), u"C"_s );

  QCOMPARE( tableFromXml->rowHeights(), QList<double>() << 5.5 << 4.0 );
  QCOMPARE( tableFromXml->columnWidths(), QList<double>() << 15.5 << 14.0 << 13.4 );
}

void TestQgsLayoutManualTable::scopeForCell()
{
  QgsPrintLayout l( QgsProject::instance() );
  l.initializeDefaults();
  l.setName( u"my layout"_s );
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  std::unique_ptr<QgsExpressionContextScope> scope( table->scopeForCell( 1, 2 ) );

  // variable values for row/col should start at 1, not 0!
  QCOMPARE( scope->variable( u"row_number"_s ).toInt(), 2 );
  QCOMPARE( scope->variable( u"column_number"_s ).toInt(), 3 );
}

void TestQgsLayoutManualTable::expressionContents()
{
  QVector<QStringList> expectedRows;

  QgsPrintLayout l( QgsProject::instance() );
  l.initializeDefaults();
  l.setName( u"my layout"_s );
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  QStringList row;

  // 2 x 3
  row << u"Jet"_s << u"1,2"_s << u"1,3"_s;
  expectedRows.append( row );
  row.clear();
  row << u"my layout"_s << u"Helicopter"_s << u"Plane"_s;
  expectedRows.append( row );

  table->setTableContents(
    QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( QgsProperty::fromExpression( u"@row_number  || ',' || @column_number"_s ) ) << QgsTableCell( QgsProperty::fromExpression( u"@row_number  || ',' || @column_number"_s ) ) )
                       << ( QgsTableRow() << QgsTableCell( QgsProperty::fromExpression( u"@layout_name"_s ) ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) )
  );
  compareTable( table, expectedRows );

  // save and restore

  //write to XML
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s
  );
  QDomDocument doc( documentType );
  QDomElement tableElement = doc.createElement( u"table"_s );
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
  table->setTableContents( QgsTableContents() << ( QgsTableRow() << c11 << c12 << c13 ) << ( QgsTableRow() << c21 << c22 << c23 ) );
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
  row << u"Jet"_s << u"76"_s << u"$87.00"_s;
  expectedRows.clear();
  expectedRows.append( row );

  QgsTableCell c3;
  c3.setContent( 87 );
  auto format = std::make_unique<QgsCurrencyNumericFormat>();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( u"$"_s );
  c3.setNumericFormat( format.release() );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( 76 ) << c3 ) );
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

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"B"_s ) << QgsTableCell( u"C"_s ) ) );

  table->setRowHeights( QList<double>() << 0 << 40.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_rowheight"_s, &l );
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

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"B"_s ) << QgsTableCell( u"C"_s ) ) );

  table->setColumnWidths( QList<double>() << 0 << 10.0 << 30.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_columnwidth"_s, &l );
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

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );
  QgsTextFormat headerFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s, 16 ) ) );
  headerFormat.setColor( QColor( 255, 0, 255 ) );
  table->setHeaderTextFormat( headerFormat );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"Jet"_s ) << QgsTableCell( u"Helicopter"_s ) << QgsTableCell( u"Plane"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"B"_s ) << QgsTableCell( u"C"_s ) ) );
  table->setIncludeTableHeader( true );
  table->setHeaders( QgsLayoutTableColumns() << QgsLayoutTableColumn( u"header1"_s ) << QgsLayoutTableColumn( u"h2"_s ) << QgsLayoutTableColumn( u"header 3"_s ) );

  QGSVERIFYLAYOUTCHECK( u"manualtable_headers"_s, &l );
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

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  QgsTableCell c1( u"Jet"_s );
  QgsTextFormat f1 = table->contentTextFormat();
  f1.setSize( 20 );
  f1.buffer().setEnabled( true );
  f1.buffer().setColor( QColor( 100, 130, 150 ) );
  f1.buffer().setSize( 1 );
  c1.setTextFormat( f1 );

  QgsTableCell c3( u"Plane"_s );
  QgsTextFormat f2 = table->contentTextFormat();
  f2.setSize( 16 );
  f2.buffer().setEnabled( true );
  f2.buffer().setColor( QColor( 150, 110, 90 ) );
  f2.buffer().setSize( 1 );
  c3.setTextFormat( f2 );

  QgsTableCell c5( u"B"_s );
  QgsTextFormat f3 = table->contentTextFormat();
  f3.setSize( 36 );
  f3.buffer().setEnabled( true );
  f3.buffer().setColor( QColor( 200, 110, 90 ) );
  f3.buffer().setSize( 1 );
  c5.setTextFormat( f3 );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << c1 << QgsTableCell( u"Helicopter"_s ) << c3 ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << c5 << QgsTableCell( u"C"_s ) ) );

  table->setColumnWidths( QList<double>() << 0 << 0.0 << 30.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_textformat"_s, &l );
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

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  QgsTableCell c1( u"Jet"_s );
  c1.setHorizontalAlignment( Qt::AlignRight );
  c1.setVerticalAlignment( Qt::AlignBottom );

  QgsTableCell c3( u"Plane"_s );
  c3.setHorizontalAlignment( Qt::AlignCenter );
  c3.setVerticalAlignment( Qt::AlignTop );

  QgsTableCell c5( u"B"_s );
  c5.setHorizontalAlignment( Qt::AlignRight );
  c5.setVerticalAlignment( Qt::AlignTop );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << c1 << QgsTableCell( u"Helicopter\nHelicopter"_s ) << c3 ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << c5 << QgsTableCell( u"C"_s ) ) );

  table->setColumnWidths( QList<double>() << 0 << 0.0 << 30.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_textalign"_s, &l );
}

void TestQgsLayoutManualTable::mergedCells()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  QgsTableCell c1( u"Jet"_s );
  c1.setSpan( 3, 2 );
  c1.setBackgroundColor( QColor( 255, 200, 220 ) );

  QgsTableCell c3( u"Plane"_s );
  c3.setHorizontalAlignment( Qt::AlignCenter );
  c3.setVerticalAlignment( Qt::AlignTop );
  c3.setSpan( 2, 1 );
  c3.setBackgroundColor( QColor( 255, 230, 200 ) );

  QgsTableCell c5( u"B"_s );
  c5.setHorizontalAlignment( Qt::AlignRight );
  c5.setVerticalAlignment( Qt::AlignTop );
  c5.setSpan( 1, 3 );
  c5.setBackgroundColor( QColor( 200, 250, 200 ) );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"A1"_s ) << c1 << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"Something"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"C"_s ) ) << ( QgsTableRow() << QgsTableCell( u"C"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"hidden by span"_s ) << c3 ) << ( QgsTableRow() << QgsTableCell( u"D"_s ) << QgsTableCell( u"E"_s ) << QgsTableCell( u"F"_s ) << QgsTableCell( u"hidden by span"_s ) ) << ( QgsTableRow() << c5 << QgsTableCell( u"hidden"_s ) << QgsTableCell( u"hidden"_s ) << QgsTableCell( u"G"_s ) ) );

  table->setColumnWidths( QList<double>() << 30 << 50.0 << 40.0 << 25.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_merged"_s, &l );
}

void TestQgsLayoutManualTable::mergedCellsVertOnly()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( false );
  table->setVerticalGrid( true );

  QgsTableCell c1( u"Jet"_s );
  c1.setSpan( 3, 2 );
  c1.setBackgroundColor( QColor( 255, 200, 220 ) );

  QgsTableCell c3( u"Plane"_s );
  c3.setHorizontalAlignment( Qt::AlignCenter );
  c3.setVerticalAlignment( Qt::AlignTop );
  c3.setSpan( 2, 1 );
  c3.setBackgroundColor( QColor( 255, 230, 200 ) );

  QgsTableCell c5( u"B"_s );
  c5.setHorizontalAlignment( Qt::AlignRight );
  c5.setVerticalAlignment( Qt::AlignTop );
  c5.setSpan( 1, 3 );
  c5.setBackgroundColor( QColor( 200, 250, 200 ) );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"A1"_s ) << c1 << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"Something"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"C"_s ) ) << ( QgsTableRow() << QgsTableCell( u"C"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"hidden by span"_s ) << c3 ) << ( QgsTableRow() << QgsTableCell( u"D"_s ) << QgsTableCell( u"E"_s ) << QgsTableCell( u"F"_s ) << QgsTableCell( u"hidden by span"_s ) ) << ( QgsTableRow() << c5 << QgsTableCell( u"hidden"_s ) << QgsTableCell( u"hidden"_s ) << QgsTableCell( u"G"_s ) ) );

  table->setColumnWidths( QList<double>() << 30 << 50.0 << 40.0 << 25.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_merged_vert_only"_s, &l );
}

void TestQgsLayoutManualTable::mergedCellsHozOnly()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( false );

  QgsTableCell c1( u"Jet"_s );
  c1.setSpan( 3, 2 );
  c1.setBackgroundColor( QColor( 255, 200, 220 ) );

  QgsTableCell c3( u"Plane"_s );
  c3.setHorizontalAlignment( Qt::AlignCenter );
  c3.setVerticalAlignment( Qt::AlignTop );
  c3.setSpan( 2, 1 );
  c3.setBackgroundColor( QColor( 255, 230, 200 ) );

  QgsTableCell c5( u"B"_s );
  c5.setHorizontalAlignment( Qt::AlignRight );
  c5.setVerticalAlignment( Qt::AlignTop );
  c5.setSpan( 1, 3 );
  c5.setBackgroundColor( QColor( 200, 250, 200 ) );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( u"A1"_s ) << c1 << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"Something"_s ) ) << ( QgsTableRow() << QgsTableCell( u"A"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"C"_s ) ) << ( QgsTableRow() << QgsTableCell( u"C"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"hidden by span"_s ) << c3 ) << ( QgsTableRow() << QgsTableCell( u"D"_s ) << QgsTableCell( u"E"_s ) << QgsTableCell( u"F"_s ) << QgsTableCell( u"hidden by span"_s ) ) << ( QgsTableRow() << c5 << QgsTableCell( u"hidden"_s ) << QgsTableCell( u"hidden"_s ) << QgsTableCell( u"G"_s ) ) );

  table->setColumnWidths( QList<double>() << 30 << 50.0 << 40.0 << 25.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_merged_hoz_only"_s, &l );
}

void TestQgsLayoutManualTable::mergedCellsBackgroundColor()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, table );
  frame1->attemptSetSceneRect( QRectF( 5, 5, 100, 60 ) );
  frame1->setFrameEnabled( true );
  table->addFrame( frame1 );
  table->setBackgroundColor( Qt::yellow );

  table->setContentTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) ) );

  frame1->setFrameEnabled( false );
  table->setShowGrid( true );
  table->setHorizontalGrid( true );
  table->setVerticalGrid( true );

  QgsLayoutTableStyle lastColumnStyle;
  lastColumnStyle.enabled = true;
  lastColumnStyle.cellBackgroundColor = QColor( 200, 250, 220 );
  table->setCellStyle( QgsLayoutTable::CellStyleGroup::LastColumn, lastColumnStyle );

  QgsLayoutTableStyle lastRowStyle;
  lastRowStyle.enabled = true;
  lastRowStyle.cellBackgroundColor = QColor( 220, 220, 250 );
  table->setCellStyle( QgsLayoutTable::CellStyleGroup::LastRow, lastRowStyle );

  QgsLayoutTableStyle oddColumnStyle;
  oddColumnStyle.enabled = true;
  oddColumnStyle.cellBackgroundColor = QColor( 100, 250, 220 );
  table->setCellStyle( QgsLayoutTable::CellStyleGroup::OddColumns, oddColumnStyle );

  QgsLayoutTableStyle oddRowStyle;
  oddRowStyle.enabled = true;
  oddRowStyle.cellBackgroundColor = QColor( 200, 150, 220 );
  table->setCellStyle( QgsLayoutTable::CellStyleGroup::OddRows, oddRowStyle );

  QgsTableCell c1( u"Jet"_s );
  c1.setSpan( 2, 2 );

  QgsTableCell c3( u"Plane"_s );
  c3.setHorizontalAlignment( Qt::AlignCenter );
  c3.setVerticalAlignment( Qt::AlignTop );
  c3.setSpan( 3, 1 );

  QgsTableCell c4( u"Plane"_s );
  c4.setHorizontalAlignment( Qt::AlignCenter );
  c4.setVerticalAlignment( Qt::AlignTop );
  c4.setSpan( 2, 1 );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << c1 << QgsTableCell( u"A2"_s ) << c1 << QgsTableCell( u"Something"_s ) ) << ( QgsTableRow() << QgsTableCell( u"B"_s ) << QgsTableCell( u"B2"_s ) << QgsTableCell( u"hidden by span"_s ) << QgsTableCell( u"hidden by span"_s ) ) << ( QgsTableRow() << QgsTableCell( u"C"_s ) << QgsTableCell( u"C2"_s ) << QgsTableCell( u"C3"_s ) << c3 ) << ( QgsTableRow() << c4 << QgsTableCell( u"E"_s ) << QgsTableCell( u"F"_s ) << QgsTableCell( u"hidden by span"_s ) ) << ( QgsTableRow() << QgsTableCell( u"hidden"_s ) << QgsTableCell( u"D2"_s ) << QgsTableCell( u"D3"_s ) << QgsTableCell( u"G"_s ) ) );

  table->setColumnWidths( QList<double>() << 30 << 50.0 << 40.0 << 25.0 );
  QGSVERIFYLAYOUTCHECK( u"manualtable_merged_background_color"_s, &l );
}

QGSTEST_MAIN( TestQgsLayoutManualTable )
#include "testqgslayoutmanualtable.moc"
