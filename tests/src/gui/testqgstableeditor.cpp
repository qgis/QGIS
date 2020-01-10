/***************************************************************************
    testqgstableeditor.cpp
     --------------------------------------
    Date                 : January 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgstableeditorwidget.h"
#include "qgscurrencynumericformat.h"

class TestQgsTableEditor: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void testData();
    void insertRowsBelow();
    void insertRowsAbove();
    void insertColumnsBefore();
    void insertColumnsAfter();

  private:

};

void TestQgsTableEditor::initTestCase()
{

}

void TestQgsTableEditor::cleanupTestCase()
{
}

void TestQgsTableEditor::init()
{
}

void TestQgsTableEditor::cleanup()
{
}

void TestQgsTableEditor::testData()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableData().isEmpty() );

  QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = qgis::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  c2.setForegroundColor( QColor( 0, 255, 0 ) );
  w.setTableData( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableData().size(), 1 );
  QCOMPARE( w.tableData().at( 0 ).size(), 3 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );

}

void TestQgsTableEditor::insertRowsBelow()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableData().isEmpty() );

  QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = qgis::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  c2.setForegroundColor( QColor( 0, 255, 0 ) );
  w.setTableData( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertRowsBelow();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableData().size(), 2 );
  QCOMPARE( w.tableData().at( 0 ).size(), 3 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableData().at( 1 ).size(), 3 );
  QCOMPARE( w.tableData().at( 1 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).foregroundColor().isValid() );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 1, 2 ), QItemSelectionModel::Select );
  w.insertRowsBelow();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableData().size(), 4 );
  QCOMPARE( w.tableData().at( 0 ).size(), 3 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableData().at( 1 ).size(), 3 );
  QCOMPARE( w.tableData().at( 1 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 2 ).size(), 3 );
  QCOMPARE( w.tableData().at( 2 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 2 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 2 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 2 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 2 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 2 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 2 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 2 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 2 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 3 ).size(), 3 );
  QCOMPARE( w.tableData().at( 3 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 3 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 3 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 3 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 3 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 3 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 3 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 3 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 3 ).at( 2 ).foregroundColor().isValid() );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 2, 2 ), QItemSelectionModel::Select );
  w.insertRowsBelow();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableData().size(), 4 );
}

void TestQgsTableEditor::insertRowsAbove()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableData().isEmpty() );

  QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = qgis::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  c2.setForegroundColor( QColor( 0, 255, 0 ) );
  w.setTableData( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertRowsAbove();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableData().size(), 2 );
  QCOMPARE( w.tableData().at( 0 ).size(), 3 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).size(), 3 );
  QCOMPARE( w.tableData().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 1 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 1 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 1 ).at( 1 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 1 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableData().at( 1 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 1 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableData().at( 1 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 1, 2 ), QItemSelectionModel::Select );
  w.insertRowsAbove();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableData().size(), 4 );
  QCOMPARE( w.tableData().at( 0 ).size(), 3 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).size(), 3 );
  QCOMPARE( w.tableData().at( 1 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 1 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 1 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 2 ).size(), 3 );
  QCOMPARE( w.tableData().at( 2 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 2 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 2 ).at( 0 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 2 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 2 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 2 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 2 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 2 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 2 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 3 ).size(), 3 );
  QCOMPARE( w.tableData().at( 3 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 3 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 3 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 3 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 3 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 3 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 3 ).at( 1 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 3 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableData().at( 3 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 3 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 3 ).at( 2 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 3 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableData().at( 3 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 2, 2 ), QItemSelectionModel::Select );
  w.insertRowsAbove();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableData().size(), 4 );
}

void TestQgsTableEditor::insertColumnsBefore()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableData().isEmpty() );

  QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = qgis::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  c2.setForegroundColor( QColor( 0, 255, 0 ) );
  w.setTableData( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertColumnsBefore();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableData().size(), 1 );
  QCOMPARE( w.tableData().at( 0 ).size(), 4 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 0 ).at( 3 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 3 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::Select );
  w.insertColumnsBefore();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableData().size(), 1 );
  QCOMPARE( w.tableData().at( 0 ).size(), 6 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 3 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 4 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 0 ).at( 4 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 0 ).at( 4 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 0 ).at( 4 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 5 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 5 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 5 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 0 ).at( 5 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 5 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  w.insertColumnsBefore();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableData().at( 0 ).size(), 6 );
}

void TestQgsTableEditor::insertColumnsAfter()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableData().isEmpty() );

  QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = qgis::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  c2.setForegroundColor( QColor( 0, 255, 0 ) );
  w.setTableData( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertColumnsAfter();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableData().size(), 1 );
  QCOMPARE( w.tableData().at( 0 ).size(), 4 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 0 ).at( 3 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 3 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::Select );
  w.insertColumnsAfter();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableData().size(), 1 );
  QCOMPARE( w.tableData().at( 0 ).size(), 6 );
  QCOMPARE( w.tableData().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).foregroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableData().at( 0 ).at( 1 ).foregroundColor(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableData().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 2 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 3 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 3 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 4 ).content().toString(), QString() );
  QVERIFY( !w.tableData().at( 0 ).at( 4 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 4 ).foregroundColor().isValid() );
  QCOMPARE( w.tableData().at( 0 ).at( 5 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableData().at( 0 ).at( 5 ).backgroundColor().isValid() );
  QVERIFY( !w.tableData().at( 0 ).at( 5 ).foregroundColor().isValid() );
  QVERIFY( w.tableData().at( 0 ).at( 5 ).numericFormat() );
  QCOMPARE( w.tableData().at( 0 ).at( 5 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  w.insertColumnsAfter();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableData().at( 0 ).size(), 6 );
}


QGSTEST_MAIN( TestQgsTableEditor )
#include "testqgstableeditor.moc"
