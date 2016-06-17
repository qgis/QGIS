/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QtConcurrentMap>
#include <QSharedPointer>

#include <qgsapplication.h>
//header for class being tested
#include <qgsexpression.h>
#include <qgsfeature.h>
#include <qgsfeaturerequest.h>
#include <qgsgeometry.h>
#include <qgsrenderchecker.h>
#include "qgsexpressioncontext.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
#include "qgsproject.h"

static void _parseAndEvalExpr( int arg )
{
  Q_UNUSED( arg );
  for ( int i = 0; i < 100; ++i )
  {
    QgsExpression exp( "1 + 2 * 2" );
    exp.evaluate();
  }
}

class TestQgsExpression: public QObject
{
    Q_OBJECT

  public:

    TestQgsExpression()
        : mPointsLayer( nullptr )
        , mMemoryLayer( nullptr )
        , mAggregatesLayer( nullptr )
        , mChildLayer( nullptr )
    {}

  private:

    QgsVectorLayer* mPointsLayer;
    QgsVectorLayer* mMemoryLayer;
    QgsVectorLayer* mAggregatesLayer;
    QgsVectorLayer* mChildLayer;

  private slots:

    void initTestCase()
    {
      //
      // Runs once before any tests are run
      //
      // init QGIS's paths - true means that all path will be inited from prefix
      QgsApplication::init();
      QgsApplication::initQgis();
      // Will make sure the settings dir with the style file for color ramp is created
      QgsApplication::createDB();
      QgsApplication::showSettings();

      //create a point layer that will be used in all tests...
      QString testDataDir = QString( TEST_DATA_DIR ) + '/';
      QString pointsFileName = testDataDir + "points.shp";
      QFileInfo pointFileInfo( pointsFileName );
      mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                         pointFileInfo.completeBaseName(), "ogr" );
      QgsMapLayerRegistry::instance()->addMapLayer( mPointsLayer );
      mPointsLayer->setTitle( "layer title" );
      mPointsLayer->setAbstract( "layer abstract" );
      mPointsLayer->setKeywordList( "layer,keywords" );
      mPointsLayer->setDataUrl( "data url" );
      mPointsLayer->setAttribution( "layer attribution" );
      mPointsLayer->setAttributionUrl( "attribution url" );
      mPointsLayer->setMaximumScale( 500 );
      mPointsLayer->setMinimumScale( 1000 );

