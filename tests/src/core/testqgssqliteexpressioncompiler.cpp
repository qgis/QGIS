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
  QgsExpression exp( expString.join( QStringLiteral( ") OR (" ) ).prepend( '(' ).append( ')' ) );
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
  QgsExpression exp( makeExpression( 1 ) );
  QVERIFY( exp.isValid() );
  QCOMPARE( QString( exp ), QString( "((\"Z\" >= 0) AND (\"Bottom\" <= 1))" ) );
  QgsExpression exp2( makeExpression( 2 ) );
  QVERIFY( exp2.isValid() );
  QCOMPARE( QString( exp2 ), QString( "((\"Z\" >= 0) AND (\"Bottom\" <= 1)) OR ((\"Z\" >= 1) AND (\"Bottom\" <= 2))" ) );
}

void TestQgsSQLiteExpressionCompiler::testCompiler()
{
  QgsSQLiteExpressionCompiler compiler = QgsSQLiteExpressionCompiler( mPointsLayer->fields() );
  QgsExpression exp( makeExpression( 1 ) );
  QCOMPARE( compiler.compile( &exp ), QgsSqlExpressionCompiler::Result::Complete );
  QCOMPARE( compiler.result(), QString( exp ) );
  exp = makeExpression( 3 );
  QCOMPARE( compiler.compile( &exp ), QgsSqlExpressionCompiler::Result::Complete );
  // Check that parenthesis matches
  QCOMPARE( compiler.result().count( '(' ),  compiler.result().count( ')' ) );
  QCOMPARE( compiler.result(), QString( "((((\"Z\" >= 0) AND (\"Bottom\" <= 1)) OR ((\"Z\" >= 1) AND (\"Bottom\" <= 2))) OR ((\"Z\" >= 2) AND (\"Bottom\" <= 3)))" ) );
}



QGSTEST_MAIN( TestQgsSQLiteExpressionCompiler )

#include "testqgssqliteexpressioncompiler.moc"
