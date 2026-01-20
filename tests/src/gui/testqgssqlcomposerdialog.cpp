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


#include "qgslogger.h"
#include "qgssqlcomposerdialog.h"
#include "qgstest.h"

class TestQgsSQLComposerDialog : public QObject
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
  //  QgsDebugMsgLevel( u"Test disabled"_s, 1 );
  //  return false;
  //}
  return true;
}

static QWidget *getQueryEdit( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( u"mQueryEdit"_s );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getColumnsEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( u"mColumnsEditor"_s );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getTablesEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( u"mTablesEditor"_s );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getWhereEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( u"mWhereEditor"_s );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getOrderEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( u"mOrderEditor"_s );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getTablesCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( u"mTablesCombo"_s );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getColumnsCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( u"mColumnsCombo"_s );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getFunctionsCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( u"mFunctionsCombo"_s );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getSpatialPredicatesCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( u"mSpatialPredicatesCombo"_s );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getOperatorsCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( u"mOperatorsCombo"_s );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getResetButton( QgsSQLComposerDialog &d )
{
  QDialogButtonBox *mButtonBox = d.findChild<QDialogButtonBox *>( u"mButtonBox"_s );
  Q_ASSERT( mButtonBox );
  QPushButton *button = mButtonBox->button( QDialogButtonBox::Reset );
  Q_ASSERT( button );
  return button;
}

static QTableWidget *getTableJoins( QgsSQLComposerDialog &d )
{
  QTableWidget *widget = d.findChild<QTableWidget *>( u"mTableJoins"_s );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getAddJoinButton( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( u"mAddJoinButton"_s );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getRemoveJoinButton( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( u"mRemoveJoinButton"_s );
  Q_ASSERT( widget );
  return widget;
}

static void gotoEndOfLine( QWidget *w )
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
  const QString oriSql( u"SELECT a_column FROM my_table JOIN join_table ON cond WHERE where_expr ORDER BY column DESC"_s );
  d.setSql( oriSql );
  QCOMPARE( d.sql(), oriSql );

  gotoEndOfLine( getColumnsEditor( d ) );
  QTest::keyClicks( getColumnsEditor( d ), u", another_column"_s );
  gotoEndOfLine( getTablesEditor( d ) );
  QTest::keyClicks( getTablesEditor( d ), u", another_from_table"_s );
  gotoEndOfLine( getWhereEditor( d ) );
  QTest::keyClicks( getWhereEditor( d ), u" AND another_cond"_s );
  gotoEndOfLine( getOrderEditor( d ) );
  QTest::keyClicks( getOrderEditor( d ), u", another_column_asc"_s );
  QCOMPARE( d.sql(), QString( "SELECT a_column, another_column FROM my_table, another_from_table JOIN join_table ON cond WHERE where_expr AND another_cond ORDER BY column DESC, another_column_asc" ) );

  QTest::mouseClick( getResetButton( d ), Qt::LeftButton );
  QCOMPARE( d.sql(), oriSql );
}

static void setFocusIn( QWidget *widget )
{
  QFocusEvent focusInEvent( QEvent::FocusIn );
  QApplication::sendEvent( widget, &focusInEvent );
}

void TestQgsSQLComposerDialog::testSelectTable()
{
  if ( !runTest() )
    return;
  QgsSQLComposerDialog d;
  d.addTableNames( QStringList() << u"my_table"_s );
  d.addTableNames( QList<QgsSQLComposerDialog::PairNameTitle>() << QgsSQLComposerDialog::PairNameTitle( u"another_table"_s, u"title"_s ) );

  QCOMPARE( getTablesCombo( d )->itemText( 1 ), QString( "my_table" ) );
  QCOMPARE( getTablesCombo( d )->itemText( 2 ), QString( "another_table (title)" ) );

  d.setSql( u"SELECT * FROM "_s );
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
  d.addColumnNames( QList<QgsSQLComposerDialog::PairNameType>() << QgsSQLComposerDialog::PairNameType( u"a"_s, QString() ) << QgsSQLComposerDialog::PairNameType( u"b"_s, u"type"_s ), u"my_table"_s );

  QCOMPARE( getColumnsCombo( d )->itemText( 1 ), QString( "a" ) );
  QCOMPARE( getColumnsCombo( d )->itemText( 2 ), QString( "b (type)" ) );

  d.setSql( u"SELECT "_s );
  gotoEndOfLine( getQueryEdit( d ) );

  // Set focus in SQL zone
  setFocusIn( getQueryEdit( d ) );

  // Select dummy entry in combo
  getColumnsCombo( d )->setCurrentIndex( 0 );

  // Select first entry in combo
  getColumnsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT a" ) );

  gotoEndOfLine( getQueryEdit( d ) );
  QTest::keyClicks( getQueryEdit( d ), u" FROM my_table"_s );

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
    f.name = u"first_func"_s;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = u"second_func"_s;
    f.returnType = u"xs:int"_s;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = u"third_func"_s;
    f.minArgs = 1;
    f.maxArgs = 1;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = u"fourth_func"_s;
    f.minArgs = 1;
    f.maxArgs = 2;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = u"fifth_func"_s;
    f.minArgs = 1;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = u"sixth_func"_s;
    f.argumentList << QgsSQLComposerDialog::Argument( u"arg1"_s, QString() );
    f.argumentList << QgsSQLComposerDialog::Argument( u"arg2"_s, u"xs:double"_s );
    f.argumentList << QgsSQLComposerDialog::Argument( u"arg3"_s, u"gml:AbstractGeometryType"_s );
    f.argumentList << QgsSQLComposerDialog::Argument( u"number"_s, u"xs:int"_s );
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = u"seventh_func"_s;
    f.argumentList << QgsSQLComposerDialog::Argument( u"arg1"_s, QString() );
    f.argumentList << QgsSQLComposerDialog::Argument( u"arg2"_s, u"xs:double"_s );
    f.minArgs = 1;
    functions << f;
  }
  d.addFunctions( functions );

  QCOMPARE( getFunctionsCombo( d )->itemText( 1 ), QString( "first_func()" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 2 ), QString( "second_func(): int" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 3 ), QString( "third_func(1 argument(s))" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 4 ), QString( "fourth_func(1 to 2 argument(s))" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 5 ), QString( "fifth_func(1 argument(s) or more)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 6 ), QString( "sixth_func(arg1, arg2: double, arg3: geometry, int)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 7 ), QString( "seventh_func(arg1[, arg2: double])" ) );

  d.setSql( u"SELECT * FROM my_table"_s );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );

  getFunctionsCombo( d )->setCurrentIndex( 0 );
  getFunctionsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE first_func(" ) );

  // Set focus in SQL zone
  d.setSql( u"SELECT * FROM my_table WHERE "_s );
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
  d.addSpatialPredicates( QList<QgsSQLComposerDialog::Function>() << QgsSQLComposerDialog::Function( u"predicate"_s, 2 ) );

  d.setSql( u"SELECT * FROM my_table"_s );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 0 );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE predicate(" ) );

  // Set focus in SQL zone
  d.setSql( u"SELECT * FROM my_table WHERE "_s );
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

  d.setSql( u"SELECT * FROM my_table"_s );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );
  getOperatorsCombo( d )->setCurrentIndex( 0 );
  getOperatorsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE AND" ) );

  // Set focus in SQL zone
  d.setSql( u"SELECT * FROM my_table WHERE "_s );
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
  d.setSql( u"SELECT * FROM my_table"_s );
  d.setSupportMultipleTables( true );

  QTableWidget *table = getTableJoins( d );
  QCOMPARE( table->rowCount(), 1 );
  QVERIFY( table->item( 0, 0 ) );
  table->item( 0, 0 )->setText( u"join_table"_s );
  table->item( 0, 1 )->setText( u"join_expr"_s );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr" ) );

  QTest::mouseClick( getAddJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 2 );
  table->item( 1, 0 )->setText( u"join2_table"_s );
  table->item( 1, 1 )->setText( u"join2_expr"_s );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr JOIN join2_table ON join2_expr" ) );

  table->setCurrentCell( 0, 0 );
  QTest::mouseClick( getAddJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 3 );
  table->item( 1, 0 )->setText( u"join15_table"_s );
  table->item( 1, 1 )->setText( u"join15_expr"_s );

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
  QVERIFY( table->item( 0, 0 ) );
}

QGSTEST_MAIN( TestQgsSQLComposerDialog )
#include "testqgssqlcomposerdialog.moc"
