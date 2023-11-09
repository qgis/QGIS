/***************************************************************************

 testqgssqliteexpressioncompiler.cpp

 ---------------------
 begin                : 14.8.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"


#include <qgsapplication.h>
//header for class being tested
#include "qgsexpression.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgssqliteexpressioncompiler.h"

class TestQgsSQLiteExpressionCompiler: public QObject
{
    Q_OBJECT

  public:

    TestQgsSQLiteExpressionCompiler() = default;

    QgsExpression makeExpression( const int length );


  private slots:

    void initTestCase();
    void cleanupTestCase();
    void testMakeExpression();
    void testCompiler();
    void testPreparedCachedNodes();

  private:

    QgsVectorLayer *mPointsLayer = nullptr;
};



QgsExpression TestQgsSQLiteExpressionCompiler::makeExpression( const int length )
{
  QStringList expString;
  for ( int i = 0; i < length; ++i )
  {
    expString.append( QStringLiteral( "(\"Z\" >= %1) AND (\"Bottom\" <= %2)" ).arg( i ).arg( i + 1 ) );
  }
  const QgsExpression exp( expString.join( QLatin1String( ") OR (" ) ).prepend( '(' ).append( ')' ) );
  return exp;
}

void TestQgsSQLiteExpressionCompiler::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  // Will make sure the settings dir with the style file for color ramp is created
  QgsApplication::createDatabase();
  QgsApplication::showSettings();
  //create a point layer that will be used in all tests...
  mPointsLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=Z:integer&field=Bottom:integer" ), QStringLiteral( "test mem layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( mPointsLayer );
}

void TestQgsSQLiteExpressionCompiler::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsSQLiteExpressionCompiler::testMakeExpression()
{
  const QgsExpression exp( makeExpression( 1 ) );
  QVERIFY( exp.isValid() );
  QCOMPARE( QString( exp ), QString( "((\"Z\" >= 0) AND (\"Bottom\" <= 1))" ) );
  const QgsExpression exp2( makeExpression( 2 ) );
  QVERIFY( exp2.isValid() );
  QCOMPARE( QString( exp2 ), QString( "((\"Z\" >= 0) AND (\"Bottom\" <= 1)) OR ((\"Z\" >= 1) AND (\"Bottom\" <= 2))" ) );
}

void TestQgsSQLiteExpressionCompiler::testCompiler()
{
  QgsSQLiteExpressionCompiler compiler = QgsSQLiteExpressionCompiler( mPointsLayer->fields(), true );
  QgsExpression exp( makeExpression( 1 ) );
  QCOMPARE( compiler.compile( &exp ), QgsSqlExpressionCompiler::Result::Complete );
  QCOMPARE( compiler.result(), QString( exp ) );
  exp = makeExpression( 3 );
  QCOMPARE( compiler.compile( &exp ), QgsSqlExpressionCompiler::Result::Complete );
  // Check that parenthesis matches
  QCOMPARE( compiler.result().count( '(' ),  compiler.result().count( ')' ) );
  QCOMPARE( compiler.result(), QStringLiteral( "((((\"Z\" >= 0) AND (\"Bottom\" <= 1)) OR ((\"Z\" >= 1) AND (\"Bottom\" <= 2))) OR ((\"Z\" >= 2) AND (\"Bottom\" <= 3)))" ) );

  const QgsExpression ilike( QStringLiteral( "'a' ilike 'A'" ) );
  QCOMPARE( compiler.compile( &ilike ), QgsSqlExpressionCompiler::Result::Complete );
  QCOMPARE( compiler.result(), QStringLiteral( "lower('a') LIKE lower('A') ESCAPE '\\'" ) );

  const QgsExpression nilike( QStringLiteral( "'a' not ilike 'A'" ) );
  QCOMPARE( compiler.compile( &nilike ), QgsSqlExpressionCompiler::Result::Complete );
  QCOMPARE( compiler.result(), QStringLiteral( "lower('a') NOT LIKE lower('A') ESCAPE '\\'" ) );

  const QgsExpression nbetween( QStringLiteral( "'b' between 'a' and 'c'" ) );
  QCOMPARE( compiler.compile( &nbetween ), QgsSqlExpressionCompiler::Result::Complete );
  QCOMPARE( compiler.result(), QStringLiteral( "'b' BETWEEN 'a' AND 'c'" ) );
}

void TestQgsSQLiteExpressionCompiler::testPreparedCachedNodes()
{
  // test that expression compilation of an expression which has precalculated static values for nodes will take advantage of these values

  QgsSQLiteExpressionCompiler compiler = QgsSQLiteExpressionCompiler( mPointsLayer->fields(), false );
  QgsExpression exp( QStringLiteral( "\"Z\" = (1 + 2) OR \"z\" < (@static_var + 5)" ) );

  QgsExpressionContext context;
  std::unique_ptr< QgsExpressionContextScope > scope = std::make_unique< QgsExpressionContextScope >();
  scope->setVariable( QStringLiteral( "static_var" ), 10, true );
  context.appendScope( scope.release() );
  // not possible to compile due to use of a variable
  QCOMPARE( compiler.compile( &exp ), QgsSqlExpressionCompiler::Result::Fail );

  // now prepare the expression, so that the static nodes will be precalculated
  exp.prepare( &context );
  // should now succeed -- the variable node was identified as a static value and replaced by a pre-computed value
  QCOMPARE( compiler.compile( &exp ), QgsSqlExpressionCompiler::Result::Complete );
  QCOMPARE( compiler.result(), QStringLiteral( "((\"Z\" = 3) OR (\"Z\" < 15))" ) );

  // let's try again, denying the compiler the ability to use pre-computed values
  QgsSQLiteExpressionCompiler compiler2 = QgsSQLiteExpressionCompiler( mPointsLayer->fields(), true );
  // will fail, because it can't take advantage of the pre-computer variable value and a variable can't be compiled
  QCOMPARE( compiler2.compile( &exp ), QgsSqlExpressionCompiler::Result::Fail );
}


QGSTEST_MAIN( TestQgsSQLiteExpressionCompiler )

#include "testqgssqliteexpressioncompiler.moc"
