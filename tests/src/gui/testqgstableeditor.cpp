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
#include "qgsbearingnumericformat.h"
#include <QSignalSpy>

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
    void deleteRows();
    void deleteColumns();
    void selectRows();
    void selectColumns();
    void clearSelected();
    void foregroundColor();
    void backgroundColor();
    void alignment();
    void properties();
    void textFormat();
    void numericFormat();
    void rowHeight();
    void columnWidth();
    void headers();

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
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  c3.setHorizontalAlignment( Qt::AlignJustify );
  c3.setVerticalAlignment( Qt::AlignBottom );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat textFormat;
  textFormat.setSize( 12.6 );
  textFormat.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( textFormat );
  c2.setHorizontalAlignment( Qt::AlignRight );
  c2.setVerticalAlignment( Qt::AlignTop );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).horizontalAlignment(), Qt::AlignLeft );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).verticalAlignment(), Qt::AlignVCenter );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QVERIFY( w.tableContents().at( 0 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().size(), 12.6 );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).horizontalAlignment(), Qt::AlignRight );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).verticalAlignment(), Qt::AlignTop );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).horizontalAlignment(), Qt::AlignJustify );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).verticalAlignment(), Qt::AlignBottom );
}

void TestQgsTableEditor::insertRowsBelow()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) ) ;
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertRowsBelow();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableContents().size(), 2 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 1 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).textFormat().isValid() );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 1, 2 ), QItemSelectionModel::Select );
  w.insertRowsBelow();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableContents().size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 1 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 2 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 2 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 2 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 2 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 2 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 2 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 2 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 2 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 2 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 2 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 3 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 3 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 3 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 3 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 3 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 3 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 3 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 3 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 3 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 3 ).at( 2 ).textFormat().isValid() );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 2, 2 ), QItemSelectionModel::Select );
  w.insertRowsBelow();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableContents().size(), 4 );
}

void TestQgsTableEditor::insertRowsAbove()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertRowsAbove();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableContents().size(), 2 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 1 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 1 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 1, 2 ), QItemSelectionModel::Select );
  w.insertRowsAbove();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableContents().size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 1 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 2 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 2 ).at( 0 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 2 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 2 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 2 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 2 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 2 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 2 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 2 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 2 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 3 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 3 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 3 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 3 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 3 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 3 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 3 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 3 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 3 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 3 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 3 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 3 ).at( 2 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 3 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 3 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 2, 2 ), QItemSelectionModel::Select );
  w.insertRowsAbove();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableContents().size(), 4 );
}

void TestQgsTableEditor::insertColumnsBefore()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertColumnsBefore();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 3 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::Select );
  w.insertColumnsBefore();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 6 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 4 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 4 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 4 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 4 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 5 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 5 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 5 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 5 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 5 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  w.insertColumnsBefore();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 6 );
}

void TestQgsTableEditor::insertColumnsAfter()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.insertColumnsAfter();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 3 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // two rows selected = insert two rows, etc
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::Select );
  w.insertColumnsAfter();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 6 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 4 ).content().toString(), QString() );
  QVERIFY( !w.tableContents().at( 0 ).at( 4 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 4 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 5 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 5 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 5 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 5 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 5 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // non consecutive selection = no action
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  w.insertColumnsAfter();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 6 );
}

void TestQgsTableEditor::deleteRows()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  QgsTableCell c2( 76 );
  c2.setNumericFormat( format.release() );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) )
                      << ( QgsTableRow() << c2 )
                      << ( QgsTableRow() << c3 )
                      << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  // no selection
  w.selectionModel()->clear();
  w.deleteRows();
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.tableContents().size(), 4 );

  w.selectionModel()->select( w.model()->index( 2, 0 ), QItemSelectionModel::ClearAndSelect );
  w.deleteRows();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableContents().size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 2 ).at( 0 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 2 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 2 ).at( 0 ).textFormat().isValid() );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 2, 0 ), QItemSelectionModel::Select );
  w.deleteRows();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // last row can't be deleted
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  w.deleteRows();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 1 );

}

