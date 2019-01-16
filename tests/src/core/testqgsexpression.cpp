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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QtConcurrentMap>

#include <qgsapplication.h>
//header for class being tested
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsgeometry.h"
#include "qgsrenderchecker.h"
#include "qgsexpressioncontext.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsvectorlayerutils.h"

static void _parseAndEvalExpr( int arg )
{
  Q_UNUSED( arg );
  for ( int i = 0; i < 100; ++i )
  {
    QgsExpression exp( QStringLiteral( "1 + 2 * 2" ) );
    exp.evaluate();
  }
}

class TestQgsExpression: public QObject
{
    Q_OBJECT

  public:

    TestQgsExpression() = default;

  private:

    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mMemoryLayer = nullptr;
    QgsVectorLayer *mAggregatesLayer = nullptr;
    QgsVectorLayer *mChildLayer = nullptr;
    QgsRasterLayer *mRasterLayer = nullptr;

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
      QgsApplication::createDatabase();
      QgsApplication::showSettings();

      //create a point layer that will be used in all tests...
      QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
      QString pointsFileName = testDataDir + "points.shp";
      QFileInfo pointFileInfo( pointsFileName );
      mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                         pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
      QgsProject::instance()->addMapLayer( mPointsLayer );
      mPointsLayer->setTitle( QStringLiteral( "layer title" ) );
      mPointsLayer->setAbstract( QStringLiteral( "layer abstract" ) );
      mPointsLayer->setKeywordList( QStringLiteral( "layer,keywords" ) );
      mPointsLayer->setDataUrl( QStringLiteral( "data url" ) );
      mPointsLayer->setAttribution( QStringLiteral( "layer attribution" ) );
      mPointsLayer->setAttributionUrl( QStringLiteral( "attribution url" ) );
      mPointsLayer->setMinimumScale( 500 );
      mPointsLayer->setMaximumScale( 1000 );

