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

#include <QObject>
#include "qgstest.h"

class TestQgsLayoutManualTable : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutManualTable() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void setContents();
    void cellStyles();
    void cellFormat();

  private:
    QString mReport;

    //compares rows in table to expected rows
    void compareTable( QgsLayoutItemManualTable *table, const QVector<QStringList> &expectedRows );
};

void TestQgsLayoutManualTable::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport = QStringLiteral( "<h1>Layout Manual Table Tests</h1>\n" );
}

void TestQgsLayoutManualTable::cleanupTestCase()
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

void TestQgsLayoutManualTable::init()
{
}

void TestQgsLayoutManualTable::cleanup()
{
}

void TestQgsLayoutManualTable::compareTable( QgsLayoutItemManualTable *table, const QVector<QStringList> &expectedRows )
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

  // save and restore

  //write to XML
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
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
}

void TestQgsLayoutManualTable::cellStyles()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemManualTable *table = new QgsLayoutItemManualTable( &l );

  QgsTableCell c11;
  QgsTableCell c12;
  c12.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTableCell c13;
  c13.setForegroundColor( QColor( 0, 255, 0 ) );
  QgsTableCell c21;
  QgsTableCell c22;
  c22.setBackgroundColor( QColor( 255, 255, 0 ) );
  c22.setForegroundColor( QColor( 255, 0, 255 ) );
  QgsTableCell c23;

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
  std::unique_ptr< QgsCurrencyNumericFormat > format = qgis::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );

  table->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << QgsTableCell( 76 ) << c3 ) );
  compareTable( table, expectedRows );
}

QGSTEST_MAIN( TestQgsLayoutManualTable )
#include "testqgslayoutmanualtable.moc"