void TestQgsTableEditor::deleteColumns()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  QgsTableCell c2( 76 );
  c2.setNumericFormat( format.release() );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  // no selection
  w.selectionModel()->clear();
  w.deleteColumns();
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );

  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::ClearAndSelect );
  w.deleteColumns();
  QCOMPARE( spy.count(), 2 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::Select );
  w.deleteColumns();
  QCOMPARE( spy.count(), 3 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).numericFormat()->id(), QStringLiteral( "currency" ) );

  // last column can't be deleted
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  w.deleteColumns();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 1 );

}

void TestQgsTableEditor::selectRows()
{
  QgsTableEditorWidget w;
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell()  << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() ) );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  w.expandRowSelection();
  QCOMPARE( w.selectionModel()->selectedIndexes().size(), 3 );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 0, 0 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 0, 1 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 0, 2 ) ) );


  w.selectionModel()->select( w.model()->index( 1, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 1, 2 ), QItemSelectionModel::Select );
  w.selectionModel()->select( w.model()->index( 2, 0 ), QItemSelectionModel::Select );
  w.expandRowSelection();
  QCOMPARE( w.selectionModel()->selectedIndexes().size(), 6 );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 1, 0 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 1, 1 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 1, 2 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 2, 0 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 2, 1 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 2, 2 ) ) );

  w.selectionModel()->clearSelection();
  w.expandRowSelection();
  QCOMPARE( w.selectionModel()->selectedIndexes().size(), 0 );
}

void TestQgsTableEditor::selectColumns()
{
  QgsTableEditorWidget w;
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell()  << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() ) );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  w.expandColumnSelection();
  QCOMPARE( w.selectionModel()->selectedIndexes().size(), 3 );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 0, 0 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 1, 0 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 2, 0 ) ) );

  w.selectionModel()->select( w.model()->index( 1, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 2, 1 ), QItemSelectionModel::Select );
  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::Select );
  w.expandColumnSelection();
  QCOMPARE( w.selectionModel()->selectedIndexes().size(), 6 );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 0, 1 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 1, 1 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 2, 1 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 0, 2 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 1, 2 ) ) );
  QVERIFY( w.selectionModel()->isSelected( w.model()->index( 2, 2 ) ) );

  w.selectionModel()->clearSelection();
  w.expandColumnSelection();
  QCOMPARE( w.selectionModel()->selectedIndexes().size(), 0 );
}