      QString rasterFileName = testDataDir + "tenbytenraster.asc";
      QFileInfo rasterFileInfo( rasterFileName );
      mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                         rasterFileInfo.completeBaseName() );
      QgsProject::instance()->addMapLayer( mRasterLayer );

      // test memory layer for get_feature tests
      mMemoryLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:string" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
      QVERIFY( mMemoryLayer->isValid() );
      QgsFeature f1( mMemoryLayer->dataProvider()->fields(), 1 );
      f1.setAttribute( QStringLiteral( "col1" ), 10 );
      f1.setAttribute( QStringLiteral( "col2" ), "test1" );
      QgsFeature f2( mMemoryLayer->dataProvider()->fields(), 2 );
      f2.setAttribute( QStringLiteral( "col1" ), 11 );
      f2.setAttribute( QStringLiteral( "col2" ), "test2" );
      QgsFeature f3( mMemoryLayer->dataProvider()->fields(), 3 );
      f3.setAttribute( QStringLiteral( "col1" ), 3 );
      f3.setAttribute( QStringLiteral( "col2" ), "test3" );
      QgsFeature f4( mMemoryLayer->dataProvider()->fields(), 4 );
      f4.setAttribute( QStringLiteral( "col1" ), 41 );
      f4.setAttribute( QStringLiteral( "col2" ), "test4" );
      mMemoryLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );
      QgsProject::instance()->addMapLayer( mMemoryLayer );

      // test layer for aggregates
      mAggregatesLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:string&field=col3:integer&field=col4:string" ), QStringLiteral( "aggregate_layer" ), QStringLiteral( "memory" ) );
      QVERIFY( mAggregatesLayer->isValid() );
      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      af1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 0, 0 ) ) );
      af1.setAttribute( QStringLiteral( "col1" ), 4 );
      af1.setAttribute( QStringLiteral( "col2" ), "test" );
      af1.setAttribute( QStringLiteral( "col3" ), 2 );
      af1.setAttribute( QStringLiteral( "col4" ), QVariant( QVariant::String ) );
      QgsFeature af2( mAggregatesLayer->dataProvider()->fields(), 2 );
      af2.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 0 ) ) );
      af2.setAttribute( QStringLiteral( "col1" ), 2 );
      af2.setAttribute( QStringLiteral( "col2" ), QVariant( QVariant::String ) );
      af2.setAttribute( QStringLiteral( "col3" ), 1 );
      af2.setAttribute( QStringLiteral( "col4" ), QVariant( QVariant::String ) );
      QgsFeature af3( mAggregatesLayer->dataProvider()->fields(), 3 );
      af3.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 2, 0 ) ) );
      af3.setAttribute( QStringLiteral( "col1" ), 3 );
      af3.setAttribute( QStringLiteral( "col2" ), "test333" );
      af3.setAttribute( QStringLiteral( "col3" ), 2 );
      af3.setAttribute( QStringLiteral( "col4" ), QVariant( QVariant::String ) );
      QgsFeature af4( mAggregatesLayer->dataProvider()->fields(), 4 );
      af4.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 0 ) ) );
      af4.setAttribute( QStringLiteral( "col1" ), 2 );
      af4.setAttribute( QStringLiteral( "col2" ), "test4" );
      af4.setAttribute( QStringLiteral( "col3" ), 2 );
      af4.setAttribute( QStringLiteral( "col4" ), "" );
      QgsFeature af5( mAggregatesLayer->dataProvider()->fields(), 5 );
      af5.setGeometry( QgsGeometry() );
      af5.setAttribute( QStringLiteral( "col1" ), 5 );
      af5.setAttribute( QStringLiteral( "col2" ), QVariant( QVariant::String ) );
      af5.setAttribute( QStringLiteral( "col3" ), 3 );
      af5.setAttribute( QStringLiteral( "col4" ), "test" );
      QgsFeature af6( mAggregatesLayer->dataProvider()->fields(), 6 );
      af6.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 5, 0 ) ) );
      af6.setAttribute( QStringLiteral( "col1" ), 8 );
      af6.setAttribute( QStringLiteral( "col2" ), "test4" );
      af6.setAttribute( QStringLiteral( "col3" ), 3 );
      af6.setAttribute( QStringLiteral( "col4" ), "test" );
      mAggregatesLayer->dataProvider()->addFeatures( QgsFeatureList() << af1 << af2 << af3 << af4 << af5 << af6 );
      QgsProject::instance()->addMapLayer( mAggregatesLayer );

      mChildLayer = new QgsVectorLayer( QStringLiteral( "Point?field=parent:integer&field=col2:string&field=col3:integer" ), QStringLiteral( "child_layer" ), QStringLiteral( "memory" ) );
      QVERIFY( mChildLayer->isValid() );
      QgsFeature cf1( mChildLayer->dataProvider()->fields(), 1 );
      cf1.setAttribute( QStringLiteral( "parent" ), 4 );
      cf1.setAttribute( QStringLiteral( "col2" ), "test" );
      cf1.setAttribute( QStringLiteral( "col3" ), 2 );
      QgsFeature cf2( mChildLayer->dataProvider()->fields(), 2 );
      cf2.setAttribute( QStringLiteral( "parent" ), 4 );
      cf2.setAttribute( QStringLiteral( "col2" ), QVariant( QVariant::String ) );
      cf2.setAttribute( QStringLiteral( "col3" ), 1 );
      QgsFeature cf3( mChildLayer->dataProvider()->fields(), 3 );
      cf3.setAttribute( QStringLiteral( "parent" ), 4 );
      cf3.setAttribute( QStringLiteral( "col2" ), "test333" );
      cf3.setAttribute( QStringLiteral( "col3" ), 2 );
      QgsFeature cf4( mChildLayer->dataProvider()->fields(), 4 );
      cf4.setAttribute( QStringLiteral( "parent" ), 3 );
      cf4.setAttribute( QStringLiteral( "col2" ), "test4" );
      cf4.setAttribute( QStringLiteral( "col3" ), 2 );
      QgsFeature cf5( mChildLayer->dataProvider()->fields(), 5 );
      cf5.setAttribute( QStringLiteral( "parent" ), 3 );
      cf5.setAttribute( QStringLiteral( "col2" ), QVariant( QVariant::String ) );
      cf5.setAttribute( QStringLiteral( "col3" ), 7 );
      mChildLayer->dataProvider()->addFeatures( QgsFeatureList() << cf1 << cf2 << cf3 << cf4 << cf5 );
      QgsProject::instance()->addMapLayer( mChildLayer );

      QgsRelation rel;
      rel.setId( QStringLiteral( "my_rel" ) );
      rel.setName( QStringLiteral( "relation name" ) );
      rel.setReferencedLayer( mAggregatesLayer->id() );
      rel.setReferencingLayer( mChildLayer->id() );
      rel.addFieldPair( QStringLiteral( "parent" ), QStringLiteral( "col1" ) );
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
      QTest::newRow( "invalid function not known no args" ) << "watwat()" << false;
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

    void parsing_error_line_column_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<int>( "firstLine" );
      QTest::addColumn<int>( "firstColumn" );
      QTest::addColumn<int>( "lastLine" );
      QTest::addColumn<int>( "lastColumn" );

      // invalid strings
      QTest::newRow( "No close brace" ) << "(" << 1 << 1 << 1 << 2;
      QTest::newRow( "No close brace 2" ) << "to_string(" << 1 << 10 << 1 << 11;
      QTest::newRow( "No close brace 2 - Multiline" ) << "to_string\n(" << 2 << 0 << 2 << 1;
    }

    void parsing_error_line_column()
    {
      QFETCH( QString, string );
      QFETCH( int, firstLine );
      QFETCH( int, firstColumn );
      QFETCH( int, lastLine );
      QFETCH( int, lastColumn );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), true );
      QCOMPARE( exp.parserErrors().first().firstLine, firstLine );
      QCOMPARE( exp.parserErrors().first().firstColumn, firstColumn );
      QCOMPARE( exp.parserErrors().first().lastLine, lastLine );
      QCOMPARE( exp.parserErrors().first().lastColumn, lastColumn );
    }

    void max_errors()
    {
      QgsExpression e( "wkt_geom&#x9;OBJECTID&#x9;id&#x9;a&#x9;b&#x9;c&#x9;d&#x9;e&#x9;f&#x9;g&#x9;h&#x9;i&#x9;j&#x9;k&#x9;l&#x9;m&#x9;n&#x9;o&#x9;p&#x9;q&#x9;r&#x9;" );
      QVERIFY( e.hasParserError() );
      // want parsing to abort after a maximum of 10 errors
      QCOMPARE( e.parserErrors().count(), 10 );
    }

    void parsing_with_locale()
    {
      // check that parsing of numbers works correctly even when using some other locale

      char *old_locale = setlocale( LC_NUMERIC, nullptr );
      qDebug( "Old locale: %s", old_locale );
      setlocale( LC_NUMERIC, "de_DE.UTF8" );
      char *new_locale = setlocale( LC_NUMERIC, nullptr );
      qDebug( "New locale: %s", new_locale );

      QgsExpression exp( QStringLiteral( "1.23 + 4.56" ) );
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
      QTest::newRow( "optional parameters unspecified" ) << "wordwrap( text:='test string', length:=5 )" << false << "wordwrap('test string', 5, '')" << QVariant( "test\nstring" );
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
      QVERIFY( exp.prepare( &context ) );

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

    void represent_value()
    {
      QVariantMap config;
      QVariantMap map;
      map.insert( QStringLiteral( "one" ), QStringLiteral( "1" ) );
      map.insert( QStringLiteral( "two" ), QStringLiteral( "2" ) );
      map.insert( QStringLiteral( "three" ), QStringLiteral( "3" ) );

      config.insert( QStringLiteral( "map" ), map );
      mPointsLayer->setEditorWidgetSetup( 3, QgsEditorWidgetSetup( QStringLiteral( "ValueMap" ), config ) );

      // Usage on a value map
      QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mPointsLayer ) );
      QgsExpression expression( "represent_value(\"Pilots\", 'Pilots')" );
      if ( expression.hasParserError() )
        qDebug() << expression.parserErrorString();
      QVERIFY( !expression.hasParserError() );
      if ( expression.hasEvalError() )
        qDebug() << expression.evalErrorString();
      QVERIFY( !expression.hasEvalError() );
      expression.prepare( &context );

      QgsFeature feature;
      mPointsLayer->getFeatures( QgsFeatureRequest().setFilterExpression( "Pilots = 1" ) ).nextFeature( feature );
      context.setFeature( feature );
      QCOMPARE( expression.evaluate( &context ).toString(), QStringLiteral( "one" ) );

      // Usage on a simple string
      QgsExpression expression2( "represent_value(\"Class\", 'Class')" );
      if ( expression2.hasParserError() )
        qDebug() << expression2.parserErrorString();
      QVERIFY( !expression2.hasParserError() );
      if ( expression2.hasEvalError() )
        qDebug() << expression2.evalErrorString();
      QVERIFY( !expression2.hasEvalError() );
      expression2.prepare( &context );
      mPointsLayer->getFeatures( QgsFeatureRequest().setFilterExpression( "Class = 'Jet'" ) ).nextFeature( feature );
      context.setFeature( feature );
      QCOMPARE( expression2.evaluate( &context ).toString(), QStringLiteral( "Jet" ) );

      // Test with implicit field name discovery
      QgsExpression expression3( "represent_value(\"Pilots\")" );
      if ( expression3.hasParserError() )
        qDebug() << expression.parserErrorString();
      QVERIFY( !expression3.hasParserError() );
      if ( expression3.hasEvalError() )
        qDebug() << expression3.evalErrorString();
      QVERIFY( !expression3.hasEvalError() );

      mPointsLayer->getFeatures( QgsFeatureRequest().setFilterExpression( "Pilots = 1" ) ).nextFeature( feature );
      context.setFeature( feature );

      QCOMPARE( expression.evaluate( &context ).toString(), QStringLiteral( "one" ) );
      expression3.prepare( &context );
      QCOMPARE( expression.evaluate( &context ).toString(), QStringLiteral( "one" ) );

      QgsExpression expression4( "represent_value('Class')" );
      expression4.evaluate();
      if ( expression4.hasParserError() )
        qDebug() << expression4.parserErrorString();
      QVERIFY( !expression4.hasParserError() );
      if ( expression4.hasEvalError() )
        qDebug() << expression4.evalErrorString();
      QVERIFY( expression4.hasEvalError() );

      expression4.prepare( &context );
      if ( expression4.hasParserError() )
        qDebug() << expression4.parserErrorString();
      QVERIFY( !expression4.hasParserError() );
      if ( expression4.hasEvalError() )
        qDebug() << expression4.evalErrorString();
      QVERIFY( expression4.hasEvalError() );
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
      QTest::newRow( "like 2" ) << "'hello' like '_el%'" << false << QVariant( 1 );
      QTest::newRow( "like 3" ) << "'hello' like 'lo'" << false << QVariant( 0 );
      QTest::newRow( "like 4" ) << "'hello' like '%LO'" << false << QVariant( 0 );
      QTest::newRow( "ilike" ) << "'hello' ilike '%LO'" << false << QVariant( 1 );
      // the \\\\ is like \\ in the interface
      QTest::newRow( "like escape 1" ) << "'1%' like '1\\\\%'" << false << QVariant( 1 );
      QTest::newRow( "like escape 2" ) << "'1_' like '1\\\\%'" << false << QVariant( 0 );
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
      QTest::newRow( "round(1234.4) - round down to int" ) << "round(1234.4)" << false << QVariant( 1234 );
      QTest::newRow( "max(1)" ) << "max(1)" << false << QVariant( 1. );
      QTest::newRow( "max(1,3.5,-2.1)" ) << "max(1,3.5,-2.1)" << false << QVariant( 3.5 );
      QTest::newRow( "max(3.5,-2.1,1)" ) << "max(3.5,-2.1,1)" << false << QVariant( 3.5 );
      QTest::newRow( "max with null value" ) << "max(1,3.5,null)" << false << QVariant( 3.5 );
      QTest::newRow( "max with null value first" ) << "max(null,-3.5,2)" << false << QVariant( 2. );
      QTest::newRow( "max with no params" ) << "max()" << false << QVariant( QVariant::Double );
      QTest::newRow( "max with only null value" ) << "max(null)" << false << QVariant( QVariant::Double );
      QTest::newRow( "min(-1.5)" ) << "min(-1.5)" << false << QVariant( -1.5 );
      QTest::newRow( "min(-16.6,3.5,-2.1)" ) << "min(-16.6,3.5,-2.1)" << false << QVariant( -16.6 );
      QTest::newRow( "min(5,3.5,-2.1)" ) << "min(5,3.5,-2.1)" << false << QVariant( -2.1 );
      QTest::newRow( "min with null value" ) << "min(5,null,-2.1)" << false << QVariant( -2.1 );
      QTest::newRow( "min with null value first" ) << "min(null,3.2,6.5)" << false << QVariant( 3.2 );
      QTest::newRow( "min with no params" ) << "min()" << false << QVariant( QVariant::Double );
      QTest::newRow( "min with only null value" ) << "min(null)" << false << QVariant( QVariant::Double );
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

      // DMS conversion
      QTest::newRow( "X coordinate to degree minute aligned" ) << "to_dm(6.3545681,'x',2,'aligned')" << false << QVariant( "6°21.27′E" );
      QTest::newRow( "X coordinate to degree minute with suffix" ) << "to_dm(6.3545681,'x',2,'suffix')" << false << QVariant( "6°21.27′E" );
      QTest::newRow( "X coordinate to degree minute without formatting" ) << "to_dm(6.3545681,'x',2,'')" << false << QVariant( "6°21.27′" );
      QTest::newRow( "X coordinate to degree minute" ) << "to_dm(6.3545681,'x',2)" << false << QVariant( "6°21.27′" );
      QTest::newRow( "Y coordinate to degree minute second aligned" ) << "to_dms(6.3545681,'y',2,'aligned')" << false << QVariant( "6°21′16.45″N" );
      QTest::newRow( "Y coordinate to degree minute second with suffix" ) << "to_dms(6.3545681,'y',2,'suffix')" << false << QVariant( "6°21′16.45″N" );
      QTest::newRow( "Y coordinate to degree minute second without formatting" ) << "to_dms(6.3545681,'y',2,'')" << false << QVariant( "6°21′16.45″" );
      QTest::newRow( "Y coordinate to degree minute second" ) << "to_dms(6.3545681,'y',2)" << false << QVariant( "6°21′16.45″" );

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
      QTest::newRow( "nodes_to_points point" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POINT(1 2)')))" << false << QVariant( QStringLiteral( "MultiPoint ((1 2))" ) );
      QTest::newRow( "nodes_to_points polygon" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( QStringLiteral( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-1 -1))" ) );
      QTest::newRow( "nodes_to_points polygon with rings" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))')))" << false
          << QVariant( QStringLiteral( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-1 -1),(-0.1 -0.1),(0.4 0),(0.4 0.2),(0 0.2),(-0.1 -0.1),(-0.3 -0.9),(-0.3 0),(4 -0.1),(0.1 2.1),(-0.3 -0.9))" ) );
      QTest::newRow( "nodes_to_points line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false
                                              << QVariant( QStringLiteral( "MultiPoint ((0 0),(1 1),(2 2))" ) );
      QTest::newRow( "nodes_to_points collection 1" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))')))" << false
          << QVariant( QStringLiteral( "MultiPoint ((0 1),(0 0),(1 0),(1 1))" ) );
      QTest::newRow( "nodes_to_points collection 2" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION(POINTZM(0 1 2 3), POINTZM(0 0 3 4), POINTZM(1 1 5 6), POLYGONZM((-1 -1 7 8, 4 0 1 2, 4 2 7 6, 0 2 1 3, -1 -1 7 8),(-0.1 -0.1 5 4, 0.4 0 9 8, 0.4 0.2 7 10, 0 0.2 0 0, -0.1 -0.1 5 4),(-1 -1 0 0, 4 0 0 1, 4 2 1 2, 0 2 2 3, -1 -1 0 0)), POINTZM(1 0 1 2))')))" << false
          << QVariant( QStringLiteral( "MultiPointZM ((0 1 2 3),(0 0 3 4),(1 1 5 6),(-1 -1 7 8),(4 0 1 2),(4 2 7 6),(0 2 1 3),(-1 -1 7 8),(-0.1 -0.1 5 4),(0.4 0 9 8),(0.4 0.2 7 10),(0 0.2 0 0),(-0.1 -0.1 5 4),(-1 -1 0 0),(4 0 0 1),(4 2 1 2),(0 2 2 3),(-1 -1 0 0),(1 0 1 2))" ) );
      QTest::newRow( "nodes_to_points empty collection" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION()')))" << false <<
          QVariant( QStringLiteral( "MultiPoint ()" ) );
      QTest::newRow( "nodes_to_points no close polygon" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'),true))" << false << QVariant( QStringLiteral( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2))" ) );
      QTest::newRow( "nodes_to_points no close polygon with rings" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))'),true))" << false
          << QVariant( QStringLiteral( "MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-0.1 -0.1),(0.4 0),(0.4 0.2),(0 0.2),(-0.3 -0.9),(-0.3 0),(4 -0.1),(0.1 2.1))" ) );
      QTest::newRow( "nodes_to_points no close unclosed line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'),true))" << false
          << QVariant( QStringLiteral( "MultiPoint ((0 0),(1 1),(2 2))" ) );
      QTest::newRow( "nodes_to_points no close closed line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2, 0 0)'),true))" << false
          << QVariant( QStringLiteral( "MultiPoint ((0 0),(1 1),(2 2))" ) );
      QTest::newRow( "segments_to_lines not geom" ) << "segments_to_lines('g')" << true << QVariant();
      QTest::newRow( "segments_to_lines null" ) << "segments_to_lines(NULL)" << false << QVariant();
      QTest::newRow( "segments_to_lines point" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POINT(1 2)')))" << false << QVariant( QStringLiteral( "MultiLineString ()" ) );
      QTest::newRow( "segments_to_lines polygon" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( QStringLiteral( "MultiLineString ((-1 -1, 4 0),(4 0, 4 2),(4 2, 0 2),(0 2, -1 -1))" ) );
      QTest::newRow( "segments_to_lines polygon with rings" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))')))" << false
          << QVariant( QStringLiteral( "MultiLineString ((-1 -1, 4 0),(4 0, 4 2),(4 2, 0 2),(0 2, -1 -1),(-0.1 -0.1, 0.4 0),(0.4 0, 0.4 0.2),(0.4 0.2, 0 0.2),(0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0),(-0.3 0, 4 -0.1),(4 -0.1, 0.1 2.1),(0.1 2.1, -0.3 -0.9))" ) );
      QTest::newRow( "segments_to_lines line" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false
          << QVariant( QStringLiteral( "MultiLineString ((0 0, 1 1),(1 1, 2 2))" ) );
      QTest::newRow( "segments_to_lines collection 1" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))')))" << false
          << QVariant( QStringLiteral( "MultiLineString ()" ) );
      QTest::newRow( "segments_to_lines collection 2" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION(POINTZM(0 1 2 3), LINESTRINGZM(0 0 1 2, 1 1 3 4, 2 2 5 6), POINTZM(1 1 5 6), POLYGONZM((-1 -1 7 8, 4 0 1 2, 4 2 7 6, 0 2 1 3, -1 -1 7 8)), POINTZM(1 0 1 2))')))" << false
          << QVariant( QStringLiteral( "MultiLineStringZM ((0 0 1 2, 1 1 3 4),(1 1 3 4, 2 2 5 6),(-1 -1 7 8, 4 0 1 2),(4 0 1 2, 4 2 7 6),(4 2 7 6, 0 2 1 3),(0 2 1 3, -1 -1 7 8))" ) );
      QTest::newRow( "segments_to_lines empty collection" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION()')))" << false <<
          QVariant( QStringLiteral( "MultiLineString ()" ) );
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
          << QVariant( QStringLiteral( "LineString (-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1)" ) );
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
      QTest::newRow( "geometry_n collection" ) << "geom_to_wkt(geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),3))" << false << QVariant( QStringLiteral( "Point (1 0)" ) );
      QTest::newRow( "geometry_n collection bad index 1" ) << "geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),0)" << false << QVariant();
      QTest::newRow( "geometry_n collection bad index 2" ) << "geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),5)" << false << QVariant();
      QTest::newRow( "force_rhr not geom" ) << "force_rhr('g')" << true << QVariant();
      QTest::newRow( "force_rhr null" ) << "force_rhr(NULL)" << false << QVariant();
      QTest::newRow( "force_rhr point" ) << "geom_to_wkt(force_rhr(geom_from_wkt('POINT(1 2)')))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "force_rhr polygon" ) << "geom_to_wkt(force_rhr(geometry:=geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( "Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1))" );
      QTest::newRow( "force_rhr multipolygon" ) << "geom_to_wkt(force_rhr(geometry:=geom_from_wkt('MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))')))" << false << QVariant( "MultiPolygon (((-1 -1, 0 2, 4 2, 4 0, -1 -1)),((100 100, 100 200, 200 200, 200 100, 100 100)))" );
      QTest::newRow( "force_rhr line" ) << "geom_to_wkt(force_rhr(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false << QVariant( "LineString (0 0, 1 1, 2 2)" );
      QTest::newRow( "boundary not geom" ) << "boundary('g')" << true << QVariant();
      QTest::newRow( "boundary null" ) << "boundary(NULL)" << false << QVariant();
      QTest::newRow( "boundary point" ) << "boundary(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "boundary polygon" ) << "geom_to_wkt(boundary(geometry:=geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( "LineString (-1 -1, 4 0, 4 2, 0 2, -1 -1)" );
      QTest::newRow( "boundary line" ) << "geom_to_wkt(boundary(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false << QVariant( "MultiPoint ((0 0),(2 2))" );
      QTest::newRow( "line_merge not geom" ) << "line_merge('g')" << true << QVariant();
      QTest::newRow( "line_merge null" ) << "line_merge(NULL)" << false << QVariant();
      QTest::newRow( "line_merge point" ) << "line_merge(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "line_merge line" ) << "geom_to_wkt(line_merge(geometry:=geom_from_wkt('LineString(0 0, 10 10)')))" << false << QVariant( "LineString (0 0, 10 10)" );
      QTest::newRow( "line_merge multiline" ) << "geom_to_wkt(line_merge(geom_from_wkt('MultiLineString((0 0, 10 10),(10 10, 20 20))')))" << false << QVariant( "LineString (0 0, 10 10, 20 20)" );
      QTest::newRow( "offset_curve not geom" ) << "offset_curve('g', 5)" << true << QVariant();
      QTest::newRow( "offset_curve null" ) << "offset_curve(NULL, 5)" << false << QVariant();
      QTest::newRow( "offset_curve point" ) << "offset_curve(geom_from_wkt('POINT(1 2)'),5)" << false << QVariant();
      QTest::newRow( "offset_curve line" ) << "geom_to_wkt(offset_curve(geom_from_wkt('LineString(0 0, 10 0)'),1,segments:=4))" << false << QVariant( "LineString (10 1, 0 1)" );
      QTest::newRow( "offset_curve line miter" ) << "geom_to_wkt(offset_curve(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),distance:=-1,join:=2,miter_limit:=1))" << false << QVariant( "LineString (0 -1, 10 -1)" );
      QTest::newRow( "offset_curve line bevel" ) << "geom_to_wkt(offset_curve(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),distance:=1,join:=3))" << false << QVariant( "LineString (0 1, 9 1, 9 10)" );
      QTest::newRow( "wedge_buffer not geom" ) << "wedge_buffer('g', 0, 45, 1)" << true << QVariant();
      QTest::newRow( "wedge_buffer null" ) << "wedge_buffer(NULL, 0, 45, 1)" << false << QVariant();
      QTest::newRow( "wedge_buffer point" ) << "geom_to_wkt(wedge_buffer(center:=geom_from_wkt('POINT(1 2)'),azimuth:=90,width:=180,outer_radius:=1))" << false << QVariant( QStringLiteral( "CurvePolygon (CompoundCurve (CircularString (1 3, 2 2, 1 1),(1 1, 1 2),(1 2, 1 3)))" ) );
      QTest::newRow( "wedge_buffer point inner" ) << "geom_to_wkt(wedge_buffer(center:=geom_from_wkt('POINT(1 2)'),azimuth:=90,width:=180,outer_radius:=2,inner_radius:=1))" << false << QVariant( QStringLiteral( "CurvePolygon (CompoundCurve (CircularString (1 4, 3 2, 1 0),(1 0, 1 1),CircularString (1 1, 0 2, 1 3),(1 3, 1 4)))" ) );
      QTest::newRow( "tapered_buffer not geom" ) << "tapered_buffer('g', 1, 2, 8)" << true << QVariant();
      QTest::newRow( "tapered_buffer null" ) << "tapered_buffer(NULL, 1, 2, 8)" << false << QVariant();
      QTest::newRow( "tapered_buffer point" ) << "geom_to_wkt(tapered_buffer(geometry:=geom_from_wkt('POINT(1 2)'),start_width:=1,end_width:=2,segments:=10))" << true << QVariant();
      QTest::newRow( "tapered_buffer line" ) << "geom_to_wkt(tapered_buffer(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),start_width:=1,end_width:=2,segments:=3))" << false << QVariant( QStringLiteral( "MultiPolygon (((-0 -0.5, -0.25 -0.4330127, -0.4330127 -0.25, -0.5 0, -0.4330127 0.25, -0.25 0.4330127, 0 0.5, 10 1, 10.5 0.8660254, 10.8660254 0.5, 11 -0, 10.8660254 -0.5, 10.5 -0.8660254, 10 -1, -0 -0.5)))" ) );
      QTest::newRow( "tapered_buffer line 2" ) << "geom_to_wkt(tapered_buffer(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),start_width:=2,end_width:=1,segments:=3))" << false << QVariant( QStringLiteral( "MultiPolygon (((-0 -1, -0.5 -0.8660254, -0.8660254 -0.5, -1 0, -0.8660254 0.5, -0.5 0.8660254, 0 1, 10 0.5, 10.25 0.4330127, 10.4330127 0.25, 10.5 -0, 10.4330127 -0.25, 10.25 -0.4330127, 10 -0.5, -0 -1)))" ) );
      QTest::newRow( "buffer_by_m not geom" ) << "buffer_by_m('g', 8)" << true << QVariant();
      QTest::newRow( "buffer_by_m null" ) << "buffer_by_m(NULL, 8)" << false << QVariant();
      QTest::newRow( "buffer_by_m point" ) << "geom_to_wkt(buffer_by_m(geometry:=geom_from_wkt('POINT(1 2)'),segments:=10))" << true << QVariant();
      QTest::newRow( "buffer_by_m line" ) << "geom_to_wkt(buffer_by_m(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),segments:=3))" << false << QVariant( QStringLiteral( "GeometryCollection ()" ) );
      QTest::newRow( "buffer_by_m linem" ) << "geom_to_wkt(buffer_by_m(geometry:=geom_from_wkt('LineStringM(0 0 1, 10 0 2)'),segments:=3))" << false << QVariant( QStringLiteral( "MultiPolygon (((-0 -0.5, -0.25 -0.4330127, -0.4330127 -0.25, -0.5 0, -0.4330127 0.25, -0.25 0.4330127, 0 0.5, 10 1, 10.5 0.8660254, 10.8660254 0.5, 11 -0, 10.8660254 -0.5, 10.5 -0.8660254, 10 -1, -0 -0.5)))" ) );
      QTest::newRow( "single_sided_buffer not geom" ) << "single_sided_buffer('g', 5)" << true << QVariant();
      QTest::newRow( "single_sided_buffer null" ) << "single_sided_buffer(NULL, 5)" << false << QVariant();
      QTest::newRow( "single_sided_buffer point" ) << "single_sided_buffer(geom_from_wkt('POINT(1 2)'),5)" << false << QVariant();
      QTest::newRow( "single_sided_buffer line" ) << "geom_to_wkt(single_sided_buffer(geom_from_wkt('LineString(0 0, 10 0)'),1,segments:=4))" << false << QVariant( "Polygon ((10 0, 0 0, 0 1, 10 1, 10 0))" );
      QTest::newRow( "single_sided_buffer line miter" ) << "geom_to_wkt(single_sided_buffer(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),distance:=-1,join:=2,miter_limit:=1))" << false << QVariant( "Polygon ((0 0, 10 0, 10 -1, 0 -1, 0 0))" );
      QTest::newRow( "single_sided_buffer line bevel" ) << "geom_to_wkt(single_sided_buffer(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),distance:=1,join:=3))" << false << QVariant( "Polygon ((10 10, 10 0, 0 0, 0 1, 9 1, 9 10, 10 10))" );
      QTest::newRow( "extend not geom" ) << "extend('g', 1, 2)" << true << QVariant();
      QTest::newRow( "extend null" ) << "extend(NULL, 1, 2)" << false << QVariant();
      QTest::newRow( "extend point" ) << "extend(geom_from_wkt('POINT(1 2)'),1,2)" << false << QVariant();
      QTest::newRow( "extend line" ) << "geom_to_wkt(extend(geom_from_wkt('LineString(0 0, 1 0, 1 1)'),1,2))" << false << QVariant( "LineString (-1 0, 1 0, 1 3)" );
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
      QTest::newRow( "reverse multiline" ) << "geom_to_wkt(reverse(geom_from_wkt('MULTILINESTRING((0 0, 1 1, 2 2),(10 10, 11 11, 12 12))')))" << false << QVariant( "MultiLineString ((2 2, 1 1, 0 0),(12 12, 11 11, 10 10))" );
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
      QTest::newRow( "pole_of_inaccessibility polygon" ) << "round(x(pole_of_inaccessibility( geomFromWKT('POLYGON((0 1,0 9,3 10,3 3, 10 3, 10 1, 0 1))'), 0.1))*100)" << false << QVariant( 155 );
      QTest::newRow( "pole_of_inaccessibility polygon" ) << "round(y(pole_of_inaccessibility( geomFromWKT('POLYGON((0 1,0 9,3 10,3 3, 10 3, 10 1, 0 1))'), 0.1))*100)" << false << QVariant( 255 );
      QTest::newRow( "pole_of_inaccessibility not poly" ) << "geom_to_wkt(pole_of_inaccessibility( geomFromWKT('POINT (1.5 0.5)'), 0.1 ))" << false << QVariant();
      QTest::newRow( "pole_of_inaccessibility not geom" ) << "pole_of_inaccessibility('g',0.1)" << true << QVariant();
      QTest::newRow( "pole_of_inaccessibility null" ) << "pole_of_inaccessibility(NULL,0.1)" << false << QVariant();
      QTest::newRow( "is_closed not geom" ) << "is_closed('g')" << true << QVariant();
      QTest::newRow( "is_closed null" ) << "is_closed(NULL)" << false << QVariant();
      QTest::newRow( "is_closed point" ) << "is_closed(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "is_closed polygon" ) << "is_closed(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant();
      QTest::newRow( "is_closed not closed" ) << "is_closed(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant( false );
      QTest::newRow( "is_closed closed" ) << "is_closed(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2, 0 0)'))" << false << QVariant( true );
      QTest::newRow( "is_closed multiline" ) << "is_closed(geom_from_wkt('MultiLineString ((6501338.13976828 4850981.51459331, 6501343.09036573 4850984.01453377, 6501338.13976828 4850988.96491092, 6501335.63971657 4850984.01453377, 6501338.13976828 4850981.51459331))'))" << false << QVariant( true );
      QTest::newRow( "is_closed multiline" ) << "is_closed(geom_from_wkt('MultiLineString ((6501338.13976828 4850981.51459331, 6501343.09036573 4850984.01453377, 6501338.13976828 4850988.96491092, 6501335.63971657 4850984.01453377, 6501438.13976828 4850981.51459331))'))" << false << QVariant( false );
      QTest::newRow( "is_closed multiline" ) << "is_closed(geom_from_wkt('MultiLineString ()'))" << false << QVariant();
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
      QTest::newRow( "make_triangle not geom" ) << "geom_to_wkt(make_triangle(make_point(2,4), 'g', make_point(3,5)))" << true << QVariant();
      QTest::newRow( "make_triangle null" ) << "geom_to_wkt(make_triangle(make_point(2,4), NULL, make_point(3,5)))" << false << QVariant();
      QTest::newRow( "make_triangle duplicated point" ) << "geom_to_wkt(make_triangle(make_point(2,4), make_point(2,4), make_point(3,5)))" << false << QVariant( "Triangle ((2 4, 2 4, 3 5, 2 4))" );
      QTest::newRow( "make_triangle colinear points" ) << "geom_to_wkt(make_triangle(make_point(0,1), make_point(0,2), make_point(0,3)))" << false << QVariant( "Triangle ((0 1, 0 2, 0 3, 0 1))" );
      QTest::newRow( "make_triangle" ) << "geom_to_wkt(make_triangle(make_point(0,0), make_point(5,5), make_point(0,10)))" << false << QVariant( "Triangle ((0 0, 5 5, 0 10, 0 0))" );
      QTest::newRow( "make_circle not geom" ) << "make_circle('a', 5)" << true << QVariant();
      QTest::newRow( "make_circle null" ) << "make_circle(NULL, 5)" << false << QVariant();
      QTest::newRow( "make_circle bad" ) << "make_circle(make_line(make_point(1,2), make_point(3,4)), 5)" << false << QVariant();
      QTest::newRow( "make_circle" ) << "geom_to_wkt(make_circle(make_point(10,10), 5, 4))" << false << QVariant( "Polygon ((10 15, 15 10, 10 5, 5 10, 10 15))" );
      QTest::newRow( "make_ellipse not geom" ) << "make_ellipse('a', 5, 2, 0)" << true << QVariant();
      QTest::newRow( "make_ellipse null" ) << "make_ellipse(NULL, 5, 2, 0)" << false << QVariant();
      QTest::newRow( "make_ellipse bad" ) << "make_ellipse(make_line(make_point(1,2), make_point(3,4)), 5, 2, 0)" << false << QVariant();
      QTest::newRow( "make_ellipse" ) << "geom_to_wkt(make_ellipse(make_point(10,10), 5, 2, 90, 4))" << false << QVariant( "Polygon ((15 10, 10 8, 5 10, 10 12, 15 10))" );
      QTest::newRow( "make_regular_polygon not geom (center)" ) << "make_regular_polygon('a', make_point(0,5), 5)" << true << QVariant();
      QTest::newRow( "make_regular_polygon not geom (vertice)" ) << "make_regular_polygon(make_point(0,0), 'a', 5)" << true << QVariant();
      QTest::newRow( "make_regular_polygon bad (center)" ) << "make_regular_polygon(make_line(make_point(1,2), make_point(3,4)), make_point(0,5), 5)" << false << QVariant();
      QTest::newRow( "make_regular_polygon bad (vertice)" ) << "make_regular_polygon(make_point(0,0), make_line(make_point(1,2), make_point(3,4)), 5)" << false << QVariant();
      QTest::newRow( "make_regular_polygon bad (numEdges < 3)" ) << "make_regular_polygon(make_point(0,0), make_point(0,5), 2)" << true << QVariant();
      QTest::newRow( "make_regular_polygon bad (invalid option)" ) << "make_regular_polygon(make_point(0,0), make_point(0,5), 5, 5)" << true << QVariant();
      QTest::newRow( "make_regular_polygon bad (numEdges < 3)" ) << "make_regular_polygon(make_point(0,0), make_point(0,5), 2)" << true << QVariant();
      QTest::newRow( "make_regular_polygon" ) << "geom_to_wkt(make_regular_polygon(make_point(0,0), make_point(0,5), 5), 2)" << false << QVariant( "Polygon ((0 5, 4.76 1.55, 2.94 -4.05, -2.94 -4.05, -4.76 1.55, 0 5))" );
      QTest::newRow( "make_regular_polygon" ) << "geom_to_wkt(make_regular_polygon(make_point(0,0), project(make_point(0,0), 4.0451, radians(36)), 5, 1), 2)" << false << QVariant( "Polygon ((0 5, 4.76 1.55, 2.94 -4.05, -2.94 -4.05, -4.76 1.55, 0 5))" );
      QTest::newRow( "make_square not geom (point 1)" ) << "make_square(make_line(make_point(1,2), make_point(3,4)), make_point(5,5))" << false << QVariant();
      QTest::newRow( "make_square not geom (point 2)" ) << "make_square(make_point(0,0), make_line(make_point(1,2), make_point(3,4)))" << false << QVariant();
      QTest::newRow( "make_square bad (point 1)" ) << "make_square('a', make_point(5,5))" << true << QVariant();
      QTest::newRow( "make_square bad (point 2)" ) << "make_square(make_point(0,0), 'a')" << true << QVariant();
      QTest::newRow( "make_square" ) << "geom_to_wkt(make_square(make_point(5, 5), make_point(1, 1)))" << false << QVariant( "Polygon ((5 5, 5 1, 1 1, 1 5, 5 5))" );
      QTest::newRow( "make_rectangle_3points not geom (point 1)" ) << "make_rectangle_3points( make_line(make_point(1,2), make_point(3,4)), make_point(0,5), make_point(5,5))" << false << QVariant();
      QTest::newRow( "make_rectangle_3points not geom (point 2)" ) << "make_rectangle_3points(make_point(0,0), make_line(make_point(1,2), make_point(3,4)), make_point(5,5))" << false << QVariant();
      QTest::newRow( "make_rectangle_3points not geom (point 3)" ) << "make_rectangle_3points(make_point(0,0), make_point(0,5), make_line(make_point(1,2), make_point(3,4)))" << false << QVariant();
      QTest::newRow( "make_rectangle_3points bad (point 1)" ) << "make_rectangle_3points('a', make_point(0,5), make_point(5,5))" << true << QVariant();
      QTest::newRow( "make_rectangle_3points bad (point 2)" ) << "make_rectangle_3points(make_point(0,0), 'a', make_point(5,5))" << true << QVariant();
      QTest::newRow( "make_rectangle_3points bad (point 3)" ) << "make_rectangle_3points(make_point(0,0), make_point(0,5), 'a')" << true << QVariant();
      QTest::newRow( "make_rectangle_3points bad (invalid option)" ) << "make_rectangle_3points(make_point(0,0), make_point(0,5), make_point(5,5), 2)" << true << QVariant();
      QTest::newRow( "make_rectangle_3points (distance default)" ) << "geom_to_wkt(make_rectangle_3points(make_point(0, 0), make_point(0,5), make_point(5, 5)))" << false << QVariant( "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0))" );
      QTest::newRow( "make_rectangle_3points (distance)" ) << "geom_to_wkt(make_rectangle_3points(make_point(0, 0), make_point(0,5), make_point(5, 5), 0))" << false << QVariant( "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0))" );
      QTest::newRow( "make_rectangle_3points (projected)" ) << "geom_to_wkt(make_rectangle_3points(make_point(0, 0), make_point(0,5), make_point(5, 3), 1))" << false << QVariant( "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0))" );
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
      QTest::newRow( "project y" ) << "toint(y(project( point:=make_point( 1, 2 ), distance:=3, azimuth:=radians(270)))*1000000)" << false << QVariant( 2 * 1000000 );
      QTest::newRow( "project m value preserved" ) << "geom_to_wkt(project( make_point( 1, 2, 2, 5), 1, 0.0, 0.0 ) )" << false << QVariant( "PointZM (1 2 3 5)" );
      QTest::newRow( "project 2D Point" ) << "geom_to_wkt(project( point:=make_point( 1, 2), distance:=1, azimuth:=radians(0), elevation:=0 ) )" << false << QVariant( "PointZ (1 2 nan)" );
      QTest::newRow( "project 3D Point" ) << "geom_to_wkt(project( make_point( 1, 2, 2), 5, radians(450), radians (450) ) )" << false << QVariant( "PointZ (6 2 2)" );
      QTest::newRow( "inclination not geom first" ) << "inclination( 'a', make_point( 1, 2, 2 ) )" << true << QVariant();
      QTest::newRow( "inclination not geom second" ) << " inclination( make_point( 1, 2, 2 ), 'a' )" << true << QVariant();
      QTest::newRow( "inclination not point first" ) << "inclination( geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'), make_point( 1, 2, 2) )" << true << QVariant();
      QTest::newRow( "inclination not point second" ) << " inclination( make_point( 1, 2, 2 ), geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)') )" << true << QVariant();
      QTest::newRow( "inclination" ) << "ceil(inclination( make_point( 159, 753, 460 ), make_point( 123, 456, 789 ) ))" << false << QVariant( 43.0 );
      QTest::newRow( "inclination" ) << " inclination( make_point( 5, 10, 0 ), make_point( 5, 10, 5 ) )" << false << QVariant( 0.0 );
      QTest::newRow( "inclination" ) << " inclination( make_point( 5, 10, 0 ), make_point( 5, 10, 0 ) )" << false << QVariant( 90.0 );
      QTest::newRow( "inclination" ) << " inclination( make_point( 5, 10, 0 ), make_point( 5, 10, -5 ) )" << false << QVariant( 180.0 );
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
      QTest::newRow( "line_interpolate_point not geom" ) << "line_interpolate_point('g', 5)" << true << QVariant();
      QTest::newRow( "line_interpolate_point null" ) << "line_interpolate_point(NULL, 5)" << false << QVariant();
      QTest::newRow( "line_interpolate_point point" ) << "line_interpolate_point(geom_from_wkt('POINT(1 2)'),5)" << false << QVariant();
      QTest::newRow( "line_interpolate_point line" ) << "geom_to_wkt(line_interpolate_point(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),distance:=5))" << false << QVariant( "Point (5 0)" );
      QTest::newRow( "line_locate_point not geom" ) << "line_locate_point('g', geom_from_wkt('Point 5 0'))" << false << QVariant();
      QTest::newRow( "line_locate_point null" ) << "line_locate_point(NULL, geom_from_wkt('Point 5 0'))" << false << QVariant();
      QTest::newRow( "line_locate_point point" ) << "line_locate_point(geom_from_wkt('POINT(1 2)'),geom_from_wkt('Point 5 0'))" << false << QVariant();
      QTest::newRow( "line_locate_point line" ) << "line_locate_point(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),point:=geom_from_wkt('Point(5 0)'))" << false << QVariant( 5.0 );
      QTest::newRow( "line_interpolate_angle not geom" ) << "line_interpolate_angle('g', 5)" << true << QVariant();
      QTest::newRow( "line_interpolate_angle null" ) << "line_interpolate_angle(NULL, 5)" << false << QVariant();
      QTest::newRow( "line_interpolate_angle point" ) << "line_interpolate_angle(geom_from_wkt('POINT(1 2)'),5)" << false << QVariant( 0.0 );
      QTest::newRow( "line_interpolate_angle line" ) << "line_interpolate_angle(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),distance:=5)" << false << QVariant( 90.0 );
      QTest::newRow( "angle_at_vertex not geom" ) << "angle_at_vertex('g', 5)" << true << QVariant();
      QTest::newRow( "angle_at_vertex null" ) << "angle_at_vertex(NULL, 0)" << false << QVariant();
      QTest::newRow( "angle_at_vertex point" ) << "angle_at_vertex(geom_from_wkt('POINT(1 2)'),0)" << false << QVariant( 0.0 );
      QTest::newRow( "angle_at_vertex line" ) << "angle_at_vertex(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),vertex:=1)" << false << QVariant( 45.0 );
      QTest::newRow( "distance_to_vertex not geom" ) << "distance_to_vertex('g', 5)" << true << QVariant();
      QTest::newRow( "distance_to_vertex null" ) << "distance_to_vertex(NULL, 0)" << false << QVariant();
      QTest::newRow( "distance_to_vertex point" ) << "distance_to_vertex(geom_from_wkt('POINT(1 2)'),0)" << false << QVariant( 0.0 );
      QTest::newRow( "distance_to_vertex line" ) << "distance_to_vertex(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),vertex:=1)" << false << QVariant( 10.0 );
      QTest::newRow( "line_substring not geom" ) << "line_substring('g', 5, 6)" << true << QVariant();
      QTest::newRow( "line_substring null" ) << "line_substring(NULL, 5, 6)" << false << QVariant();
      QTest::newRow( "line_substring point" ) << "line_substring(geom_from_wkt('POINT(1 2)'),5,6)" << true << QVariant();
      QTest::newRow( "line_substring line" ) << "geom_to_wkt(line_substring(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),start_distance:=5,end_distance:=6))" << false << QVariant( "LineString (5 0, 6 0)" );
      QTest::newRow( "simplify not geom" ) << "simplify('g',5)" << true << QVariant();
      QTest::newRow( "simplify null" ) << "simplify(NULL,5)" << false << QVariant();
      QTest::newRow( "simplify point" ) << "geom_to_wkt(simplify(geom_from_wkt('POINT(1 2)'),10))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "simplify line" ) << "geom_to_wkt(simplify(geometry:=geom_from_wkt('LineString(0 0, 5 0, 10 0)'),tolerance:=5))" << false << QVariant( "LineString (0 0, 10 0)" );
      QTest::newRow( "simplify_vw not geom" ) << "simplify_vw('g',5)" << true << QVariant();
      QTest::newRow( "simplify_vw null" ) << "simplify_vw(NULL,5)" << false << QVariant();
      QTest::newRow( "simplify_vw point" ) << "geom_to_wkt(simplify_vw(geom_from_wkt('POINT(1 2)'),10))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "simplify_vw line" ) << "geom_to_wkt(simplify_vw(geometry:=geom_from_wkt('LineString(0 0, 5 0, 5.01 10, 5.02 0, 10 0)'),tolerance:=5))" << false << QVariant( "LineString (0 0, 10 0)" );
      QTest::newRow( "smooth not geom" ) << "smooth('g',5)" << true << QVariant();
      QTest::newRow( "smooth null" ) << "smooth(NULL,5)" << false << QVariant();
      QTest::newRow( "smooth point" ) << "geom_to_wkt(smooth(geom_from_wkt('POINT(1 2)'),10))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "smooth line" ) << "geom_to_wkt(smooth(geometry:=geom_from_wkt('LineString(0 0, 5 0, 5 5)'),iterations:=1,offset:=0.2,min_length:=-1,max_angle:=180))" << false << QVariant( "LineString (0 0, 4 0, 5 1, 5 5)" );
      QTest::newRow( "transform invalid" ) << "transform(make_point(500,500),'EPSG:4326','EPSG:28356')" << false << QVariant();
      QTest::newRow( "hausdorff line to line" ) << " hausdorff_distance( geometry1:= geom_from_wkt('LINESTRING (0 0, 2 1)'),geometry2:=geom_from_wkt('LINESTRING (0 0, 2 0)'))" << false << QVariant( 1.0 );
      QTest::newRow( "hausdorff line to line default" ) << " round(hausdorff_distance( geom_from_wkt('LINESTRING (130 0, 0 0, 0 150)'),geom_from_wkt('LINESTRING (10 10, 10 150, 130 10)')))" << false << QVariant( 14 );
      QTest::newRow( "hausdorff line to line densify" ) << " round(hausdorff_distance( geom_from_wkt('LINESTRING (130 0, 0 0, 0 150)'),geom_from_wkt('LINESTRING (10 10, 10 150, 130 10)'),0.5))" << false << QVariant( 70 );
      QTest::newRow( "hausdorff not geom 1" ) << " hausdorff_distance( 'a',geom_from_wkt('LINESTRING (0 0, 2 0)'))" << true << QVariant();
      QTest::newRow( "hausdorff not geom 2" ) << " hausdorff_distance( geom_from_wkt('LINESTRING (0 0, 2 0)'), 'b')" << true << QVariant();
      QTest::newRow( "flip_coordinates not geom" ) << "flip_coordinates('g')" << true << QVariant();
      QTest::newRow( "flip_coordinates null" ) << "flip_coordinates(NULL)" << false << QVariant();
      QTest::newRow( "flip_coordinates point" ) << "geom_to_wkt(flip_coordinates(geom_from_wkt('POINT(1 2)')))" << false << QVariant( "Point (2 1)" );

      // string functions
      QTest::newRow( "format_number" ) << "format_number(1999.567,2)" << false << QVariant( "1,999.57" );
      QTest::newRow( "format_number large" ) << "format_number(9000000.0,0)" << false << QVariant( "9,000,000" );
      QTest::newRow( "format_number many decimals" ) << "format_number(123.45600,4)" << false << QVariant( "123.4560" );
      QTest::newRow( "format_number no decimals" ) << "format_number(1999.567,0)" << false << QVariant( "2,000" );
      QTest::newRow( "lower" ) << "lower('HeLLo')" << false << QVariant( "hello" );
      QTest::newRow( "upper" ) << "upper('HeLLo')" << false << QVariant( "HELLO" );
      QTest::newRow( "length" ) << "length('HeLLo')" << false << QVariant( 5 );
      QTest::newRow( "replace" ) << "replace('HeLLo', 'LL', 'xx')" << false << QVariant( "Hexxo" );
      QTest::newRow( "replace (array replaced by array)" ) << "replace('321', array('1','2','3'), array('7','8','9'))" << false << QVariant( "987" );
      QTest::newRow( "replace (array replaced by string)" ) << "replace('12345', array('2','4'), '')" << false << QVariant( "135" );
      QTest::newRow( "replace (unbalanced array, before > after)" ) << "replace('12345', array('1','2','3'), array('6','7'))" << true << QVariant();
      QTest::newRow( "replace (unbalanced array, before < after)" ) << "replace('12345', array('1','2'), array('6','7','8'))" << true << QVariant();
      QTest::newRow( "replace (map)" ) << "replace('APP SHOULD ROCK',map('APP','QGIS','SHOULD','DOES'))" << false << QVariant( "QGIS DOES ROCK" );
      QTest::newRow( "regexp_replace" ) << "regexp_replace('HeLLo','[eL]+', '-')" << false << QVariant( "H-o" );
      QTest::newRow( "regexp_replace greedy" ) << "regexp_replace('HeLLo','(?<=H).*L', '-')" << false << QVariant( "H-o" );
      QTest::newRow( "regexp_replace non greedy" ) << "regexp_replace('HeLLo','(?<=H).*?L', '-')" << false << QVariant( "H-Lo" );
      QTest::newRow( "regexp_replace cap group" ) << "regexp_replace('HeLLo','(eL)', 'x\\\\1x')" << false << QVariant( "HxeLxLo" );
      QTest::newRow( "regexp_replace invalid" ) << "regexp_replace('HeLLo','[[[', '-')" << true << QVariant();
      QTest::newRow( "substr" ) << "substr('HeLLo', 3,2)" << false << QVariant( "LL" );
      QTest::newRow( "substr negative start" ) << "substr('HeLLo', -4)" << false << QVariant( "eLLo" );
      QTest::newRow( "substr negative length" ) << "substr('HeLLo', 1,-3)" << false << QVariant( "He" );
      QTest::newRow( "substr positive start and negative length" ) << "substr('HeLLo', 3,-1)" << false << QVariant( "LL" );
      QTest::newRow( "substr start only" ) << "substr('HeLLo', 3)" << false << QVariant( "LLo" );
      QTest::newRow( "substr null value" ) << "substr(NULL, 3,2)" << false << QVariant();
      QTest::newRow( "substr null start" ) << "substr('Hello',NULL,2)" << false << QVariant();
      QTest::newRow( "regexp_substr" ) << "regexp_substr('abc123','(\\\\d+)')" << false << QVariant( "123" );
      QTest::newRow( "regexp_substr non-greedy" ) << "regexp_substr('abc123','(\\\\d+?)')" << false << QVariant( "1" );
      QTest::newRow( "regexp_substr no hit" ) << "regexp_substr('abcdef','(\\\\d+)')" << false << QVariant( "" );
      QTest::newRow( "regexp_substr invalid" ) << "regexp_substr('abc123','([[[')" << true << QVariant();
      QTest::newRow( "regexp_substr ignored part" ) << "regexp_substr('abc123','c(.)')" << false << QVariant( "1" );
      QTest::newRow( "regexp_substr no capture group" ) << "regexp_substr('abc123','c\\\\d')" << false << QVariant( "c1" );
      QTest::newRow( "regexp_matches" ) << "array_get(regexp_matches('qgis=>rOcks;hello=>world','qgis=>(.*)[;$]'),0)" << false << QVariant( "rOcks" );
      QTest::newRow( "regexp_matches empty custom value" ) << "array_get(regexp_matches('qgis=>;hello=>world','qgis=>(.*)[;$]','empty'),0)" << false << QVariant( "empty" );
      QTest::newRow( "regexp_matches no match" ) << "regexp_matches('123','no()match')" << false << QVariant();
      QTest::newRow( "regexp_matches no capturing group" ) << "regexp_matches('some string','.*')" << false << QVariant( QVariantList() );
      QTest::newRow( "regexp_matches invalid" ) << "regexp_matches('invalid','(')" << true << QVariant();
      QTest::newRow( "strpos" ) << "strpos('Hello World','World')" << false << QVariant( 7 );
      QTest::newRow( "strpos non-regexp" ) << "strpos('Hello.World','.')" << false << QVariant( 6 );
      QTest::newRow( "strpos outside" ) << "strpos('Hello World','blah')" << false << QVariant( 0 );
      QTest::newRow( "left" ) << "left('Hello World',5)" << false << QVariant( "Hello" );
      QTest::newRow( "right" ) << "right('Hello World', 5)" << false << QVariant( "World" );
      QTest::newRow( "rpad" ) << "rpad('Hello', 10, 'x')" << false << QVariant( "Helloxxxxx" );
      QTest::newRow( "rpad truncate" ) << "rpad('Hello', 4, 'x')" << false << QVariant( "Hell" );
      QTest::newRow( "lpad" ) << "lpad('Hello', 10, 'x')" << false << QVariant( "xxxxxHello" );
      QTest::newRow( "lpad truncate" ) << "lpad('Hello', 4, 'x')" << false << QVariant( "Hell" );
      QTest::newRow( "title" ) << "title(' HeLlO   WORLD ')" << false << QVariant( " Hello   World " );
      QTest::newRow( "trim" ) << "trim('   Test String ')" << false << QVariant( "Test String" );
      QTest::newRow( "trim empty string" ) << "trim('')" << false << QVariant( "" );
      QTest::newRow( "char" ) << "char(81)" << false << QVariant( "Q" );
      QTest::newRow( "wordwrap" ) << "wordwrap('university of qgis',13)" << false << QVariant( "university of\nqgis" );
      QTest::newRow( "wordwrap with custom delimiter" ) << "wordwrap('university of qgis',13,' ')" << false << QVariant( "university of\nqgis" );
      QTest::newRow( "wordwrap with negative length" ) << "wordwrap('university of qgis',-3)" << false << QVariant( "university\nof qgis" );
      QTest::newRow( "wordwrap with negative length, custom delimiter" ) << "wordwrap('university of qgis',-3,' ')" << false << QVariant( "university\nof qgis" );
      QTest::newRow( "wordwrap on multi line" ) << "wordwrap('university of qgis\nsupports many multiline',-5,' ')" << false << QVariant( "university\nof qgis\nsupports\nmany multiline" );
      QTest::newRow( "wordwrap on zero-space width" ) << QStringLiteral( "wordwrap('test%1zero-width space',4)" ).arg( QChar( 8203 ) ) << false << QVariant( "test\nzero-width\nspace" );
      QTest::newRow( "format" ) << "format('%1 %2 %3 %1', 'One', 'Two', 'Three')" << false << QVariant( "One Two Three One" );
      QTest::newRow( "concat" ) << "concat('a', 'b', 'c', 'd')" << false << QVariant( "abcd" );
      QTest::newRow( "concat function single" ) << "concat('a')" << false << QVariant( "a" );
      QTest::newRow( "concat function with NULL" ) << "concat(NULL,'a','b')" << false << QVariant( "ab" );
      QTest::newRow( "array_to_string" ) << "array_to_string(array(1,2,3),',')" << false << QVariant( "1,2,3" );
      QTest::newRow( "array_to_string with custom empty value" ) << "array_to_string(array(1,'',3),',','*')" << false << QVariant( "1,*,3" );
      QTest::newRow( "array_to_string fail passing non-array" ) << "array_to_string('non-array',',')" << true << QVariant();
      QTest::newRow( "array_unique" ) << "array_to_string(array_distinct(array('hello','world','world','hello')))" << false << QVariant( "hello,world" );
      QTest::newRow( "array_unique fail passing non-array" ) << "array_distinct('non-array')" << true << QVariant();

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
      QTest::newRow( "nullif no substitution" ) << "nullif(3, '(none)')" << false << QVariant( 3 );
      QTest::newRow( "nullif NULL" ) << "nullif(NULL, '(none)')" << false << QVariant();
      QTest::newRow( "nullif substitute string" ) << "nullif('(none)', '(none)')" << false << QVariant();
      QTest::newRow( "nullif substitute double" ) << "nullif(3.3, 3.3)" << false << QVariant();
      QTest::newRow( "nullif substitute int" ) << "nullif(0, 0)" << false << QVariant();
      QTest::newRow( "regexp match" ) << "regexp_match('abc','.b.')" << false << QVariant( 1 );
      QTest::newRow( "regexp match invalid" ) << "regexp_match('abc DEF','[[[')" << true << QVariant();
      QTest::newRow( "regexp match escaped" ) << "regexp_match('abc DEF','\\\\s[A-Z]+')" << false << QVariant( 4 );
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
      QTest::newRow( "big age" ) << "age('2000-01-01','1000-01-01')" << false << QVariant::fromValue( QgsInterval( 31556908800LL ) );
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
      QTest::newRow( "date - date" ) << "to_date('2013-03-04') - to_date('2013-03-01')" << false << QVariant( QgsInterval( 3 * 24 * 60 * 60 ) );
      QTest::newRow( "datetime - datetime" ) << "to_datetime('2013-03-04 08:30:00') - to_datetime('2013-03-01 05:15:00')" << false << QVariant( QgsInterval( 3 * 24 * 60 * 60 + 3 * 60 * 60 + 15 * 60 ) );
      QTest::newRow( "time - time" ) << "to_time('08:30:00') - to_time('05:15:00')" << false << QVariant( QgsInterval( 3 * 60 * 60 + 15 * 60 ) );
      QTest::newRow( "epoch" ) << "epoch(to_datetime('2017-01-01T00:00:01+00:00'))" << false << QVariant( 1483228801000LL );
      QTest::newRow( "epoch invalid date" ) << "epoch('invalid')" << true << QVariant();

      // Color functions
      QTest::newRow( "ramp color" ) << "ramp_color('Spectral',0.3)" << false << QVariant( "254,190,116,255" );
      QTest::newRow( "create ramp color, wrong parameter" ) << "create_ramp(1)" << true << QVariant();
      QTest::newRow( "create ramp color, no color" ) << "create_ramp(map())" << true << QVariant();
      QTest::newRow( "create ramp color, one color" ) << "ramp_color(create_ramp(map(0,'0,0,0')),0.5)" << false << QVariant( "0,0,0,255" );
      QTest::newRow( "create ramp color, two colors" ) << "ramp_color(create_ramp(map(0,'0,0,0',1,'255,0,0')),0.33)" << false << QVariant( "84,0,0,255" );
      QTest::newRow( "create ramp color, four colors" ) << "ramp_color(create_ramp(map(0,'0,0,0',0.33,'0,255,0',0.66,'0,0,255',1,'255,0,0')),0.5)" << false << QVariant( "0,124,131,255" );
      QTest::newRow( "create ramp color, discrete" ) << "ramp_color(create_ramp(map(0,'0,0,0',0.33,'0,255,0',0.66,'0,0,255',1,'255,0,0'),true),0.6)" << false << QVariant( "0,255,0,255" );
      QTest::newRow( "color rgb" ) << "color_rgb(255,127,0)" << false << QVariant( "255,127,0" );
      QTest::newRow( "color rgba" ) << "color_rgba(255,127,0,200)" << false << QVariant( "255,127,0,200" );
      QTest::newRow( "color hsl" ) << "color_hsl(100,50,70)" << false << QVariant( "166,217,140" );
      QTest::newRow( "color hsla" ) << "color_hsla(100,50,70,200)" << false << QVariant( "166,217,140,200" );
      QTest::newRow( "color hsv" ) << "color_hsv(40,100,100)" << false << QVariant( "255,170,0" );
      QTest::newRow( "color hsva" ) << "color_hsva(40,100,100,200)" << false << QVariant( "255,170,0,200" );
      QTest::newRow( "color cmyk" ) << "color_cmyk(100,50,33,10)" << false << QVariant( "0,115,154" );
      QTest::newRow( "color cmyka" ) << "color_cmyka(50,25,90,60,200)" << false << QVariant( "51,76,10,200" );
      QTest::newRow( "color grayscale average" ) << "color_grayscale_average('255,100,50')" << false << QVariant( "135,135,135,255" );
      QTest::newRow( "color mix rgb" ) << "color_mix_rgb('0,0,0,100','255,255,255',0.5)" << false << QVariant( "127,127,127,177" );

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
      QTest::newRow( "layer_property no property" ) << QStringLiteral( "layer_property('%1','')" ).arg( mPointsLayer->name() ) << false << QVariant();
      QTest::newRow( "layer_property bad property" ) << QStringLiteral( "layer_property('%1','bad')" ).arg( mPointsLayer->name() ) << false << QVariant();
      QTest::newRow( "layer_property by id" ) << QStringLiteral( "layer_property('%1','name')" ).arg( mPointsLayer->id() ) << false << QVariant( mPointsLayer->name() );
      QTest::newRow( "layer_property name" ) << QStringLiteral( "layer_property('%1','name')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->name() );
      QTest::newRow( "layer_property id" ) << QStringLiteral( "layer_property('%1','id')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->id() );
      QTest::newRow( "layer_property title" ) << QStringLiteral( "layer_property('%1','title')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->title() );
      QTest::newRow( "layer_property abstract" ) << QStringLiteral( "layer_property('%1','abstract')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->abstract() );
      QTest::newRow( "layer_property keywords" ) << QStringLiteral( "layer_property('%1','keywords')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->keywordList() );
      QTest::newRow( "layer_property data_url" ) << QStringLiteral( "layer_property('%1','data_url')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->dataUrl() );
      QTest::newRow( "layer_property attribution" ) << QStringLiteral( "layer_property('%1','attribution')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->attribution() );
      QTest::newRow( "layer_property attribution_url" ) << QStringLiteral( "layer_property('%1','attribution_url')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->attributionUrl() );
      QTest::newRow( "layer_property source" ) << QStringLiteral( "layer_property('%1','source')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->publicSource() );
      QTest::newRow( "layer_property min_scale" ) << QStringLiteral( "layer_property('%1','min_scale')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->minimumScale() );
      QTest::newRow( "layer_property max_scale" ) << QStringLiteral( "layer_property('%1','max_scale')" ).arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->maximumScale() );
      QTest::newRow( "layer_property crs" ) << QStringLiteral( "layer_property('%1','crs')" ).arg( mPointsLayer->name() ) << false << QVariant( "EPSG:4326" );
      QTest::newRow( "layer_property crs_description" ) << QStringLiteral( "layer_property('%1','crs_description')" ).arg( mPointsLayer->name() ) << false << QVariant( "WGS 84" );
      QTest::newRow( "layer_property crs_definition" ) << QStringLiteral( "layer_property('%1','crs_definition')" ).arg( mPointsLayer->name() ) << false << QVariant( "+proj=longlat +datum=WGS84 +no_defs" );
      QTest::newRow( "layer_property extent" ) << QStringLiteral( "geom_to_wkt(layer_property('%1','extent'))" ).arg( mPointsLayer->name() ) << false << QVariant( "Polygon ((-118.88888889 22.80020704, -83.33333333 22.80020704, -83.33333333 46.87198068, -118.88888889 46.87198068, -118.88888889 22.80020704))" );
      QTest::newRow( "layer_property type" ) << QStringLiteral( "layer_property('%1','type')" ).arg( mPointsLayer->name() ) << false << QVariant( "Vector" );
      QTest::newRow( "layer_property storage_type" ) << QStringLiteral( "layer_property('%1','storage_type')" ).arg( mPointsLayer->name() ) << false << QVariant( "ESRI Shapefile" );
      QTest::newRow( "layer_property geometry_type" ) << QStringLiteral( "layer_property('%1','geometry_type')" ).arg( mPointsLayer->name() ) << false << QVariant( "Point" );

      QTest::newRow( "decode_uri shp path" ) << QStringLiteral( "array_last(string_to_array(replace(decode_uri('%1', 'path'), '\\\\', '/'), '/'))" ).arg( mPointsLayer->name() ) << false << QVariant( "points.shp" );

      // raster_statistic tests
      QTest::newRow( "raster_statistic no layer" ) << "raster_statistic('',1,'min')" << false << QVariant();
      QTest::newRow( "raster_statistic bad layer" ) << "raster_statistic('bad',1,'min')" << false << QVariant();
      QTest::newRow( "raster_statistic bad band" ) << QStringLiteral( "raster_statistic('%1',0,'min')" ).arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic bad band 2" ) << QStringLiteral( "raster_statistic('%1',100,'min')" ).arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic no property" ) << QStringLiteral( "raster_statistic('%1',1,'')" ).arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic bad property" ) << QStringLiteral( "raster_statistic('%1',1,'bad')" ).arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic min by id" ) << QStringLiteral( "raster_statistic('%1',1,'min')" ).arg( mRasterLayer->id() ) << false << QVariant( 0.0 );
      QTest::newRow( "raster_statistic min name" ) << QStringLiteral( "raster_statistic('%1',1,'min')" ).arg( mRasterLayer->name() ) << false << QVariant( 0.0 );
      QTest::newRow( "raster_statistic max" ) << QStringLiteral( "raster_statistic('%1',1,'max')" ).arg( mRasterLayer->id() ) << false << QVariant( 9.0 );
      QTest::newRow( "raster_statistic avg" ) << QStringLiteral( "round(10*raster_statistic('%1',1,'avg'))" ).arg( mRasterLayer->id() ) << false << QVariant( 45 );
      QTest::newRow( "raster_statistic stdev" ) << QStringLiteral( "round(100*raster_statistic('%1',1,'stdev'))" ).arg( mRasterLayer->id() ) << false << QVariant( 287 );
      QTest::newRow( "raster_statistic range" ) << QStringLiteral( "raster_statistic('%1',1,'range')" ).arg( mRasterLayer->id() ) << false << QVariant( 9.0 );
      QTest::newRow( "raster_statistic sum" ) << QStringLiteral( "round(raster_statistic('%1',1,'sum'))" ).arg( mRasterLayer->id() ) << false << QVariant( 450 );

      // raster_value tests
      QTest::newRow( "raster_value no layer" ) << "raster_value('',1,make_point(1,1))" << true << QVariant();
      QTest::newRow( "raster_value bad layer" ) << "raster_value('bad',1,make_point(1,1))" << true << QVariant();
      QTest::newRow( "raster_value bad band" ) << QStringLiteral( "raster_value('%1',0,make_point(1,1))" ).arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_value bad band 2" ) << QStringLiteral( "raster_value('%1',100,make_point(1,1))" ).arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_value invalid geometry" ) << QStringLiteral( "raster_value('%1',1,'invalid geom')" ).arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_value valid" ) << QStringLiteral( "raster_value('%1',1,make_point(1535390,5083270))" ).arg( mRasterLayer->name() ) << false << QVariant( 1.0 );
      QTest::newRow( "raster_value outside extent" ) << QStringLiteral( "raster_value('%1',1,make_point(1535370,5083250))" ).arg( mRasterLayer->name() ) << false << QVariant();

      //test conversions to bool
      QTest::newRow( "feature to bool false" ) << QStringLiteral( "case when get_feature('none','none',499) then true else false end" ) << false << QVariant( false );
      QTest::newRow( "feature to bool true" ) << QStringLiteral( "case when get_feature('test','col1',10) then true else false end" ) << false << QVariant( true );
      QTest::newRow( "geometry to bool false" ) << QStringLiteral( "case when geom_from_wkt('') then true else false end" ) << false << QVariant( false );
      QTest::newRow( "geometry to bool true" ) << QStringLiteral( "case when geom_from_wkt('Point(3 4)') then true else false end" ) << false << QVariant( true );

      // is not
      QTest::newRow( "1 is (not 2)" ) << QStringLiteral( "1 is (not 2)" ) << false << QVariant( 0 );
      QTest::newRow( "1 is not 2" ) << QStringLiteral( "1 is not 2" ) << false << QVariant( 1 );
      QTest::newRow( "1 is  not 2" ) << QStringLiteral( "1 is  not 2" ) << false << QVariant( 1 );

      // not like
      QTest::newRow( "'a' not like 'a%'" ) << QStringLiteral( "'a' not like 'a%'" ) << false << QVariant( 0 );
      QTest::newRow( "'a' not  like 'a%'" ) << QStringLiteral( "'a' not  like 'a%'" ) << false << QVariant( 0 );

      // with_variable
      QTest::newRow( "with_variable(name:='five', value:=5, expression:=@five * 2)" ) << QStringLiteral( "with_variable(name:='five', value:=5, expression:=@five * 2)" ) << false << QVariant( 10 );
      QTest::newRow( "with_variable('nothing', NULL, COALESCE(@nothing, 'something'))" ) << QStringLiteral( "with_variable('nothing', NULL, COALESCE(@nothing, 'something'))" ) << false << QVariant( "something" );

      // array_first, array_last
      QTest::newRow( "array_first(array('a', 'b', 'c'))" ) << QStringLiteral( "array_first(array('a', 'b', 'c'))" ) << false << QVariant( "a" );
      QTest::newRow( "array_first(array())" ) << QStringLiteral( "array_first(array())" ) << false << QVariant();
      QTest::newRow( "array_last(array('a', 'b', 'c'))" ) << QStringLiteral( "array_last(array('a', 'b', 'c'))" ) << false << QVariant( "c" );
      QTest::newRow( "array_last(array())" ) << QStringLiteral( "array_last(array())" ) << false << QVariant();

      //
    }

    void run_evaluation_test( QgsExpression &exp, bool evalError, QVariant &expected )
    {
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();
      QCOMPARE( exp.hasParserError(), false );

      QVariant result = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      if ( result.type() != expected.type() )
      {
        qDebug() << "got type " << result.typeName() << "(" << result.type() << ") instead of " << expected.typeName() << "(" << expected.type() << ")";
      }
      //qDebug() << res.type() << " " << result.type();
      //qDebug() << "type " << res.typeName();
      QCOMPARE( exp.hasEvalError(), evalError );

      QgsExpressionContext context;

      QVERIFY( exp.prepare( &context ) );

      QVariant::Type resultType = result.type();
      QVariant::Type expectedType = expected.type();

      if ( resultType == QVariant::Int )
        resultType = QVariant::LongLong;
      if ( expectedType == QVariant::Int )
        expectedType = QVariant::LongLong;

      QCOMPARE( resultType, expectedType );
      switch ( resultType )
      {
        case QVariant::Invalid:
          break; // nothing more to check
        case QVariant::LongLong:
          QCOMPARE( result.toLongLong(), expected.toLongLong() );
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
        case QVariant::List:
          QCOMPARE( result.toList(), expected.toList() );
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
          QVERIFY( false ); // should never happen
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

    void eval_columns()
    {
      QgsFields fields;
      fields.append( QgsField( QStringLiteral( "x1" ) ) );
      fields.append( QgsField( QStringLiteral( "x2" ) ) );
      fields.append( QgsField( QStringLiteral( "foo" ), QVariant::Int ) );
      fields.append( QgsField( QStringLiteral( "sin" ), QVariant::Int ) );

      QgsFeature f;
      f.initAttributes( 4 );
      f.setAttribute( 2, QVariant( 20 ) );
      f.setAttribute( 3, QVariant( 10 ) );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );

      // good exp
      QgsExpression exp( QStringLiteral( "foo + 1" ) );
      bool prepareRes = exp.prepare( &context );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp.hasEvalError(), false );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( res.type(), QVariant::LongLong );
      QCOMPARE( res.toInt(), 21 );

      // bad exp
      QgsExpression exp2( QStringLiteral( "bar + 1" ) );
      bool prepareRes2 = exp2.prepare( &context );
      QCOMPARE( prepareRes2, false );
      QCOMPARE( exp2.hasEvalError(), true );
      QVariant res2 = exp2.evaluate( &context );
      QCOMPARE( res2.type(), QVariant::Invalid );

      // Has field called sin and function
      QgsExpression exp3( QStringLiteral( "sin" ) );
      prepareRes = exp3.prepare( &context );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp3.hasEvalError(), false );
      res = exp3.evaluate( &context );
      QCOMPARE( res.type(), QVariant::Int );
      QCOMPARE( res.toInt(), 10 );

      QgsExpression exp4( QStringLiteral( "sin(3.14)" ) );
      prepareRes = exp4.prepare( &context );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp4.hasEvalError(), false );
      res = exp4.evaluate( &context );
      QCOMPARE( res.toInt(), 0 );
    }

    void eval_feature_id()
    {
      QgsFeature f( 100 );
      QgsExpression exp( QStringLiteral( "$id * 2" ) );
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v = exp.evaluate( &context );
      QCOMPARE( v.toInt(), 200 );
    }

    void eval_current_feature()
    {
      QgsFeature f( 100 );
      QgsExpression exp( QStringLiteral( "$currentfeature" ) );
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v = exp.evaluate( &context );
      QgsFeature evalFeature = v.value<QgsFeature>();
      QCOMPARE( evalFeature.id(), f.id() );
    }

    void eval_feature_attribute()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( QStringLiteral( "col1" ) ) );
      fields.append( QgsField( QStringLiteral( "second_column" ), QVariant::Int ) );
      f.setFields( fields, true );
      f.setAttribute( QStringLiteral( "col1" ), QStringLiteral( "test value" ) );
      f.setAttribute( QStringLiteral( "second_column" ), 5 );
      QgsExpression exp( QStringLiteral( "attribute($currentfeature,'col1')" ) );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v = exp.evaluate( &context );
      QCOMPARE( v.toString(), QString( "test value" ) );
      QgsExpression exp2( QStringLiteral( "attribute($currentfeature,'second'||'_column')" ) );
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
      QTest::newRow( "get_feature 3" ) << QStringLiteral( "get_feature('%1','col1',11)" ).arg( mMemoryLayer->id() ) << true << 2;
      QTest::newRow( "get_feature 4" ) << QStringLiteral( "get_feature('%1','col2','test2')" ).arg( mMemoryLayer->id() ) << true << 2;

      //no matching features
      QTest::newRow( "get_feature no match1" ) << "get_feature('test','col1',499)" << false << -1;
      QTest::newRow( "get_feature no match2" ) << "get_feature('test','col2','no match!')" << false << -1;
      //no matching layer
      QTest::newRow( "get_feature no match layer" ) << "get_feature('not a layer!','col1',10)" << false << -1;

      // get_feature_by_id
      QTest::newRow( "get_feature_by_id" ) << "get_feature_by_id('test', 1)" << true << 1;
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
        QCOMPARE( feat.id(), static_cast<QgsFeatureId>( featureId ) );
      }
    }

    void test_sqliteFetchAndIncrement()
    {
      QTemporaryDir dir;
      QString testGpkgName = QStringLiteral( "humanbeings.gpkg" );
      QFile::copy( QStringLiteral( TEST_DATA_DIR ) + '/' + testGpkgName, dir.filePath( testGpkgName ) );

      QgsExpressionContext context;
      QgsExpressionContextScope *scope = new QgsExpressionContextScope();
      scope->setVariable( QStringLiteral( "test_database" ), dir.filePath( testGpkgName ) );
      scope->setVariable( QStringLiteral( "username" ), "some_username" );
      context << scope;

      // Test database file does not exist
      QgsExpression exp1( QStringLiteral( "sqlite_fetch_and_increment('/path/does/not/exist', 'T_KEY_OBJECT', 'T_LastUniqueId', 'T_Key', 'T_Id')" ) );

      exp1.evaluate( &context );
      QCOMPARE( exp1.hasEvalError(), true );
      const QString evalErrorString1 = exp1.evalErrorString();
      QVERIFY2( evalErrorString1.contains( "/path/does/not/exist" ), QStringLiteral( "Path not found in %1" ).arg( evalErrorString1 ).toUtf8().constData() );
      QVERIFY2( evalErrorString1.contains( "Error" ), QStringLiteral( "\"Error\" not found in %1" ).arg( evalErrorString1 ).toUtf8().constData() );

      // Test default values are not properly quoted
      QgsExpression exp2( QStringLiteral( "sqlite_fetch_and_increment(@test_database, 'T_KEY_OBJECT', 'T_LastUniqueId', 'T_Key', 'T_Id', map('T_LastChange','date(''now'')','T_CreateDate','date(''now'')','T_User', @username))" ) );
      exp2.evaluate( &context );
      QCOMPARE( exp2.hasEvalError(), true );
      const QString evalErrorString2 = exp2.evalErrorString();
      QVERIFY2( evalErrorString2.contains( "some_username" ), QStringLiteral( "'some_username' not found in '%1'" ).arg( evalErrorString2 ).toUtf8().constData() );

      // Test incrementation logic
      QgsExpression exp( QStringLiteral( "sqlite_fetch_and_increment(@test_database, 'T_KEY_OBJECT', 'T_LastUniqueId', 'T_Key', 'T_Id', map('T_LastChange','date(''now'')','T_CreateDate','date(''now'')','T_User','''me'''))" ) );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( res.toInt(), 0 );

      res = exp.evaluate( &context );
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      QCOMPARE( exp.hasEvalError(), false );

      QCOMPARE( res.toInt(), 1 );
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

      QTest::newRow( "geometry collect" ) << "geom_to_wkt(aggregate('aggregate_layer','collect',$geometry))" << false << QVariant( QStringLiteral( "MultiPoint ((0 0),(1 0),(2 0),(3 0),(5 0))" ) );

      QVariantList array;
      array << "test" << QVariant( QVariant::String ) << "test333" << "test4" << QVariant( QVariant::String ) << "test4";
      QTest::newRow( "array aggregate" ) << "aggregate('aggregate_layer','array_agg',\"col2\")" << false << QVariant( array );

      QTest::newRow( "sub expression" ) << "aggregate('test','sum',\"col1\" * 2)" << false << QVariant( 65 * 2 );
      QTest::newRow( "bad sub expression" ) << "aggregate('test','sum',\"xcvxcv\" * 2)" << true << QVariant();

      QTest::newRow( "filter" ) << "aggregate('test','sum',\"col1\", \"col1\" <= 10)" << false << QVariant( 13 );
      QTest::newRow( "filter context" ) << "aggregate('test','sum',\"col1\", \"col1\" <= @test_var)" << false << QVariant( 13 );
      QTest::newRow( "filter named" ) << "aggregate(layer:='test',aggregate:='sum',expression:=\"col1\", filter:=\"col1\" <= 10)" << false << QVariant( 13 );
      QTest::newRow( "filter no matching" ) << "aggregate('test','sum',\"col1\", \"col1\" <= -10)" << false << QVariant( 0 );
      QTest::newRow( "filter no matching max" ) << "aggregate('test','max',\"col1\", \"col1\" > 1000000 )" << false << QVariant();

      QTest::newRow( "filter by @parent attribute" ) << "aggregate(layer:='child_layer', aggregate:='min', expression:=\"col3\", filter:=\"parent\"=attribute(@parent,'col1'))" << false << QVariant( 1 );
    }

    void aggregate()
    {
      QgsExpressionContext context;
      QgsExpressionContextScope *scope = new QgsExpressionContextScope();
      scope->setVariable( QStringLiteral( "test_var" ), 10 );
      context << scope;
      QgsFeature f;
      mAggregatesLayer->getFeatures( QStringLiteral( "col1 = 4 " ) ).nextFeature( f );
      context.setFeature( f );

      QFETCH( QString, string );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res;

      //try evaluating once without context (only if variables aren't required)
      if ( !string.contains( QLatin1String( "@" ) ) )
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

      QTest::newRow( "geometry collect" ) << "geom_to_wkt(collect($geometry))" << false << QVariant( QStringLiteral( "MultiPoint ((0 0),(1 0),(2 0),(3 0),(5 0))" ) );
      QTest::newRow( "geometry collect with null geometry first" ) << "geom_to_wkt(collect($geometry, filter:=\"col3\"=3))" << false << QVariant( QStringLiteral( "MultiPoint ((5 0))" ) );

      QTest::newRow( "bad expression" ) << "sum(\"xcvxcvcol1\")" << true << QVariant();
      QTest::newRow( "aggregate named" ) << "sum(expression:=\"col1\")" << false << QVariant( 24.0 );
      QTest::newRow( "string aggregate on int" ) << "max_length(\"col1\")" << true << QVariant();

      QTest::newRow( "sub expression" ) << "sum(\"col1\" * 2)" << false << QVariant( 48 );
      QTest::newRow( "bad sub expression" ) << "sum(\"xcvxcv\" * 2)" << true << QVariant();

      QTest::newRow( "filter" ) << "sum(\"col1\", NULL, \"col1\" >= 5)" << false << QVariant( 13 );
      QTest::newRow( "filter named" ) << "sum(expression:=\"col1\", filter:=\"col1\" >= 5)" << false << QVariant( 13 );
      QTest::newRow( "filter no matching" ) << "sum(expression:=\"col1\", filter:=\"col1\" <= -5)" << false << QVariant( 0 );
      QTest::newRow( "filter no matching max" ) << "maximum(\"col1\", filter:=\"col1\" <= -5)" << false << QVariant();

      QTest::newRow( "group by" ) << "sum(\"col1\", \"col3\")" << false << QVariant( 9 );
      QTest::newRow( "group by and filter" ) << "sum(\"col1\", \"col3\", \"col1\">=3)" << false << QVariant( 7 );
      QTest::newRow( "group by and filter named" ) << "sum(expression:=\"col1\", group_by:=\"col3\", filter:=\"col1\">=3)" << false << QVariant( 7 );
      QTest::newRow( "group by expression" ) << "sum(\"col1\", \"col1\" % 2)" << false << QVariant( 16 );
      QTest::newRow( "group by with null value" ) << "sum(\"col1\", \"col4\")" << false << QVariant( 9 );
    }

    void selection()
    {
      QFETCH( QgsFeatureIds, selectedFeatures );
      QFETCH( QString, expression );
      QFETCH( QVariant, result );
      QFETCH( QgsFeature, feature );
      QFETCH( QgsVectorLayer *, layer );

      QgsExpressionContext context;
      if ( layer )
        context.appendScope( QgsExpressionContextUtils::layerScope( layer ) );

      QgsFeatureIds backupSelection = mMemoryLayer->selectedFeatureIds();
      context.setFeature( feature );

      mMemoryLayer->selectByIds( selectedFeatures );

      QgsExpression exp( expression );
      QCOMPARE( exp.parserErrorString(), QString() );
      exp.prepare( &context );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( res, result );

      mMemoryLayer->selectByIds( backupSelection );
    }

    void selection_data()
    {
      QTest::addColumn<QString>( "expression" );
      QTest::addColumn<QgsFeatureIds>( "selectedFeatures" );
      QTest::addColumn<QgsFeature>( "feature" );
      QTest::addColumn<QgsVectorLayer *>( "layer" );
      QTest::addColumn<QVariant>( "result" );

      QgsFeature firstFeature = mMemoryLayer->getFeature( 1 );
      QgsVectorLayer *noLayer = nullptr;

      QTest::newRow( "empty selection num_selected" ) << "num_selected()" << QgsFeatureIds() << firstFeature << mMemoryLayer << QVariant( 0 );
      QTest::newRow( "empty selection is_selected" ) << "is_selected()" << QgsFeatureIds() << firstFeature << mMemoryLayer << QVariant( false );
      QTest::newRow( "two_selected" ) << "num_selected()" << ( QgsFeatureIds() << 1 << 2 ) << firstFeature << mMemoryLayer << QVariant( 2 );
      QTest::newRow( "is_selected" ) << "is_selected()" << ( QgsFeatureIds() << 1 << 2 ) << firstFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "not_selected" ) << "is_selected()" << ( QgsFeatureIds() << 4 << 2 ) << firstFeature << mMemoryLayer << QVariant( false );
      QTest::newRow( "no layer num_selected" ) << "num_selected()" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QVariant( QVariant::LongLong );
      QTest::newRow( "no layer is_selected" ) << "is_selected()" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QVariant( QVariant::Bool );
      QTest::newRow( "no layer num_selected" ) << "num_selected()" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QVariant( QVariant::LongLong );
      QTest::newRow( "is_selected with params" ) << "is_selected('test', get_feature('test', 'col1', 10))" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QVariant( QVariant::Bool );
      QTest::newRow( "num_selected with params" ) << "num_selected('test')" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QVariant( 2 );
    }

    void layerAggregates()
    {
      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::layerScope( mAggregatesLayer ) );

      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      af1.setAttribute( QStringLiteral( "col1" ), 4 );
      af1.setAttribute( QStringLiteral( "col2" ), "test" );
      af1.setAttribute( QStringLiteral( "col3" ), 2 );
      af1.setAttribute( QStringLiteral( "col4" ), QVariant() );
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
      af1.setAttribute( QStringLiteral( "col1" ), parentKey );
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
      QgsExpression exp( QStringLiteral( "x(geometry(get_feature('%1','heading',340)))" ).arg( mPointsLayer->id() ) );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();

      QCOMPARE( exp.hasEvalError(), false );
      QGSCOMPARENEAR( res.toDouble(), -85.65217, 0.00001 );
    }

    void eval_rand()
    {
      QgsExpression exp1( QStringLiteral( "rand(1,10)" ) );
      QVariant v1 = exp1.evaluate();
      QCOMPARE( v1.toInt() <= 10, true );
      QCOMPARE( v1.toInt() >= 1, true );

      QgsExpression exp2( QStringLiteral( "rand(min:=-5,max:=-5)" ) );
      QVariant v2 = exp2.evaluate();
      QCOMPARE( v2.toInt(), -5 );

      // Invalid expression since max<min
      QgsExpression exp3( QStringLiteral( "rand(10,1)" ) );
      QVariant v3 = exp3.evaluate();
      QCOMPARE( v3.type(),  QVariant::Invalid );
    }

    void eval_randf()
    {
      QgsExpression exp1( QStringLiteral( "randf(1.5,9.5)" ) );
      QVariant v1 = exp1.evaluate();
      QCOMPARE( v1.toDouble() <= 9.5, true );
      QCOMPARE( v1.toDouble() >= 1.5, true );

      QgsExpression exp2( QStringLiteral( "randf(min:=-0.0005,max:=-0.0005)" ) );
      QVariant v2 = exp2.evaluate();
      QCOMPARE( v2.toDouble(), -0.0005 );

      // Invalid expression since max<min
      QgsExpression exp3( QStringLiteral( "randf(9.3333,1.784)" ) );
      QVariant v3 = exp3.evaluate();
      QCOMPARE( v3.type(),  QVariant::Invalid );
    }

    void referenced_columns()
    {
      QSet<QString> expectedCols;
      expectedCols << QStringLiteral( "foo" ) << QStringLiteral( "bar" ) << QStringLiteral( "ppp" ) << QStringLiteral( "qqq" ) << QStringLiteral( "rrr" );
      QgsExpression exp( QStringLiteral( "length(Bar || FOO) = 4 or foo + sqrt(bar) > 0 or case when ppp then qqq else rrr end" ) );
      QCOMPARE( exp.hasParserError(), false );
      QSet<QString> refCols = exp.referencedColumns();
      // make sure we have lower case
      QSet<QString> refColsSet;
      Q_FOREACH ( const QString &col, refCols )
        refColsSet.insert( col.toLower() );

      QCOMPARE( refColsSet, expectedCols );

      expectedCols.clear();
      expectedCols << QgsFeatureRequest::ALL_ATTRIBUTES
                   << QStringLiteral( "parent_col1" )
                   << QStringLiteral( "parent_col2" );
      // sub expression fields, "child_field", "child_field2" should not be included in referenced columns
      exp = QgsExpression( QStringLiteral( "relation_aggregate(relation:=\"parent_col1\" || 'my_rel',aggregate:='sum' || \"parent_col2\", expression:=\"child_field\" * \"child_field2\")" ) );
      QCOMPARE( exp.hasParserError(), false );
      refCols = exp.referencedColumns();
      QCOMPARE( refCols, expectedCols );
    }

    void referenced_variables()
    {
      QSet<QString> expectedVars;
      expectedVars << QStringLiteral( "foo" )
                   << QStringLiteral( "bar" )
                   << QStringLiteral( "ppp" )
                   << QStringLiteral( "qqq" )
                   << QStringLiteral( "rrr" )
                   << QStringLiteral( "sss" )
                   << QStringLiteral( "ttt" );
      QgsExpression exp( QStringLiteral( "CASE WHEN intersects(@bar, $geometry) THEN @ppp ELSE @qqq * @rrr END + @foo IN (1, 2, @sss) OR @ttt" ) );
      QCOMPARE( exp.hasParserError(), false );
      QSet<QString> refVar = exp.referencedVariables();

      QCOMPARE( refVar, expectedVars );
    }


    void referenced_functions()
    {
      QSet<QString> expectedFunctions;
      expectedFunctions << QStringLiteral( "current_value" )
                        << QStringLiteral( "var" )
                        << QStringLiteral( "intersects" )
                        << QStringLiteral( "$geometry" )
                        << QStringLiteral( "buffer" );

      QgsExpression exp( QStringLiteral( "current_value( 'FIELD_NAME' ) = 'A_VALUE' AND intersects(buffer($geometry, 10), @current_geometry)" ) );
      QCOMPARE( exp.hasParserError(), false );
      QSet<QString> refVar = exp.referencedFunctions();

      QCOMPARE( refVar, expectedFunctions );
    }

    void findNodes()
    {
      QSet<QString> expectedFunctions;
      expectedFunctions << QStringLiteral( "current_value" )
                        << QStringLiteral( "intersects" )
                        << QStringLiteral( "var" )
                        << QStringLiteral( "$geometry" )
                        << QStringLiteral( "buffer" );
      QgsExpression exp( QStringLiteral( "current_value( 'FIELD_NAME' ) = 'A_VALUE' AND intersects(buffer($geometry, 10), @current_geometry)" ) );
      QList<const QgsExpressionNodeFunction *> functionNodes( exp.findNodes<QgsExpressionNodeFunction>() );
      QCOMPARE( functionNodes.size(), 5 );
      QgsExpressionFunction *fd;
      QSet<QString> actualFunctions;
      for ( const auto &f : functionNodes )
      {
        QCOMPARE( f->nodeType(), QgsExpressionNode::NodeType::ntFunction );
        fd = QgsExpression::QgsExpression::Functions()[f->fnIndex()];
        actualFunctions << fd->name();
      }
      QCOMPARE( actualFunctions, expectedFunctions );

      QSet<QgsExpressionNodeBinaryOperator::BinaryOperator> expectedBinaryOps;
      expectedBinaryOps << QgsExpressionNodeBinaryOperator::BinaryOperator::boAnd;
      expectedBinaryOps << QgsExpressionNodeBinaryOperator::BinaryOperator::boEQ;
      QList<const QgsExpressionNodeBinaryOperator *> binaryOpsNodes( exp.findNodes<QgsExpressionNodeBinaryOperator>() );
      QCOMPARE( binaryOpsNodes.size(), 2 );
      QSet<QgsExpressionNodeBinaryOperator::BinaryOperator> actualBinaryOps;
      for ( const auto &f : binaryOpsNodes )
      {
        QCOMPARE( f->nodeType(), QgsExpressionNode::NodeType::ntBinaryOperator );
        actualBinaryOps << f->op();
      }
      QCOMPARE( actualBinaryOps, expectedBinaryOps );

    }

    void referenced_columns_all_attributes()
    {
      QgsExpression exp( QStringLiteral( "attribute($currentfeature,'test')" ) );
      QCOMPARE( exp.hasParserError(), false );
      QSet<QString> refCols = exp.referencedColumns();
      // make sure we get the all attributes flag
      bool allAttributesFlag = refCols.contains( QgsFeatureRequest::ALL_ATTRIBUTES );
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
      QTest::newRow( "aggregate with parent" ) << "aggregate(layer:='test',aggregate:='sum',expression:=\"col1\", filter:=intersects(geometry(@parent), make_point(1, 1)))" << true;
      QTest::newRow( "aggregate without parent" ) << "aggregate(layer:='test',aggregate:='sum',expression:=\"col1\", filter:=\"c\" = 2)" << false;
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
      QTest::addColumn<QgsGeometry>( "geom" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QgsPointXY point( 123, 456 );
      QgsPolylineXY line;
      line << QgsPointXY( 1, 1 ) << QgsPointXY( 4, 2 ) << QgsPointXY( 3, 1 );

      QTest::newRow( "geom x" ) << "$x" << QgsGeometry::fromPointXY( point ) << false << QVariant( 123. );
      QTest::newRow( "geom y" ) << "$y" << QgsGeometry::fromPointXY( point ) << false << QVariant( 456. );
      QTest::newRow( "geom xat" ) << "xat(-1)" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 3. );
      QTest::newRow( "geom yat" ) << "yat(1)" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 2. );
      QTest::newRow( "null geometry" ) << "$geometry" << QgsGeometry() << false << QVariant( QVariant::UserType );
    }

    void eval_geometry()
    {
      QFETCH( QString, string );
      QFETCH( QgsGeometry, geom );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), true );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out, result );
    }

    void eval_geometry_calc()
    {
      QgsPolylineXY polyline, polygon_ring;
      polyline << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 );
      polygon_ring << QgsPointXY( 2, 1 ) << QgsPointXY( 10, 1 ) << QgsPointXY( 10, 6 ) << QgsPointXY( 2, 6 ) << QgsPointXY( 2, 1 );
      QgsPolygonXY polygon;
      polygon << polygon_ring;
      QgsFeature fPolygon, fPolyline;
      QgsGeometry polylineGeom = QgsGeometry::fromPolylineXY( polyline );
      fPolyline.setGeometry( polylineGeom );
      QgsGeometry polygonGeom = QgsGeometry::fromPolygonXY( polygon );
      fPolygon.setGeometry( polygonGeom );

      QgsExpressionContext context;

      QgsExpression exp1( QStringLiteral( "$area" ) );
      context.setFeature( fPolygon );
      QVariant vArea = exp1.evaluate( &context );
      QCOMPARE( vArea.toDouble(), 40. );

      QgsExpression exp2( QStringLiteral( "$length" ) );
      context.setFeature( fPolyline );
      QVariant vLength = exp2.evaluate( &context );
      QCOMPARE( vLength.toDouble(), 10. );

      QgsExpression exp3( QStringLiteral( "$perimeter" ) );
      context.setFeature( fPolygon );
      QVariant vPerimeter = exp3.evaluate( &context );
      QCOMPARE( vPerimeter.toDouble(), 26. );

      QgsExpression deprecatedExpXAt( QStringLiteral( "$x_at(1)" ) );
      context.setFeature( fPolygon );
      QVariant xAt = deprecatedExpXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      context.setFeature( fPolyline );
      xAt = deprecatedExpXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );

      QgsExpression deprecatedExpXAtNeg( QStringLiteral( "$x_at(-2)" ) );
      context.setFeature( fPolygon );
      xAt = deprecatedExpXAtNeg.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 2.0 );

      QgsExpression deprecatedExpYAt( QStringLiteral( "$y_at(2)" ) );
      context.setFeature( fPolygon );
      QVariant yAt = deprecatedExpYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      QgsExpression deprecatedExpYAt2( QStringLiteral( "$y_at(1)" ) );
      context.setFeature( fPolyline );
      yAt = deprecatedExpYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 0.0 );

      QgsExpression deprecatedExpYAtNeg( QStringLiteral( "$y_at(-2)" ) );
      context.setFeature( fPolygon );
      yAt = deprecatedExpYAtNeg.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );

      QgsExpression expXAt( QStringLiteral( "x_at(1)" ) );
      context.setFeature( fPolygon );
      xAt = expXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      context.setFeature( fPolyline );
      xAt = expXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );

      QgsExpression expXAtNeg( QStringLiteral( "x_at(-2)" ) );
      context.setFeature( fPolygon );
      xAt = expXAtNeg.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 2.0 );

      QgsExpression expYAt( QStringLiteral( "y_at(2)" ) );
      context.setFeature( fPolygon );
      yAt = expYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      QgsExpression expYAt2( QStringLiteral( "$y_at(1)" ) );
      context.setFeature( fPolyline );
      yAt = expYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 0.0 );

      QgsExpression expYAtNeg( QStringLiteral( "y_at(-2)" ) );
      context.setFeature( fPolygon );
      yAt = expYAtNeg.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );

      QgsExpression exp4( QStringLiteral( "bounds_width($geometry)" ) );
      context.setFeature( fPolygon );
      QVariant vBoundsWidth = exp4.evaluate( &context );
      QCOMPARE( vBoundsWidth.toDouble(), 8.0 );

      QgsExpression exp5( QStringLiteral( "bounds_height($geometry)" ) );
      QVariant vBoundsHeight = exp5.evaluate( &context );
      QCOMPARE( vBoundsHeight.toDouble(), 5.0 );

      QgsExpression exp6( QStringLiteral( "xmin($geometry)" ) );
      QVariant vXMin = exp6.evaluate( &context );
      QCOMPARE( vXMin.toDouble(), 2.0 );

      QgsExpression exp7( QStringLiteral( "xmax($geometry)" ) );
      QVariant vXMax = exp7.evaluate( &context );
      QCOMPARE( vXMax.toDouble(), 10.0 );

      QgsExpression exp8( QStringLiteral( "ymin($geometry)" ) );
      QVariant vYMin = exp8.evaluate( &context );
      QCOMPARE( vYMin.toDouble(), 1.0 );

      QgsExpression exp9( QStringLiteral( "ymax($geometry)" ) );
      QVariant vYMax = exp9.evaluate( &context );
      QCOMPARE( vYMax.toDouble(), 6.0 );

      QgsExpression exp10( QStringLiteral( "num_points($geometry)" ) );
      QVariant vVertices = exp10.evaluate( &context );
      QCOMPARE( vVertices.toInt(), 5 );

      context.setFeature( fPolyline );
      QgsExpression exp11( QStringLiteral( "length($geometry)" ) );
      QVariant vLengthLine = exp11.evaluate( &context );
      QCOMPARE( vLengthLine.toDouble(), 10.0 );

      context.setFeature( fPolygon );
      QgsExpression exp12( QStringLiteral( "area($geometry)" ) );
      QVariant vAreaPoly = exp12.evaluate( &context );
      QCOMPARE( vAreaPoly.toDouble(), 40.0 );

      QgsExpression exp13( QStringLiteral( "perimeter($geometry)" ) );
      QVariant vPerimeterPoly = exp13.evaluate( &context );
      QCOMPARE( vPerimeterPoly.toDouble(), 26.0 );
    }

    void geom_calculator()
    {
      //test calculations with and without geometry calculator set
      QgsDistanceArea da;
      da.setSourceCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:3111" ) ), QgsProject::instance()->transformContext() );
      da.setEllipsoid( QStringLiteral( "WGS84" ) );

      QgsFeature feat;
      QgsPolylineXY polygonRing3111;
      polygonRing3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 ) << QgsPointXY( 2520109, 2397715 ) << QgsPointXY( 2520792, 2425494 ) << QgsPointXY( 2484588, 2425722 );
      QgsPolygonXY polygon3111;
      polygon3111 << polygonRing3111;
      QgsGeometry polygon3111G = QgsGeometry::fromPolygonXY( polygon3111 );
      feat.setGeometry( polygon3111G );
      QgsExpressionContext context;
      context.setFeature( feat );

      // test area without geomCalculator
      QgsExpression expArea( QStringLiteral( "$area" ) );
      QVariant vArea = expArea.evaluate( &context );
      double expected = 1005640568.0;
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      // units should not be converted if no geometry calculator set
      expArea.setAreaUnits( QgsUnitTypes::AreaSquareFeet );
      vArea = expArea.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      expArea.setAreaUnits( QgsUnitTypes::AreaSquareNauticalMiles );
      vArea = expArea.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );

      // test area with geomCalculator
      QgsExpression expArea2( QStringLiteral( "$area" ) );
      expArea2.setGeomCalculator( &da );
      vArea = expArea2.evaluate( &context );
      expected = 1005721496.780085;
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      // test unit conversion
      expArea2.setAreaUnits( QgsUnitTypes::AreaSquareMeters ); //default units should be square meters
      vArea = expArea2.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      expArea2.setAreaUnits( QgsUnitTypes::AreaUnknownUnit ); //unknown units should not be converted
      vArea = expArea2.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      expArea2.setAreaUnits( QgsUnitTypes::AreaSquareMiles );
      expected = 388.311241;
      vArea = expArea2.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 0.001 );

      // test perimeter without geomCalculator
      QgsExpression expPerimeter( QStringLiteral( "$perimeter" ) );
      QVariant vPerimeter = expPerimeter.evaluate( &context );
      expected = 128282.086;
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      // units should not be converted if no geometry calculator set
      expPerimeter.setDistanceUnits( QgsUnitTypes::DistanceFeet );
      vPerimeter = expPerimeter.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      expPerimeter.setDistanceUnits( QgsUnitTypes::DistanceNauticalMiles );
      vPerimeter = expPerimeter.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );

      // test perimeter with geomCalculator
      QgsExpression expPerimeter2( QStringLiteral( "$perimeter" ) );
      expPerimeter2.setGeomCalculator( &da );
      vPerimeter = expPerimeter2.evaluate( &context );
      expected = 128289.074;
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      // test unit conversion
      expPerimeter2.setDistanceUnits( QgsUnitTypes::DistanceMeters ); //default units should be meters
      vPerimeter = expPerimeter2.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      expPerimeter2.setDistanceUnits( QgsUnitTypes::DistanceUnknownUnit ); //unknown units should not be converted
      vPerimeter = expPerimeter2.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      expPerimeter2.setDistanceUnits( QgsUnitTypes::DistanceFeet );
      expected = 420895.9120735;
      vPerimeter = expPerimeter2.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );

      // test length without geomCalculator
      QgsPolylineXY line3111;
      line3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 );
      QgsGeometry line3111G =  QgsGeometry::fromPolylineXY( line3111 ) ;
      feat.setGeometry( line3111G );
      context.setFeature( feat );

      QgsExpression expLength( QStringLiteral( "$length" ) );
      QVariant vLength = expLength.evaluate( &context );
      expected = 26930.637;
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      // units should not be converted if no geometry calculator set
      expLength.setDistanceUnits( QgsUnitTypes::DistanceFeet );
      vLength = expLength.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      expLength.setDistanceUnits( QgsUnitTypes::DistanceNauticalMiles );
      vLength = expLength.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );

      // test length with geomCalculator
      QgsExpression expLength2( QStringLiteral( "$length" ) );
      expLength2.setGeomCalculator( &da );
      vLength = expLength2.evaluate( &context );
      expected = 26932.156;
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      // test unit conversion
      expLength2.setDistanceUnits( QgsUnitTypes::DistanceMeters ); //default units should be meters
      vLength = expLength2.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      expLength2.setDistanceUnits( QgsUnitTypes::DistanceUnknownUnit ); //unknown units should not be converted
      vLength = expLength2.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      expLength2.setDistanceUnits( QgsUnitTypes::DistanceFeet );
      expected = 88360.0918635;
      vLength = expLength2.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
    }

    void eval_geometry_wkt()
    {
      QgsPolylineXY polyline, polygon_ring;
      polyline << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 );
      polygon_ring << QgsPointXY( 2, 1 ) << QgsPointXY( 10, 1 ) << QgsPointXY( 10, 6 ) << QgsPointXY( 2, 6 ) << QgsPointXY( 2, 1 );

      QgsPolygonXY polygon;
      polygon << polygon_ring;

      QgsFeature fPoint, fPolygon, fPolyline;
      QgsGeometry fPointG = QgsGeometry::fromPointXY( QgsPointXY( -1.23456789, 9.87654321 ) );
      fPoint.setGeometry( fPointG );
      QgsGeometry fPolylineG = QgsGeometry::fromPolylineXY( polyline );
      fPolyline.setGeometry( fPolylineG );
      QgsGeometry fPolygonG = QgsGeometry::fromPolygonXY( polygon );
      fPolygon.setGeometry( fPolygonG );

      QgsExpressionContext context;

      QgsExpression exp1( QStringLiteral( "geomToWKT($geometry)" ) );
      context.setFeature( fPolyline );
      QVariant vWktLine = exp1.evaluate( &context );
      QCOMPARE( vWktLine.toString(), QString( "LineString (0 0, 10 0)" ) );

      QgsExpression exp2( QStringLiteral( "geomToWKT($geometry)" ) );
      context.setFeature( fPolygon );
      QVariant vWktPolygon = exp2.evaluate( &context );
      QCOMPARE( vWktPolygon.toString(), QString( "Polygon ((2 1, 10 1, 10 6, 2 6, 2 1))" ) );

      QgsExpression exp3( QStringLiteral( "geomToWKT($geometry)" ) );
      context.setFeature( fPoint );
      QVariant vWktPoint = exp3.evaluate( &context );
      QCOMPARE( vWktPoint.toString(), QString( "Point (-1.23456789 9.87654321)" ) );

      QgsExpression exp4( QStringLiteral( "geomToWKT($geometry, 3)" ) );
      QVariant vWktPointSimplify = exp4.evaluate( &context );
      QCOMPARE( vWktPointSimplify.toString(), QString( "Point (-1.235 9.877)" ) );
    }

    void eval_geometry_constructor_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<QgsGeometry>( "geom" );
      QTest::addColumn<bool>( "evalError" );

      QgsPointXY point( 123, 456 );
      QgsPolylineXY line;
      line << QgsPointXY( 1, 1 ) << QgsPointXY( 4, 2 ) << QgsPointXY( 3, 1 );

      QgsPolylineXY polyline, polygon_ring;
      polyline << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 );
      polygon_ring << QgsPointXY( 1, 1 ) << QgsPointXY( 6, 1 ) << QgsPointXY( 6, 6 ) << QgsPointXY( 1, 6 ) << QgsPointXY( 1, 1 );
      QgsPolygonXY polygon;
      polygon << polygon_ring;

      QgsGeometry sourcePoint( QgsGeometry::fromPointXY( point ) );
      QTest::newRow( "geomFromWKT Point" ) << "geom_from_wkt('" + sourcePoint.asWkt() + "')" << QgsGeometry::fromPointXY( point ) << false;
      QgsGeometry sourceLine( QgsGeometry::fromPolylineXY( line ) );
      QTest::newRow( "geomFromWKT Line" ) << "geomFromWKT('" + sourceLine.asWkt() + "')" << QgsGeometry::fromPolylineXY( line ) << false;
      QgsGeometry sourcePolyline( QgsGeometry::fromPolylineXY( polyline ) );
      QTest::newRow( "geomFromWKT Polyline" ) << "geomFromWKT('" + sourcePolyline.asWkt() + "')" << QgsGeometry::fromPolylineXY( polyline ) << false;
      QgsGeometry sourcePolygon( QgsGeometry::fromPolygonXY( polygon ) );
      QTest::newRow( "geomFromWKT Polygon" ) << "geomFromWKT('" + sourcePolygon.asWkt() + "')" << QgsGeometry::fromPolygonXY( polygon ) << false;

      // GML Point
      QTest::newRow( "GML Point (coordinates)" ) << "geomFromGML('<gml:Point><gml:coordinates>123,456</gml:coordinates></gml:Point>')" << QgsGeometry::fromPointXY( point ) << false;
      // gml:pos if from GML3
      QTest::newRow( "GML Point (pos)" ) << "geomFromGML('<gml:Point srsName=\"foo\"><gml:pos srsDimension=\"2\">123 456</gml:pos></gml:Point>')" << QgsGeometry::fromPointXY( point ) << false;

      // GML Box
      QgsRectangle rect( 135.2239, 34.4879, 135.8578, 34.8471 );
      QTest::newRow( "GML Box" ) << "geomFromGML('<gml:Box srsName=\"foo\"><gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>')" << QgsGeometry::fromRect( rect ) << false;
      // Envelope is from GML3 ?
      QTest::newRow( "GML Envelope" ) << "geomFromGML('<gml:Envelope>"
                                      "<gml:lowerCorner>135.2239 34.4879</gml:lowerCorner>"
                                      "<gml:upperCorner>135.8578 34.8471</gml:upperCorner>"
                                      "</gml:Envelope>')" << QgsGeometry::fromRect( rect ) << false;
    }

    void eval_geometry_constructor()
    {
      QFETCH( QString, string );
      QFETCH( QgsGeometry, geom );
      QFETCH( bool, evalError );

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), false );

      //replacement method
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QCOMPARE( geom.equals( outGeom ), true );
    }

    void eval_geometry_access_transform_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<QgsGeometry>( "geom" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<bool>( "needsGeom" );

      QgsPointXY point( 123, 456 );
      QgsPolylineXY line;
      line << QgsPointXY( 1, 1 ) << QgsPointXY( 4, 2 ) << QgsPointXY( 3, 1 );

      QgsPolylineXY polyline, polygon_ring;
      polyline << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 );
      polygon_ring << QgsPointXY( 1, 1 ) << QgsPointXY( 6, 1 ) << QgsPointXY( 6, 6 ) << QgsPointXY( 1, 6 ) << QgsPointXY( 1, 1 );
      QgsPolygonXY polygon;
      polygon << polygon_ring;

      QTest::newRow( "geometry Point" ) << "geometry( $currentfeature )" << QgsGeometry::fromPointXY( point ) << false << true;
      QTest::newRow( "geometry Line" ) << "geometry( $currentfeature )" << QgsGeometry::fromPolylineXY( line ) << false << true;
      QTest::newRow( "geometry Polyline" ) << "geometry( $currentfeature )" << QgsGeometry::fromPolylineXY( polyline ) << false << true;
      QTest::newRow( "geometry Polygon" ) << "geometry( $currentfeature )" << QgsGeometry::fromPolygonXY( polygon ) << false << true;

      QgsCoordinateReferenceSystem s;
      s.createFromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
      QgsCoordinateReferenceSystem d;
      d.createFromOgcWmsCrs( QStringLiteral( "EPSG:3857" ) );
      QgsCoordinateTransform t( s, d, QgsProject::instance() );

      QgsGeometry tLine = QgsGeometry::fromPolylineXY( line );
      tLine.transform( t );
      QgsGeometry tPolygon = QgsGeometry::fromPolygonXY( polygon );
      tPolygon.transform( t );

      QgsGeometry oLine = QgsGeometry::fromPolylineXY( line );
      QgsGeometry oPolygon = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "transform Line" ) << "transform( geomFromWKT('" + oLine.asWkt() + "'), 'EPSG:4326', 'EPSG:3857' )" << tLine << false << false;
      QTest::newRow( "transform Polygon" ) << "transform( geomFromWKT('" + oPolygon.asWkt() + "'), 'EPSG:4326', 'EPSG:3857' )" << tPolygon << false << false;
    }

    void eval_geometry_access_transform()
    {
      QFETCH( QString, string );
      QFETCH( QgsGeometry, geom );
      QFETCH( bool, evalError );
      QFETCH( bool, needsGeom );

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), needsGeom );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QCOMPARE( geom.equals( outGeom ), true );
    }

    void eval_spatial_operator_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<QgsGeometry>( "geom" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QgsPointXY point( 0, 0 );
      QgsPolylineXY line, polygon_ring;
      line << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 10 );
      polygon_ring << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 10 ) << QgsPointXY( 10, 0 ) << QgsPointXY( 0, 0 );
      QgsPolygonXY polygon;
      polygon << polygon_ring;

      QTest::newRow( "No Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 2 0, 0 2 )') )" << QgsGeometry::fromPointXY( point ) << false << QVariant( 0 );
      QTest::newRow( "Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 2 )') )" << QgsGeometry::fromPointXY( point ) << false << QVariant( 1 );
      QTest::newRow( "No Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 2 )') )" << QgsGeometry::fromPointXY( point ) << false << QVariant( 0 );
      QTest::newRow( "Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 2 0, 0 2 )') )" << QgsGeometry::fromPointXY( point ) << false << QVariant( 1 );

      // OGR test
      QTest::newRow( "OGR Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Intersects" ) << "intersects( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR no Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Disjoint" ) << "disjoint( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR Touches" ) << "touches( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 10 )') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Touches" ) << "touches( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Crosses" ) << "crosses( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Crosses" ) << "crosses( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 10 )') )" << QgsGeometry::fromPolylineXY( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Within" ) << "within( $geometry, geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))') )" << QgsGeometry::fromPolygonXY( polygon ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Within" ) << "within( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << QgsGeometry::fromPolygonXY( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR Contains" ) << "contains( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << QgsGeometry::fromPolygonXY( polygon ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Contains" ) << "contains( $geometry, geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))') )" << QgsGeometry::fromPolygonXY( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR no Overlaps" ) << "overlaps( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << QgsGeometry::fromPolygonXY( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR overlaps" ) << "overlaps( geomFromWKT('POLYGON((0 -5,10 5,10 -5,0 -5))'), $geometry )" << QgsGeometry::fromPolygonXY( polygon ) << false << QVariant( 1 );
    }

    void eval_spatial_operator()
    {
      QFETCH( QString, string );
      QFETCH( QgsGeometry, geom );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), true );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.toInt(), result.toInt() );
    }

    void eval_geometry_method_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<QgsGeometry>( "geom" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<bool>( "needGeom" );
      QTest::addColumn<QgsGeometry>( "result" );

      QgsPointXY point( 0, 0 );
      QgsPolylineXY line, polygon_ring;
      line << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 10 );
      polygon_ring << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 10 ) << QgsPointXY( 10, 0 ) << QgsPointXY( 0, 0 );
      QgsPolygonXY polygon;
      polygon << polygon_ring;

      QgsGeometry geom;

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "buffer" ) << "buffer( $geometry, 1.0, 3)" << geom << false << true << geom.buffer( 1.0, 3 );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "buffer" ) << "buffer( $geometry, 2.0)" << geom << false << true << geom.buffer( 2.0, 8 );

      QgsPointXY point1( 10, 20 );
      QgsPointXY point2( 30, 20 );
      QgsGeometry pnt1 = QgsGeometry::fromPointXY( point1 );
      QgsGeometry pnt2 = QgsGeometry::fromPointXY( point2 );
      QTest::newRow( "union" ) << "union( $geometry, geomFromWKT('" + pnt2.asWkt() + "') )" << pnt1 << false << true << pnt1.combine( pnt2 );

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "intersection" ) << "intersection( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << geom << false << true << QgsGeometry::fromWkt( QStringLiteral( "POLYGON ((0 0,5 5,10 0,0 0))" ) );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "difference" ) << "difference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << geom << false << true << QgsGeometry::fromWkt( QStringLiteral( "POLYGON ((5 5,10 10,10 0,5 5))" ) );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "symDifference" ) << "symDifference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << geom << false << true << QgsGeometry::fromWkt( QStringLiteral( "MULTIPOLYGON(((5 5,0 0,0 10,5 5)),((5 5,10 10,10 0,5 5)))" ) );

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "convexHull simple" ) << "convexHull( $geometry )" << geom << false << true << geom.convexHull();
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "convexHull multi" ) << "convexHull( geomFromWKT('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))') )" << geom << false << false << QgsGeometry::fromWkt( QStringLiteral( "POLYGON ((0 0,0 1,1 1,1 0,0 0))" ) );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "bounds" ) << "bounds( $geometry )" << geom << false << true << QgsGeometry::fromRect( geom.boundingBox() );

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "oriented_bbox" ) << "oriented_bbox( $geometry )" << geom << false << true << geom.orientedMinimumBoundingBox( );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "minimal_circle" ) << "minimal_circle( $geometry )" << geom << false << true << geom.minimalEnclosingCircle( );

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "translate" ) << "translate( $geometry, 1, 2)" << geom << false << true << QgsGeometry::fromWkt( QStringLiteral( "POLYGON ((1 2,11 12,11 2,1 2))" ) );
      geom = QgsGeometry::fromPolylineXY( line );
      QTest::newRow( "translate" ) << "translate( $geometry, -1, 2)" << geom << false << true << QgsGeometry::fromWkt( QStringLiteral( "LINESTRING (-1 2, 9 12)" ) );
      geom = QgsGeometry::fromPointXY( point );
      QTest::newRow( "translate" ) << "translate( $geometry, 1, -2)" << geom << false << true << QgsGeometry::fromWkt( QStringLiteral( "POINT(1 -2)" ) );
    }

    void eval_geometry_method()
    {
      QFETCH( QString, string );
      QFETCH( QgsGeometry, geom );
      QFETCH( bool, evalError );
      QFETCH( bool, needGeom );
      QFETCH( QgsGeometry, result );

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), needGeom );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant out = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QVERIFY( compareWkt( outGeom.asWkt(), result.asWkt() ) );
    }

    void eval_eval()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( QStringLiteral( "col1" ) ) );
      fields.append( QgsField( QStringLiteral( "second_column" ), QVariant::Int ) );
      f.setFields( fields, true );
      f.setAttribute( QStringLiteral( "col1" ), QStringLiteral( "test value" ) );
      f.setAttribute( QStringLiteral( "second_column" ), 5 );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );

      QgsExpression exp1( QStringLiteral( "eval()" ) );
      QVariant v1 = exp1.evaluate( &context );

      QVERIFY( !v1.isValid() );

      QgsExpression exp2( QStringLiteral( "eval('4')" ) );
      QVariant v2 = exp2.evaluate( &context );
      QCOMPARE( v2, QVariant( 4 ) );

      QgsExpression exp3( QStringLiteral( "eval('\"second_column\" * 2')" ) );
      QVariant v3 = exp3.evaluate( &context );
      QCOMPARE( v3, QVariant( 10 ) );

      QgsExpression exp4( QStringLiteral( "eval('\"col1\"')" ) );
      QVariant v4 = exp4.evaluate( &context );
      QCOMPARE( v4, QVariant( "test value" ) );
    }

    void eval_generate_series()
    {
      QVariantList array;
      array << 1 << 2 << 3 << 4;
      QCOMPARE( QgsExpression( "generate_series(1,4)" ).evaluate(), QVariant( array ) );
      array.clear();
      array << 1 << 1.25 << 1.5 << 1.75 << 2;
      QCOMPARE( QgsExpression( "generate_series(1,2,0.25)" ).evaluate(), QVariant( array ) );
      array.clear();
      array << 10 << 9 << 8;
      QCOMPARE( QgsExpression( "generate_series(10,8,-1)" ).evaluate(), QVariant( array ) );

      QCOMPARE( QgsExpression( "generate_series(10,11,-1)" ).evaluate(), QVariant() );
      QCOMPARE( QgsExpression( "generate_series(10,5)" ).evaluate(), QVariant() );
      QCOMPARE( QgsExpression( "generate_series(1,2,0)" ).evaluate(), QVariant() );
    }

    void eval_string_array()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( QStringLiteral( "col1" ) ) );
      fields.append( QgsField( QStringLiteral( "strings" ), QVariant::StringList, QStringLiteral( "string[]" ), 0, 0, QString(), QVariant::String ) );
      f.setFields( fields, true );
      f.setAttribute( QStringLiteral( "col1" ), QStringLiteral( "test value" ) );
      QStringList array;
      array << QStringLiteral( "one" ) << QStringLiteral( "two" );
      f.setAttribute( QStringLiteral( "strings" ), array );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );

      QVariantList builderExpected;
      QCOMPARE( QgsExpression( "array()" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected << "hello";
      QCOMPARE( QgsExpression( "array('hello')" ).evaluate( &context ), QVariant( builderExpected ) );
      QCOMPARE( QgsExpression( "string_to_array('hello',',')" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected << "world";
      QCOMPARE( QgsExpression( "array('hello', 'world')" ).evaluate( &context ), QVariant( builderExpected ) );
      QCOMPARE( QgsExpression( "string_to_array('hello,world',',')" ).evaluate( &context ), QVariant( builderExpected ) );
      QCOMPARE( QgsExpression( "regexp_matches('hello=>world','([A-Za-z]*)=>([A-Za-z]*)')" ).evaluate( &context ), QVariant( builderExpected ) );

      builderExpected << QVariant();
      QCOMPARE( QgsExpression( "array('hello', 'world', NULL)" ).evaluate( &context ), QVariant( builderExpected ) );

      QCOMPARE( QgsExpression( "array_length(\"strings\")" ).evaluate( &context ), QVariant( 2 ) );

      QCOMPARE( QgsExpression( "array_contains(\"strings\", 'two')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_contains(\"strings\", 'three')" ).evaluate( &context ), QVariant( false ) );

      QCOMPARE( QgsExpression( "array_find(\"strings\", 'two')" ).evaluate( &context ), QVariant( 1 ) );
      QCOMPARE( QgsExpression( "array_find(\"strings\", 'three')" ).evaluate( &context ), QVariant( -1 ) );

      QCOMPARE( QgsExpression( "array_get(\"strings\", 1)" ).evaluate( &context ), QVariant( "two" ) );
      QCOMPARE( QgsExpression( "array_get(\"strings\", 2)" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array_get(\"strings\", -1)" ).evaluate( &context ), QVariant() );

      QStringList appendExpected = array;
      appendExpected << QStringLiteral( "three" );
      QCOMPARE( QgsExpression( "array_append(\"strings\", 'three')" ).evaluate( &context ), QVariant( appendExpected ) );

      QStringList prependExpected = array;
      prependExpected.prepend( QStringLiteral( "zero" ) );
      QCOMPARE( QgsExpression( "array_prepend(\"strings\", 'zero')" ).evaluate( &context ), QVariant( prependExpected ) );

      QStringList insertExpected = array;
      insertExpected.insert( 1, QStringLiteral( "one and a half" ) );
      QCOMPARE( QgsExpression( "array_insert(\"strings\", 1, 'one and a half')" ).evaluate( &context ), QVariant( insertExpected ) );

      QStringList removeAtExpected = array;
      removeAtExpected.removeAt( 0 );
      QCOMPARE( QgsExpression( "array_remove_at(\"strings\", 0)" ).evaluate( &context ), QVariant( removeAtExpected ) );

      QStringList removeAllExpected;
      removeAllExpected << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "d" );
      QCOMPARE( QgsExpression( "array_remove_all(array('a', 'b', 'c', 'd', 'c'), 'c')" ).evaluate( &context ), QVariant( removeAllExpected ) );

      QStringList concatExpected = array;
      concatExpected << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" );
      QCOMPARE( QgsExpression( "array_cat(\"strings\", array('a', 'b'), array('c'))" ).evaluate( &context ), QVariant( concatExpected ) );

      QVariantList foreachExpected;
      foreachExpected << QStringLiteral( "ABC" ) << QStringLiteral( "HELLO" );
      QCOMPARE( QgsExpression( "array_foreach(array:=array('abc', 'hello'), expression:=upper(@element))" ).evaluate( &context ), QVariant( foreachExpected ) );

      QVariantList filterExpected = QVariantList() << QStringLiteral( "A: a" ) << QStringLiteral( "A: d" );
      QCOMPARE( QgsExpression( "array_filter(array:=array('A: a', 'B: b', 'C: c', 'A: d'), expression:=substr(@element, 1, 2) = 'A:')" ).evaluate( &context ), QVariant( filterExpected ) );

      QCOMPARE( QgsExpression( "array_intersect(array('1', '2', '3', '4'), array('4', '0', '2', '5'))" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_intersect(array('1', '2', '3', '4'), array('0', '5'))" ).evaluate( &context ), QVariant( false ) );

      QCOMPARE( QgsExpression( "array_reverse(array('Dufour','Valmiera','Chugiak','Wien','Pisa','Lyon','Essen','Nødebo','Las Palmas')) = array('Las Palmas','Nødebo','Essen','Lyon','Pisa','Wien','Chugiak','Valmiera','Dufour')" ).evaluate( &context ), QVariant( true ) );

      QCOMPARE( QgsExpression( "array_slice(array('Dufour','Valmiera','Chugiak','Brighton'),1,2) = array('Valmiera','Chugiak')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice(array('Dufour','Valmiera','Chugiak','Brighton'),-2,-1) = array('Chugiak','Brighton')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice( array(), 0, 3) = array()" ).evaluate( &context ), QVariant( true ) );

      QCOMPARE( QgsExpression( "array_sort(array('Banana','Cake','Apple'))" ).evaluate( & context ), QVariant( QStringList() << QStringLiteral( "Apple" ) <<  QStringLiteral( "Banana" ) << QStringLiteral( "Cake" ) ) );
      QCOMPARE( QgsExpression( "array_sort(array('Banana','Cake','Apple'),false)" ).evaluate( & context ), QVariant( QStringList() << QStringLiteral( "Cake" ) <<  QStringLiteral( "Banana" ) << QStringLiteral( "Apple" ) ) );
    }

    void eval_int_array()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( QStringLiteral( "col1" ) ) );
      fields.append( QgsField( QStringLiteral( "ints" ), QVariant::List, QStringLiteral( "int[]" ), 0, 0, QString(), QVariant::Int ) );
      f.setFields( fields, true );
      f.setAttribute( QStringLiteral( "col1" ), QStringLiteral( "test value" ) );
      QVariantList array;
      array << 1 << -2;
      f.setAttribute( QStringLiteral( "ints" ), array );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );

      QVariantList builderExpected;
      builderExpected << 1;
      QCOMPARE( QgsExpression( "array(1)" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected << 2;
      QCOMPARE( QgsExpression( "array(1, 2)" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected << QVariant();
      QCOMPARE( QgsExpression( "array(1, 2, NULL)" ).evaluate( &context ), QVariant( builderExpected ) );

      QCOMPARE( QgsExpression( "array_contains(\"ints\", 1)" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_contains(\"ints\", 2)" ).evaluate( &context ), QVariant( false ) );

      QCOMPARE( QgsExpression( "array_find(\"ints\", -2)" ).evaluate( &context ), QVariant( 1 ) );
      QCOMPARE( QgsExpression( "array_find(\"ints\", 3)" ).evaluate( &context ), QVariant( -1 ) );

      QCOMPARE( QgsExpression( "array_get(\"ints\", 1)" ).evaluate( &context ), QVariant( -2 ) );
      QCOMPARE( QgsExpression( "array_get(\"ints\", 2)" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array_get(\"ints\", -1)" ).evaluate( &context ), QVariant() );

      QVariantList appendExpected = array;
      appendExpected << 3;
      QCOMPARE( QgsExpression( "array_append(\"ints\", 3)" ).evaluate( &context ), QVariant( appendExpected ) );

      QVariantList prependExpected = array;
      prependExpected.prepend( 0 );
      QCOMPARE( QgsExpression( "array_prepend(\"ints\", 0)" ).evaluate( &context ), QVariant( prependExpected ) );

      QVariantList insertExpected = array;
      insertExpected.insert( 1, 2 );
      QCOMPARE( QgsExpression( "array_insert(\"ints\", 1, 2)" ).evaluate( &context ), QVariant( insertExpected ) );

      QVariantList removeAtExpected = array;
      removeAtExpected.removeAt( 0 );
      QCOMPARE( QgsExpression( "array_remove_at(\"ints\", 0)" ).evaluate( &context ), QVariant( removeAtExpected ) );

      QVariantList removeAllExpected;
      removeAllExpected << 1 << 2 << 4;
      QCOMPARE( QgsExpression( "array_remove_all(array(1, 2, 3, 4, 3), 3)" ).evaluate( &context ), QVariant( removeAllExpected ) );
      QCOMPARE( QgsExpression( "array_remove_all(array(1, 2, 3, 4, 3), '3')" ).evaluate( &context ), QVariant( removeAllExpected ) );

      QVariantList concatExpected = array;
      concatExpected << 56 << 57;
      QCOMPARE( QgsExpression( "array_cat(\"ints\", array(56, 57))" ).evaluate( &context ), QVariant( concatExpected ) );

      QCOMPARE( QgsExpression( "array_intersect(array(1, 2, 3, 4), array(4, 0, 2, 5))" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_intersect(array(1, 2, 3, 4), array(0, 5))" ).evaluate( &context ), QVariant( false ) );

      QCOMPARE( QgsExpression( "array_reverse(array(2,4,0,10)) = array(10,0,4,2)" ).evaluate( &context ), QVariant( true ) );

      QCOMPARE( QgsExpression( "array_slice(array(1,2,3,4,5),0,3) = array(1,2,3,4)" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice(array(1,2,3,4,5),0,-1) = array(1,2,3,4,5)" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice(array(1,2,3,4,5),-5,-1) = array(1,2,3,4,5)" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice(array(1,2,3,4,5),0,0) = array(1)" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice(array(1,2,3,4,5),-2,-1) = array(4,5)" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice(array(1,2,3,4,5),-1,-1) = array(5)" ).evaluate( &context ), QVariant( true ) );

      QCOMPARE( QgsExpression( "array_sort(array(1,10,2,30,4))" ).evaluate( &context ), QVariant( QVariantList() << 1 << 2 << 4 << 10 << 30 ) );
      QCOMPARE( QgsExpression( "array_sort(array(1,10,2,30,4),false)" ).evaluate( &context ), QVariant( QVariantList() << 30 << 10 << 4 << 2 << 1 ) );

      QVariantList foreachExpected;
      foreachExpected << 10 << 20 << 40;
      QCOMPARE( QgsExpression( "array_foreach(array(1, 2, 4), @element * 10)" ).evaluate( &context ), QVariant( foreachExpected ) );

      QVariantList filterExpected = QVariantList() << 1 << 2;
      QCOMPARE( QgsExpression( "array_filter(array(1, 2, 4), @element < 3)" ).evaluate( &context ), QVariant( filterExpected ) );

      QgsExpression badArray( QStringLiteral( "array_get('not an array', 0)" ) );
      QCOMPARE( badArray.evaluate( &context ), QVariant() );
      QVERIFY( badArray.hasEvalError() );
      QCOMPARE( badArray.evalErrorString(), QString( "Cannot convert 'not an array' to array" ) );
    }

    void compare_arrays()
    {
      QCOMPARE( QgsExpression( "array() = array()" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(NULL) = array(NULL)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array() = array(NULL)" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array(1, NULL) = array(NULL, 1)" ).evaluate(), QVariant( false ) );

      QCOMPARE( QgsExpression( "array('hello') = array('hello')" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array('hello') = array('hello2')" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('h', 'e', 'l', 'l', 'o') = array('h', 'e', 'l', 'l', 'o')" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array('h', 'e', 'l', 'l', 'o') = array('h', 'e', 'l', 'l')" ).evaluate(), QVariant( false ) );

      QCOMPARE( QgsExpression( "array('1') = array(1)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array('1.2') = array(1.2)" ).evaluate(), QVariant( true ) );

      QCOMPARE( QgsExpression( "array() != array()" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array(NULL) != array(NULL)" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array() != array(NULL)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array('hello') != array('hello')" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('hello') != array('hello2')" ).evaluate(), QVariant( true ) );

      QCOMPARE( QgsExpression( "array() < array(1)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(1) < array(NULL)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(1) < array(1)" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array(1) < array(2)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(2) < array(1)" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array(1) < array(1, 2)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(1, 2) < array(1)" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('h', 'e', 'l', 'l', 'o') < array('h', 'e', 'l', 'l')" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('h', 'e', 'l', 'l', 'o') > array('h', 'e', 'l', 'l')" ).evaluate(), QVariant( true ) );

      QCOMPARE( QgsExpression( "array() <= array(1)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(1) <= array(NULL)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(1) <= array(1)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(1) <= array(2)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(2) <= array(1)" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array(1) <= array(1, 2)" ).evaluate(), QVariant( true ) );
      QCOMPARE( QgsExpression( "array(1, 2) <= array(1)" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('h', 'e', 'l', 'l', 'o') <= array('h', 'e', 'l', 'l')" ).evaluate(), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('h', 'e', 'l', 'l', 'o') >= array('h', 'e', 'l', 'l')" ).evaluate(), QVariant( true ) );
    }

    void eval_map()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( QStringLiteral( "col1" ) ) );
      fields.append( QgsField( QStringLiteral( "map" ), QVariant::Map, QStringLiteral( "map" ), 0, 0, QString(), QVariant::String ) );
      f.setFields( fields, true );
      f.setAttribute( QStringLiteral( "col1" ), QStringLiteral( "test value" ) );
      QVariantMap map;
      map[QStringLiteral( "1" )] = "one";
      map[QStringLiteral( "2" )] = "two";
      f.setAttribute( QStringLiteral( "map" ), map );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );

      QVariantMap builderExpected;
      QCOMPARE( QgsExpression( "map()" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected[QStringLiteral( "1" )] = "hello";
      QCOMPARE( QgsExpression( "map('1', 'hello')" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected[QStringLiteral( "2" )] = "world";
      QCOMPARE( QgsExpression( "map('1', 'hello', '2', 'world')" ).evaluate( &context ), QVariant( builderExpected ) );
      QCOMPARE( QgsExpression( "map('1', 'hello', '2', 'world', 'ignoredOddParam')" ).evaluate( &context ), QVariant( builderExpected ) );

      QCOMPARE( QgsExpression( "map_get(\"map\", '2')" ).evaluate( &context ), QVariant( "two" ) );
      QCOMPARE( QgsExpression( "map_get(\"map\", '3')" ).evaluate( &context ), QVariant() );

      QCOMPARE( QgsExpression( "map_exist(\"map\", '2')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "map_exist(\"map\", '3')" ).evaluate( &context ), QVariant( false ) );

      QVariantMap deleteExpected = map;
      deleteExpected.remove( QStringLiteral( "1" ) );
      QCOMPARE( QgsExpression( "map_delete(\"map\", '1')" ).evaluate( &context ), QVariant( deleteExpected ) );
      QCOMPARE( QgsExpression( "map_delete(\"map\", '3')" ).evaluate( &context ), QVariant( map ) );

      QVariantMap insertExpected = map;
      insertExpected.insert( QStringLiteral( "3" ), "three" );
      QCOMPARE( QgsExpression( "map_insert(\"map\", '3', 'three')" ).evaluate( &context ), QVariant( insertExpected ) );

      QVariantMap concatExpected;
      concatExpected[QStringLiteral( "1" )] = "one";
      concatExpected[QStringLiteral( "2" )] = "two";
      concatExpected[QStringLiteral( "3" )] = "three";
      QCOMPARE( QgsExpression( "map_concat(map('1', 'one', '2', 'overridden by next map'), map('2', 'two', '3', 'three'))" ).evaluate( &context ), QVariant( concatExpected ) );

      QCOMPARE( QgsExpression( "json_to_map('{\"1\":\"one\",\"2\":\"two\",\"3\":\"three\"}')" ).evaluate( &context ), QVariant( concatExpected ) );
      QCOMPARE( QgsExpression( "map_to_json(map('1','one','2','two','3','three'))" ).evaluate( &context ), QVariant( "{\"1\":\"one\",\"2\":\"two\",\"3\":\"three\"}" ) );

      QCOMPARE( QgsExpression( "hstore_to_map('1=>one,2=>two,3=>three')" ).evaluate( &context ), QVariant( concatExpected ) );
      QCOMPARE( QgsExpression( "map_to_hstore(map('1','one','2','two','3','three'))" ).evaluate( &context ), QVariant( "\"1\"=>\"one\",\"2\"=>\"two\",\"3\"=>\"three\"" ) );

      QVariantMap hstoreExpected;
      hstoreExpected[QStringLiteral( "test_quotes" )] = "test \"quote\" symbol";
      hstoreExpected[QStringLiteral( "test_slashes" )] = "test \\slash symbol";
      hstoreExpected[QStringLiteral( "test_mix" )] = "key with value in quotation marks";
      QCOMPARE( QgsExpression( "hstore_to_map('\"test_quotes\"=>\"test \\\\\"quote\\\\\" symbol\",\"test_slashes\"=>\"test \\\\slash symbol\",test_mix=>\"key with value in quotation marks\"')" ).evaluate( &context ), QVariant( hstoreExpected ) );

      hstoreExpected.clear();
      hstoreExpected[QStringLiteral( "1" )] = "one";
      // if a key is missing its closing quote, the map construction process will stop and a partial map is returned
      QCOMPARE( QgsExpression( "hstore_to_map('\"1\"=>\"one\",\"2=>\"two\"')" ).evaluate( &context ), QVariant( hstoreExpected ) );

      QStringList keysExpected;
      keysExpected << QStringLiteral( "1" ) << QStringLiteral( "2" );
      QCOMPARE( QgsExpression( "map_akeys(\"map\")" ).evaluate( &context ), QVariant( keysExpected ) );

      QVariantList valuesExpected;
      valuesExpected << "one" << "two";
      QCOMPARE( QgsExpression( "map_avals(\"map\")" ).evaluate( &context ), QVariant( valuesExpected ) );

      QgsExpression badMap( QStringLiteral( "map_get('not a map', '0')" ) );
      QCOMPARE( badMap.evaluate( &context ), QVariant() );
      QVERIFY( badMap.hasEvalError() );
      QCOMPARE( badMap.evalErrorString(), QString( "Cannot convert 'not a map' to map" ) );
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
      QVariantList array = QVariantList() << QVariant( 1 ) << QVariant( "a" ) << QVariant();
      QCOMPARE( QgsExpression::quotedValue( array ), QString( "array( 1, 'a', NULL )" ) );
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
      QgsExpression *exp = new QgsExpression( QStringLiteral( "Pilots > 2" ) );

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
      mPointsLayer->deleteAttribute( mPointsLayer->fields().lookupField( QStringLiteral( "Pilots" ) ) );

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
      QgsExpression nodeExpression( QStringLiteral( "1 IN (1, 2, 3, 4)" ) );
      QgsExpression nodeExpression2( nodeExpression );
    }

    void test_columnRefUnprepared()
    {
      //test retrieving fields from feature when expression is unprepared - explicitly specified fields collection
      //should take precedence over feature's field collection

      QgsFields fields;
      fields.append( QgsField( QStringLiteral( "f1" ), QVariant::String ) );

      QgsFeature f( 1 );
      f.setFields( fields );

      //also add a joined field - this will not be available in feature's field collection
      fields.append( QgsField( QStringLiteral( "j1" ), QVariant::String ), QgsFields::OriginJoin, 1 );

      f.setAttributes( QgsAttributes() << QVariant( "f1" ) << QVariant( "j1" ) );
      f.setValid( true );

      QgsExpression e( QStringLiteral( "\"f1\"" ) );
      QgsExpressionContext context;
      context.setFeature( f );
      context.setFields( fields );
      QVariant result = e.evaluate( &context );
      QCOMPARE( result.toString(), QString( "f1" ) );

      //test joined field
      QgsExpression e2( QStringLiteral( "\"j1\"" ) );
      result = e2.evaluate( &context );
      QCOMPARE( result.toString(), QString( "j1" ) );

      // final test - check that feature's field collection is also used when corresponding field NOT found
      // in explicitly passed field collection
      fields.append( QgsField( QStringLiteral( "f2" ), QVariant::String ) );
      f.setFields( fields );
      f.setAttributes( QgsAttributes() << QVariant( "f1" ) << QVariant( "j1" ) << QVariant( "f2" ) );
      context.setFeature( f );
      QgsExpression e3( QStringLiteral( "\"f2\"" ) );
      result = e3.evaluate( &context );
      QCOMPARE( result.toString(), QString( "f2" ) );
    }

    void test_env()
    {
      QgsExpressionContext context;

#ifdef Q_OS_WIN
      _putenv( "TESTENV_STRING=Hello World" );
#else
      setenv( "TESTENV_STRING", "Hello World", 1 );
#endif

      QgsExpression e( QStringLiteral( "env('TESTENV_STRING')" ) );

      QVariant result = e.evaluate( &context );

      QCOMPARE( result.toString(), QStringLiteral( "Hello World" ) );
#ifdef Q_OS_WIN
      _putenv( "TESTENV_STRING=" );
      _putenv( "TESTENV_INT=5" );
#else
      unsetenv( "TESTENV_STRING" );
      setenv( "TESTENV_INT", "5", 1 );
#endif

      QgsExpression e2( QStringLiteral( "env('TESTENV_INT')" ) );

      QVariant result2 = e2.evaluate( &context );

      QCOMPARE( result2.toString(), QStringLiteral( "5" ) );
#ifdef Q_OS_WIN
      _putenv( "TESTENV_INT=" );
#else
      unsetenv( "TESTENV_INT" );
#endif

      QgsExpression e3( QStringLiteral( "env('TESTENV_I_DO_NOT_EXIST')" ) );
      QVariant result3 = e3.evaluate( &context );

      QVERIFY( result3.isNull() );
    }

    void test_formatPreviewString()
    {
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( "hello" ) ), QStringLiteral( "'hello'" ) );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( QVariantMap() ) ), QStringLiteral( "{}" ) );

      QVariantMap map;
      map[QStringLiteral( "1" )] = "One";
      map[QStringLiteral( "2" )] = "Two";
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( map ) ), QStringLiteral( "{ '1': 'One', '2': 'Two' }" ) );
      map[QStringLiteral( "3" )] = "A very long string that is going to be truncated";
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( map ) ), QStringLiteral( "{ '1': 'One', '2': 'Two', '3': 'A very long string that is… }" ) );

      QVariantList list;
      list << 1 << 2 << 3;
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( list ) ), QStringLiteral( "[ 1, 2, 3 ]" ) );

      QStringList stringList;
      stringList << QStringLiteral( "One" ) << QStringLiteral( "Two" ) << QStringLiteral( "A very long string that is going to be truncated" );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( stringList ) ),
                QStringLiteral( "[ 'One', 'Two', 'A very long string that is going to be tr… ]" ) );
    }

    void test_nowStatic()
    {
      QgsExpression e( QStringLiteral( "now()" ) );
      QgsExpressionContext ctx;
      e.prepare( &ctx );
      QVariant v = e.evaluate();
      QTest::qSleep( 1000 );
      QVariant v2 = e.evaluate();

      QCOMPARE( v.toDateTime().toMSecsSinceEpoch(), v2.toDateTime().toMSecsSinceEpoch() );
    }

    void test_IndexOperator()
    {
      QgsExpressionContext context;
      QgsExpression e( QStringLiteral( "'['" ) );
      QVariant result = e.evaluate( &context );
      QCOMPARE( result.toString(), QStringLiteral( "[" ) );
      e = QgsExpression( QStringLiteral( "']'" ) );
      QCOMPARE( e.evaluate( &context ).toString(), QStringLiteral( "]" ) );
      e = QgsExpression( QStringLiteral( "'[3]'" ) );
      QCOMPARE( e.evaluate( &context ).toString(), QStringLiteral( "[3]" ) );
      e = QgsExpression( QStringLiteral( "'a[3]'" ) );
      QCOMPARE( e.evaluate( &context ).toString(), QStringLiteral( "a[3]" ) );
      e = QgsExpression( QStringLiteral( "\"a[3]\"" ) );
      QCOMPARE( e.evaluate( &context ).toString(), QStringLiteral( "[a[3]]" ) );
      e = QgsExpression( QStringLiteral( "(1+2)[0]" ) );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( e.hasEvalError() );
      e = QgsExpression( QStringLiteral( "(1+2)['a']" ) );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( e.hasEvalError() );
      // arrays
      e = QgsExpression( QStringLiteral( "array(1,2,3)[0]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( QStringLiteral( "((array(1,2,3)))[0]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[1]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[2]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[-1]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[-2]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[-3]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[1+1]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[(3-2)]" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( QStringLiteral( "array(1,2,3)[3]" ) );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
      e = QgsExpression( QStringLiteral( "array(1,2,3)[-4]" ) );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this

      // maps
      e = QgsExpression( QStringLiteral( "map('a',1,'b',2,'c',3)[0]" ) );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
      e = QgsExpression( QStringLiteral( "map('a',1,'b',2,'c',3)['d']" ) );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
      e = QgsExpression( QStringLiteral( "map('a',1,'b',2,'c',3)['a']" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( QStringLiteral( "map('a',1,'b',2,'c',3)['b']" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( QStringLiteral( "map('a',1,'b',2,'c',3)['c']" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( QStringLiteral( "map('a',1,'bbb',2,'c',3)['b'||'b'||'b']" ) );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
    }


    void testSqliteFetchAndIncrementWithTranscationMode()
    {
      QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
      QTemporaryDir tempDir;
      QFile::copy( testDataDir + QStringLiteral( "kbs.qgs" ), tempDir.filePath( "kbs.qgs" ) );
      QFile::copy( testDataDir + QStringLiteral( "kbs.gpkg" ), tempDir.filePath( "kbs.gpkg" ) );

      QgsProject *project = QgsProject::instance();
      QVERIFY( project->read( tempDir.filePath( "kbs.qgs" ) ) );

      QgsVectorLayer *zustaendigkeitskataster = project->mapLayer<QgsVectorLayer *>( QStringLiteral( "zustaendigkeitkataster_2b5bb693_3151_4c82_967f_b49d4d348a17" ) );

      // There is a default expression setup, dear reader of this test
      QVERIFY( zustaendigkeitskataster->defaultValueDefinition( 0 ).expression().contains( "sqlite_fetch_and_increment" ) );

      zustaendigkeitskataster->startEditing();

      QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( zustaendigkeitskataster ) );
      QgsFeature feature = QgsVectorLayerUtils::createFeature( zustaendigkeitskataster, QgsGeometry(), QgsAttributeMap(), &context );
      QCOMPARE( feature.attribute( "T_Id" ).toInt(), 0 );
      feature.setAttribute( "url_behoerde", "url_behoerde" );
      feature.setAttribute( "url_kataster", "url_kataster" );
      zustaendigkeitskataster->addFeature( feature );

      QgsFeature feature2 = QgsVectorLayerUtils::createFeature( zustaendigkeitskataster, QgsGeometry(), QgsAttributeMap(), &context );
      QCOMPARE( feature2.attribute( "T_Id" ).toInt(), 1 );
      feature2.setAttribute( "url_behoerde", "url_behoerde_x" );
      feature2.setAttribute( "url_kataster", "url_kataster_x" );
      zustaendigkeitskataster->addFeature( feature2 );

      zustaendigkeitskataster->commitChanges();
      QCOMPARE( zustaendigkeitskataster->dataProvider()->featureCount(), 2l );

      QCOMPARE( zustaendigkeitskataster->editBuffer(), nullptr );
      QCOMPARE( zustaendigkeitskataster->dataProvider()->transaction(), nullptr );

      zustaendigkeitskataster->startEditing();
      QgsExpressionContext context2( QgsExpressionContextUtils::globalProjectLayerScopes( zustaendigkeitskataster ) );
      QgsFeature feature3 = QgsVectorLayerUtils::createFeature( zustaendigkeitskataster, QgsGeometry(), QgsAttributeMap(), &context );
      QCOMPARE( feature3.attribute( "T_Id" ).toInt(), 2 );
      feature3.setAttribute( "url_behoerde", "url_behoerde" );
      feature3.setAttribute( "url_kataster", "url_kataster" );
      zustaendigkeitskataster->addFeature( feature3 );

      QgsFeature feature4 = QgsVectorLayerUtils::createFeature( zustaendigkeitskataster, QgsGeometry(), QgsAttributeMap(), &context );
      QCOMPARE( feature4.attribute( "T_Id" ).toInt(), 3 );
      feature4.setAttribute( "url_behoerde", "url_behoerde_x" );
      feature4.setAttribute( "url_kataster", "url_kataster_x" );
      zustaendigkeitskataster->addFeature( feature4 );

      zustaendigkeitskataster->commitChanges();

      QCOMPARE( zustaendigkeitskataster->dataProvider()->featureCount(), 4l );
    }

};

QGSTEST_MAIN( TestQgsExpression )

#include "testqgsexpression.moc"