      // test memory layer for get_feature tests
      mMemoryLayer = new QgsVectorLayer( "Point?field=col1:integer&field=col2:string", "test", "memory" );
      QVERIFY( mMemoryLayer->isValid() );
      QgsFeature f1( mMemoryLayer->dataProvider()->fields(), 1 );
      f1.setAttribute( "col1", 10 );
      f1.setAttribute( "col2", "test1" );
      QgsFeature f2( mMemoryLayer->dataProvider()->fields(), 2 );
      f2.setAttribute( "col1", 11 );
      f2.setAttribute( "col2", "test2" );
      QgsFeature f3( mMemoryLayer->dataProvider()->fields(), 3 );
      f3.setAttribute( "col1", 3 );
      f3.setAttribute( "col2", "test3" );
      QgsFeature f4( mMemoryLayer->dataProvider()->fields(), 4 );
      f4.setAttribute( "col1", 41 );
      f4.setAttribute( "col2", "test4" );
      mMemoryLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );
      QgsMapLayerRegistry::instance()->addMapLayer( mMemoryLayer );

      // test layer for aggregates
      mAggregatesLayer = new QgsVectorLayer( "Point?field=col1:integer&field=col2:string&field=col3:integer", "aggregate_layer", "memory" );
      QVERIFY( mAggregatesLayer->isValid() );
      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      af1.setAttribute( "col1", 4 );
      af1.setAttribute( "col2", "test" );
      af1.setAttribute( "col3", 2 );
      QgsFeature af2( mAggregatesLayer->dataProvider()->fields(), 2 );
      af2.setAttribute( "col1", 2 );
      af2.setAttribute( "col2", QVariant( QVariant::String ) );
      af2.setAttribute( "col3", 1 );
      QgsFeature af3( mAggregatesLayer->dataProvider()->fields(), 3 );
      af3.setAttribute( "col1", 3 );
      af3.setAttribute( "col2", "test333" );
      af3.setAttribute( "col3", 2 );
      QgsFeature af4( mAggregatesLayer->dataProvider()->fields(), 4 );
      af4.setAttribute( "col1", 2 );
      af4.setAttribute( "col2", "test4" );
      af4.setAttribute( "col3", 2 );
      QgsFeature af5( mAggregatesLayer->dataProvider()->fields(), 5 );
      af5.setAttribute( "col1", 5 );
      af5.setAttribute( "col2", QVariant( QVariant::String ) );
      af5.setAttribute( "col3", 3 );
      QgsFeature af6( mAggregatesLayer->dataProvider()->fields(), 6 );
      af6.setAttribute( "col1", 8 );
      af6.setAttribute( "col2", "test4" );
      af6.setAttribute( "col3", 3 );
      mAggregatesLayer->dataProvider()->addFeatures( QgsFeatureList() << af1 << af2 << af3 << af4 << af5 << af6 );
      QgsMapLayerRegistry::instance()->addMapLayer( mAggregatesLayer );

      mChildLayer = new QgsVectorLayer( "Point?field=parent:integer&field=col2:string&field=col3:integer", "child_layer", "memory" );
      QVERIFY( mChildLayer->isValid() );
      QgsFeature cf1( mChildLayer->dataProvider()->fields(), 1 );
      cf1.setAttribute( "parent", 4 );
      cf1.setAttribute( "col2", "test" );
      cf1.setAttribute( "col3", 2 );
      QgsFeature cf2( mChildLayer->dataProvider()->fields(), 2 );
      cf2.setAttribute( "parent", 4 );
      cf2.setAttribute( "col2", QVariant( QVariant::String ) );
      cf2.setAttribute( "col3", 1 );
      QgsFeature cf3( mChildLayer->dataProvider()->fields(), 3 );
      cf3.setAttribute( "parent", 4 );
      cf3.setAttribute( "col2", "test333" );
      cf3.setAttribute( "col3", 2 );
      QgsFeature cf4( mChildLayer->dataProvider()->fields(), 4 );
      cf4.setAttribute( "parent", 3 );
      cf4.setAttribute( "col2", "test4" );
      cf4.setAttribute( "col3", 2 );
      QgsFeature cf5( mChildLayer->dataProvider()->fields(), 5 );
      cf5.setAttribute( "parent", 3 );
      cf5.setAttribute( "col2", QVariant( QVariant::String ) );
      cf5.setAttribute( "col3", 7 );
      mChildLayer->dataProvider()->addFeatures( QgsFeatureList() << cf1 << cf2 << cf3 << cf4 << cf5 );
      QgsMapLayerRegistry::instance()->addMapLayer( mChildLayer );

      QgsRelation rel;
      rel.setRelationId( "my_rel" );
      rel.setRelationName( "relation name" );
      rel.setReferencedLayer( mAggregatesLayer->id() );
      rel.setReferencingLayer( mChildLayer->id() );
      rel.addFieldPair( "parent", "col1" );
      QVERIFY( rel.isValid() );
      QgsProject::instance()->relationManager()->addRelation( rel );
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void parsing_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "valid" );

      // invalid strings
      QTest::newRow( "empty string" ) << "" << false;
      QTest::newRow( "invalid character" ) << "@" << false;
      QTest::newRow( "invalid column reference" ) << "my col" << false;
      QTest::newRow( "invalid binary operator" ) << "1+" << false;
      QTest::newRow( "invalid function no params" ) << "cos" << false;
      QTest::newRow( "invalid function not known" ) << "coz(1)" << false;
      QTest::newRow( "invalid operator IN" ) << "n in p" << false;
      QTest::newRow( "empty node list" ) << "1 in ()" << false;
      QTest::newRow( "invalid sqrt params" ) << "sqrt(2,4)" << false;
      QTest::newRow( "special column as function" ) << "$id()" << false;
      QTest::newRow( "unknown special column" ) << "$idx" << false;

      // valid strings
      QTest::newRow( "null" ) << "NULL" << true;
      QTest::newRow( "int literal" ) << "1" << true;
      QTest::newRow( "float literal" ) << "1.23" << true;
      QTest::newRow( "string literal" ) << "'test'" << true;
      QTest::newRow( "column reference" ) << "my_col" << true;
      QTest::newRow( "quoted column" ) << "\"my col\"" << true;
      QTest::newRow( "unary minus" ) << "-(-3)" << true;
      QTest::newRow( "function" ) << "cos(0)" << true;
      QTest::newRow( "function2" ) << "atan2(0,1)" << true;
      QTest::newRow( "operator IN" ) << "n in (a,b)" << true;
      QTest::newRow( "pow" ) << "2 ^ 8" << true;
      QTest::newRow( "$id" ) << "$id + 1" << true;

      QTest::newRow( "arithmetics" ) << "1+2*3" << true;
      QTest::newRow( "logic" ) << "be or not be" << true;

      QTest::newRow( "conditions +1" ) << "case when n then p end" << true;
      QTest::newRow( "conditions +2" ) << "case when n then p else o end" << true;
      QTest::newRow( "conditions +3" ) << "case when n then p when a then b end" << true;
      QTest::newRow( "conditions +4" ) << "case when n then ym when a then b else p end" << true;

      QTest::newRow( "conditions -1" ) << "case end" << false;
      QTest::newRow( "conditions -2" ) << "when n then p" << false;
      QTest::newRow( "conditions -3" ) << "case" << false;
      QTest::newRow( "conditions -4" ) << "case when n p end" << false;
      QTest::newRow( "conditions -5" ) << "case p end" << false;
    }
    void parsing()
    {
      QFETCH( QString, string );
      QFETCH( bool, valid );

      QgsExpression exp( string );

      if ( exp.hasParserError() )
        qDebug() << "Parser error: " << exp.parserErrorString();
      else
        qDebug() << "Parsed string: " << exp.expression();

      QCOMPARE( !exp.hasParserError(), valid );
    }

    void parsing_with_locale()
    {
      // check that parsing of numbers works correctly even when using some other locale

      char* old_locale = setlocale( LC_NUMERIC, nullptr );
      qDebug( "Old locale: %s", old_locale );
      setlocale( LC_NUMERIC, "de_DE.UTF8" );
      char* new_locale = setlocale( LC_NUMERIC, nullptr );
      qDebug( "New locale: %s", new_locale );

      QgsExpression exp( "1.23 + 4.56" );
      QVERIFY( !exp.hasParserError() );

      setlocale( LC_NUMERIC, "" );

      QVariant v = exp.evaluate();
      QCOMPARE( v.toDouble(), 5.79 );
    }

    void alias_data()
    {
      //test function aliases
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QString>( "dump" );
      QTest::addColumn<QVariant>( "result" );

      QTest::newRow( "toint alias" ) << "toint(3.2)" << false << "to_int(3.2)" << QVariant( 3 );
      QTest::newRow( "int to double" ) << "toreal(3)" << false << "to_real(3)" << QVariant( 3. );
      QTest::newRow( "int to text" ) << "tostring(6)" << false << "to_string(6)" << QVariant( "6" );
    }

    void alias()
    {
      QFETCH( QString, string );
      QFETCH( bool, evalError );
      QFETCH( QString, dump );
      QFETCH( QVariant, result );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      if ( res.type() != result.type() )
      {
        qDebug() << "got " << res.typeName() << " instead of " << result.typeName();
      }
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( res, result );
      QCOMPARE( exp.dump(), dump );
    }

    void named_parameter_data()
    {
      //test passing named parameters to functions
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "parserError" );
      QTest::addColumn<QString>( "dump" );
      QTest::addColumn<QVariant>( "result" );

      QTest::newRow( "unsupported" ) << "min( val1:=1, val2:=2, val3:=3 )" << true << "" << QVariant();
      QTest::newRow( "named params named" ) << "clamp( min:=1, value:=2, max:=3)" << false << "clamp(1, 2, 3)" << QVariant( 2.0 );
      QTest::newRow( "named params unnamed" ) << "clamp(1,2,3)" << false << "clamp(1, 2, 3)" << QVariant( 2.0 );
      QTest::newRow( "named params mixed" ) << "clamp( 1, value:=2, max:=3)" << false << "clamp(1, 2, 3)" << QVariant( 2.0 );
      QTest::newRow( "named params mixed bad" ) << "clamp( 1, value:=2, 3)" << true << "" << QVariant();
      QTest::newRow( "named params mixed 2" ) << "clamp( 1, 2, max:=3)" << false << "clamp(1, 2, 3)" << QVariant( 2.0 );
      QTest::newRow( "named params reordered" ) << "clamp( value := 2, max:=3, min:=1)" << false << "clamp(1, 2, 3)" << QVariant( 2.0 );
      QTest::newRow( "named params mixed case" ) << "clamp( Min:=1, vAlUe:=2,MAX:=3)" << false << "clamp(1, 2, 3)" << QVariant( 2.0 );
      QTest::newRow( "named params expression node" ) << "clamp( min:=1*2, value:=2+2, max:=3+1+2)" << false << "clamp(1 * 2, 2 + 2, 3 + 1 + 2)" << QVariant( 4.0 );
      QTest::newRow( "named params bad name" ) << "clamp( min:=1, x:=2, y:=3)" << true << "" << QVariant();
      QTest::newRow( "named params dupe implied" ) << "clamp( 1, 2, value:= 3, max:=4)" << true << "" << QVariant();
      QTest::newRow( "named params dupe explicit" ) << "clamp( 1, value := 2, value:= 3, max:=4)" << true << "" << QVariant();
      QTest::newRow( "named params dupe explicit 2" ) << "clamp( value:=1, value := 2, max:=4)" << true << "" << QVariant();
      QTest::newRow( "named params non optional omitted" ) << "clamp( min:=1, max:=2)" << true << "" << QVariant();
      QTest::newRow( "optional parameters specified" ) << "wordwrap( 'testxstring', 5, 'x')" << false << "wordwrap('testxstring', 5, 'x')" << QVariant( "test\nstring" );
      QTest::newRow( "optional parameters specified named" ) << "wordwrap( text:='testxstring', length:=5, delimiter:='x')" << false << "wordwrap('testxstring', 5, 'x')" << QVariant( "test\nstring" );
      QTest::newRow( "optional parameters unspecified" ) << "wordwrap( text:='test string', length:=5 )" << false << "wordwrap('test string', 5, ' ')" << QVariant( "test\nstring" );
      QTest::newRow( "named params dupe explicit 3" ) << "wordwrap( 'test string', 5, length:=6 )" << true << "" << QVariant();
      QTest::newRow( "named params dupe explicit 4" ) << "wordwrap( text:='test string', length:=5, length:=6 )" << true << "" << QVariant();
    }

    void named_parameter()
    {
      QFETCH( QString, string );
      QFETCH( bool, parserError );
      QFETCH( QString, dump );
      QFETCH( QVariant, result );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), parserError );
      if ( exp.hasParserError() )
      {
        //parser error, so no point continuing testing
        qDebug() << exp.parserErrorString();
        return;
      }

      QgsExpressionContext context;
      Q_ASSERT( exp.prepare( &context ) );

      QVariant res = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      if ( res.type() != result.type() )
      {
        qDebug() << "got " << res.typeName() << " instead of " << result.typeName();
      }
      QCOMPARE( res, result );
      QCOMPARE( exp.dump(), dump );
    }

    void evaluation_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      // literal evaluation
      QTest::newRow( "literal null" ) << "NULL" << false << QVariant();
      QTest::newRow( "literal int" ) << "123" << false << QVariant( 123 );
      QTest::newRow( "literal double" ) << "1.2" << false << QVariant( 1.2 );
      QTest::newRow( "literal text" ) << "'hello'" << false << QVariant( "hello" );
      QTest::newRow( "literal double" ) << ".000001" << false << QVariant( 0.000001 );
      QTest::newRow( "literal double" ) << "1.0e-6" << false << QVariant( 0.000001 );
      QTest::newRow( "literal double" ) << "1e-6" << false << QVariant( 0.000001 );
      QTest::newRow( "literal FALSE" ) << "FALSE" << false << QVariant( false );
      QTest::newRow( "literal TRUE" ) << "TRUE" << false << QVariant( true );

      // unary minus
      QTest::newRow( "unary minus double" ) << "-1.3" << false << QVariant( -1.3 );
      QTest::newRow( "unary minus int" ) << "-1" << false << QVariant( -1 );
      QTest::newRow( "unary minus text" ) << "-'hello'" << true << QVariant();
      QTest::newRow( "unary minus null" ) << "-null" << true << QVariant();

      // arithmetics
      QTest::newRow( "plus int" ) << "1+3" << false << QVariant( 4 );
      QTest::newRow( "plus double" ) << "1+1.3" << false << QVariant( 2.3 );
      QTest::newRow( "plus with null" ) << "null+3" << false << QVariant();
      QTest::newRow( "plus invalid" ) << "1+'foo'" << true << QVariant();

      QTest::newRow( "minus int" ) << "1-3" << false << QVariant( -2 );
      QTest::newRow( "minus nan" ) << "1-'nan'" << true << QVariant();
      QTest::newRow( "minus inf" ) << "1-'inf'" << true << QVariant();
      QTest::newRow( "mul int" ) << "8*7" << false << QVariant( 56 );
      QTest::newRow( "div int" ) << "5/2" << false << QVariant( 2.5 );
      QTest::newRow( "mod int" ) << "20%6" << false << QVariant( 2 );
      QTest::newRow( "minus double" ) << "5.2-3.1" << false << QVariant( 2.1 );
      QTest::newRow( "mul double" ) << "2.1*5" << false << QVariant( 10.5 );
      QTest::newRow( "div double" ) << "11.0/2" << false << QVariant( 5.5 );
      QTest::newRow( "mod double" ) << "6.1 % 2.5" << false << QVariant( 1.1 );
      QTest::newRow( "pow" ) << "2^8" << false << QVariant( 256. );
      QTest::newRow( "division by zero" ) << "1/0" << false << QVariant();
      QTest::newRow( "division by zero" ) << "1.0/0.0" << false << QVariant();
      QTest::newRow( "int division" ) << "5//2" << false << QVariant( 2 );
      QTest::newRow( "int division with doubles" ) << "5.0//2.0" << false << QVariant( 2 );
      QTest::newRow( "negative int division" ) << "-5//2" << false << QVariant( -3 );
      QTest::newRow( "negative int division with doubles" ) << "-5.0//2.0" << false << QVariant( -3 );
      QTest::newRow( "int division by zero" ) << "1//0" << false << QVariant();
      QTest::newRow( "int division by zero with floats" ) << "1.0//0.0" << false << QVariant();

      // comparison
      QTest::newRow( "eq int" ) << "1+1 = 2" << false << QVariant( 1 );
      QTest::newRow( "eq double" ) << "3.2 = 2.2+1" << false << QVariant( 1 );
      QTest::newRow( "eq string" ) << "'a' = 'b'" << false << QVariant( 0 );
      QTest::newRow( "eq null" ) << "2 = null" << false << QVariant();
      QTest::newRow( "eq mixed" ) << "'a' = 1" << false << QVariant( 0 );
      QTest::newRow( "ne int 1" ) << "3 != 4" << false << QVariant( 1 );
      QTest::newRow( "ne int 2" ) << "3 != 3" << false << QVariant( 0 );
      QTest::newRow( "lt int 1" ) << "3 < 4" << false << QVariant( 1 );
      QTest::newRow( "lt int 2" ) << "3 < 3" << false << QVariant( 0 );
      QTest::newRow( "gt int 1" ) << "3 > 4" << false << QVariant( 0 );
      QTest::newRow( "gt int 2" ) << "3 > 3" << false << QVariant( 0 );
      QTest::newRow( "le int 1" ) << "3 <= 4" << false << QVariant( 1 );
      QTest::newRow( "le int 2" ) << "3 <= 3" << false << QVariant( 1 );
      QTest::newRow( "ge int 1" ) << "3 >= 4" << false << QVariant( 0 );
      QTest::newRow( "ge int 2" ) << "3 >= 3" << false << QVariant( 1 );
      QTest::newRow( "lt text 1" ) << "'bar' < 'foo'" << false << QVariant( 1 );
      QTest::newRow( "lt text 2" ) << "'foo' < 'bar'" << false << QVariant( 0 );
      QTest::newRow( "'nan'='nan'" ) << "'nan'='nan'" << false << QVariant( 1 );
      QTest::newRow( "'nan'='x'" ) << "'nan'='x'" << false << QVariant( 0 );
      QTest::newRow( "'inf'='inf'" ) << "'inf'='inf'" << false << QVariant( 1 );
      QTest::newRow( "'inf'='x'" ) << "'inf'='x'" << false << QVariant( 0 );
      QTest::newRow( "'1.1'='1.1'" ) << "'1.1'='1.1'" << false << QVariant( 1 );
      QTest::newRow( "'1.1'!='1.1'" ) << "'1.1'!='1.1'" << false << QVariant( 0 );
      QTest::newRow( "'1.1'='1.10'" ) << "'1.1'='1.10'" << false << QVariant( 0 );
      QTest::newRow( "'1.1'!='1.10'" ) << "'1.1'!='1.10'" << false << QVariant( 1 );
      QTest::newRow( "1.1=1.10" ) << "1.1=1.10" << false << QVariant( 1 );
      QTest::newRow( "1.1 != 1.10" ) << "1.1 != 1.10" << false << QVariant( 0 );
      QTest::newRow( "'1.1'=1.1" ) << "'1.1'=1.1" << false << QVariant( 1 );
      QTest::newRow( "'1.10'=1.1" ) << "'1.10'=1.1" << false << QVariant( 1 );
      QTest::newRow( "1.1='1.10'" ) << "1.1='1.10'" << false << QVariant( 1 );
      QTest::newRow( "'1.1'='1.10000'" ) << "'1.1'='1.10000'" << false << QVariant( 0 );
      QTest::newRow( "'1E-23'='1E-23'" ) << "'1E-23'='1E-23'" << false << QVariant( 1 );
      QTest::newRow( "'1E-23'!='1E-23'" ) << "'1E-23'!='1E-23'" << false << QVariant( 0 );
      QTest::newRow( "'1E-23'='2E-23'" ) << "'1E-23'='2E-23'" << false << QVariant( 0 );
      QTest::newRow( "'1E-23'!='2E-23'" ) << "'1E-23'!='2E-23'" << false << QVariant( 1 );

      // is, is not
      QTest::newRow( "is null,null" ) << "null is null" << false << QVariant( 1 );
      QTest::newRow( "is not null,null" ) << "null is not null" << false << QVariant( 0 );
      QTest::newRow( "is null" ) << "1 is null" << false << QVariant( 0 );
      QTest::newRow( "is not null" ) << "1 is not null" << false << QVariant( 1 );
      QTest::newRow( "is int" ) << "1 is 1" << false << QVariant( 1 );
      QTest::newRow( "is not int" ) << "1 is not 1" << false << QVariant( 0 );
      QTest::newRow( "is text" ) << "'x' is 'y'" << false << QVariant( 0 );
      QTest::newRow( "is not text" ) << "'x' is not 'y'" << false << QVariant( 1 );
      QTest::newRow( "'1.1' is '1.10'" ) << "'1.1' is '1.10'" << false << QVariant( 0 );
      QTest::newRow( "'1.1' is '1.10000'" ) << "'1.1' is '1.10000'" << false << QVariant( 0 );
      QTest::newRow( "1.1 is '1.10'" ) << "1.1 is '1.10'" << false << QVariant( 1 );
      QTest::newRow( "'1.10' is 1.1" ) << "'1.10' is 1.1" << false << QVariant( 1 );

      //  logical
      QTest::newRow( "T or F" ) << "1=1 or 2=3" << false << QVariant( 1 );
      QTest::newRow( "F or F" ) << "1=2 or 2=3" << false << QVariant( 0 );
      QTest::newRow( "T and F" ) << "1=1 and 2=3" << false << QVariant( 0 );
      QTest::newRow( "T and T" ) << "1=1 and 2=2" << false << QVariant( 1 );
      QTest::newRow( "not T" ) << "not 1=1" << false << QVariant( 0 );
      QTest::newRow( "not F" ) << "not 2=3" << false << QVariant( 1 );
      QTest::newRow( "null" ) << "null=1" << false << QVariant();
      QTest::newRow( "U or F" ) << "null=1 or 2=3" << false << QVariant();
      QTest::newRow( "U and F" ) << "null=1 and 2=3" << false << QVariant( 0 );
      QTest::newRow( "invalid and" ) << "'foo' and 2=3" << true << QVariant();
      QTest::newRow( "invalid or" ) << "'foo' or 2=3" << true << QVariant();
      QTest::newRow( "invalid not" ) << "not 'foo'" << true << QVariant();

      // in, not in
      QTest::newRow( "in 1" ) << "1 in (1,2,3)" << false << QVariant( 1 );
      QTest::newRow( "in 2" ) << "1 in (1,null,3)" << false << QVariant( 1 );
      QTest::newRow( "in 3" ) << "1 in (null,2,3)" << false << QVariant();
      QTest::newRow( "in 4" ) << "null in (1,2,3)" << false << QVariant();
      QTest::newRow( "not in 1" ) << "1 not in (1,2,3)" << false << QVariant( 0 );
      QTest::newRow( "not in 2" ) << "1 not in (1,null,3)" << false << QVariant( 0 );
      QTest::newRow( "not in 3" ) << "1 not in (null,2,3)" << false << QVariant();
      QTest::newRow( "not in 4" ) << "null not in (1,2,3)" << false << QVariant();

      // regexp, like
      QTest::newRow( "like 1" ) << "'hello' like '%ll_'" << false << QVariant( 1 );
      QTest::newRow( "like 2" ) << "'hello' like 'lo'" << false << QVariant( 0 );
      QTest::newRow( "like 3" ) << "'hello' like '%LO'" << false << QVariant( 0 );
      QTest::newRow( "ilike" ) << "'hello' ilike '%LO'" << false << QVariant( 1 );
      QTest::newRow( "regexp 1" ) << "'hello' ~ 'll'" << false << QVariant( 1 );
      QTest::newRow( "regexp 2" ) << "'hello' ~ '^ll'" << false << QVariant( 0 );
      QTest::newRow( "regexp 3" ) << "'hello' ~ 'llo$'" << false << QVariant( 1 );

      // concatenation
      QTest::newRow( "concat with plus" ) << "'a' + 'b'" << false << QVariant( "ab" );
      QTest::newRow( "concat" ) << "'a' || 'b'" << false << QVariant( "ab" );
      QTest::newRow( "concat with int" ) << "'a' || 1" << false << QVariant( "a1" );
      QTest::newRow( "concat with int" ) << "2 || 'b'" << false << QVariant( "2b" );
      QTest::newRow( "concat with null" ) << "'a' || null" << false << QVariant();
      QTest::newRow( "concat numbers" ) << "1 || 2" << false << QVariant( "12" );

      // math functions
      QTest::newRow( "pi" ) << "pi()" << false << QVariant( M_PI );
      QTest::newRow( "sqrt" ) << "sqrt(16)" << false << QVariant( 4. );
      QTest::newRow( "sqrt" ) << "sqrt(value:=16)" << false << QVariant( 4. );
      QTest::newRow( "abs(0.1)" ) << "abs(0.1)" << false << QVariant( 0.1 );
      QTest::newRow( "abs(0)" ) << "abs(0)" << false << QVariant( 0. );
      QTest::newRow( "abs( value:=-0.1)" ) << "abs(value:=-0.1)" << false << QVariant( 0.1 );
      QTest::newRow( "invalid sqrt value" ) << "sqrt('a')" << true << QVariant();
      QTest::newRow( "degrees to radians" ) << "toint(radians(degrees:=45)*1000000)" << false << QVariant( 785398 ); // sorry for the nasty hack to work around floating point comparison problems
      QTest::newRow( "radians to degrees" ) << "toint(degrees(radians:=2)*1000)" << false << QVariant( 114592 );
      QTest::newRow( "sin 0" ) << "sin(angle:=0)" << false << QVariant( 0. );
      QTest::newRow( "cos 0" ) << "cos(angle:=0)" << false << QVariant( 1. );
      QTest::newRow( "tan 0" ) << "tan(angle:=0)" << false << QVariant( 0. );
      QTest::newRow( "asin 0" ) << "asin(value:=0)" << false << QVariant( 0. );
      QTest::newRow( "acos 1" ) << "acos(value:=1)" << false << QVariant( 0. );
      QTest::newRow( "atan 0" ) << "atan(value:=0)" << false << QVariant( 0. );
      QTest::newRow( "atan2(0,1)" ) << "atan2(0,1)" << false << QVariant( 0. );
      QTest::newRow( "atan2(1,0)" ) << "atan2(dx:=1,dy:=0)" << false << QVariant( M_PI / 2 );
      QTest::newRow( "exp(0)" ) << "exp(0)" << false << QVariant( 1. );
      QTest::newRow( "exp(1)" ) << "exp(value:=1)" << false << QVariant( exp( 1. ) );
      QTest::newRow( "ln(0)" ) << "ln(0)" << false << QVariant();
      QTest::newRow( "log10(-1)" ) << "log10(-1)" << false << QVariant();
      QTest::newRow( "ln(1)" ) << "ln(value:=1)" << false << QVariant( log( 1. ) );
      QTest::newRow( "log10(100)" ) << "log10(100)" << false << QVariant( 2. );
      QTest::newRow( "log(2,32)" ) << "log(2,32)" << false << QVariant( 5. );
      QTest::newRow( "log(10,1000)" ) << "log(base:=10,value:=1000)" << false << QVariant( 3. );
      QTest::newRow( "log(-2,32)" ) << "log(-2,32)" << false << QVariant();
      QTest::newRow( "log(2,-32)" ) << "log(2,-32)" << false << QVariant();
      QTest::newRow( "log(0.5,32)" ) << "log(0.5,32)" << false << QVariant( -5. );
      QTest::newRow( "round(1234.557,2) - round up" ) << "round(1234.557,2)" << false << QVariant( 1234.56 );
      QTest::newRow( "round(1234.554,2) - round down" ) << "round(1234.554,2)" << false << QVariant( 1234.55 );
      QTest::newRow( "round(1234.6) - round up to int" ) << "round(1234.6)" << false << QVariant( 1235 );
      QTest::newRow( "round(1234.6) - round down to int" ) << "round(1234.4)" << false << QVariant( 1234 );
      QTest::newRow( "max(1)" ) << "max(1)" << false << QVariant( 1. );
      QTest::newRow( "max(1,3.5,-2.1)" ) << "max(1,3.5,-2.1)" << false << QVariant( 3.5 );
      QTest::newRow( "min(-1.5)" ) << "min(-1.5)" << false << QVariant( -1.5 );
      QTest::newRow( "min(-16.6,3.5,-2.1)" ) << "min(-16.6,3.5,-2.1)" << false << QVariant( -16.6 );
      QTest::newRow( "min(5,3.5,-2.1)" ) << "min(5,3.5,-2.1)" << false << QVariant( -2.1 );
      QTest::newRow( "clamp(-2,1,5)" ) << "clamp(-2,1,5)" << false << QVariant( 1.0 );
      QTest::newRow( "clamp(min:=-2,value:=1,max:=5)" ) << "clamp(min:=-2,value:=1,max:=5)" << false << QVariant( 1.0 );
      QTest::newRow( "clamp(-2,-10,5)" ) << "clamp(-2,-10,5)" << false << QVariant( -2.0 );
      QTest::newRow( "clamp(-2,100,5)" ) << "clamp(-2,100,5)" << false << QVariant( 5.0 );
      QTest::newRow( "floor(4.9)" ) << "floor(4.9)" << false << QVariant( 4. );
      QTest::newRow( "floor(-4.9)" ) << "floor(-4.9)" << false << QVariant( -5. );
      QTest::newRow( "ceil(4.9)" ) << "ceil(4.9)" << false << QVariant( 5. );
      QTest::newRow( "ceil(-4.9)" ) << "ceil(-4.9)" << false << QVariant( -4. );
      QTest::newRow( "scale_linear(0.5,0,1,0,1)" ) << "scale_linear(0.5,0,1,0,1)" << false << QVariant( 0.5 );
      QTest::newRow( "scale_linear(0,0,10,100,200)" ) << "scale_linear(0,0,10,100,200)" << false << QVariant( 100. );
      QTest::newRow( "scale_linear(5,0,10,100,200)" ) << "scale_linear(5,0,10,100,200)" << false << QVariant( 150. );
      QTest::newRow( "scale_linear(10,0,10,100,200)" ) << "scale_linear(10,0,10,100,200)" << false << QVariant( 200. );
      QTest::newRow( "scale_linear(-1,0,10,100,200)" ) << "scale_linear(-1,0,10,100,200)" << false << QVariant( 100. );
      QTest::newRow( "scale_linear(11,0,10,100,200)" ) << "scale_linear(11,0,10,100,200)" << false << QVariant( 200. );

      QTest::newRow( "scale_exp(0.5,0,1,0,1,2)" ) << "scale_exp(0.5,0,1,0,1,2)" << false << QVariant( 0.25 );
      QTest::newRow( "scale_exp(0,0,10,100,200,2)" ) << "scale_exp(0,0,10,100,200,2)" << false << QVariant( 100. );
      QTest::newRow( "scale_exp(5,0,10,100,200,2)" ) << "scale_exp(5,0,10,100,200,2)" << false << QVariant( 125. );
      QTest::newRow( "scale_exp(10,0,10,100,200,0.5)" ) << "scale_exp(10,0,10,100,200,0.5)" << false << QVariant( 200. );
      QTest::newRow( "scale_exp(-1,0,10,100,200,0.5)" ) << "scale_exp(-1,0,10,100,200,0.5)" << false << QVariant( 100. );
      QTest::newRow( "scale_exp(4,0,9,0,90,0.5)" ) << "scale_exp(4,0,9,0,90,0.5)" << false << QVariant( 60. );

      // cast functions
      QTest::newRow( "double to int" ) << "toint(3.2)" << false << QVariant( 3 );
      QTest::newRow( "text to int" ) << "toint('53')" << false << QVariant( 53 );
      QTest::newRow( "null to int" ) << "toint(null)" << false << QVariant();
      QTest::newRow( "int to double" ) << "toreal(3)" << false << QVariant( 3. );
      QTest::newRow( "text to double" ) << "toreal('53.1')" << false << QVariant( 53.1 );
      QTest::newRow( "null to double" ) << "toreal(null)" << false << QVariant();
      QTest::newRow( "int to text" ) << "tostring(6)" << false << QVariant( "6" );
      QTest::newRow( "double to text" ) << "tostring(1.23)" << false << QVariant( "1.23" );
      QTest::newRow( "null to text" ) << "tostring(null)" << false << QVariant();

      // geometry functions
      QTest::newRow( "num_points" ) << "num_points(geom_from_wkt('GEOMETRYCOLLECTION(LINESTRING(0 0, 1 0),POINT(6 5))'))" << false << QVariant( 3 );
      QTest::newRow( "num_interior_rings not geom" ) << "num_interior_rings('g')" << true << QVariant();
      QTest::newRow( "num_interior_rings null" ) << "num_interior_rings(NULL)" << false << QVariant();
      QTest::newRow( "num_interior_rings point" ) << "num_interior_rings(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "num_interior_rings polygon" ) << "num_interior_rings(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( 0 );
      QTest::newRow( "num_interior_rings polygon with rings" ) << "num_interior_rings(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( 2 );
      QTest::newRow( "num_interior_rings line" ) << "num_interior_rings(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant();
      QTest::newRow( "num_interior_rings collection no polygon" ) << "num_interior_rings(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'))" << false << QVariant();
      QTest::newRow( "num_interior_rings collection with polygon" ) << "num_interior_rings(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 1), POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1)), POINT(1 0))'))" << false << QVariant( 2 );
      QTest::newRow( "num_rings not geom" ) << "num_rings('g')" << true << QVariant();
      QTest::newRow( "num_rings null" ) << "num_rings(NULL)" << false << QVariant();
      QTest::newRow( "num_rings point" ) << "num_rings(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "num_rings polygon" ) << "num_rings(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( 1 );
      QTest::newRow( "num_rings polygon with rings" ) << "num_rings(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( 3 );
      QTest::newRow( "num_rings line" ) << "num_rings(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant();
      QTest::newRow( "num_rings collection no polygon" ) << "num_rings(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'))" << false << QVariant();
      QTest::newRow( "num_rings collection with polygon" ) << "num_rings(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 1), POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1)), POINT(1 0))'))" << false << QVariant( 3 );
      QTest::newRow( "num_rings collection two polygons" ) << "num_rings(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1)), POINT(0 0), POINT(1 1), POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1)), POINT(1 0))'))" << false << QVariant( 4 );
      QTest::newRow( "num_geometries not geom" ) << "num_geometries('g')" << true << QVariant();
      QTest::newRow( "num_geometries null" ) << "num_geometries(NULL)" << false << QVariant();
      QTest::newRow( "num_geometries point" ) << "num_geometries(geom_from_wkt('POINT(1 2)'))" << false << QVariant( 1 );
      QTest::newRow( "num_geometries polygon" ) << "num_geometries(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( 1 );
      QTest::newRow( "num_geometries line" ) << "num_geometries(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant( 1 );
      QTest::newRow( "num_geometries collection 1" ) << "num_geometries(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'))" << false << QVariant( 4 );
      QTest::newRow( "num_geometries collection 2" ) << "num_geometries(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 1), POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1)), POINT(1 0))'))" << false << QVariant( 5 );
      QTest::newRow( "num_geometries empty collection" ) << "num_geometries(geom_from_wkt('GEOMETRYCOLLECTION()'))" << false << QVariant( 0 );
      QTest::newRow( "nodes_to_points not geom" ) << "nodes_to_points('g')" << true << QVariant();
      QTest::newRow( "nodes_to_points null" ) << "nodes_to_points(NULL)" << false << QVariant();
      QTest::newRow( "nodes_to_points point" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POINT(1 2)')))" << false << QVariant( QString( "MultiPoint ((1 2))" ) );
      QTest::newRow( "nodes_to_points polygon" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( QString( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-1 -1))" ) );
      QTest::newRow( "nodes_to_points polygon with rings" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))')))" << false
      << QVariant( QString( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-1 -1),(-0.1 -0.1),(0.4 0),(0.4 0.2),(0 0.2),(-0.1 -0.1),(-0.3 -0.9),(-0.3 0),(4 -0.1),(0.1 2.1),(-0.3 -0.9))" ) );
      QTest::newRow( "nodes_to_points line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false
      << QVariant( QString( "MultiPoint ((0 0),(1 1),(2 2))" ) );
      QTest::newRow( "nodes_to_points collection 1" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))')))" << false
      << QVariant( QString( "MultiPoint ((0 1),(0 0),(1 0),(1 1))" ) );
      QTest::newRow( "nodes_to_points collection 2" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION(POINTZM(0 1 2 3), POINTZM(0 0 3 4), POINTZM(1 1 5 6), POLYGONZM((-1 -1 7 8, 4 0 1 2, 4 2 7 6, 0 2 1 3, -1 -1 7 8),(-0.1 -0.1 5 4, 0.4 0 9 8, 0.4 0.2 7 10, 0 0.2 0 0, -0.1 -0.1 5 4),(-1 -1 0 0, 4 0 0 1, 4 2 1 2, 0 2 2 3, -1 -1 0 0)), POINTZM(1 0 1 2))')))" << false
      << QVariant( QString( "MultiPointZM ((0 1 2 3),(0 0 3 4),(1 1 5 6),(-1 -1 7 8),(4 0 1 2),(4 2 7 6),(0 2 1 3),(-1 -1 7 8),(-0.1 -0.1 5 4),(0.4 0 9 8),(0.4 0.2 7 10),(0 0.2 0 0),(-0.1 -0.1 5 4),(-1 -1 0 0),(4 0 0 1),(4 2 1 2),(0 2 2 3),(-1 -1 0 0),(1 0 1 2))" ) );
      QTest::newRow( "nodes_to_points empty collection" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION()')))" << false <<
      QVariant( QString( "MultiPoint ()" ) );
      QTest::newRow( "nodes_to_points no close polygon" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'),true))" << false << QVariant( QString( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2))" ) );
      QTest::newRow( "nodes_to_points no close polygon with rings" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))'),true))" << false
      << QVariant( QString( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-0.1 -0.1),(0.4 0),(0.4 0.2),(0 0.2),(-0.3 -0.9),(-0.3 0),(4 -0.1),(0.1 2.1))" ) );
      QTest::newRow( "nodes_to_points no close unclosed line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'),true))" << false
      << QVariant( QString( "MultiPoint ((0 0),(1 1),(2 2))" ) );
      QTest::newRow( "nodes_to_points no close closed line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2, 0 0)'),true))" << false
      << QVariant( QString( "MultiPoint ((0 0),(1 1),(2 2))" ) );
      QTest::newRow( "segments_to_lines not geom" ) << "segments_to_lines('g')" << true << QVariant();
      QTest::newRow( "segments_to_lines null" ) << "segments_to_lines(NULL)" << false << QVariant();
      QTest::newRow( "segments_to_lines point" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POINT(1 2)')))" << false << QVariant( QString( "MultiLineString ()" ) );
      QTest::newRow( "segments_to_lines polygon" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( QString( "MultiLineString ((-1 -1, 4 0),(4 0, 4 2),(4 2, 0 2),(0 2, -1 -1))" ) );
      QTest::newRow( "segments_to_lines polygon with rings" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))')))" << false
      << QVariant( QString( "MultiLineString ((-1 -1, 4 0),(4 0, 4 2),(4 2, 0 2),(0 2, -1 -1),(-0.1 -0.1, 0.4 0),(0.4 0, 0.4 0.2),(0.4 0.2, 0 0.2),(0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0),(-0.3 0, 4 -0.1),(4 -0.1, 0.1 2.1),(0.1 2.1, -0.3 -0.9))" ) );
      QTest::newRow( "segments_to_lines line" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false
      << QVariant( QString( "MultiLineString ((0 0, 1 1),(1 1, 2 2))" ) );
      QTest::newRow( "segments_to_lines collection 1" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))')))" << false
      << QVariant( QString( "MultiLineString ()" ) );
      QTest::newRow( "segments_to_lines collection 2" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION(POINTZM(0 1 2 3), LINESTRINGZM(0 0 1 2, 1 1 3 4, 2 2 5 6), POINTZM(1 1 5 6), POLYGONZM((-1 -1 7 8, 4 0 1 2, 4 2 7 6, 0 2 1 3, -1 -1 7 8)), POINTZM(1 0 1 2))')))" << false
      << QVariant( QString( "MultiLineStringZM ((0 0 1 2, 1 1 3 4),(1 1 3 4, 2 2 5 6),(-1 -1 7 8, 4 0 1 2),(4 0 1 2, 4 2 7 6),(4 2 7 6, 0 2 1 3),(0 2 1 3, -1 -1 7 8))" ) );
      QTest::newRow( "segments_to_lines empty collection" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION()')))" << false <<
      QVariant( QString( "MultiLineString ()" ) );
      QTest::newRow( "length line" ) << "length(geom_from_wkt('LINESTRING(0 0, 4 0)'))" << false << QVariant( 4.0 );
      QTest::newRow( "length polygon" ) << "length(geom_from_wkt('POLYGON((0 0, 4 0, 4 2, 0 2, 0 0))'))" << false << QVariant();
      QTest::newRow( "length point" ) << "length(geom_from_wkt('POINT(0 0)'))" << false << QVariant();
      QTest::newRow( "area polygon" ) << "area(geom_from_wkt('POLYGON((0 0, 4 0, 4 2, 0 2, 0 0))'))" << false << QVariant( 8.0 );
      QTest::newRow( "area line" ) << "area(geom_from_wkt('LINESTRING(0 0, 4 0)'))" << false << QVariant();
      QTest::newRow( "area point" ) << "area(geom_from_wkt('POINT(0 0)'))" << false << QVariant();
      QTest::newRow( "perimeter polygon" ) << "perimeter(geom_from_wkt('POLYGON((0 0, 4 0, 4 2, 0 2, 0 0))'))" << false << QVariant( 12.0 );
      QTest::newRow( "perimeter line" ) << "perimeter(geom_from_wkt('LINESTRING(0 0, 4 0)'))" << false << QVariant();
      QTest::newRow( "perimeter point" ) << "perimeter(geom_from_wkt('POINT(0 0)'))" << false << QVariant();
      QTest::newRow( "point_n point" ) << "geom_to_wkt(point_n(geom_from_wkt('POINT(0 0)'),1))" << false << QVariant( "Point (0 0)" );
      QTest::newRow( "point_n bad index" ) << "geom_to_wkt(point_n(geom_from_wkt('POINT(0 0)'),0))" << true << QVariant();
      QTest::newRow( "point_n bad index" ) << "geom_to_wkt(point_n(geom_from_wkt('POINT(0 0)'),2))" << true << QVariant();
      QTest::newRow( "point_n multipoint" ) << "geom_to_wkt(point_n(geom_from_wkt('MULTIPOINT((0 0), (1 1), (2 2))'),2))" << false << QVariant( "Point (1 1)" );
      QTest::newRow( "point_n line" ) << "geom_to_wkt(point_n(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'),3))" << false << QVariant( "Point (2 2)" );
      QTest::newRow( "point_n polygon" ) << "geom_to_wkt(point_n(geom_from_wkt('POLYGON((0 0, 4 0, 4 2, 0 2, 0 0))'),3))" << false << QVariant( "Point (4 2)" );
      QTest::newRow( "interior_ring_n not geom" ) << "interior_ring_n('g', 1)" << true << QVariant();
      QTest::newRow( "interior_ring_n null" ) << "interior_ring_n(NULL, 1)" << false << QVariant();
      QTest::newRow( "interior_ring_n point" ) << "interior_ring_n(geom_from_wkt('POINT(1 2)'), 1)" << false << QVariant();
      QTest::newRow( "interior_ring_n polygon no rings" ) << "interior_ring_n(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'),1)" << false << QVariant();
      QTest::newRow( "interior_ring_n polygon with rings" ) << "geom_to_wkt(interior_ring_n(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1))'),1))" << false
      << QVariant( QString( "LineString (-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1)" ) );
      QTest::newRow( "interior_ring_n polygon with rings bad index 1" ) << "interior_ring_n(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1))'),0)" << false
      << QVariant();
      QTest::newRow( "interior_ring_n polygon with rings bad index 2" ) << "interior_ring_n(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1))'),3)" << false
      << QVariant();
      QTest::newRow( "interior_ring_n line" ) << "interior_ring_n(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'), 1)" << false << QVariant();
      QTest::newRow( "interior_ring_n collection" ) << "interior_ring_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),1)" << false << QVariant();
      QTest::newRow( "geometry_n not geom" ) << "geometry_n('g', 1)" << true << QVariant();
      QTest::newRow( "geometry_n null" ) << "geometry_n(NULL, 1)" << false << QVariant();
      QTest::newRow( "geometry_n point" ) << "geometry_n(geom_from_wkt('POINT(1 2)'), 1)" << false << QVariant();
      QTest::newRow( "geometry_n polygon" ) << "geometry_n(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'),1)" << false << QVariant();
      QTest::newRow( "geometry_n line" ) << "geometry_n(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'), 1)" << false << QVariant();
      QTest::newRow( "geometry_n collection" ) << "geom_to_wkt(geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),3))" << false << QVariant( QString( "Point (1 0)" ) );
      QTest::newRow( "geometry_n collection bad index 1" ) << "geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),0)" << false << QVariant();
      QTest::newRow( "geometry_n collection bad index 2" ) << "geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),5)" << false << QVariant();
      QTest::newRow( "start_point point" ) << "geom_to_wkt(start_point(geom_from_wkt('POINT(2 0)')))" << false << QVariant( "Point (2 0)" );
      QTest::newRow( "start_point multipoint" ) << "geom_to_wkt(start_point(geom_from_wkt('MULTIPOINT((3 3), (1 1), (2 2))')))" << false << QVariant( "Point (3 3)" );
      QTest::newRow( "start_point line" ) << "geom_to_wkt(start_point(geom_from_wkt('LINESTRING(4 1, 1 1, 2 2)')))" << false << QVariant( "Point (4 1)" );
      QTest::newRow( "start_point polygon" ) << "geom_to_wkt(start_point(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( "Point (-1 -1)" );
      QTest::newRow( "end_point point" ) << "geom_to_wkt(end_point(geom_from_wkt('POINT(2 0)')))" << false << QVariant( "Point (2 0)" );
      QTest::newRow( "end_point multipoint" ) << "geom_to_wkt(end_point(geom_from_wkt('MULTIPOINT((3 3), (1 1), (2 2))')))" << false << QVariant( "Point (2 2)" );
      QTest::newRow( "end_point line" ) << "geom_to_wkt(end_point(geom_from_wkt('LINESTRING(4 1, 1 1, 2 2)')))" << false << QVariant( "Point (2 2)" );
      QTest::newRow( "end_point polygon" ) << "geom_to_wkt(end_point(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( "Point (-1 -1)" );
      QTest::newRow( "reverse not geom" ) << "reverse('g')" << true << QVariant();
      QTest::newRow( "reverse null" ) << "reverse(NULL)" << false << QVariant();
      QTest::newRow( "reverse point" ) << "reverse(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "reverse polygon" ) << "reverse(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant();
      QTest::newRow( "reverse line" ) << "geom_to_wkt(reverse(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false << QVariant( "LineString (2 2, 1 1, 0 0)" );
      QTest::newRow( "exterior_ring not geom" ) << "exterior_ring('g')" << true << QVariant();
      QTest::newRow( "exterior_ring null" ) << "exterior_ring(NULL)" << false << QVariant();
      QTest::newRow( "exterior_ring point" ) << "exterior_ring(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "exterior_ring polygon" ) << "geom_to_wkt(exterior_ring(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),( 0.1 0.1, 0.1 0.2, 0.2 0.2, 0.2, 0.1, 0.1 0.1))')))" << false << QVariant( "LineString (-1 -1, 4 0, 4 2, 0 2, -1 -1)" );
      QTest::newRow( "exterior_ring line" ) << "exterior_ring(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant();
      QTest::newRow( "centroid polygon" ) << "geom_to_wkt(centroid( geomFromWKT('POLYGON((0 0,0 9,9 0,0 0))')))" << false << QVariant( "Point (3 3)" );
      QTest::newRow( "centroid multi polygon" ) << "geom_to_wkt(centroid( geomFromWKT('MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)),((2 0,2 1,3 1,3 0,2 0)))') ))" << false << QVariant( "Point (1.5 0.5)" );
      QTest::newRow( "centroid point" ) << "geom_to_wkt(centroid( geomFromWKT('POINT (1.5 0.5)') ))" << false << QVariant( "Point (1.5 0.5)" );
      QTest::newRow( "centroid line" ) << "geom_to_wkt(centroid( geomFromWKT('LINESTRING (-1 2, 9 12)') ))" << false << QVariant( "Point (4 7)" );
      QTest::newRow( "centroid not geom" ) << "centroid('g')" << true << QVariant();
      QTest::newRow( "centroid null" ) << "centroid(NULL)" << false << QVariant();
      QTest::newRow( "point on surface polygon" ) << "geom_to_wkt(point_on_surface( geomFromWKT('POLYGON((0 0,0 9,9 0,0 0))')))" << false << QVariant( "Point (2.25 4.5)" );
      QTest::newRow( "point on surface multi polygon" ) << "geom_to_wkt(point_on_surface( geomFromWKT('MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)),((2 0,2 1,3 1,3 0,2 0)))') ))" << false << QVariant( "Point (0.5 0.5)" );
      QTest::newRow( "point on surface point" ) << "geom_to_wkt(point_on_surface( geomFromWKT('POINT (1.5 0.5)') ))" << false << QVariant( "Point (1.5 0.5)" );
      QTest::newRow( "point on surface line" ) << "geom_to_wkt(point_on_surface( geomFromWKT('LINESTRING (-1 2, 9 12)') ))" << false << QVariant( "Point (-1 2)" );
      QTest::newRow( "point on surface not geom" ) << "point_on_surface('g')" << true << QVariant();
      QTest::newRow( "point on surface null" ) << "point_on_surface(NULL)" << false << QVariant();
      QTest::newRow( "is_closed not geom" ) << "is_closed('g')" << true << QVariant();
      QTest::newRow( "is_closed null" ) << "is_closed(NULL)" << false << QVariant();
      QTest::newRow( "is_closed point" ) << "is_closed(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "is_closed polygon" ) << "is_closed(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant();
      QTest::newRow( "is_closed not closed" ) << "is_closed(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant( false );
      QTest::newRow( "is_closed closed" ) << "is_closed(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2, 0 0)'))" << false << QVariant( true );
      QTest::newRow( "make_point" ) << "geom_to_wkt(make_point(2.2,4.4))" << false << QVariant( "Point (2.2 4.4)" );
      QTest::newRow( "make_point z" ) << "geom_to_wkt(make_point(2.2,4.4,5.5))" << false << QVariant( "PointZ (2.2 4.4 5.5)" );
      QTest::newRow( "make_point zm" ) << "geom_to_wkt(make_point(2.2,4.4,5.5,6.6))" << false << QVariant( "PointZM (2.2 4.4 5.5 6.6)" );
      QTest::newRow( "make_point bad" ) << "make_point(2.2)" << true << QVariant();
      QTest::newRow( "make_point bad 2" ) << "make_point(2.2, 3, 3, 3, 3)" << true << QVariant();
      QTest::newRow( "make_point_m" ) << "geom_to_wkt(make_point_m(2.2,4.4,5.5))" << false << QVariant( "PointM (2.2 4.4 5.5)" );
      QTest::newRow( "make_line bad" ) << "make_line(make_point(2,4))" << false << QVariant();
      QTest::newRow( "make_line" ) << "geom_to_wkt(make_line(make_point(2,4),make_point(4,6)))" << false << QVariant( "LineString (2 4, 4 6)" );
      QTest::newRow( "make_line" ) << "geom_to_wkt(make_line(make_point(2,4),make_point(4,6),make_point(7,9)))" << false << QVariant( "LineString (2 4, 4 6, 7 9)" );
      QTest::newRow( "make_line" ) << "geom_to_wkt(make_line(make_point(2,4,1,3),make_point(4,6,9,8),make_point(7,9,3,4)))" << false << QVariant( "LineStringZM (2 4 1 3, 4 6 9 8, 7 9 3 4)" );
      QTest::newRow( "make_polygon bad" ) << "make_polygon(make_point(2,4))" << false << QVariant();
      QTest::newRow( "make_polygon" ) << "geom_to_wkt(make_polygon(geom_from_wkt('LINESTRING( 0 0, 0 1, 1 1, 1 0, 0 0 )')))" << false << QVariant( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))" );
      QTest::newRow( "make_polygon rings" ) << "geom_to_wkt(make_polygon(geom_from_wkt('LINESTRING( 0 0, 0 1, 1 1, 1 0, 0 0 )'),geom_from_wkt('LINESTRING( 0.1 0.1, 0.1 0.2, 0.2 0.2, 0.2 0.1, 0.1 0.1 )'),geom_from_wkt('LINESTRING( 0.8 0.8, 0.8 0.9, 0.9 0.9, 0.9 0.8, 0.8 0.8 )')))" << false
      << QVariant( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0),(0.1 0.1, 0.1 0.2, 0.2 0.2, 0.2 0.1, 0.1 0.1),(0.8 0.8, 0.8 0.9, 0.9 0.9, 0.9 0.8, 0.8 0.8))" );
      QTest::newRow( "x point" ) << "x(make_point(2.2,4.4))" << false << QVariant( 2.2 );
      QTest::newRow( "y point" ) << "y(make_point(2.2,4.4))" << false << QVariant( 4.4 );
      QTest::newRow( "z point" ) << "z(make_point(2.2,4.4,6.6))" << false << QVariant( 6.6 );
      QTest::newRow( "z not point" ) << "z(geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'))" << false << QVariant();
      QTest::newRow( "m point" ) << "m(make_point_m(2.2,4.4,7.7))" << false << QVariant( 7.7 );
      QTest::newRow( "m not point" ) << "m(geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'))" << false << QVariant();
      QTest::newRow( "x line" ) << "x(geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'))" << false << QVariant( 2.5 );
      QTest::newRow( "x line" ) << "y(geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'))" << false << QVariant( 1.2 );
      QTest::newRow( "x polygon" ) << "x(geom_from_wkt('POLYGON((2 0,2 2, 3 2, 3 0, 2 0))'))" << false << QVariant( 2.5 );
      QTest::newRow( "x polygon" ) << "y(geom_from_wkt('POLYGON((2 0,2 2, 3 2, 3 0, 2 0))'))" << false << QVariant( 1.0 );
      QTest::newRow( "relate valid" ) << "relate(geom_from_wkt('POINT(110 120)'),geom_from_wkt('POLYGON((60 120,60 40,160 40,160 120,60 120))'))" << false << QVariant( "F0FFFF212" );
      QTest::newRow( "relate bad 1" ) << "relate(geom_from_wkt(''),geom_from_wkt('POLYGON((60 120,60 40,160 40,160 120,60 120))'))" << false << QVariant();
      QTest::newRow( "relate bad 2" ) << "relate(geom_from_wkt('POINT(110 120)'),geom_from_wkt(''))" << false << QVariant();
      QTest::newRow( "relate pattern true" ) << "relate( geom_from_wkt( 'LINESTRING(40 40,120 120)' ), geom_from_wkt( 'LINESTRING(40 40,60 120)' ), '**1F001**' )" << false << QVariant( true );
      QTest::newRow( "relate pattern false" ) << "relate( geom_from_wkt( 'LINESTRING(40 40,120 120)' ), geom_from_wkt( 'LINESTRING(40 40,60 120)' ), '**1F002**' )" << false << QVariant( false );
      QTest::newRow( "azimuth" ) << "toint(degrees(azimuth( point_a := make_point(25, 45), point_b := make_point(75, 100)))*1000000)" << false << QVariant( 42273689 );
      QTest::newRow( "azimuth" ) << "toint(degrees( azimuth( make_point(75, 100), make_point(25,45) ) )*1000000)" << false << QVariant( 222273689 );
      QTest::newRow( "project not geom" ) << "project( 'asd', 1, 2 )" << true << QVariant();
      QTest::newRow( "project not point" ) << "project( geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'), 1, 2 )" << true << QVariant();
      QTest::newRow( "project x" ) << "toint(x(project( make_point( 1, 2 ), 3, radians(270)))*1000000)" << false << QVariant( -2 * 1000000 );
      QTest::newRow( "project y" ) << "toint(y(project( point:=make_point( 1, 2 ), distance:=3, bearing:=radians(270)))*1000000)" << false << QVariant( 2 * 1000000 );
      QTest::newRow( "extrude geom" ) << "geom_to_wkt(extrude( geom_from_wkt('LineString( 1 2, 3 2, 4 3)'),1,2))" << false << QVariant( "Polygon ((1 2, 3 2, 4 3, 5 5, 4 4, 2 4, 1 2))" );
      QTest::newRow( "extrude not geom" ) << "extrude('g',5,6)" << true << QVariant();
      QTest::newRow( "extrude null" ) << "extrude(NULL,5,6)" << false << QVariant();
      QTest::newRow( "order parts" ) << "geom_to_wkt(order_parts(geom_from_wkt('MultiPolygon (((1 1, 5 1, 5 5, 1 5, 1 1)),((1 1, 9 1, 9 9, 1 9, 1 1)))'), 'area($geometry)', False ) )" << false << QVariant( "MultiPolygon (((1 1, 9 1, 9 9, 1 9, 1 1)),((1 1, 5 1, 5 5, 1 5, 1 1)))" );
      QTest::newRow( "order parts not geom" ) << "order_parts('g', 'area($geometry)', False )" << true << QVariant();
      QTest::newRow( "order parts single geom" ) << "geom_to_wkt(order_parts(geom_from_wkt('POLYGON((2 0,2 2, 3 2, 3 0, 2 0))'), 'area($geometry)', False))" << false << QVariant( "Polygon ((2 0, 2 2, 3 2, 3 0, 2 0))" );
      QTest::newRow( "closest_point geom" ) << "geom_to_wkt(closest_point( geom_from_wkt('LineString( 1 1, 5 1, 5 5 )'),geom_from_wkt('Point( 6 3 )')))" << false << QVariant( "Point (5 3)" );
      QTest::newRow( "closest_point not geom" ) << "closest_point('g','b')" << true << QVariant();
      QTest::newRow( "closest_point null" ) << "closest_point(NULL,NULL)" << false << QVariant();
      QTest::newRow( "shortest_line geom" ) << "geom_to_wkt(shortest_line( geom_from_wkt('LineString( 1 1, 5 1, 5 5 )'),geom_from_wkt('Point( 6 3 )')))" << false << QVariant( "LineString (5 3, 6 3)" );
      QTest::newRow( "shortest_line not geom" ) << "shortest_line('g','a')" << true << QVariant();
      QTest::newRow( "shortest_line null" ) << "shortest_line(NULL,NULL)" << false << QVariant();

      // string functions
      QTest::newRow( "lower" ) << "lower('HeLLo')" << false << QVariant( "hello" );
      QTest::newRow( "upper" ) << "upper('HeLLo')" << false << QVariant( "HELLO" );
      QTest::newRow( "length" ) << "length('HeLLo')" << false << QVariant( 5 );
      QTest::newRow( "replace" ) << "replace('HeLLo', 'LL', 'xx')" << false << QVariant( "Hexxo" );
      QTest::newRow( "regexp_replace" ) << "regexp_replace('HeLLo','[eL]+', '-')" << false << QVariant( "H-o" );
      QTest::newRow( "regexp_replace invalid" ) << "regexp_replace('HeLLo','[[[', '-')" << true << QVariant();
      QTest::newRow( "substr" ) << "substr('HeLLo', 3,2)" << false << QVariant( "LL" );
      QTest::newRow( "substr outside" ) << "substr('HeLLo', -5,2)" << false << QVariant( "" );
      QTest::newRow( "regexp_substr" ) << "regexp_substr('abc123','(\\\\d+)')" << false << QVariant( "123" );
      QTest::newRow( "regexp_substr no hit" ) << "regexp_substr('abcdef','(\\\\d+)')" << false << QVariant( "" );
      QTest::newRow( "regexp_substr invalid" ) << "regexp_substr('abc123','([[[')" << true << QVariant();
      QTest::newRow( "strpos" ) << "strpos('Hello World','World')" << false << QVariant( 7 );
      QTest::newRow( "strpos outside" ) << "strpos('Hello World','blah')" << false << QVariant( 0 );
      QTest::newRow( "left" ) << "left('Hello World',5)" << false << QVariant( "Hello" );
      QTest::newRow( "right" ) << "right('Hello World', 5)" << false << QVariant( "World" );
      QTest::newRow( "rpad" ) << "rpad('Hello', 10, 'x')" << false << QVariant( "Helloxxxxx" );
      QTest::newRow( "rpad truncate" ) << "rpad('Hello', 4, 'x')" << false << QVariant( "Hell" );
      QTest::newRow( "lpad" ) << "lpad('Hello', 10, 'x')" << false << QVariant( "xxxxxHello" );
      QTest::newRow( "lpad truncate" ) << "rpad('Hello', 4, 'x')" << false << QVariant( "Hell" );
      QTest::newRow( "title" ) << "title(' HeLlO   WORLD ')" << false << QVariant( " Hello   World " );
      QTest::newRow( "trim" ) << "trim('   Test String ')" << false << QVariant( "Test String" );
      QTest::newRow( "trim empty string" ) << "trim('')" << false << QVariant( "" );
      QTest::newRow( "char" ) << "char(81)" << false << QVariant( "Q" );
      QTest::newRow( "wordwrap" ) << "wordwrap('university of qgis',13)" << false << QVariant( "university of\nqgis" );
      QTest::newRow( "wordwrap" ) << "wordwrap('university of qgis',13,' ')" << false << QVariant( "university of\nqgis" );
      QTest::newRow( "wordwrap" ) << "wordwrap('university of qgis',-3)" << false << QVariant( "university\nof qgis" );
      QTest::newRow( "wordwrap" ) << "wordwrap('university of qgis',-3,' ')" << false << QVariant( "university\nof qgis" );
      QTest::newRow( "wordwrap" ) << "wordwrap('university of qgis\nsupports many multiline',-5,' ')" << false << QVariant( "university\nof qgis\nsupports\nmany multiline" );
      QTest::newRow( "format" ) << "format('%1 %2 %3 %1', 'One', 'Two', 'Three')" << false << QVariant( "One Two Three One" );
      QTest::newRow( "concat" ) << "concat('a', 'b', 'c', 'd')" << false << QVariant( "abcd" );
      QTest::newRow( "concat function single" ) << "concat('a')" << false << QVariant( "a" );
      QTest::newRow( "concat function with NULL" ) << "concat(NULL,'a','b')" << false << QVariant( "ab" );

      //fuzzy matching
      QTest::newRow( "levenshtein" ) << "levenshtein('kitten','sitting')" << false << QVariant( 3 );
      QTest::newRow( "levenshtein" ) << "levenshtein('kitten','kiTTen')" << false << QVariant( 2 );
      QTest::newRow( "levenshtein" ) << "levenshtein('','')" << false << QVariant( 0 );
      QTest::newRow( "longest_common_substring" ) << "longest_common_substring('expression','impression')" << false << QVariant( "pression" );
      QTest::newRow( "longest_common_substring" ) << "longest_common_substring('abCdE','abcde')" << false << QVariant( "ab" );
      QTest::newRow( "longest_common_substring" ) << "longest_common_substring('','')" << false << QVariant( "" );
      QTest::newRow( "hamming_distance" ) << "hamming_distance('abc','xec')" << false << QVariant( 2 );
      QTest::newRow( "hamming_distance" ) << "hamming_distance('abc','ABc')" << false << QVariant( 2 );
      QTest::newRow( "hamming_distance" ) << "hamming_distance('abcd','xec')" << false << QVariant();
      QTest::newRow( "soundex" ) << "soundex('jackson')" << false << QVariant( "J250" );
      QTest::newRow( "soundex" ) << "soundex('')" << false << QVariant( "" );

      // implicit conversions
      QTest::newRow( "implicit int->text" ) << "length(123)" << false << QVariant( 3 );
      QTest::newRow( "implicit double->text" ) << "length(1.23)" << false << QVariant( 4 );
      QTest::newRow( "implicit int->bool" ) << "1 or 0" << false << QVariant( 1 );
      QTest::newRow( "implicit double->bool" ) << "0.1 or 0" << false << QVariant( 1 );
      QTest::newRow( "implicit text->int" ) << "'5'+2" << false << QVariant( 7 );
      QTest::newRow( "implicit text->double" ) << "'5.1'+2" << false << QVariant( 7.1 );
      QTest::newRow( "implicit text->bool" ) << "'0.1' or 0" << false << QVariant( 1 );

      // conditions (without base expression, i.e. CASE WHEN ... THEN ... END)
      QTest::newRow( "condition when" ) << "case when 2>1 then 'good' end" << false << QVariant( "good" );
      QTest::newRow( "condition else" ) << "case when 1=0 then 'bad' else 678 end" << false << QVariant( 678 );
      QTest::newRow( "condition null" ) << "case when length(123)=0 then 111 end" << false << QVariant();
      QTest::newRow( "condition 2 when" ) << "case when 2>3 then 23 when 3>2 then 32 else 0 end" << false << QVariant( 32 );
      QTest::newRow( "coalesce null" ) << "coalesce(NULL)" << false << QVariant();
      QTest::newRow( "coalesce mid-null" ) << "coalesce(1, NULL, 3)" << false << QVariant( 1 );
      QTest::newRow( "coalesce exp" ) << "coalesce(NULL, 1+1)" << false << QVariant( 2 );
      QTest::newRow( "regexp match" ) << "regexp_match('abc','.b.')" << false << QVariant( 1 );
      QTest::newRow( "regexp match invalid" ) << "regexp_match('abc DEF','[[[')" << true << QVariant();
      QTest::newRow( "regexp match escaped" ) << "regexp_match('abc DEF','\\\\s[A-Z]+')" << false << QVariant( 1 );
      QTest::newRow( "regexp match false" ) << "regexp_match('abc DEF','\\\\s[a-z]+')" << false << QVariant( 0 );
      QTest::newRow( "if true" ) << "if(1=1, 1, 0)" << false << QVariant( 1 );
      QTest::newRow( "if false" ) << "if(1=2, 1, 0)" << false << QVariant( 0 );

      // Datetime functions
      QTest::newRow( "to date" ) << "todate('2012-06-28')" << false << QVariant( QDate( 2012, 6, 28 ) );
      QTest::newRow( "to interval" ) << "tointerval('1 Year 1 Month 1 Week 1 Hour 1 Minute')" << false << QVariant::fromValue( QgsInterval( 34758060 ) );
      QTest::newRow( "day with date" ) << "day('2012-06-28')" << false << QVariant( 28 );
      QTest::newRow( "day with interval" ) << "day(tointerval('28 days'))" << false << QVariant( 28.0 );
      QTest::newRow( "month with date" ) << "month('2012-06-28')" << false << QVariant( 6 );
      QTest::newRow( "month with interval" ) << "month(tointerval('2 months'))" << false << QVariant( 2.0 );
      QTest::newRow( "year with date" ) << "year('2012-06-28')" << false << QVariant( 2012 );
      QTest::newRow( "year with interval" ) << "year(tointerval('2 years'))" << false << QVariant( 2.0 );
      QTest::newRow( "age" ) << "age('2012-06-30','2012-06-28')" << false << QVariant::fromValue( QgsInterval( 172800 ) );
      QTest::newRow( "negative age" ) << "age('2012-06-28','2012-06-30')" << false << QVariant::fromValue( QgsInterval( -172800 ) );
      QTest::newRow( "day of week date" ) << "day_of_week(todate('2015-09-21'))" << false << QVariant( 1 );
      QTest::newRow( "day of week datetime" ) << "day_of_week(to_datetime('2015-09-20 13:01:43'))" << false << QVariant( 0 );
      QTest::newRow( "hour datetime" ) << "hour(to_datetime('2015-09-20 13:01:43'))" << false << QVariant( 13 );
      QTest::newRow( "hour time" ) << "hour(to_time('14:01:43'))" << false << QVariant( 14 );
      QTest::newRow( "hour date" ) << "hour(to_date('2004-01-03'))" << true << QVariant();
      QTest::newRow( "hour string" ) << "hour('not a time')" << true << QVariant();
      QTest::newRow( "hour null" ) << "hour(NULL)" << false << QVariant();
      QTest::newRow( "minute datetime" ) << "minute(to_datetime('2015-09-20 13:43:43'))" << false << QVariant( 43 );
      QTest::newRow( "minute time" ) << "minute(to_time('14:22:43'))" << false << QVariant( 22 );
      QTest::newRow( "minute date" ) << "minute(to_date('2004-01-03'))" << true << QVariant();
      QTest::newRow( "minute string" ) << "minute('not a time')" << true << QVariant();
      QTest::newRow( "minute null" ) << "minute(NULL)" << false << QVariant();
      QTest::newRow( "second datetime" ) << "second(to_datetime('2015-09-20 13:43:23'))" << false << QVariant( 23 );
      QTest::newRow( "second time" ) << "second(to_time('14:22:43'))" << false << QVariant( 43 );
      QTest::newRow( "second date" ) << "second(to_date('2004-01-03'))" << true << QVariant();
      QTest::newRow( "second string" ) << "second('not a time')" << true << QVariant();
      QTest::newRow( "second null" ) << "second(NULL)" << false << QVariant();
      QTest::newRow( "age time" ) << "second(age(to_time('08:30:22'),to_time('07:12:10')))" << false << QVariant( 4692.0 );
      QTest::newRow( "age date" ) << "day(age(to_date('2004-03-22'),to_date('2004-03-12')))" << false << QVariant( 10.0 );
      QTest::newRow( "age datetime" ) << "hour(age(to_datetime('2004-03-22 08:30:22'),to_datetime('2004-03-12 07:30:22')))" << false << QVariant( 241.0 );
      QTest::newRow( "date + time" ) << "to_date('2013-03-04') + to_time('13:14:15')" << false << QVariant( QDateTime( QDate( 2013, 3, 4 ), QTime( 13, 14, 15 ) ) );
      QTest::newRow( "time + date" ) << "to_time('13:14:15') + to_date('2013-03-04')" << false << QVariant( QDateTime( QDate( 2013, 3, 4 ), QTime( 13, 14, 15 ) ) );
      QTest::newRow( "date - date" ) << "to_date('2013-03-04') - to_date('2013-03-01')" << false << QVariant( QgsInterval( 3*24*60*60 ) );
      QTest::newRow( "datetime - datetime" ) << "to_datetime('2013-03-04 08:30:00') - to_datetime('2013-03-01 05:15:00')" << false << QVariant( QgsInterval( 3*24*60*60 + 3 * 60*60 + 15*60 ) );
      QTest::newRow( "time - time" ) << "to_time('08:30:00') - to_time('05:15:00')" << false << QVariant( QgsInterval( 3 * 60*60 + 15*60 ) );

      // Color functions
      QTest::newRow( "ramp color" ) << "ramp_color('Spectral',0.3)" << false << QVariant( "254,190,116,255" );
      QTest::newRow( "color rgb" ) << "color_rgb(255,127,0)" << false << QVariant( "255,127,0" );
      QTest::newRow( "color rgba" ) << "color_rgba(255,127,0,200)" << false << QVariant( "255,127,0,200" );
      QTest::newRow( "color hsl" ) << "color_hsl(100,50,70)" << false << QVariant( "166,217,140" );
      QTest::newRow( "color hsla" ) << "color_hsla(100,50,70,200)" << false << QVariant( "166,217,140,200" );
      QTest::newRow( "color hsv" ) << "color_hsv(40,100,100)" << false << QVariant( "255,170,0" );
      QTest::newRow( "color hsva" ) << "color_hsva(40,100,100,200)" << false << QVariant( "255,170,0,200" );
      QTest::newRow( "color cmyk" ) << "color_cmyk(100,50,33,10)" << false << QVariant( "0,115,154" );
      QTest::newRow( "color cmyka" ) << "color_cmyka(50,25,90,60,200)" << false << QVariant( "51,76,10,200" );

      QTest::newRow( "color part bad color" ) << "color_part('notacolor','red')" << true << QVariant();
      QTest::newRow( "color part bad part" ) << "color_part(color_rgb(255,127,0),'bad')" << true << QVariant();
      QTest::newRow( "color part red" ) << "color_part(color_rgba(200,127,150,100),'red')" << false << QVariant( 200 );
      QTest::newRow( "color part green" ) << "color_part(color_rgba(200,127,150,100),'green')" << false << QVariant( 127 );
      QTest::newRow( "color part blue" ) << "color_part(color_rgba(200,127,150,100),'blue')" << false << QVariant( 150 );
      QTest::newRow( "color part alpha" ) << "color_part(color_rgba(200,127,150,100),'alpha')" << false << QVariant( 100 );
      QTest::newRow( "color part hue" ) << "color_part(color_hsv(40,100,80),'hue')" << false << QVariant( 40.0 );
      QTest::newRow( "color part saturation" ) << "color_part(color_hsv(40,100,80),'saturation')" << false << QVariant( 100.0 );
      //some rounding due to conversions between color spaces:
      QTest::newRow( "color part value" ) << "to_int(color_part(color_hsv(40,100,80),'value'))" << false << QVariant( 80 );
      QTest::newRow( "color part hsl_hue" ) << "to_int(color_part(color_hsl(100,50,70),'hsl_hue'))" << false << QVariant( 100 );
      QTest::newRow( "color part hsl_saturation" ) << "to_int(color_part(color_hsl(100,50,70),'hsl_saturation'))" << false << QVariant( 50 );
      QTest::newRow( "color part lightness" ) << "to_int(color_part(color_hsl(100,50,70),'lightness'))" << false << QVariant( 70 );
      QTest::newRow( "color part cyan" ) << "to_int(color_part(color_cmyk(21,0,92,70),'cyan'))" << false << QVariant( 21 );
      QTest::newRow( "color part magenta" ) << "to_int(color_part(color_cmyk(0,10,90,76),'magenta'))" << false << QVariant( 10 );
      QTest::newRow( "color part yellow" ) << "to_int(color_part(color_cmyk(21,0,92,70),'yellow'))" << false << QVariant( 92 );
      QTest::newRow( "color part black" ) << "to_int(color_part(color_cmyk(21,0,92,70),'black'))" << false << QVariant( 70 );
      QTest::newRow( "set color part bad color" ) << "set_color_part('notacolor','red', 5)" << true << QVariant();
      QTest::newRow( "set color part bad part" ) << "set_color_part(color_rgb(255,127,0),'bad', 5)" << true << QVariant();
      QTest::newRow( "set color part red" ) << "color_part(set_color_part(color_rgba(200,127,150,100),'red',100),'red')" << false << QVariant( 100 );
      QTest::newRow( "set color part green" ) << "color_part(set_color_part(color_rgba(200,127,150,100),'green',30),'green')" << false << QVariant( 30 );
      QTest::newRow( "set color part blue" ) << "color_part(set_color_part(color_rgba(200,127,150,100),'blue',120),'blue')" << false << QVariant( 120 );
      QTest::newRow( "set color part alpha" ) << "color_part(set_color_part(color_rgba(200,127,150,100),'alpha',120),'alpha')" << false << QVariant( 120 );
      //some rounding due to conversions between color spaces:
      QTest::newRow( "set color part hue" ) << "to_int(color_part(set_color_part(color_hsv(40,100,80),'hue',30),'hue'))" << false << QVariant( 30 );
      QTest::newRow( "set color part saturation" ) << "to_int(color_part(set_color_part(color_hsv(40,100,80),'saturation',40),'saturation'))" << false << QVariant( 40 );
      QTest::newRow( "set color part value" ) << "to_int(color_part(set_color_part(color_hsv(40,100,80),'value',50),'value'))" << false << QVariant( 50 );
      QTest::newRow( "set color part hsl_hue" ) << "to_int(color_part(set_color_part(color_hsl(100,50,70),'hsl_hue',270),'hsl_hue'))" << false << QVariant( 270 );
      QTest::newRow( "set color part hsl_saturation" ) << "to_int(color_part(set_color_part(color_hsl(100,50,70),'hsl_saturation',30),'hsl_saturation'))" << false << QVariant( 30 );
      QTest::newRow( "set color part lightness" ) << "to_int(color_part(set_color_part(color_hsl(100,50,70),'lightness',20),'lightness'))" << false << QVariant( 20 );
      QTest::newRow( "set color part cyan" ) << "to_int(color_part(set_color_part(color_cmyk(21,0,92,70),'cyan',12),'cyan'))" << false << QVariant( 12 );
      QTest::newRow( "set color part magenta" ) << "to_int(color_part(set_color_part(color_cmyk(0,10,90,76),'magenta',31),'magenta'))" << false << QVariant( 31 );
      QTest::newRow( "set color part yellow" ) << "to_int(color_part(set_color_part(color_cmyk(21,0,92,70),'yellow',96),'yellow'))" << false << QVariant( 96 );
      QTest::newRow( "set color part black" ) << "to_int(color_part(set_color_part(color_cmyk(21,0,92,70),'black',100),'black'))" << false << QVariant( 100 );

      QTest::newRow( "color darker" ) << "darker('200,100,30',150)" << false << QVariant( "133,66,20,255" );
      QTest::newRow( "color darker bad color" ) << "darker('notacolor',150)" << true << QVariant();
      QTest::newRow( "color lighter" ) << "lighter('200,100,30',150)" << false << QVariant( "255,154,83,255" );
      QTest::newRow( "color lighter bad color" ) << "lighter('notacolor',150)" << true << QVariant();

      // Precedence and associativity
      QTest::newRow( "multiplication first" ) << "1+2*3" << false << QVariant( 7 );
      QTest::newRow( "brackets first" ) << "(1+2)*(3+4)" << false << QVariant( 21 );
      QTest::newRow( "right associativity" ) << "(2^3)^2" << false << QVariant( 64. );
      QTest::newRow( "left associativity" ) << "1-(2-1)" << false << QVariant( 0 );

      // layer_property tests
      QTest::newRow( "layer_property no layer" ) << "layer_property('','title')" << false << QVariant();
      QTest::newRow( "layer_property bad layer" ) << "layer_property('bad','title')" << false << QVariant();
      QTest::newRow( "layer_property no property" ) << QString( "layer_property('%1','')" ).arg( mPointsLayer->name() ) << false << QVariant();
      QTest::newRow( "layer_property bad property" ) << QString( "layer_property('%1','bad')" ).arg( mPointsLayer->name() ) << false << QVariant();
      QTest::newRow( "layer_property by id" ) << QString( "layer_property('%1','name')" ).arg( mPointsLayer->id() ) << false << QVariant( mPointsLayer->name() );
      QTest::newRow( "layer_property name" ) << QString( "layer_property('%1','name')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->name() );
      QTest::newRow( "layer_property id" ) << QString( "layer_property('%1','id')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->id() );
      QTest::newRow( "layer_property title" ) << QString( "layer_property('%1','title')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->title() );
      QTest::newRow( "layer_property abstract" ) << QString( "layer_property('%1','abstract')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->abstract() );
      QTest::newRow( "layer_property keywords" ) << QString( "layer_property('%1','keywords')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->keywordList() );
      QTest::newRow( "layer_property data_url" ) << QString( "layer_property('%1','data_url')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->dataUrl() );
      QTest::newRow( "layer_property attribution" ) << QString( "layer_property('%1','attribution')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->attribution() );
      QTest::newRow( "layer_property attribution_url" ) << QString( "layer_property('%1','attribution_url')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->attributionUrl() );
      QTest::newRow( "layer_property source" ) << QString( "layer_property('%1','source')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->publicSource() );
      QTest::newRow( "layer_property min_scale" ) << QString( "layer_property('%1','min_scale')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->minimumScale() );
      QTest::newRow( "layer_property max_scale" ) << QString( "layer_property('%1','max_scale')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->maximumScale() );
      QTest::newRow( "layer_property crs" ) << QString( "layer_property('%1','crs')" ).arg( mPointsLayer->name() ) << false << QVariant( "EPSG:4326" );
      QTest::newRow( "layer_property extent" ) << QString( "geom_to_wkt(layer_property('%1','extent'))" ).arg( mPointsLayer->name() ) << false << QVariant( "Polygon ((-118.88888889 22.80020704, -83.33333333 22.80020704, -83.33333333 46.87198068, -118.88888889 46.87198068, -118.88888889 22.80020704))" );
      QTest::newRow( "layer_property type" ) << QString( "layer_property('%1','type')" ).arg( mPointsLayer->name() ) << false << QVariant( "Vector" );
      QTest::newRow( "layer_property storage_type" ) << QString( "layer_property('%1','storage_type')" ).arg( mPointsLayer->name() ) << false << QVariant( "ESRI Shapefile" );
      QTest::newRow( "layer_property geometry_type" ) << QString( "layer_property('%1','geometry_type')" ).arg( mPointsLayer->name() ) << false << QVariant( "Point" );

      //test conversions to bool
      QTest::newRow( "feature to bool false" ) << QString( "case when get_feature('none','none',499) then true else false end" ) << false << QVariant( false );
      QTest::newRow( "feature to bool true" ) << QString( "case when get_feature('test','col1',10) then true else false end" ) << false << QVariant( true );
      QTest::newRow( "geometry to bool false" ) << QString( "case when geom_from_wkt('') then true else false end" ) << false << QVariant( false );
      QTest::newRow( "geometry to bool true" ) << QString( "case when geom_from_wkt('Point(3 4)') then true else false end" ) << false << QVariant( true );

      // is not
      QTest::newRow( "1 is (not 2)" ) << QString( "1 is (not 2)" ) << false << QVariant( 0 );
      QTest::newRow( "1 is not 2" ) << QString( "1 is not 2" ) << false << QVariant( 1 );
      QTest::newRow( "1 is  not 2" ) << QString( "1 is  not 2" ) << false << QVariant( 1 );

      // not like
      QTest::newRow( "'a' not like 'a%'" ) << QString( "'a' not like 'a%'" ) << false << QVariant( 0 );
      QTest::newRow( "'a' not  like 'a%'" ) << QString( "'a' not  like 'a%'" ) << false << QVariant( 0 );
    }

    void run_evaluation_test( QgsExpression& exp, bool evalError, QVariant& expected )
    {
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant result = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      if ( result.type() != expected.type() )
      {
        qDebug() << "got " << result.typeName() << " instead of " << expected.typeName();
      }
      //qDebug() << res.type() << " " << result.type();
      //qDebug() << "type " << res.typeName();
      QCOMPARE( exp.hasEvalError(), evalError );

      QgsExpressionContext context;

      Q_ASSERT( exp.prepare( &context ) );

      QCOMPARE( result.type(), expected.type() );
      switch ( result.type() )
      {
        case QVariant::Invalid:
          break; // nothing more to check
        case QVariant::Int:
          QCOMPARE( result.toInt(), expected.toInt() );
          break;
        case QVariant::Double:
          QCOMPARE( result.toDouble(), expected.toDouble() );
          break;
        case QVariant::Bool:
          QCOMPARE( result.toBool(), expected.toBool() );
          break;
        case QVariant::String:
          QCOMPARE( result.toString(), expected.toString() );
          break;
        case QVariant::Date:
          QCOMPARE( result.toDate(), expected.toDate() );
          break;
        case QVariant::DateTime:
          QCOMPARE( result.toDateTime(), expected.toDateTime() );
          break;
        case QVariant::Time:
          QCOMPARE( result.toTime(), expected.toTime() );
          break;
        case QVariant::UserType:
        {
          if ( result.userType() == qMetaTypeId<QgsInterval>() )
          {
            QgsInterval inter = result.value<QgsInterval>();
            QgsInterval gotinter = expected.value<QgsInterval>();
            QCOMPARE( inter.seconds(), gotinter.seconds() );
          }
          else
          {
            QFAIL( "unexpected user type" );
          }
          break;
        }
        default:
          Q_ASSERT( false ); // should never happen
      }
    }

    void evaluation()
    {
      QFETCH( QString, string );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpression exp( string );
      run_evaluation_test( exp, evalError, result );
      QgsExpression exp2( exp.dump() );
      run_evaluation_test( exp2, evalError, result );
      QgsExpression exp3( exp.expression() );
      run_evaluation_test( exp3, evalError, result );
      QgsExpression exp4( exp );
      run_evaluation_test( exp4, evalError, result );
    }

    void eval_precedence()
    {
      QCOMPARE( QgsExpression::BinaryOperatorText[QgsExpression::boDiv], "/" );
      QCOMPARE( QgsExpression::BinaryOperatorText[QgsExpression::boConcat], "||" );
    }

    void eval_columns()
    {
      QgsFields fields;
      fields.append( QgsField( "x1" ) );
      fields.append( QgsField( "x2" ) );
      fields.append( QgsField( "foo", QVariant::Int ) );

      QgsFeature f;
      f.initAttributes( 3 );
      f.setAttribute( 2, QVariant( 20 ) );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );

      // good exp
      QgsExpression exp( "foo + 1" );
      bool prepareRes = exp.prepare( &context );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp.hasEvalError(), false );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( res.type(), QVariant::Int );
      QCOMPARE( res.toInt(), 21 );

      // bad exp
      QgsExpression exp2( "bar + 1" );
      bool prepareRes2 = exp2.prepare( &context );
      QCOMPARE( prepareRes2, false );
      QCOMPARE( exp2.hasEvalError(), true );
      QVariant res2 = exp2.evaluate( &context );
      QCOMPARE( res2.type(), QVariant::Invalid );
    }

    void eval_rownum()
    {
      QgsExpression exp( "$rownum + 1" );
      QVariant v1 = exp.evaluate();
      QCOMPARE( v1.toInt(), 1 );

      Q_NOWARN_DEPRECATED_PUSH
      exp.setCurrentRowNumber( 100 );
      Q_NOWARN_DEPRECATED_POP
      QVariant v2 = exp.evaluate();
      QCOMPARE( v2.toInt(), 101 );

      QgsExpressionContext context;
      context << new QgsExpressionContextScope();
      context.lastScope()->setVariable( "row_number", 101 );
      QVariant v3 = exp.evaluate();
      QCOMPARE( v3.toInt(), 101 );
    }

    void eval_scale()
    {
      QgsExpression exp( "$scale" );
      QVariant v1 = exp.evaluate();
      QCOMPARE( v1.toInt(), 0 );

      exp.setScale( 100.00 );
      QVariant v2 = exp.evaluate();
      QCOMPARE( v2.toDouble(), 100.00 );
    }

    void eval_feature_id()
    {
      QgsFeature f( 100 );
      QgsExpression exp( "$id * 2" );
      Q_NOWARN_DEPRECATED_PUSH
      QVariant v = exp.evaluate( &f );
      Q_NOWARN_DEPRECATED_POP
      QCOMPARE( v.toInt(), 200 );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v2 = exp.evaluate( &context );
      QCOMPARE( v2.toInt(), 200 );
    }

    void eval_current_feature()
    {
      QgsFeature f( 100 );
      QgsExpression exp( "$currentfeature" );
      Q_NOWARN_DEPRECATED_PUSH
      QVariant v = exp.evaluate( &f );
      Q_NOWARN_DEPRECATED_POP
      QgsFeature evalFeature = v.value<QgsFeature>();
      QCOMPARE( evalFeature.id(), f.id() );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      v = exp.evaluate( &context );
      evalFeature = v.value<QgsFeature>();
      QCOMPARE( evalFeature.id(), f.id() );
    }

    void eval_feature_attribute()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( "col1" ) );
      fields.append( QgsField( "second_column", QVariant::Int ) );
      f.setFields( fields, true );
      f.setAttribute( QString( "col1" ), QString( "test value" ) );
      f.setAttribute( QString( "second_column" ), 5 );
      QgsExpression exp( "attribute($currentfeature,'col1')" );
      Q_NOWARN_DEPRECATED_PUSH
      QVariant v = exp.evaluate( &f );
      QCOMPARE( v.toString(), QString( "test value" ) );
      QgsExpression exp2( "attribute($currentfeature,'second'||'_column')" );
      v = exp2.evaluate( &f );
      Q_NOWARN_DEPRECATED_POP
      QCOMPARE( v.toInt(), 5 );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      v = exp.evaluate( &context );
      QCOMPARE( v.toString(), QString( "test value" ) );
      v = exp2.evaluate( &context );
      QCOMPARE( v.toInt(), 5 );
    }

    void eval_get_feature_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "featureMatched" );
      QTest::addColumn<int>( "featureId" );

      // get_feature evaluation

      //by layer name
      QTest::newRow( "get_feature 1" ) << "get_feature('test','col1',10)" << true << 1;
      QTest::newRow( "get_feature 2" ) << "get_feature('test','col2','test1')" << true << 1;
      QTest::newRow( "get_feature 3" ) << "get_feature('test','col1',11)" << true << 2;
      QTest::newRow( "get_feature 4" ) << "get_feature('test','col2','test2')" << true << 2;
      QTest::newRow( "get_feature 5" ) << "get_feature('test','col1',3)" << true << 3;
      QTest::newRow( "get_feature 6" ) << "get_feature('test','col2','test3')" << true << 3;
      QTest::newRow( "get_feature 7" ) << "get_feature('test','col1',41)" << true << 4;
      QTest::newRow( "get_feature 8" ) << "get_feature('test','col2','test4')" << true << 4;

      //by layer id
      QTest::newRow( "get_feature 3" ) << QString( "get_feature('%1','col1',11)" ).arg( mMemoryLayer->id() ) << true << 2;
      QTest::newRow( "get_feature 4" ) << QString( "get_feature('%1','col2','test2')" ).arg( mMemoryLayer->id() ) << true << 2;

      //no matching features
      QTest::newRow( "get_feature no match1" ) << "get_feature('test','col1',499)" << false << -1;
      QTest::newRow( "get_feature no match2" ) << "get_feature('test','col2','no match!')" << false << -1;
      //no matching layer
      QTest::newRow( "get_feature no match layer" ) << "get_feature('not a layer!','col1',10)" << false << -1;
    }

    void eval_get_feature()
    {
      QFETCH( QString, string );
      QFETCH( bool, featureMatched );
      QFETCH( int, featureId );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();

      QCOMPARE( exp.hasEvalError(), false );
      QCOMPARE( res.canConvert<QgsFeature>(), featureMatched );
      if ( featureMatched )
      {
        QgsFeature feat = res.value<QgsFeature>();
        QCOMPARE( feat.id(), ( long long )featureId );
      }
    }

    void aggregate_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QTest::newRow( "bad layer" ) << "aggregate('xxxtest','sum',\"col1\")" << true << QVariant();
      QTest::newRow( "bad aggregate" ) << "aggregate('test','xxsum',\"col1\")" << true << QVariant();
      QTest::newRow( "bad expression" ) << "aggregate('test','sum',\"xcvxcvcol1\")" << true << QVariant();

      QTest::newRow( "int aggregate 1" ) << "aggregate('test','sum',\"col1\")" << false << QVariant( 65 );
      QTest::newRow( "int aggregate 2" ) << "aggregate('test','max',\"col1\")" << false << QVariant( 41 );
      QTest::newRow( "int aggregate named" ) << "aggregate(layer:='test',aggregate:='sum',expression:=\"col1\")" << false << QVariant( 65 );
      QTest::newRow( "string aggregate on int" ) << "aggregate('test','max_length',\"col1\")" << true << QVariant();
      QTest::newRow( "string aggregate 1" ) << "aggregate('test','min',\"col2\")" << false << QVariant( "test1" );
      QTest::newRow( "string aggregate 2" ) << "aggregate('test','min_length',\"col2\")" << false << QVariant( 5 );
      QTest::newRow( "string concatenate" ) << "aggregate('test','concatenate',\"col2\",concatenator:=' , ')" << false << QVariant( "test1 , test2 , test3 , test4" );

      QTest::newRow( "sub expression" ) << "aggregate('test','sum',\"col1\" * 2)" << false << QVariant( 65 * 2 );
      QTest::newRow( "bad sub expression" ) << "aggregate('test','sum',\"xcvxcv\" * 2)" << true << QVariant();

      QTest::newRow( "filter" ) << "aggregate('test','sum',\"col1\", \"col1\" <= 10)" << false << QVariant( 13 );
      QTest::newRow( "filter context" ) << "aggregate('test','sum',\"col1\", \"col1\" <= @test_var)" << false << QVariant( 13 );
      QTest::newRow( "filter named" ) << "aggregate(layer:='test',aggregate:='sum',expression:=\"col1\", filter:=\"col1\" <= 10)" << false << QVariant( 13 );
      QTest::newRow( "filter no matching" ) << "aggregate('test','sum',\"col1\", \"col1\" <= -10)" << false << QVariant( 0 );
    }

    void aggregate()
    {
      QgsExpressionContext context;
      QgsExpressionContextScope* scope = new QgsExpressionContextScope();
      scope->setVariable( "test_var", 10 );
      context << scope;

      QFETCH( QString, string );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res;

      //try evaluating once without context (only if variables aren't required)
      if ( !string.contains( "@" ) )
      {
        res = exp.evaluate();
        if ( exp.hasEvalError() )
          qDebug() << exp.evalErrorString();

        QCOMPARE( exp.hasEvalError(), evalError );
        QCOMPARE( res, result );
      }

      //try evaluating with context
      res = exp.evaluate( &context );
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();

      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( res, result );

      // check again - make sure value was correctly cached
      res = exp.evaluate( &context );
      QCOMPARE( res, result );
    }

    void layerAggregates_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QTest::newRow( "count" ) << "count(\"col1\")" << false << QVariant( 6.0 );
      QTest::newRow( "count_distinct" ) << "count_distinct(\"col1\")" << false << QVariant( 5.0 );
      QTest::newRow( "count_missing" ) << "count_missing(\"col2\")" << false << QVariant( 2 );
      QTest::newRow( "sum" ) << "sum(\"col1\")" << false << QVariant( 24.0 );
      QTest::newRow( "minimum" ) << "minimum(\"col1\")" << false << QVariant( 2.0 );
      QTest::newRow( "maximum" ) << "maximum(\"col1\")" << false << QVariant( 8.0 );
      QTest::newRow( "mean" ) << "mean(\"col1\")" << false << QVariant( 4.0 );
      QTest::newRow( "median" ) << "median(\"col1\")" << false << QVariant( 3.5 );
      QTest::newRow( "stdev" ) << "round(stdev(\"col1\")*10000)" << false << QVariant( 22804 );
      QTest::newRow( "range" ) << "range(\"col1\")" << false << QVariant( 6.0 );
      QTest::newRow( "minority" ) << "minority(\"col3\")" << false << QVariant( 1 );
      QTest::newRow( "majority" ) << "majority(\"col3\")" << false << QVariant( 2 );
      QTest::newRow( "q1" ) << "q1(\"col1\")" << false << QVariant( 2 );
      QTest::newRow( "q3" ) << "q3(\"col1\")" << false << QVariant( 5 );
      QTest::newRow( "iqr" ) << "iqr(\"col1\")" << false << QVariant( 3 );
      QTest::newRow( "min_length" ) << "min_length(\"col2\")" << false << QVariant( 0 );
      QTest::newRow( "max_length" ) << "max_length(\"col2\")" << false << QVariant( 7 );
      QTest::newRow( "concatenate" ) << "concatenate(\"col2\",concatenator:=',')" << false << QVariant( "test,,test333,test4,,test4" );

      QTest::newRow( "bad expression" ) << "sum(\"xcvxcvcol1\")" << true << QVariant();
      QTest::newRow( "aggregate named" ) << "sum(expression:=\"col1\")" << false << QVariant( 24.0 );
      QTest::newRow( "string aggregate on int" ) << "max_length(\"col1\")" << true << QVariant();

      QTest::newRow( "sub expression" ) << "sum(\"col1\" * 2)" << false << QVariant( 48 );
      QTest::newRow( "bad sub expression" ) << "sum(\"xcvxcv\" * 2)" << true << QVariant();

      QTest::newRow( "filter" ) << "sum(\"col1\", NULL, \"col1\" >= 5)" << false << QVariant( 13 );
      QTest::newRow( "filter named" ) << "sum(expression:=\"col1\", filter:=\"col1\" >= 5)" << false << QVariant( 13 );
      QTest::newRow( "filter no matching" ) << "sum(expression:=\"col1\", filter:=\"col1\" <= -5)" << false << QVariant( 0 );

      QTest::newRow( "group by" ) << "sum(\"col1\", \"col3\")" << false << QVariant( 9 );
      QTest::newRow( "group by and filter" ) << "sum(\"col1\", \"col3\", \"col1\">=3)" << false << QVariant( 7 );
      QTest::newRow( "group by and filter named" ) << "sum(expression:=\"col1\", group_by:=\"col3\", filter:=\"col1\">=3)" << false << QVariant( 7 );
      QTest::newRow( "group by expression" ) << "sum(\"col1\", \"col1\" % 2)" << false << QVariant( 16 );
    }

    void layerAggregates()
    {
      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::layerScope( mAggregatesLayer ) );

      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      af1.setAttribute( "col1", 4 );
      af1.setAttribute( "col2", "test" );
      af1.setAttribute( "col3", 2 );
      context.setFeature( af1 );

      QFETCH( QString, string );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      //try evaluating with context
      QVariant res = exp.evaluate( &context );
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();

      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( res, result );

      // check again - make sure value was correctly cached
      res = exp.evaluate( &context );
      QCOMPARE( res, result );
    }

    void relationAggregate_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<int>( "parentKey" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QTest::newRow( "bad relation" ) << "relation_aggregate('xxxtest','sum',\"col3\")" << 0 << true << QVariant();
      QTest::newRow( "bad aggregate" ) << "relation_aggregate('my_rel','xxsum',\"col3\")" << 0 << true << QVariant();
      QTest::newRow( "bad expression" ) << "relation_aggregate('my_rel','sum',\"xcvxcvcol1\")" << 0 << true << QVariant();

      QTest::newRow( "relation aggregate 1" ) << "relation_aggregate('my_rel','sum',\"col3\")" << 4 << false << QVariant( 5 );
      QTest::newRow( "relation aggregate by name" ) << "relation_aggregate('relation name','sum',\"col3\")" << 4 << false << QVariant( 5 );
      QTest::newRow( "relation aggregate 2" ) << "relation_aggregate('my_rel','sum',\"col3\")" << 3 << false << QVariant( 9 );
      QTest::newRow( "relation aggregate 2" ) << "relation_aggregate('my_rel','sum',\"col3\")" << 6 << false << QVariant( 0 );
      QTest::newRow( "relation aggregate count 1" ) << "relation_aggregate('my_rel','count',\"col3\")" << 4 << false << QVariant( 3 );
      QTest::newRow( "relation aggregate count 2" ) << "relation_aggregate('my_rel','count',\"col3\")" << 3 << false << QVariant( 2 );
      QTest::newRow( "relation aggregate count 2" ) << "relation_aggregate('my_rel','count',\"col3\")" << 6 << false << QVariant( 0 );
      QTest::newRow( "relation aggregate concatenation" ) << "relation_aggregate('my_rel','concatenate',to_string(\"col3\"),concatenator:=',')" << 3 << false << QVariant( "2,7" );

      QTest::newRow( "named relation aggregate 1" ) << "relation_aggregate(relation:='my_rel',aggregate:='sum',expression:=\"col3\")" << 4 << false << QVariant( 5 );
      QTest::newRow( "relation aggregate sub expression 1" ) << "relation_aggregate('my_rel','sum',\"col3\" * 2)" << 4 << false << QVariant( 10 );
      QTest::newRow( "relation aggregate bad sub expression" ) << "relation_aggregate('my_rel','sum',\"fsdfsddf\" * 2)" << 4 << true << QVariant();
    }

    void relationAggregate()
    {
      QFETCH( QString, string );
      QFETCH( int, parentKey );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::layerScope( mAggregatesLayer ) );

      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      af1.setAttribute( "col1", parentKey );
      context.setFeature( af1 );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res;

      //try evaluating with context
      res = exp.evaluate( &context );
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();

      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( res, result );

      // check again - make sure value was correctly cached
      res = exp.evaluate( &context );
      QCOMPARE( res, result );
    }

    void get_feature_geometry()
    {
      //test that get_feature fetches feature's geometry
      QgsExpression exp( QString( "x(geometry(get_feature('%1','heading',340)))" ).arg( mPointsLayer->id() ) );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();

      QCOMPARE( exp.hasEvalError(), false );
      QVERIFY( qgsDoubleNear( res.toDouble(), -85.65217, 0.00001 ) );
    }

    void eval_rand()
    {
      QgsExpression exp1( "rand(1,10)" );
      QVariant v1 = exp1.evaluate();
      QCOMPARE( v1.toInt() <= 10, true );
      QCOMPARE( v1.toInt() >= 1, true );

      QgsExpression exp2( "rand(min:=-5,max:=-5)" );
      QVariant v2 = exp2.evaluate();
      QCOMPARE( v2.toInt(), -5 );

      // Invalid expression since max<min
      QgsExpression exp3( "rand(10,1)" );
      QVariant v3 = exp3.evaluate();
      QCOMPARE( v3.type(),  QVariant::Invalid );
    }

    void eval_randf()
    {
      QgsExpression exp1( "randf(1.5,9.5)" );
      QVariant v1 = exp1.evaluate();
      QCOMPARE( v1.toDouble() <= 9.5, true );
      QCOMPARE( v1.toDouble() >= 1.5, true );

      QgsExpression exp2( "randf(min:=-0.0005,max:=-0.0005)" );
      QVariant v2 = exp2.evaluate();
      QCOMPARE( v2.toDouble(), -0.0005 );

      // Invalid expression since max<min
      QgsExpression exp3( "randf(9.3333,1.784)" );
      QVariant v3 = exp3.evaluate();
      QCOMPARE( v3.type(),  QVariant::Invalid );
    }

    void referenced_columns()
    {
      QSet<QString> expectedCols;
      expectedCols << "foo" << "bar" << "ppp" << "qqq" << "rrr";
      QgsExpression exp( "length(Bar || FOO) = 4 or foo + sqrt(bar) > 0 or case when ppp then qqq else rrr end" );
      QCOMPARE( exp.hasParserError(), false );
      QStringList refCols = exp.referencedColumns();
      // make sure we have lower case
      QSet<QString> refColsSet;
      Q_FOREACH ( const QString& col, refCols )
        refColsSet.insert( col.toLower() );

      QCOMPARE( refColsSet, expectedCols );
    }

    void referenced_columns_all_attributes()
    {
      QgsExpression exp( "attribute($currentfeature,'test')" );
      QCOMPARE( exp.hasParserError(), false );
      QStringList refCols = exp.referencedColumns();
      // make sure we get the all attributes flag
      bool allAttributesFlag = refCols.contains( QgsFeatureRequest::AllAttributes );
      QCOMPARE( allAttributesFlag, true );
    }

    void needs_geometry_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "needsGeom" );

      // literal evaluation
      QTest::newRow( "n > 0" ) << "n > 0" << false;
      QTest::newRow( "1 = 1" ) << "1 = 1" << false;
      QTest::newRow( "$x > 0" ) << "$x > 0" << true;
      QTest::newRow( "xat(0) > 0" ) << "xat(0) > 0" << true;
      QTest::newRow( "$x" ) << "$x" << true;
      QTest::newRow( "$area" ) << "$area" << true;
      QTest::newRow( "$length" ) << "$length" << true;
      QTest::newRow( "$perimeter" ) << "$perimeter" << true;
      QTest::newRow( "toint($perimeter)" ) << "toint($perimeter)" << true;
      QTest::newRow( "toint(123)" ) << "toint(123)" << false;
      QTest::newRow( "case 0" ) << "case when 1 then 0 end" << false;
      QTest::newRow( "case 1" ) << "case when $area > 0 then 1 end" << true;
      QTest::newRow( "case 2" ) << "case when 1 then $area end" << true;
      QTest::newRow( "case 3" ) << "case when 1 then 0 else $area end" << true;
    }

    void needs_geometry()
    {
      QFETCH( QString, string );
      QFETCH( bool, needsGeom );

      QgsExpression exp( string );
      if ( exp.hasParserError() )
        qDebug() << "parser error! " << exp.parserErrorString();
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), needsGeom );
    }

    void eval_geometry_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<double>( "result" );

      QgsPoint point( 123, 456 );
      QgsPolyline line;
      line << QgsPoint( 1, 1 ) << QgsPoint( 4, 2 ) << QgsPoint( 3, 1 );

      QTest::newRow( "geom x" ) << "$x" << ( void* ) QgsGeometry::fromPoint( point ) << false << 123.;
      QTest::newRow( "geom y" ) << "$y" << ( void* ) QgsGeometry::fromPoint( point ) << false << 456.;
      QTest::newRow( "geom xat" ) << "xat(-1)" << ( void* ) QgsGeometry::fromPolyline( line ) << false << 3.;
      QTest::newRow( "geom yat" ) << "yat(1)" << ( void* ) QgsGeometry::fromPolyline( line ) << false << 2.;
    }

    void eval_geometry()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );
      QFETCH( double, result );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), true );
      Q_NOWARN_DEPRECATED_PUSH
      QVariant out = exp.evaluate( &f );
      Q_NOWARN_DEPRECATED_POP
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.toDouble(), result );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.toDouble(), result );
    }

    void eval_geometry_calc()
    {
      QgsPolyline polyline, polygon_ring;
      polyline << QgsPoint( 0, 0 ) << QgsPoint( 10, 0 );
      polygon_ring << QgsPoint( 2, 1 ) << QgsPoint( 10, 1 ) << QgsPoint( 10, 6 ) << QgsPoint( 2, 6 ) << QgsPoint( 2, 1 );
      QgsPolygon polygon;
      polygon << polygon_ring;
      QgsFeature fPolygon, fPolyline;
      fPolyline.setGeometry( QgsGeometry::fromPolyline( polyline ) );
      fPolygon.setGeometry( QgsGeometry::fromPolygon( polygon ) );

      QgsExpressionContext context;

      Q_NOWARN_DEPRECATED_PUSH
      QgsExpression exp1( "$area" );
      QVariant vArea = exp1.evaluate( &fPolygon );
      QCOMPARE( vArea.toDouble(), 40. );

      context.setFeature( fPolygon );
      vArea = exp1.evaluate( &context );
      QCOMPARE( vArea.toDouble(), 40. );

      QgsExpression exp2( "$length" );
      QVariant vLength = exp2.evaluate( &fPolyline );
      QCOMPARE( vLength.toDouble(), 10. );

      context.setFeature( fPolyline );
      vLength = exp2.evaluate( &context );
      QCOMPARE( vLength.toDouble(), 10. );

      QgsExpression exp3( "$perimeter" );
      QVariant vPerimeter = exp3.evaluate( &fPolygon );
      QCOMPARE( vPerimeter.toDouble(), 26. );

      context.setFeature( fPolygon );
      vPerimeter = exp3.evaluate( &context );
      QCOMPARE( vPerimeter.toDouble(), 26. );

      QgsExpression deprecatedExpXAt( "$x_at(1)" );
      context.setFeature( fPolygon );
      QVariant xAt = deprecatedExpXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      context.setFeature( fPolyline );
      xAt = deprecatedExpXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );

      QgsExpression deprecatedExpXAtNeg( "$x_at(-2)" );
      context.setFeature( fPolygon );
      xAt = deprecatedExpXAtNeg.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 2.0 );

      QgsExpression deprecatedExpYAt( "$y_at(2)" );
      context.setFeature( fPolygon );
      QVariant yAt = deprecatedExpYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      QgsExpression deprecatedExpYAt2( "$y_at(1)" );
      context.setFeature( fPolyline );
      yAt = deprecatedExpYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 0.0 );

      QgsExpression deprecatedExpYAtNeg( "$y_at(-2)" );
      context.setFeature( fPolygon );
      yAt = deprecatedExpYAtNeg.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );

      QgsExpression expXAt( "x_at(1)" );
      context.setFeature( fPolygon );
      xAt = expXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      context.setFeature( fPolyline );
      xAt = expXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );

      QgsExpression expXAtNeg( "x_at(-2)" );
      context.setFeature( fPolygon );
      xAt = expXAtNeg.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 2.0 );

      QgsExpression expYAt( "y_at(2)" );
      context.setFeature( fPolygon );
      yAt = expYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      QgsExpression expYAt2( "$y_at(1)" );
      context.setFeature( fPolyline );
      yAt = expYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 0.0 );

      QgsExpression expYAtNeg( "y_at(-2)" );
      context.setFeature( fPolygon );
      yAt = expYAtNeg.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );

      QgsExpression exp4( "bounds_width($geometry)" );
      QVariant vBoundsWidth = exp4.evaluate( &fPolygon );
      QCOMPARE( vBoundsWidth.toDouble(), 8.0 );

      context.setFeature( fPolygon );
      vBoundsWidth = exp4.evaluate( &context );
      QCOMPARE( vBoundsWidth.toDouble(), 8.0 );

      QgsExpression exp5( "bounds_height($geometry)" );
      QVariant vBoundsHeight = exp5.evaluate( &fPolygon );
      QCOMPARE( vBoundsHeight.toDouble(), 5.0 );

      vBoundsHeight = exp5.evaluate( &context );
      QCOMPARE( vBoundsHeight.toDouble(), 5.0 );

      QgsExpression exp6( "xmin($geometry)" );
      QVariant vXMin = exp6.evaluate( &fPolygon );
      QCOMPARE( vXMin.toDouble(), 2.0 );

      vXMin = exp6.evaluate( &context );
      QCOMPARE( vXMin.toDouble(), 2.0 );

      QgsExpression exp7( "xmax($geometry)" );
      QVariant vXMax = exp7.evaluate( &fPolygon );
      QCOMPARE( vXMax.toDouble(), 10.0 );

      vXMax = exp7.evaluate( &context );
      QCOMPARE( vXMax.toDouble(), 10.0 );

      QgsExpression exp8( "ymin($geometry)" );
      QVariant vYMin = exp8.evaluate( &fPolygon );
      QCOMPARE( vYMin.toDouble(), 1.0 );

      vYMin = exp8.evaluate( &context );
      QCOMPARE( vYMin.toDouble(), 1.0 );

      QgsExpression exp9( "ymax($geometry)" );
      QVariant vYMax = exp9.evaluate( &fPolygon );
      QCOMPARE( vYMax.toDouble(), 6.0 );

      exp9.evaluate( &context );
      QCOMPARE( vYMax.toDouble(), 6.0 );

      QgsExpression exp10( "num_points($geometry)" );
      QVariant vVertices = exp10.evaluate( &fPolygon );
      QCOMPARE( vVertices.toInt(), 5 );

      QgsExpression exp11( "length($geometry)" );
      QVariant vLengthLine = exp11.evaluate( &fPolyline );
      QCOMPARE( vLengthLine.toDouble(), 10.0 );

      QgsExpression exp12( "area($geometry)" );
      QVariant vAreaPoly = exp12.evaluate( &fPolygon );
      QCOMPARE( vAreaPoly.toDouble(), 40.0 );

      QgsExpression exp13( "perimeter($geometry)" );
      QVariant vPerimeterPoly = exp13.evaluate( &fPolygon );
      QCOMPARE( vPerimeterPoly.toDouble(), 26.0 );

      Q_NOWARN_DEPRECATED_POP

    }

    void geom_calculator()
    {
      //test calculations with and without geometry calculator set
      QgsDistanceArea da;
      da.setSourceAuthId( "EPSG:3111" );
      da.setEllipsoid( "WGS84" );
      da.setEllipsoidalMode( true );

      QgsFeature feat;
      QgsPolyline polygonRing3111;
      polygonRing3111 << QgsPoint( 2484588, 2425722 ) << QgsPoint( 2482767, 2398853 ) << QgsPoint( 2520109, 2397715 ) << QgsPoint( 2520792, 2425494 ) << QgsPoint( 2484588, 2425722 );
      QgsPolygon polygon3111;
      polygon3111 << polygonRing3111;
      feat.setGeometry( QgsGeometry::fromPolygon( polygon3111 ) );
      QgsExpressionContext context;
      context.setFeature( feat );

      // test area without geomCalculator
      QgsExpression expArea( "$area" );
      QVariant vArea = expArea.evaluate( &context );
      double expected = 1005640568.0;
      QVERIFY( qgsDoubleNear( vArea.toDouble(), expected, 1.0 ) );
      // units should not be converted if no geometry calculator set
      expArea.setAreaUnits( QgsUnitTypes::SquareFeet );
      vArea = expArea.evaluate( &context );
      QVERIFY( qgsDoubleNear( vArea.toDouble(), expected, 1.0 ) );
      expArea.setAreaUnits( QgsUnitTypes::SquareNauticalMiles );
      vArea = expArea.evaluate( &context );
      QVERIFY( qgsDoubleNear( vArea.toDouble(), expected, 1.0 ) );

      // test area with geomCalculator
      QgsExpression expArea2( "$area" );
      expArea2.setGeomCalculator( da );
      vArea = expArea2.evaluate( &context );
      expected = 1009089817.0;
      QVERIFY( qgsDoubleNear( vArea.toDouble(), expected, 1.0 ) );
      // test unit conversion
      expArea2.setAreaUnits( QgsUnitTypes::SquareMeters ); //default units should be square meters
      vArea = expArea2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vArea.toDouble(), expected, 1.0 ) );
      expArea2.setAreaUnits( QgsUnitTypes::UnknownAreaUnit ); //unknown units should not be converted
      vArea = expArea2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vArea.toDouble(), expected, 1.0 ) );
      expArea2.setAreaUnits( QgsUnitTypes::SquareMiles );
      expected = 389.6117565069;
      vArea = expArea2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vArea.toDouble(), expected, 0.001 ) );

      // test perimeter without geomCalculator
      QgsExpression expPerimeter( "$perimeter" );
      QVariant vPerimeter = expPerimeter.evaluate( &context );
      expected = 128282.086;
      QVERIFY( qgsDoubleNear( vPerimeter.toDouble(), expected, 0.001 ) );
      // units should not be converted if no geometry calculator set
      expPerimeter.setDistanceUnits( QGis::Feet );
      vPerimeter = expPerimeter.evaluate( &context );
      QVERIFY( qgsDoubleNear( vPerimeter.toDouble(), expected, 0.001 ) );
      expPerimeter.setDistanceUnits( QGis::NauticalMiles );
      vPerimeter = expPerimeter.evaluate( &context );
      QVERIFY( qgsDoubleNear( vPerimeter.toDouble(), expected, 0.001 ) );

      // test perimeter with geomCalculator
      QgsExpression expPerimeter2( "$perimeter" );
      expPerimeter2.setGeomCalculator( da );
      vPerimeter = expPerimeter2.evaluate( &context );
      expected = 128289.074;
      QVERIFY( qgsDoubleNear( vPerimeter.toDouble(), expected, 0.001 ) );
      // test unit conversion
      expPerimeter2.setDistanceUnits( QGis::Meters ); //default units should be meters
      vPerimeter = expPerimeter2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vPerimeter.toDouble(), expected, 0.001 ) );
      expPerimeter2.setDistanceUnits( QGis::UnknownUnit ); //unknown units should not be converted
      vPerimeter = expPerimeter2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vPerimeter.toDouble(), expected, 0.001 ) );
      expPerimeter2.setDistanceUnits( QGis::Feet );
      expected = 420895.9120735;
      vPerimeter = expPerimeter2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vPerimeter.toDouble(), expected, 0.001 ) );

      // test length without geomCalculator
      QgsPolyline line3111;
      line3111 << QgsPoint( 2484588, 2425722 ) << QgsPoint( 2482767, 2398853 );
      feat.setGeometry( QgsGeometry::fromPolyline( line3111 ) );
      context.setFeature( feat );

      QgsExpression expLength( "$length" );
      QVariant vLength = expLength.evaluate( &context );
      expected = 26930.637;
      QVERIFY( qgsDoubleNear( vLength.toDouble(), expected, 0.001 ) );
      // units should not be converted if no geometry calculator set
      expLength.setDistanceUnits( QGis::Feet );
      vLength = expLength.evaluate( &context );
      QVERIFY( qgsDoubleNear( vLength.toDouble(), expected, 0.001 ) );
      expLength.setDistanceUnits( QGis::NauticalMiles );
      vLength = expLength.evaluate( &context );
      QVERIFY( qgsDoubleNear( vLength.toDouble(), expected, 0.001 ) );

      // test length with geomCalculator
      QgsExpression expLength2( "$length" );
      expLength2.setGeomCalculator( da );
      vLength = expLength2.evaluate( &context );
      expected = 26932.156;
      QVERIFY( qgsDoubleNear( vLength.toDouble(), expected, 0.001 ) );
      // test unit conversion
      expLength2.setDistanceUnits( QGis::Meters ); //default units should be meters
      vLength = expLength2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vLength.toDouble(), expected, 0.001 ) );
      expLength2.setDistanceUnits( QGis::UnknownUnit ); //unknown units should not be converted
      vLength = expLength2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vLength.toDouble(), expected, 0.001 ) );
      expLength2.setDistanceUnits( QGis::Feet );
      expected = 88360.0918635;
      vLength = expLength2.evaluate( &context );
      QVERIFY( qgsDoubleNear( vLength.toDouble(), expected, 0.001 ) );
    }

    void eval_geometry_wkt()
    {
      QgsPolyline polyline, polygon_ring;
      polyline << QgsPoint( 0, 0 ) << QgsPoint( 10, 0 );
      polygon_ring << QgsPoint( 2, 1 ) << QgsPoint( 10, 1 ) << QgsPoint( 10, 6 ) << QgsPoint( 2, 6 ) << QgsPoint( 2, 1 );

      QgsPolygon polygon;
      polygon << polygon_ring;

      QgsFeature fPoint, fPolygon, fPolyline;
      fPoint.setGeometry( QgsGeometry::fromPoint( QgsPoint( -1.23456789, 9.87654321 ) ) );
      fPolyline.setGeometry( QgsGeometry::fromPolyline( polyline ) );
      fPolygon.setGeometry( QgsGeometry::fromPolygon( polygon ) );

      QgsExpressionContext context;

      Q_NOWARN_DEPRECATED_PUSH
      QgsExpression exp1( "geomToWKT($geometry)" );
      QVariant vWktLine = exp1.evaluate( &fPolyline );
      QCOMPARE( vWktLine.toString(), QString( "LineString (0 0, 10 0)" ) );

      context.setFeature( fPolyline );
      vWktLine = exp1.evaluate( &context );
      QCOMPARE( vWktLine.toString(), QString( "LineString (0 0, 10 0)" ) );

      QgsExpression exp2( "geomToWKT($geometry)" );
      QVariant vWktPolygon = exp2.evaluate( &fPolygon );
      QCOMPARE( vWktPolygon.toString(), QString( "Polygon ((2 1, 10 1, 10 6, 2 6, 2 1))" ) );

      context.setFeature( fPolygon );
      vWktPolygon = exp2.evaluate( &context );
      QCOMPARE( vWktPolygon.toString(), QString( "Polygon ((2 1, 10 1, 10 6, 2 6, 2 1))" ) );

      QgsExpression exp3( "geomToWKT($geometry)" );
      QVariant vWktPoint = exp3.evaluate( &fPoint );
      QCOMPARE( vWktPoint.toString(), QString( "Point (-1.23456789 9.87654321)" ) );

      context.setFeature( fPoint );
      vWktPoint = exp3.evaluate( &context );
      QCOMPARE( vWktPoint.toString(), QString( "Point (-1.23456789 9.87654321)" ) );

      QgsExpression exp4( "geomToWKT($geometry, 3)" );
      QVariant vWktPointSimplify = exp4.evaluate( &fPoint );
      QCOMPARE( vWktPointSimplify.toString(), QString( "Point (-1.235 9.877)" ) );

      vWktPointSimplify = exp4.evaluate( &context );
      QCOMPARE( vWktPointSimplify.toString(), QString( "Point (-1.235 9.877)" ) );

      Q_NOWARN_DEPRECATED_POP

    }

    void eval_geometry_constructor_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );

      QgsPoint point( 123, 456 );
      QgsPolyline line;
      line << QgsPoint( 1, 1 ) << QgsPoint( 4, 2 ) << QgsPoint( 3, 1 );

      QgsPolyline polyline, polygon_ring;
      polyline << QgsPoint( 0, 0 ) << QgsPoint( 10, 0 );
      polygon_ring << QgsPoint( 1, 1 ) << QgsPoint( 6, 1 ) << QgsPoint( 6, 6 ) << QgsPoint( 1, 6 ) << QgsPoint( 1, 1 );
      QgsPolygon polygon;
      polygon << polygon_ring;

      QScopedPointer<QgsGeometry> sourcePoint( QgsGeometry::fromPoint( point ) );
      QTest::newRow( "geomFromWKT Point" ) << "geom_from_wkt('" + sourcePoint->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPoint( point ) << false;
      QScopedPointer<QgsGeometry> sourceLine( QgsGeometry::fromPolyline( line ) );
      QTest::newRow( "geomFromWKT Line" ) << "geomFromWKT('" + sourceLine->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPolyline( line ) << false;
      QScopedPointer<QgsGeometry> sourcePolyline( QgsGeometry::fromPolyline( polyline ) );
      QTest::newRow( "geomFromWKT Polyline" ) << "geomFromWKT('" + sourcePolyline->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPolyline( polyline ) << false;
      QScopedPointer<QgsGeometry> sourcePolygon( QgsGeometry::fromPolygon( polygon ) );
      QTest::newRow( "geomFromWKT Polygon" ) << "geomFromWKT('" + sourcePolygon->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false;

      // GML Point
      QTest::newRow( "GML Point (coordinates)" ) << "geomFromGML('<gml:Point><gml:coordinates>123,456</gml:coordinates></gml:Point>')" << ( void * ) QgsGeometry::fromPoint( point ) << false;
      // gml:pos if from GML3
      QTest::newRow( "GML Point (pos)" ) << "geomFromGML('<gml:Point srsName=\"foo\"><gml:pos srsDimension=\"2\">123 456</gml:pos></gml:Point>')" << ( void * ) QgsGeometry::fromPoint( point ) << false;

      // GML Box
      QgsRectangle rect( 135.2239, 34.4879, 135.8578, 34.8471 );
      QTest::newRow( "GML Box" ) << "geomFromGML('<gml:Box srsName=\"foo\"><gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>')" << ( void * ) QgsGeometry::fromRect( rect ) << false;
      // Envelope is from GML3 ?
      QTest::newRow( "GML Envelope" ) << "geomFromGML('<gml:Envelope>"
      "<gml:lowerCorner>135.2239 34.4879</gml:lowerCorner>"
      "<gml:upperCorner>135.8578 34.8471</gml:upperCorner>"
      "</gml:Envelope>')" << ( void * ) QgsGeometry::fromRect( rect ) << false;
    }

    void eval_geometry_constructor()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), false );

      //deprecated method
      Q_NOWARN_DEPRECATED_PUSH
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QCOMPARE( geom->equals( &outGeom ), true );
      Q_NOWARN_DEPRECATED_POP

      //replacement method
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      outGeom = out.value<QgsGeometry>();
      QCOMPARE( geom->equals( &outGeom ), true );
    }

    void eval_geometry_access_transform_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<bool>( "needsGeom" );

      QgsPoint point( 123, 456 );
      QgsPolyline line;
      line << QgsPoint( 1, 1 ) << QgsPoint( 4, 2 ) << QgsPoint( 3, 1 );

      QgsPolyline polyline, polygon_ring;
      polyline << QgsPoint( 0, 0 ) << QgsPoint( 10, 0 );
      polygon_ring << QgsPoint( 1, 1 ) << QgsPoint( 6, 1 ) << QgsPoint( 6, 6 ) << QgsPoint( 1, 6 ) << QgsPoint( 1, 1 );
      QgsPolygon polygon;
      polygon << polygon_ring;

      QTest::newRow( "geometry Point" ) << "geometry( $currentfeature )" << ( void* ) QgsGeometry::fromPoint( point ) << false << true;
      QTest::newRow( "geometry Line" ) << "geometry( $currentfeature )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << true;
      QTest::newRow( "geometry Polyline" ) << "geometry( $currentfeature )" << ( void* ) QgsGeometry::fromPolyline( polyline ) << false << true;
      QTest::newRow( "geometry Polygon" ) << "geometry( $currentfeature )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << true;

      QgsCoordinateReferenceSystem s;
      s.createFromOgcWmsCrs( "EPSG:4326" );
      QgsCoordinateReferenceSystem d;
      d.createFromOgcWmsCrs( "EPSG:3857" );
      QgsCoordinateTransform t( s, d );

      QgsGeometry* tLine = QgsGeometry::fromPolyline( line );
      tLine->transform( t );
      QgsGeometry* tPolygon = QgsGeometry::fromPolygon( polygon );
      tPolygon->transform( t );

      QgsGeometry* oLine = QgsGeometry::fromPolyline( line );
      QgsGeometry* oPolygon = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "transform Line" ) << "transform( geomFromWKT('" + oLine->exportToWkt() + "'), 'EPSG:4326', 'EPSG:3857' )" << ( void* ) tLine << false << false;
      QTest::newRow( "transform Polygon" ) << "transform( geomFromWKT('" + oPolygon->exportToWkt() + "'), 'EPSG:4326', 'EPSG:3857' )" << ( void* ) tPolygon << false << false;
      delete oLine;
      delete oPolygon;
    }

    void eval_geometry_access_transform()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );
      QFETCH( bool, needsGeom );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), needsGeom );

      //deprecated method
      Q_NOWARN_DEPRECATED_PUSH
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QCOMPARE( geom->equals( &outGeom ), true );
      Q_NOWARN_DEPRECATED_POP

      //replacement method
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      outGeom = out.value<QgsGeometry>();
      QCOMPARE( geom->equals( &outGeom ), true );
    }

    void eval_spatial_operator_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QgsPoint point( 0, 0 );
      QgsPolyline line, polygon_ring;
      line << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 );
      polygon_ring << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 );
      QgsPolygon polygon;
      polygon << polygon_ring;

      QTest::newRow( "No Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 2 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 0 );
      QTest::newRow( "Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 1 );
      QTest::newRow( "No Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 0 );
      QTest::newRow( "Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 2 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 1 );

      // OGR test
      QTest::newRow( "OGR Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Intersects" ) << "intersects( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR no Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Disjoint" ) << "disjoint( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR Touches" ) << "touches( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Touches" ) << "touches( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Crosses" ) << "crosses( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Crosses" ) << "crosses( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Within" ) << "within( $geometry, geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))') )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Within" ) << "within( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR Contians" ) << "contains( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Contains" ) << "contains( $geometry, geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))') )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR no Overlaps" ) << "overlaps( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR overlaps" ) << "overlaps( geomFromWKT('POLYGON((0 -5,10 5,10 -5,0 -5))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 1 );
    }

    void eval_spatial_operator()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), true );

      //deprecated method
      Q_NOWARN_DEPRECATED_PUSH
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.toInt(), result.toInt() );
      Q_NOWARN_DEPRECATED_POP

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.toInt(), result.toInt() );
    }

    void eval_geometry_method_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<bool>( "needGeom" );
      QTest::addColumn<void*>( "resultptr" );

      QgsPoint point( 0, 0 );
      QgsPolyline line, polygon_ring;
      line << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 );
      polygon_ring << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 );
      QgsPolygon polygon;
      polygon << polygon_ring;

      QgsGeometry *geom;

      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "buffer" ) << "buffer( $geometry, 1.0, 3)" << ( void* ) geom << false << true << ( void* ) geom->buffer( 1.0, 3 );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "buffer" ) << "buffer( $geometry, 2.0)" << ( void* ) geom << false << true << ( void* ) geom->buffer( 2.0, 8 );

      QgsPoint point1( 10, 20 );
      QgsPoint point2( 30, 20 );
      QgsGeometry *pnt1 = QgsGeometry::fromPoint( point1 );
      QgsGeometry *pnt2 = QgsGeometry::fromPoint( point2 );
      QTest::newRow( "union" ) << "union( $geometry, geomFromWKT('" + pnt2->exportToWkt() + "') )" << ( void* ) pnt1 << false << true << ( void* ) pnt1->combine( pnt2 );
      delete pnt2;

      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "intersection" ) << "intersection( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "POLYGON ((0 0,5 5,10 0,0 0))" );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "difference" ) << "difference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "POLYGON ((5 5,10 10,10 0,5 5))" );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "symDifference" ) << "symDifference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "MULTIPOLYGON(((5 5,0 0,0 10,5 5)),((5 5,10 10,10 0,5 5)))" );

      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "convexHull simple" ) << "convexHull( $geometry )" << ( void* ) geom << false << true << ( void* ) geom->convexHull();
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "convexHull multi" ) << "convexHull( geomFromWKT('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))') )" << ( void* ) geom << false << false << ( void* ) QgsGeometry::fromWkt( "POLYGON ((0 0,0 1,1 1,1 0,0 0))" );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "bounds" ) << "bounds( $geometry )" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromRect( geom->boundingBox() );

      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "translate" ) << "translate( $geometry, 1, 2)" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "POLYGON ((1 2,11 12,11 2,1 2))" );
      geom = QgsGeometry::fromPolyline( line );
      QTest::newRow( "translate" ) << "translate( $geometry, -1, 2)" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "LINESTRING (-1 2, 9 12)" );
      geom = QgsGeometry::fromPoint( point );
      QTest::newRow( "translate" ) << "translate( $geometry, 1, -2)" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "POINT(1 -2)" );
    }

    void eval_geometry_method()
    {
      QFETCH( QString, string );
      QFETCH( void *, geomptr );
      QFETCH( bool, evalError );
      QFETCH( bool, needGeom );
      QFETCH( void *, resultptr );

      QgsGeometry *geom = ( QgsGeometry * ) geomptr;
      QgsGeometry *result = ( QgsGeometry * ) resultptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), needGeom );

      //deprecated method
      Q_NOWARN_DEPRECATED_PUSH
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QVERIFY( compareWkt( outGeom.exportToWkt(), result->exportToWkt() ) );
      Q_NOWARN_DEPRECATED_POP

      //replacement method
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      outGeom = out.value<QgsGeometry>();
      QVERIFY( compareWkt( outGeom.exportToWkt(), result->exportToWkt() ) );

      delete result;
    }

    void eval_special_columns()
    {
      Q_NOWARN_DEPRECATED_PUSH
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<QVariant>( "result" );

      QgsExpression::setSpecialColumn( "$var1", QVariant(( int )42 ) );

      QgsExpression exp( "$var1 + 1" );
      QVariant v1 = exp.evaluate();
      QCOMPARE( v1.toInt(), 43 );

      QgsExpression::setSpecialColumn( "$var1", QVariant(( int )100 ) );
      QVariant v2 = exp.evaluate();
      QCOMPARE( v2.toInt(), 101 );

      QgsExpression exp2( "_specialcol_('$var1')+1" );
      QVariant v3 = exp2.evaluate();
      QCOMPARE( v3.toInt(), 101 );

      QgsExpression exp3( "_specialcol_('undefined')" );
      QVariant v4 = exp3.evaluate();
      QCOMPARE( v4, QVariant() );

      QgsExpression::unsetSpecialColumn( "$var1" );
      Q_NOWARN_DEPRECATED_POP
    }

    void eval_eval()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( "col1" ) );
      fields.append( QgsField( "second_column", QVariant::Int ) );
      f.setFields( fields, true );
      f.setAttribute( QString( "col1" ), QString( "test value" ) );
      f.setAttribute( QString( "second_column" ), 5 );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );

      QgsExpression exp1( "eval()" );
      QVariant v1 = exp1.evaluate( &context );

      Q_ASSERT( !v1.isValid() );

      QgsExpression exp2( "eval('4')" );
      QVariant v2 = exp2.evaluate( &context );
      QCOMPARE( v2, QVariant( 4 ) );

      QgsExpression exp3( "eval('\"second_column\" * 2')" );
      QVariant v3 = exp3.evaluate( &context );
      QCOMPARE( v3, QVariant( 10 ) );

      QgsExpression exp4( "eval('\"col1\"')" );
      QVariant v4 = exp4.evaluate( &context );
      QCOMPARE( v4, QVariant( "test value" ) );
    }

    void expression_from_expression_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::newRow( "column ref" ) << "my_column";
      QTest::newRow( "column ref with space" ) << "\"my column\"";
      QTest::newRow( "string literal" ) << "'hello'";
      QTest::newRow( "string with quote" ) << "'hel''lo'";
    }

    void expression_from_expression()
    {
      QFETCH( QString, string );

      QgsExpression e( string );
      QVERIFY( !e.hasParserError() );
      qDebug() << e.expression();
      QCOMPARE( e.expression(), QgsExpression( e.expression() ).expression() );
    }

    void quote_string()
    {
      QCOMPARE( QgsExpression::quotedString( "hello\nworld" ), QString( "'hello\\nworld'" ) );
      QCOMPARE( QgsExpression::quotedString( "hello\tworld" ), QString( "'hello\\tworld'" ) );
      QCOMPARE( QgsExpression::quotedString( "hello\\world" ), QString( "'hello\\\\world'" ) );
    }

    void quoted_value()
    {
      QCOMPARE( QgsExpression::quotedValue( QVariant( "a string" ) ), QString( "'a string'" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( "a\nstring" ) ), QString( "'a\\nstring'" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( 5 ) ), QString( "5" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( 5 ), QVariant::String ), QString( "'5'" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( 5LL ) ), QString( "5" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( 5.5 ) ), QString( "5.5" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( true ) ), QString( "TRUE" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( true ), QVariant::String ), QString( "'true'" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant() ), QString( "NULL" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant(), QVariant::String ), QString( "NULL" ) );
    }

    void reentrant()
    {
      // this simply should not crash

      QList<int> lst;
      for ( int i = 0; i < 10; ++i )
        lst << i;
      QtConcurrent::blockingMap( lst, _parseAndEvalExpr );
    }

    void evaluateToDouble()
    {
      QCOMPARE( QgsExpression::evaluateToDouble( QString( "5" ), 0.0 ), 5.0 );
      QCOMPARE( QgsExpression::evaluateToDouble( QString( "5+6" ), 0.0 ), 11.0 );
      QCOMPARE( QgsExpression::evaluateToDouble( QString( "5*" ), 7.0 ), 7.0 );
      QCOMPARE( QgsExpression::evaluateToDouble( QString( "a" ), 9.0 ), 9.0 );
      QCOMPARE( QgsExpression::evaluateToDouble( QString(), 9.0 ), 9.0 );
    }

    void eval_isField()
    {
      QCOMPARE( QgsExpression( "" ).isField(), false );
      QCOMPARE( QgsExpression( "42" ).isField(), false );
      QCOMPARE( QgsExpression( "foo" ).isField(), true );
      QCOMPARE( QgsExpression( "\"foo bar\"" ).isField(), true );
      QCOMPARE( QgsExpression( "sqrt(foo)" ).isField(), false );
      QCOMPARE( QgsExpression( "foo + bar" ).isField(), false );
    }

    void test_implicitSharing()
    {
      QgsExpression* exp = new QgsExpression( "Pilots > 2" );

      QgsExpression expcopy( *exp );

      QVERIFY( expcopy.rootNode() );
      QCOMPARE( expcopy.expression(), QString( "Pilots > 2" ) );

      // Delete original instance, should preserve copy.
      delete exp;

      // This mainly should not crash, root node should have outlived the original one
      QVERIFY( !expcopy.rootNode()->dump().isEmpty() );

      // Let's take another copy
      QgsExpression expcopy2( expcopy );

      QgsExpressionContext ctx = QgsExpressionContextUtils::createFeatureBasedContext( 0, mPointsLayer->fields() );

      // Prepare with the current set of fields
      expcopy.prepare( &ctx );

      QgsFeatureIterator it = mPointsLayer->getFeatures();
      QgsFeature feat;

      // Let's count some features
      int count = 0;
      while ( it.nextFeature( feat ) )
      {
        QgsExpressionContext ctx = QgsExpressionContextUtils::createFeatureBasedContext( feat, mPointsLayer->fields() );
        if ( expcopy.evaluate( &ctx ).toBool() )
          count++;
      }

      QCOMPARE( count, 6 );

      // Let's remove the field referenced in the expression
      mPointsLayer->startEditing();
      mPointsLayer->deleteAttribute( mPointsLayer->fieldNameIndex( "Pilots" ) );

      // Now the prepared expression is broken
      // The cached field index points to the index which now is
      // used by "Cabin Crew". Not a particularly good test
      // since it actually relies on undefined behavior (changing
      // fields after prepare() )

      it = mPointsLayer->getFeatures();
      count = 0;
      while ( it.nextFeature( feat ) )
      {
        QgsExpressionContext ctx = QgsExpressionContextUtils::createFeatureBasedContext( feat, mPointsLayer->fields() );
        if ( expcopy.evaluate( &ctx ).toBool() )
          count++;
      }

      QCOMPARE( count, 3 );

      // But the copy should not have cached field indexes
      it = mPointsLayer->getFeatures();
      count = 0;
      while ( it.nextFeature( feat ) )
      {
        QgsExpressionContext ctx = QgsExpressionContextUtils::createFeatureBasedContext( feat, mPointsLayer->fields() );
        if ( expcopy2.evaluate( &ctx ).toBool() )
          count++;
      }

      QCOMPARE( count, 0 );

      // Detach a more complex expression
      QgsExpression nodeExpression( "1 IN (1, 2, 3, 4)" );
      QgsExpression nodeExpression2( nodeExpression );
    }

    void test_columnRefUnprepared()
    {
      //test retrieving fields from feature when expression is unprepared - explicitly specified fields collection
      //should take precedence over feature's field collection

      QgsFields fields;
      fields.append( QgsField( "f1", QVariant::String ) );

      QgsFeature f( 1 );
      f.setFields( fields );

      //also add a joined field - this will not be available in feature's field collection
      fields.append( QgsField( "j1", QVariant::String ), QgsFields::OriginJoin, 1 );

      f.setAttributes( QgsAttributes() << QVariant( "f1" ) << QVariant( "j1" ) );
      f.setValid( true );

      QgsExpression e( "\"f1\"" );
      QgsExpressionContext context;
      context.setFeature( f );
      context.setFields( fields );
      QVariant result = e.evaluate( &context );
      QCOMPARE( result.toString(), QString( "f1" ) );

      //test joined field
      QgsExpression e2( "\"j1\"" );
      result = e2.evaluate( &context );
      QCOMPARE( result.toString(), QString( "j1" ) );

      // final test - check that feature's field collection is also used when corresponding field NOT found
      // in explicitly passed field collection
      fields.append( QgsField( "f2", QVariant::String ) );
      f.setFields( fields );
      f.setAttributes( QgsAttributes() << QVariant( "f1" ) << QVariant( "j1" ) << QVariant( "f2" ) );
      context.setFeature( f );
      QgsExpression e3( "\"f2\"" );
      result = e3.evaluate( &context );
      QCOMPARE( result.toString(), QString( "f2" ) );
    }

};

QTEST_MAIN( TestQgsExpression )

#include "testqgsexpression.moc"