void TestQgsTableEditor::clearSelected()
{
  QgsTableEditorWidget w;
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "A1" ) ) << QgsTableCell( QStringLiteral( "A2" ) ) << QgsTableCell( QStringLiteral( "A3" ) ) )
                      << ( QgsTableRow() << QgsTableCell( QStringLiteral( "B1" ) )  << QgsTableCell( QStringLiteral( "B2" ) ) << QgsTableCell( QStringLiteral( "B3" ) ) )
                      << ( QgsTableRow() << QgsTableCell( QStringLiteral( "C1" ) ) << QgsTableCell( QStringLiteral( "C2" ) ) << QgsTableCell( QStringLiteral( "C3" ) ) ) );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  w.selectionModel()->clearSelection();
  w.clearSelectedCells();
  QCOMPARE( spy.count(), 0 );
  QCOMPARE( w.tableContents().size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "A1" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "A2" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "A3" ) );
  QCOMPARE( w.tableContents().at( 1 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "B1" ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content().toString(), QStringLiteral( "B2" ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content().toString(), QStringLiteral( "B3" ) );
  QCOMPARE( w.tableContents().at( 2 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 2 ).at( 0 ).content().toString(), QStringLiteral( "C1" ) );
  QCOMPARE( w.tableContents().at( 2 ).at( 1 ).content().toString(), QStringLiteral( "C2" ) );
  QCOMPARE( w.tableContents().at( 2 ).at( 2 ).content().toString(), QStringLiteral( "C3" ) );

  w.selectionModel()->select( w.model()->index( 1, 1 ), QItemSelectionModel::ClearAndSelect );
  w.selectionModel()->select( w.model()->index( 1, 2 ), QItemSelectionModel::Select );
  w.selectionModel()->select( w.model()->index( 0, 2 ), QItemSelectionModel::Select );
  w.clearSelectedCells();
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.tableContents().size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "A1" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "A2" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QString() );
  QCOMPARE( w.tableContents().at( 1 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content().toString(), QStringLiteral( "B1" ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content().toString(), QString() );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content().toString(), QString() );
  QCOMPARE( w.tableContents().at( 2 ).size(), 3 );
  QCOMPARE( w.tableContents().at( 2 ).at( 0 ).content().toString(), QStringLiteral( "C1" ) );
  QCOMPARE( w.tableContents().at( 2 ).at( 1 ).content().toString(), QStringLiteral( "C2" ) );
  QCOMPARE( w.tableContents().at( 2 ).at( 2 ).content().toString(), QStringLiteral( "C3" ) );

}

void TestQgsTableEditor::foregroundColor()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat f = c2.textFormat();
  f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).numericFormat() );

  w.selectionModel()->clearSelection();
  QgsTextFormat f2;
  f2.setColor( QColor( 255, 255, 0 ) );
  w.setSelectionTextFormat( f2 );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QVERIFY( !w.selectionTextFormat().isValid() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionTextFormat().isValid() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionTextFormat().color(), QColor( 0, 255, 0 ) );
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionTextFormat().isValid() );
  w.setSelectionTextFormat( f2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.selectionTextFormat().color(), QColor( 255, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).textFormat().color(), QColor( 255, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 255, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionTextFormat().isValid() );
}

void TestQgsTableEditor::backgroundColor()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QVERIFY( w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).numericFormat() );

  w.selectionModel()->clearSelection();
  w.setSelectionBackgroundColor( QColor( 255, 255, 0 ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QVERIFY( !w.selectionBackgroundColor().isValid() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionBackgroundColor().isValid() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionBackgroundColor(), QColor( 255, 0, 0 ) );
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionBackgroundColor().isValid() );
  w.setSelectionBackgroundColor( QColor( 255, 255, 0 ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.selectionBackgroundColor(), QColor( 255, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).backgroundColor(), QColor( 255, 255, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 255, 0 ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionBackgroundColor().isValid() );
}

void TestQgsTableEditor::alignment()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  QgsTableCell c2( 76 );
  c2.setHorizontalAlignment( Qt::AlignRight );
  c2.setVerticalAlignment( Qt::AlignBottom );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).horizontalAlignment(), Qt::AlignLeft );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).verticalAlignment(), Qt::AlignVCenter );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).horizontalAlignment(), Qt::AlignRight );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).verticalAlignment(), Qt::AlignBottom );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).horizontalAlignment(), Qt::AlignLeft );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).verticalAlignment(), Qt::AlignVCenter );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "Jet3" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).horizontalAlignment(), Qt::AlignLeft );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).verticalAlignment(), Qt::AlignVCenter );

  w.selectionModel()->clearSelection();
  w.setSelectionVerticalAlignment( Qt::AlignTop );
  w.setSelectionHorizontalAlignment( Qt::AlignCenter );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).horizontalAlignment(), Qt::AlignLeft );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).verticalAlignment(), Qt::AlignVCenter );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionHorizontalAlignment(), Qt::AlignLeft | Qt::AlignTop );
  QCOMPARE( w.selectionVerticalAlignment(), Qt::AlignLeft | Qt::AlignTop );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionHorizontalAlignment(), Qt::AlignRight );
  QCOMPARE( w.selectionVerticalAlignment(), Qt::AlignBottom );
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionHorizontalAlignment(), Qt::AlignLeft | Qt::AlignTop );
  QCOMPARE( w.selectionVerticalAlignment(), Qt::AlignLeft | Qt::AlignTop );
  w.setSelectionHorizontalAlignment( Qt::AlignJustify );
  w.setSelectionVerticalAlignment( Qt::AlignTop );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.selectionHorizontalAlignment(), Qt::AlignJustify );
  QCOMPARE( w.selectionVerticalAlignment(), Qt::AlignTop );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).horizontalAlignment(), Qt::AlignJustify );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).verticalAlignment(), Qt::AlignTop );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).horizontalAlignment(), Qt::AlignJustify );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).verticalAlignment(), Qt::AlignTop );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).horizontalAlignment(), Qt::AlignLeft );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).verticalAlignment(), Qt::AlignVCenter );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).horizontalAlignment(), Qt::AlignLeft );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).verticalAlignment(), Qt::AlignVCenter );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionHorizontalAlignment(), Qt::AlignLeft | Qt::AlignTop );
  QCOMPARE( w.selectionVerticalAlignment(), Qt::AlignLeft | Qt::AlignTop );
}

