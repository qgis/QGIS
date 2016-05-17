/***************************************************************************
    testqgssqlcomposerdialog.cpp
     --------------------------------------
    Date                 : April 2016
    Copyright            : (C) 2016 Even Rouault
    Email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtTest/QtTest>

#include <qgslogger.h>
#include <qgssqlcomposerdialog.h>

class TestQgsSQLComposerDialog: public QObject
{
    Q_OBJECT
  private slots:
    void testReciprocalEditorsUpdate();
    void testSelectTable();
    void testSelectColumn();
    void testSelectFunction();
    void testSelectSpatialPredicate();
    void testSelectOperator();
    void testJoins();
  private:
    bool runTest();
};

bool TestQgsSQLComposerDialog::runTest()
{
  // Those tests are fragile because may depend on focus works without
  // widget being displayed. Or shortcuts to go to end of line
  //const char* travis = getenv( "TRAVIS_OS_NAME" );
  //if ( travis && strcmp( travis, "osx" ) == 0 )
  //{
  //  QgsDebugMsg( "Test disabled" );
  //  return false;
  //}
  return true;
}

static QWidget* getQueryEdit( QgsSQLComposerDialog& d )
{
  QWidget* widget = d.findChild<QWidget*>( "mQueryEdit" );
  Q_ASSERT( widget );
  return widget;
}

static QWidget* getColumnsEditor( QgsSQLComposerDialog& d )
{
  QWidget* widget = d.findChild<QWidget*>( "mColumnsEditor" );
  Q_ASSERT( widget );
  return widget;
}

static QWidget* getTablesEditor( QgsSQLComposerDialog& d )
{
  QWidget* widget = d.findChild<QWidget*>( "mTablesEditor" );
  Q_ASSERT( widget );
  return widget;
}

static QWidget* getWhereEditor( QgsSQLComposerDialog& d )
{
  QWidget* widget = d.findChild<QWidget*>( "mWhereEditor" );
  Q_ASSERT( widget );
  return widget;
}

static QWidget* getOrderEditor( QgsSQLComposerDialog& d )
{
  QWidget* widget = d.findChild<QWidget*>( "mOrderEditor" );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox* getTablesCombo( QgsSQLComposerDialog& d )
{
  QComboBox* widget = d.findChild<QComboBox*>( "mTablesCombo" );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox* getColumnsCombo( QgsSQLComposerDialog& d )
{
  QComboBox* widget = d.findChild<QComboBox*>( "mColumnsCombo" );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox* getFunctionsCombo( QgsSQLComposerDialog& d )
{
  QComboBox* widget = d.findChild<QComboBox*>( "mFunctionsCombo" );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox* getSpatialPredicatesCombo( QgsSQLComposerDialog& d )
{
  QComboBox* widget = d.findChild<QComboBox*>( "mSpatialPredicatesCombo" );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox* getOperatorsCombo( QgsSQLComposerDialog& d )
{
  QComboBox* widget = d.findChild<QComboBox*>( "mOperatorsCombo" );
  Q_ASSERT( widget );
  return widget;
}

static QWidget* getResetButton( QgsSQLComposerDialog& d )
{
  QDialogButtonBox* mButtonBox = d.findChild<QDialogButtonBox*>( "mButtonBox" );
  Q_ASSERT( mButtonBox );
  QPushButton* button = mButtonBox->button( QDialogButtonBox::Reset );
  Q_ASSERT( button );
  return button;
}

static QTableWidget* getTableJoins( QgsSQLComposerDialog& d )
{
  QTableWidget* widget = d.findChild<QTableWidget*>( "mTableJoins" );
  Q_ASSERT( widget );
  return widget;
}

static QWidget* getAddJoinButton( QgsSQLComposerDialog& d )
{
  QWidget* widget = d.findChild<QWidget*>( "mAddJoinButton" );
  Q_ASSERT( widget );
  return widget;
}

static QWidget* getRemoveJoinButton( QgsSQLComposerDialog& d )
{
  QWidget* widget = d.findChild<QWidget*>( "mRemoveJoinButton" );
  Q_ASSERT( widget );
  return widget;
}

static void gotoEndOfLine( QWidget* w )
{
#ifdef Q_OS_MAC
  QTest::keyPress( w, Qt::Key_Right, Qt::ControlModifier );
#else
  QTest::keyPress( w, Qt::Key_End );
#endif
}

void TestQgsSQLComposerDialog::testReciprocalEditorsUpdate()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;
  QString oriSql( "SELECT a_column FROM my_table JOIN join_table ON cond WHERE where_expr ORDER BY column DESC" );
  d.setSql( oriSql );
  QCOMPARE( d.sql(), oriSql );

  gotoEndOfLine( getColumnsEditor( d ) );
  QTest::keyClicks( getColumnsEditor( d ), ", another_column" );
  gotoEndOfLine( getTablesEditor( d ) );
  QTest::keyClicks( getTablesEditor( d ), ", another_from_table" );
  gotoEndOfLine( getWhereEditor( d ) );
  QTest::keyClicks( getWhereEditor( d ), " AND another_cond" );
  gotoEndOfLine( getOrderEditor( d ) );
  QTest::keyClicks( getOrderEditor( d ), ", another_column_asc" );
  QCOMPARE( d.sql(), QString( "SELECT a_column, another_column FROM my_table, another_from_table JOIN join_table ON cond WHERE where_expr AND another_cond ORDER BY column DESC, another_column_asc" ) );

  QTest::mouseClick( getResetButton( d ), Qt::LeftButton );
  QCOMPARE( d.sql(), oriSql );
}

static void setFocusIn( QWidget* widget )
{
  QFocusEvent focusInEvent( QEvent::FocusIn );
  QApplication::sendEvent( widget, &focusInEvent );
}

void TestQgsSQLComposerDialog::testSelectTable()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;
  d.addTableNames( QStringList() << "my_table" );
  d.addTableNames( QList<QgsSQLComposerDialog::PairNameTitle>() << QgsSQLComposerDialog::PairNameTitle( "another_table", "title" ) );

  QCOMPARE( getTablesCombo( d )->itemText( 1 ), QString( "my_table" ) );
  QCOMPARE( getTablesCombo( d )->itemText( 2 ), QString( "another_table (title)" ) );

  d.setSql( "SELECT * FROM " );
  gotoEndOfLine( getQueryEdit( d ) );

  // Set focus in SQL zone
  setFocusIn( getQueryEdit( d ) );

  // Select dummy entry in combo
  getTablesCombo( d )->setCurrentIndex( 0 );

  // Select first entry in combo
  getTablesCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table" ) );

  // Set focus in table editor
  setFocusIn( getTablesEditor( d ) );

  // Select second entry in combo
  getTablesCombo( d )->setCurrentIndex( 2 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table, another_table" ) );
}

void TestQgsSQLComposerDialog::testSelectColumn()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;
  d.addColumnNames( QList<QgsSQLComposerDialog::PairNameType>() <<
                    QgsSQLComposerDialog::PairNameType( "a", "" ) <<
                    QgsSQLComposerDialog::PairNameType( "b", "type" ), "my_table" );

  QCOMPARE( getColumnsCombo( d )->itemText( 1 ), QString( "a" ) );
  QCOMPARE( getColumnsCombo( d )->itemText( 2 ), QString( "b (type)" ) );

  d.setSql( "SELECT " );
  gotoEndOfLine( getQueryEdit( d ) );

  // Set focus in SQL zone
  setFocusIn( getQueryEdit( d ) );

  // Select dummy entry in combo
  getColumnsCombo( d )->setCurrentIndex( 0 );

  // Select first entry in combo
  getColumnsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT a" ) );

  gotoEndOfLine( getQueryEdit( d ) );
  QTest::keyClicks( getQueryEdit( d ), " FROM my_table" );

  QCOMPARE( d.sql(), QString( "SELECT a FROM my_table" ) );

  // Set focus in column editor
  setFocusIn( getColumnsEditor( d ) );
  QTest::keyPress( getColumnsEditor( d ), Qt::Key_End );

  // Select second entry in combo
  getColumnsCombo( d )->setCurrentIndex( 2 );

  QCOMPARE( d.sql(), QString( "SELECT a,\nb FROM my_table" ) );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );
  getColumnsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT a,\nb FROM my_table WHERE a" ) );

  // Set focus in order editor
  setFocusIn( getOrderEditor( d ) );
  getColumnsCombo( d )->setCurrentIndex( 2 );

  QCOMPARE( d.sql(), QString( "SELECT a,\nb FROM my_table WHERE a ORDER BY b" ) );
}

void TestQgsSQLComposerDialog::testSelectFunction()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;

  QList<QgsSQLComposerDialog::Function> functions;
  {
    QgsSQLComposerDialog::Function f;
    f.name = "first_func";
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = "second_func";
    f.returnType = "xs:int";
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = "third_func";
    f.minArgs = 1;
    f.maxArgs = 1;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = "fourth_func";
    f.minArgs = 1;
    f.maxArgs = 2;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = "fifth_func";
    f.minArgs = 1;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = "sixth_func";
    f.argumentList << QgsSQLComposerDialog::Argument( "arg1", "" );
    f.argumentList << QgsSQLComposerDialog::Argument( "arg2", "xs:double" );
    f.argumentList << QgsSQLComposerDialog::Argument( "arg3", "gml:AbstractGeometryType" );
    f.argumentList << QgsSQLComposerDialog::Argument( "number", "xs:int" );
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = "seventh_func";
    f.argumentList << QgsSQLComposerDialog::Argument( "arg1", "" );
    f.argumentList << QgsSQLComposerDialog::Argument( "arg2", "xs:double" );
    f.minArgs = 1;
    functions << f;
  }
  d.addFunctions( functions );

  QCOMPARE( getFunctionsCombo( d )->itemText( 1 ), QString( "first_func()" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 2 ), QString( "second_func(): int" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 3 ), QString( "third_func(1 argument)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 4 ), QString( "fourth_func(1 to 2 arguments)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 5 ), QString( "fifth_func(1 argument or more)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 6 ), QString( "sixth_func(arg1, arg2: double, arg3: geometry, int)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 7 ), QString( "seventh_func(arg1[, arg2: double])" ) );

  d.setSql( "SELECT * FROM my_table" );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );

  getFunctionsCombo( d )->setCurrentIndex( 0 );
  getFunctionsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE first_func(" ) );

  // Set focus in SQL zone
  d.setSql( "SELECT * FROM my_table WHERE " );
  setFocusIn( getQueryEdit( d ) );
  gotoEndOfLine( getQueryEdit( d ) );
  getFunctionsCombo( d )->setCurrentIndex( 0 );
  getFunctionsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE first_func(" ) );
}

void TestQgsSQLComposerDialog::testSelectSpatialPredicate()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;
  d.addSpatialPredicates( QList<QgsSQLComposerDialog::Function>() << QgsSQLComposerDialog::Function( "predicate", 2 ) );

  d.setSql( "SELECT * FROM my_table" );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 0 );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE predicate(" ) );

  // Set focus in SQL zone
  d.setSql( "SELECT * FROM my_table WHERE " );
  setFocusIn( getQueryEdit( d ) );
  gotoEndOfLine( getQueryEdit( d ) );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 0 );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE predicate(" ) );
}

void TestQgsSQLComposerDialog::testSelectOperator()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;

  d.setSql( "SELECT * FROM my_table" );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );
  getOperatorsCombo( d )->setCurrentIndex( 0 );
  getOperatorsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE AND" ) );

  // Set focus in SQL zone
  d.setSql( "SELECT * FROM my_table WHERE " );
  setFocusIn( getQueryEdit( d ) );
  gotoEndOfLine( getQueryEdit( d ) );
  getOperatorsCombo( d )->setCurrentIndex( 0 );
  getOperatorsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE AND" ) );
}

void TestQgsSQLComposerDialog::testJoins()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;
  d.setSql( "SELECT * FROM my_table" );
  d.setSupportMultipleTables( true );

  QTableWidget* table = getTableJoins( d );
  QCOMPARE( table->rowCount(), 1 );
  QCOMPARE( table->item( 0, 0 ) != nullptr, true );
  table->item( 0, 0 )->setText( "join_table" );
  table->item( 0, 1 )->setText( "join_expr" );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr" ) );

  QTest::mouseClick( getAddJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 2 );
  table->item( 1, 0 )->setText( "join2_table" );
  table->item( 1, 1 )->setText( "join2_expr" );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr JOIN join2_table ON join2_expr" ) );

  table->setCurrentCell( 0, 0 );
  QTest::mouseClick( getAddJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 3 );
  table->item( 1, 0 )->setText( "join15_table" );
  table->item( 1, 1 )->setText( "join15_expr" );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr JOIN join15_table ON join15_expr JOIN join2_table ON join2_expr" ) );

  table->setCurrentCell( 1, 0 );
  QTest::mouseClick( getRemoveJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 2 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr JOIN join2_table ON join2_expr" ) );

  QTest::mouseClick( getRemoveJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr" ) );

  QTest::mouseClick( getRemoveJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 1 );

  table->setCurrentCell( 0, 0 );
  QTest::mouseClick( getRemoveJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 0 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table" ) );

  QTest::mouseClick( getAddJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 1 );
  QCOMPARE( table->item( 0, 0 ) != nullptr, true );
}

QTEST_MAIN( TestQgsSQLComposerDialog )
#include "testqgssqlcomposerdialog.moc"
