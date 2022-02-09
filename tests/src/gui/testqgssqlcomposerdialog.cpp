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


#include "qgstest.h"

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
  //  QgsDebugMsg( QStringLiteral( "Test disabled" ) );
  //  return false;
  //}
  return true;
}

static QWidget *getQueryEdit( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( QStringLiteral( "mQueryEdit" ) );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getColumnsEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( QStringLiteral( "mColumnsEditor" ) );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getTablesEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( QStringLiteral( "mTablesEditor" ) );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getWhereEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( QStringLiteral( "mWhereEditor" ) );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getOrderEditor( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( QStringLiteral( "mOrderEditor" ) );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getTablesCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( QStringLiteral( "mTablesCombo" ) );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getColumnsCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( QStringLiteral( "mColumnsCombo" ) );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getFunctionsCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( QStringLiteral( "mFunctionsCombo" ) );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getSpatialPredicatesCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( QStringLiteral( "mSpatialPredicatesCombo" ) );
  Q_ASSERT( widget );
  return widget;
}

static QComboBox *getOperatorsCombo( QgsSQLComposerDialog &d )
{
  QComboBox *widget = d.findChild<QComboBox *>( QStringLiteral( "mOperatorsCombo" ) );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getResetButton( QgsSQLComposerDialog &d )
{
  QDialogButtonBox *mButtonBox = d.findChild<QDialogButtonBox *>( QStringLiteral( "mButtonBox" ) );
  Q_ASSERT( mButtonBox );
  QPushButton *button = mButtonBox->button( QDialogButtonBox::Reset );
  Q_ASSERT( button );
  return button;
}

static QTableWidget *getTableJoins( QgsSQLComposerDialog &d )
{
  QTableWidget *widget = d.findChild<QTableWidget *>( QStringLiteral( "mTableJoins" ) );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getAddJoinButton( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( QStringLiteral( "mAddJoinButton" ) );
  Q_ASSERT( widget );
  return widget;
}

static QWidget *getRemoveJoinButton( QgsSQLComposerDialog &d )
{
  QWidget *widget = d.findChild<QWidget *>( QStringLiteral( "mRemoveJoinButton" ) );
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
  const QString oriSql( QStringLiteral( "SELECT a_column FROM my_table JOIN join_table ON cond WHERE where_expr ORDER BY column DESC" ) );
  d.setSql( oriSql );
  QCOMPARE( d.sql(), oriSql );

  gotoEndOfLine( getColumnsEditor( d ) );
  QTest::keyClicks( getColumnsEditor( d ), QStringLiteral( ", another_column" ) );
  gotoEndOfLine( getTablesEditor( d ) );
  QTest::keyClicks( getTablesEditor( d ), QStringLiteral( ", another_from_table" ) );
  gotoEndOfLine( getWhereEditor( d ) );
  QTest::keyClicks( getWhereEditor( d ), QStringLiteral( " AND another_cond" ) );
  gotoEndOfLine( getOrderEditor( d ) );
  QTest::keyClicks( getOrderEditor( d ), QStringLiteral( ", another_column_asc" ) );
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
  d.addTableNames( QStringList() << QStringLiteral( "my_table" ) );
  d.addTableNames( QList<QgsSQLComposerDialog::PairNameTitle>() << QgsSQLComposerDialog::PairNameTitle( QStringLiteral( "another_table" ), QStringLiteral( "title" ) ) );

  QCOMPARE( getTablesCombo( d )->itemText( 1 ), QString( "my_table" ) );
  QCOMPARE( getTablesCombo( d )->itemText( 2 ), QString( "another_table (title)" ) );

  d.setSql( QStringLiteral( "SELECT * FROM " ) );
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
                    QgsSQLComposerDialog::PairNameType( QStringLiteral( "a" ), QString() ) <<
                    QgsSQLComposerDialog::PairNameType( QStringLiteral( "b" ), QStringLiteral( "type" ) ), QStringLiteral( "my_table" ) );

  QCOMPARE( getColumnsCombo( d )->itemText( 1 ), QString( "a" ) );
  QCOMPARE( getColumnsCombo( d )->itemText( 2 ), QString( "b (type)" ) );

  d.setSql( QStringLiteral( "SELECT " ) );
  gotoEndOfLine( getQueryEdit( d ) );

  // Set focus in SQL zone
  setFocusIn( getQueryEdit( d ) );

  // Select dummy entry in combo
  getColumnsCombo( d )->setCurrentIndex( 0 );

  // Select first entry in combo
  getColumnsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT a" ) );

  gotoEndOfLine( getQueryEdit( d ) );
  QTest::keyClicks( getQueryEdit( d ), QStringLiteral( " FROM my_table" ) );

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
    f.name = QStringLiteral( "first_func" );
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = QStringLiteral( "second_func" );
    f.returnType = QStringLiteral( "xs:int" );
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = QStringLiteral( "third_func" );
    f.minArgs = 1;
    f.maxArgs = 1;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = QStringLiteral( "fourth_func" );
    f.minArgs = 1;
    f.maxArgs = 2;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = QStringLiteral( "fifth_func" );
    f.minArgs = 1;
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = QStringLiteral( "sixth_func" );
    f.argumentList << QgsSQLComposerDialog::Argument( QStringLiteral( "arg1" ), QString() );
    f.argumentList << QgsSQLComposerDialog::Argument( QStringLiteral( "arg2" ), QStringLiteral( "xs:double" ) );
    f.argumentList << QgsSQLComposerDialog::Argument( QStringLiteral( "arg3" ), QStringLiteral( "gml:AbstractGeometryType" ) );
    f.argumentList << QgsSQLComposerDialog::Argument( QStringLiteral( "number" ), QStringLiteral( "xs:int" ) );
    functions << f;
  }
  {
    QgsSQLComposerDialog::Function f;
    f.name = QStringLiteral( "seventh_func" );
    f.argumentList << QgsSQLComposerDialog::Argument( QStringLiteral( "arg1" ), QString() );
    f.argumentList << QgsSQLComposerDialog::Argument( QStringLiteral( "arg2" ), QStringLiteral( "xs:double" ) );
    f.minArgs = 1;
    functions << f;
  }
  d.addFunctions( functions );

  QCOMPARE( getFunctionsCombo( d )->itemText( 1 ), QString( "first_func()" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 2 ), QString( "second_func(): int" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 3 ), QString( "third_func(1 argument(s))" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 4 ), QString( "fourth_func(1 to 2 arguments)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 5 ), QString( "fifth_func(1 argument(s) or more)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 6 ), QString( "sixth_func(arg1, arg2: double, arg3: geometry, int)" ) );
  QCOMPARE( getFunctionsCombo( d )->itemText( 7 ), QString( "seventh_func(arg1[, arg2: double])" ) );

  d.setSql( QStringLiteral( "SELECT * FROM my_table" ) );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );

  getFunctionsCombo( d )->setCurrentIndex( 0 );
  getFunctionsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE first_func(" ) );

  // Set focus in SQL zone
  d.setSql( QStringLiteral( "SELECT * FROM my_table WHERE " ) );
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
  d.addSpatialPredicates( QList<QgsSQLComposerDialog::Function>() << QgsSQLComposerDialog::Function( QStringLiteral( "predicate" ), 2 ) );

  d.setSql( QStringLiteral( "SELECT * FROM my_table" ) );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 0 );
  getSpatialPredicatesCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE predicate(" ) );

  // Set focus in SQL zone
  d.setSql( QStringLiteral( "SELECT * FROM my_table WHERE " ) );
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

  d.setSql( QStringLiteral( "SELECT * FROM my_table" ) );

  // Set focus in where editor
  setFocusIn( getWhereEditor( d ) );
  getOperatorsCombo( d )->setCurrentIndex( 0 );
  getOperatorsCombo( d )->setCurrentIndex( 1 );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table WHERE AND" ) );

  // Set focus in SQL zone
  d.setSql( QStringLiteral( "SELECT * FROM my_table WHERE " ) );
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
  d.setSql( QStringLiteral( "SELECT * FROM my_table" ) );
  d.setSupportMultipleTables( true );

  QTableWidget *table = getTableJoins( d );
  QCOMPARE( table->rowCount(), 1 );
  QVERIFY( table->item( 0, 0 ) );
  table->item( 0, 0 )->setText( QStringLiteral( "join_table" ) );
  table->item( 0, 1 )->setText( QStringLiteral( "join_expr" ) );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr" ) );

  QTest::mouseClick( getAddJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 2 );
  table->item( 1, 0 )->setText( QStringLiteral( "join2_table" ) );
  table->item( 1, 1 )->setText( QStringLiteral( "join2_expr" ) );

  QCOMPARE( d.sql(), QString( "SELECT * FROM my_table JOIN join_table ON join_expr JOIN join2_table ON join2_expr" ) );

  table->setCurrentCell( 0, 0 );
  QTest::mouseClick( getAddJoinButton( d ), Qt::LeftButton );
  QCOMPARE( table->rowCount(), 3 );
  table->item( 1, 0 )->setText( QStringLiteral( "join15_table" ) );
  table->item( 1, 1 )->setText( QStringLiteral( "join15_expr" ) );

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