void TestQgsTableEditor::properties()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  c3.setNumericFormat( format.release() );
  const QgsTableCell c2( QVariant::fromValue( QgsProperty::fromExpression( "1+2" ) ) );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).content().canConvert< QgsProperty >() );
  QVERIFY( w.tableContents().at( 0 ).at( 1 ).content().canConvert< QgsProperty >() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().value< QgsProperty >().asExpression(), QStringLiteral( "1+2" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).content().canConvert< QgsProperty >() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).content().canConvert< QgsProperty >() );

  w.selectionModel()->clearSelection();
  w.setSelectionCellProperty( QgsProperty::fromExpression( QStringLiteral( "2+3" ) ) );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).content().canConvert< QgsProperty >() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionCellProperty().isActive() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  QVERIFY( w.selectionCellProperty().isActive() );
  QCOMPARE( w.selectionCellProperty().asExpression(), QStringLiteral( "1+2" ) );
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionCellProperty().isActive() );
  w.setSelectionCellProperty( QgsProperty::fromExpression( QStringLiteral( "3+4" ) ) );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( w.selectionCellProperty().isActive() );
  QCOMPARE( w.selectionCellProperty().asExpression(), QStringLiteral( "3+4" ) );
  QVERIFY( w.tableContents().at( 0 ).at( 0 ).content().canConvert< QgsProperty >() );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().value< QgsProperty >().asExpression(), QStringLiteral( "3+4" ) );
  QVERIFY( w.tableContents().at( 0 ).at( 1 ).content().canConvert< QgsProperty >() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().value< QgsProperty >().asExpression(), QStringLiteral( "3+4" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).content().canConvert< QgsProperty >() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).content().canConvert< QgsProperty >() );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionCellProperty().isActive() );
}

void TestQgsTableEditor::textFormat()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  QgsTextFormat format;
  format.setSize( 12.6 );
  c3.setTextFormat( format );
  QgsTableCell c2( 76 );
  c2.setTextFormat( format );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().size(), 12.6 );
  QVERIFY( w.tableContents().at( 0 ).at( 1 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).textFormat().size(), 12.6 );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );

  w.selectionModel()->clearSelection();
  format.setSize( 21 );
  w.setSelectionTextFormat( format );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QVERIFY( w.selectionTextFormat().size() !=  21.0 );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QVERIFY( w.selectionTextFormat().size() !=  21.0 );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionTextFormat().size(), 12.6 );
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::Select );
  QVERIFY( w.selectionTextFormat().size() !=  21.0 );
  w.setSelectionTextFormat( format );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( w.selectionTextFormat().isValid() );
  QCOMPARE( w.selectionTextFormat().size(), 21.0 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).textFormat().size(), 21.0 );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().size(), 21.0 );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).textFormat().size(), 12.6 );
  QVERIFY( w.tableContents().at( 0 ).at( 3 ).textFormat().size() != 21 );
}

void TestQgsTableEditor::numericFormat()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  QgsTableCell c3;
  c3.setContent( 87 );
  std::unique_ptr< QgsCurrencyNumericFormat > format = std::make_unique< QgsCurrencyNumericFormat >();
  format->setNumberDecimalPlaces( 2 );
  format->setPrefix( QStringLiteral( "$" ) );
  QgsTableCell c2( 76 );
  c2.setNumericFormat( format.release() );
  c2.setBackgroundColor( QColor( 255, 0, 0 ) );
  QgsTextFormat c2f;
  c2f.setColor( QColor( 0, 255, 0 ) );
  c2.setTextFormat( c2f );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( QStringLiteral( "Jet" ) ) << c2 << c3 << QgsTableCell( QStringLiteral( "Jet3" ) ) ) );
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( w.tableContents().size(), 1 );
  QCOMPARE( w.tableContents().at( 0 ).size(), 4 );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content().toString(), QStringLiteral( "Jet" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 0 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content().toString(), QStringLiteral( "76" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).backgroundColor(), QColor( 255, 0, 0 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).textFormat().color(), QColor( 0, 255, 0 ) );
  QVERIFY( w.tableContents().at( 0 ).at( 1 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).numericFormat()->id(), QStringLiteral( "currency" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content().toString(), QStringLiteral( "87" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 3 ).content().toString(), QStringLiteral( "Jet3" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).backgroundColor().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).textFormat().isValid() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).numericFormat() );

  w.selectionModel()->clearSelection();
  w.setSelectionNumericFormat( new QgsBearingNumericFormat() );
  QVERIFY( !w.hasMixedSelectionNumericFormat() );
  QCOMPARE( spy.count(), 1 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QVERIFY( !w.selectionNumericFormat() );
  QVERIFY( !w.hasMixedSelectionNumericFormat() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionNumericFormat() );
  QVERIFY( w.hasMixedSelectionNumericFormat() );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionNumericFormat()->id(), QStringLiteral( "currency" ) );
  QVERIFY( !w.hasMixedSelectionNumericFormat() );
  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionNumericFormat() );
  QVERIFY( w.hasMixedSelectionNumericFormat() );
  w.setSelectionNumericFormat( new QgsBearingNumericFormat() );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.selectionNumericFormat()->id(), QStringLiteral( "bearing" ) );
  QVERIFY( !w.hasMixedSelectionNumericFormat() );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).numericFormat()->id(), QStringLiteral( "bearing" ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).numericFormat()->id(), QStringLiteral( "bearing" ) );
  QVERIFY( !w.tableContents().at( 0 ).at( 2 ).numericFormat() );
  QVERIFY( !w.tableContents().at( 0 ).at( 3 ).numericFormat() );
  w.selectionModel()->select( w.model()->index( 0, 3 ), QItemSelectionModel::Select );
  QVERIFY( !w.selectionNumericFormat() );
  QVERIFY( w.hasMixedSelectionNumericFormat() );
}

void TestQgsTableEditor::rowHeight()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell()  << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() ) );
  QCOMPARE( spy.count(), 1 );
  w.setTableRowHeight( 1, 14.0 );

  QCOMPARE( w.selectionRowHeight(), 0.0 );
  QCOMPARE( w.tableRowHeight( 0 ), 0.0 );
  QCOMPARE( w.tableRowHeight( 1 ), 14.0 );
  QCOMPARE( w.tableRowHeight( 2 ), 0.0 );
  w.selectionModel()->clearSelection();
  w.setSelectionRowHeight( 15.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.selectionRowHeight(), 0.0 );
  QCOMPARE( w.tableRowHeight( 0 ), 0.0 );
  QCOMPARE( w.tableRowHeight( 1 ), 14.0 );
  QCOMPARE( w.tableRowHeight( 2 ), 0.0 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionRowHeight(), 0.0 );
  w.selectionModel()->select( w.model()->index( 1, 0 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionRowHeight(), -1.0 );
  w.selectionModel()->select( w.model()->index( 1, 0 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionRowHeight(), 14.0 );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionRowHeight(), -1.0 );
  w.setSelectionRowHeight( 15.0 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.selectionRowHeight(), 15.0 );
  QCOMPARE( w.tableRowHeight( 0 ), 15.0 );
  QCOMPARE( w.tableRowHeight( 1 ), 15.0 );
  QCOMPARE( w.tableRowHeight( 2 ), 0.0 );
  w.selectionModel()->select( w.model()->index( 2, 2 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionRowHeight(), -1.0 );
}

void TestQgsTableEditor::columnWidth()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell()  << QgsTableCell() << QgsTableCell() )
                      << ( QgsTableRow() << QgsTableCell() << QgsTableCell() << QgsTableCell() ) );
  QCOMPARE( spy.count(), 1 );
  w.setTableColumnWidth( 1, 14.0 );

  QCOMPARE( w.selectionColumnWidth(), 0.0 );
  QCOMPARE( w.tableColumnWidth( 0 ), 0.0 );
  QCOMPARE( w.tableColumnWidth( 1 ), 14.0 );
  QCOMPARE( w.tableColumnWidth( 2 ), 0.0 );
  w.selectionModel()->clearSelection();
  w.setSelectionColumnWidth( 15.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.selectionColumnWidth(), 0.0 );
  QCOMPARE( w.tableColumnWidth( 0 ), 0.0 );
  QCOMPARE( w.tableColumnWidth( 1 ), 14.0 );
  QCOMPARE( w.tableColumnWidth( 2 ), 0.0 );

  w.selectionModel()->select( w.model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionColumnWidth(), 0.0 );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionColumnWidth(), -1.0 );
  w.selectionModel()->select( w.model()->index( 0, 1 ), QItemSelectionModel::ClearAndSelect );
  QCOMPARE( w.selectionColumnWidth(), 14.0 );
  w.selectionModel()->select( w.model()->index( 1, 0 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionColumnWidth(), -1.0 );
  w.setSelectionColumnWidth( 15.0 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.selectionColumnWidth(), 15.0 );
  QCOMPARE( w.tableColumnWidth( 0 ), 15.0 );
  QCOMPARE( w.tableColumnWidth( 1 ), 15.0 );
  QCOMPARE( w.tableColumnWidth( 2 ), 0.0 );
  w.selectionModel()->select( w.model()->index( 2, 2 ), QItemSelectionModel::Select );
  QCOMPARE( w.selectionColumnWidth(), -1.0 );
}

void TestQgsTableEditor::headers()
{
  QgsTableEditorWidget w;
  QVERIFY( w.tableContents().isEmpty() );

  const QSignalSpy spy( &w, &QgsTableEditorWidget::tableChanged );
  w.setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell( 1 ) << QgsTableCell( 2 ) << QgsTableCell( 3 ) )
                      << ( QgsTableRow() << QgsTableCell( 4 )  << QgsTableCell( 5 ) << QgsTableCell( 6 ) ) );
  QCOMPARE( spy.count(), 1 );

  w.setIncludeTableHeader( true );
  QCOMPARE( w.tableHeaders(), QVariantList() << QVariant() << QVariant() << QVariant() );

  w.setTableHeaders( QVariantList() << QVariant( 11 ) << QVariant( 12 ) << QVariant( 13 ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.tableHeaders(), QVariantList() << QVariant( 11 ) << QVariant( 12 ) << QVariant( 13 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content(), QVariant( 1 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content(), QVariant( 2 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content(), QVariant( 3 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content(), QVariant( 4 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content(), QVariant( 5 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content(), QVariant( 6 ) );

  w.setIncludeTableHeader( false );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.tableHeaders(), QVariantList() );
  QCOMPARE( w.tableContents().at( 0 ).at( 0 ).content(), QVariant( 1 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 1 ).content(), QVariant( 2 ) );
  QCOMPARE( w.tableContents().at( 0 ).at( 2 ).content(), QVariant( 3 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 0 ).content(), QVariant( 4 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 1 ).content(), QVariant( 5 ) );
  QCOMPARE( w.tableContents().at( 1 ).at( 2 ).content(), QVariant( 6 ) );

}


QGSTEST_MAIN( TestQgsTableEditor )
#include "testqgstableeditor.moc"
