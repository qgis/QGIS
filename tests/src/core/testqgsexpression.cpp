/**************************************************************************
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
#include "qgsapplication.h"
#include "qgstest.h"

#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QTextDocument>
#include <QtConcurrentMap>

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
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionutils.h"
#include "qgsmeshlayer.h"
#include <geos_c.h>

static void _parseAndEvalExpr( int arg )
{
  Q_UNUSED( arg );
  for ( int i = 0; i < 100; ++i )
  {
    QgsExpression exp( u"1 + 2 * 2"_s );
    exp.evaluate();
  }
}

class RunLambdaInThread : public QThread
{
    Q_OBJECT

  public:
    RunLambdaInThread( const std::function<void()> &function )
      : mFunction( function )
    {}

    void run() override
    {
      mFunction();
      exit();
    }

  private:
    std::function<void()> mFunction;
};

class TestQgsExpression : public QObject
{
    Q_OBJECT

  public:
    TestQgsExpression() = default;

  private:
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPointsLayerMetadata = nullptr;
    QgsVectorLayer *mMemoryLayer = nullptr;
    QgsVectorLayer *mAggregatesLayer = nullptr;
    QgsVectorLayer *mChildLayer2 = nullptr; // relation with composite keys
    QgsVectorLayer *mChildLayer = nullptr;
    QgsRasterLayer *mRasterLayer = nullptr;
    QgsRasterLayer *mRasterLayerWithAttributeTable = nullptr;
    QgsMeshLayer *mMeshLayer = nullptr;

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
      mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), u"ogr"_s );
      QgsProject::instance()->addMapLayer( mPointsLayer );
      mPointsLayer->serverProperties()->setTitle( u"layer title"_s );
      mPointsLayer->serverProperties()->setAbstract( u"layer abstract"_s );
      mPointsLayer->serverProperties()->setKeywordList( u"layer,keywords"_s );
      mPointsLayer->serverProperties()->setDataUrl( u"data url"_s );
      mPointsLayer->serverProperties()->setAttribution( u"layer attribution"_s );
      mPointsLayer->serverProperties()->setAttributionUrl( u"attribution url"_s );
      mPointsLayer->setMinimumScale( 500 );
      mPointsLayer->setMaximumScale( 1000 );
      mPointsLayer->setMapTipTemplate( u"Maptip with class = [% \"Class\" %]"_s );
      mPointsLayer->setDisplayExpression( u"'Display expression with class = ' ||  \"Class\""_s );

      mPointsLayerMetadata = new QgsVectorLayer( pointFileInfo.filePath(), pointFileInfo.completeBaseName() + "_metadata", u"ogr"_s );
      QgsProject::instance()->addMapLayer( mPointsLayerMetadata );
      QgsLayerMetadata metadata;
      metadata.setTitle( u"metadata title"_s );
      metadata.setAbstract( u"metadata abstract"_s );
      QMap<QString, QStringList> keywords;
      keywords.insert( u"key1"_s, QStringList() << u"val1"_s << u"val2"_s );
      keywords.insert( u"key2"_s, QStringList() << u"val3"_s );
      metadata.setKeywords( keywords );
      metadata.setRights( QStringList() << u"right1"_s << u"right2"_s );
      mPointsLayerMetadata->setMetadata( metadata );
      mPointsLayerMetadata->serverProperties()->setTitle( u"layer title"_s );
      mPointsLayerMetadata->serverProperties()->setAbstract( u"layer abstract"_s );
      mPointsLayerMetadata->serverProperties()->setKeywordList( u"layer,keywords"_s );
      mPointsLayerMetadata->serverProperties()->setDataUrl( u"data url"_s );
      mPointsLayerMetadata->serverProperties()->setAttribution( u"layer attribution"_s );
      mPointsLayerMetadata->serverProperties()->setAttributionUrl( u"attribution url"_s );

      QString rasterFileName = testDataDir + "tenbytenraster.asc";
      QFileInfo rasterFileInfo( rasterFileName );
      mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(), rasterFileInfo.completeBaseName() );
      QgsProject::instance()->addMapLayer( mRasterLayer );

      QString rasterWithAttributeTableFileName = testDataDir + "/raster/band1_byte_attribute_table_epsg4326.tif";
      QFileInfo rasterWithAttributeTableFileInfo( rasterWithAttributeTableFileName );
      mRasterLayerWithAttributeTable = new QgsRasterLayer( rasterWithAttributeTableFileInfo.filePath(), rasterWithAttributeTableFileInfo.completeBaseName() );
      Q_ASSERT( mRasterLayerWithAttributeTable->isValid() );
      QgsProject::instance()->addMapLayer( mRasterLayerWithAttributeTable );

      QString meshFileName = testDataDir + "/mesh/quad_flower.2dm";
      mMeshLayer = new QgsMeshLayer( meshFileName, "mesh layer", "mdal" );
      mMeshLayer->updateTriangularMesh();
      QgsProject::instance()->addMapLayer( mMeshLayer );

      // test memory layer for get_feature tests
      mMemoryLayer = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:string&field=datef:date(0,0)"_s, u"test"_s, u"memory"_s );
      QVERIFY( mMemoryLayer->isValid() );
      QgsFeature f1( mMemoryLayer->dataProvider()->fields(), 1 );
      f1.setAttribute( u"col1"_s, 10 );
      f1.setAttribute( u"col2"_s, "test1" );
      f1.setAttribute( u"datef"_s, QDate( 2021, 9, 23 ) );
      QgsFeature f2( mMemoryLayer->dataProvider()->fields(), 2 );
      f2.setAttribute( u"col1"_s, 11 );
      f2.setAttribute( u"col2"_s, "test2" );
      f2.setAttribute( u"datef"_s, QDate( 2022, 9, 23 ) );
      QgsFeature f3( mMemoryLayer->dataProvider()->fields(), 3 );
      f3.setAttribute( u"col1"_s, 3 );
      f3.setAttribute( u"col2"_s, "test3" );
      f3.setAttribute( u"datef"_s, QDate( 2021, 9, 23 ) );
      QgsFeature f4( mMemoryLayer->dataProvider()->fields(), 4 );
      f4.setAttribute( u"col1"_s, 41 );
      f4.setAttribute( u"col2"_s, "test4" );
      f4.setAttribute( u"datef"_s, QDate( 2022, 9, 23 ) );
      mMemoryLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );
      mMemoryLayer->setConstraintExpression( 0, u"col1 > 10"_s, u"col0 is not null"_s );
      mMemoryLayer->setFieldConstraint( 0, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthHard );
      QgsProject::instance()->addMapLayer( mMemoryLayer );

      // test layer for aggregates
      mAggregatesLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer"
                                                             "&field=col2:string"
                                                             "&field=col3:integer"
                                                             "&field=col4:string" ),
                                             u"aggregate_layer"_s, u"memory"_s );
      QVERIFY( mAggregatesLayer->isValid() );
      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      af1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 0, 0 ) ) );
      af1.setAttribute( u"col1"_s, 4 );
      af1.setAttribute( u"col2"_s, "test" );
      af1.setAttribute( u"col3"_s, 2 );
      af1.setAttribute( u"col4"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      QgsFeature af2( mAggregatesLayer->dataProvider()->fields(), 2 );
      af2.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 0 ) ) );
      af2.setAttribute( u"col1"_s, 1 );
      af2.setAttribute( u"col2"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      af2.setAttribute( u"col3"_s, 1 );
      af2.setAttribute( u"col4"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      QgsFeature af3( mAggregatesLayer->dataProvider()->fields(), 3 );
      af3.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 2, 0 ) ) );
      af3.setAttribute( u"col1"_s, 3 );
      af3.setAttribute( u"col2"_s, "test333" );
      af3.setAttribute( u"col3"_s, 2 );
      af3.setAttribute( u"col4"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      QgsFeature af4( mAggregatesLayer->dataProvider()->fields(), 4 );
      af4.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 0 ) ) );
      af4.setAttribute( u"col1"_s, 2 );
      af4.setAttribute( u"col2"_s, "test4" );
      af4.setAttribute( u"col3"_s, 2 );
      af4.setAttribute( u"col4"_s, "" );
      QgsFeature af5( mAggregatesLayer->dataProvider()->fields(), 5 );
      af5.setGeometry( QgsGeometry() );
      af5.setAttribute( u"col1"_s, 5 );
      af5.setAttribute( u"col2"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      af5.setAttribute( u"col3"_s, 3 );
      af5.setAttribute( u"col4"_s, "test" );
      QgsFeature af6( mAggregatesLayer->dataProvider()->fields(), 6 );
      af6.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 5, 0 ) ) );
      af6.setAttribute( u"col1"_s, 8 );
      af6.setAttribute( u"col2"_s, "test4" );
      af6.setAttribute( u"col3"_s, 3 );
      af6.setAttribute( u"col4"_s, "test" );
      QgsFeature af7( mAggregatesLayer->dataProvider()->fields(), 7 );
      af7.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 6, 0 ) ) );
      af7.setAttribute( u"col1"_s, 19 );
      af7.setAttribute( u"col2"_s, "test7" );
      af7.setAttribute( u"col3"_s, 1961 );
      af7.setAttribute( u"col4"_s, "Sputnik" );
      mAggregatesLayer->dataProvider()->addFeatures( QgsFeatureList() << af1 << af2 << af3 << af4 << af5 << af6 << af7 );
      QgsProject::instance()->addMapLayer( mAggregatesLayer );

      mChildLayer = new QgsVectorLayer( QStringLiteral( "Point?field=parent:integer"
                                                        "&field=col2:string"
                                                        "&field=col3:integer" ),
                                        u"child_layer"_s, u"memory"_s );
      QVERIFY( mChildLayer->isValid() );
      QgsFeature cf1( mChildLayer->dataProvider()->fields(), 1 );
      cf1.setAttribute( u"parent"_s, 4 );
      cf1.setAttribute( u"col2"_s, "test" );
      cf1.setAttribute( u"col3"_s, 2 );
      QgsFeature cf2( mChildLayer->dataProvider()->fields(), 2 );
      cf2.setAttribute( u"parent"_s, 4 );
      cf2.setAttribute( u"col2"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      cf2.setAttribute( u"col3"_s, 1 );
      QgsFeature cf3( mChildLayer->dataProvider()->fields(), 3 );
      cf3.setAttribute( u"parent"_s, 4 );
      cf3.setAttribute( u"col2"_s, "test333" );
      cf3.setAttribute( u"col3"_s, 2 );
      QgsFeature cf4( mChildLayer->dataProvider()->fields(), 4 );
      cf4.setAttribute( u"parent"_s, 3 );
      cf4.setAttribute( u"col2"_s, "test4" );
      cf4.setAttribute( u"col3"_s, 2 );
      QgsFeature cf5( mChildLayer->dataProvider()->fields(), 5 );
      cf5.setAttribute( u"parent"_s, 3 );
      cf5.setAttribute( u"col2"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      cf5.setAttribute( u"col3"_s, 7 );
      mChildLayer->dataProvider()->addFeatures( QgsFeatureList() << cf1 << cf2 << cf3 << cf4 << cf5 );
      QgsProject::instance()->addMapLayer( mChildLayer );

      QgsRelation rel;
      rel.setId( u"my_rel"_s );
      rel.setName( u"relation name"_s );
      rel.setReferencedLayer( mAggregatesLayer->id() );
      rel.setReferencingLayer( mChildLayer->id() );
      rel.addFieldPair( u"parent"_s, u"col1"_s );
      QVERIFY( rel.isValid() );
      QgsProject::instance()->relationManager()->addRelation( rel );

      mChildLayer2 = new QgsVectorLayer( QStringLiteral( "Point?field=name:string"
                                                         "&field=year:integer"
                                                         "&field=my_value:integer" ),
                                         u"child_layer_2"_s, u"memory"_s );
      QVERIFY( mChildLayer2->isValid() );
      QgsFeature afc1( mChildLayer2->dataProvider()->fields(), 1 );
      afc1.setAttribute( u"year"_s, 1961 );
      afc1.setAttribute( u"name"_s, u"Sputnik"_s );
      afc1.setAttribute( u"my_value"_s, 21070000 );
      QgsFeature afc2( mChildLayer2->dataProvider()->fields(), 2 );
      afc2.setAttribute( u"year"_s, 1961 );
      afc2.setAttribute( u"name"_s, u"Sputnik"_s );
      afc2.setAttribute( u"my_value"_s, 1969 );
      mChildLayer2->dataProvider()->addFeatures( QgsFeatureList() << afc1 << afc2 );
      QgsProject::instance()->addMapLayer( mChildLayer2 );

      QgsRelation rel_ck;
      rel_ck.setId( u"my_rel_composite"_s );
      rel_ck.setName( u"relation with composite"_s );
      rel_ck.setReferencedLayer( mAggregatesLayer->id() );
      rel_ck.setReferencingLayer( mChildLayer2->id() );
      rel_ck.addFieldPair( u"year"_s, u"col3"_s );
      rel_ck.addFieldPair( u"name"_s, u"col4"_s );
      QVERIFY( rel_ck.isValid() );
      QgsProject::instance()->relationManager()->addRelation( rel_ck );
    }

    void evalMeshElement()
    {
      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::meshExpressionScope( QgsMesh::Vertex ) );
      context.lastScope()->setVariable( u"_mesh_vertex_index"_s, 2 );
      const QgsMesh mesh = *mMeshLayer->nativeMesh();
      context.lastScope()->setVariable( u"_native_mesh"_s, QVariant::fromValue( mesh ) );

      QgsExpression expression( u"$vertex_x"_s );
      QCOMPARE( expression.evaluate( &context ).toDouble(), 2500.0 );

      expression = QgsExpression( u"$vertex_y"_s );
      QCOMPARE( expression.evaluate( &context ).toDouble(), 2500.0 );

      expression = QgsExpression( u"$vertex_z"_s );
      QCOMPARE( expression.evaluate( &context ).toDouble(), 800.0 );

      expression = QgsExpression( u"$vertex_index"_s );
      QCOMPARE( expression.evaluate( &context ).toInt(), 2 );

      expression = QgsExpression( u"$vertex_as_point"_s );
      QVariant out = expression.evaluate( &context );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QgsGeometry geom( new QgsPoint( 2500, 2500, 800 ) );
      QCOMPARE( geom.equals( outGeom ), true );

      context.appendScope( QgsExpressionContextUtils::meshExpressionScope( QgsMesh::Face ) );
      context.lastScope()->setVariable( u"_mesh_face_index"_s, 2 );
      expression = QgsExpression( u"$face_area"_s );
      QCOMPARE( expression.evaluate( &context ).toDouble(), 250000 );

      expression = QgsExpression( u"$face_index"_s );
      QCOMPARE( expression.evaluate( &context ).toInt(), 2 );
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

      QTest::newRow( "arithmetic" ) << "1+2*3" << true;
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

      QTest::newRow( "multiline comment after bool 1" ) << R"(True/*"b"*/)" << true;
      QTest::newRow( "multiline comment after bool 2" ) << "True/*\"\nb\"*/" << true;
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

    void parser_error_message_replace()
    {
      QgsExpression exp( QString( "1+" ) );
      QCOMPARE( exp.hasParserError(), true );
      QCOMPARE( exp.parserErrorString(), "\nIncomplete expression. You might not have finished the full expression." );
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

      QgsExpression exp( u"1.23 + 4.56"_s );
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

      QTest::newRow( "toint alias" ) << "toint(3.2)" << false << "to_int(3.20000000000000018)" << QVariant( 3 );
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
      if ( res.userType() != result.userType() )
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
      if ( res.userType() != result.userType() )
      {
        qDebug() << "got " << res.typeName() << " instead of " << result.typeName();
      }
      QCOMPARE( res, result );
      QCOMPARE( exp.dump(), dump );
    }

    void represent_attributes()
    {
      QgsVectorLayer layer { u"Point?field=col1:integer&field=col2:string"_s, u"test_represent_attributes"_s, u"memory"_s };
      QVERIFY( layer.isValid() );
      QgsFeature f1( layer.dataProvider()->fields(), 1 );
      f1.setAttribute( u"col1"_s, 1 );
      f1.setAttribute( u"col2"_s, "test1" );
      QgsFeature f2( layer.dataProvider()->fields(), 2 );
      f2.setAttribute( u"col1"_s, 2 );
      f2.setAttribute( u"col2"_s, "test2" );
      layer.dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 );

      QVariantMap config;
      QVariantMap map;
      map.insert( u"one"_s, u"1"_s );
      map.insert( u"two"_s, u"2"_s );

      config.insert( u"map"_s, map );
      layer.setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"ValueMap"_s, config ) );

      QgsExpressionContext context( { QgsExpressionContextUtils::layerScope( &layer ) } );
      context.setFeature( f2 );
      QgsExpression expression( "represent_attributes()" );

      if ( expression.hasParserError() )
        qDebug() << expression.parserErrorString();
      QVERIFY( !expression.hasParserError() );

      expression.prepare( &context );

      QVariantMap result { expression.evaluate( &context ).toMap() };

      QCOMPARE( result.value( u"col1"_s ).toString(), u"two"_s );
      QCOMPARE( result.value( u"col2"_s ).toString(), u"test2"_s );

      QgsExpressionContext context2( { QgsExpressionContextUtils::layerScope( &layer ) } );
      context2.setFeature( f2 );
      expression = QgsExpression( "represent_attributes($currentfeature)" );

      result = expression.evaluate( &context2 ).toMap();

      QVERIFY( !expression.hasEvalError() );

      QCOMPARE( result.value( u"col1"_s ).toString(), u"two"_s );
      QCOMPARE( result.value( u"col2"_s ).toString(), u"test2"_s );

      QgsProject::instance()->addMapLayer( &layer, false, false );
      QgsExpressionContext context3;
      context3.setFeature( f2 );
      expression = QgsExpression( "represent_attributes('test_represent_attributes', $currentfeature)" );

      result = expression.evaluate( &context3 ).toMap();

      QVERIFY( !expression.hasEvalError() );

      QCOMPARE( result.value( u"col1"_s ).toString(), u"two"_s );
      QCOMPARE( result.value( u"col2"_s ).toString(), u"test2"_s );

      // Test the cached value
      QCOMPARE( context3.cachedValue( u"repvalfcnval:%1:%2:%3"_s.arg( layer.id(), u"col1"_s, u"2"_s ) ).toString(), u"two"_s );

      // Test errors
      QgsProject::instance()->removeMapLayer( layer.id() );
      expression = QgsExpression( "represent_attributes('test_represent_attributes', $currentfeature)" );
      QgsExpressionContext context4;
      result = expression.evaluate( &context4 ).toMap();
      QVERIFY( expression.hasEvalError() );
    };

    void represent_value()
    {
      QVariantMap config;
      QVariantMap map;
      map.insert( u"one"_s, u"1"_s );
      map.insert( u"two"_s, u"2"_s );
      map.insert( u"three"_s, u"3"_s );

      config.insert( u"map"_s, map );
      mPointsLayer->setEditorWidgetSetup( 3, QgsEditorWidgetSetup( u"ValueMap"_s, config ) );

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
      QCOMPARE( expression.evaluate( &context ).toString(), u"one"_s );

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
      QCOMPARE( expression2.evaluate( &context ).toString(), u"Jet"_s );

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

      QCOMPARE( expression.evaluate( &context ).toString(), u"one"_s );
      expression3.prepare( &context );
      QCOMPARE( expression.evaluate( &context ).toString(), u"one"_s );

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

    void fieldsButNoFeature()
    {
      // test evaluating an expression with fields in the context but no feature
      QgsExpressionContext context;
      QgsExpressionContextScope *scope = new QgsExpressionContextScope();

      QgsFields fields;
      fields.append( QgsField( u"x"_s ) );
      fields.append( QgsField( u"y"_s ) );
      fields.append( QgsField( u"z"_s ) );
      scope->setFields( fields );
      context.appendScope( scope );

      // doesn't exist
      QgsExpression expression( "\"a\"" );
      QVERIFY( !expression.hasParserError() );
      QVERIFY( !expression.evaluate( &context ).isValid() );
      QVERIFY( expression.hasEvalError() );
      QCOMPARE( expression.evalErrorString(), u"Field 'a' not found"_s );
      expression = QgsExpression( "\"x\"" );
      QVERIFY( !expression.hasParserError() );
      QVERIFY( !expression.evaluate( &context ).isValid() );
      QVERIFY( expression.hasEvalError() );
      QCOMPARE( expression.evalErrorString(), u"No feature available for field 'x' evaluation"_s );
      expression = QgsExpression( "\"y\"" );
      QVERIFY( !expression.hasParserError() );
      QVERIFY( !expression.evaluate( &context ).isValid() );
      QVERIFY( expression.hasEvalError() );
      QCOMPARE( expression.evalErrorString(), u"No feature available for field 'y' evaluation"_s );

      QgsFeature f( fields );
      f.setValid( true );
      f.setAttributes( QgsAttributes() << 1 << 2 << 3 );
      scope->setFeature( f );
      expression = QgsExpression( "\"z\"" );
      QVERIFY( !expression.hasParserError() );
      QCOMPARE( expression.evaluate( &context ).toInt(), 3 );
      QVERIFY( !expression.hasEvalError() );
    }

    void evaluation_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      // literal evaluation
      QTest::newRow( "literal null" ) << "NULL" << false << QVariant();
      QTest::newRow( "literal int" ) << "123" << false << QVariant( 123 );
      QTest::newRow( "literal int64" ) << "1234567890123456789" << false << QVariant( qlonglong( 1234567890123456789 ) );
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

      // arithmetic
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

      // equal

      QTest::newRow( "eq int" ) << "1+1 = 2" << false << QVariant( 1 );
      QTest::newRow( "eq double" ) << "3.2 = 2.2+1" << false << QVariant( 1 );
      QTest::newRow( "eq string" ) << "'a' = 'b'" << false << QVariant( 0 );
      QTest::newRow( "eq null" ) << "2 = null" << false << QVariant();
      QTest::newRow( "eq mixed" ) << "'a' = 1" << false << QVariant( 0 );

      // equal, boolean values

      QTest::newRow( "eq bool both true/true" ) << "TRUE = TRUE" << false << QVariant( 1 );
      QTest::newRow( "eq bool both true/false" ) << "TRUE = FALSE" << false << QVariant( 0 );
      QTest::newRow( "eq bool both false/true" ) << "FALSE = TRUE" << false << QVariant( 0 );
      QTest::newRow( "eq bool both false/false" ) << "FALSE = FALSE" << false << QVariant( 1 );
      QTest::newRow( "eq bool first int second true true/true" ) << "TRUE = 1" << false << QVariant( 1 );
      QTest::newRow( "eq bool first int second false true/false" ) << "TRUE = 0" << false << QVariant( 0 );
      QTest::newRow( "eq bool first int second true false/true" ) << "FALSE = 1" << false << QVariant( 0 );
      QTest::newRow( "eq bool first int second false false/false" ) << "FALSE = 0" << false << QVariant( 1 );
      QTest::newRow( "eq int first bool second true true/true" ) << "1 = TRUE" << false << QVariant( 1 );
      QTest::newRow( "eq int first bool second false true/false" ) << "1 = FALSE" << false << QVariant( 0 );
      QTest::newRow( "eq int first bool second true false/true" ) << "0 = TRUE" << false << QVariant( 0 );
      QTest::newRow( "eq int first bool second false false/false" ) << "0 = FALSE" << false << QVariant( 1 );
      QTest::newRow( "eq bool first string int second true true/true" ) << "TRUE = '1'" << false << QVariant( 1 );
      QTest::newRow( "eq bool first string int second false true/false" ) << "TRUE = '0'" << false << QVariant( 0 );
      QTest::newRow( "eq bool first string int second true false/true" ) << "FALSE = '1'" << false << QVariant( 0 );
      QTest::newRow( "eq bool first string int second false false/false" ) << "FALSE = '0'" << false << QVariant( 1 );
      QTest::newRow( "eq string int first bool second true true/true" ) << "'1' = TRUE" << false << QVariant( 1 );
      QTest::newRow( "eq string int first bool second false true/false" ) << "'1' = FALSE" << false << QVariant( 0 );
      QTest::newRow( "eq string int first bool second true false/true" ) << "'0' = TRUE" << false << QVariant( 0 );
      QTest::newRow( "eq string int first bool second false false/false" ) << "'0' = FALSE" << false << QVariant( 1 );
      QTest::newRow( "eq bool first string bool second true true/true" ) << "TRUE = 'true'" << false << QVariant( 1 );
      QTest::newRow( "eq bool first string bool second false true/false" ) << "TRUE = 'false'" << false << QVariant( 0 );
      QTest::newRow( "eq bool first string bool second true false/true" ) << "FALSE = 'true'" << false << QVariant( 0 );
      QTest::newRow( "eq bool first string bool second false false/false" ) << "FALSE = 'false'" << false << QVariant( 1 );
      QTest::newRow( "eq string bool first bool second true true/true" ) << "'true' = TRUE" << false << QVariant( 1 );
      QTest::newRow( "eq string bool first bool second false true/false" ) << "'true' = FALSE" << false << QVariant( 0 );
      QTest::newRow( "eq string bool first bool second true false/true" ) << "'false' = TRUE" << false << QVariant( 0 );
      QTest::newRow( "eq string bool first bool second false false/false" ) << "'false' = FALSE" << false << QVariant( 1 );

      // not equal
      QTest::newRow( "ne int 1" ) << "3 != 4" << false << QVariant( 1 );
      QTest::newRow( "ne int 2" ) << "3 != 3" << false << QVariant( 0 );

      // not equal, boolean values

      QTest::newRow( "ne bool both true/true" ) << "TRUE != TRUE" << false << QVariant( 0 );
      QTest::newRow( "ne bool both true/false" ) << "TRUE != FALSE" << false << QVariant( 1 );
      QTest::newRow( "ne bool both false/true" ) << "FALSE != TRUE" << false << QVariant( 1 );
      QTest::newRow( "ne bool both false/false" ) << "FALSE != FALSE" << false << QVariant( 0 );
      QTest::newRow( "ne bool first int second true true/true" ) << "TRUE != 1" << false << QVariant( 0 );
      QTest::newRow( "ne bool first int second false true/false" ) << "TRUE != 0" << false << QVariant( 1 );
      QTest::newRow( "ne bool first int second true false/true" ) << "FALSE != 1" << false << QVariant( 1 );
      QTest::newRow( "ne bool first int second false false/false" ) << "FALSE != 0" << false << QVariant( 0 );
      QTest::newRow( "ne int first bool second true true/true" ) << "1 != TRUE" << false << QVariant( 0 );
      QTest::newRow( "ne int first bool second false true/false" ) << "1 != FALSE" << false << QVariant( 1 );
      QTest::newRow( "ne int first bool second true false/true" ) << "0 != TRUE" << false << QVariant( 1 );
      QTest::newRow( "ne int first bool second false false/false" ) << "0 != FALSE" << false << QVariant( 0 );
      QTest::newRow( "ne bool first string int second true true/true" ) << "TRUE != '1'" << false << QVariant( 0 );
      QTest::newRow( "ne bool first string int second false true/false" ) << "TRUE != '0'" << false << QVariant( 1 );
      QTest::newRow( "ne bool first string int second true false/true" ) << "FALSE != '1'" << false << QVariant( 1 );
      QTest::newRow( "ne bool first string int second false false/false" ) << "FALSE != '0'" << false << QVariant( 0 );
      QTest::newRow( "ne string int first bool second true true/true" ) << "'1' != TRUE" << false << QVariant( 0 );
      QTest::newRow( "ne string int first bool second false true/false" ) << "'1' != FALSE" << false << QVariant( 1 );
      QTest::newRow( "ne string int first bool second true false/true" ) << "'0' != TRUE" << false << QVariant( 1 );
      QTest::newRow( "ne string int first bool second false false/false" ) << "'0' != FALSE" << false << QVariant( 0 );
      QTest::newRow( "ne bool first string bool second true true/true" ) << "TRUE != 'true'" << false << QVariant( 0 );
      QTest::newRow( "ne bool first string bool second false true/false" ) << "TRUE != 'false'" << false << QVariant( 1 );
      QTest::newRow( "ne bool first string bool second true false/true" ) << "FALSE != 'true'" << false << QVariant( 1 );
      QTest::newRow( "ne bool first string bool second false false/false" ) << "FALSE != 'false'" << false << QVariant( 0 );
      QTest::newRow( "ne string bool first bool second true true/true" ) << "'true' != TRUE" << false << QVariant( 0 );
      QTest::newRow( "ne string bool first bool second false true/false" ) << "'true' != FALSE" << false << QVariant( 1 );
      QTest::newRow( "ne string bool first bool second true false/true" ) << "'false' != TRUE" << false << QVariant( 1 );
      QTest::newRow( "ne string bool first bool second false false/false" ) << "'false' != FALSE" << false << QVariant( 0 );

      // less than

      QTest::newRow( "lt int 1" ) << "3 < 4" << false << QVariant( 1 );
      QTest::newRow( "lt int 2" ) << "3 < 3" << false << QVariant( 0 );

      // greater than

      QTest::newRow( "gt int 1" ) << "3 > 4" << false << QVariant( 0 );
      QTest::newRow( "gt int 2" ) << "3 > 3" << false << QVariant( 0 );

      // less than or equal to

      QTest::newRow( "le int 1" ) << "3 <= 4" << false << QVariant( 1 );
      QTest::newRow( "le int 2" ) << "3 <= 3" << false << QVariant( 1 );

      // greater than or equal to

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
      QTest::newRow( "to_interval('1 minute') < to_interval('20 days')" ) << "to_interval('1 minute') < to_interval('20 days')" << false << QVariant( 1 );
      QTest::newRow( "to_interval('1 minute') > to_interval('20 days')" ) << "to_interval('1 minute') > to_interval('20 days')" << false << QVariant( 0 );
      QTest::newRow( "to_interval('1 minute') = to_interval('20 days')" ) << "to_interval('1 minute') = to_interval('20 days')" << false << QVariant( 0 );
      QTest::newRow( "to_interval('1 minute') != to_interval('20 days')" ) << "to_interval('1 minute') != to_interval('20 days')" << false << QVariant( 1 );
      QTest::newRow( "to_interval('1 minute') = to_interval('60 seconds')" ) << "to_interval('1 minute') = to_interval('60 seconds')" << false << QVariant( 1 );
      QTest::newRow( "make_date(2010,9,8) < make_date(2010,9,9)" ) << "make_date(2010,9,8) < make_date(2010,9,9)" << false << QVariant( 1 );
      QTest::newRow( "make_date(2010,9,8) > make_date(2010,9,9)" ) << "make_date(2010,9,8) > make_date(2010,9,9)" << false << QVariant( 0 );
      QTest::newRow( "make_date(2010,9,9) > make_date(2010,9,8)" ) << "make_date(2010,9,9) > make_date(2010,9,8)" << false << QVariant( 1 );
      QTest::newRow( "make_date(2010,9,9) < make_date(2010,9,8)" ) << "make_date(2010,9,9) < make_date(2010,9,8)" << false << QVariant( 0 );
      QTest::newRow( "make_date(2010,9,8) = make_date(2010,9,8)" ) << "make_date(2010,9,8) = make_date(2010,9,8)" << false << QVariant( 1 );
      QTest::newRow( "make_date(2010,9,8) = make_date(2010,9,9)" ) << "make_date(2010,9,8) = make_date(2010,9,9)" << false << QVariant( 0 );
      QTest::newRow( "make_date(2010,9,8) != make_date(2010,9,9)" ) << "make_date(2010,9,8) != make_date(2010,9,9)" << false << QVariant( 1 );
      QTest::newRow( "make_date(2010,9,8) != make_date(2010,9,8)" ) << "make_date(2010,9,8) != make_date(2010,9,8)" << false << QVariant( 0 );
      QTest::newRow( "make_time(12,9,8) < make_time(12,9,9)" ) << "make_time(12,9,8) < make_time(12,9,9)" << false << QVariant( 1 );
      QTest::newRow( "make_time(12,9,8) > make_time(12,9,9)" ) << "make_time(12,9,8) > make_time(12,9,9)" << false << QVariant( 0 );
      QTest::newRow( "make_time(12,9,9) > make_time(12,9,8)" ) << "make_time(12,9,9) > make_time(12,9,8)" << false << QVariant( 1 );
      QTest::newRow( "make_time(12,9,9) < make_time(12,9,8)" ) << "make_time(12,9,9) < make_time(12,9,8)" << false << QVariant( 0 );
      QTest::newRow( "make_time(12,9,8) = make_time(12,9,8)" ) << "make_time(12,9,8) = make_time(12,9,8)" << false << QVariant( 1 );
      QTest::newRow( "make_time(12,9,8) = make_time(12,9,9)" ) << "make_time(12,9,8) = make_time(12,9,9)" << false << QVariant( 0 );
      QTest::newRow( "make_time(12,9,8) != make_time(12,9,9)" ) << "make_time(12,9,8) != make_time(12,9,9)" << false << QVariant( 1 );
      QTest::newRow( "make_time(12,9,8) != make_time(12,9,8)" ) << "make_time(12,9,8) != make_time(12,9,8)" << false << QVariant( 0 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,8) < make_datetime(2012,3,4,12,9,9)" ) << "make_datetime(2012,3,4,12,9,8) < make_datetime(2012,3,4,12,9,9)" << false << QVariant( 1 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,8) > make_datetime(2012,3,4,12,9,9)" ) << "make_datetime(2012,3,4,12,9,8) > make_datetime(2012,3,4,12,9,9)" << false << QVariant( 0 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,9) > make_datetime(2012,3,4,12,9,8)" ) << "make_datetime(2012,3,4,12,9,9) > make_datetime(2012,3,4,12,9,8)" << false << QVariant( 1 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,9) < make_datetime(2012,3,4,12,9,8)" ) << "make_datetime(2012,3,4,12,9,9) < make_datetime(2012,3,4,12,9,8)" << false << QVariant( 0 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,8) = make_datetime(2012,3,4,12,9,8)" ) << "make_datetime(2012,3,4,12,9,8) = make_datetime(2012,3,4,12,9,8)" << false << QVariant( 1 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,8) = make_datetime(2012,3,4,12,9,9)" ) << "make_datetime(2012,3,4,12,9,8) = make_datetime(2012,3,4,12,9,9)" << false << QVariant( 0 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,8) != make_datetime(2012,3,4,12,9,9)" ) << "make_datetime(2012,3,4,12,9,8) != make_datetime(2012,3,4,12,9,9)" << false << QVariant( 1 );
      QTest::newRow( "make_datetime(2012,3,4,12,9,8) != make_datetime(2012,3,4,12,9,8)" ) << "make_datetime(2012,3,4,12,9,8) != make_datetime(2012,3,4,12,9,8)" << false << QVariant( 0 );

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

      // is, boolean values

      QTest::newRow( "is bool both true/true" ) << "TRUE IS TRUE" << false << QVariant( 1 );
      QTest::newRow( "is bool both true/false" ) << "TRUE IS FALSE" << false << QVariant( 0 );
      QTest::newRow( "is bool both false/true" ) << "FALSE IS TRUE" << false << QVariant( 0 );
      QTest::newRow( "is bool both false/false" ) << "FALSE IS FALSE" << false << QVariant( 1 );
      QTest::newRow( "is bool first int second true true/true" ) << "TRUE IS 1" << false << QVariant( 1 );
      QTest::newRow( "is bool first int second false true/false" ) << "TRUE IS 0" << false << QVariant( 0 );
      QTest::newRow( "is bool first int second true false/true" ) << "FALSE IS 1" << false << QVariant( 0 );
      QTest::newRow( "is bool first int second false false/false" ) << "FALSE IS 0" << false << QVariant( 1 );
      QTest::newRow( "is int first bool second true true/true" ) << "1 IS TRUE" << false << QVariant( 1 );
      QTest::newRow( "is int first bool second false true/false" ) << "1 IS FALSE" << false << QVariant( 0 );
      QTest::newRow( "is int first bool second true false/true" ) << "0 IS TRUE" << false << QVariant( 0 );
      QTest::newRow( "is int first bool second false false/false" ) << "0 IS FALSE" << false << QVariant( 1 );
      QTest::newRow( "is bool first string int second true true/true" ) << "TRUE IS '1'" << false << QVariant( 1 );
      QTest::newRow( "is bool first string int second false true/false" ) << "TRUE IS '0'" << false << QVariant( 0 );
      QTest::newRow( "is bool first string int second true false/true" ) << "FALSE IS '1'" << false << QVariant( 0 );
      QTest::newRow( "is bool first string int second false false/false" ) << "FALSE IS '0'" << false << QVariant( 1 );
      QTest::newRow( "is string int first bool second true true/true" ) << "'1' IS TRUE" << false << QVariant( 1 );
      QTest::newRow( "is string int first bool second false true/false" ) << "'1' IS FALSE" << false << QVariant( 0 );
      QTest::newRow( "is string int first bool second true false/true" ) << "'0' IS TRUE" << false << QVariant( 0 );
      QTest::newRow( "is string int first bool second false false/false" ) << "'0' IS FALSE" << false << QVariant( 1 );
      QTest::newRow( "is bool first string bool second true true/true" ) << "TRUE IS 'true'" << false << QVariant( 1 );
      QTest::newRow( "is bool first string bool second false true/false" ) << "TRUE IS 'false'" << false << QVariant( 0 );
      QTest::newRow( "is bool first string bool second true false/true" ) << "FALSE IS 'true'" << false << QVariant( 0 );
      QTest::newRow( "is bool first string bool second false false/false" ) << "FALSE IS 'false'" << false << QVariant( 1 );
      QTest::newRow( "is string bool first bool second true true/true" ) << "'true' IS TRUE" << false << QVariant( 1 );
      QTest::newRow( "is string bool first bool second false true/false" ) << "'true' IS FALSE" << false << QVariant( 0 );
      QTest::newRow( "is string bool first bool second true false/true" ) << "'false' IS TRUE" << false << QVariant( 0 );
      QTest::newRow( "is string bool first bool second false false/false" ) << "'false' IS FALSE" << false << QVariant( 1 );

      // is not, boolean values

      QTest::newRow( "is not bool both true/true" ) << "TRUE IS NOT TRUE" << false << QVariant( 0 );
      QTest::newRow( "is not bool both true/false" ) << "TRUE IS NOT FALSE" << false << QVariant( 1 );
      QTest::newRow( "is not bool both false/true" ) << "FALSE IS NOT TRUE" << false << QVariant( 1 );
      QTest::newRow( "is not bool both false/false" ) << "FALSE IS NOT FALSE" << false << QVariant( 0 );
      QTest::newRow( "is not bool first int second true true/true" ) << "TRUE IS NOT 1" << false << QVariant( 0 );
      QTest::newRow( "is not bool first int second false true/false" ) << "TRUE IS NOT 0" << false << QVariant( 1 );
      QTest::newRow( "is not bool first int second true false/true" ) << "FALSE IS NOT 1" << false << QVariant( 1 );
      QTest::newRow( "is not bool first int second false false/false" ) << "FALSE IS NOT 0" << false << QVariant( 0 );
      QTest::newRow( "is not int first bool second true true/true" ) << "1 IS NOT TRUE" << false << QVariant( 0 );
      QTest::newRow( "is not int first bool second false true/false" ) << "1 IS NOT FALSE" << false << QVariant( 1 );
      QTest::newRow( "is not int first bool second true false/true" ) << "0 IS NOT TRUE" << false << QVariant( 1 );
      QTest::newRow( "is not int first bool second false false/false" ) << "0 IS NOT FALSE" << false << QVariant( 0 );
      QTest::newRow( "is not bool first string int second true true/true" ) << "TRUE IS NOT '1'" << false << QVariant( 0 );
      QTest::newRow( "is not bool first string int second false true/false" ) << "TRUE IS NOT '0'" << false << QVariant( 1 );
      QTest::newRow( "is not bool first string int second true false/true" ) << "FALSE IS NOT '1'" << false << QVariant( 1 );
      QTest::newRow( "is not bool first string int second false false/false" ) << "FALSE IS NOT '0'" << false << QVariant( 0 );
      QTest::newRow( "is not string int first bool second true true/true" ) << "'1' IS NOT TRUE" << false << QVariant( 0 );
      QTest::newRow( "is not string int first bool second false true/false" ) << "'1' IS NOT FALSE" << false << QVariant( 1 );
      QTest::newRow( "is not string int first bool second true false/true" ) << "'0' IS NOT TRUE" << false << QVariant( 1 );
      QTest::newRow( "is not string int first bool second false false/false" ) << "'0' IS NOT FALSE" << false << QVariant( 0 );
      QTest::newRow( "is not bool first string bool second true true/true" ) << "TRUE IS NOT 'true'" << false << QVariant( 0 );
      QTest::newRow( "is not bool first string bool second false true/false" ) << "TRUE IS NOT 'false'" << false << QVariant( 1 );
      QTest::newRow( "is not bool first string bool second true false/true" ) << "FALSE IS NOT 'true'" << false << QVariant( 1 );
      QTest::newRow( "is not bool first string bool second false false/false" ) << "FALSE IS NOT 'false'" << false << QVariant( 0 );
      QTest::newRow( "is not string bool first bool second true true/true" ) << "'true' IS NOT TRUE" << false << QVariant( 0 );
      QTest::newRow( "is not string bool first bool second false true/false" ) << "'true' IS NOT FALSE" << false << QVariant( 1 );
      QTest::newRow( "is not string bool first bool second true false/true" ) << "'false' IS NOT TRUE" << false << QVariant( 1 );
      QTest::newRow( "is not string bool first bool second false false/false" ) << "'false' IS NOT FALSE" << false << QVariant( 0 );

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
      QTest::newRow( "in 5" ) << "'a' in (1,2,3)" << false << QVariant( 0 );
      QTest::newRow( "in 6" ) << "'a' in (1,'a',3)" << false << QVariant( 1 );
      QTest::newRow( "in 7" ) << "'b' in (1,'a',3)" << false << QVariant( 0 );
      QTest::newRow( "in 8" ) << "1.2 in (1,2,3)" << false << QVariant( 0 );
      QTest::newRow( "in 9" ) << "'010080383000187224' in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 0 );
      QTest::newRow( "in 10" ) << "'010080383000187219' in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 1 );
      QTest::newRow( "in 11" ) << "'010080383000187218' in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 1 );
      QTest::newRow( "in 12" ) << "'010080383000187223' in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 1 );
      QTest::newRow( "not in 1" ) << "1 not in (1,2,3)" << false << QVariant( 0 );
      QTest::newRow( "not in 2" ) << "1 not in (1,null,3)" << false << QVariant( 0 );
      QTest::newRow( "not in 3" ) << "1 not in (null,2,3)" << false << QVariant();
      QTest::newRow( "not in 4" ) << "null not in (1,2,3)" << false << QVariant();
      QTest::newRow( "not in 5" ) << "'a' not in (1,2,3)" << false << QVariant( 1 );
      QTest::newRow( "not in 6" ) << "'a' not in (1,'a',3)" << false << QVariant( 0 );
      QTest::newRow( "not in 7" ) << "'b' not in (1,'a',3)" << false << QVariant( 1 );
      QTest::newRow( "not in 8" ) << "1.2 not in (1,2,3)" << false << QVariant( 1 );
      QTest::newRow( "not in 9" ) << "'010080383000187224' not in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 1 );
      QTest::newRow( "not in 10" ) << "'010080383000187219' not in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 0 );
      QTest::newRow( "not in 11" ) << "'010080383000187218' not in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 0 );
      QTest::newRow( "not in 12" ) << "'010080383000187223' not in ('010080383000187219','010080383000187218','010080383000187223')" << false << QVariant( 0 );

      // regexp, like
      QTest::newRow( "like 1" ) << "'hello' like '%ll_'" << false << QVariant( 1 );
      QTest::newRow( "like 2" ) << "'hello' like '_el%'" << false << QVariant( 1 );
      QTest::newRow( "like 3" ) << "'hello' like 'lo'" << false << QVariant( 0 );
      QTest::newRow( "like 4" ) << "'hello' like '%LO'" << false << QVariant( 0 );
      QTest::newRow( "like 5" ) << "'QGIS' like '%G%'" << false << QVariant( 1 );
      QTest::newRow( "like 6" ) << "'[testing]' like '[testing%'" << false << QVariant( 1 );
      QTest::newRow( "like 7" ) << "'hello\nworld' like '%world%'" << false << QVariant( 1 );
      QTest::newRow( "ilike" ) << "'hello' ilike '%LO'" << false << QVariant( 1 );
      QTest::newRow( "ilike with dot" ) << "'QGIS .123' ilike 'qgis .123'" << false << QVariant( 1 );
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
      QTest::newRow( "round('1234.434', 1) - round string down to 1 decimal" ) << "round('1234.434', 1)" << false << QVariant( 1234.4 );
      QTest::newRow( "round('1234.434') - round string down to int" ) << "round('1234.434')" << false << QVariant( 1234 );
      QTest::newRow( "round('not number') - round not a number" ) << "round('not number')" << true << QVariant();
      QTest::newRow( "max(1)" ) << "max(1)" << false << QVariant( 1. );
      QTest::newRow( "max(1,3.5,-2.1)" ) << "max(1,3.5,-2.1)" << false << QVariant( 3.5 );
      QTest::newRow( "max(3.5,-2.1,1)" ) << "max(3.5,-2.1,1)" << false << QVariant( 3.5 );
      QTest::newRow( "max with null value" ) << "max(1,3.5,null)" << false << QVariant( 3.5 );
      QTest::newRow( "max with null value first" ) << "max(null,-3.5,2)" << false << QVariant( 2. );
      QTest::newRow( "max with no params" ) << "max()" << false << QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
      QTest::newRow( "max with only null value" ) << "max(null)" << false << QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
      QTest::newRow( "min(-1.5)" ) << "min(-1.5)" << false << QVariant( -1.5 );
      QTest::newRow( "min(-16.6,3.5,-2.1)" ) << "min(-16.6,3.5,-2.1)" << false << QVariant( -16.6 );
      QTest::newRow( "min(5,3.5,-2.1)" ) << "min(5,3.5,-2.1)" << false << QVariant( -2.1 );
      QTest::newRow( "min with null value" ) << "min(5,null,-2.1)" << false << QVariant( -2.1 );
      QTest::newRow( "min with null value first" ) << "min(null,3.2,6.5)" << false << QVariant( 3.2 );
      QTest::newRow( "min with no params" ) << "min()" << false << QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
      QTest::newRow( "min with only null value" ) << "min(null)" << false << QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
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
      QTest::newRow( "scale_linear(5,0,10,0,100)" ) << "scale_linear(5,0,10,0,100)" << false << QVariant( 50. );
      QTest::newRow( "scale_linear(0.2,0,1,0,360)" ) << "scale_linear(0.2,0,1,0,360)" << false << QVariant( 72. );
      QTest::newRow( "scale_linear(1500,1000,10000,9,20)" ) << "scale_linear(1500,1000,10000,9,20)" << false << QVariant( 9.61111111111111 );

      // previously had name scale_exp, but renamed to scale_polynomial as it uses polynomial interpolation formula
      // see https://github.com/qgis/QGIS/pull/53164 for more details
      QTest::newRow( "scale_polynomial(0.5,0,1,0,1,2)" ) << "scale_polynomial(0.5,0,1,0,1,2)" << false << QVariant( 0.25 );
      QTest::newRow( "scale_polynomial(0,0,10,100,200,2)" ) << "scale_polynomial(0,0,10,100,200,2)" << false << QVariant( 100. );
      QTest::newRow( "scale_polynomial(5,0,10,100,200,2)" ) << "scale_polynomial(5,0,10,100,200,2)" << false << QVariant( 125. );
      QTest::newRow( "scale_polynomial(10,0,10,100,200,0.5)" ) << "scale_polynomial(10,0,10,100,200,0.5)" << false << QVariant( 200. );
      QTest::newRow( "scale_polynomial(-1,0,10,100,200,0.5)" ) << "scale_polynomial(-1,0,10,100,200,0.5)" << false << QVariant( 100. );
      QTest::newRow( "scale_polynomial(4,0,9,0,90,0.5)" ) << "scale_polynomial(4,0,9,0,90,0.5)" << false << QVariant( 60. );
      QTest::newRow( "scale_polynomial(5,0,10,0,100,2)" ) << "scale_polynomial(5,0,10,0,100,2)" << false << QVariant( 25. );
      QTest::newRow( "scale_polynomial(3,0,10,0,100,0.5)" ) << "scale_polynomial(3,0,10,0,100,0.5)" << false << QVariant( 54.77225575051661 );
      // this is an alias for scale_polynomial to preserve backward compatibility
      QTest::newRow( "scale_exp(0.5,0,1,0,1,2)" ) << "scale_exp(0.5,0,1,0,1,2)" << false << QVariant( 0.25 );
      QTest::newRow( "scale_exp(0,0,10,100,200,2)" ) << "scale_exp(0,0,10,100,200,2)" << false << QVariant( 100. );
      QTest::newRow( "scale_exp(5,0,10,100,200,2)" ) << "scale_exp(5,0,10,100,200,2)" << false << QVariant( 125. );
      QTest::newRow( "scale_exp(10,0,10,100,200,0.5)" ) << "scale_exp(10,0,10,100,200,0.5)" << false << QVariant( 200. );
      QTest::newRow( "scale_exp(-1,0,10,100,200,0.5)" ) << "scale_exp(-1,0,10,100,200,0.5)" << false << QVariant( 100. );
      QTest::newRow( "scale_exp(4,0,9,0,90,0.5)" ) << "scale_exp(4,0,9,0,90,0.5)" << false << QVariant( 60. );

      QTest::newRow( "scale_exponential(0.5,0,1,0,1,2)" ) << "scale_exponential(0.5,0,1,0,1,2)" << false << QVariant( 0.414213562373 );
      QTest::newRow( "scale_exponential(0,0,10,100,200,2)" ) << "scale_exponential(0,0,10,100,200,2)" << false << QVariant( 100. );
      QTest::newRow( "scale_exponential(5,0,10,100,200,2)" ) << "scale_exponential(5,0,10,100,200,2)" << false << QVariant( 103.0303030303 );
      QTest::newRow( "scale_exponential(10,0,10,100,200,0.5)" ) << "scale_exponential(10,0,10,100,200,0.5)" << false << QVariant( 200. );
      QTest::newRow( "scale_exponential(-1,0,10,100,200,0.5)" ) << "scale_exponential(-1,0,10,100,200,0.5)" << false << QVariant( 100. );
      QTest::newRow( "scale_exponential(4,0,9,0,90,0.5)" ) << "scale_exponential(4,0,9,0,90,0.5)" << false << QVariant( 84.5401174168 );
      QTest::newRow( "scale_exponential(5,0,10,0,100,2)" ) << "scale_exponential(5,0,10,0,100,2)" << false << QVariant( 3.0303030303030303 );
      QTest::newRow( "scale_exponential(3,0,10,0,100,0.5)" ) << "scale_exponential(3,0,10,0,100,0.5)" << false << QVariant( 87.58553274682306 );

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
      QTest::newRow( "degree minute second string to decimal" ) << "to_decimal('6°21′16.45″N')" << false << QVariant( 6.35456944444 );
      QTest::newRow( "wrong degree minute second string to decimal" ) << "to_decimal('qgis')" << false << QVariant();

      QTest::newRow( "extract degrees positive int" ) << "extract_degrees(135)" << false << QVariant( 135 );
      QTest::newRow( "extract degrees negative int" ) << "extract_degrees(-135)" << false << QVariant( -135 );
      // make sure these aren't rounding, but should instead truncate towards zero
      QTest::newRow( "extract degrees positive float low" ) << "extract_degrees(135.112341)" << false << QVariant( 135 );
      QTest::newRow( "extract degrees negative float low" ) << "extract_degrees(-135.11234)" << false << QVariant( -135 );
      QTest::newRow( "extract degrees positive float high" ) << "extract_degrees(135.78785)" << false << QVariant( 135 );
      QTest::newRow( "extract degrees negative float high" ) << "extract_degrees(-135.7878)" << false << QVariant( -135 );

      QTest::newRow( "extract minutes positive int" ) << "extract_minutes(135)" << false << QVariant( 0 );
      QTest::newRow( "extract minutes negative int" ) << "extract_minutes(-135)" << false << QVariant( 0 );
      QTest::newRow( "extract minutes positive round minutes" ) << "extract_minutes(135 + 45/60)" << false << QVariant( 45 );
      QTest::newRow( "extract minutes negative round minutes" ) << "extract_minutes(-135 - 45/60)" << false << QVariant( 45 );
      QTest::newRow( "extract minutes positive with low seconds" ) << "extract_minutes(135 + 45.123/60)" << false << QVariant( 45 );
      QTest::newRow( "extract minutes negative with low seconds" ) << "extract_minutes(-135 - 45.123/60)" << false << QVariant( 45 );
      QTest::newRow( "extract minutes positive with high seconds" ) << "extract_minutes(135 + 45.888/60)" << false << QVariant( 45 );
      QTest::newRow( "extract minutes negative with high seconds" ) << "extract_minutes(-135 - 45.888/60)" << false << QVariant( 45 );

      QTest::newRow( "extract seconds positive int" ) << "extract_seconds(135)" << false << QVariant( 0.0 );
      QTest::newRow( "extract seconds negative int" ) << "extract_seconds(-135)" << false << QVariant( 0.0 );
      QTest::newRow( "extract seconds positive round minutes" ) << "extract_seconds(135 + 45/60)" << false << QVariant( 0.0 );
      QTest::newRow( "extract seconds negative round minutes" ) << "extract_seconds(-135 - 45/60)" << false << QVariant( 0.0 );
      QTest::newRow( "extract seconds positive with round low seconds" ) << "extract_seconds(135 + 45/60 + 12/3600)" << false << QVariant( 12.0 );
      QTest::newRow( "extract seconds negative with round low seconds" ) << "extract_seconds(-135 - 45/60 - 12/3600)" << false << QVariant( 12.0 );
      QTest::newRow( "extract seconds positive with round high seconds" ) << "extract_seconds(135 + 45/60 + 58/3600)" << false << QVariant( 58.0 );
      QTest::newRow( "extract seconds negative with round high seconds" ) << "extract_seconds(-135 - 45/60 - 58/3600)" << false << QVariant( 58.0 );
      QTest::newRow( "extract seconds positive with decimal low seconds" ) << "round(extract_seconds(135 + 45/60 + 12.222/3600), 3)" << false << QVariant( 12.222 );
      QTest::newRow( "extract seconds negative with decimal low seconds" ) << "round(extract_seconds(-135 - 45/60 - 12.222/3600), 3)" << false << QVariant( 12.222 );
      QTest::newRow( "extract seconds positive with decimal high seconds" ) << "round(extract_seconds(135 + 45/60 + 58.222/3600), 3)" << false << QVariant( 58.222 );
      QTest::newRow( "extract seconds negative with decimal high seconds" ) << "round(extract_seconds(-135 - 45/60 - 58.222/3600), 3)" << false << QVariant( 58.222 );

      // geometry functions
      QTest::newRow( "geom_to_wkb" ) << "geom_to_wkt(geom_from_wkb(geom_to_wkb(make_point(4,5))))" << false << QVariant( "Point (4 5)" );
      QTest::newRow( "geom_to_wkb not geom" ) << "geom_to_wkt(geom_from_wkb(geom_to_wkb('a')))" << true << QVariant();
      QTest::newRow( "geom_from_wkb not geom" ) << "geom_to_wkt(geom_from_wkb(make_point(4,5)))" << true << QVariant();
      QTest::newRow( "geom_from_wkb null" ) << "geom_to_wkt(geom_from_wkb(NULL))" << false << QVariant();
      QTest::newRow( "geometry_type not geom" ) << "geometry_type('g')" << true << QVariant();
      QTest::newRow( "geometry_type null" ) << "geometry_type(NULL)" << false << QVariant();
      QTest::newRow( "geometry_type point" ) << "geometry_type(geom_from_wkt('POINT(1 2)'))" << false << QVariant( u"Point"_s );
      QTest::newRow( "geometry_type polygon" ) << "geometry_type(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( u"Polygon"_s );
      QTest::newRow( "geometry_type line" ) << "geometry_type(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant( u"Line"_s );
      QTest::newRow( "geometry_type multipoint" ) << "geometry_type(geom_from_wkt('MULTIPOINT((0 1),(0 0))'))" << false << QVariant( u"Point"_s );
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
      QTest::newRow( "nodes_to_points point" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POINT(1 2)')))" << false << QVariant( u"MultiPoint ((1 2))"_s );
      QTest::newRow( "nodes_to_points polygon" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( u"MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-1 -1))"_s );
      QTest::newRow( "nodes_to_points polygon with rings" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))')))" << false
                                                            << QVariant( u"MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-1 -1),(-0.1 -0.1),(0.4 0),(0.4 0.2),(0 0.2),(-0.1 -0.1),(-0.3 -0.9),(-0.3 0),(4 -0.1),(0.1 2.1),(-0.3 -0.9))"_s );
      QTest::newRow( "nodes_to_points line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false
                                              << QVariant( u"MultiPoint ((0 0),(1 1),(2 2))"_s );
      QTest::newRow( "nodes_to_points collection 1" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))')))" << false
                                                      << QVariant( u"MultiPoint ((0 1),(0 0),(1 0),(1 1))"_s );
      QTest::newRow( "nodes_to_points collection 2" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION(POINTZM(0 1 2 3), POINTZM(0 0 3 4), POINTZM(1 1 5 6), POLYGONZM((-1 -1 7 8, 4 0 1 2, 4 2 7 6, 0 2 1 3, -1 -1 7 8),(-0.1 -0.1 5 4, 0.4 0 9 8, 0.4 0.2 7 10, 0 0.2 0 0, -0.1 -0.1 5 4),(-1 -1 0 0, 4 0 0 1, 4 2 1 2, 0 2 2 3, -1 -1 0 0)), POINTZM(1 0 1 2))')))" << false
                                                      << QVariant( u"MultiPoint ZM ((0 1 2 3),(0 0 3 4),(1 1 5 6),(-1 -1 7 8),(4 0 1 2),(4 2 7 6),(0 2 1 3),(-1 -1 7 8),(-0.1 -0.1 5 4),(0.4 0 9 8),(0.4 0.2 7 10),(0 0.2 0 0),(-0.1 -0.1 5 4),(-1 -1 0 0),(4 0 0 1),(4 2 1 2),(0 2 2 3),(-1 -1 0 0),(1 0 1 2))"_s );
      QTest::newRow( "nodes_to_points empty collection" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('GEOMETRYCOLLECTION()')))" << false << QVariant( u"MultiPoint EMPTY"_s );
      QTest::newRow( "nodes_to_points no close polygon" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'),true))" << false << QVariant( u"MultiPoint ((-1 -1),(4 0),(4 2),(0 2))"_s );
      QTest::newRow( "nodes_to_points no close polygon with rings" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))'),true))" << false
                                                                     << QVariant( u"MultiPoint ((-1 -1),(4 0),(4 2),(0 2),(-0.1 -0.1),(0.4 0),(0.4 0.2),(0 0.2),(-0.3 -0.9),(-0.3 0),(4 -0.1),(0.1 2.1))"_s );
      QTest::newRow( "nodes_to_points no close unclosed line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'),true))" << false
                                                                << QVariant( u"MultiPoint ((0 0),(1 1),(2 2))"_s );
      QTest::newRow( "nodes_to_points no close closed line" ) << "geom_to_wkt(nodes_to_points(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2, 0 0)'),true))" << false
                                                              << QVariant( u"MultiPoint ((0 0),(1 1),(2 2))"_s );
      QTest::newRow( "segments_to_lines not geom" ) << "segments_to_lines('g')" << true << QVariant();
      QTest::newRow( "segments_to_lines null" ) << "segments_to_lines(NULL)" << false << QVariant();
      QTest::newRow( "segments_to_lines point" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POINT(1 2)')))" << false << QVariant( u"MultiLineString EMPTY"_s );
      QTest::newRow( "segments_to_lines polygon" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( u"MultiLineString ((-1 -1, 4 0),(4 0, 4 2),(4 2, 0 2),(0 2, -1 -1))"_s );
      QTest::newRow( "segments_to_lines polygon with rings" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0, 4 -0.1, 0.1 2.1, -0.3 -0.9))')))" << false
                                                              << QVariant( u"MultiLineString ((-1 -1, 4 0),(4 0, 4 2),(4 2, 0 2),(0 2, -1 -1),(-0.1 -0.1, 0.4 0),(0.4 0, 0.4 0.2),(0.4 0.2, 0 0.2),(0 0.2, -0.1 -0.1),(-0.3 -0.9, -0.3 0),(-0.3 0, 4 -0.1),(4 -0.1, 0.1 2.1),(0.1 2.1, -0.3 -0.9))"_s );
      QTest::newRow( "segments_to_lines line" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false
                                                << QVariant( u"MultiLineString ((0 0, 1 1),(1 1, 2 2))"_s );
      QTest::newRow( "segments_to_lines collection 1" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))')))" << false
                                                        << QVariant( u"MultiLineString EMPTY"_s );
      QTest::newRow( "segments_to_lines collection 2" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION(POINTZM(0 1 2 3), LINESTRINGZM(0 0 1 2, 1 1 3 4, 2 2 5 6), POINTZM(1 1 5 6), POLYGONZM((-1 -1 7 8, 4 0 1 2, 4 2 7 6, 0 2 1 3, -1 -1 7 8)), POINTZM(1 0 1 2))')))" << false
                                                        << QVariant( u"MultiLineString ZM ((0 0 1 2, 1 1 3 4),(1 1 3 4, 2 2 5 6),(-1 -1 7 8, 4 0 1 2),(4 0 1 2, 4 2 7 6),(4 2 7 6, 0 2 1 3),(0 2 1 3, -1 -1 7 8))"_s );
      QTest::newRow( "segments_to_lines empty collection" ) << "geom_to_wkt(segments_to_lines(geom_from_wkt('GEOMETRYCOLLECTION()')))" << false << QVariant( u"MultiLineString EMPTY"_s );
      QTest::newRow( "length line" ) << "length(geom_from_wkt('LINESTRING(0 0, 4 0)'))" << false << QVariant( 4.0 );
      QTest::newRow( "length polygon" ) << "length(geom_from_wkt('POLYGON((0 0, 4 0, 4 2, 0 2, 0 0))'))" << false << QVariant();
      QTest::newRow( "length point" ) << "length(geom_from_wkt('POINT(0 0)'))" << false << QVariant();
      QTest::newRow( "length empty line" ) << "length(geom_from_wkt('LINESTRING EMPTY'))" << false << QVariant( 0.0 );
      QTest::newRow( "length3D lineZ" ) << "length3D(geom_from_wkt('LINESTRINGZ(0 0 0, 3 0 4)'))" << false << QVariant( 5.0 );
      QTest::newRow( "length3D line" ) << "length3D(geom_from_wkt('LINESTRING(0 0, 4 0)'))" << false << QVariant( 4.0 );
      QTest::newRow( "length3D polygon" ) << "length3D(geom_from_wkt('POLYGON((0 0, 4 0, 4 2, 0 2, 0 0))'))" << false << QVariant();
      QTest::newRow( "length3D point" ) << "length3D(geom_from_wkt('POINT(0 0)'))" << false << QVariant();
      QTest::newRow( "length3D empty linez" ) << "length3D(geom_from_wkt('LINESTRINGZ EMPTY'))" << false << QVariant( 0.0 );
      QTest::newRow( "length3D empty line" ) << "length3D(geom_from_wkt('LINESTRING EMPTY'))" << false << QVariant( 0.0 );
      QTest::newRow( "length3D multiline" ) << "length3D(geom_from_wkt('MULTILINESTRINGZ((0 0 0, 3 0 4),(10 0 0, 13 0 4))'))" << false << QVariant( 10.0 );
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
      QTest::newRow( "point_n multipoint negative" ) << "geom_to_wkt(point_n(geom_from_wkt('MULTIPOINT((0 0), (1 1), (2 2))'),-1))" << false << QVariant( "Point (2 2)" );
      QTest::newRow( "point_n line" ) << "geom_to_wkt(point_n(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'),3))" << false << QVariant( "Point (2 2)" );
      QTest::newRow( "point_n polygon" ) << "geom_to_wkt(point_n(geom_from_wkt('POLYGON((0 0, 4 0, 4 2, 0 2, 0 0))'),3))" << false << QVariant( "Point (4 2)" );
      QTest::newRow( "interior_ring_n not geom" ) << "interior_ring_n('g', 1)" << true << QVariant();
      QTest::newRow( "interior_ring_n null" ) << "interior_ring_n(NULL, 1)" << false << QVariant();
      QTest::newRow( "interior_ring_n point" ) << "interior_ring_n(geom_from_wkt('POINT(1 2)'), 1)" << false << QVariant();
      QTest::newRow( "interior_ring_n polygon no rings" ) << "interior_ring_n(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'),1)" << false << QVariant();
      QTest::newRow( "interior_ring_n polygon with rings" ) << "geom_to_wkt(interior_ring_n(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1),(-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1),(-1 -1, 4 0, 4 2, 0 2, -1 -1))'),1))" << false
                                                            << QVariant( u"LineString (-0.1 -0.1, 0.4 0, 0.4 0.2, 0 0.2, -0.1 -0.1)"_s );
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
      QTest::newRow( "geometry_n collection" ) << "geom_to_wkt(geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),3))" << false << QVariant( u"Point (1 0)"_s );
      QTest::newRow( "geometry_n collection bad index 1" ) << "geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),0)" << false << QVariant();
      QTest::newRow( "geometry_n collection bad index 2" ) << "geometry_n(geom_from_wkt('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))'),5)" << false << QVariant();
      QTest::newRow( "force_rhr not geom" ) << "force_rhr('g')" << true << QVariant();
      QTest::newRow( "force_rhr null" ) << "force_rhr(NULL)" << false << QVariant();
      QTest::newRow( "force_rhr point" ) << "geom_to_wkt(force_rhr(geom_from_wkt('POINT(1 2)')))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "force_rhr polygon" ) << "geom_to_wkt(force_rhr(geometry:=geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( "Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1))" );
      QTest::newRow( "force_rhr multipolygon" ) << "geom_to_wkt(force_rhr(geometry:=geom_from_wkt('MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))')))" << false << QVariant( "MultiPolygon (((-1 -1, 0 2, 4 2, 4 0, -1 -1)),((100 100, 100 200, 200 200, 200 100, 100 100)))" );
      QTest::newRow( "force_rhr line" ) << "geom_to_wkt(force_rhr(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false << QVariant( "LineString (0 0, 1 1, 2 2)" );
      QTest::newRow( "force_polygon_ccw not geom" ) << "force_polygon_ccw('g')" << true << QVariant();
      QTest::newRow( "force_polygon_ccw null" ) << "force_polygon_ccw(NULL)" << false << QVariant();
      QTest::newRow( "force_polygon_ccw point" ) << "geom_to_wkt(force_polygon_ccw(geom_from_wkt('POINT(1 2)')))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "force_polygon_ccw polygon" ) << "geom_to_wkt(force_polygon_ccw(geometry:=geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( "Polygon ((-1 -1, 4 0, 4 2, 0 2, -1 -1))" );
      QTest::newRow( "force_polygon_ccw multipolygon" ) << "geom_to_wkt(force_polygon_ccw(geometry:=geom_from_wkt('MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))')))" << false << QVariant( "MultiPolygon (((-1 -1, 4 0, 4 2, 0 2, -1 -1)),((100 100, 200 100, 200 200, 100 200, 100 100)))" );
      QTest::newRow( "force_polygon_ccw line" ) << "geom_to_wkt(force_polygon_ccw(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false << QVariant( "LineString (0 0, 1 1, 2 2)" );
      QTest::newRow( "force_polygon_cw not geom" ) << "force_polygon_cw('g')" << true << QVariant();
      QTest::newRow( "force_polygon_cw null" ) << "force_polygon_cw(NULL)" << false << QVariant();
      QTest::newRow( "force_polygon_cw point" ) << "geom_to_wkt(force_polygon_cw(geom_from_wkt('POINT(1 2)')))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "force_polygon_cw polygon" ) << "geom_to_wkt(force_polygon_cw(geometry:=geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))')))" << false << QVariant( "Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1))" );
      QTest::newRow( "force_polygon_cw multipolygon" ) << "geom_to_wkt(force_polygon_cw(geometry:=geom_from_wkt('MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))')))" << false << QVariant( "MultiPolygon (((-1 -1, 0 2, 4 2, 4 0, -1 -1)),((100 100, 100 200, 200 200, 200 100, 100 100)))" );
      QTest::newRow( "force_polygon_cw line" ) << "geom_to_wkt(force_polygon_cw(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)')))" << false << QVariant( "LineString (0 0, 1 1, 2 2)" );
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
      QTest::newRow( "shared_paths not geom 1" ) << "shared_paths('g', geom_from_wkt('LineString(0 0, 10 10)'))" << true << QVariant();
      QTest::newRow( "shared_paths not geom 2" ) << "shared_paths(geom_from_wkt('LineString(0 0, 10 10)'), 'g')" << true << QVariant();
      QTest::newRow( "shared_paths null 1" ) << "shared_paths(NULL, geom_from_wkt('LineString(0 0, 10 10)'))" << false << QVariant();
      QTest::newRow( "shared_paths null 2" ) << "shared_paths(geom_from_wkt('LineString(0 0, 10 10)'), NULL)" << false << QVariant();
      QTest::newRow( "shared_paths point 1" ) << "shared_paths(make_point(1,2), geom_from_wkt('LineString(0 0, 10 10)'))" << false << QVariant();
      QTest::newRow( "shared_paths point 2" ) << "shared_paths(geom_from_wkt('LineString(0 0, 10 10)'), make_point(1,2))" << false << QVariant();
      QTest::newRow( "shared_paths lines 1" ) << "geom_to_wkt(shared_paths(geometry1:=geom_from_wkt('MULTILINESTRING((26 125,26 200,126 200,126 125,26 125),(51 150,101 150,76 175,51 150))'), geometry2:=geom_from_wkt('LINESTRING(151 100,126 156.25,126 125,90 161, 76 175)')))" << false << QVariant( "GeometryCollection (MultiLineString ((126 156.25, 126 125),(101 150, 90 161),(90 161, 76 175)),MultiLineString EMPTY)" );
      QTest::newRow( "shared_paths lines 2" ) << "geom_to_wkt(shared_paths(geometry1:=geom_from_wkt('MULTILINESTRING((26 125,26 200,126 200,126 125,26 125),(51 150,101 150,76 175,51 150))'), geometry2:=reverse(geom_from_wkt('LINESTRING(151 100,126 156.25,126 125,90 161, 76 175)'))))" << false << QVariant( "GeometryCollection (MultiLineString EMPTY,MultiLineString ((126 156.25, 126 125),(101 150, 90 161),(90 161, 76 175)))" );
      QTest::newRow( "offset_curve not geom" ) << "offset_curve('g', 5)" << true << QVariant();
      QTest::newRow( "offset_curve null" ) << "offset_curve(NULL, 5)" << false << QVariant();
      QTest::newRow( "offset_curve point" ) << "offset_curve(geom_from_wkt('POINT(1 2)'),5)" << false << QVariant();
      QTest::newRow( "offset_curve line" ) << "geom_to_wkt(offset_curve(geom_from_wkt('LineString(0 0, 10 0)'),1,segments:=4))" << false << QVariant( "LineString (10 1, 0 1)" );
      QTest::newRow( "offset_curve line miter" ) << "geom_to_wkt(offset_curve(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),distance:=-1,join:=2,miter_limit:=1))" << false << QVariant( "LineString (0 -1, 10 -1)" );
      QTest::newRow( "offset_curve line bevel" ) << "geom_to_wkt(offset_curve(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),distance:=1,join:=3))" << false << QVariant( "LineString (0 1, 9 1, 9 10)" );
      QTest::newRow( "wedge_buffer not geom" ) << "wedge_buffer('g', 0, 45, 1)" << true << QVariant();
      QTest::newRow( "wedge_buffer null" ) << "wedge_buffer(NULL, 0, 45, 1)" << false << QVariant();
      QTest::newRow( "wedge_buffer point" ) << "geom_to_wkt(wedge_buffer(center:=geom_from_wkt('POINT(1 2)'),azimuth:=90,width:=180,outer_radius:=1))" << false << QVariant( u"CurvePolygon (CompoundCurve (CircularString (1 3, 2 2, 1 1),(1 1, 1 2),(1 2, 1 3)))"_s );
      QTest::newRow( "wedge_buffer point inner" ) << "geom_to_wkt(wedge_buffer(center:=geom_from_wkt('POINT(1 2)'),azimuth:=90,width:=180,outer_radius:=2,inner_radius:=1))" << false << QVariant( u"CurvePolygon (CompoundCurve (CircularString (1 4, 3 2, 1 0),(1 0, 1 1),CircularString (1 1, 0 2, 1 3),(1 3, 1 4)))"_s );
      QTest::newRow( "tapered_buffer not geom" ) << "tapered_buffer('g', 1, 2, 8)" << true << QVariant();
      QTest::newRow( "tapered_buffer null" ) << "tapered_buffer(NULL, 1, 2, 8)" << false << QVariant();
      QTest::newRow( "tapered_buffer point" ) << "geom_to_wkt(tapered_buffer(geometry:=geom_from_wkt('POINT(1 2)'),start_width:=1,end_width:=2,segments:=10))" << true << QVariant();
      QTest::newRow( "tapered_buffer line" ) << "geom_to_wkt(tapered_buffer(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),start_width:=1,end_width:=2,segments:=3))" << false << QVariant( u"MultiPolygon (((-0.25 -0.4330127, -0.4330127 -0.25, -0.5 0, -0.4330127 0.25, -0.25 0.4330127, 0 0.5, 10 1, 10.5 0.8660254, 10.8660254 0.5, 11 0, 10.8660254 -0.5, 10.5 -0.8660254, 10 -1, 0 -0.5, -0.25 -0.4330127)))"_s );
      QTest::newRow( "tapered_buffer line 2" ) << "geom_to_wkt(tapered_buffer(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),start_width:=2,end_width:=1,segments:=3))" << false << QVariant( u"MultiPolygon (((-0.5 -0.8660254, -0.8660254 -0.5, -1 0, -0.8660254 0.5, -0.5 0.8660254, 0 1, 10 0.5, 10.25 0.4330127, 10.4330127 0.25, 10.5 0, 10.4330127 -0.25, 10.25 -0.4330127, 10 -0.5, 0 -1, -0.5 -0.8660254)))"_s );
      QTest::newRow( "buffer_by_m not geom" ) << "buffer_by_m('g', 8)" << true << QVariant();
      QTest::newRow( "buffer_by_m null" ) << "buffer_by_m(NULL, 8)" << false << QVariant();
      QTest::newRow( "buffer_by_m point" ) << "geom_to_wkt(buffer_by_m(geometry:=geom_from_wkt('POINT(1 2)'),segments:=10))" << true << QVariant();
      QTest::newRow( "buffer_by_m line" ) << "geom_to_wkt(buffer_by_m(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),segments:=3))" << false << QVariant( u"GeometryCollection EMPTY"_s );
      QTest::newRow( "buffer_by_m linem" ) << "geom_to_wkt(buffer_by_m(geometry:=geom_from_wkt('LineStringM(0 0 1, 10 0 2)'),segments:=3))" << false << QVariant( u"MultiPolygon (((-0.25 -0.4330127, -0.4330127 -0.25, -0.5 0, -0.4330127 0.25, -0.25 0.4330127, 0 0.5, 10 1, 10.5 0.8660254, 10.8660254 0.5, 11 0, 10.8660254 -0.5, 10.5 -0.8660254, 10 -1, 0 -0.5, -0.25 -0.4330127)))"_s );
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
      QTest::newRow( "centroid named argument geom" ) << "geom_to_wkt(centroid( geom:=geomFromWKT('POLYGON((0 0,0 9,9 0,0 0))')))" << false << QVariant( "Point (3 3)" );
      QTest::newRow( "centroid named argument geometry" ) << "geom_to_wkt(centroid( geometry:=geomFromWKT('POLYGON((0 0,0 9,9 0,0 0))')))" << false << QVariant( "Point (3 3)" );
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
      QTest::newRow( "is_valid not geom" ) << "is_valid('g')" << true << QVariant();
      QTest::newRow( "is_valid null" ) << "is_valid(NULL)" << false << QVariant();
      QTest::newRow( "is_valid point" ) << "is_valid(geom_from_wkt('POINT(1 2)'))" << false << QVariant( true );
      QTest::newRow( "is_valid valid" ) << "is_valid(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2, 0 0)'))" << false << QVariant( true );
      QTest::newRow( "is_valid not valid" ) << "is_valid(geom_from_wkt('LINESTRING(0 0)'))" << false << QVariant( false );
      QTest::newRow( "is_closed not geom" ) << "is_closed('g')" << true << QVariant();
      QTest::newRow( "is_closed null" ) << "is_closed(NULL)" << false << QVariant();
      QTest::newRow( "is_closed point" ) << "is_closed(geom_from_wkt('POINT(1 2)'))" << false << QVariant();
      QTest::newRow( "is_closed polygon" ) << "is_closed(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant();
      QTest::newRow( "is_closed not closed" ) << "is_closed(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2)'))" << false << QVariant( false );
      QTest::newRow( "is_closed closed" ) << "is_closed(geom_from_wkt('LINESTRING(0 0, 1 1, 2 2, 0 0)'))" << false << QVariant( true );
      QTest::newRow( "is_closed multiline" ) << "is_closed(geom_from_wkt('MultiLineString ((6501338.13976828 4850981.51459331, 6501343.09036573 4850984.01453377, 6501338.13976828 4850988.96491092, 6501335.63971657 4850984.01453377, 6501338.13976828 4850981.51459331))'))" << false << QVariant( true );
      QTest::newRow( "is_closed multiline" ) << "is_closed(geom_from_wkt('MultiLineString ((6501338.13976828 4850981.51459331, 6501343.09036573 4850984.01453377, 6501338.13976828 4850988.96491092, 6501335.63971657 4850984.01453377, 6501438.13976828 4850981.51459331))'))" << false << QVariant( false );
      QTest::newRow( "is_closed multiline" ) << "is_closed(geom_from_wkt('MultiLineString EMPTY'))" << false << QVariant();
      QTest::newRow( "close_line not geom" ) << "close_line('g')" << true << QVariant();
      QTest::newRow( "close_line null" ) << "close_line(NULL)" << false << QVariant();
      QTest::newRow( "close_line point" ) << "close_line(geom_from_wkt('POINT(0 0)'))" << false << QVariant();
      QTest::newRow( "close_line polygon" ) << "close_line(geom_from_wkt('POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))'))" << false << QVariant();
      QTest::newRow( "close_line not closed" ) << "geom_to_wkt(close_line(geom_from_wkt('LINESTRING(0 0, 1 0, 1 1)')))" << false << QVariant( "LineString (0 0, 1 0, 1 1, 0 0)" );
      QTest::newRow( "close_line closed" ) << "geom_to_wkt(close_line(geom_from_wkt('LINESTRING(0 0, 1 0, 1 1, 0 0)')))" << false << QVariant( "LineString (0 0, 1 0, 1 1, 0 0)" );
      QTest::newRow( "close_line multiline" ) << "geom_to_wkt(close_line(geom_from_wkt('MULTILINESTRING ((0 0, 1 1, 1 0),(2 2, 3 3, 3 2))')))" << false << QVariant( "MultiLineString ((0 0, 1 1, 1 0, 0 0),(2 2, 3 3, 3 2, 2 2))" );
      QTest::newRow( "is_empty not geom" ) << "is_empty('g')" << true << QVariant();
      QTest::newRow( "is_empty null" ) << "is_empty(NULL)" << false << QVariant();
      QTest::newRow( "is_empty point" ) << "is_empty(geom_from_wkt('POINT(1 2)'))" << false << QVariant( false );
      QTest::newRow( "is_empty empty point" ) << "is_empty(geom_from_wkt('POINT EMPTY'))" << false << QVariant( true );
      QTest::newRow( "is_empty polygon" ) << "is_empty(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( false );
      QTest::newRow( "is_empty empty polygon" ) << "is_empty(geom_from_wkt('POLYGON EMPTY'))" << false << QVariant( true );
      QTest::newRow( "is_empty_or_null not geom" ) << "is_empty_or_null('g')" << true << QVariant();
      QTest::newRow( "is_empty_or_null null" ) << "is_empty_or_null(NULL)" << false << QVariant( true );
      QTest::newRow( "is_empty_or_null point" ) << "is_empty_or_null(geom_from_wkt('POINT(1 2)'))" << false << QVariant( false );
      QTest::newRow( "is_empty_or_null empty point" ) << "is_empty_or_null(geom_from_wkt('POINT EMPTY'))" << false << QVariant( true );
      QTest::newRow( "is_empty_or_null polygon" ) << "is_empty_or_null(geom_from_wkt('POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))'))" << false << QVariant( false );
      QTest::newRow( "is_empty_or_null empty polygon" ) << "is_empty_or_null(geom_from_wkt('POLYGON EMPTY'))" << false << QVariant( true );
      QTest::newRow( "collect_geometries none" ) << "geom_to_wkt(collect_geometries())" << false << QVariant( "" );
      QTest::newRow( "collect_geometries not" ) << "geom_to_wkt(collect_geometries(45))" << true << QVariant();
      QTest::newRow( "collect_geometries one" ) << "geom_to_wkt(collect_geometries(make_point(4,5)))" << false << QVariant( "MultiPoint ((4 5))" );
      QTest::newRow( "collect_geometries two" ) << "geom_to_wkt(collect_geometries(make_point(4,5), make_point(6,7)))" << false << QVariant( "MultiPoint ((4 5),(6 7))" );
      QTest::newRow( "collect_geometries mixed" ) << "geom_to_wkt(collect_geometries(make_point(4,5), 'x'))" << true << QVariant();
      QTest::newRow( "collect_geometries array empty" ) << "geom_to_wkt(collect_geometries(array()))" << false << QVariant( "" );
      QTest::newRow( "collect_geometries array one" ) << "geom_to_wkt(collect_geometries(array(make_point(4,5))))" << false << QVariant( "MultiPoint ((4 5))" );
      QTest::newRow( "collect_geometries array two" ) << "geom_to_wkt(collect_geometries(array(make_point(4,5), make_point(6,7))))" << false << QVariant( "MultiPoint ((4 5),(6 7))" );
      QTest::newRow( "make_point" ) << "geom_to_wkt(make_point(2.2,4.4))" << false << QVariant( "Point (2.2 4.4)" );
      QTest::newRow( "make_point z" ) << "geom_to_wkt(make_point(2.2,4.4,5.5))" << false << QVariant( "Point Z (2.2 4.4 5.5)" );
      QTest::newRow( "make_point zm" ) << "geom_to_wkt(make_point(2.2,4.4,5.5,6.6))" << false << QVariant( "Point ZM (2.2 4.4 5.5 6.6)" );
      QTest::newRow( "make_point bad" ) << "make_point(2.2)" << true << QVariant();
      QTest::newRow( "make_point bad 2" ) << "make_point(2.2, 3, 3, 3, 3)" << true << QVariant();
      QTest::newRow( "make_point_m" ) << "geom_to_wkt(make_point_m(2.2,4.4,5.5))" << false << QVariant( "Point M (2.2 4.4 5.5)" );
      QTest::newRow( "make_line bad" ) << "make_line(make_point(2,4))" << false << QVariant();
      QTest::newRow( "make_line" ) << "geom_to_wkt(make_line(make_point(2,4),make_point(4,6)))" << false << QVariant( "LineString (2 4, 4 6)" );
      QTest::newRow( "make_line" ) << "geom_to_wkt(make_line(make_point(2,4),make_point(4,6),make_point(7,9)))" << false << QVariant( "LineString (2 4, 4 6, 7 9)" );
      QTest::newRow( "make_line" ) << "geom_to_wkt(make_line(make_point(2,4,1,3),make_point(4,6,9,8),make_point(7,9,3,4)))" << false << QVariant( "LineString ZM (2 4 1 3, 4 6 9 8, 7 9 3 4)" );
      QTest::newRow( "make_line array" ) << "geom_to_wkt(make_line(array(make_point(2,4),make_point(4,6))))" << false << QVariant( "LineString (2 4, 4 6)" );
      QTest::newRow( "make_line one" ) << "geom_to_wkt(make_line(array(make_point(2,4))))" << false << QVariant();
      QTest::newRow( "make_line array mixed" ) << "geom_to_wkt(make_line(array(make_point(2,4),make_point(4,6)),make_point(8,9)))" << false << QVariant( "LineString (2 4, 4 6, 8 9)" );
      QTest::newRow( "make_line array bad" ) << "geom_to_wkt(make_line(array(make_point(2,4),66)))" << true << QVariant();
      QTest::newRow( "make_line array empty" ) << "geom_to_wkt(make_line(array()))" << false << QVariant();
      QTest::newRow( "make_polygon bad" ) << "make_polygon(make_point(2,4))" << false << QVariant();
      QTest::newRow( "make_polygon" ) << "geom_to_wkt(make_polygon(geom_from_wkt('LINESTRING( 0 0, 0 1, 1 1, 1 0, 0 0 )')))" << false << QVariant( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))" );
      QTest::newRow( "make_polygon already polygon" ) << "geom_to_wkt(make_polygon(geom_from_wkt('POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0 ))')))" << false << QVariant( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))" );
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
#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 10
      QTest::newRow( "make_valid_extravert" ) << "geom_to_wkt(make_valid(geom_from_wkt('POLYGON((3 2, 4 1, 5 8, 3 2, 4 2))')))" << false << QVariant( "GeometryCollection (Polygon ((5 8, 4 1, 3 2, 5 8)),LineString (3 2, 4 2))" );
#else
      QTest::newRow( "make_valid_extravert" ) << "geom_to_wkt(make_valid(geom_from_wkt('POLYGON((3 2, 4 1, 5 8, 3 2, 4 2))')))" << false << QVariant( "Polygon ((3 2, 5 8, 4 1, 3 2))" );
#endif
#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 10
      QTest::newRow( "make_valid_missingEnd" ) << "geom_to_wkt(make_valid(geom_from_wkt('POLYGON((3 2, 4 1, 5 8))')))" << false << QVariant( "Polygon ((3 2, 4 1, 5 8, 3 2))" );
#else
      QTest::newRow( "make_valid_missingEnd" ) << "geom_to_wkt(make_valid(geom_from_wkt('POLYGON((3 2, 4 1, 5 8))')))" << false << QVariant( "Polygon ((3 2, 5 8, 4 1, 3 2))" );
      QTest::newRow( "make_valid_missingEnd linework" ) << "geom_to_wkt(make_valid(geom_from_wkt('POLYGON((3 2, 4 1, 5 8))'), method:='linework'))" << false << QVariant( "Polygon ((3 2, 4 1, 5 8, 3 2))" );
#endif
      QTest::newRow( "make_valid_wrongInput" ) << "make_valid('not a geometry')" << true << QVariant();
      QTest::newRow( "x point" ) << "x(make_point(2.2,4.4))" << false << QVariant( 2.2 );
      QTest::newRow( "y point" ) << "y(make_point(2.2,4.4))" << false << QVariant( 4.4 );
      QTest::newRow( "z point" ) << "z(make_point(2.2,4.4,6.6))" << false << QVariant( 6.6 );
      QTest::newRow( "z not point" ) << "z(geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'))" << false << QVariant();
      QTest::newRow( "z no z coord" ) << "z( geom_from_wkt( 'POINT ( 0 0 )' ) )" << false << QVariant();
      QTest::newRow( "m point" ) << "m(make_point_m(2.2,4.4,7.7))" << false << QVariant( 7.7 );
      QTest::newRow( "m not point" ) << "m(geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'))" << false << QVariant();
      QTest::newRow( "m no m coord" ) << "m( geom_from_wkt( 'POINT ( 0 0 )' ) )" << false << QVariant();
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
      QTest::newRow( "bearing 1" ) << "to_int(bearing( make_point(16198544, -4534850), make_point(18736872, -1877769), 'EPSG:3857', 'EPSG:7030')*1000000)" << false << QVariant( 872317 );
      QTest::newRow( "bearing 1 with CRS" ) << "to_int(bearing( make_point(16198544, -4534850), make_point(18736872, -1877769), crs_from_text('EPSG:3857'), 'EPSG:7030')*1000000)" << false << QVariant( 872317 );
      QTest::newRow( "bearing 2" ) << "to_int(bearing( make_point(-2074453, 9559553), make_point(-55665, 6828252), 'EPSG:3857', 'EPSG:7030')*1000000)" << false << QVariant( 2356910 );
      QTest::newRow( "bearing 3" ) << "to_int(degrees( bearing( make_point(16198544, -4534850), make_point(18736872, -1877769), 'EPSG:3857', 'EPSG:7030'))*1000000)" << false << QVariant( 49980071 );
      QTest::newRow( "bearing 4" ) << "to_int(degrees( bearing( make_point(18736872, -1877769), make_point(16198544, -4534850), 'EPSG:3857', 'WGS84'))*1000000)" << false << QVariant( 219282386 );
      QTest::newRow( "bearing multi point 1 part" ) << "to_int(bearing( geom_from_wkt('MULTIPOINT((16198544 -4534850))'), make_point(18736872, -1877769), 'EPSG:3857', 'EPSG:7030')*1000000)" << false << QVariant( 872317 );
      QTest::newRow( "bearing multi point 2 parts" ) << "bearing( geom_from_wkt('MULTIPOINT((16198544 -4534850),(16198545 -4534851))'), make_point(18736872, -1877769), 'EPSG:3857', 'EPSG:7030')" << true << QVariant();
      QTest::newRow( "bearing nonfinite" ) << "bearing( make_point(16198544, -4534850), make_point(18736872, -1877769), 'EPSG:4326', 'EPSG:7030')" << false << QVariant();
      QTest::newRow( "bearing transform error exception" ) << "bearing( make_point(16198544, -4534850), make_point(18736872, -1877769), 'EPSG:32633', 'EPSG:7030')" << false << QVariant();
      QTest::newRow( "bearing invalid crs" ) << "to_int(bearing( make_point(16198544, -4534850), make_point(18736872, -1877769), crs_from_text('dummy_crs'), 'EPSG:7030')*1000000)" << true << QVariant();
      QTest::newRow( "project not geom" ) << "project( 'asd', 1, 2 )" << true << QVariant();
      QTest::newRow( "project not point" ) << "project( geom_from_wkt('LINESTRING(2 0,2 2, 3 2, 3 0)'), 1, 2 )" << true << QVariant();
      QTest::newRow( "project x" ) << "toint(x(project( make_point( 1, 2 ), 3, radians(270)))*1000000)" << false << QVariant( -2 * 1000000 );
      QTest::newRow( "project y" ) << "toint(y(project( point:=make_point( 1, 2 ), distance:=3, azimuth:=radians(270)))*1000000)" << false << QVariant( 2 * 1000000 );
      QTest::newRow( "project m value preserved" ) << "geom_to_wkt(project( make_point( 1, 2, 2, 5), 1, 0.0, 0.0 ) )" << false << QVariant( "Point ZM (1 2 3 5)" );
      QTest::newRow( "project 2D Point" ) << "geom_to_wkt(project( point:=make_point( 1, 2), distance:=1, azimuth:=radians(0), elevation:=0 ) )" << false << QVariant( "Point Z (1 2 nan)" );
      QTest::newRow( "project 3D Point" ) << "geom_to_wkt(project( make_point( 1, 2, 2), 5, radians(450), radians (450) ) )" << false << QVariant( "Point Z (6 2 2)" );
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
      QTest::newRow( "line_interpolate_point_by_m not geom" ) << "line_interpolate_point_by_m('g', 5)" << true << QVariant();
      QTest::newRow( "line_interpolate_point_by_m null" ) << "line_interpolate_point_by_m(NULL, 5)" << false << QVariant();
      QTest::newRow( "line_interpolate_point_by_m point" ) << "line_interpolate_point_by_m(geom_from_wkt('POINT(1 2)'),5)" << false << QVariant();
      QTest::newRow( "line_interpolate_point_by_m line" ) << "geom_to_wkt(line_interpolate_point_by_m(geometry:=geom_from_wkt('LineStringM(0 0 0, 10 10 10)'),m:=5))" << false << QVariant( "Point (5 5)" );
      QTest::newRow( "line_locate_point not geom" ) << "line_locate_point('g', geom_from_wkt('Point 5 0'))" << false << QVariant();
      QTest::newRow( "line_locate_point null" ) << "line_locate_point(NULL, geom_from_wkt('Point 5 0'))" << false << QVariant();
      QTest::newRow( "line_locate_point point" ) << "line_locate_point(geom_from_wkt('POINT(1 2)'),geom_from_wkt('Point 5 0'))" << false << QVariant();
      QTest::newRow( "line_locate_point line" ) << "line_locate_point(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),point:=geom_from_wkt('Point(5 0)'))" << false << QVariant( 5.0 );
      QTest::newRow( "line_locate_m not geom" ) << "line_locate_m('g', 0)" << true << QVariant();
      QTest::newRow( "line_locate_m null" ) << "line_locate_m(NULL, 0)" << false << QVariant();
      QTest::newRow( "line_locate_m point" ) << "line_locate_m(geom_from_wkt('POINT(1 2)'),0)" << false << QVariant();
      QTest::newRow( "line_locate_m line out of range" ) << "line_locate_m(geom_from_wkt('LineStringM(0 0 0, 10 10 10)'),m:=20)" << false << QVariant();
      QTest::newRow( "line_locate_m line" ) << "line_locate_m(geom_from_wkt('LineStringM(0 0 0, 10 10 10)'),m:=0)" << false << QVariant( 0.0 );
      QTest::newRow( "line_interpolate_angle not geom" ) << "line_interpolate_angle('g', 5)" << true << QVariant();
      QTest::newRow( "line_interpolate_angle null" ) << "line_interpolate_angle(NULL, 5)" << false << QVariant();
      QTest::newRow( "line_interpolate_angle point" ) << "line_interpolate_angle(geom_from_wkt('POINT(1 2)'),5)" << false << QVariant( 0.0 );
      QTest::newRow( "line_interpolate_angle line" ) << "line_interpolate_angle(geometry:=geom_from_wkt('LineString(0 0, 10 0)'),distance:=5)" << false << QVariant( 90.0 );
      QTest::newRow( "angle_at_vertex not geom" ) << "angle_at_vertex('g', 5)" << true << QVariant();
      QTest::newRow( "angle_at_vertex null" ) << "angle_at_vertex(NULL, 0)" << false << QVariant();
      QTest::newRow( "angle_at_vertex point" ) << "angle_at_vertex(geom_from_wkt('POINT(1 2)'),0)" << false << QVariant( 0.0 );
      QTest::newRow( "angle_at_vertex line" ) << "angle_at_vertex(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),vertex:=1)" << false << QVariant( 45.0 );
      QTest::newRow( "angle_at_vertex line negative" ) << "angle_at_vertex(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),vertex:=-1)" << false << QVariant( 0.0 );
      QTest::newRow( "distance_to_vertex not geom" ) << "distance_to_vertex('g', 5)" << true << QVariant();
      QTest::newRow( "distance_to_vertex null" ) << "distance_to_vertex(NULL, 0)" << false << QVariant();
      QTest::newRow( "distance_to_vertex point" ) << "distance_to_vertex(geom_from_wkt('POINT(1 2)'),0)" << false << QVariant( 0.0 );
      QTest::newRow( "distance_to_vertex line" ) << "distance_to_vertex(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),vertex:=1)" << false << QVariant( 10.0 );
      QTest::newRow( "distance_to_vertex line negative" ) << "distance_to_vertex(geometry:=geom_from_wkt('LineString(0 0, 10 0, 10 10)'),vertex:=-1)" << false << QVariant( 20.0 );
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
      QTest::newRow( "rotate not geom" ) << "rotate('g', 90)" << true << QVariant();
      QTest::newRow( "rotate null" ) << "rotate(NULL, 90)" << false << QVariant();
      QTest::newRow( "rotate point" ) << "geom_to_wkt(rotate(geom_from_wkt('POINT( 20 10)'), 90, geom_from_wkt('POINT( 30 15)')))" << false << QVariant( "Point (25 25)" );
      QTest::newRow( "rotate line centroid" ) << "geom_to_wkt(rotate(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),90))" << false << QVariant( "LineString (0 10, 0 0, 10 0)" );
      QTest::newRow( "rotate line fixed point" ) << "geom_to_wkt(rotate(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),90, make_point(5, 2)))" << false << QVariant( "LineString (3 7, 3 -3, 13 -3)" );
      QTest::newRow( "rotate line fixed point not geom" ) << "geom_to_wkt(rotate(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),90, 'a'))" << true << QVariant();
      QTest::newRow( "rotate line fixed point not point" ) << "geom_to_wkt(rotate(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),90, geom_from_wkt('LineString(0 0, 10 0, 10 10)')))" << true << QVariant();
      QTest::newRow( "rotate line fixed multi point" ) << "geom_to_wkt(rotate(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),90, geom_from_wkt('MULTIPOINT((-5 -3))')))" << false << QVariant( "LineString (-2 -8, -2 -18, 8 -18)" );
      QTest::newRow( "rotate line fixed multi point multiple" ) << "geom_to_wkt(rotate(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),90, geom_from_wkt('MULTIPOINT(-5 -3,1 2)')))" << true << QVariant();
      QTest::newRow( "rotate polygon centroid" ) << "geom_to_wkt(rotate(geom_from_wkt('Polygon((0 0, 10 0, 10 10, 0 0))'),-90))" << false << QVariant( "Polygon ((10 0, 10 10, 0 10, 10 0))" );
      QTest::newRow( "rotate multiline centroid, not per part" ) << "geom_to_wkt(rotate(geom_from_wkt('MultiLineString((0 0, 10 0, 10 10), (12 0, 12 12))'),90))" << false << QVariant( "MultiLineString ((0 12, 0 2, 10 2),(0 0, 12 0))" );
      QTest::newRow( "rotate multiline centroid, per part" ) << "geom_to_wkt(rotate(geom_from_wkt('MultiLineString((0 0, 10 0, 10 10), (12 0, 12 12))'),90, per_part:=true))" << false << QVariant( "MultiLineString ((0 10, 0 0, 10 0),(6 6, 18 6))" );
      // per part switch is ignored if explicit center is set
      QTest::newRow( "rotate multiline explicit center, per part" ) << "geom_to_wkt(rotate(geom_from_wkt('MultiLineString((0 0, 10 0, 10 10), (12 0, 12 12))'),90, make_point(5,2), per_part:=true))" << false << QVariant( "MultiLineString ((3 7, 3 -3, 13 -3),(3 -5, 15 -5))" );
      QTest::newRow( "scale not geom" ) << "scale('g', 1.2, 0.8)" << true << QVariant();
      QTest::newRow( "scale null" ) << "scale(NULL, 1.2, 0.8)" << false << QVariant();
      QTest::newRow( "scale point" ) << "geom_to_wkt(scale(geom_from_wkt('POINT( 20 10)'), 1.2, 0.8, geom_from_wkt('POINT( 30 15)')))" << false << QVariant( "Point (18 11)" );
      QTest::newRow( "scale line centroid" ) << "geom_to_wkt(scale(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),1.2, 0.8))" << false << QVariant( "LineString (-1 1, 11 1, 11 9)" );
      QTest::newRow( "scale line fixed point" ) << "geom_to_wkt(scale(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),1.2, 0.8, make_point(5, 2)))" << false << QVariant( "LineString (-1 0.4, 11 0.4, 11 8.4)" );
      QTest::newRow( "scale line fixed point not geom" ) << "geom_to_wkt(scale(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),1.2, 0.8, 'a'))" << true << QVariant();
      QTest::newRow( "scale line fixed point not point" ) << "geom_to_wkt(scale(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),1.2, 0.8, geom_from_wkt('LineString(0 0, 10 0, 10 10)')))" << true << QVariant();
      QTest::newRow( "scale line fixed multi point" ) << "geom_to_wkt(scale(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),1.2, 0.8, geom_from_wkt('MULTIPOINT((-5 -3))')))" << false << QVariant( "LineString (1 -0.6, 13 -0.6, 13 7.4)" );
      QTest::newRow( "scale line fixed multi point multiple" ) << "geom_to_wkt(scale(geom_from_wkt('LineString(0 0, 10 0, 10 10)'),1.2, 0.8, geom_from_wkt('MULTIPOINT(-5 -3,1 2)')))" << true << QVariant();
      QTest::newRow( "scale polygon centroid" ) << "geom_to_wkt(scale(geom_from_wkt('Polygon((0 0, 10 0, 10 10, 0 0))'), 1.2, 0.8))" << false << QVariant( "Polygon ((-1 1, 11 1, 11 9, -1 1))" );
      QTest::newRow( "affine_transform not geom" ) << "affine_transform('g', 0, 0, 0, 0, 0, 0)" << true << QVariant();
      QTest::newRow( "affine_transform null" ) << "affine_transform(NULL, 0, 0, 0, 0, 0, 0)" << false << QVariant();
      QTest::newRow( "affine_transform point XYZM" ) << "geom_to_wkt(affine_transform(geom_from_wkt('POINT(2 2 2 2)'), 2, 2, 180, 0, 1, 1, 1, 2, 2))" << false << QVariant( "Point ZM (2 0 5 5)" );
      QTest::newRow( "affine_transform point with negative scale" ) << "geom_to_wkt(affine_transform(geom_from_wkt('POINT(1 1)'), 0, 0, 90, -2, -2))" << false << QVariant( "Point (2 -2)" );
      QTest::newRow( "affine_transform line XY" ) << "geom_to_wkt(affine_transform(geom_from_wkt('LINESTRING(1 0, 2 0)'), 0, 0, 90, 2, 1))" << false << QVariant( "LineString (0 2, 0 4)" );
      QTest::newRow( "affine_transform polygon XYZ" ) << "geom_to_wkt(affine_transform(geom_from_wkt('POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))'), 0, 0, -90, 0.5, 0.5))" << false << QVariant( "Polygon ((0 0, 0.5 0, 0.5 -0.5, 0 -0.5, 0 0))" );
      QTest::newRow( "affine_transform point XY with translation on ZM" ) << "geom_to_wkt(affine_transform(geom_from_wkt('POINT(1 1)'), 0, 0, 0, 1, 1, 3, 4))" << false << QVariant( "Point ZM (1 1 3 4)" );
      QTest::newRow( "triangular_wave not geom" ) << "triangular_wave('g', 1, 2)" << true << QVariant();
      QTest::newRow( "triangular_wave null" ) << "triangular_wave(NULL, 1, 2)" << false << QVariant();
      QTest::newRow( "triangular_wave point" ) << "geom_to_wkt(triangular_wave(make_point(1,2), 1, 2))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "triangular_wave geometry" ) << "geom_to_wkt(triangular_wave(geom_from_wkt('LINESTRING(1 1, 10 1)'), 5, 1))" << false << QVariant( "LineString (1 1, 2.125 2, 4.375 0, 6.625 2, 8.875 0, 10 1)" );
      QTest::newRow( "triangular_wave_randomized not geom" ) << "triangular_wave_randomized('g', 1, 2, 3, 4)" << true << QVariant();
      QTest::newRow( "triangular_wave_randomized null" ) << "triangular_wave_randomized(NULL, 1, 2, 3, 4)" << false << QVariant();
      QTest::newRow( "triangular_wave_randomized point" ) << "geom_to_wkt(triangular_wave_randomized(make_point(1,2), 1, 2, 3, 4))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "triangular_wave_randomized geometry" ) << "geom_to_wkt(triangular_wave_randomized(geom_from_wkt('LINESTRING(1 1, 10 1)'), 2, 3, 0.5, 1, 1), 1)" << false << QVariant( "LineString (1 1, 1.7 2, 2.8 0, 3.9 1.7, 5.1 0.2, 6.6 1.9, 7.7 0.2, 9 1.6, 10 1)" );
      QTest::newRow( "square_wave not geom" ) << "square_wave('g', 1, 2)" << true << QVariant();
      QTest::newRow( "square_wave null" ) << "square_wave(NULL, 1, 2)" << false << QVariant();
      QTest::newRow( "square_wave point" ) << "geom_to_wkt(square_wave(make_point(1,2), 1, 2))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "square_wave geometry" ) << "geom_to_wkt(square_wave(geom_from_wkt('LINESTRING(1 1, 10 1)'), 5, 1))" << false << QVariant( "LineString (1 1, 1 2, 3.25 2, 3.25 0, 5.5 0, 5.5 2, 7.75 2, 7.75 0, 10 0, 10 1)" );
      QTest::newRow( "square_wave_randomized not geom" ) << "square_wave_randomized('g', 1, 2, 3, 4)" << true << QVariant();
      QTest::newRow( "square_wave_randomized null" ) << "square_wave_randomized(NULL, 1, 2, 3, 4)" << false << QVariant();
      QTest::newRow( "square_wave_randomized point" ) << "geom_to_wkt(square_wave_randomized(make_point(1,2), 1, 2, 3, 4))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "square_wave_randomized geometry" ) << "geom_to_wkt(square_wave_randomized(geom_from_wkt('LINESTRING(1 1, 10 1)'), 2, 3, 0.5, 1, 1), 1)" << false << QVariant( "LineString (1 1, 1 2, 2.5 2, 2.5 0.4, 4 0.4, 4 1.6, 5.2 1.6, 5.2 0.3, 6.5 0.3, 6.5 2, 7.9 2, 7.9 0.3, 9.2 0.3, 9.2 1.7, 10 1.7, 10 1)" );
      QTest::newRow( "wave not geom" ) << "wave('g', 1, 2)" << true << QVariant();
      QTest::newRow( "wave null" ) << "wave(NULL, 1, 2)" << false << QVariant();
      QTest::newRow( "wave point" ) << "geom_to_wkt(wave(make_point(1,2), 1, 2))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "wave geometry" ) << "left(geom_to_wkt(wave(geom_from_wkt('LINESTRING(1 1, 10 1)'), 5, 1), 1), 100)" << false << QVariant( "LineString (1 1, 1.1 0.9, 1.2 0.7, 1.3 0.6, 1.4 0.4, 1.6 0.3, 1.7 0.2, 1.8 0.1, 1.9 0.1, 2 0, 2.1 0," );
      QTest::newRow( "wave_randomized not geom" ) << "wave_randomized('g', 1, 2, 3, 4)" << true << QVariant();
      QTest::newRow( "wave_randomized null" ) << "wave_randomized(NULL, 1, 2, 3, 4)" << false << QVariant();
      QTest::newRow( "wave_randomized point" ) << "geom_to_wkt(wave_randomized(make_point(1,2), 1, 2, 3, 4))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "wave_randomized geometry" ) << "left(geom_to_wkt(wave_randomized(geom_from_wkt('LINESTRING(1 1, 10 1)'), 2, 3, 0.5, 1, 1), 1), 100)" << false << QVariant( "LineString (1 1, 1.1 0.9, 1.1 0.7, 1.2 0.6, 1.3 0.4, 1.4 0.3, 1.4 0.2, 1.5 0.1, 1.6 0.1, 1.7 0, 1.7 " );
      QTest::newRow( "apply_dash_pattern not geom" ) << "apply_dash_pattern('g', array(1, 2))" << true << QVariant();
      QTest::newRow( "apply_dash_pattern null" ) << "apply_dash_pattern(NULL, array(1, 2))" << false << QVariant();
      QTest::newRow( "apply_dash_pattern point" ) << "geom_to_wkt(apply_dash_pattern(make_point(1,2), array(1, 2)))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "apply_dash_pattern bad pattern" ) << "apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(1, 'a'))" << true << QVariant();
      QTest::newRow( "apply_dash_pattern bad pattern 2" ) << "apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(1, 2, 3))" << true << QVariant();
      QTest::newRow( "apply_dash_pattern bad pattern 3" ) << "apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), 3)" << true << QVariant();
      QTest::newRow( "apply_dash_pattern bad rule 1" ) << "apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(1,2), 'bad')" << true << QVariant();
      QTest::newRow( "apply_dash_pattern bad rule 2" ) << "apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(1,2), 'full_dash', 'bad')" << true << QVariant();
      QTest::newRow( "apply_dash_pattern bad rule 3" ) << "apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(1,2), 'full_dash', 'full_dash', 'bad')" << true << QVariant();
      QTest::newRow( "apply_dash_pattern bad rule 4" ) << "apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(1,2), 'full_dash', 'full_dash', 'both', 'a')" << true << QVariant();
      QTest::newRow( "apply_dash_pattern geometry" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1)))" << false << QVariant( "MultiLineString ((1 1, 4 1),(5 1, 8 1),(9 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern polygon" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('POLYGON((1 1, 10 1, 10 10, 1 10, 1 1))'), array(3, 1)))" << false << QVariant( "MultiLineString ((1 1, 4 1),(5 1, 8 1),(9 1, 10 1, 10 3),(10 4, 10 7),(10 8, 10 10, 9 10),(8 10, 5 10),(4 10, 1 10),(1 9, 1 6),(1 5, 1 2),(1 1, 1 1, 1 1))" );
      QTest::newRow( "apply_dash_pattern geometry rule 1" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), start_rule:='full_dash'))" << false << QVariant( "MultiLineString ((1 1, 4 1),(5 1, 8 1),(9 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry rule 2" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), start_rule:='half_dash'))" << false << QVariant( "MultiLineString ((1 1, 2.5 1),(3.5 1, 6.5 1),(7.5 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry rule 3" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), start_rule:='full_gap'))" << false << QVariant( "MultiLineString ((2 1, 5 1),(6 1, 9 1),(10 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry rule 4" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), start_rule:='half_gap'))" << false << QVariant( "MultiLineString ((1.5 1, 4.5 1),(5.5 1, 8.5 1),(9.5 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry end rule 1" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), end_rule:='full_dash'))" << false << QVariant( "MultiLineString ((1 1, 3.45454545 1),(4.27272727 1, 6.72727273 1),(7.54545455 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry end rule 2" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), end_rule:='half_dash'))" << false << QVariant( "MultiLineString ((1 1, 3.84210526 1),(4.78947368 1, 7.63157895 1),(8.57894737 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry end rule 3" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), end_rule:='full_gap'))" << false << QVariant( "MultiLineString ((1 1, 4.375 1),(5.5 1, 8.875 1),(10 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry end rule 4" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), end_rule:='half_gap'))" << false << QVariant( "MultiLineString ((1 1, 4.6 1),(5.8 1, 9.4 1))" );
      QTest::newRow( "apply_dash_pattern geometry adjust 1" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), end_rule:='full_dash', adjustment:='both'))" << false << QVariant( "MultiLineString ((1 1, 3.45454545 1),(4.27272727 1, 6.72727273 1),(7.54545455 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry adjust 2" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), end_rule:='full_dash', adjustment:='dash'))" << false << QVariant( "MultiLineString ((1 1, 3.33333333 1),(4.33333333 1, 6.66666667 1),(7.66666667 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry adjust 3" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), end_rule:='full_dash', adjustment:='gap'))" << false << QVariant( "MultiLineString ((1 1, 4 1),(4 1, 7 1),(7 1, 10 1),(10 1, 10 1, 10 1))" );
      QTest::newRow( "apply_dash_pattern geometry pattern offset" ) << "geom_to_wkt(apply_dash_pattern(geom_from_wkt('LINESTRING(1 1, 10 1)'), array(3, 1), pattern_offset:=3))" << false << QVariant( "MultiLineString ((2 1, 5 1),(6 1, 9 1),(10 1, 10 1, 10 1))" );
      QTest::newRow( "densify_by_count not geom" ) << "densify_by_count('g', 3)" << true << QVariant();
      QTest::newRow( "densify_by_count null" ) << "densify_by_count(NULL, 3)" << false << QVariant();
      QTest::newRow( "densify_by_count point" ) << "geom_to_wkt(densify_by_count(make_point(1,2),3))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "densify_by_count geometry" ) << "geom_to_wkt(densify_by_count(geom_from_wkt('LINESTRING(1 1, 10 1)'), 3))" << false << QVariant( "LineString (1 1, 3.25 1, 5.5 1, 7.75 1, 10 1)" );
      QTest::newRow( "densify_by_count polygon" ) << "geom_to_wkt(densify_by_count(geom_from_wkt('POLYGON((1 1, 10 1, 10 10, 1 10, 1 1))'), 2))" << false << QVariant( "Polygon ((1 1, 4 1, 7 1, 10 1, 10 4, 10 7, 10 10, 7 10, 4 10, 1 10, 1 7, 1 4, 1 1))" );
      QTest::newRow( "densify_by_distance not geom" ) << "densify_by_distance('g', 3)" << true << QVariant();
      QTest::newRow( "densify_by_distance null" ) << "densify_by_distance(NULL, 3)" << false << QVariant();
      QTest::newRow( "densify_by_distance point" ) << "geom_to_wkt(densify_by_distance(make_point(1,2),3))" << false << QVariant( "Point (1 2)" );
      QTest::newRow( "densify_by_distance geometry" ) << "geom_to_wkt(densify_by_distance(geom_from_wkt('LINESTRING(1 1, 10 1)'), 4))" << false << QVariant( "LineString (1 1, 4 1, 7 1, 10 1)" );
      QTest::newRow( "densify_by_distance polygon" ) << "geom_to_wkt(densify_by_distance(geom_from_wkt('POLYGON((1 1, 10 1, 10 10, 1 10, 1 1))'), 2))" << false << QVariant( "Polygon ((1 1, 2.8 1, 4.6 1, 6.4 1, 8.2 1, 10 1, 10 2.8, 10 4.6, 10 6.4, 10 8.2, 10 10, 8.2 10, 6.4 10, 4.6 10, 2.8 10, 1 10, 1 8.2, 1 6.4, 1 4.6, 1 2.8, 1 1))" );
      QTest::newRow( "is_multipart true" ) << "is_multipart(geom_from_wkt('MULTIPOINT ((0 0),(1 1),(2 2))'))" << false << QVariant( true );
      QTest::newRow( "is_multipart false" ) << "is_multipart(geom_from_wkt('POINT (0 0)'))" << false << QVariant( false );
      QTest::newRow( "is_multipart false empty geometry" ) << "is_multipart(geom_from_wkt('POINT EMPTY'))" << false << QVariant( false );
      QTest::newRow( "is_multipart null" ) << "is_multipart(NULL)" << false << QVariant();
      QTest::newRow( "z_max no 3D" ) << "z_max(geom_from_wkt('POINT (0 0)'))" << false << QVariant();
      QTest::newRow( "z_max NULL" ) << "z_max(geom_from_wkt(NULL))" << false << QVariant();
      QTest::newRow( "z_max point" ) << "z_max(geom_from_wkt('POINT (0 0 1)'))" << false << QVariant( 1.0 );
      QTest::newRow( "z_max point Z NaN" ) << "z_max(geom_from_wkt('PointZ (1 1 nan)'))" << false << QVariant();
      QTest::newRow( "z_max line Z NaN" ) << "z_max(make_line(geom_from_wkt('PointZ (0 0 nan)'),make_point(-1,-1,-2)))" << false << QVariant();
      QTest::newRow( "z_max line" ) << "z_max(make_line(make_point(0,0,0),make_point(-1,-1,-2)))" << false << QVariant( 0.0 );
      QTest::newRow( "z_min no 3D" ) << "z_min(geom_from_wkt('POINT (0 0)'))" << false << QVariant();
      QTest::newRow( "z_min NULL" ) << "z_min(geom_from_wkt(NULL))" << false << QVariant();
      QTest::newRow( "z_min point" ) << "z_min(geom_from_wkt('POINT (0 0 1)'))" << false << QVariant( 1.0 );
      QTest::newRow( "z_min point Z NaN" ) << "z_min(geom_from_wkt('PointZ (1 1 nan)'))" << false << QVariant();
      QTest::newRow( "z_min line Z NaN" ) << "z_min(make_line(geom_from_wkt('PointZ (0 0 nan)'),make_point(-1,-1,-2)))" << false << QVariant();
      QTest::newRow( "z_min line" ) << "z_min(make_line(make_point(0,0,0),make_point(-1,-1,-2)))" << false << QVariant( -2.0 );
      QTest::newRow( "m_max no measure" ) << "m_max(geom_from_wkt('POINT (0 0)'))" << false << QVariant();
      QTest::newRow( "m_max NULL" ) << "m_max(geom_from_wkt(NULL))" << false << QVariant();
      QTest::newRow( "m_max point" ) << "m_max(make_point_m(0,0,1))" << false << QVariant( 1.0 );
      QTest::newRow( "m_max point M NaN" ) << "m_max(geom_from_wkt('PointZM (0 0 0 nan)'))" << false << QVariant();
      QTest::newRow( "m_max line M NaN" ) << "m_max(make_line(geom_from_wkt('PointZM (0 0 0 nan)'),geom_from_wkt('PointZM (1 1 1 2)')))" << false << QVariant();
      QTest::newRow( "m_max line" ) << "m_max(make_line(make_point_m(0,0,1),make_point_m(-1,-1,2),make_point_m(-2,-2,0)))" << false << QVariant( 2.0 );
      QTest::newRow( "m_min no measure" ) << "m_min(geom_from_wkt('POINT (0 0)'))" << false << QVariant();
      QTest::newRow( "m_min NULL" ) << "m_min(geom_from_wkt(NULL))" << false << QVariant();
      QTest::newRow( "m_min point" ) << "m_min(make_point_m(0,0,1))" << false << QVariant( 1.0 );
      QTest::newRow( "m_min point M NaN" ) << "m_min(geom_from_wkt('PointZM (0 0 0 nan)'))" << false << QVariant();
      QTest::newRow( "m_min line M NaN" ) << "m_min(make_line(geom_from_wkt('PointZM (0 0 0 nan)'),geom_from_wkt('PointZM (1 1 1 2)')))" << false << QVariant();
      QTest::newRow( "m_min line" ) << "m_min(make_line(make_point_m(0,0,1),make_point_m(-1,-1,2),make_point_m(-2,-2,0)))" << false << QVariant( 0.0 );
      QTest::newRow( "main angle polygon" ) << "round(main_angle( geom_from_wkt('POLYGON((0 0,2 9,9 2,0 0))')))" << false << QVariant( 77 );
      QTest::newRow( "main angle polygon edge case" ) << "round(main_angle( geom_from_wkt('POLYGON((353542.63843526 378974.92373469, 353544.95808017 378975.73690545, 353545.27173175 378974.84218528, 353542.95208684 378974.02901451, 353542.63843526 378974.92373469))')))" << false << QVariant( 71 );
      QTest::newRow( "main angle multi polygon" ) << "round(main_angle( geom_from_wkt('MULTIPOLYGON(((0 0,3 10,1 10,1 6,0 0)))')))" << false << QVariant( 17 );
      QTest::newRow( "main angle point" ) << "main_angle( geom_from_wkt('POINT (1.5 0.5)') )" << false << QVariant( 0.0 );
      QTest::newRow( "main angle line" ) << "round(main_angle( geom_from_wkt('LINESTRING (-1 2, 9 12)') ))" << false << QVariant( 45 );
      QTest::newRow( "main angle not geom" ) << "main_angle('g')" << true << QVariant();
      QTest::newRow( "main angle null" ) << "main_angle(NULL)" << false << QVariant();
      QTest::newRow( "main angle edge case 2" ) << "round(main_angle( geom_from_wkt('MULTIPOLYGON(((-57 -30, -56.5 -30, -56 -30, -55.5 -30, -55.5 -29.6667, -55.5 -29.333, -55.5 -29, -56 -29, -56.5 -29, -57 -29, -57 -29.3333, -57 -29.6666, -57 -30)))')))" << false << QVariant( 90 );
      QTest::newRow( "sinuosity not geom" ) << "sinuosity('g')" << true << QVariant();
      QTest::newRow( "sinuosity null" ) << "sinuosity(NULL)" << false << QVariant();
      QTest::newRow( "sinuosity point" ) << "sinuosity(geom_from_wkt('POINT(1 2)'))" << true << QVariant();
      QTest::newRow( "sinuosity multi linestring" ) << "sinuosity(geom_from_wkt('MULTILINESTRING( (0 0, 1 1), (2 2, 3 3) )'))" << true << QVariant();
      QTest::newRow( "sinuosity linestring" ) << "round(sinuosity(geom_from_wkt('LINESTRING(2 0, 2 2, 3 2, 3 3)')), 3)" << false << QVariant( 1.265 );
      QTest::newRow( "sinuosity linestring" ) << "sinuosity(geom_from_wkt('LINESTRING( 3 1, 5 1)'))" << false << QVariant( 1.0 );
      QTest::newRow( "sinuosity closed linestring" ) << "sinuosity(geom_from_wkt('LINESTRING( 3 1, 5 1, 2 2, 3 1)'))" << false << QVariant( std::numeric_limits<double>::quiet_NaN() );
      QTest::newRow( "sinuosity circularstring" ) << "round(sinuosity(geom_from_wkt('CircularString (20 30, 50 30, 50 90)')), 3)" << false << QVariant( 1.571 );
      QTest::newRow( "sinuosity closed circularstring" ) << "sinuosity(geom_from_wkt('CircularString (20 30, 50 30, 20 30)'))" << false << QVariant( std::numeric_limits<double>::quiet_NaN() );
      QTest::newRow( "straight_distance_2d not geom" ) << "straight_distance_2d('g')" << true << QVariant();
      QTest::newRow( "straight_distance_2d null" ) << "straight_distance_2d(NULL)" << false << QVariant();
      QTest::newRow( "straight_distance_2d point" ) << "straight_distance_2d(geom_from_wkt('POINT(1 2)'))" << true << QVariant();
      QTest::newRow( "straight_distance_2d multi linestring" ) << "straight_distance_2d(geom_from_wkt('MULTILINESTRING( (0 0, 1 1), (2 2, 3 3) )'))" << true << QVariant();
      QTest::newRow( "straight_distance_2d multi linestring with single part" ) << "straight_distance_2d(geom_from_wkt('MULTILINESTRING( (0 0, 0 1) ) )'))" << false << QVariant( 1.0 );
      QTest::newRow( "straight_distance_2d linestring" ) << "straight_distance_2d(geom_from_wkt('LINESTRING(1 0, 4 4)'))" << false << QVariant( 5.0 );
      QTest::newRow( "straight_distance_2d linestring" ) << "round(straight_distance_2d(geom_from_wkt('LINESTRING(1 4, 3 5, 5 0)')), 3)" << false << QVariant( 5.657 );
      QTest::newRow( "straight_distance_2d closed linestring" ) << "straight_distance_2d(geom_from_wkt('LINESTRING(2 2, 3 6, 2 2)'))" << false << QVariant( 0.0 );
      QTest::newRow( "straight_distance_2d circularstring" ) << "round(straight_distance_2d(geom_from_wkt('CircularString (20 30, 50 30, 10 50)')), 3)" << false << QVariant( 22.361 );
      QTest::newRow( "roundness not geom" ) << "roundness('r')" << true << QVariant();
      QTest::newRow( "roundness null" ) << "roundness(NULL)" << false << QVariant();
      QTest::newRow( "roundness not polygon" ) << "roundness(geom_from_wkt('POINT(1 2)'))" << true << QVariant();
      QTest::newRow( "roundness polygon" ) << "round(roundness(geom_from_wkt('POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0))')), 3)" << false << QVariant( 0.785 );
      QTest::newRow( "roundness single part multi polygon" ) << "round(roundness(geom_from_wkt('MULTIPOLYGON (((0 0, 0 1, 1 1, 1 0, 0 0)))')), 3)" << false << QVariant( 0.785 );
      QTest::newRow( "roundness multi polygon" ) << "round(roundness(geom_from_wkt('MULTIPOLYGON( ((0 0, 0 1, 1 1, 1 0, 0 0)), ((5 2, 4 9, 5 9, 6 5, 5 2)) )')))" << true << QVariant();
      QTest::newRow( "roundness thin polygon" ) << "roundness(geom_from_wkt('POLYGON(( 0 0, 0.5 0, 1 0, 0.6 0, 0 0))'))" << false << QVariant( 0.0 );
      QTest::newRow( "roundness circle polygon" ) << "roundness(geom_from_wkt('CurvePolygon (CompoundCurve (CircularString (0 0, 0 1, 1 1, 1 0, 0 0)))'))" << false << QVariant( 1.0 );
      QTest::newRow( "geometries_to_array_collection0" ) << "geom_to_wkt(array_get(geometries_to_array(geom_from_wkt('GeometryCollection (Polygon ((5 8, 4 1, 3 2, 5 8)),LineString (3 2, 4 2))')),0))" << false << QVariant( "Polygon ((5 8, 4 1, 3 2, 5 8))" );
      QTest::newRow( "geometries_to_array_collection1" ) << "geom_to_wkt(array_get(geometries_to_array(geom_from_wkt('GeometryCollection (Polygon ((5 8, 4 1, 3 2, 5 8)),LineString (3 2, 4 2))')),1))" << false << QVariant( "LineString (3 2, 4 2)" );
      QTest::newRow( "geometries_to_array_singlePoly" ) << "geom_to_wkt(array_first(geometries_to_array(geom_from_wkt('Polygon ((5 8, 4 1, 3 2, 5 8))'))))" << false << QVariant( "Polygon ((5 8, 4 1, 3 2, 5 8))" );
      QTest::newRow( "geometries_to_array_multipoly" ) << "geom_to_wkt(array_get(geometries_to_array(geom_from_wkt('MULTIPOLYGON(((5 5,0 0,0 10,5 5)),((5 5,10 10,10 0,5 5)))')),1))" << false << QVariant( "Polygon ((5 5, 10 10, 10 0, 5 5))" );
      QTest::newRow( "geometries_to_array_emptygeom" ) << "array_length(geometries_to_array(geom_from_wkt('LINESTRING EMPTY')))" << false << QVariant( 1 );
      QTest::newRow( "geometries_to_array_nongeom" ) << "geometries_to_array('just a string')" << true << QVariant();
#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 11 )
      QTest::newRow( "concave_hull not geom" ) << "concave_hull('r', 1)" << true << QVariant();
      QTest::newRow( "concave_hull null" ) << "concave_hull(NULL, 1)" << false << QVariant();
      QTest::newRow( "concave_hull point" ) << "geom_to_wkt(concave_hull(geom_from_wkt('Point(0 0)'), 0.99))" << false << QVariant( "Point (0 0)" );
      QTest::newRow( "concave_hull multilinestring" ) << "geom_to_wkt(concave_hull(geom_from_wkt('MULTILINESTRING((106 164,30 112,74 70,82 112,130 94,130 62,122 40,156 32,162 76,172 88),(132 178,134 148,128 136,96 128,132 108,150 130,170 142,174 110,156 96,158 90,158 88),(22 64,66 28,94 38,94 68,114 76,112 30,132 10,168 18,178 34,186 52,184 74,190 100,190 122,182 148,178 170,176 184,156 164,146 178,132 186,92 182,56 158,36 150,62 150,76 128,88 118))'), 0.99))" << false << QVariant( "Polygon ((30 112, 36 150, 92 182, 132 186, 176 184, 190 122, 190 100, 186 52, 178 34, 168 18, 132 10, 66 28, 22 64, 30 112))" );
#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 12 )
      QTest::newRow( "concave_hull multipoint" ) << "geom_to_wkt(concave_hull(geom_from_wkt('MultiPoint ((6.3 8.4),(7.6 8.8),(6.8 7.3),(5.3 1.8),(9.1 5),(8.1 7),(8.8 2.9),(2.4 8.2),(3.2 5.1),(3.7 2.3),(2.7 5.4),(8.4 1.9),(7.5 8.7),(4.4 4.2),(7.7 6.7),(9 3),(3.6 6.1),(3.2 6.5),(8.1 4.7),(8.8 5.8),(6.8 7.3),(4.9 9.5),(8.1 6),(8.7 5),(7.8 1.6),(7.9 2.1),(3 2.2),(7.8 4.3),(2.6 8.5),(4.8 3.4),(3.5 3.5),(3.6 4),(3.1 7.9),(8.3 2.9),(2.7 8.4),(5.2 9.8),(7.2 9.5),(8.5 7.1),(7.5 8.4),(7.5 7.7),(8.1 2.9),(7.7 7.3),(4.1 4.2),(8.3 7.2),(2.3 3.6),(8.9 5.3),(2.7 5.7),(5.7 9.7),(2.7 7.7),(3.9 8.8),(6 8.1),(8 7.2),(5.4 3.2),(5.5 2.6),(6.2 2.2),(7 2),(7.6 2.7),(8.4 3.5),(8.7 4.2),(8.2 5.4),(8.3 6.4),(6.9 8.6),(6 9),(5 8.6),(4.3 8),(3.6 7.3),(3.6 6.8),(4 7.5),(2.4 6.7),(2.3 6),(2.6 4.4),(2.8 3.3),(4 3.2),(4.3 1.9),(6.5 1.6),(7.3 1.6),(3.8 4.6),(3.1 5.9),(3.4 8.6),(4.5 9),(6.4 9.7))'), 0.99),2)" << false << QVariant( "Polygon ((2.4 8.2, 2.6 8.5, 5.2 9.8, 6.4 9.7, 7.2 9.5, 7.6 8.8, 8.5 7.1, 9.1 5, 9 3, 8.4 1.9, 7.8 1.6, 7.3 1.6, 6.5 1.6, 4.3 1.9, 3 2.2, 2.3 3.6, 2.3 6, 2.4 8.2))" );
#else
      QTest::newRow( "concave_hull multipoint" ) << "geom_to_wkt(concave_hull(geom_from_wkt('MultiPoint ((6.3 8.4),(7.6 8.8),(6.8 7.3),(5.3 1.8),(9.1 5),(8.1 7),(8.8 2.9),(2.4 8.2),(3.2 5.1),(3.7 2.3),(2.7 5.4),(8.4 1.9),(7.5 8.7),(4.4 4.2),(7.7 6.7),(9 3),(3.6 6.1),(3.2 6.5),(8.1 4.7),(8.8 5.8),(6.8 7.3),(4.9 9.5),(8.1 6),(8.7 5),(7.8 1.6),(7.9 2.1),(3 2.2),(7.8 4.3),(2.6 8.5),(4.8 3.4),(3.5 3.5),(3.6 4),(3.1 7.9),(8.3 2.9),(2.7 8.4),(5.2 9.8),(7.2 9.5),(8.5 7.1),(7.5 8.4),(7.5 7.7),(8.1 2.9),(7.7 7.3),(4.1 4.2),(8.3 7.2),(2.3 3.6),(8.9 5.3),(2.7 5.7),(5.7 9.7),(2.7 7.7),(3.9 8.8),(6 8.1),(8 7.2),(5.4 3.2),(5.5 2.6),(6.2 2.2),(7 2),(7.6 2.7),(8.4 3.5),(8.7 4.2),(8.2 5.4),(8.3 6.4),(6.9 8.6),(6 9),(5 8.6),(4.3 8),(3.6 7.3),(3.6 6.8),(4 7.5),(2.4 6.7),(2.3 6),(2.6 4.4),(2.8 3.3),(4 3.2),(4.3 1.9),(6.5 1.6),(7.3 1.6),(3.8 4.6),(3.1 5.9),(3.4 8.6),(4.5 9),(6.4 9.7))'), 0.99),2)" << false << QVariant( "Polygon ((2.3 6, 2.4 8.2, 2.6 8.5, 5.2 9.8, 6.4 9.7, 7.2 9.5, 7.6 8.8, 8.5 7.1, 9.1 5, 9 3, 8.4 1.9, 7.8 1.6, 7.3 1.6, 6.5 1.6, 4.3 1.9, 3 2.2, 2.3 3.6, 2.3 6))" );
#endif
#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 12 )
      QTest::newRow( "concave_hull multipoint allow holes" ) << "geom_to_wkt(concave_hull(geom_from_wkt('MultiPoint ((6.3 8.4),(7.6 8.8),(6.8 7.3),(5.3 1.8),(9.1 5),(8.1 7),(8.8 2.9),(2.4 8.2),(3.2 5.1),(3.7 2.3),(2.7 5.4),(8.4 1.9),(7.5 8.7),(4.4 4.2),(7.7 6.7),(9 3),(3.6 6.1),(3.2 6.5),(8.1 4.7),(8.8 5.8),(6.8 7.3),(4.9 9.5),(8.1 6),(8.7 5),(7.8 1.6),(7.9 2.1),(3 2.2),(7.8 4.3),(2.6 8.5),(4.8 3.4),(3.5 3.5),(3.6 4),(3.1 7.9),(8.3 2.9),(2.7 8.4),(5.2 9.8),(7.2 9.5),(8.5 7.1),(7.5 8.4),(7.5 7.7),(8.1 2.9),(7.7 7.3),(4.1 4.2),(8.3 7.2),(2.3 3.6),(8.9 5.3),(2.7 5.7),(5.7 9.7),(2.7 7.7),(3.9 8.8),(6 8.1),(8 7.2),(5.4 3.2),(5.5 2.6),(6.2 2.2),(7 2),(7.6 2.7),(8.4 3.5),(8.7 4.2),(8.2 5.4),(8.3 6.4),(6.9 8.6),(6 9),(5 8.6),(4.3 8),(3.6 7.3),(3.6 6.8),(4 7.5),(2.4 6.7),(2.3 6),(2.6 4.4),(2.8 3.3),(4 3.2),(4.3 1.9),(6.5 1.6),(7.3 1.6),(3.8 4.6),(3.1 5.9),(3.4 8.6),(4.5 9),(6.4 9.7))'), 0.99, true),2)" << false
                                                             << QVariant( "Polygon ((2.6 8.5, 5.2 9.8, 6.4 9.7, 7.2 9.5, 7.6 8.8, 8.5 7.1, 9.1 5, 9 3, 8.4 1.9, 7.8 1.6, 7.3 1.6, 6.5 1.6, 4.3 1.9, 3 2.2, 2.3 3.6, 2.3 6, 2.4 8.2, 2.6 8.5),(3.6 6.1, 4.4 4.2, 7.8 4.3, 6.8 7.3, 3.6 6.1))" );
#else
      QTest::newRow( "concave_hull multipoint allow holes" ) << "geom_to_wkt(concave_hull(geom_from_wkt('MultiPoint ((6.3 8.4),(7.6 8.8),(6.8 7.3),(5.3 1.8),(9.1 5),(8.1 7),(8.8 2.9),(2.4 8.2),(3.2 5.1),(3.7 2.3),(2.7 5.4),(8.4 1.9),(7.5 8.7),(4.4 4.2),(7.7 6.7),(9 3),(3.6 6.1),(3.2 6.5),(8.1 4.7),(8.8 5.8),(6.8 7.3),(4.9 9.5),(8.1 6),(8.7 5),(7.8 1.6),(7.9 2.1),(3 2.2),(7.8 4.3),(2.6 8.5),(4.8 3.4),(3.5 3.5),(3.6 4),(3.1 7.9),(8.3 2.9),(2.7 8.4),(5.2 9.8),(7.2 9.5),(8.5 7.1),(7.5 8.4),(7.5 7.7),(8.1 2.9),(7.7 7.3),(4.1 4.2),(8.3 7.2),(2.3 3.6),(8.9 5.3),(2.7 5.7),(5.7 9.7),(2.7 7.7),(3.9 8.8),(6 8.1),(8 7.2),(5.4 3.2),(5.5 2.6),(6.2 2.2),(7 2),(7.6 2.7),(8.4 3.5),(8.7 4.2),(8.2 5.4),(8.3 6.4),(6.9 8.6),(6 9),(5 8.6),(4.3 8),(3.6 7.3),(3.6 6.8),(4 7.5),(2.4 6.7),(2.3 6),(2.6 4.4),(2.8 3.3),(4 3.2),(4.3 1.9),(6.5 1.6),(7.3 1.6),(3.8 4.6),(3.1 5.9),(3.4 8.6),(4.5 9),(6.4 9.7))'), 0.99, true),2)" << false
                                                             << QVariant( "Polygon ((2.4 8.2, 2.6 8.5, 5.2 9.8, 6.4 9.7, 7.2 9.5, 7.6 8.8, 8.5 7.1, 9.1 5, 9 3, 8.4 1.9, 7.8 1.6, 7.3 1.6, 6.5 1.6, 4.3 1.9, 3 2.2, 2.3 3.6, 2.3 6, 2.4 8.2),(3.6 6.1, 4.4 4.2, 7.8 4.3, 6.8 7.3, 3.6 6.1))" );
#endif
#endif
      // string functions
      QTest::newRow( "format_number" ) << "format_number(1999.567,2)" << false << QVariant( "1,999.57" );
      QTest::newRow( "format_number large" ) << "format_number(9000000.0,0)" << false << QVariant( "9,000,000" );
      QTest::newRow( "format_number many decimals" ) << "format_number(123.45600,4)" << false << QVariant( "123.4560" );
      QTest::newRow( "format_number no decimals" ) << "format_number(1999.567,0)" << false << QVariant( "2,000" );
      QTest::newRow( "format_number omit group separator" ) << "format_number(1002999.567,0,omit_group_separators:=true)" << false << QVariant( "1003000" );
      QTest::newRow( "format_number omit group separator small" ) << "format_number(999,0,omit_group_separators:=true)" << false << QVariant( "999" );
      QTest::newRow( "format_number trim trailing zeros" ) << "format_number(123.45600,4,trim_trailing_zeroes:=true)" << false << QVariant( "123.456" );
      QTest::newRow( "format_number trim trailing zeros none" ) << "format_number(123.45600,2,trim_trailing_zeroes:=true)" << false << QVariant( "123.46" );
      QTest::newRow( "format_number trim trailing zeros many" ) << "format_number(123.45600,10,trim_trailing_zeroes:=true)" << false << QVariant( "123.456" );
      QTest::newRow( "format_number trim trailing zeros no decimal" ) << "format_number(123,0,trim_trailing_zeroes:=true)" << false << QVariant( "123" );
      QTest::newRow( "format_number language parameter" ) << "format_number(123457.00,2,'fr')" << false << QVariant( "123\u202F457,00" );
      QTest::newRow( "lower" ) << "lower('HeLLo')" << false << QVariant( "hello" );
      QTest::newRow( "upper" ) << "upper('HeLLo')" << false << QVariant( "HELLO" );
      QTest::newRow( "length" ) << "length('HeLLo')" << false << QVariant( 5 );
      QTest::newRow( "repeat 0" ) << "repeat('HeLLo', 0)" << false << QVariant( "" );
      QTest::newRow( "repeat 1" ) << "repeat('HeLLo', 1)" << false << QVariant( "HeLLo" );
      QTest::newRow( "repeat 3" ) << "repeat('HeLLo', 3)" << false << QVariant( "HeLLoHeLLoHeLLo" );
      QTest::newRow( "repeat -1" ) << "repeat('HeLLo', -1)" << false << QVariant( "" );
      QTest::newRow( "replace" ) << "replace('HeLLo', 'LL', 'xx')" << false << QVariant( "Hexxo" );
      QTest::newRow( "replace (array replaced by array)" ) << "replace('321', array('1','2','3'), array('7','8','9'))" << false << QVariant( "987" );
      QTest::newRow( "replace (array replaced by string)" ) << "replace('12345', array('2','4'), '')" << false << QVariant( "135" );
      QTest::newRow( "replace (unbalanced array, before > after)" ) << "replace('12345', array('1','2','3'), array('6','7'))" << true << QVariant();
      QTest::newRow( "replace (unbalanced array, before < after)" ) << "replace('12345', array('1','2'), array('6','7','8'))" << true << QVariant();
      QTest::newRow( "replace (map)" ) << "replace('APP SHOULD ROCK',map('APP','QGIS','SHOULD','DOES'))" << false << QVariant( "QGIS DOES ROCK" );
      QTest::newRow( "replace (map with overlapping keys)" ) << "replace('11111',map('1','small','11','large'))" << false << QVariant( "largelargesmall" );
      QTest::newRow( "regexp_replace" ) << "regexp_replace('HeLLo','[eL]+', '-')" << false << QVariant( "H-o" );
      QTest::newRow( "regexp_replace greedy" ) << "regexp_replace('HeLLo','(?<=H).*L', '-')" << false << QVariant( "H-o" );
      QTest::newRow( "regexp_replace non greedy" ) << "regexp_replace('HeLLo','(?<=H).*?L', '-')" << false << QVariant( "H-Lo" );
      QTest::newRow( "regexp_replace cap group" ) << "regexp_replace('HeLLo','(eL)', 'x\\\\1x')" << false << QVariant( "HxeLxLo" );
      QTest::newRow( "regexp_replace invalid" ) << "regexp_replace('HeLLo','[[[', '-')" << true << QVariant();
      QTest::newRow( "substr_count basic" ) << "substr_count('banana', 'an')" << false << QVariant( 2 );
      QTest::newRow( "substr_count basic funny" ) << "substr_count('Funniness', 'n')" << false << QVariant( 3 );
      QTest::newRow( "substr_count non-overlapping counted" ) << "substr_count('aaaaa', 'aa')" << false << QVariant( 2 );
      QTest::newRow( "substr_count overlapping counted" ) << "substr_count('aaaaa', 'aa', true)" << false << QVariant( 4 );
      QTest::newRow( "substr_count empty needle" ) << "substr_count('abc', '')" << false << QVariant( 0 );
      QTest::newRow( "substr_count case sensitivity" ) << "substr_count('BANANA', 'an')" << false << QVariant( 0 );
      QTest::newRow( "reverse string" ) << "reverse('HeLLo')" << false << QVariant( "oLLeH" );
      QTest::newRow( "reverse empty string" ) << "reverse('')" << false << QVariant( "" );
      // unaccent() tests aligned with PostgreSQL's contrib/unaccent/sql/unaccent.sql and some more tests
      // Source: https://raw.githubusercontent.com/postgres/postgres/refs/heads/master/contrib/unaccent/sql/unaccent.sql
      QTest::newRow( "unaccent basic french" ) << "unaccent('Hôtel crème brûlée')" << false << QVariant( "Hotel creme brulee" );
      QTest::newRow( "unaccent basic romanian" ) << "unaccent('Românește')" << false << QVariant( "Romaneste" );
      QTest::newRow( "unaccent ligatures and Polish" ) << "unaccent('Æsir & Œuvre, Łódź')" << false << QVariant( "AEsir & OEuvre, Lodz" );
      QTest::newRow( "unaccent lowercase accents" ) << "unaccent('crème brûlée')" << false << QVariant( "creme brulee" );
      QTest::newRow( "unaccent special letters" ) << "unaccent('straße, Łódź')" << false << QVariant( "strasse, Lodz" );
      QTest::newRow( "unaccent noop ascii" ) << "unaccent('plain ASCII')" << false << QVariant( "plain ASCII" );
      QTest::newRow( "unaccent empty" ) << "unaccent('')" << false << QVariant( "" );
      QTest::newRow( "unaccent null" ) << "unaccent(NULL)" << false << QVariant();
      QTest::newRow( "unaccent cyrillic small yo" ) << "unaccent('ёлка')" << false << QVariant( "елка" );
      QTest::newRow( "unaccent cyrillic Cyrillic capital yo: Ё → Е" ) << "unaccent('ЁЖИК')" << false << QVariant( "ЕЖИК" );
      QTest::newRow( "unaccent modifier symbols" ) << "unaccent('˃˖˗˜')" << false << QVariant( ">+-~" );
      QTest::newRow( "unaccent combining grave" ) << "unaccent('À')" << false << QVariant( "A" );
      QTest::newRow( "unaccent degree celsius fahrenheit" ) << "unaccent('℃℉')" << false << QVariant( "°C°F" );
      QTest::newRow( "unaccent sound recording copyright ℗ → P" ) << "unaccent('℗')" << false << QVariant( "(P)" );
      QTest::newRow( "unaccent vulgar fraction" ) << "unaccent('1½')" << false << QVariant( "1 1/2" );
      QTest::newRow( "unaccent quotation mark variant" ) << "unaccent('〝')" << false << QVariant( "\"" );
      QTest::newRow( "unaccent blackletter H" ) << "unaccent('ℌ')" << false << QVariant( "H" );
      QTest::newRow( "unaccent fullwidth number sign FE5F" ) << "unaccent('﹟')" << false << QVariant( "#" );
      QTest::newRow( "unaccent fullwidth hash FF03" ) << "unaccent('＃')" << false << QVariant( "#" );
      QTest::newRow( "substr" ) << "substr('HeLLo', 3,2)" << false << QVariant( "LL" );
      QTest::newRow( "substr named parameters" ) << "substr(string:='HeLLo',start:=3,length:=2)" << false << QVariant( "LL" );
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
      QTest::newRow( "ltrim none" ) << "ltrim('trim   ')" << false << QVariant( "trim   " );
      QTest::newRow( "ltrim space" ) << "ltrim('    trim  ')" << false << QVariant( "trim  " );
      QTest::newRow( "ltrim empty string" ) << "ltrim('')" << false << QVariant( "" );
      QTest::newRow( "ltrim('zzzytrim', 'xyz')" ) << "ltrim('zzzytrim', 'xyz')" << false << QVariant( "trim" );
      QTest::newRow( "ltrim('zzzytrim', 'a')" ) << "ltrim('zzzytrim', 'a')" << false << QVariant( "zzzytrim" );
      QTest::newRow( "ltrim('zzzytrim', '[(*')" ) << "ltrim('zzzytrim', '[(*')" << false << QVariant( "zzzytrim" );
      QTest::newRow( "ltrim('))(* *[[trim', '[())* ')" ) << "ltrim('))(* *[[trim', '[())* ')" << false << QVariant( "trim" );
      QTest::newRow( "rtrim none" ) << "rtrim('  trim')" << false << QVariant( "  trim" );
      QTest::newRow( "rtrim space" ) << "rtrim('    trim  ')" << false << QVariant( "    trim" );
      QTest::newRow( "rtrim empty string" ) << "rtrim('')" << false << QVariant( "" );
      QTest::newRow( "rtrim('trimzzzy', 'xyz')" ) << "rtrim('trimzzzy', 'xyz')" << false << QVariant( "trim" );
      QTest::newRow( "rtrim('trimzzzy', 'a')" ) << "rtrim('trimzzzy', 'a')" << false << QVariant( "trimzzzy" );
      QTest::newRow( "rtrim('trimzzzy', '[(*')" ) << "rtrim('trimzzzy', '[(*')" << false << QVariant( "trimzzzy" );
      QTest::newRow( "rtrim('trim)(* *[[', '[()* ')" ) << "rtrim('trim)(* *[[', '[()* ')" << false << QVariant( "trim" );
      QTest::newRow( "char" ) << "char(81)" << false << QVariant( "Q" );
      QTest::newRow( "ascii single letter" ) << "ascii('Q')" << false << QVariant( 81 );
      QTest::newRow( "ascii word" ) << "ascii('QGIS')" << false << QVariant( 81 );
      QTest::newRow( "ascii empty" ) << "ascii('')" << false << QVariant();
      QTest::newRow( "wordwrap" ) << "wordwrap('university of qgis',13)" << false << QVariant( "university of\nqgis" );
      QTest::newRow( "wordwrap with custom delimiter" ) << "wordwrap('university of qgis',13,' ')" << false << QVariant( "university of\nqgis" );
      QTest::newRow( "wordwrap with negative length" ) << "wordwrap('university of qgis',-3)" << false << QVariant( "university\nof qgis" );
      QTest::newRow( "wordwrap with negative length, custom delimiter" ) << "wordwrap('university of qgis',-3,' ')" << false << QVariant( "university\nof qgis" );
      QTest::newRow( "wordwrap on multi line" ) << "wordwrap('university of qgis\nsupports many multiline',-5,' ')" << false << QVariant( "university\nof qgis\nsupports\nmany multiline" );
      QTest::newRow( "wordwrap on zero-space width" ) << u"wordwrap('test%1zero-width space',4)"_s.arg( QChar( 8203 ) ) << false << QVariant( "test\nzero-width\nspace" );
      QTest::newRow( "format none" ) << "format()" << true << QVariant();
      QTest::newRow( "format one" ) << "format('bbb')" << false << QVariant( "bbb" );
      QTest::newRow( "format" ) << "format('%1 %2 %3 %1', 'One', 'Two', 'Three')" << false << QVariant( "One Two Three One" );
      QTest::newRow( "concat" ) << "concat('a', 'b', 'c', 'd')" << false << QVariant( "abcd" );
      QTest::newRow( "concat function single" ) << "concat('a')" << false << QVariant( "a" );
      QTest::newRow( "concat function with NULL" ) << "concat(NULL,'a','b')" << false << QVariant( "ab" );
      QTest::newRow( "array_to_string" ) << "array_to_string(array(1,2,3),',')" << false << QVariant( "1,2,3" );
      QTest::newRow( "array_to_string with custom empty value" ) << "array_to_string(array(1,'',3),',','*')" << false << QVariant( "1,*,3" );
      QTest::newRow( "array_to_string fail passing non-array" ) << "array_to_string('non-array',',')" << true << QVariant();
      QTest::newRow( "array_unique" ) << "array_to_string(array_distinct(array('hello','world','world','hello')))" << false << QVariant( "hello,world" );
      QTest::newRow( "array_unique fail passing non-array" ) << "array_distinct('non-array')" << true << QVariant();
      QTest::newRow( "array_replace" ) << "array_replace(array('H','e','L','L','o'), 'L', 'x')" << false << QVariant( QVariantList() << "H" << "e" << "x" << "x" << "o" );
      QTest::newRow( "array_replace (array replaced by array)" ) << "array_replace(array(3,2,1), array(1,2,3), array(7,8,9))" << false << QVariant( QVariantList() << 9 << 8 << 7 );
      QTest::newRow( "array_replace (arrayreplaced by string)" ) << "array_replace(array(1,2,3,4,5), array(2,4), '')" << false << QVariant( QVariantList() << 1 << "" << 3 << "" << 5 );
      QTest::newRow( "array_replace (unbalanced array, before > after)" ) << "array_replace(array(1,2,3,4,5), array(1,2,3), array(6,7))" << true << QVariant();
      QTest::newRow( "array_replace (unbalanced array, before < after)" ) << "array_replace(array(1,2,3,4,5), array(1,2), array(6,7,8))" << true << QVariant();
      QTest::newRow( "array_replace (map)" ) << "array_replace(array('APP','SHOULD','ROCK'),map('APP','QGIS','SHOULD','DOES'))" << false << QVariant( QVariantList() << "QGIS" << "DOES" << "ROCK" );

      // map HTML formatting
      QTest::newRow( "map_to_html_table (map)" ) << "map_to_html_table(map('APP','QGIS','<SHOULD>','DOES'))" << false << QVariant( "\n  <table>\n    <thead>\n      <tr><th>&lt;SHOULD&gt;</th><th>APP</th></tr>\n    </thead>\n    <tbody>\n      <tr><td>DOES</td><td>QGIS</td></tr>\n    </tbody>\n  </table>" );
      QTest::newRow( "map_to_html_dl (map)" ) << "map_to_html_dl(map('APP','QGIS','<SHOULD>','DOES'))" << false << QVariant( "\n  <dl>\n    <dt>&lt;SHOULD&gt;</dt><dd>DOES</dd><dt>APP</dt><dd>QGIS</dd>\n  </dl>" );

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
      // testing unicode and \b, see #41453. \b tests for a 'word boundary'
      QTest::newRow( "regexp match unicode" ) << "regexp_match('Budač','Buda\\\\b')" << false << QVariant( 0 );
      QTest::newRow( "regexp match unicode 2" ) << "regexp_match('Buda','Buda\\\\b')" << false << QVariant( 1 );
      QTest::newRow( "regexp match unicode 3" ) << "regexp_match('Budač','Budač\\\\b')" << false << QVariant( 1 );

      QTest::newRow( "regexp match invalid" ) << "regexp_match('abc DEF','[[[')" << true << QVariant();
      QTest::newRow( "regexp match escaped" ) << "regexp_match('abc DEF','\\\\s[A-Z]+')" << false << QVariant( 4 );
      QTest::newRow( "regexp match false" ) << "regexp_match('abc DEF','\\\\s[a-z]+')" << false << QVariant( 0 );
      QTest::newRow( "if true" ) << "if(1=1, 1, 0)" << false << QVariant( 1 );
      QTest::newRow( "if false" ) << "if(1=2, 1, 0)" << false << QVariant( 0 );
      QTest::newRow( "try valid" ) << "try(to_int('1'),0)" << false << QVariant( 1 );
      QTest::newRow( "try invalid with alternative" ) << "try(to_int('a'),0)" << false << QVariant( 0 );
      QTest::newRow( "try invalid without alternative" ) << "try(to_int('a'))" << false << QVariant();

      QTest::newRow( "to_bool with empty string" ) << "to_bool('')" << false << QVariant( false );
      QTest::newRow( "to_bool with non-empty string" ) << "to_bool('0')" << false << QVariant( true );
      QTest::newRow( "to_bool with zero" ) << "to_bool(0)" << false << QVariant( false );
      QTest::newRow( "to_bool with number" ) << "to_bool(123)" << false << QVariant( true );
      QTest::newRow( "to_bool with null" ) << "to_bool(null)" << false << QVariant( false );
      QTest::newRow( "to_bool with empty list" ) << "to_bool(array())" << false << QVariant( false );
      QTest::newRow( "to_bool with non-empty list" ) << "to_bool(array(1,2,3))" << false << QVariant( true );

      // Datetime functions
      QTest::newRow( "make date" ) << "make_date(2012,6,28)" << false << QVariant( QDate( 2012, 6, 28 ) );
      QTest::newRow( "make date invalid" ) << "make_date('a',6,28)" << true << QVariant();
      QTest::newRow( "make date invalid 2" ) << "make_date(2012,16,28)" << true << QVariant();
      QTest::newRow( "make time" ) << "make_time(13,6,28)" << false << QVariant( QTime( 13, 6, 28 ) );
      QTest::newRow( "make time with ms" ) << "make_time(13,6,28.5)" << false << QVariant( QTime( 13, 6, 28, 500 ) );
      QTest::newRow( "make time invalid" ) << "make_time('a',6,28)" << true << QVariant();
      QTest::newRow( "make time invalid 2" ) << "make_time(2012,16,28)" << true << QVariant();
      QTest::newRow( "make datetime" ) << "make_datetime(2012,7,8,13,6,28)" << false << QVariant( QDateTime( QDate( 2012, 7, 8 ), QTime( 13, 6, 28 ) ) );
      QTest::newRow( "make datetime with ms" ) << "make_datetime(2012,7,8,13,6,28.5)" << false << QVariant( QDateTime( QDate( 2012, 7, 8 ), QTime( 13, 6, 28, 500 ) ) );
      QTest::newRow( "make datetime invalid" ) << "make_datetime(2012,7,8,'a',6,28)" << true << QVariant();
      QTest::newRow( "make datetime invalid 2" ) << "make_datetime(2012,7,8,2012,16,28)" << true << QVariant();
      QTest::newRow( "make interval years" ) << "second(make_interval(years:=2))" << false << QVariant( 63115200.0 );
      QTest::newRow( "make interval months" ) << "second(make_interval(months:=2))" << false << QVariant( 5184000.0 );
      QTest::newRow( "make interval weeks" ) << "second(make_interval(weeks:=2))" << false << QVariant( 1209600.0 );
      QTest::newRow( "make interval days" ) << "second(make_interval(days:=2))" << false << QVariant( 172800.0 );
      QTest::newRow( "make interval hours" ) << "second(make_interval(hours:=2))" << false << QVariant( 7200.0 );
      QTest::newRow( "make interval minutes" ) << "second(make_interval(minutes:=2))" << false << QVariant( 120.0 );
      QTest::newRow( "make interval seconds" ) << "second(make_interval(seconds:=2))" << false << QVariant( 2.0 );
      QTest::newRow( "make interval mixed" ) << "second(make_interval(2,3,4,5,6,7,8))" << false << QVariant( 73764428.0 );
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
      // datetime_from_epoch will always return a local datetime, so here we create some circular magic to create a local datetime during test (so test can be ran in every timezone...)
      QTest::newRow( "datetime_from_epoch" ) << "datetime_from_epoch(epoch(to_datetime('2017-01-01T00:00:01')))" << false << QVariant( QDateTime( QDate( 2017, 1, 1 ), QTime( 0, 0, 1 ), Qt::LocalTime ) );
      QTest::newRow( "datetime_from_epoch_null" ) << "datetime_from_epoch(NULL)" << false << QgsVariantUtils::createNullVariant( QMetaType::Type::UnknownType );
      QTest::newRow( "date from format" ) << "to_date('June 29, 2019','MMMM d, yyyy')" << false << QVariant( QDate( 2019, 6, 29 ) );
      QTest::newRow( "date from format and language" ) << "to_date('29 juin, 2019','d MMMM, yyyy','fr')" << false << QVariant( QDate( 2019, 6, 29 ) );
      QTest::newRow( "date from format, wrong string" ) << "to_date('wrong.string.here','yyyy.MM.dd')" << true << QVariant();
      QTest::newRow( "date from format, wrong format" ) << "to_date('2019-01-01','wrong')" << true << QVariant();
      QTest::newRow( "date from format, language missing format" ) << "to_date('2019-01-01',language:='fr')" << true << QVariant();
      QTest::newRow( "datetime from format" ) << "to_datetime('June 29, 2019 @ 11:00','MMMM d, yyyy @ HH:mm')" << false << QVariant( QDateTime( QDate( 2019, 6, 29 ), QTime( 11, 0 ) ) );
      QTest::newRow( "datetime from format and language" ) << "to_datetime('29 juin, 2019 @ 11:00','d MMMM, yyyy @ HH:mm','fr')" << false << QVariant( QDateTime( QDate( 2019, 6, 29 ), QTime( 11, 0 ) ) );
      QTest::newRow( "time from format" ) << "to_time('12:34:56','HH:mm:ss')" << false << QVariant( QTime( 12, 34, 56 ) );
      QTest::newRow( "time from format and language" ) << "to_time('12:34:56','HH:mm:ss','fr')" << false << QVariant( QTime( 12, 34, 56 ) );
      QTest::newRow( "formatted string from date" ) << "format_date('2019-06-29','MMMM d, yyyy')" << false << QVariant( QString( "June 29, 2019" ) );
      QTest::newRow( "formatted string from date with language" ) << "format_date('2019-06-29','d MMMM yyyy','fr')" << false << QVariant( QString( "29 juin 2019" ) );
      QTest::newRow( "formatted string with Z" ) << "format_date(to_datetime('2019-06-29T13:34:56+01:00'),'yyyy-MM-ddTHH:mm:ssZ')" << false << QVariant( QString( "2019-06-29T12:34:56Z" ) );

      // time zone functions
      QTest::newRow( "timezone_from_id NULL" ) << "timezone_from_id(NULL)" << false << QVariant();
      QTest::newRow( "timezone_from_id not string" ) << "timezone_from_id(123)" << true << QVariant();
      QTest::newRow( "timezone_from_id invalid id" ) << "timezone_from_id('xxxx')" << true << QVariant();
      QTest::newRow( "timezone_id NULL" ) << "timezone_id(NULL)" << false << QVariant();
      QTest::newRow( "timezone_id not timezone" ) << "timezone_id(123)" << true << QVariant();
      QTest::newRow( "timezone_id valid timezone" ) << "timezone_id(timezone_from_id('Australia/Brisbane'))" << false << QVariant( QString( "Australia/Brisbane" ) );
      QTest::newRow( "timezone_id UTC+10:30" ) << "timezone_id(timezone_from_id('UTC+10:30'))" << false << QVariant( QString( "UTC+10:30" ) );

#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )
      QTest::newRow( "timezone_id UTC-3" ) << "timezone_id(timezone_from_id('UTC-3'))" << false << QVariant( QString( "UTC-03:00" ) );
      QTest::newRow( "convert timezone, fixed datetime" ) << "format_date(convert_timezone(to_datetime('2012-05-04 12:50:00+3'), timezone_from_id('UTC+10')), 'yyyy-MM-dd HH:mm:ss t')" << false << QVariant( QString( "2012-05-04 19:50:00 UTC+10:00" ) );
      QTest::newRow( "set timezone, fixed datetime" ) << "format_date(set_timezone(to_datetime('2012-05-04 12:50:00+3'), timezone_from_id('UTC+10')), 'yyyy-MM-dd HH:mm:ss t')" << false << QVariant( QString( "2012-05-04 12:50:00 UTC+10:00" ) );
      QTest::newRow( "get timezone, fixed datetime" ) << "timezone_id(get_timezone(to_datetime('2012-05-04 12:50:00+3')))" << false << QVariant( QString( "UTC+03:00" ) );
#else
      QTest::newRow( "timezone_id UTC-3" ) << "timezone_id(timezone_from_id('UTC-3'))" << false << QVariant( QString( "UTC-03" ) );
      QTest::newRow( "convert timezone, fixed datetime" ) << "format_date(convert_timezone(to_datetime('2012-05-04 12:50:00+3'), timezone_from_id('UTC+10')), 'yyyy-MM-dd HH:mm:ss t')" << false << QVariant( QString( "2012-05-04 19:50:00 UTC+10" ) );
      QTest::newRow( "set timezone, fixed datetime" ) << "format_date(set_timezone(to_datetime('2012-05-04 12:50:00+3'), timezone_from_id('UTC+10')), 'yyyy-MM-dd HH:mm:ss t')" << false << QVariant( QString( "2012-05-04 12:50:00 UTC+10" ) );
      QTest::newRow( "get timezone, fixed datetime" ) << "timezone_id(get_timezone(to_datetime('2012-05-04 12:50:00+3')))" << false << QVariant( QString( "UTC+03" ) );
#endif


      // Color functions
      QTest::newRow( "ramp color" ) << "ramp_color('Spectral',0.3)" << false << QVariant( "253,190,116,255" );
      QTest::newRow( "ramp color error" ) << "ramp_color('NotExistingRamp',0.3)" << true << QVariant();
      QTest::newRow( "ramp color object" ) << "ramp_color_object('Spectral',0.3)" << false << QVariant( QColor::fromRgbF( 0.994, 0.746, 0.454 ) );
      QTest::newRow( "ramp color object error" ) << "ramp_color_object('NotExistingRamp',0.3)" << true << QVariant();
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
      QTest::newRow( "color grayscale average object" ) << "color_grayscale_average(color_rgbf(0.6,0.5,0.1))" << false << QVariant( QColor::fromRgbF( 0.4, 0.4, 0.4 ) );
      QTest::newRow( "color grayscale average cmyk" ) << "color_grayscale_average(color_cmykf(0.6,0.5,0.1,0.8))" << false << QVariant( QColor::fromCmykF( 0.4, 0.4, 0.4, 0.8 ) );
      QTest::newRow( "color mix rgb" ) << "color_mix_rgb('0,0,0,100','255,255,255',0.5)" << false << QVariant( "127,127,127,177" );
      QTest::newRow( "color mix" ) << "color_mix('0,0,0,100','255,255,255',0.5)" << false << QVariant( "128,128,128,178" );
      QTest::newRow( "color mix mixed types" ) << "color_mix('0,0,0,100',color_rgbf(1.0,1.0,1.0),0.5)" << true << QVariant();
      QTest::newRow( "color mix mixed color types" ) << "color_mix(color_cmykf(1.0,1.0,1.0,1.0),color_rgbf(1.0,1.0,1.0),0.5)" << true << QVariant();
      QTest::newRow( "color mix cmyk" ) << "color_mix(color_cmykf(0.9,0.9,0.9,0.9),color_cmykf(0.1,0.1,0.1,0.1),0.5)" << false << QVariant( QColor::fromCmykF( 0.5, 0.5, 0.5, 0.5 ) );

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
      QTest::newRow( "color part object" ) << "to_int(color_part(color_cmykf(0.1,0.2,0.3,0.9),'black'))" << false << QVariant( 90 );
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
      QTest::newRow( "set color part hsl_saturation" ) << "to_int(color_part(set_color_part(color_hsl(100,50,70),'hsl_saturation',30),'hsl_saturation'))" << false << QVariant( 29 );
      QTest::newRow( "set color part lightness" ) << "to_int(color_part(set_color_part(color_hsl(100,50,70),'lightness',20),'lightness'))" << false << QVariant( 20 );
      QTest::newRow( "set color part cyan" ) << "to_int(color_part(set_color_part(color_cmyk(21,0,92,70),'cyan',12),'cyan'))" << false << QVariant( 12 );
      QTest::newRow( "set color part magenta" ) << "to_int(color_part(set_color_part(color_cmyk(0,10,90,76),'magenta',31),'magenta'))" << false << QVariant( 31 );
      QTest::newRow( "set color part yellow" ) << "to_int(color_part(set_color_part(color_cmyk(21,0,92,70),'yellow',96),'yellow'))" << false << QVariant( 96 );
      QTest::newRow( "set color part black" ) << "to_int(color_part(set_color_part(color_cmyk(21,0,92,70),'black',100),'black'))" << false << QVariant( 100 );
      QTest::newRow( "set color part object" ) << "to_int(color_part(set_color_part(color_cmykf(0.21,0,0.92,0.70),'black',100),'black'))" << false << QVariant( 100 );

      QTest::newRow( "color darker" ) << "darker('200,100,30',150)" << false << QVariant( "133,67,20,255" );
      QTest::newRow( "color darker bad color" ) << "darker('notacolor',150)" << true << QVariant();
      QTest::newRow( "color lighter" ) << "lighter('200,100,30',150)" << false << QVariant( "255,154,83,255" );
      QTest::newRow( "color lighter bad color" ) << "lighter('notacolor',150)" << true << QVariant();
      QTest::newRow( "color darker object " ) << "darker(color_cmykf(0.1,0.4,0.6,0.6),150)" << false << QVariant( QColor::fromCmykF( 0, 0.333, 0.5555, 0.76 ) );
      QTest::newRow( "color lighter object" ) << "lighter(color_cmykf(0.1,0.4,0.6,0.6),150)" << false << QVariant( QColor::fromCmykF( 0, 0.333, 0.5555, 0.46 ) );

      QTest::newRow( "color rgb float" ) << "color_rgbf(1,0.4989,0)" << false << QVariant( QColor::fromRgbF( 1., 0.4989, 0 ) );
      QTest::newRow( "color rgba float" ) << "color_rgbf(1,0.4989,0,0.21)" << false << QVariant( QColor::fromRgbF( 1., 0.4989, 0, 0.21 ) );
      QTest::newRow( "color cmyk float" ) << "color_cmykf(1,0.9012,0,0.8034)" << false << QVariant( QColor::fromCmykF( 1., 0.9012, 0, 0.8034 ) );
      QTest::newRow( "color cmyka float" ) << "color_cmykf(1,0.9012,0,0.8034,0.5069)" << false << QVariant( QColor::fromCmykF( 1.f, 0.9012, 0, 0.8034, 0.5069 ) );
      QTest::newRow( "color cmyk invalid float" ) << "color_cmykf(1.5,0.1,0,0)" << false << QVariant( QColor::fromCmykF( 1.f, 0.1, 0, 0 ) );
      QTest::newRow( "color hsl float" ) << "color_hslf(1,0.9012,0)" << false << QVariant( QColor::fromHslF( 1., 0.9012, 0 ) );
      QTest::newRow( "color hsla float" ) << "color_hslf(0.5,0.9012,0,0.8034)" << false << QVariant( QColor::fromHslF( 0.5f, 0.9012, 0, 0.8034 ) );
      QTest::newRow( "color hsl invalid float" ) << "color_hslf(1.5,0.1,0,0)" << false << QVariant( QColor::fromHslF( 1., 0.1, 0, 0 ) );
      QTest::newRow( "color hsv float" ) << "color_hsvf(1,0.9012,0)" << false << QVariant( QColor::fromHsvF( 1., 0.9012, 0 ) );
      QTest::newRow( "color hsva float" ) << "color_hsvf(0.5,0.9012,0,0.8034)" << false << QVariant( QColor::fromHsvF( 0.5f, 0.9012, 0, 0.8034 ) );
      QTest::newRow( "color hsv invalid float" ) << "color_hsvf(1.5,0.1,0,0)" << false << QVariant( QColor::fromHsvF( 1, 0.1, 0, 0 ) );

      // Precedence and associativity
      QTest::newRow( "multiplication first" ) << "1+2*3" << false << QVariant( 7 );
      QTest::newRow( "brackets first" ) << "(1+2)*(3+4)" << false << QVariant( 21 );
      QTest::newRow( "right associativity" ) << "(2^3)^2" << false << QVariant( 64. );
      QTest::newRow( "left associativity" ) << "1-(2-1)" << false << QVariant( 0 );

      // eval_template tests
      QTest::newRow( "eval_template" ) << u"eval_template(\'this is a [% \\'template\\' || \\'!\\' %]\')"_s << false << QVariant( "this is a template!" );
      QTest::newRow( "eval_template string" ) << u"eval_template('string')"_s << false << QVariant( "string" );
      QTest::newRow( "eval_template expression" ) << u"eval_template('a' || ' string')"_s << false << QVariant( "a string" );

      // layer_property tests
      QTest::newRow( "layer_property no layer" ) << "layer_property('','title')" << false << QVariant();
      QTest::newRow( "layer_property bad layer" ) << "layer_property('bad','title')" << false << QVariant();
      QTest::newRow( "layer_property no property" ) << u"layer_property('%1','')"_s.arg( mPointsLayer->name() ) << false << QVariant();
      QTest::newRow( "layer_property bad property" ) << u"layer_property('%1','bad')"_s.arg( mPointsLayer->name() ) << false << QVariant();
      QTest::newRow( "layer_property by id" ) << u"layer_property('%1','name')"_s.arg( mPointsLayer->id() ) << false << QVariant( mPointsLayer->name() );
      QTest::newRow( "layer_property name" ) << u"layer_property('%1','name')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->name() );
      QTest::newRow( "layer_property id" ) << u"layer_property('%1','id')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->id() );
      QTest::newRow( "layer_property title" ) << u"layer_property('%1','title')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->serverProperties()->title() );
      QTest::newRow( "layer_property abstract" ) << u"layer_property('%1','abstract')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->serverProperties()->abstract() );
      QTest::newRow( "layer_property keywords" ) << u"layer_property('%1','keywords')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->serverProperties()->keywordList() );
      QTest::newRow( "layer_property data_url" ) << u"layer_property('%1','data_url')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->serverProperties()->dataUrl() );
      QTest::newRow( "layer_property attribution" ) << u"layer_property('%1','attribution')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->serverProperties()->attribution() );
      QTest::newRow( "layer_property attribution_url" ) << u"layer_property('%1','attribution_url')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->serverProperties()->attributionUrl() );
      QTest::newRow( "layer_property source" ) << u"layer_property('%1','source')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->publicSource() );
      QTest::newRow( "layer_property min_scale" ) << u"layer_property('%1','min_scale')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->minimumScale() );
      QTest::newRow( "layer_property max_scale" ) << u"layer_property('%1','max_scale')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->maximumScale() );
      QTest::newRow( "layer_property is_editable" ) << u"layer_property('%1','is_editable')"_s.arg( mPointsLayer->name() ) << false << QVariant( mPointsLayer->isEditable() );
      QTest::newRow( "layer_property crs" ) << u"layer_property('%1','crs')"_s.arg( mPointsLayer->name() ) << false << QVariant( "EPSG:4326" );
      QTest::newRow( "layer_property crs_description" ) << u"layer_property('%1','crs_description')"_s.arg( mPointsLayer->name() ) << false << QVariant( "WGS 84" );
      QTest::newRow( "layer_property crs_definition" ) << u"layer_property('%1','crs_definition')"_s.arg( mPointsLayer->name() ) << false << QVariant( "+proj=longlat +datum=WGS84 +no_defs" );
      QTest::newRow( "layer_property crs_ellipsoid" ) << u"layer_property('%1','crs_ellipsoid')"_s.arg( mPointsLayer->name() ) << false << QVariant( "EPSG:7030" );
      QTest::newRow( "layer_property extent" ) << u"geom_to_wkt(layer_property('%1','extent'))"_s.arg( mPointsLayer->name() ) << false << QVariant( "Polygon ((-118.88888889 22.80020704, -83.33333333 22.80020704, -83.33333333 46.87198068, -118.88888889 46.87198068, -118.88888889 22.80020704))" );
      QTest::newRow( "layer_property distance_units" ) << u"layer_property('%1','distance_units')"_s.arg( mPointsLayer->name() ) << false << QVariant( "degrees" );
      QTest::newRow( "layer_property type" ) << u"layer_property('%1','type')"_s.arg( mPointsLayer->name() ) << false << QVariant( "Vector" );
      QTest::newRow( "layer_property storage_type" ) << u"layer_property('%1','storage_type')"_s.arg( mPointsLayer->name() ) << false << QVariant( "ESRI Shapefile" );
      QTest::newRow( "layer_property geometry_type" ) << u"layer_property('%1','geometry_type')"_s.arg( mPointsLayer->name() ) << false << QVariant( "Point" );

      QTest::newRow( "layer_property title with metadata" ) << u"layer_property('%1','title')"_s.arg( mPointsLayerMetadata->name() ) << false << QVariant( "metadata title" );
      QTest::newRow( "layer_property abstract with metadata" ) << u"layer_property('%1','abstract')"_s.arg( mPointsLayerMetadata->name() ) << false << QVariant( "metadata abstract" );
      QTest::newRow( "layer_property keywords with metadata" ) << u"array_to_string(layer_property('%1','keywords'),',')"_s.arg( mPointsLayerMetadata->name() ) << false << QVariant( "val1,val2,val3" );
      QTest::newRow( "layer_property attribution with metadata" ) << u"array_to_string(layer_property('%1','attribution'))"_s.arg( mPointsLayerMetadata->name() ) << false << QVariant( "right1,right2" );

      QTest::newRow( "decode_uri shp path" ) << u"array_last(string_to_array(replace(decode_uri('%1', 'path'), '\\\\', '/'), '/'))"_s.arg( mPointsLayer->name() ) << false << QVariant( "points.shp" );
      QTest::newRow( "layer_property path vector" ) << u"file_name(layer_property('%1','path'))"_s.arg( mPointsLayer->name() ) << false << QVariant( "points.shp" );
      QTest::newRow( "layer_property path raster" ) << u"file_name(layer_property('%1','path'))"_s.arg( mRasterLayer->id() ) << false << QVariant( "tenbytenraster.asc" );

      // Mime type
      QTest::newRow( "mime_type empty" ) << u"mime_type('')"_s << false << QVariant( "application/x-zerosize" );
      QTest::newRow( "mime_type ascii" ) << u"mime_type('TEXT')"_s << false << QVariant( "text/plain" );
      QTest::newRow( "mime_type gif" ) << u"mime_type(from_base64('R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAIAOw=='))"_s << false << QVariant( "image/gif" );
      QTest::newRow( "mime_type pdf" ) << u"mime_type(from_base64('JVBERi0xLgp0cmFpbGVyPDwvUm9vdDw8L1BhZ2VzPDwvS2lkc1s8PC9NZWRpYUJveFswIDAgMyAzXT4+XT4+Pj4+Pg=='))"_s << false << QVariant( "application/pdf" );
      QTest::newRow( "mime_type html" ) << u"mime_type('<html><body></body></html>')"_s << false << QVariant( "text/html" );

      // raster_statistic tests
      QTest::newRow( "raster_statistic no layer" ) << "raster_statistic('',1,'min')" << false << QVariant();
      QTest::newRow( "raster_statistic bad layer" ) << "raster_statistic('bad',1,'min')" << false << QVariant();
      QTest::newRow( "raster_statistic bad band" ) << u"raster_statistic('%1',0,'min')"_s.arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic bad band 2" ) << u"raster_statistic('%1',100,'min')"_s.arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic no property" ) << u"raster_statistic('%1',1,'')"_s.arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic bad property" ) << u"raster_statistic('%1',1,'bad')"_s.arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_statistic min by id" ) << u"raster_statistic('%1',1,'min')"_s.arg( mRasterLayer->id() ) << false << QVariant( 0.0 );
      QTest::newRow( "raster_statistic min name" ) << u"raster_statistic('%1',1,'min')"_s.arg( mRasterLayer->name() ) << false << QVariant( 0.0 );
      QTest::newRow( "raster_statistic max" ) << u"raster_statistic('%1',1,'max')"_s.arg( mRasterLayer->id() ) << false << QVariant( 9.0 );
      QTest::newRow( "raster_statistic avg" ) << u"round(10*raster_statistic('%1',1,'avg'))"_s.arg( mRasterLayer->id() ) << false << QVariant( 45 );
      QTest::newRow( "raster_statistic stdev" ) << u"round(100*raster_statistic('%1',1,'stdev'))"_s.arg( mRasterLayer->id() ) << false << QVariant( 287 );
      QTest::newRow( "raster_statistic range" ) << u"raster_statistic('%1',1,'range')"_s.arg( mRasterLayer->id() ) << false << QVariant( 9.0 );
      QTest::newRow( "raster_statistic sum" ) << u"round(raster_statistic('%1',1,'sum'))"_s.arg( mRasterLayer->id() ) << false << QVariant( 450 );

      // raster_value tests
      QTest::newRow( "raster_value no layer" ) << "raster_value('',1,make_point(1,1))" << true << QVariant();
      QTest::newRow( "raster_value bad layer" ) << "raster_value('bad',1,make_point(1,1))" << true << QVariant();
      QTest::newRow( "raster_value bad band" ) << u"raster_value('%1',0,make_point(1,1))"_s.arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_value bad band 2" ) << u"raster_value('%1',100,make_point(1,1))"_s.arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_value invalid geometry" ) << u"raster_value('%1',1,'invalid geom')"_s.arg( mRasterLayer->name() ) << true << QVariant();
      QTest::newRow( "raster_value valid" ) << u"raster_value('%1',1,make_point(1535390,5083270))"_s.arg( mRasterLayer->name() ) << false << QVariant( 1.0 );
      QTest::newRow( "raster_value outside extent" ) << u"raster_value('%1',1,make_point(1535370,5083250))"_s.arg( mRasterLayer->name() ) << false << QVariant();

      // Raster attributes
      QTest::newRow( "raster_attributes band 1" ) << u"map_to_html_dl(raster_attributes('%1',1,246))"_s.arg( mRasterLayerWithAttributeTable->name() ) << false << QVariant( "\n  <dl>\n    <dt>Class</dt><dd>2</dd><dt>Value</dt><dd>246</dd>\n  </dl>" );
      QTest::newRow( "raster_attributes band 1 not found value" ) << u"raster_attributes('%1',1,100000)"_s.arg( mRasterLayerWithAttributeTable->name() ) << false << QVariant();
      QTest::newRow( "raster_attributes wrong band 2" ) << u"raster_attributes('%1',2,243)"_s.arg( mRasterLayerWithAttributeTable->name() ) << true << QVariant();
      QTest::newRow( "raster_attributes no attributes" ) << u"raster_attributes('%1',1,1)"_s.arg( mRasterLayerWithAttributeTable->name() ) << false << QVariant();

      //test conversions to bool
      QTest::newRow( "feature to bool false" ) << u"case when get_feature('none','none',499) then true else false end"_s << false << QVariant( false );
      QTest::newRow( "feature to bool true" ) << u"case when get_feature('test','col1',10) then true else false end"_s << false << QVariant( true );
      QTest::newRow( "geometry to bool false" ) << u"case when geom_from_wkt('') then true else false end"_s << false << QVariant( false );
      QTest::newRow( "geometry to bool true" ) << u"case when geom_from_wkt('Point(3 4)') then true else false end"_s << false << QVariant( true );

      // is not
      QTest::newRow( "1 is (not 2)" ) << u"1 is (not 2)"_s << false << QVariant( 0 );
      QTest::newRow( "1 is not 2" ) << u"1 is not 2"_s << false << QVariant( 1 );
      QTest::newRow( "1 is  not 2" ) << u"1 is  not 2"_s << false << QVariant( 1 );

      // not like
      QTest::newRow( "'a' not like 'a%'" ) << u"'a' not like 'a%'"_s << false << QVariant( 0 );
      QTest::newRow( "'a' not  like 'a%'" ) << u"'a' not  like 'a%'"_s << false << QVariant( 0 );

      // with_variable
      QTest::newRow( "with_variable(name:='five', value:=5, expression:=@five * 2)" ) << u"with_variable(name:='five', value:=5, expression:=@five * 2)"_s << false << QVariant( 10 );
      QTest::newRow( "with_variable('nothing', NULL, COALESCE(@nothing, 'something'))" ) << u"with_variable('nothing', NULL, COALESCE(@nothing, 'something'))"_s << false << QVariant( "something" );

      // array_first, array_last
      QTest::newRow( "array_first(array('a', 'b', 'c'))" ) << u"array_first(array('a', 'b', 'c'))"_s << false << QVariant( "a" );
      QTest::newRow( "array_first(array())" ) << u"array_first(array())"_s << false << QVariant();
      QTest::newRow( "array_last(array('a', 'b', 'c'))" ) << u"array_last(array('a', 'b', 'c'))"_s << false << QVariant( "c" );
      QTest::newRow( "array_last(array())" ) << u"array_last(array())"_s << false << QVariant();

      // array_min, array_max, array_mean, array_median, array_majority, array_minority, array_sum
      QTest::newRow( "array_min('forty two')" ) << u"array_min('forty two')"_s << true << QVariant();
      QTest::newRow( "array_min(42)" ) << u"array_min(42)"_s << true << QVariant();
      QTest::newRow( "array_min(array())" ) << u"array_min(array())"_s << false << QVariant();
      QTest::newRow( "array_min(array(-1, 0, 1, 2))" ) << u"array_min(array(-1, 0, 1, 2))"_s << false << QVariant( -1 );
      QTest::newRow( "array_min(array(make_date(2020,12,11),make_date(2020,12,12),make_date(2020,12,13)))" ) << u"array_min(array(make_date(2020,12,11),make_date(2020,12,12),make_date(2020,12,13)))"_s << false << QVariant( QDate( 2020, 12, 11 ) );
      QTest::newRow( "array_max('forty two')" ) << u"array_max('forty two')"_s << true << QVariant();
      QTest::newRow( "array_max(42)" ) << u"array_max(42)"_s << true << QVariant();
      QTest::newRow( "array_max(array())" ) << u"array_max(array())"_s << false << QVariant();
      QTest::newRow( "array_max(array(-1, 0, 1, 2))" ) << u"array_max(array(-1, 0, 1, 2))"_s << false << QVariant( 2 );
      QTest::newRow( "array_max(array(make_date(2020,12,11),make_date(2020,12,12),make_date(2020,12,13)))" ) << u"array_max(array(make_date(2020,12,11),make_date(2020,12,12),make_date(2020,12,13)))"_s << false << QVariant( QDate( 2020, 12, 13 ) );
      QTest::newRow( "array_mean('forty two')" ) << u"array_mean('forty two')"_s << true << QVariant();
      QTest::newRow( "array_mean(42)" ) << u"array_mean(42)"_s << true << QVariant();
      QTest::newRow( "array_mean(array())" ) << u"array_mean(array())"_s << false << QVariant();
      QTest::newRow( "array_mean(array(0,1,7,66.6,135.4))" ) << u"array_mean(array(0,1,7,66.6,135.4))"_s << false << QVariant( 42.0 );
      QTest::newRow( "array_mean(array(0,84,'a','b','c'))" ) << u"array_mean(array(0,84,'a','b','c'))"_s << false << QVariant( 42.0 );
      QTest::newRow( "array_median('forty two')" ) << u"array_median('forty two')"_s << true << QVariant();
      QTest::newRow( "array_median(42)" ) << u"array_median(42)"_s << true << QVariant();
      QTest::newRow( "array_median(array())" ) << u"array_median(array())"_s << false << QVariant();
      QTest::newRow( "array_median(array(0,1,42,42,43))" ) << u"array_median(array(0,1,42,42,43))"_s << false << QVariant( 42 );
      QTest::newRow( "array_median(array(0,0,1,2,2,42,'a','b'))" ) << u"array_median(array(0,0,1,2,2,42,'a','b'))"_s << false << QVariant( 1.5 );
      QTest::newRow( "array_majority('forty two')" ) << u"array_majority('forty two')"_s << true << QVariant();
      QTest::newRow( "array_majority(42)" ) << u"array_majority(42)"_s << true << QVariant();
      QTest::newRow( "array_majority(array())" ) << u"array_majority(array())"_s << false << QVariant( QVariantList() );
      QTest::newRow( "array_majority(array(0,1,42,42,43), 'all')" ) << u"array_majority(array(0,1,42,42,43), 'all')"_s << false << QVariant( QVariantList() << 42 );
      QTest::newRow( "array_majority(array(0,1,43,'a','a','b'), 'all')" ) << u"array_majority(array(0,1,43,'a','a','b'), 'all')"_s << false << QVariant( QVariantList() << "a" );
      QTest::newRow( "array_majority(array(0,1,42,42,43,1)" ) << u"array_sort(array_majority(array(0,1,42,42,43,1)))"_s << false << QVariant( QVariantList() << 1 << 42 );
      QTest::newRow( "array_majority(array(0,1,42,42,43), 'any')" ) << u"array_majority(array(0,1,42,42,43), 'any')"_s << false << QVariant( 42 );
      QTest::newRow( "array_majority(array(0,1,1,2,2,42), 'median')" ) << u"array_majority(array(0,1,1,2,2,42), 'median')"_s << false << QVariant( 1.5 );
      QTest::newRow( "array_majority(array(0,1,1,2,2,42,'a','b'), 'median')" ) << u"array_majority(array(0,1,1,2,2,42,'a','b'), 'median')"_s << false << QVariant( 1.5 );
      QTest::newRow( "array_majority(array(0,1,2,42,'a','b','a'), 'median')" ) << u"array_majority(array(0,1,2,42,'a','b','a'), 'median')"_s << false << QVariant();
      QTest::newRow( "array_majority(array(0,1,42,42,43), 'real_majority')" ) << u"array_majority(array(0,1,42,42,43), 'real_majority')"_s << false << QVariant();
      QTest::newRow( "array_majority(array(0,1,42,42,43,42), 'real_majority')" ) << u"array_majority(array(0,1,42,42,43,42), 'real_majority')"_s << false << QVariant();
      QTest::newRow( "array_majority(array(0,1,42,42,43,42,42), 'real_majority')" ) << u"array_majority(array(0,1,42,42,43,42,42), 'real_majority')"_s << false << QVariant( 42 );
      QTest::newRow( "array_minority('forty two')" ) << u"array_minority('forty two')"_s << true << QVariant();
      QTest::newRow( "array_minority(42)" ) << u"array_minority(42)"_s << true << QVariant();
      QTest::newRow( "array_minority(array())" ) << u"array_minority(array())"_s << false << QVariant( QVariantList() );
      QTest::newRow( "array_minority(array(0,42,42), 'all')" ) << u"array_minority(array(0,42,42), 'all')"_s << false << QVariant( QVariantList() << 0 );
      QTest::newRow( "array_minority(array(42,42,'a'), 'all')" ) << u"array_minority(array(42,42,'a'), 'all')"_s << false << QVariant( QVariantList() << "a" );
      QTest::newRow( "array_minority(array(0,1,42,42))" ) << u"array_sort(array_minority(array(0,1,42,42)))"_s << false << QVariant( QVariantList() << 0 << 1 );
      QTest::newRow( "array_minority(array(0,42,42), 'any')" ) << u"array_minority(array(0,42,42), 'any')"_s << false << QVariant( 0 );
      QTest::newRow( "array_minority(array(1,2,3,3), 'median')" ) << u"array_minority(array(1,2,3,3), 'median')"_s << false << QVariant( 1.5 );
      QTest::newRow( "array_minority(array(1,2,3,3,'a'), 'median')" ) << u"array_minority(array(1,2,3,3,'a'), 'median')"_s << false << QVariant( 1.5 );
      QTest::newRow( "array_minority(array(1,1,3,3,'a'), 'median')" ) << u"array_minority(array(1,1,3,3,'a'), 'median')"_s << false << QVariant();
      QTest::newRow( "array_minority(array(0,1,42,42,43), 'real_minority')" ) << u"array_sort(array_minority(array(0,1,42,42,43), 'real_minority'))"_s << false << QVariant( QVariantList() << 0 << 1 << 42 << 43 );
      QTest::newRow( "array_minority(array(0,1,42,42,43,42), 'real_minority')" ) << u"array_sort(array_minority(array(0,1,42,42,43,42), 'real_minority'))"_s << false << QVariant( QVariantList() << 0 << 1 << 42 << 43 );
      QTest::newRow( "array_minority(array(0,1,42,42,43,42,42), 'real_minority')" ) << u"array_sort(array_minority(array(0,1,42,42,43,42,42), 'real_minority'))"_s << false << QVariant( QVariantList() << 0 << 1 << 43 );
      QTest::newRow( "array_sum('forty two')" ) << u"array_sum('forty two')"_s << true << QVariant();
      QTest::newRow( "array_sum(42)" ) << u"array_sum(42)"_s << true << QVariant();
      QTest::newRow( "array_sum(array())" ) << u"array_sum(array())"_s << false << QVariant();
      QTest::newRow( "array_sum(array('a','b','c'))" ) << u"array_sum(array('a','b','c'))"_s << false << QVariant();
      QTest::newRow( "array_sum(array(0,1,39.4,1.6,'a'))" ) << u"array_sum(array(0,1,39.4,1.6,'a'))"_s << false << QVariant( 42.0 );

      // file functions
      QTest::newRow( "base_file_name(5)" ) << u"base_file_name(5)"_s << false << QVariant( "5" );
      QTest::newRow( "base_file_name(NULL)" ) << u"base_file_name(NULL)"_s << false << QVariant();
      QTest::newRow( "base_file_name('/home/qgis/test.qgs')" ) << u"base_file_name('/home/qgis/test.qgs')"_s << false << QVariant( "test" );
      QTest::newRow( "base_file_name(points.shp)" ) << u"base_file_name('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( "points" );
      QTest::newRow( "base_file_name(map layer)" ) << u"base_file_name('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( "points" );
      QTest::newRow( "base_file_name(not path)" ) << u"base_file_name(make_point(1,2))"_s << true << QVariant();
      QTest::newRow( "file_exists(NULL)" ) << u"file_exists(NULL)"_s << false << QVariant();
      QTest::newRow( "file_exists('/home/qgis/test.qgs')" ) << u"file_exists('/home/qgis/test.qgs')"_s << false << QVariant( false );
      QTest::newRow( "file_exists(points.shp)" ) << u"file_exists('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( true );
      QTest::newRow( "file_exists(map layer)" ) << u"file_exists('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( true );
      QTest::newRow( "file_exists(not path)" ) << u"file_exists(make_point(1,2))"_s << true << QVariant();
      QTest::newRow( "file_name(5)" ) << u"file_name(5)"_s << false << QVariant( "5" );
      QTest::newRow( "file_name(NULL)" ) << u"file_name(NULL)"_s << false << QVariant();
      QTest::newRow( "file_name('/home/qgis/test.qgs')" ) << u"file_name('/home/qgis/test.qgs')"_s << false << QVariant( "test.qgs" );
      QTest::newRow( "file_name(points.shp)" ) << u"file_name('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( "points.shp" );
      QTest::newRow( "file_name(map layer)" ) << u"file_name('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( "points.shp" );
      QTest::newRow( "file_name(not path)" ) << u"file_name(make_point(1,2))"_s << true << QVariant();
      QTest::newRow( "file_path(5)" ) << u"file_path(5)"_s << false << QVariant( "." );
      QTest::newRow( "file_path(NULL)" ) << u"file_path(NULL)"_s << false << QVariant();
      QTest::newRow( "file_path('/home/qgis/test.qgs')" ) << u"file_path('/home/qgis/test.qgs')"_s << false << QVariant( "/home/qgis" );
      QTest::newRow( "file_path(points.shp)" ) << u"file_path('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( TEST_DATA_DIR );
      QTest::newRow( "file_path(map layer)" ) << u"file_path('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( TEST_DATA_DIR );
      QTest::newRow( "file_path(not path)" ) << u"file_path(make_point(1,2))"_s << true << QVariant();
      QTest::newRow( "file_size(5)" ) << u"file_size(5)"_s << false << QVariant( 0LL );
      QTest::newRow( "file_size(NULL)" ) << u"file_size(NULL)"_s << false << QVariant();
      QTest::newRow( "file_size('/home/qgis/test.qgs')" ) << u"file_size('/home/qgis/test.qgs')"_s << false << QVariant( 0LL );
      QTest::newRow( "file_size(points.shp)" ) << u"file_size('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( 576LL );
      QTest::newRow( "file_size(map layer)" ) << u"file_size('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( 576LL );
      QTest::newRow( "file_size(not path)" ) << u"file_size(make_point(1,2))"_s << true << QVariant();
      QTest::newRow( "file_suffix(5)" ) << u"file_suffix(5)"_s << false << QVariant( "" );
      QTest::newRow( "file_suffix(NULL)" ) << u"file_suffix(NULL)"_s << false << QVariant();
      QTest::newRow( "file_suffix('/home/qgis/test.qgs')" ) << u"file_suffix('/home/qgis/test.qgs')"_s << false << QVariant( "qgs" );
      QTest::newRow( "file_suffix(points.shp)" ) << u"file_suffix('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( "shp" );
      QTest::newRow( "file_suffix(map layer)" ) << u"file_suffix('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( "shp" );
      QTest::newRow( "file_suffix(not path)" ) << u"file_suffix(make_point(1,2))"_s << true << QVariant();
      QTest::newRow( "is_directory(5)" ) << u"is_directory(5)"_s << false << QVariant( false );
      QTest::newRow( "is_directory(NULL)" ) << u"is_directory(NULL)"_s << false << QVariant();
      QTest::newRow( "is_directory('/home/qgis/test.qgs')" ) << u"is_directory('/home/qgis/test.qgs')"_s << false << QVariant( false );
      QTest::newRow( "is_directory(points.shp)" ) << u"is_directory('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( false );
      QTest::newRow( "is_directory(valid)" ) << u"is_directory('%1')"_s.arg( TEST_DATA_DIR ) << false << QVariant( true );
      QTest::newRow( "is_directory(map layer)" ) << u"is_directory('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( false );
      QTest::newRow( "is_directory(not path)" ) << u"is_directory(make_point(1,2))"_s << true << QVariant();
      QTest::newRow( "is_file(5)" ) << u"is_file(5)"_s << false << QVariant( false );
      QTest::newRow( "is_file(NULL)" ) << u"is_file(NULL)"_s << false << QVariant();
      QTest::newRow( "is_file('/home/qgis/test.qgs')" ) << u"is_file('/home/qgis/test.qgs')"_s << false << QVariant( false );
      QTest::newRow( "is_file(points.shp)" ) << u"is_file('%1/points.shp')"_s.arg( TEST_DATA_DIR ) << false << QVariant( true );
      QTest::newRow( "is_file(valid)" ) << u"is_file('%1')"_s.arg( TEST_DATA_DIR ) << false << QVariant( false );
      QTest::newRow( "is_file(map layer)" ) << u"is_file('%1')"_s.arg( mPointsLayer->id() ) << false << QVariant( true );
      QTest::newRow( "is_file(not path)" ) << u"is_file(make_point(1,2))"_s << true << QVariant();

      // hash functions
      QTest::newRow( "md5(NULL)" ) << u"md5(NULL)"_s << false << QVariant();
      QTest::newRow( "md5('QGIS')" ) << u"md5('QGIS')"_s << false << QVariant( "57470aaa9e22adaefac7f5f342f1c6da" );
      QTest::newRow( "sha256(NULL)" ) << u"sha256(NULL)"_s << false << QVariant();
      QTest::newRow( "sha256('QGIS')" ) << u"sha256('QGIS')"_s << false << QVariant( "eb045cba7a797aaa06ac58830846e40c8e8c780bc0676d3393605fae50c05309" );
      QTest::newRow( "hash('QGIS', 'qsdf')" ) << u"hash('QGIS', 'qsdf')"_s << true << QVariant();
      QTest::newRow( "hash('QGIS', 'md4')" ) << u"hash('QGIS', 'md4')"_s << false << QVariant( "c0fc71c241cdebb6e888cbac0e2b68eb" );
      QTest::newRow( "hash('QGIS', 'md5')" ) << u"hash('QGIS', 'md5')"_s << false << QVariant( "57470aaa9e22adaefac7f5f342f1c6da" );
      QTest::newRow( "hash('QGIS', 'sha1')" ) << u"hash('QGIS', 'sha1')"_s << false << QVariant( "f87cfb2b74cdd5867db913237024e7001e62b114" );
      QTest::newRow( "hash('QGIS', 'sha224')" ) << u"hash('QGIS', 'sha224')"_s << false << QVariant( "4093a619ada631c770f44bc643ead18fb393b93d6a6af1861fcfece0" );
      QTest::newRow( "hash('QGIS', 'sha256')" ) << u"hash('QGIS', 'sha256')"_s << false << QVariant( "eb045cba7a797aaa06ac58830846e40c8e8c780bc0676d3393605fae50c05309" );
      QTest::newRow( "hash('QGIS', 'sha384')" ) << u"hash('QGIS', 'sha384')"_s << false << QVariant( "91c1de038cc3d09fdd512e99f9dd9922efadc39ed21d3922e69a4305cc25506033aee388e554b78714c8734f9cd7e610" );
      QTest::newRow( "hash('QGIS', 'sha512')" ) << u"hash('QGIS', 'sha512')"_s << false << QVariant( "c2c092f2ab743bf8edbeb6d028a745f30fc720408465ed369421f0a4e20fa5e27f0c90ad72d3f1d836eaa5d25cd39897d4cf77e19984668ef58da6e3159f18ac" );
      QTest::newRow( "hash('QGIS', 'sha3_224')" ) << u"hash('QGIS', 'sha3_224')"_s << false << QVariant( "467f49a5039e7280d5d42fd433e80d203439e338eaabd701f0d6c17d" );
      QTest::newRow( "hash('QGIS', 'sha3_256')" ) << u"hash('QGIS', 'sha3_256')"_s << false << QVariant( "540f7354b6b8a6e735f2845250f15f4f3ba4f666c55574d9e9354575de0e980f" );
      QTest::newRow( "hash('QGIS', 'sha3_384')" ) << u"hash('QGIS', 'sha3_384')"_s << false << QVariant( "96052da1e77679e9a65f60d7ead961b287977823144786386eb43647b0901fd8516fa6f1b9d243fb3f28775e6dde6107" );
      QTest::newRow( "hash('QGIS', 'sha3_512')" ) << u"hash('QGIS', 'sha3_512')"_s << false << QVariant( "900d079dc69761da113980253aa8ac0414a8bd6d09879a916228f8743707c4758051c98445d6b8945ec854ff90655005e02aceb0a2ffc6a0ebf818745d665349" );
      QTest::newRow( "hash('QGIS', 'keccak_224')" ) << u"hash('QGIS', 'keccak_224')"_s << false << QVariant( "5b0ce6acef8b0a121d4ac4f3eaa8503c799ad4e26a3392d1fb201478" );
      QTest::newRow( "hash('QGIS', 'keccak_256')" ) << u"hash('QGIS', 'keccak_256')"_s << false << QVariant( "991c520aa6815392de24087f61b2ae0fd56abbfeee4a8ca019c1011d327c577e" );
      QTest::newRow( "hash('QGIS', 'keccak_384')" ) << u"hash('QGIS', 'keccak_384')"_s << false << QVariant( "c57a3aed9d856fa04e5eeee9b62b6e027cca81ba574116d3cc1f0d48a1ef9e5886ff463ea8d0fac772ee473bf92f810d" );
      QTest::newRow( "hash('QGIS', 'keccak_512')" ) << u"hash('QGIS', 'keccak_512')"_s << false << QVariant( "6f0f751776b505e317de222508fa5d3ed7099d8f07c74fed54ccee6e7cdc6b89b4a085e309f2ee5210c942bbeb142bdfe48f84f912e0f3f41bdbf47110c2d344" );
      QTest::newRow( "to_base64 NULL" ) << u"to_base64(NULL)"_s << false << QVariant();
      QTest::newRow( "to_base64" ) << u"to_base64('QGIS')"_s << false << QVariant( "UUdJUw==" );
      QTest::newRow( "from_base64 NULL" ) << u"from_base64(NULL)"_s << false << QVariant();
      QTest::newRow( "from_base64" ) << u"from_base64('UUdJUw==')"_s << false << QVariant( QByteArray( QString( "QGIS" ).toLocal8Bit() ) );
      QTest::newRow( "uuid()" ) << u"regexp_match( uuid(), '({[a-zA-Z\\\\d]{8}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{12}})')"_s << false << QVariant( 1 );
      QTest::newRow( "$uuid alias" ) << u"regexp_match( $uuid, '({[a-zA-Z\\\\d]{8}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{12}})')"_s << false << QVariant( 1 );
      QTest::newRow( "uuid('WithBraces')" ) << u"regexp_match( uuid('WithBraces'), '({[a-zA-Z\\\\d]{8}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{12}})')"_s << false << QVariant( 1 );
      QTest::newRow( "uuid('WithoutBraces')" ) << u"regexp_match( uuid('WithoutBraces'), '([a-zA-Z\\\\d]{8}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{12})')"_s << false << QVariant( 1 );
      QTest::newRow( "uuid('Id128')" ) << u"regexp_match( uuid('Id128'), '([a-zA-Z\\\\d]{32})')"_s << false << QVariant( 1 );
      QTest::newRow( "uuid('invalid-format')" ) << u"regexp_match( uuid('invalid-format'), '({[a-zA-Z\\\\d]{8}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{4}\\\\-[a-zA-Z\\\\d]{12}})')"_s << false << QVariant( 1 );

      //exif functions
      QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
      QTest::newRow( "exif number" ) << u"exif('%1photos/0997.JPG','Exif.GPSInfo.GPSAltitude')"_s.arg( testDataDir ) << false << QVariant( 422.19101123595505 );
      QTest::newRow( "exif number from map" ) << u"exif('%1photos/0997.JPG')['Exif.GPSInfo.GPSAltitude']"_s.arg( testDataDir ) << false << QVariant( 422.19101123595505 );
      QTest::newRow( "exif date" ) << u"exif('%1photos/0997.JPG','Exif.Image.DateTime')"_s.arg( testDataDir ) << false << QVariant( QDateTime( QDate( 2018, 3, 16 ), QTime( 12, 19, 19 ) ) );
      QTest::newRow( "exif date from map" ) << u"exif('%1photos/0997.JPG')['Exif.Image.DateTime']"_s.arg( testDataDir ) << false << QVariant( QDateTime( QDate( 2018, 3, 16 ), QTime( 12, 19, 19 ) ) );
      QTest::newRow( "exif bad tag" ) << u"exif('%1photos/0997.JPG','bad tag')"_s.arg( testDataDir ) << false << QVariant();
      QTest::newRow( "exif bad file path" ) << u"exif('bad path','Exif.Image.DateTime')"_s << false << QVariant();
      QTest::newRow( "exif_geotag" ) << u"geom_to_wkt(exif_geotag('%1photos/0997.JPG'))"_s.arg( testDataDir ) << false << QVariant( "Point Z (149.27516667 -37.2305 422.19101124)" );
      QTest::newRow( "exif_geotag bad file path" ) << u"geom_to_wkt(exif_geotag('bad path'))"_s.arg( testDataDir ) << false << QVariant( "Point EMPTY" );

      // Form encoding tests
      QTest::newRow( "url_encode" ) << u"url_encode(map())"_s.arg( testDataDir ) << false << QVariant( "" );
      QTest::newRow( "url_encode" ) << u"url_encode(map('a b', 'a b', 'c &% d', 'c &% d'))"_s.arg( testDataDir ) << false << QVariant( "a%20b=a%20b&c%20%26%25%20d=c%20%26%25%20d" );
      QTest::newRow( "url_encode" ) << u"url_encode(map('a&+b', 'a and plus b', 'a=b', 'a equals b'))"_s.arg( testDataDir ) << false << QVariant( "a%26+b=a%20and%20plus%20b&a%3Db=a%20equals%20b" );

      // Between
      QTest::newRow( "between chars" ) << u"'b' between 'a' AND 'c'"_s << false << QVariant( true );
      QTest::newRow( "between chars false" ) << u"'c' between 'a' AND 'b'"_s << false << QVariant( false );
      QTest::newRow( "not between chars" ) << u"'b' not between 'a' AND 'c'"_s << false << QVariant( false );
      QTest::newRow( "not between chars true" ) << u"'a' not between 'b' AND 'c'"_s << false << QVariant( true );
      QTest::newRow( "between chars bounds" ) << u"'b' between 'b' AND 'c'"_s << false << QVariant( true );
      QTest::newRow( "not between chars bounds" ) << u"'b' not between 'b' AND 'c'"_s << false << QVariant( false );
      QTest::newRow( "between strings" ) << u"'ab' between 'aa' AND 'ac'"_s << false << QVariant( true );
      QTest::newRow( "not between strings" ) << u"'ab' not between 'aa' AND 'ac'"_s << false << QVariant( false );
      QTest::newRow( "between ints" ) << u"1 between 0 AND 2"_s << false << QVariant( true );
      QTest::newRow( "not between ints" ) << u"1 not between 0 AND 2"_s << false << QVariant( false );
      QTest::newRow( "between floats" ) << u"1.01 between 1.0 AND 1.02"_s << false << QVariant( true );
      QTest::newRow( "not between floats" ) << u"1.01 not between 1.0 AND 1.02"_s << false << QVariant( false );
      QTest::newRow( "between date" ) << u"make_date(2010,9,8) between make_date(2009,9,8) AND make_date(2011,9,8)"_s << false << QVariant( true );
      QTest::newRow( "not between date" ) << u"make_date(2010,9,8) not between make_date(2009,9,8) AND make_date(2011,9,8)"_s << false << QVariant( false );
      QTest::newRow( "between time" ) << u"make_time(10,10,10) between make_time(9,10,10) AND make_time(11,10,10)"_s << false << QVariant( true );
      QTest::newRow( "not between time" ) << u"make_time(10,10,10) not between make_time(9,10,10) AND make_time(11,10,10)"_s << false << QVariant( false );
      QTest::newRow( "between datetime" ) << u"make_datetime(9,10,10,1,1,1) between make_datetime(9,10,10,1,1,0) AND make_datetime(9,10,10,1,1,2)"_s << false << QVariant( true );
      QTest::newRow( "not between datetime" ) << u"make_datetime(9,10,10,1,1,1) not between make_datetime(9,10,10,1,1,0) AND make_datetime(9,10,10,1,1,2)"_s << false << QVariant( false );
      QTest::newRow( "between nulls" ) << u"'b' between NULL AND 'c'"_s << false << QVariant();
      QTest::newRow( "between nulls 2" ) << u"'b' between NULL AND NULL"_s << false << QVariant();
      QTest::newRow( "between nulls 3" ) << u"NULL between 'a' AND 'c'"_s << false << QVariant();
      QTest::newRow( "between nulls 4" ) << u"'b' between 'a' AND NULL"_s << false << QVariant();
      QTest::newRow( "between nulls 5" ) << u"NULL between NULL AND NULL"_s << false << QVariant();
      QTest::newRow( "not between nulls " ) << u"'c' between NULL AND 'b'"_s << false << QVariant( false );
      // Test NULL -> TRUE
      QTest::newRow( "not between nulls TRUE" ) << u"'a' not between 'b' AND NULL"_s << false << QVariant( true );
      QTest::newRow( "not between nulls TRUE 2" ) << u"'c' not between NULL AND 'b'"_s << false << QVariant( true );
      // Test NULL -> FALSE
      QTest::newRow( "between nulls FALSE" ) << u"'c' between NULL AND 'b'"_s << false << QVariant( false );
      QTest::newRow( "between nulls FALSE 2" ) << u"'a' between 'b' AND NULL"_s << false << QVariant( false );

      // CRS functions
      QTest::newRow( "crs_from_text epsg id" ) << "crs_from_text('EPSG:4326')" << false << QVariant( QgsCoordinateReferenceSystem( "EPSG:4326" ) );
      QTest::newRow( "crs_from_text invalid def" ) << "crs_from_text('dummy crs')" << true << QVariant();
      QTest::newRow( "crs_to_authid" ) << "crs_to_authid(crs_from_text('EPSG:3857'))" << false << QVariant( "EPSG:3857" );
    }

    void run_evaluation_test( QgsExpression &exp, bool evalError, QVariant &expected )
    {
      QLocale::setDefault( QLocale::c() );

      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();
      QCOMPARE( exp.hasParserError(), false );

      QVariant result = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      if ( result.userType() != expected.userType() )
      {
        qDebug() << "got type " << result.typeName() << "(" << static_cast<QMetaType::Type>( result.userType() ) << ") instead of " << expected.typeName() << "(" << static_cast<QMetaType::Type>( expected.userType() ) << ")";
      }
      //qDebug() << res.type() << " " << result.type();
      //qDebug() << "type " << res.typeName();
      QCOMPARE( exp.hasEvalError(), evalError );

      QgsExpressionContext context;

      QVERIFY( exp.prepare( &context ) );

      QMetaType::Type resultType = static_cast<QMetaType::Type>( result.userType() );
      QMetaType::Type expectedType = static_cast<QMetaType::Type>( expected.userType() );

      if ( resultType == QMetaType::Type::Int )
        resultType = QMetaType::Type::LongLong;
      if ( expectedType == QMetaType::Type::Int )
        expectedType = QMetaType::Type::LongLong;

      QCOMPARE( resultType, expectedType );
      switch ( resultType )
      {
        case QMetaType::Type::UnknownType:
          break; // nothing more to check
        case QMetaType::Type::LongLong:
          QCOMPARE( result.toLongLong(), expected.toLongLong() );
          break;
        case QMetaType::Type::Double:
          QCOMPARE( result.toDouble(), expected.toDouble() );
          break;
        case QMetaType::Type::Bool:
          QCOMPARE( result.toBool(), expected.toBool() );
          break;
        case QMetaType::Type::QString:
          QCOMPARE( result.toString(), expected.toString() );
          break;
        case QMetaType::Type::QDate:
          QCOMPARE( result.toDate(), expected.toDate() );
          break;
        case QMetaType::Type::QDateTime:
          QCOMPARE( result.toDateTime(), expected.toDateTime() );
          break;
        case QMetaType::Type::QTime:
          QCOMPARE( result.toTime(), expected.toTime() );
          break;
        case QMetaType::Type::QVariantList:
          QCOMPARE( result.toList(), expected.toList() );
          break;
        case QMetaType::Type::QByteArray:
          QCOMPARE( result.toByteArray(), expected.toByteArray() );
          break;
        case QMetaType::Type::QColor:
        {
          const QColor resultColor = result.value<QColor>();
          const QColor expectedColor = expected.value<QColor>();
          QCOMPARE( resultColor.spec(), expectedColor.spec() );
          switch ( resultColor.spec() )
          {
            case QColor::Spec::Cmyk:
              QGSCOMPARENEAR( resultColor.cyanF(), expectedColor.cyanF(), 0.001 );
              QGSCOMPARENEAR( resultColor.magentaF(), expectedColor.magentaF(), 0.001 );
              QGSCOMPARENEAR( resultColor.yellowF(), expectedColor.yellowF(), 0.001 );
              QGSCOMPARENEAR( resultColor.blackF(), expectedColor.blackF(), 0.001 );
              QGSCOMPARENEAR( resultColor.alphaF(), expectedColor.alphaF(), 0.001 );
              break;

            case QColor::Spec::Hsl:
              QGSCOMPARENEAR( resultColor.hslHueF(), expectedColor.hslHueF(), 0.001 );
              QGSCOMPARENEAR( resultColor.hslSaturationF(), expectedColor.hslSaturationF(), 0.001 );
              QGSCOMPARENEAR( resultColor.lightnessF(), expectedColor.lightnessF(), 0.001 );
              QGSCOMPARENEAR( resultColor.alphaF(), expectedColor.alphaF(), 0.001 );
              break;

            case QColor::Spec::Hsv:
              QGSCOMPARENEAR( resultColor.hsvHueF(), expectedColor.hsvHueF(), 0.001 );
              QGSCOMPARENEAR( resultColor.hsvSaturationF(), expectedColor.hsvSaturationF(), 0.001 );
              QGSCOMPARENEAR( resultColor.valueF(), expectedColor.valueF(), 0.001 );
              QGSCOMPARENEAR( resultColor.alphaF(), expectedColor.alphaF(), 0.001 );
              break;

            case QColor::Spec::ExtendedRgb:
            case QColor::Spec::Rgb:
              QGSCOMPARENEAR( resultColor.redF(), expectedColor.redF(), 0.001 );
              QGSCOMPARENEAR( resultColor.greenF(), expectedColor.greenF(), 0.001 );
              QGSCOMPARENEAR( resultColor.blueF(), expectedColor.blueF(), 0.001 );
              QGSCOMPARENEAR( resultColor.alphaF(), expectedColor.alphaF(), 0.001 );
              break;

            case QColor::Spec::Invalid:
              QVERIFY( false );
          }
          break;
        }
        default:
          if ( result.userType() == qMetaTypeId<QgsInterval>() )
          {
            QgsInterval inter = result.value<QgsInterval>();
            QgsInterval gotinter = expected.value<QgsInterval>();
            QCOMPARE( inter.seconds(), gotinter.seconds() );
          }
          else if ( result.userType() == qMetaTypeId<QgsCoordinateReferenceSystem>() )
          {
            QgsCoordinateReferenceSystem crs = result.value<QgsCoordinateReferenceSystem>();
            QgsCoordinateReferenceSystem expectedCrs = expected.value<QgsCoordinateReferenceSystem>();
            QCOMPARE( crs.authid(), expectedCrs.authid() );
          }
          else if ( result.userType() >= QMetaType::Type::User )
          {
            QFAIL( "unexpected user type" );
          }
          else
          {
            QVERIFY( false ); // should never happen
          }
          break;
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
      fields.append( QgsField( u"x1"_s ) );
      fields.append( QgsField( u"x2"_s ) );
      fields.append( QgsField( u"foo"_s, QMetaType::Type::Int ) );
      fields.append( QgsField( u"sin"_s, QMetaType::Type::Int ) );

      QgsFeature f;
      f.initAttributes( 4 );
      f.setAttribute( 2, QVariant( 20 ) );
      f.setAttribute( 3, QVariant( 10 ) );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );

      // good exp
      QgsExpression exp( u"foo + 1"_s );
      bool prepareRes = exp.prepare( &context );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp.hasEvalError(), false );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( static_cast<QMetaType::Type>( res.userType() ), QMetaType::Type::LongLong );
      QCOMPARE( res.toInt(), 21 );

      // bad exp
      QgsExpression exp2( u"bar + 1"_s );
      bool prepareRes2 = exp2.prepare( &context );
      QCOMPARE( prepareRes2, false );
      QCOMPARE( exp2.hasEvalError(), true );
      QVariant res2 = exp2.evaluate( &context );
      QCOMPARE( static_cast<QMetaType::Type>( res2.userType() ), QMetaType::Type::UnknownType );

      // Has field called sin and function
      QgsExpression exp3( u"sin"_s );
      prepareRes = exp3.prepare( &context );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp3.hasEvalError(), false );
      res = exp3.evaluate( &context );
      QCOMPARE( static_cast<QMetaType::Type>( res.userType() ), QMetaType::Type::Int );
      QCOMPARE( res.toInt(), 10 );

      QgsExpression exp4( u"sin(3.14)"_s );
      prepareRes = exp4.prepare( &context );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp4.hasEvalError(), false );
      res = exp4.evaluate( &context );
      QCOMPARE( res.toInt(), 0 );
    }

    void eval_feature_id()
    {
      QgsFeature f( 100 );
      f.setValid( true );
      // older form
      QgsExpression exp( u"$id * 2"_s );
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v = exp.evaluate( &context );
      QCOMPARE( v.toInt(), 200 );

      // newer form
      QgsExpression exp2( u"@id * 2"_s );
      v = exp.evaluate( &context );
      QCOMPARE( v.toInt(), 200 );

      QgsExpression exp3( u"feature_id(@feature)"_s );
      v = exp3.evaluate( &context );
      QCOMPARE( v.toInt(), 100 );
    }

    void eval_current_feature()
    {
      QgsFeature f( 100 );
      f.setValid( true );
      // older form
      QgsExpression exp( u"$currentfeature"_s );
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v = exp.evaluate( &context );
      QgsFeature evalFeature = v.value<QgsFeature>();
      QCOMPARE( evalFeature.id(), f.id() );

      QgsExpression expId( u"feature_id($currentfeature)"_s );
      v = expId.evaluate( &context );
      QCOMPARE( v.toInt(), f.id() );

      // newer form
      QgsExpression exp2( u"@feature"_s );
      QVERIFY( exp2.needsGeometry() );
      v = exp2.evaluate( &context );
      evalFeature = v.value<QgsFeature>();
      QCOMPARE( evalFeature.id(), f.id() );

      QgsExpression exp2Id( u"feature_id(@feature)"_s );
      v = exp2Id.evaluate( &context );
      QCOMPARE( v.toInt(), f.id() );
    }

    void eval_current_geometry()
    {
      QgsFeature featureWithGeometry( 100 );
      featureWithGeometry.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 2 ) ) );
      QgsFeature featureWithNoGeometry( 100 );

      // older form
      QgsExpression exp( u"geom_to_wkt($geometry)"_s );
      QgsExpressionContext contextWithGeometry = QgsExpressionContextUtils::createFeatureBasedContext( featureWithGeometry, QgsFields() );
      QgsExpressionContext contextWithNoGeometry = QgsExpressionContextUtils::createFeatureBasedContext( featureWithNoGeometry, QgsFields() );
      QVariant v = exp.evaluate( &contextWithGeometry );
      QCOMPARE( v.toString(), u"Point (1 2)"_s );
      v = exp.evaluate( &contextWithNoGeometry );
      QVERIFY( v.isNull() );

      // newer form
      QgsExpression exp2( u"geom_to_wkt(@geometry)"_s );
      QVERIFY( exp2.needsGeometry() );
      v = exp2.evaluate( &contextWithGeometry );
      QCOMPARE( v.toString(), u"Point (1 2)"_s );
      v = exp2.evaluate( &contextWithNoGeometry );
      QVERIFY( v.isNull() );
    }

    void eval_feature_attribute()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( u"col1"_s ) );
      fields.append( QgsField( u"second_column"_s, QMetaType::Type::Int ) );
      f.setFields( fields, true );
      f.setAttribute( u"col1"_s, u"test value"_s );
      f.setAttribute( u"second_column"_s, 5 );
      QgsExpression exp( u"attribute($currentfeature,'col1')"_s );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v = exp.evaluate( &context );
      QCOMPARE( v.toString(), QString( "test value" ) );
      QgsExpression exp2( u"attribute($currentfeature,'second'||'_column')"_s );
      v = exp2.evaluate( &context );
      QCOMPARE( v.toInt(), 5 );

      QgsExpression exp3( u"attribute()"_s );
      v = exp3.evaluate( &context );
      QVERIFY( v.isNull() );
      QVERIFY( exp3.hasEvalError() );

      QgsExpression exp4( u"attribute('a','b','c')"_s );
      v = exp4.evaluate( &context );
      QVERIFY( v.isNull() );
      QVERIFY( exp4.hasEvalError() );

      QgsExpression exp5( u"attribute('col1')"_s );
      v = exp5.evaluate( &context );
      QCOMPARE( v.toString(), QString( "test value" ) );
    }

    void eval_feature_attributes()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( u"col1"_s ) );
      fields.append( QgsField( u"second_column"_s, QMetaType::Type::Int ) );
      f.setFields( fields, true );
      f.setAttribute( u"col1"_s, u"test value"_s );
      f.setAttribute( u"second_column"_s, 5 );
      QgsExpression exp( u"attributes()['col1']"_s );
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
      QVariant v = exp.evaluate( &context );
      QCOMPARE( v.toString(), QString( "test value" ) );
      QgsExpression exp2( u"attributes()['second_column']"_s );
      v = exp2.evaluate( &context );
      QCOMPARE( v.toInt(), 5 );

      QgsExpression exp3( u"attributes($currentfeature)['col1']"_s );
      v = exp.evaluate( &context );
      QCOMPARE( v.toString(), QString( "test value" ) );
      QgsExpression exp4( u"attributes($currentfeature)['second_column']"_s );
      v = exp4.evaluate( &context );
      QCOMPARE( v.toInt(), 5 );

      QgsExpression exp5( u"attributes('a')"_s );
      v = exp5.evaluate( &context );
      QVERIFY( v.isNull() );
      QVERIFY( exp5.hasEvalError() );
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
      QTest::newRow( "get_feature 4" ) << "get_feature('test',attribute:= 'col2',value:='test2')" << true << 2;
      QTest::newRow( "get_feature 5" ) << "get_feature('test','col1',3)" << true << 3;
      QTest::newRow( "get_feature 6" ) << "get_feature('test','col2','test3')" << true << 3;
      QTest::newRow( "get_feature 7" ) << "get_feature('test',attribute:= 'col1',value:= 41)" << true << 4;
      QTest::newRow( "get_feature 8" ) << "get_feature('test','col2','test4')" << true << 4;

      //by layer id
      QTest::newRow( "get_feature 3" ) << u"get_feature('%1',attribute:= 'col1',value:= 11)"_s.arg( mMemoryLayer->id() ) << true << 2;
      QTest::newRow( "get_feature 4" ) << u"get_feature('%1','col2','test2')"_s.arg( mMemoryLayer->id() ) << true << 2;

      //no matching features
      QTest::newRow( "get_feature no match1" ) << "get_feature('test','col1',499)" << false << -1;
      QTest::newRow( "get_feature no match2" ) << "get_feature('test','col2','no match!')" << false << -1;
      //no matching layer
      QTest::newRow( "get_feature no match layer" ) << "get_feature('not a layer!','col1',10)" << false << -1;

      // get_feature_by_id
      QTest::newRow( "get_feature_by_id" ) << "get_feature_by_id('test', 1)" << true << 1;

      // multi-param
      QTest::newRow( "get_feature multi1" ) << "get_feature('test',attribute:= map('col1','11','col2','test2'))" << true << 2;
      QTest::newRow( "get_feature multi2" ) << "get_feature('test',attribute:= map('col1',3,'col2','test3'))" << true << 3;
      QTest::newRow( "get_feature multi2" ) << "get_feature('test',map('col1','41','datef',to_date('2022-09-23')))" << true << 4;

      // multi-param no match
      QTest::newRow( "get_feature no match-multi1" ) << "get_feature('test',attribute:= map('col1','col2'),value:='no match!')" << false << -1;
      QTest::newRow( "get_feature no match-multi2" ) << "get_feature('test',map('col2','10','col4','test3'))" << false << -1;
      QTest::newRow( "get_feature no match-multi2" ) << "get_feature('test',map('col1',10,'datef',to_date('2021-09-24')))" << false << -1;
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
      QCOMPARE( res.userType() == qMetaTypeId<QgsFeature>(), featureMatched );
      if ( featureMatched )
      {
        QgsFeature feat = res.value<QgsFeature>();
        QCOMPARE( feat.id(), static_cast<QgsFeatureId>( featureId ) );

        QgsExpression featureIdExp( u"feature_id(%1)"_s.arg( string ) );
        const QVariant idRes = featureIdExp.evaluate();
        QCOMPARE( idRes.toInt(), featureId );
      }
    }

    void test_sqliteFetchAndIncrement()
    {
      QTemporaryDir dir;
      QString testGpkgName = u"humanbeings.gpkg"_s;
      QFile::copy( QStringLiteral( TEST_DATA_DIR ) + '/' + testGpkgName, dir.filePath( testGpkgName ) );

      QgsExpressionContext context;
      QgsExpressionContextScope *scope = new QgsExpressionContextScope();
      scope->setVariable( u"test_database"_s, dir.filePath( testGpkgName ) );
      scope->setVariable( u"username"_s, "some_username" );
      context << scope;

      // Test database file does not exist
      QgsExpression exp1( u"sqlite_fetch_and_increment('/path/does/not/exist', 'T_KEY_OBJECT', 'T_LastUniqueId', 'T_Key', 'T_Id')"_s );

      exp1.evaluate( &context );
      QCOMPARE( exp1.hasEvalError(), true );
      const QString evalErrorString1 = exp1.evalErrorString();
      QVERIFY2( evalErrorString1.contains( "/path/does/not/exist" ), u"Path not found in %1"_s.arg( evalErrorString1 ).toUtf8().constData() );
      QVERIFY2( evalErrorString1.contains( "Error" ), u"\"Error\" not found in %1"_s.arg( evalErrorString1 ).toUtf8().constData() );

      // Test default values are not properly quoted
      QgsExpression exp2( u"sqlite_fetch_and_increment(@test_database, 'T_KEY_OBJECT', 'T_LastUniqueId', 'T_Key', 'T_Id', map('T_LastChange','date(''now'')','T_CreateDate','date(''now'')','T_User', @username))"_s );
      exp2.evaluate( &context );
      QCOMPARE( exp2.hasEvalError(), true );
      const QString evalErrorString2 = exp2.evalErrorString();
      QVERIFY2( evalErrorString2.contains( "some_username" ), u"'some_username' not found in '%1'"_s.arg( evalErrorString2 ).toUtf8().constData() );

      // Test incrementation logic
      QgsExpression exp( u"sqlite_fetch_and_increment(@test_database, 'T_KEY_OBJECT', 'T_LastUniqueId', 'T_Key', 'T_Id', map('T_LastChange','date(''now'')','T_CreateDate','date(''now'')','T_User','''me'''))"_s );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( res.toInt(), 0 );

      res = exp.evaluate( &context );
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      QCOMPARE( exp.hasEvalError(), false );

      QCOMPARE( res.toInt(), 1 );
    }

    void test_aggregate_with_variable_feature()
    {
      // this checks that a variable can be non static in a aggregate, i.e. the result will change across the fetched features
      // see https://github.com/qgis/QGIS/issues/33382
      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::layerScope( mAggregatesLayer ) );
      QgsFeature f;

      QgsFeatureIterator it = mAggregatesLayer->getFeatures();

      while ( it.nextFeature( f ) )
      {
        context.setFeature( f );
        QgsExpression exp( QString( "with_variable('my_var',\"col1\", aggregate(layer:='aggregate_layer', aggregate:='concatenate_unique', expression:=\"col2\", filter:=\"col1\"=@my_var))" ) );
        QString res = exp.evaluate( &context ).toString();
        QCOMPARE( res, f.attribute( "col2" ).toString() );

        // also test for generic aggregates
        exp = QString( "with_variable('my_var',\"col1\", sum(expression:=\"col1\", filter:=\"col1\"=@my_var))" );
        int res2 = exp.evaluate( &context ).toInt();
        QCOMPARE( res2, f.attribute( "col1" ).toInt() );
      }
    }

    void test_aggregate_with_variables()
    {
      // this checks that a variable can be non static in a aggregate, i.e. the result will change across the fetched features
      // see https://github.com/qgis/QGIS/issues/58221
      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::layerScope( mAggregatesLayer ) );
      context.lastScope()->setVariable( u"my_var"_s, u"3"_s );

      QgsExpression exp( QString( "aggregate(layer:='aggregate_layer', aggregate:='concatenate_unique', expression:=\"col2\", filter:=\"col1\"=@my_var)" ) );
      QString res = exp.evaluate( &context ).toString();
      QCOMPARE( res, u"test333"_s );

      context.lastScope()->setVariable( u"my_var"_s, u"4"_s );
      res = exp.evaluate( &context ).toString();
      QCOMPARE( res, u"test"_s );
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
      QTest::newRow( "string concatenate unique" ) << "aggregate('test','concatenate_unique',\"col2\",concatenator:=' , ')" << false << QVariant( "test1 , test2 , test3 , test4" );

      QTest::newRow( "geometry collect" ) << "geom_to_wkt(aggregate('aggregate_layer','collect',$geometry))" << false << QVariant( u"MultiPoint ((0 0),(1 0),(2 0),(3 0),(5 0),(6 0))"_s );

      QVariantList array;
      array << "test" << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << "test333" << "test4" << QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) << "test4" << "test7";
      QTest::newRow( "array aggregate" ) << "aggregate('aggregate_layer','array_agg',\"col2\")" << false << QVariant( array );

      QTest::newRow( "sub expression" ) << "aggregate('test','sum',\"col1\" * 2)" << false << QVariant( 65 * 2 );
      QTest::newRow( "bad sub expression" ) << "aggregate('test','sum',\"xcvxcv\" * 2)" << true << QVariant();

      QTest::newRow( "filter" ) << "aggregate('test','sum',\"col1\", \"col1\" <= 10)" << false << QVariant( 13 );
      QTest::newRow( "filter context" ) << "aggregate('test','sum',\"col1\", \"col1\" <= @test_var)" << false << QVariant( 13 );
      QTest::newRow( "filter named" ) << "aggregate(layer:='test',aggregate:='sum',expression:=\"col1\", filter:=\"col1\" <= 10)" << false << QVariant( 13 );
      QTest::newRow( "filter no matching" ) << "aggregate('test','sum',\"col1\", \"col1\" <= -10)" << false << QVariant( 0 );
      QTest::newRow( "filter no matching max" ) << "aggregate('test','max',\"col1\", \"col1\" > 1000000 )" << false << QVariant();

      QTest::newRow( "filter by @parent attribute" ) << "aggregate(layer:='child_layer', aggregate:='min', expression:=\"col3\", filter:=\"parent\"=attribute(@parent,'col1'))" << false << QVariant( 1 );

      // order by
      QTest::newRow( "string concatenate order by" ) << "aggregate('test','concatenate',\"col2\",concatenator:=' , ',order_by:=col1)" << false << QVariant( "test3 , test1 , test2 , test4" );
      QTest::newRow( "array agg concatenate order by" ) << "aggregate('test','array_agg',\"col2\",order_by:=col1)" << false << QVariant( QVariantList() << "test3" << "test1" << "test2" << "test4" );
    }

    void aggregate()
    {
      QgsExpressionContext context;
      QgsExpressionContextScope *scope = new QgsExpressionContextScope();
      scope->setVariable( u"test_var"_s, 10 );
      context << scope;
      QgsFeature f;
      mAggregatesLayer->getFeatures( u"col1 = 4 "_s ).nextFeature( f );
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
      if ( !string.contains( "@"_L1 ) )
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
      // if first evaluation has an eval error, so should any subsequent evaluations!
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( res, result );
    }

    void layerAggregates_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QTest::newRow( "count" ) << "count(\"col1\")" << false << QVariant( 7.0 );
      QTest::newRow( "count_distinct" ) << "count_distinct(\"col3\")" << false << QVariant( 4.0 );
      QTest::newRow( "count_missing" ) << "count_missing(\"col2\")" << false << QVariant( 2 );
      QTest::newRow( "sum" ) << "sum(\"col1\")" << false << QVariant( 42.0 );
      QTest::newRow( "minimum" ) << "minimum(\"col1\")" << false << QVariant( 1.0 );
      QTest::newRow( "maximum" ) << "maximum(\"col1\")" << false << QVariant( 19.0 );
      QTest::newRow( "mean" ) << "mean(\"col1\")" << false << QVariant( 6.0 );
      QTest::newRow( "median" ) << "median(\"col1\")" << false << QVariant( 4.0 );
      QTest::newRow( "stdev" ) << "round(stdev(\"col1\")*10000)" << false << QVariant( 61644 );
      QTest::newRow( "range" ) << "range(\"col1\")" << false << QVariant( 18.0 );
      QTest::newRow( "minority" ) << "minority(\"col3\")" << false << QVariant( 1 );
      QTest::newRow( "majority" ) << "majority(\"col3\")" << false << QVariant( 2 );
      QTest::newRow( "minority string" ) << "minority(\"col2\")" << false << QVariant( "test" );
      QTest::newRow( "majority string" ) << "majority(\"col2\")" << false << QVariant( "" );
      QTest::newRow( "q1" ) << "q1(\"col1\")" << false << QVariant( 2.5 );
      QTest::newRow( "q3" ) << "q3(\"col1\")" << false << QVariant( 6.5 );
      QTest::newRow( "iqr" ) << "iqr(\"col1\")" << false << QVariant( 4 );
      QTest::newRow( "min_length" ) << "min_length(\"col2\")" << false << QVariant( 0 );
      QTest::newRow( "max_length" ) << "max_length(\"col2\")" << false << QVariant( 7 );
      QTest::newRow( "concatenate" ) << "concatenate(\"col2\",concatenator:=',')" << false << QVariant( "test,,test333,test4,,test4,test7" );
      QTest::newRow( "concatenate with order 1" ) << "concatenate(\"col2\",concatenator:=',', order_by:=col1)" << false << QVariant( ",test4,test333,test,,test4,test7" );
      QTest::newRow( "concatenate with order 2" ) << "concatenate(\"col2\",concatenator:=',', order_by:=col2)" << false << QVariant( "test,test333,test4,test4,test7,," );
      QTest::newRow( "concatenate unique" ) << "concatenate_unique(\"col4\",concatenator:=',')" << false << QVariant( ",test,Sputnik" );
      QTest::newRow( "concatenate unique 2" ) << "concatenate_unique(\"col2\",concatenator:=',')" << false << QVariant( "test,,test333,test4,test7" );
      QTest::newRow( "concatenate unique with order" ) << "concatenate_unique(\"col2\",concatenator:=',', order_by:=col2)" << false << QVariant( "test,test333,test4,test7," );

      QTest::newRow( "array agg" ) << "array_agg(\"col2\")" << false << QVariant( QVariantList() << "test" << "" << "test333" << "test4" << "" << "test4" << "test7" );
      QTest::newRow( "array agg with order" ) << "array_agg(\"col2\",order_by:=col2)" << false << QVariant( QVariantList() << "test" << "test333" << "test4" << "test4" << "test7" << "" << "" );

      QTest::newRow( "geometry collect" ) << "geom_to_wkt(collect($geometry))" << false << QVariant( u"MultiPoint ((0 0),(1 0),(2 0),(3 0),(5 0),(6 0))"_s );
      QTest::newRow( "geometry collect with null geometry first" ) << "geom_to_wkt(collect($geometry, filter:=\"col3\"=3))" << false << QVariant( u"MultiPoint ((5 0))"_s );

      QTest::newRow( "bad expression" ) << "sum(\"xcvxcvcol1\")" << true << QVariant();
      QTest::newRow( "aggregate named" ) << "sum(expression:=\"col1\")" << false << QVariant( 42.0 );
      QTest::newRow( "string aggregate on int" ) << "max_length(\"col1\")" << true << QVariant();

      QTest::newRow( "sub expression" ) << "sum(\"col1\" * 2)" << false << QVariant( 84 );
      QTest::newRow( "bad sub expression" ) << "sum(\"xcvxcv\" * 2)" << true << QVariant();

      QTest::newRow( "filter" ) << "sum(\"col1\", NULL, \"col1\" >= 5)" << false << QVariant( 32 );
      QTest::newRow( "filter named" ) << "sum(expression:=\"col1\", filter:=\"col1\" >= 5)" << false << QVariant( 32 );
      QTest::newRow( "filter no matching" ) << "sum(expression:=\"col1\", filter:=\"col1\" <= -5)" << false << QVariant( 0 );
      QTest::newRow( "filter no matching max" ) << "maximum(\"col1\", filter:=\"col1\" <= -5)" << false << QVariant();

      QTest::newRow( "group by" ) << "sum(\"col1\", \"col3\")" << false << QVariant( 9 );
      QTest::newRow( "group by and filter" ) << "sum(\"col1\", \"col3\", \"col1\">=3)" << false << QVariant( 7 );
      QTest::newRow( "group by and filter named" ) << "sum(expression:=\"col1\", group_by:=\"col3\", filter:=\"col1\">=3)" << false << QVariant( 7 );
      QTest::newRow( "group by expression" ) << "sum(\"col1\", \"col1\" % 2)" << false << QVariant( 14 );
      QTest::newRow( "group by with null value" ) << "sum(\"col1\", \"col4\")" << false << QVariant( 8 );

      QTest::newRow( "filter by @parent attribute in generic aggregate" ) << "maximum(\"col1\", filter:=\"col1\"<attribute(@parent,'col1'))" << false << QVariant( 3 );
    }

    void maptip_display_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<QgsFeature>( "feature" );
      QTest::addColumn<QgsVectorLayer *>( "layer" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QgsFeature firstFeature = mPointsLayer->getFeature( 1 );
      QgsVectorLayer *noLayer = nullptr;

      QTest::newRow( "display not evaluated" ) << u"display_expression(@layer_id, $currentfeature, False)"_s << firstFeature << mPointsLayer << false << QVariant( "'Display expression with class = ' ||  \"Class\"" );
      QTest::newRow( "display wrong layer" ) << u"display_expression()"_s << firstFeature << noLayer << true << QVariant();
      QTest::newRow( "display wrong feature" ) << u"display_expression()"_s << QgsFeature() << mPointsLayer << true << QVariant();

      QTest::newRow( "maptip wrong feature" ) << u"maptip()"_s << QgsFeature() << mPointsLayer << true << QVariant();
      QTest::newRow( "maptip wrong layer" ) << u"maptip()"_s << firstFeature << noLayer << true << QVariant();
      QTest::newRow( "maptip not evaluated" ) << u"maptip(@layer_id, $currentfeature, False)"_s << firstFeature << mPointsLayer << false << QVariant( "Maptip with class = [% \"Class\" %]" );

      QTest::newRow( "maptip with 2 params" ) << u"maptip(@layer_id, $currentfeature)"_s << firstFeature << mPointsLayer << false << QVariant( "Maptip with class = Biplane" );
      QTest::newRow( "maptip with 1 param" ) << u"maptip($currentfeature)"_s << firstFeature << mPointsLayer << false << QVariant( "Maptip with class = Biplane" );
      QTest::newRow( "maptip with 0 param" ) << u"maptip()"_s << firstFeature << mPointsLayer << false << QVariant( "Maptip with class = Biplane" );

      QTest::newRow( "display with 2 params" ) << u"display_expression(@layer_id, $currentfeature)"_s << firstFeature << mPointsLayer << false << QVariant( "Display expression with class = Biplane" );
      QTest::newRow( "display with 1 param" ) << u"display_expression($currentfeature)"_s << firstFeature << mPointsLayer << false << QVariant( "Display expression with class = Biplane" );
      QTest::newRow( "display with 0 param" ) << u"display_expression()"_s << firstFeature << mPointsLayer << false << QVariant( "Display expression with class = Biplane" );
    }

    void maptip_display()
    {
      QFETCH( QString, string );
      QFETCH( QgsFeature, feature );
      QFETCH( QgsVectorLayer *, layer );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::globalScope() );
      context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
      if ( layer )
      {
        //layer->setDisplayExpression( u"Display expression with class = ' || Class"_s );
        context.appendScope( QgsExpressionContextUtils::layerScope( layer ) );
      }
      context.setFeature( feature );

      QgsExpression exp( string );
      exp.prepare( &context );

      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();
      QCOMPARE( exp.hasParserError(), false );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( res.toString(), result.toString() );
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
      QTest::newRow( "no layer num_selected" ) << "num_selected()" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );
      QTest::newRow( "no layer is_selected" ) << "is_selected()" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QgsVariantUtils::createNullVariant( QMetaType::Type::Bool );
      QTest::newRow( "no layer num_selected" ) << "num_selected()" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );
      QTest::newRow( "is_selected with params" ) << "is_selected('test', get_feature('test', 'col1', 10))" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QgsVariantUtils::createNullVariant( QMetaType::Type::Bool );
      QTest::newRow( "num_selected with params" ) << "num_selected('test')" << ( QgsFeatureIds() << 4 << 2 ) << QgsFeature() << noLayer << QVariant( 2 );
    }

    void constraintsValidity()
    {
      QFETCH( QString, expression );
      QFETCH( QgsFeature, feature );
      QFETCH( QgsVectorLayer *, layer );
      QFETCH( QVariant, result );

      QgsExpressionContext context;
      if ( layer )
        context.appendScope( QgsExpressionContextUtils::layerScope( layer ) );

      context.setFeature( feature );

      QgsExpression exp( expression );
      QCOMPARE( exp.parserErrorString(), QString() );
      exp.prepare( &context );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( res, result );
    }

    void constraintsValidity_data()
    {
      QTest::addColumn<QString>( "expression" );
      QTest::addColumn<QgsFeature>( "feature" );
      QTest::addColumn<QgsVectorLayer *>( "layer" );
      QTest::addColumn<QVariant>( "result" );

      QgsFeature firstFeature = mMemoryLayer->getFeature( 1 );  // hard constraint failure on col1
      QgsFeature secondFeature = mMemoryLayer->getFeature( 2 ); // all constraints valid
      QgsVectorLayer *noLayer = nullptr;

      QTest::newRow( "is_feature_valid hard failure" ) << "is_feature_valid()" << firstFeature << mMemoryLayer << QVariant( false );
      QTest::newRow( "is_feature_valid hard constraint failure" ) << "is_feature_valid(strength:='hard')" << firstFeature << mMemoryLayer << QVariant( false );
      QTest::newRow( "is_feature_valid soft constraint valid" ) << "is_feature_valid(strength:='soft')" << firstFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_feature_valid valid" ) << "is_feature_valid()" << secondFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_feature_valid hard constraint valid" ) << "is_feature_valid(strength:='hard')" << secondFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_feature_valid soft constraint valid" ) << "is_feature_valid(strength:='soft')" << secondFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_attribute_valid failure" ) << "is_attribute_valid('col1')" << firstFeature << mMemoryLayer << QVariant( false );
      QTest::newRow( "is_attribute_valid hard constraint failure" ) << "is_attribute_valid('col1',strength:='hard')" << firstFeature << mMemoryLayer << QVariant( false );
      QTest::newRow( "is_attribute_valid soft constraint valid" ) << "is_attribute_valid('col1',strength:='soft')" << firstFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_attribute_valid valid" ) << "is_attribute_valid('col1')" << secondFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_attribute_valid hard constraint valid" ) << "is_attribute_valid('col1',strength:='hard')" << secondFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_attribute_valid soft constraint valid" ) << "is_attribute_valid('col1',strength:='soft')" << secondFeature << mMemoryLayer << QVariant( true );
      QTest::newRow( "is_feature_valid no layer" ) << "is_feature_valid()" << secondFeature << noLayer << QVariant();
      QTest::newRow( "is_attribute_valid no layer" ) << "is_attribute_valid('col1')" << secondFeature << noLayer << QVariant();
      QTest::newRow( "is_attribute_valid wrong attribute name" ) << "is_attribute_valid('WRONG_NAME')" << secondFeature << mMemoryLayer << QVariant();
    }

    void layerAggregates()
    {
      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::layerScope( mAggregatesLayer ) );

      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      af1.setAttribute( u"col1"_s, 4 );
      af1.setAttribute( u"col2"_s, "test" );
      af1.setAttribute( u"col3"_s, 2 );
      af1.setAttribute( u"col4"_s, QVariant() );
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
      QTest::addColumn<QString>( "expression" );
      QTest::addColumn<QVariantMap>( "fields" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QTest::newRow( "bad relation" ) << "relation_aggregate('xxxtest','sum',\"col3\")" << QVariantMap( { { u"col1"_s, 0 } } ) << true << QVariant();
      QTest::newRow( "bad aggregate" ) << "relation_aggregate('my_rel','xxsum',\"col3\")" << QVariantMap( { { u"col1"_s, 0 } } ) << true << QVariant();
      QTest::newRow( "bad expression" ) << "relation_aggregate('my_rel','sum',\"xcvxcvcol1\")" << QVariantMap( { { u"col1"_s, 0 } } ) << true << QVariant();

      QTest::newRow( "relation aggregate 1" ) << "relation_aggregate('my_rel','sum',\"col3\")" << QVariantMap( { { u"col1"_s, 4 } } ) << false << QVariant( 5 );
      QTest::newRow( "relation aggregate by name" ) << "relation_aggregate('relation name','sum',\"col3\")" << QVariantMap( { { u"col1"_s, 4 } } ) << false << QVariant( 5 );
      QTest::newRow( "relation aggregate 2" ) << "relation_aggregate('my_rel','sum',\"col3\")" << QVariantMap( { { u"col1"_s, 3 } } ) << false << QVariant( 9 );
      QTest::newRow( "relation aggregate 2" ) << "relation_aggregate('my_rel','sum',\"col3\")" << QVariantMap( { { u"col1"_s, 6 } } ) << false << QVariant( 0 );
      QTest::newRow( "relation aggregate count 1" ) << "relation_aggregate('my_rel','count',\"col3\")" << QVariantMap( { { u"col1"_s, 4 } } ) << false << QVariant( 3 );
      QTest::newRow( "relation aggregate count 2" ) << "relation_aggregate('my_rel','count',\"col3\")" << QVariantMap( { { u"col1"_s, 3 } } ) << false << QVariant( 2 );
      QTest::newRow( "relation aggregate count 2" ) << "relation_aggregate('my_rel','count',\"col3\")" << QVariantMap( { { u"col1"_s, 6 } } ) << false << QVariant( 0 );
      QTest::newRow( "relation aggregate concatenation" ) << "relation_aggregate('my_rel','concatenate',to_string(\"col3\"),concatenator:=',')" << QVariantMap( { { u"col1"_s, 3 } } ) << false << QVariant( "2,7" );

      QTest::newRow( "relation aggregate concatenation with order" ) << "relation_aggregate('my_rel','concatenate',to_string(\"col2\"),concatenator:=',', order_by:=col2)" << QVariantMap( { { u"col1"_s, 4 } } ) << false << QVariant( "test,test333," );
      QTest::newRow( "relation aggregate concatenation with order 2" ) << "relation_aggregate('my_rel','concatenate',to_string(\"col2\"),concatenator:=',', order_by:=col3)" << QVariantMap( { { u"col1"_s, 4 } } ) << false << QVariant( ",test,test333" );

      QTest::newRow( "named relation aggregate 1" ) << "relation_aggregate(relation:='my_rel',aggregate:='sum',expression:=\"col3\")" << QVariantMap( { { u"col1"_s, 4 } } ) << false << QVariant( 5 );
      QTest::newRow( "relation aggregate sub expression 1" ) << "relation_aggregate('my_rel','sum',\"col3\" * 2)" << QVariantMap( { { u"col1"_s, 4 } } ) << false << QVariant( 10 );
      QTest::newRow( "relation aggregate bad sub expression" ) << "relation_aggregate('my_rel','sum',\"fsdfsddf\" * 2)" << QVariantMap( { { u"col1"_s, 4 } } ) << true << QVariant();

      QTest::newRow( "relation aggregate with composite keys" ) << "relation_aggregate('my_rel_composite','sum',\"my_value\")" << QVariantMap( { { u"col3"_s, 1961 }, { u"col4"_s, u"Sputnik"_s } } ) << false << QVariant( 21071969 );
    }

    void relationAggregate()
    {
      QFETCH( QString, expression );
      QFETCH( QVariantMap, fields );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpressionContext context;
      context.appendScope( QgsExpressionContextUtils::layerScope( mAggregatesLayer ) );

      QgsFeature af1( mAggregatesLayer->dataProvider()->fields(), 1 );
      QVariantMap::const_iterator it = fields.constBegin();
      while ( it != fields.constEnd() )
      {
        af1.setAttribute( it.key(), it.value() );
        ++it;
      }
      context.setFeature( af1 );

      QgsExpression exp( expression );
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
      QgsExpression exp( u"x(geometry(get_feature('%1','heading',340)))"_s.arg( mPointsLayer->id() ) );
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
      QgsExpression exp1( u"rand(1,10)"_s );
      QVariant v1 = exp1.evaluate();
      QVERIFY2( v1.toInt() <= 10, QString( "Calculated: %1 Expected <= %2" ).arg( QString::number( v1.toInt() ), QString::number( 10 ) ).toUtf8().constData() );
      QVERIFY2( v1.toInt() >= 1, QString( "Calculated: %1 Expected >= %2" ).arg( QString::number( v1.toInt() ), QString::number( 1 ) ).toUtf8().constData() );

      QgsExpression exp2( u"rand(min:=-5,max:=-5)"_s );
      QVariant v2 = exp2.evaluate();
      QCOMPARE( v2.toInt(), -5 );

      // Invalid expression since max<min
      QgsExpression exp3( u"rand(10,1)"_s );
      QVariant v3 = exp3.evaluate();
      QCOMPARE( static_cast<QMetaType::Type>( v3.userType() ), QMetaType::Type::UnknownType );

      // Supports multiple type of seeds
      QgsExpression exp4( u"rand(1,10,123)"_s );
      QVariant v4 = exp4.evaluate();
      QVERIFY2( v4.toInt() <= 10, QString( "Calculated: %1 > Expected %2" ).arg( QString::number( v4.toInt() ), QString::number( 10 ) ).toUtf8().constData() );
      QVERIFY2( v4.toInt() >= 1, QString( "Calculated: %1 < Expected %2" ).arg( QString::number( v4.toInt() ), QString::number( 1 ) ).toUtf8().constData() );
      QgsExpression exp5( u"rand(1,10,1.23)"_s );
      QVariant v5 = exp5.evaluate();
      QVERIFY2( v5.toInt() <= 10, QString( "Calculated: %1 > Expected %2" ).arg( QString::number( v5.toInt() ), QString::number( 10 ) ).toUtf8().constData() );
      QVERIFY2( v5.toInt() >= 1, QString( "Calculated: %1 < Expected %2" ).arg( QString::number( v5.toInt() ), QString::number( 1 ) ).toUtf8().constData() );
      QgsExpression exp6( u"rand(1,10,'123')"_s );
      QVariant v6 = exp6.evaluate();
      QVERIFY2( v6.toInt() <= 10, QString( "Calculated: %1 > Expected %2" ).arg( QString::number( v6.toInt() ), QString::number( 10 ) ).toUtf8().constData() );
      QVERIFY2( v6.toInt() >= 1, QString( "Calculated: %1 < Expected %2" ).arg( QString::number( v6.toInt() ), QString::number( 1 ) ).toUtf8().constData() );
      QgsExpression exp7( u"rand(1,10,'abc')"_s );
      QVariant v7 = exp7.evaluate();
      QVERIFY2( v7.toInt() <= 10, QString( "Calculated: %1 > Expected %2" ).arg( QString::number( v7.toInt() ), QString::number( 10 ) ).toUtf8().constData() );
      QVERIFY2( v7.toInt() >= 1, QString( "Calculated: %1 < Expected %2" ).arg( QString::number( v7.toInt() ), QString::number( 1 ) ).toUtf8().constData() );

      // Two calls with the same seed always return the same number
      QgsExpression exp8( u"rand(1,1000000000,1)"_s );
      QVariant v8 = exp8.evaluate();
      QCOMPARE( v8.toInt(), 546311529 );

      // Two calls with a different seed return a different number
      QgsExpression exp9( u"rand(1,100000000000,1)"_s );
      QVariant v9 = exp9.evaluate();
      QgsExpression exp10( u"rand(1,100000000000,2)"_s );
      QVariant v10 = exp10.evaluate();
      QVERIFY2( v9.toInt() != v10.toInt(), u"Calculated: %1 Expected != %2"_s.arg( QString::number( v9.toInt() ), QString::number( v10.toInt() ) ).toUtf8().constData() );
    }

    void eval_randf()
    {
      QgsExpression exp1( u"randf(1.5,9.5)"_s );
      QVariant v1 = exp1.evaluate();
      QVERIFY2( v1.toDouble() <= 9.5, u"Calculated: %1 Expected <= %2"_s.arg( QString::number( v1.toDouble() ), QString::number( 9.5 ) ).toUtf8().constData() );
      QVERIFY2( v1.toDouble() >= 1.5, u"Calculated: %1 Expected >= %2"_s.arg( QString::number( v1.toDouble() ), QString::number( 1.5 ) ).toUtf8().constData() );

      QgsExpression exp2( u"randf(min:=-0.0005,max:=-0.0005)"_s );
      QVariant v2 = exp2.evaluate();
      QVERIFY2( qgsDoubleNear( v2.toDouble(), -0.0005 ), u"Calculated: %1 != Expected %2"_s.arg( QString::number( v2.toDouble() ), QString::number( -0.0005 ) ).toUtf8().constData() );

      // Invalid expression since max<min
      QgsExpression exp3( u"randf(9.3333,1.784)"_s );
      QVariant v3 = exp3.evaluate();
      QCOMPARE( static_cast<QMetaType::Type>( v3.userType() ), QMetaType::Type::UnknownType );

      // Supports multiple type of seeds
      QgsExpression exp4( u"randf(1.5,9.5,123)"_s );
      QVariant v4 = exp4.evaluate();
      QVERIFY2( v4.toDouble() <= 9.5, u"Calculated: %1 > Expected %2"_s.arg( QString::number( v4.toDouble() ), QString::number( 9.5 ) ).toUtf8().constData() );
      QVERIFY2( v4.toDouble() >= 1.5, u"Calculated: %1 < Expected %2"_s.arg( QString::number( v4.toDouble() ), QString::number( 1.5 ) ).toUtf8().constData() );
      QgsExpression exp5( u"randf(1.5,9.5,1.23)"_s );
      QVariant v5 = exp5.evaluate();
      QVERIFY2( v5.toDouble() <= 9.5, u"Calculated: %1 > Expected %2"_s.arg( QString::number( v5.toDouble() ), QString::number( 9.5 ) ).toUtf8().constData() );
      QVERIFY2( v5.toDouble() >= 1.5, u"Calculated: %1 < Expected %2"_s.arg( QString::number( v5.toDouble() ), QString::number( 1.5 ) ).toUtf8().constData() );
      QgsExpression exp6( u"randf(1.5,9.5,'123')"_s );
      QVariant v6 = exp6.evaluate();
      QVERIFY2( v6.toDouble() <= 9.5, u"Calculated: %1 > Expected %2"_s.arg( QString::number( v6.toDouble() ), QString::number( 9.5 ) ).toUtf8().constData() );
      QVERIFY2( v6.toDouble() >= 1.5, u"Calculated: %1 < Expected %2"_s.arg( QString::number( v6.toDouble() ), QString::number( 1.5 ) ).toUtf8().constData() );
      QgsExpression exp7( u"randf(1.5,9.5,'abc')"_s );
      QVariant v7 = exp7.evaluate();
      QVERIFY2( v7.toDouble() <= 9.5, u"Calculated: %1 > Expected %2"_s.arg( QString::number( v7.toDouble() ), QString::number( 9.5 ) ).toUtf8().constData() );
      QVERIFY2( v7.toDouble() >= 1.5, u"Calculated: %1 < Expected %2"_s.arg( QString::number( v7.toDouble() ), QString::number( 1.5 ) ).toUtf8().constData() );

      // Two calls with the same seed always return the same number
      QgsExpression exp8( u"randf(seed:=1)"_s );
      QVariant v8 = exp8.evaluate();
      QVERIFY2( qgsDoubleNear( v8.toDouble(), 0.13387664401253274 ), u"Calculated: %1 != Expected %2"_s.arg( QString::number( v8.toDouble() ), QString::number( 0.13387664401253274 ) ).toUtf8().constData() );

      // Two calls with a different seed return a different number
      QgsExpression exp9( u"randf(seed:=1)"_s );
      QVariant v9 = exp9.evaluate();
      QgsExpression exp10( u"randf(seed:=2)"_s );
      QVariant v10 = exp10.evaluate();
      QVERIFY2( !qgsDoubleNear( v9.toDouble(), v10.toDouble() ), u"Calculated: %1 == Expected %2"_s.arg( QString::number( v9.toDouble() ), QString::number( v10.toDouble() ) ).toUtf8().constData() );
    }

    void referenced_columns()
    {
      QSet<QString> expectedCols;
      expectedCols << u"foo"_s << u"bar"_s << u"ppp"_s << u"qqq"_s << u"rrr"_s;
      QgsExpression exp( u"length(Bar || FOO) = 4 or foo + sqrt(bar) > 0 or case when ppp then qqq else rrr end"_s );
      QCOMPARE( exp.hasParserError(), false );
      QSet<QString> refCols = exp.referencedColumns();
      // make sure we have lower case
      QSet<QString> refColsSet;
      for ( const QString &col : refCols )
        refColsSet.insert( col.toLower() );

      QCOMPARE( refColsSet, expectedCols );

      expectedCols.clear();
      expectedCols << QgsFeatureRequest::ALL_ATTRIBUTES
                   << u"parent_col1"_s
                   << u"parent_col2"_s;
      // sub expression fields, "child_field", "child_field2" should not be included in referenced columns
      exp = QgsExpression( u"relation_aggregate(relation:=\"parent_col1\" || 'my_rel',aggregate:='sum' || \"parent_col2\", expression:=\"child_field\" * \"child_field2\")"_s );
      QCOMPARE( exp.hasParserError(), false );
      refCols = exp.referencedColumns();
      QCOMPARE( refCols, expectedCols );
    }

    void referenced_variables()
    {
      QSet<QString> expectedVars;
      expectedVars << u"foo"_s
                   << u"bar"_s
                   << u"ppp"_s
                   << u"qqq"_s
                   << u"rrr"_s
                   << u"sss"_s
                   << u"ttt"_s;
      QgsExpression exp( u"CASE WHEN intersects(@bar, $geometry) THEN @ppp ELSE @qqq * @rrr END + @foo IN (1, 2, @sss) OR @ttt"_s );
      QCOMPARE( exp.hasParserError(), false );
      QSet<QString> refVar = exp.referencedVariables();

      QCOMPARE( refVar, expectedVars );
    }

    void referenced_functions()
    {
      QSet<QString> expectedFunctions;
      expectedFunctions << u"current_value"_s
                        << u"var"_s
                        << u"intersects"_s
                        << u"$geometry"_s
                        << u"buffer"_s;

      QgsExpression exp( u"current_value( 'FIELD_NAME' ) = 'A_VALUE' AND intersects(buffer($geometry, 10), @current_geometry)"_s );
      QCOMPARE( exp.hasParserError(), false );
      QSet<QString> refVar = exp.referencedFunctions();

      QCOMPARE( refVar, expectedFunctions );
    }

    void findNodes()
    {
      QSet<QString> expectedFunctions;
      expectedFunctions << u"current_value"_s
                        << u"intersects"_s
                        << u"var"_s
                        << u"$geometry"_s
                        << u"buffer"_s;
      QgsExpression exp( u"current_value( 'FIELD_NAME' ) = 'A_VALUE' AND intersects(buffer($geometry, 10), @current_geometry)"_s );
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
      QgsExpression exp( u"attribute($currentfeature,'test')"_s );
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

      QgsPoint point( 123, 456, 789 );
      QgsPolylineXY line;
      line << QgsPointXY( 1, 1 ) << QgsPointXY( 4, 2 ) << QgsPointXY( 3, 1 );

      QgsGeometry lineGeometry = QgsGeometry::fromPolylineXY( line );
      QTest::newRow( "geom x" ) << "$x" << QgsGeometry( std::make_unique<QgsPoint>( point ) ) << false << QVariant( 123. );
      QTest::newRow( "geom y" ) << "$y" << QgsGeometry( std::make_unique<QgsPoint>( point ) ) << false << QVariant( 456. );
      QTest::newRow( "geom z" ) << "$z" << QgsGeometry( std::make_unique<QgsPoint>( point ) ) << false << QVariant( 789. );
      QTest::newRow( "geom xat" ) << "xat(-1)" << lineGeometry << false << QVariant( 3. );
      QTest::newRow( "geom yat" ) << "yat(1)" << lineGeometry << false << QVariant( 2. );
      QTest::newRow( "geom length" ) << "length($geometry)" << lineGeometry << false << QVariant( lineGeometry.length() );
      QTest::newRow( "collected geometry" ) << "geom_to_wkt(collect_geometries($geometry, $geometry))" << lineGeometry << false << QVariant( u"MultiLineString ((1 1, 4 2, 3 1),(1 1, 4 2, 3 1))"_s );
      QTest::newRow( "null geometry" ) << "$geometry" << QgsGeometry() << false << QVariant();
      QTest::newRow( "empty geometry" ) << "geom_to_wkt($geometry)" << QgsGeometry().fromWkt( u"Point()"_s ) << false << QVariant( u"Point EMPTY"_s );
    }

    void eval_geometry()
    {
      QFETCH( QString, string );
      QFETCH( QgsGeometry, geom );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsFeature f;
      QList<QgsGeometry> geometryList;
      geometryList << geom << QgsReferencedGeometry( geom, QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );

      // With standard geometry
      {
        f.setGeometry( geom );
        QgsExpression exp( string );
        QCOMPARE( exp.hasParserError(), false );
        QCOMPARE( exp.needsGeometry(), true );

        QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
        QVariant out = exp.evaluate( &context );
        QCOMPARE( exp.hasEvalError(), evalError );
        QCOMPARE( out, result );
      }

      // With referenced geometry
      {
        f.setGeometry( QgsReferencedGeometry( geom, QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) ) );
        QgsExpression exp( string );
        QCOMPARE( exp.hasParserError(), false );
        QCOMPARE( exp.needsGeometry(), true );

        QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );
        QVariant out = exp.evaluate( &context );
        QCOMPARE( exp.hasEvalError(), evalError );
        QCOMPARE( out, result );
      }
    }

    void testGeometryFromContext()
    {
      QgsExpressionContext context;

      QgsExpression exp( u"geom_to_wkt($geometry)"_s );
      QCOMPARE( exp.evaluate( &context ).toString(), QString() );

      // the $geometry function refers to the feature's geometry usually
      QgsFeature feature;
      feature.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 4 ) ) );
      context.setFeature( feature );

      QCOMPARE( exp.evaluate( &context ).toString(), u"Point (3 4)"_s );

      // the $geometry function should prefer to get the geometry directly from the context if it's available
      context.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 2 ) ) );
      QCOMPARE( exp.evaluate( &context ).toString(), u"Point (1 2)"_s );

      context.scope( 0 )->removeGeometry();
      QCOMPARE( exp.evaluate( &context ).toString(), u"Point (3 4)"_s );
    }

    void eval_geometry_calc()
    {
      QgsPolylineXY polygon_ring;
      polygon_ring << QgsPointXY( 2, 1 ) << QgsPointXY( 10, 1 ) << QgsPointXY( 10, 6 ) << QgsPointXY( 2, 6 ) << QgsPointXY( 2, 1 );
      QgsPolygonXY polygonXY;
      polygonXY << polygon_ring;
      QgsGeometry polygonGeom = QgsGeometry::fromPolygonXY( polygonXY );
      QgsFeature fPolygon;
      fPolygon.setGeometry( polygonGeom );

      QgsPolylineXY polyline;
      polyline << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 );
      QgsGeometry polylineGeom = QgsGeometry::fromPolylineXY( polyline );
      QgsFeature fPolyline;
      fPolyline.setGeometry( polylineGeom );

      QgsPolyline polylineZ;
      polylineZ << QgsPoint( 0, 0, 0 ) << QgsPoint( 3, 0, 4 );
      QgsGeometry polylineZGeom = QgsGeometry::fromPolyline( polylineZ );
      QgsFeature fPolylineZ;
      fPolylineZ.setGeometry( polylineZGeom );

      QgsPolyline polylineM;
      polylineM << QgsPoint( Qgis::WkbType::PointM, 0, 0, 0, 0 ) << QgsPoint( Qgis::WkbType::PointM, 3, 0, 0, 8 );
      QgsGeometry polylineMGeom = QgsGeometry::fromPolyline( polylineM );
      QgsFeature fPolylineM;
      fPolylineM.setGeometry( polylineMGeom );

      QgsPolyline polylineZM;
      polylineZM << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 0, 0 ) << QgsPoint( Qgis::WkbType::PointZM, 3, 0, 4, 8 );
      QgsGeometry polylineZMGeom = QgsGeometry::fromPolyline( polylineZM );
      QgsFeature fPolylineZM;
      fPolylineZM.setGeometry( polylineZMGeom );

      QgsMultiLineString mls;
      QgsLineString part;
      part.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 10, 10, 10, 10 ) << QgsPoint( Qgis::WkbType::PointZM, 20, 20, 20, 20 ) );
      mls.addGeometry( part.clone() );
      part.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 30, 30, 30, 30 ) << QgsPoint( Qgis::WkbType::PointZM, 40, 40, 40, 40 ) );
      mls.addGeometry( part.clone() );
      QgsGeometry multiStringZMGeom;
      multiStringZMGeom.set( mls.clone() );
      QgsFeature fMultiLineStringZM;
      fMultiLineStringZM.setGeometry( multiStringZMGeom );

      QgsExpressionContext context;

      QgsExpression exp1( u"$area"_s );
      context.setFeature( fPolygon );
      QVariant vArea = exp1.evaluate( &context );
      QCOMPARE( vArea.toDouble(), 40. );

      QgsExpression exp2( u"$length"_s );
      context.setFeature( fPolyline );
      QVariant vLength = exp2.evaluate( &context );
      QCOMPARE( vLength.toDouble(), 10. );

      QgsExpression exp3( u"$perimeter"_s );
      context.setFeature( fPolygon );
      QVariant vPerimeter = exp3.evaluate( &context );
      QCOMPARE( vPerimeter.toDouble(), 26. );

      QgsExpression deprecatedExpXAt( u"$x_at(1)"_s );
      QgsExpression expXAt( u"x_at(@geometry, 1)"_s );
      context.setFeature( fPolygon );
      QVariant xAt = deprecatedExpXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      xAt = expXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      context.setFeature( fPolyline );
      xAt = deprecatedExpXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      xAt = expXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );

      QgsExpression deprecatedExpXAtNeg( u"$x_at(-2)"_s );
      QgsExpression expXAtNeg( u"x_at(@geometry, -2)"_s );
      context.setFeature( fPolygon );
      xAt = deprecatedExpXAtNeg.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 2.0 );
      xAt = expXAtNeg.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 2.0 );

      QgsExpression deprecatedExpYAt( u"$y_at(2)"_s );
      QgsExpression expYAt( u"y_at(@geometry, 2)"_s );
      context.setFeature( fPolygon );
      QVariant yAt = deprecatedExpYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      yAt = expYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      QgsExpression deprecatedExpYAt2( u"$y_at(1)"_s );
      QgsExpression expYAt2( u"y_at(@geometry, 1)"_s );
      context.setFeature( fPolyline );
      yAt = deprecatedExpYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 0.0 );

      QgsExpression deprecatedExpYAtNeg( u"$y_at(-2)"_s );
      QgsExpression expYAtNeg( u"y_at(@geometry, -2)"_s );
      context.setFeature( fPolygon );
      yAt = deprecatedExpYAtNeg.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      yAt = expYAtNeg.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );

      QgsExpression deprecatedAliasexpXAt( u"x_at(1)"_s );
      context.setFeature( fPolygon );
      xAt = deprecatedAliasexpXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );
      QgsExpression deprecatedAliasexpXAt2( u"x_at(1)"_s );
      context.setFeature( fPolyline );
      xAt = deprecatedAliasexpXAt2.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 10.0 );

      QgsExpression deprecatedAliasexpYAt( u"y_at(1)"_s );
      context.setFeature( fPolygon );
      yAt = deprecatedAliasexpYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 1.0 );
      QgsExpression deprecatedAliasexpYAt2( u"y_at(1)"_s );
      context.setFeature( fPolyline );
      yAt = deprecatedAliasexpYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 0.0 );

      // with a ZM geometry
      expXAt = QgsExpression( u"x_at(@geometry, 2)"_s );
      context.setFeature( fMultiLineStringZM );
      xAt = expXAt.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 30.0 );

      QgsExpression expXAtNeg2( u"x_at(@geometry, -2)"_s );
      context.setFeature( fPolygon );
      xAt = expXAtNeg.evaluate( &context );
      QCOMPARE( xAt.toDouble(), 2.0 );

      QgsExpression expYAt3( u"y_at(@geometry, 2)"_s );
      context.setFeature( fPolygon );
      yAt = expYAt.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );
      QgsExpression expYAt4( u"y_at(@geometry, 1)"_s );
      context.setFeature( fPolyline );
      yAt = expYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 0.0 );

      // with a ZM geometry
      expYAt2 = QgsExpression( u"y_at(@geometry, 2)"_s );
      context.setFeature( fMultiLineStringZM );
      yAt = expYAt2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 30.0 );

      QgsExpression expYAtNeg2( u"y_at(@geometry, -2)"_s );
      context.setFeature( fPolygon );
      yAt = expYAtNeg2.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 6.0 );

      // test with named parameter
      QgsExpression expYAtWithI( u"x_at(@geometry, i:=1)"_s );
      context.setFeature( fPolygon );
      yAt = expYAtWithI.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 10.0 );

      QgsExpression expYAtWithVertex( u"x_at(@geometry, vertex:=1)"_s );
      context.setFeature( fPolygon );
      yAt = expYAtWithVertex.evaluate( &context );
      QCOMPARE( yAt.toDouble(), 10.0 );


      // Test z_at

      // a basic case
      QgsExpression expZAt( u"z_at(@geometry, 1)"_s );
      context.setFeature( fPolylineZ );
      QVariant zAt = expZAt.evaluate( &context );
      QCOMPARE( zAt.toDouble(), 4.0 );

      // with a negative range
      expZAt = QgsExpression( u"z_at(@geometry, -1)"_s );
      context.setFeature( fPolylineZ );
      zAt = expZAt.evaluate( &context );
      QCOMPARE( zAt.toDouble(), 4.0 );

      // with an index out of range
      expZAt = QgsExpression( u"z_at(@geometry, 3)"_s );
      // even with a no Z geometry, an eval error should be raised.
      context.setFeature( fPolyline );
      zAt = expZAt.evaluate( &context );
      QVERIFY( expZAt.hasEvalError() );

      // with a geom with no Z
      expZAt = QgsExpression( u"z_at(@geometry, 1)"_s );
      context.setFeature( fPolyline );
      zAt = expZAt.evaluate( &context );
      QVERIFY( zAt.isNull() );

      // with a geom with no Z but with M
      expZAt = u"z_at(@geometry, 1)"_s;
      context.setFeature( fPolylineM );
      zAt = expZAt.evaluate( &context );
      QVERIFY( zAt.isNull() );

      // with a multi geom
      expZAt = u"z_at(@geometry, 2)"_s;
      context.setFeature( fMultiLineStringZM );
      zAt = expZAt.evaluate( &context );
      QCOMPARE( zAt.toDouble(), 30.0 );

      // Test m_at

      // a basic case
      QgsExpression expMAt( u"m_at(@geometry, 1)"_s );
      context.setFeature( fPolylineM );
      QVariant mAt = expMAt.evaluate( &context );
      QCOMPARE( mAt.toDouble(), 8.0 );

      // with a negative range
      expMAt = QgsExpression( u"m_at(@geometry, -1)"_s );
      context.setFeature( fPolylineM );
      mAt = expMAt.evaluate( &context );
      QCOMPARE( mAt.toDouble(), 8.0 );

      // with an index out of range
      expMAt = QgsExpression( u"m_at(@geometry, 3)"_s );
      // even with a no M geometry, an eval error should be raised.
      context.setFeature( fPolyline );
      mAt = expMAt.evaluate( &context );
      QVERIFY( expMAt.hasEvalError() );

      // with a geom with no M
      expMAt = QgsExpression( u"m_at(@geometry, 1)"_s );
      context.setFeature( fPolyline );
      mAt = expMAt.evaluate( &context );
      QVERIFY( mAt.isNull() );

      // with a geom with no M but with Z
      expMAt = u"m_at(@geometry, 1)"_s;
      context.setFeature( fPolylineZ );
      mAt = expMAt.evaluate( &context );
      QVERIFY( mAt.isNull() );

      // with a multi geom
      expMAt = u"m_at(@geometry, 3)"_s;
      context.setFeature( fMultiLineStringZM );
      mAt = expMAt.evaluate( &context );
      QCOMPARE( mAt.toDouble(), 40.0 );

      QgsExpression exp4( u"bounds_width($geometry)"_s );
      context.setFeature( fPolygon );
      QVariant vBoundsWidth = exp4.evaluate( &context );
      QCOMPARE( vBoundsWidth.toDouble(), 8.0 );

      QgsExpression exp5( u"bounds_height($geometry)"_s );
      QVariant vBoundsHeight = exp5.evaluate( &context );
      QCOMPARE( vBoundsHeight.toDouble(), 5.0 );

      QgsExpression exp6( u"xmin($geometry)"_s );
      QVariant vXMin = exp6.evaluate( &context );
      QCOMPARE( vXMin.toDouble(), 2.0 );

      QgsExpression exp7( u"xmax($geometry)"_s );
      QVariant vXMax = exp7.evaluate( &context );
      QCOMPARE( vXMax.toDouble(), 10.0 );

      QgsExpression exp8( u"ymin($geometry)"_s );
      QVariant vYMin = exp8.evaluate( &context );
      QCOMPARE( vYMin.toDouble(), 1.0 );

      QgsExpression exp9( u"ymax($geometry)"_s );
      QVariant vYMax = exp9.evaluate( &context );
      QCOMPARE( vYMax.toDouble(), 6.0 );

      QgsExpression exp10( u"num_points($geometry)"_s );
      QVariant vVertices = exp10.evaluate( &context );
      QCOMPARE( vVertices.toInt(), 5 );

      context.setFeature( fPolyline );
      QgsExpression exp11( u"length($geometry)"_s );
      QVariant vLengthLine = exp11.evaluate( &context );
      QCOMPARE( vLengthLine.toDouble(), 10.0 );

      context.setFeature( fPolygon );
      QgsExpression exp12( u"area($geometry)"_s );
      QVariant vAreaPoly = exp12.evaluate( &context );
      QCOMPARE( vAreaPoly.toDouble(), 40.0 );

      QgsExpression exp13( u"perimeter($geometry)"_s );
      QVariant vPerimeterPoly = exp13.evaluate( &context );
      QCOMPARE( vPerimeterPoly.toDouble(), 26.0 );

      context.setFeature( fPolylineZ );
      QgsExpression exp14( u"length3D($geometry)"_s );
      QVariant vLengthLineZ = exp14.evaluate( &context );
      QCOMPARE( vLengthLineZ.toDouble(), 5.0 );
    }

    void geom_calculator()
    {
      //test calculations with and without geometry calculator set
      QgsDistanceArea da;
      da.setSourceCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"EPSG:3111"_s ), QgsProject::instance()->transformContext() );
      da.setEllipsoid( u"WGS84"_s );

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
      QgsExpression expArea( u"$area"_s );
      QVariant vArea = expArea.evaluate( &context );
      double expected = 1005640568.0;
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      // units should not be converted if no geometry calculator set
      expArea.setAreaUnits( Qgis::AreaUnit::SquareFeet );
      vArea = expArea.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      expArea.setAreaUnits( Qgis::AreaUnit::SquareNauticalMiles );
      vArea = expArea.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );

      // test area with geomCalculator
      QgsExpression expArea2( u"$area"_s );
      expArea2.setGeomCalculator( &da );
      vArea = expArea2.evaluate( &context );
      expected = 1005755617.819134;
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      // test unit conversion
      expArea2.setAreaUnits( Qgis::AreaUnit::SquareMeters ); //default units should be square meters
      vArea = expArea2.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      expArea2.setAreaUnits( Qgis::AreaUnit::Unknown ); //unknown units should not be converted
      vArea = expArea2.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 1.0 );
      expArea2.setAreaUnits( Qgis::AreaUnit::SquareMiles );
      expected = 388.324415;
      vArea = expArea2.evaluate( &context );
      QGSCOMPARENEAR( vArea.toDouble(), expected, 0.001 );

      // test perimeter without geomCalculator
      QgsExpression expPerimeter( u"$perimeter"_s );
      QVariant vPerimeter = expPerimeter.evaluate( &context );
      expected = 128282.086;
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      // units should not be converted if no geometry calculator set
      expPerimeter.setDistanceUnits( Qgis::DistanceUnit::Feet );
      vPerimeter = expPerimeter.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      expPerimeter.setDistanceUnits( Qgis::DistanceUnit::NauticalMiles );
      vPerimeter = expPerimeter.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );

      // test perimeter with geomCalculator
      QgsExpression expPerimeter2( u"$perimeter"_s );
      expPerimeter2.setGeomCalculator( &da );
      vPerimeter = expPerimeter2.evaluate( &context );
      expected = 128289.074;
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      // test unit conversion
      expPerimeter2.setDistanceUnits( Qgis::DistanceUnit::Meters ); //default units should be meters
      vPerimeter = expPerimeter2.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      expPerimeter2.setDistanceUnits( Qgis::DistanceUnit::Unknown ); //unknown units should not be converted
      vPerimeter = expPerimeter2.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );
      expPerimeter2.setDistanceUnits( Qgis::DistanceUnit::Feet );
      expected = 420895.9120735;
      vPerimeter = expPerimeter2.evaluate( &context );
      QGSCOMPARENEAR( vPerimeter.toDouble(), expected, 0.001 );

      // test length without geomCalculator
      QgsPolylineXY line3111;
      line3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 );
      QgsGeometry line3111G = QgsGeometry::fromPolylineXY( line3111 );
      feat.setGeometry( line3111G );
      context.setFeature( feat );

      QgsExpression expLength( u"$length"_s );
      QVariant vLength = expLength.evaluate( &context );
      expected = 26930.637;
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      // units should not be converted if no geometry calculator set
      expLength.setDistanceUnits( Qgis::DistanceUnit::Feet );
      vLength = expLength.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      expLength.setDistanceUnits( Qgis::DistanceUnit::NauticalMiles );
      vLength = expLength.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );

      // test length with geomCalculator
      QgsExpression expLength2( u"$length"_s );
      expLength2.setGeomCalculator( &da );
      vLength = expLength2.evaluate( &context );
      expected = 26932.156;
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      // test unit conversion
      expLength2.setDistanceUnits( Qgis::DistanceUnit::Meters ); //default units should be meters
      vLength = expLength2.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      expLength2.setDistanceUnits( Qgis::DistanceUnit::Unknown ); //unknown units should not be converted
      vLength = expLength2.evaluate( &context );
      QGSCOMPARENEAR( vLength.toDouble(), expected, 0.001 );
      expLength2.setDistanceUnits( Qgis::DistanceUnit::Feet );
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

      QgsExpression exp1( u"geomToWKT($geometry)"_s );
      context.setFeature( fPolyline );
      QVariant vWktLine = exp1.evaluate( &context );
      QCOMPARE( vWktLine.toString(), QString( "LineString (0 0, 10 0)" ) );

      QgsExpression exp2( u"geomToWKT($geometry)"_s );
      context.setFeature( fPolygon );
      QVariant vWktPolygon = exp2.evaluate( &context );
      QCOMPARE( vWktPolygon.toString(), QString( "Polygon ((2 1, 10 1, 10 6, 2 6, 2 1))" ) );

      QgsExpression exp3( u"geomToWKT($geometry)"_s );
      context.setFeature( fPoint );
      QVariant vWktPoint = exp3.evaluate( &context );
      QCOMPARE( vWktPoint.toString(), QString( "Point (-1.23456789 9.87654321)" ) );

      QgsExpression exp4( u"geomToWKT($geometry, 3)"_s );
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
                                         "</gml:Envelope>')"
                                      << QgsGeometry::fromRect( rect ) << false;
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

      QCOMPARE( out.userType() == qMetaTypeId<QgsGeometry>(), true );
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
      s.createFromOgcWmsCrs( u"EPSG:4326"_s );
      QgsCoordinateReferenceSystem d;
      d.createFromOgcWmsCrs( u"EPSG:3857"_s );
      QgsCoordinateTransform t( s, d, QgsProject::instance() );

      QgsGeometry tLine = QgsGeometry::fromPolylineXY( line );
      tLine.transform( t );
      QgsGeometry tPolygon = QgsGeometry::fromPolygonXY( polygon );
      tPolygon.transform( t );

      QgsGeometry oLine = QgsGeometry::fromPolylineXY( line );
      QgsGeometry oPolygon = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "transform Line" ) << "transform( geomFromWKT('" + oLine.asWkt() + "'), 'EPSG:4326', 'EPSG:3857' )" << tLine << false << false;
      QTest::newRow( "transform Polygon" ) << "transform( geomFromWKT('" + oPolygon.asWkt() + "'), 'EPSG:4326', 'EPSG:3857' )" << tPolygon << false << false;
      QTest::newRow( "transform Polygon using CRS" ) << "transform( geomFromWKT('" + oPolygon.asWkt() + "'), crs_from_text('EPSG:4326'), crs_from_text('EPSG:3857') )" << tPolygon << false << false;
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
      QCOMPARE( out.userType() == qMetaTypeId<QgsGeometry>(), true );
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
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "buffer flat cap" ) << "buffer( $geometry, 2.0, cap:='flat' )" << geom << false << true << geom.buffer( 2.0, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "buffer square cap" ) << "buffer( $geometry, 2.0, cap:='SQUARE' )" << geom << false << true << geom.buffer( 2.0, 8, Qgis::EndCapStyle::Square, Qgis::JoinStyle::Round, 2 );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "buffer bevel join" ) << "buffer( $geometry, 2.0, join:='bevel' )" << geom << false << true << geom.buffer( 2.0, 8, Qgis::EndCapStyle::Round, Qgis::JoinStyle::Bevel, 2 );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "buffer miter join" ) << "buffer( $geometry, 2.0, join:='MITER', miter_limit:=5 )" << geom << false << true << geom.buffer( 2.0, 8, Qgis::EndCapStyle::Round, Qgis::JoinStyle::Miter, 5 );

      QgsPointXY point1( 10, 20 );
      QgsPointXY point2( 30, 20 );
      QgsGeometry pnt1 = QgsGeometry::fromPointXY( point1 );
      QgsGeometry pnt2 = QgsGeometry::fromPointXY( point2 );
      QTest::newRow( "union" ) << "union( $geometry, geomFromWKT('" + pnt2.asWkt() + "') )" << pnt1 << false << true << pnt1.combine( pnt2 );

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "intersection" ) << "intersection( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << geom << false << true << QgsGeometry::fromWkt( u"POLYGON ((0 0,5 5,10 0,0 0))"_s );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "difference" ) << "difference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << geom << false << true << QgsGeometry::fromWkt( u"POLYGON ((5 5,10 10,10 0,5 5))"_s );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "symDifference" ) << "symDifference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << geom << false << true << QgsGeometry::fromWkt( u"MULTIPOLYGON(((5 5,0 0,0 10,5 5)),((5 5,10 10,10 0,5 5)))"_s );

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "convexHull simple" ) << "convexHull( $geometry )" << geom << false << true << geom.convexHull();
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "convexHull multi" ) << "convexHull( geomFromWKT('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))') )" << geom << false << false << QgsGeometry::fromWkt( u"POLYGON ((0 0,0 1,1 1,1 0,0 0))"_s );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "bounds" ) << "bounds( $geometry )" << geom << false << true << QgsGeometry::fromRect( geom.boundingBox() );

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "oriented_bbox" ) << "oriented_bbox( $geometry )" << geom << false << true << geom.orientedMinimumBoundingBox();
      geom = QgsGeometry::fromPointXY( point1 );
      QTest::newRow( "oriented_bbox_point" ) << "oriented_bbox( $geometry )" << geom << false << true << QgsGeometry::fromWkt( u"Polygon ((10 20, 10 20, 10 20, 10 20, 10 20))"_s );
      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "minimal_circle" ) << "minimal_circle( $geometry )" << geom << false << true << geom.minimalEnclosingCircle();

      geom = QgsGeometry::fromPolygonXY( polygon );
      QTest::newRow( "translate" ) << "translate( $geometry, 1, 2)" << geom << false << true << QgsGeometry::fromWkt( u"POLYGON ((1 2,11 12,11 2,1 2))"_s );
      geom = QgsGeometry::fromPolylineXY( line );
      QTest::newRow( "translate" ) << "translate( $geometry, -1, 2)" << geom << false << true << QgsGeometry::fromWkt( u"LINESTRING (-1 2, 9 12)"_s );
      geom = QgsGeometry::fromPointXY( point );
      QTest::newRow( "translate" ) << "translate( $geometry, 1, -2)" << geom << false << true << QgsGeometry::fromWkt( u"POINT(1 -2)"_s );
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

      QCOMPARE( out.userType() == qMetaTypeId<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      outGeom.normalize();
      result.normalize();
      QVERIFY( compareWkt( outGeom.asWkt(), result.asWkt() ) );
    }

    void eval_eval()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( u"col1"_s ) );
      fields.append( QgsField( u"second_column"_s, QMetaType::Type::Int ) );
      f.setFields( fields, true );
      f.setAttribute( u"col1"_s, u"test value"_s );
      f.setAttribute( u"second_column"_s, 5 );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );

      QgsExpression exp1( u"eval()"_s );
      QVariant v1 = exp1.evaluate( &context );

      QVERIFY( !v1.isValid() );

      QgsExpression exp2( u"eval('4')"_s );
      QVariant v2 = exp2.evaluate( &context );
      QCOMPARE( v2, QVariant( 4 ) );

      QgsExpression exp3( u"eval('\"second_column\" * 2')"_s );
      QVariant v3 = exp3.evaluate( &context );
      QCOMPARE( v3, QVariant( 10 ) );

      QgsExpression exp4( u"eval('\"col1\"')"_s );
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
      fields.append( QgsField( u"col1"_s ) );
      fields.append( QgsField( u"strings"_s, QMetaType::Type::QStringList, u"string[]"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      f.setFields( fields, true );
      f.setAttribute( u"col1"_s, u"test value"_s );
      QStringList array;
      array << u"one"_s << u"two"_s;
      f.setAttribute( u"strings"_s, array );

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

      // operators
      QCOMPARE( QgsExpression( "\"strings\" = array('one', 'two')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "\"strings\" = array('two', 'one')" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "\"strings\" = array('one')" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "\"strings\" = array('one', 'two', 'three')" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "\"strings\" = 'one'" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "\"strings\" = 5" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array('one', 'two') = \"strings\"" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array('two', 'one') = \"strings\"" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('one') = \"strings\"" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "array('one', 'two', 'three') = \"strings\"" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "\"strings\" <> array('one', 'two')" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "\"strings\" <> array('two', 'one')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "\"strings\" <> array('one')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "\"strings\" <> array('one', 'two', 'three')" ).evaluate( &context ), QVariant( true ) );

      QCOMPARE( QgsExpression( "array_length(\"strings\")" ).evaluate( &context ), QVariant( 2 ) );

      QCOMPARE( QgsExpression( "array_all(array(1,2,3), array(2,3))" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_all(array(1,2,3), array(1,2,3,4))" ).evaluate( &context ), QVariant( false ) );
      QCOMPARE( QgsExpression( "array_all(array(1,2,3), 1)" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array_all('string', 123)" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array_all('string', 'invalid')" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array_contains(\"strings\", 'two')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_contains(\"strings\", 'three')" ).evaluate( &context ), QVariant( false ) );

      QCOMPARE( QgsExpression( "array_count(array(1,2,1), 1)" ).evaluate( &context ), QVariant( 2 ) );
      QCOMPARE( QgsExpression( "array_count(array(1,2,1), 2)" ).evaluate( &context ), QVariant( 1 ) );
      QCOMPARE( QgsExpression( "array_count(array(1,2,1), 3)" ).evaluate( &context ), QVariant( 0 ) );

      QCOMPARE( QgsExpression( "array_find(\"strings\", 'two')" ).evaluate( &context ), QVariant( 1 ) );
      QCOMPARE( QgsExpression( "array_find(\"strings\", 'three')" ).evaluate( &context ), QVariant( -1 ) );

      QCOMPARE( QgsExpression( "array_get(\"strings\", 1)" ).evaluate( &context ), QVariant( "two" ) );
      QCOMPARE( QgsExpression( "array_get(\"strings\", 2)" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array_get(\"strings\", -1)" ).evaluate( &context ), QVariant( "two" ) );
      QCOMPARE( QgsExpression( "array_get(\"strings\", -4)" ).evaluate( &context ), QVariant() );

      QStringList appendExpected = array;
      appendExpected << u"three"_s;
      QCOMPARE( QgsExpression( "array_append(\"strings\", 'three')" ).evaluate( &context ), QVariant( appendExpected ) );

      QStringList prependExpected = array;
      prependExpected.prepend( u"zero"_s );
      QCOMPARE( QgsExpression( "array_prepend(\"strings\", 'zero')" ).evaluate( &context ), QVariant( prependExpected ) );

      QStringList insertExpected = array;
      insertExpected.insert( 1, u"one and a half"_s );
      QCOMPARE( QgsExpression( "array_insert(\"strings\", 1, 'one and a half')" ).evaluate( &context ), QVariant( insertExpected ) );

      QStringList removeAtExpected = array;
      removeAtExpected.removeAt( 0 );
      QCOMPARE( QgsExpression( "array_remove_at(\"strings\", 0)" ).evaluate( &context ), QVariant( removeAtExpected ) );
      QCOMPARE( QgsExpression( "array_remove_at(\"strings\", -2)" ).evaluate( &context ), QVariant( removeAtExpected ) );
      QCOMPARE( QgsExpression( "array_remove_at(\"strings\", -4)" ).evaluate( &context ), QVariant( array ) );
      QCOMPARE( QgsExpression( "array_remove_at(\"strings\", 4)" ).evaluate( &context ), QVariant( array ) );
      QCOMPARE( QgsExpression( "array_remove_at(\"strings\", -40)" ).evaluate( &context ), QVariant( array ) );

      QVariantList removeAllExpected;
      removeAllExpected << u"a"_s << u"b"_s << u"d"_s;
      QCOMPARE( QgsExpression( "array_remove_all(array('a', 'b', 'c', 'd', 'c'), 'c')" ).evaluate( &context ), QVariant( removeAllExpected ) );

      QVariantList prioritizeExpected;
      prioritizeExpected << 5 << 2 << 1 << 8;
      QCOMPARE( QgsExpression( "array_prioritize(array(1, 8, 2, 5), array(5, 4, 2, 1, 3, 8))" ).evaluate( &context ), QVariant( prioritizeExpected ) );

      QStringList concatExpected = array;
      concatExpected << u"a"_s << u"b"_s << u"c"_s;
      QCOMPARE( QgsExpression( "array_cat(\"strings\", array('a', 'b'), array('c'))" ).evaluate( &context ), QVariant( concatExpected ) );

      QVariantList foreachExpected;                                                                                                                              // skip-keyword-check
      foreachExpected << u"ABC"_s << u"HELLO"_s;                                                                                                                 // skip-keyword-check
      QCOMPARE( QgsExpression( "array_foreach(array:=array('abc', 'hello'), expression:=upper(@element))" ).evaluate( &context ), QVariant( foreachExpected ) ); // skip-keyword-check

      QVariantList filterExpected = QVariantList() << u"A: a"_s << u"A: d"_s;
      QCOMPARE( QgsExpression( "array_filter(array:=array('A: a', 'B: b', 'C: c', 'A: d'), expression:=substr(@element, 1, 2) = 'A:')" ).evaluate( &context ), QVariant( filterExpected ) );
      QVariantList filterExpectedLimit = QVariantList() << u"A: a"_s;
      QCOMPARE( QgsExpression( "array_filter(array:=array('A: a', 'B: b', 'C: c', 'A: d'), expression:=substr(@element, 1, 2) = 'A:', limit:=1)" ).evaluate( &context ), QVariant( filterExpectedLimit ) );

      QCOMPARE( QgsExpression( "array_intersect(array('1', '2', '3', '4'), array('4', '0', '2', '5'))" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_intersect(array('1', '2', '3', '4'), array('0', '5'))" ).evaluate( &context ), QVariant( false ) );

      QCOMPARE( QgsExpression( "array_reverse(array('Dufour','Valmiera','Chugiak','Wien','Pisa','Lyon','Essen','Nødebo','Las Palmas')) = array('Las Palmas','Nødebo','Essen','Lyon','Pisa','Wien','Chugiak','Valmiera','Dufour')" ).evaluate( &context ), QVariant( true ) );

      QCOMPARE( QgsExpression( "array_slice(array('Dufour','Valmiera','Chugiak','Brighton'),1,2) = array('Valmiera','Chugiak')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice(array('Dufour','Valmiera','Chugiak','Brighton'),-2,-1) = array('Chugiak','Brighton')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "array_slice( array(), 0, 3) = array()" ).evaluate( &context ), QVariant( true ) );

      QCOMPARE( QgsExpression( "array_sort(array('Banana','Cake','Apple'))" ).evaluate( &context ), QVariant( QVariantList() << u"Apple"_s << u"Banana"_s << u"Cake"_s ) );
      QCOMPARE( QgsExpression( "array_sort(array('Banana','Cake','Apple'),false)" ).evaluate( &context ), QVariant( QVariantList() << u"Cake"_s << u"Banana"_s << u"Apple"_s ) );
    }

    void eval_int_array()
    {
      QgsFeature f( 100 );
      QgsFields fields;
      fields.append( QgsField( u"col1"_s ) );
      fields.append( QgsField( u"ints"_s, QMetaType::Type::QVariantList, u"int[]"_s, 0, 0, QString(), QMetaType::Type::Int ) );
      f.setFields( fields, true );
      f.setAttribute( u"col1"_s, u"test value"_s );
      QVariantList array;
      array << 1 << -2;
      f.setAttribute( u"ints"_s, array );

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
      QCOMPARE( QgsExpression( "array_get(\"ints\", -1)" ).evaluate( &context ), QVariant( -2 ) );
      QCOMPARE( QgsExpression( "array_get(\"ints\", -2)" ).evaluate( &context ), QVariant( 1 ) );
      QCOMPARE( QgsExpression( "array_get(\"ints\", -3)" ).evaluate( &context ), QVariant() );

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
      QCOMPARE( QgsExpression( "array_remove_at(\"ints\", -2)" ).evaluate( &context ), QVariant( removeAtExpected ) );
      QCOMPARE( QgsExpression( "array_remove_at(\"ints\", -5)" ).evaluate( &context ), QVariant( array ) );
      QCOMPARE( QgsExpression( "array_remove_at(\"ints\", 5)" ).evaluate( &context ), QVariant( array ) );
      QCOMPARE( QgsExpression( "array_remove_at(\"ints\", -50)" ).evaluate( &context ), QVariant( array ) );

      QVariantList removeAllExpected;
      removeAllExpected << 1 << 2 << 4;
      QCOMPARE( QgsExpression( "array_remove_all(array(1, 2, 3, 4, 3), 3)" ).evaluate( &context ), QVariant( removeAllExpected ) );
      QCOMPARE( QgsExpression( "array_remove_all(array(1, 2, 3, 4, 3), '3')" ).evaluate( &context ), QVariant( removeAllExpected ) );

      QCOMPARE( QgsExpression( "array_remove_all(NULL, 3)" ).evaluate( &context ), QVariant() );
      QCOMPARE( QgsExpression( "array_remove_all(array(1, NULL, 3, NULL, 3), 3)" ).evaluate( &context ), QVariantList( { 1, QVariant(), QVariant() } ) );
      QCOMPARE( QgsExpression( "array_remove_all(array(1, NULL, 3, NULL, 3), NULL)" ).evaluate( &context ), QVariantList( { 1, 3, 3 } ) );

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

      QVariantList foreachExpected;                                                                                                           // skip-keyword-check
      foreachExpected << 10 << 20 << 40;                                                                                                      // skip-keyword-check
      QCOMPARE( QgsExpression( "array_foreach(array(1, 2, 4), @element * 10)" ).evaluate( &context ), QVariant( foreachExpected ) );          // skip-keyword-check
      QCOMPARE( QgsExpression( "array_foreach(array(10, 19, 38), @element + @counter)" ).evaluate( &context ), QVariant( foreachExpected ) ); // skip-keyword-check

      QVariantList filterExpected = QVariantList() << 1 << 2;
      QCOMPARE( QgsExpression( "array_filter(array(1, 2, 4), @element < 3)" ).evaluate( &context ), QVariant( filterExpected ) );
      QVariantList filterExpectedLimit = QVariantList() << 1;
      QCOMPARE( QgsExpression( "array_filter(array(1, 2, 4), @element < 3, 1)" ).evaluate( &context ), QVariant( filterExpectedLimit ) );

      QgsExpression badArray( u"array_get('not an array', 0)"_s );
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
      fields.append( QgsField( u"col1"_s ) );
      fields.append( QgsField( u"map"_s, QMetaType::Type::QVariantMap, u"map"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      f.setFields( fields, true );
      f.setAttribute( u"col1"_s, u"test value"_s );
      QVariantMap map;
      map[u"1"_s] = "one";
      map[u"2"_s] = "two";
      f.setAttribute( u"map"_s, map );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, QgsFields() );

      QVariantMap builderExpected;
      QCOMPARE( QgsExpression( "map()" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected[u"1"_s] = "hello";
      QCOMPARE( QgsExpression( "map('1', 'hello')" ).evaluate( &context ), QVariant( builderExpected ) );
      builderExpected[u"2"_s] = "world";
      QCOMPARE( QgsExpression( "map('1', 'hello', '2', 'world')" ).evaluate( &context ), QVariant( builderExpected ) );
      QCOMPARE( QgsExpression( "map('1', 'hello', '2', 'world', 'ignoredOddParam')" ).evaluate( &context ), QVariant( builderExpected ) );

      QCOMPARE( QgsExpression( "map_get(\"map\", '2')" ).evaluate( &context ), QVariant( "two" ) );
      QCOMPARE( QgsExpression( "map_get(\"map\", '3')" ).evaluate( &context ), QVariant() );

      QCOMPARE( QgsExpression( "map_exist(\"map\", '2')" ).evaluate( &context ), QVariant( true ) );
      QCOMPARE( QgsExpression( "map_exist(\"map\", '3')" ).evaluate( &context ), QVariant( false ) );

      QVariantMap deleteExpected = map;
      deleteExpected.remove( u"1"_s );
      QCOMPARE( QgsExpression( "map_delete(\"map\", '1')" ).evaluate( &context ), QVariant( deleteExpected ) );
      QCOMPARE( QgsExpression( "map_delete(\"map\", '3')" ).evaluate( &context ), QVariant( map ) );

      QVariantMap insertExpected = map;
      insertExpected.insert( u"3"_s, "three" );
      QCOMPARE( QgsExpression( "map_insert(\"map\", '3', 'three')" ).evaluate( &context ), QVariant( insertExpected ) );

      QVariantMap concatExpected;
      concatExpected[u"1"_s] = "one";
      concatExpected[u"2"_s] = "two";
      concatExpected[u"3"_s] = "three";
      QCOMPARE( QgsExpression( "map_concat(map('1', 'one', '2', 'overridden by next map'), map('2', 'two', '3', 'three'))" ).evaluate( &context ), QVariant( concatExpected ) );

      QCOMPARE( QgsExpression( "json_to_map('{\"1\":\"one\",\"2\":\"two\",\"3\":\"three\"}')" ).evaluate( &context ), QVariant( concatExpected ) );
      QCOMPARE( QgsExpression( "from_json('{\"1\":\"one\",\"2\":\"two\",\"3\":\"three\"}')" ).evaluate( &context ), QVariant( concatExpected ) );
      QCOMPARE( QgsExpression( "from_json('[1,2,3]')" ).evaluate( &context ), QVariant( QVariantList() << 1 << 2 << 3 ) );
      QCOMPARE( QgsExpression( "map_to_json(map('1','one','2','two','3','three'))" ).evaluate( &context ), QVariant( "{\"1\":\"one\",\"2\":\"two\",\"3\":\"three\"}" ) );
      QCOMPARE( QgsExpression( "to_json(map('1','one','2','two','3','three'))" ).evaluate( &context ), QVariant( "{\"1\":\"one\",\"2\":\"two\",\"3\":\"three\"}" ) );
      QCOMPARE( QgsExpression( "to_json(array(1,2,3))" ).evaluate( &context ), QVariant( u"[1,2,3]"_s ) );

      QCOMPARE( QgsExpression( "hstore_to_map('1=>one,2=>two,3=>three')" ).evaluate( &context ), QVariant( concatExpected ) );
      QCOMPARE( QgsExpression( "map_to_hstore(map('1','one','2','two','3','three'))" ).evaluate( &context ), QVariant( "\"1\"=>\"one\",\"2\"=>\"two\",\"3\"=>\"three\"" ) );

      QVariantMap hstoreExpected;
      hstoreExpected[u"test_quotes"_s] = "test \"quote\" symbol";
      hstoreExpected[u"test_slashes"_s] = "test \\slash symbol";
      hstoreExpected[u"test_mix"_s] = "key with value in quotation marks";
      QCOMPARE( QgsExpression( "hstore_to_map('\"test_quotes\"=>\"test \\\\\"quote\\\\\" symbol\",\"test_slashes\"=>\"test \\\\slash symbol\",test_mix=>\"key with value in quotation marks\"')" ).evaluate( &context ), QVariant( hstoreExpected ) );

      hstoreExpected.clear();
      hstoreExpected[u"1"_s] = "one";
      // if a key is missing its closing quote, the map construction process will stop and a partial map is returned
      QCOMPARE( QgsExpression( "hstore_to_map('\"1\"=>\"one\",\"2=>\"two\"')" ).evaluate( &context ), QVariant( hstoreExpected ) );

      QStringList keysExpected;
      keysExpected << u"1"_s << u"2"_s;
      QCOMPARE( QgsExpression( "map_akeys(\"map\")" ).evaluate( &context ), QVariant( keysExpected ) );

      QVariantList valuesExpected;
      valuesExpected << "one" << "two";
      QCOMPARE( QgsExpression( "map_avals(\"map\")" ).evaluate( &context ), QVariant( valuesExpected ) );

      QgsExpression badMap( u"map_get('not a map', '0')"_s );
      QCOMPARE( badMap.evaluate( &context ), QVariant() );
      QVERIFY( badMap.hasEvalError() );
      QCOMPARE( badMap.evalErrorString(), QString( "Cannot convert 'not a map' to map" ) );

      QCOMPARE( QgsExpression( u"map_prefix_keys(map('1', 'one', '2', 'two'), 'prefix-')"_s ).evaluate( &context ), QVariantMap( { { "prefix-1", "one" }, { "prefix-2", "two" } } ) );
      QCOMPARE( QgsExpression( u"map_prefix_keys(map(), 'prefix-')"_s ).evaluate( &context ), QVariantMap() );
      QCOMPARE( QgsExpression( u"map_prefix_keys([], 'prefix-')"_s ).evaluate( &context ), QVariant() );
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
      QCOMPARE( QgsExpression::quotedValue( QVariant( 5 ), QMetaType::Type::QString ), QString( "'5'" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( 5LL ) ), QString( "5" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( 5.5 ) ), QString( "5.5" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( true ) ), QString( "TRUE" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant( true ), QMetaType::Type::QString ), QString( "'true'" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant() ), QString( "NULL" ) );
      QCOMPARE( QgsExpression::quotedValue( QVariant(), QMetaType::Type::QString ), QString( "NULL" ) );
      QVariantList array = QVariantList() << QVariant( 1 ) << QVariant( "a" ) << QVariant();
      QCOMPARE( QgsExpression::quotedValue( array ), QString( "array( 1, 'a', NULL )" ) );
      QCOMPARE( QgsExpression::quotedValue( QStringList( { u"abc"_s, u"def"_s } ) ), QString( "array( 'abc', 'def' )" ) );
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

    void testTimeZoneFunctions()
    {
      QgsExpressionContext context;
      QgsExpressionContextScope *scope = new QgsExpressionContextScope();
      scope->setVariable( u"dt_with_timezone"_s, QDateTime( QDate( 2020, 1, 1 ), QTime( 1, 2, 3 ), QTimeZone( "Australia/Brisbane" ) ) );
      context.appendScope( scope );

      // get_timezone
      QCOMPARE( QgsExpression( QString( "timezone_id(get_timezone(@dt_with_timezone))" ) ).evaluate( &context ).toString(), u"Australia/Brisbane"_s );
      QCOMPARE( QgsExpression( QString( "get_timezone(NULL)" ) ).evaluate( &context ), QVariant() );
      QgsExpression invalid( QString( "get_timezone(123)" ) );
      QCOMPARE( invalid.evaluate( &context ), QVariant() );
      QCOMPARE( invalid.evalErrorString(), u"Cannot convert '123' to DateTime"_s );

      // set_timezone
      QCOMPARE( QgsExpression( QString( "set_timezone(@dt_with_timezone, timezone_from_id('Australia/Darwin'))" ) ).evaluate( &context ).toDateTime(), QDateTime( QDate( 2020, 1, 1 ), QTime( 1, 2, 3 ), QTimeZone( "Australia/Darwin" ) ) );
      QCOMPARE( QgsExpression( QString( "set_timezone(NULL, timezone_from_id('Australia/Darwin'))" ) ).evaluate( &context ), QVariant() );
      invalid = QgsExpression( QString( "set_timezone(123, timezone_from_id('Australia/Darwin'))" ) );
      QCOMPARE( invalid.evaluate( &context ), QVariant() );
      QCOMPARE( invalid.evalErrorString(), u"Cannot convert '123' to DateTime"_s );

      // convert_timezone
      QCOMPARE( QgsExpression( QString( "convert_timezone(@dt_with_timezone, timezone_from_id('Australia/Darwin'))" ) ).evaluate( &context ).toDateTime(), QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 32, 3 ), QTimeZone( "Australia/Darwin" ) ) );
      QCOMPARE( QgsExpression( QString( "convert_timezone(NULL, timezone_from_id('Australia/Darwin'))" ) ).evaluate( &context ), QVariant() );
      invalid = QgsExpression( QString( "convert_timezone(123, timezone_from_id('Australia/Darwin'))" ) );
      QCOMPARE( invalid.evaluate( &context ), QVariant() );
      QCOMPARE( invalid.evalErrorString(), u"Cannot convert '123' to DateTime"_s );
    }

    void test_expressionToLayerFieldIndex()
    {
      std::unique_ptr layer = std::make_unique<QgsVectorLayer>( u"Point"_s, u"test"_s, u"memory"_s );
      layer->dataProvider()->addAttributes( { QgsField( u"field1"_s, QMetaType::Type::QString ), QgsField( u"another FIELD"_s, QMetaType::Type::QString ) } );
      layer->updateFields();

      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "", layer.get() ), -1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "42", layer.get() ), -1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "foo", layer.get() ), -1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "\"foo bar\"", layer.get() ), -1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "sqrt(foo)", layer.get() ), -1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "foo + bar", layer.get() ), -1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "field1", layer.get() ), 0 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "FIELD1", layer.get() ), 0 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "\"field1\"", layer.get() ), 0 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "\"FIELD1\"", layer.get() ), 0 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "  (  \"field1\"   )   ", layer.get() ), 0 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "another FIELD", layer.get() ), 1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "ANOTHER field", layer.get() ), 1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "  ANOTHER field  ", layer.get() ), 1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "\"another field\"", layer.get() ), 1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "\"ANOTHER FIELD\"", layer.get() ), 1 );
      QCOMPARE( QgsExpression::expressionToLayerFieldIndex( "  (  \"ANOTHER FIELD\"   )   ", layer.get() ), 1 );
    }

    void test_quoteFieldExpression()
    {
      std::unique_ptr layer = std::make_unique<QgsVectorLayer>( u"Point"_s, u"test"_s, u"memory"_s );
      layer->dataProvider()->addAttributes( { QgsField( u"field1"_s, QMetaType::Type::QString ), QgsField( u"another FIELD"_s, QMetaType::Type::QString ) } );
      layer->updateFields();

      QCOMPARE( QgsExpression::quoteFieldExpression( QString(), layer.get() ), QString() );
      QCOMPARE( QgsExpression::quoteFieldExpression( QString(), nullptr ), QString() );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"42"_s, layer.get() ), u"42"_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"foo"_s, layer.get() ), u"foo"_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"foo"_s, nullptr ), u"foo"_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"\"foo bar\""_s, layer.get() ), u"\"foo bar\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"sqrt(foo)"_s, layer.get() ), u"sqrt(foo)"_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"foo + bar"_s, layer.get() ), u"foo + bar"_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"field1"_s, layer.get() ), u"\"field1\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"FIELD1"_s, layer.get() ), u"\"field1\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"\"field1\""_s, layer.get() ), u"\"field1\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"\"FIELD1\""_s, layer.get() ), u"\"FIELD1\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"  (  \"field1\"   )   "_s, layer.get() ), u"  (  \"field1\"   )   "_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"another FIELD"_s, layer.get() ), u"\"another FIELD\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"ANOTHER field"_s, layer.get() ), u"\"another FIELD\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"  ANOTHER field  "_s, layer.get() ), u"\"another FIELD\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"\"another field\""_s, layer.get() ), u"\"another field\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"\"ANOTHER FIELD\""_s, layer.get() ), u"\"ANOTHER FIELD\""_s );
      QCOMPARE( QgsExpression::quoteFieldExpression( u"  (  \"ANOTHER FIELD\"   )   "_s, layer.get() ), u"  (  \"ANOTHER FIELD\"   )   "_s );
    }

    void test_implicitSharing()
    {
      QgsExpression *exp = new QgsExpression( u"Pilots > 2"_s );

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
      mPointsLayer->deleteAttribute( mPointsLayer->fields().lookupField( u"Pilots"_s ) );

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
      QgsExpression nodeExpression( u"1 IN (1, 2, 3, 4)"_s );
      QgsExpression nodeExpression2( nodeExpression );
    }

    void test_columnRefUnprepared()
    {
      //test retrieving fields from feature when expression is unprepared - explicitly specified fields collection
      //should take precedence over feature's field collection

      QgsFields fields;
      fields.append( QgsField( u"f1"_s, QMetaType::Type::QString ) );

      QgsFeature f( 1 );
      f.setFields( fields );

      //also add a joined field - this will not be available in feature's field collection
      fields.append( QgsField( u"j1"_s, QMetaType::Type::QString ), Qgis::FieldOrigin::Join, 1 );

      f.setAttributes( QgsAttributes() << QVariant( "f1" ) << QVariant( "j1" ) );
      f.setValid( true );

      QgsExpression e( u"\"f1\""_s );
      QgsExpressionContext context;
      context.setFeature( f );
      context.setFields( fields );
      QVariant result = e.evaluate( &context );
      QCOMPARE( result.toString(), QString( "f1" ) );

      //test joined field
      QgsExpression e2( u"\"j1\""_s );
      result = e2.evaluate( &context );
      QCOMPARE( result.toString(), QString( "j1" ) );

      // final test - check that feature's field collection is also used when corresponding field NOT found
      // in explicitly passed field collection
      fields.append( QgsField( u"f2"_s, QMetaType::Type::QString ) );
      f.setFields( fields );
      f.setAttributes( QgsAttributes() << QVariant( "f1" ) << QVariant( "j1" ) << QVariant( "f2" ) );
      context.setFeature( f );
      QgsExpression e3( u"\"f2\""_s );
      result = e3.evaluate( &context );
      QCOMPARE( result.toString(), QString( "f2" ) );
    }

    void testExpressionUtilsMapLayerRetrieval()
    {
      QgsWeakMapLayerPointer weakPointer( mPointsLayer );
      QVariant rawPointer = QVariant::fromValue( mPointsLayer );
      const QString pointsLayerId = mPointsLayer->id();
      const QString pointsLayerName = mPointsLayer->name();

      bool testOk = false;
      auto runTest = [weakPointer, rawPointer, pointsLayerId, pointsLayerName, this, &testOk] {
        testOk = false;
        QgsExpression exp;
        // NULL value
        QgsExpressionContext context;
        Q_NOWARN_DEPRECATED_PUSH
        QgsMapLayer *res = QgsExpressionUtils::getMapLayer( QVariant(), &context, &exp );
        QVERIFY( !res );
        QVERIFY( !exp.hasEvalError() );

        // value which CANNOT be a map layer
        res = QgsExpressionUtils::getMapLayer( QVariant( 5 ), &context, &exp );
        QVERIFY( !res );
#if 0
        // TODO probably **should** raise an eval error for this situation?
        QVERIFY( exp.hasEvalError() );
#endif

        // with weak map layer pointer
        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( QVariant::fromValue( weakPointer ), &context, &exp );
        QCOMPARE( res, mPointsLayer );
        QVERIFY( !exp.hasEvalError() );

        // with raw map layer pointer
        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( rawPointer, &context, &exp );
        QCOMPARE( res, mPointsLayer );
        QVERIFY( !exp.hasEvalError() );

        // with layer id
        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( pointsLayerId, &context, &exp );
        QCOMPARE( res, mPointsLayer );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( pointsLayerId ) );
        QCOMPARE( exp.evaluate( &context ).toString(), pointsLayerId );

        // with layer name
        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( pointsLayerName, &context, &exp );
        QCOMPARE( res, mPointsLayer );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( pointsLayerName ) );
        QCOMPARE( exp.evaluate( &context ).toString(), pointsLayerId );

        // with string which is neither id or name
        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( u"xxxA"_s, &context, &exp );
        QVERIFY( !res );

        // test using layers from layer store
        QgsMapLayerStore store;
        QgsExpressionContextScope *scope = new QgsExpressionContextScope();

        context.appendScope( scope );

        QgsVectorLayer *layer1 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:string&field=datef:date(0,0)"_s, u"test_store_1"_s, u"memory"_s );
        QgsVectorLayer *layer2 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:string&field=datef:date(0,0)"_s, u"test_store_2"_s, u"memory"_s );
        store.addMapLayer( layer1 );
        store.addMapLayer( layer2 );
        scope->addLayerStore( &store );

        // from layer store, by layer id
        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( layer1->id(), &context, &exp );
        QCOMPARE( res, layer1 );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( layer1->id() ) );
        QCOMPARE( exp.evaluate( &context ).toString(), layer1->id() );

        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( layer2->id(), &context, &exp );
        QCOMPARE( res, layer2 );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( layer2->id() ) );
        QCOMPARE( exp.evaluate( &context ).toString(), layer2->id() );

        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( pointsLayerId, &context, &exp );
        QCOMPARE( res, mPointsLayer );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( pointsLayerId ) );
        QCOMPARE( exp.evaluate( &context ).toString(), pointsLayerId );

        // with a second store in a different scope
        QgsExpressionContextScope *scope2 = new QgsExpressionContextScope();

        context.appendScope( scope2 );

        QgsMapLayerStore store2;
        QgsVectorLayer *layer3 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:string&field=datef:date(0,0)"_s, u"test_store_3"_s, u"memory"_s );
        store2.addMapLayer( layer3 );
        scope2->addLayerStore( &store2 );

        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( layer3->id(), &context, &exp );
        QCOMPARE( res, layer3 );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( layer3->id() ) );
        QCOMPARE( exp.evaluate( &context ).toString(), layer3->id() );


        // from layer store, by name

        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( layer1->name(), &context, &exp );
        QCOMPARE( res, layer1 );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( layer1->name() ) );
        QCOMPARE( exp.evaluate( &context ).toString(), layer1->id() );

        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( layer2->name(), &context, &exp );
        QCOMPARE( res, layer2 );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( layer2->name() ) );
        QCOMPARE( exp.evaluate( &context ).toString(), layer2->id() );

        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( layer3->name(), &context, &exp );
        QCOMPARE( res, layer3 );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( layer3->name() ) );
        QCOMPARE( exp.evaluate( &context ).toString(), layer3->id() );

        exp = QgsExpression();
        res = QgsExpressionUtils::getMapLayer( pointsLayerName, &context, &exp );
        QCOMPARE( res, mPointsLayer );
        QVERIFY( !exp.hasEvalError() );

        exp = QgsExpression( u"layer_property('%1', 'id')"_s.arg( pointsLayerName ) );
        QCOMPARE( exp.evaluate( &context ).toString(), pointsLayerId );

#if 0
        // TODO -- probably should flag an error here?
        QVERIFY( !exp.hasEvalError() );
#endif
        Q_NOWARN_DEPRECATED_POP

        testOk = true;
      };

      runTest();
      QVERIFY( testOk );
      testOk = false;

      // also run in a thread

      QThread *thread = new RunLambdaInThread( runTest );
      connect( thread, &QThread::finished, thread, &QObject::deleteLater );
      QSignalSpy spy( thread, &QThread::finished );
      thread->start();
      spy.wait();
      QVERIFY( testOk );
    }

    void testExpressionUtilsMapLayerExecuteLambda()
    {
      QgsWeakMapLayerPointer weakPointer( mPointsLayer );
      QVariant rawPointer = QVariant::fromValue( mPointsLayer );
      const QString pointsLayerId = mPointsLayer->id();
      const QString pointsLayerName = mPointsLayer->name();

      bool testOk = false;
      auto runTest = [weakPointer, rawPointer, pointsLayerId, pointsLayerName, &testOk] {
        testOk = false;
        QString gotLayerId;

        auto lambda = [&gotLayerId]( QgsMapLayer *layer ) {
          if ( layer )
            gotLayerId = layer->id();
          else
            gotLayerId.clear();
        };

        QgsExpression exp;
        // NULL value
        QgsExpressionContext context;

        bool foundLayer = false;
        QgsExpressionUtils::executeLambdaForMapLayer( QVariant(), &context, &exp, lambda, foundLayer );
        QVERIFY( !foundLayer );
        QVERIFY( gotLayerId.isEmpty() );

        // value which CANNOT be a map layer
        QgsExpressionUtils::executeLambdaForMapLayer( QVariant( 5 ), &context, &exp, lambda, foundLayer );
        QVERIFY( !foundLayer );
        QVERIFY( gotLayerId.isEmpty() );

        // with weak map layer pointer
        QgsExpressionUtils::executeLambdaForMapLayer( QVariant::fromValue( weakPointer ), &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, pointsLayerId );

        // with raw map layer pointer
        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( rawPointer, &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, pointsLayerId );

        // with layer id
        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( pointsLayerId, &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, pointsLayerId );

        // with layer name
        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( pointsLayerName, &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, pointsLayerId );

        // with string which is neither id or name
        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( u"xxxA"_s, &context, &exp, lambda, foundLayer );
        QVERIFY( !foundLayer );
        QVERIFY( gotLayerId.isEmpty() );

        // test using layers from layer store
        QgsMapLayerStore store;
        QgsExpressionContextScope *scope = new QgsExpressionContextScope();

        context.appendScope( scope );

        QgsVectorLayer *layer1 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:string&field=datef:date(0,0)"_s, u"test_store_1"_s, u"memory"_s );
        QgsVectorLayer *layer2 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:string&field=datef:date(0,0)"_s, u"test_store_2"_s, u"memory"_s );
        store.addMapLayer( layer1 );
        store.addMapLayer( layer2 );
        scope->addLayerStore( &store );

        // from layer store, by layer id
        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( layer1->id(), &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, layer1->id() );

        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( layer2->id(), &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, layer2->id() );

        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( pointsLayerId, &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, pointsLayerId );

        // with a second store in a different scope
        QgsExpressionContextScope *scope2 = new QgsExpressionContextScope();

        context.appendScope( scope2 );

        QgsMapLayerStore store2;
        QgsVectorLayer *layer3 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:string&field=datef:date(0,0)"_s, u"test_store_3"_s, u"memory"_s );
        store2.addMapLayer( layer3 );
        scope2->addLayerStore( &store2 );

        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( layer3->id(), &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, layer3->id() );

        // from layer store, by name
        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( layer1->name(), &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, layer1->id() );

        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( layer2->name(), &context, &exp, lambda, foundLayer );
        ;
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, layer2->id() );

        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( layer3->name(), &context, &exp, lambda, foundLayer );
        ;
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, layer3->id() );

        foundLayer = false;
        gotLayerId.clear();
        QgsExpressionUtils::executeLambdaForMapLayer( pointsLayerName, &context, &exp, lambda, foundLayer );
        QVERIFY( foundLayer );
        QCOMPARE( gotLayerId, pointsLayerId );

        testOk = true;
      };

      runTest();
      QVERIFY( testOk );
      testOk = false;

      // also run in a thread

      QThread *thread = new RunLambdaInThread( runTest );
      connect( thread, &QThread::finished, thread, &QObject::deleteLater );
      QSignalSpy spy( thread, &QThread::finished );
      thread->start();
      spy.wait();
      QVERIFY( testOk );
    }

    void testGetFilePathValue()
    {
      QgsExpression exp;
      QgsExpressionContext context;
      // NULL value
      QString path = QgsExpressionUtils::getFilePathValue( QVariant(), &context, &exp );
      QVERIFY( path.isEmpty() );
      QVERIFY( !exp.hasEvalError() );

      // value which CANNOT be a file path
      path = QgsExpressionUtils::getFilePathValue( QVariant::fromValue( QgsGeometry() ), &context, &exp );
      QVERIFY( path.isEmpty() );
      QVERIFY( exp.hasEvalError() );
      QCOMPARE( exp.evalErrorString(), u"Cannot convert value to a file path"_s );

      // good value
      exp = QgsExpression();
      path = QgsExpressionUtils::getFilePathValue( QVariant::fromValue( u"/home/me/mine.txt"_s ), &context, &exp );
      QCOMPARE( path, u"/home/me/mine.txt"_s );
      QVERIFY( !exp.hasEvalError() );

      // with map layer pointer -- should use layer path
      exp = QgsExpression();
      path = QgsExpressionUtils::getFilePathValue( QVariant::fromValue( mPointsLayer ), &context, &exp );
      QCOMPARE( path, QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s );
      QVERIFY( !exp.hasEvalError() );
    }

    void testGetTimeZoneValue()
    {
      QgsExpression exp;
      QgsExpressionContext context;
      // NULL value
      QTimeZone tz = QgsExpressionUtils::getTimeZoneValue( QVariant(), &exp );
      QVERIFY( !tz.isValid() );
      QVERIFY( !exp.hasEvalError() );

      // value which CANNOT be a time zone
      tz = QgsExpressionUtils::getTimeZoneValue( QVariant::fromValue( QgsGeometry() ), &exp );
      QVERIFY( !tz.isValid() );
      QVERIFY( exp.hasEvalError() );
      QCOMPARE( exp.evalErrorString(), u"Cannot convert '' to a time zone"_s );

      // good value
      exp = QgsExpression();
      tz = QgsExpressionUtils::getTimeZoneValue( QVariant::fromValue( QTimeZone( "Australia/Brisbane" ) ), &exp );
      QVERIFY( tz.isValid() );
      QCOMPARE( tz.id(), u"Australia/Brisbane"_s );
      QVERIFY( !exp.hasEvalError() );
    }

    void test_env()
    {
      QgsExpressionContext context;

#ifdef Q_OS_WIN
      _putenv( "TESTENV_STRING=Hello World" );
#else
      setenv( "TESTENV_STRING", "Hello World", 1 );
#endif

      QgsExpression e( u"env('TESTENV_STRING')"_s );

      QVariant result = e.evaluate( &context );

      QCOMPARE( result.toString(), u"Hello World"_s );
#ifdef Q_OS_WIN
      _putenv( "TESTENV_STRING=" );
      _putenv( "TESTENV_INT=5" );
#else
      unsetenv( "TESTENV_STRING" );
      setenv( "TESTENV_INT", "5", 1 );
#endif

      QgsExpression e2( u"env('TESTENV_INT')"_s );

      QVariant result2 = e2.evaluate( &context );

      QCOMPARE( result2.toString(), u"5"_s );
#ifdef Q_OS_WIN
      _putenv( "TESTENV_INT=" );
#else
      unsetenv( "TESTENV_INT" );
#endif

      QgsExpression e3( u"env('TESTENV_I_DO_NOT_EXIST')"_s );
      QVariant result3 = e3.evaluate( &context );

      QVERIFY( result3.isNull() );
    }

    void test_formatPreviewString()
    {
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( "hello" ) ), u"'hello'"_s );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( QVariantMap() ) ), u"{}"_s );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( QDateTime( QDate( 2020, 3, 4 ), QTime( 12, 13, 14 ), Qt::UTC ) ) ), u"<i>&lt;datetime: 2020-03-04 12:13:14 (UTC)&gt;</i>"_s );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( QDateTime( QDate( 2020, 3, 4 ), QTime( 12, 13, 14 ), Qt::OffsetFromUTC, 3600 ) ) ), u"<i>&lt;datetime: 2020-03-04 12:13:14 (UTC+01:00)&gt;</i>"_s );

      QVariantMap map;
      map[u"1"_s] = "One";
      map[u"2"_s] = "Two";
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( map ) ), u"{ '1': 'One', '2': 'Two' }"_s );
      map[u"3"_s] = "A very long string that is going to be truncated";
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( map ) ), u"{ '1': 'One', '2': 'Two', '3': 'A very long string that is… }"_s );

      QVariantList list;
      list << 1 << 2 << 3;
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( list ) ), u"[ 1, 2, 3 ]"_s );

      QStringList stringList;
      stringList << u"One"_s << u"Two"_s << u"A very long string that is going to be truncated"_s;
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( stringList ) ), u"[ 'One', 'Two', 'A very long string that is going to be tr… ]"_s );

      QColor color = QColor::fromRgbF( 1., 0.5f, 0.25f, 0.1f );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( color ) ), "RGBA: 1.00,0.50,0.25,0.10" );

      color = QColor::fromCmykF( 1., 0.5f, 0.25f, 0.1f, 0.2f );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( color ) ), "CMYKA: 1.00,0.50,0.25,0.10,0.20" );

      color = QColor::fromHslF( 0.90, 0.5f, 0.25f, 0.1f );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( color ) ), "HSLA: 0.90,0.50,0.25,0.10" );

      color = QColor::fromHsvF( 0.90, 0.5f, 0.25f, 0.1f );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( color ) ), "HSVA: 0.90,0.50,0.25,0.10" );

      QgsCoordinateReferenceSystem crs( "EPSG:4326" );
      QCOMPARE( QgsExpression::formatPreviewString( QVariant( crs ) ), u"<i>&lt;crs: EPSG:4326 - WGS 84&gt;</i>"_s );

      QTimeZone tz( "Australia/Brisbane" );
      QString res = QgsExpression::formatPreviewString( QVariant::fromValue( tz ) );
      QVERIFY2( res == QString( "<i>&lt;time zone: AEST&gt;</i>" ) || res == QString( "<i>&lt;time zone: GMT+10&gt;</i>" ), QString( "got %1, expected <i>&lt;time zone: AEST&gt;</i> or <i>&lt;time zone: GMT+10&gt;</i>" ).arg( res ).toLocal8Bit().constData() );

      QCOMPARE( QgsExpression::formatPreviewString( QVariant::fromValue( QTimeZone() ) ), u"<i>&lt;time zone: invalid&gt;</i>"_s );
    }

    void test_formatPreviewStringWithLocale()
    {
      const QVariant t_int( 12345 );
      QVariant t_uint = QgsVariantUtils::createNullVariant( QMetaType::Type::UInt );
      t_uint = 12345;
      QVariant t_long = QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );
      t_long = 12345;
      QVariant t_ulong = QgsVariantUtils::createNullVariant( QMetaType::Type::ULongLong );
      t_ulong = 12345;
      const QVariant t_float( 12345.001F );
      const QVariant t_double( 12345.001 );

      QLocale::setDefault( QLocale::English );

      QCOMPARE( QgsExpression::formatPreviewString( t_int ), u"12,345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_uint ), u"12,345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_long ), u"12,345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_ulong ), u"12,345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_float ), u"12,345.0009765625"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_double ), u"12,345.001"_s );

      QLocale::setDefault( QLocale::Italian );

      QCOMPARE( QgsExpression::formatPreviewString( t_int ), u"12.345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_uint ), u"12.345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_long ), u"12.345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_ulong ), u"12.345"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_float ), u"12.345,0009765625"_s );
      QCOMPARE( QgsExpression::formatPreviewString( t_double ), u"12.345,001"_s );

      QLocale::setDefault( QLocale::English );
    }

    void test_nowStatic()
    {
      QgsExpression e( u"now()"_s );
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
      QgsExpression e( u"'['"_s );
      QVariant result = e.evaluate( &context );
      QCOMPARE( result.toString(), u"["_s );
      e = QgsExpression( u"']'"_s );
      QCOMPARE( e.evaluate( &context ).toString(), u"]"_s );
      e = QgsExpression( u"'[3]'"_s );
      QCOMPARE( e.evaluate( &context ).toString(), u"[3]"_s );
      e = QgsExpression( u"'a[3]'"_s );
      QCOMPARE( e.evaluate( &context ).toString(), u"a[3]"_s );
      e = QgsExpression( u"try(\"a[3]\", '[a[3]]')"_s );
      QCOMPARE( e.evaluate( &context ).toString(), u"[a[3]]"_s );
      e = QgsExpression( u"(1+2)[0]"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( e.hasEvalError() );
      e = QgsExpression( u"(1+2)['a']"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( e.hasEvalError() );
      // arrays
      e = QgsExpression( u"array(1,2,3)[0]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( u"((array(1,2,3)))[0]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( u"array(1,2,3)[1]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( u"array(1,2,3)[2]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( u"array(1,2,3)[-1]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( u"array(1,2,3)[-2]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( u"array(1,2,3)[-3]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( u"array(1,2,3)[1+1]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( u"array(1,2,3)[(3-2)]"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( u"array(1,2,3)[3]"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
      e = QgsExpression( u"array(1,2,3)[-4]"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
      e = QgsExpression( u"@null_variable[0]"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this

      // maps
      e = QgsExpression( u"map('a',1,'b',2,'c',3)[0]"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
      e = QgsExpression( u"map('a',1,'b',2,'c',3)['d']"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
      e = QgsExpression( u"map('a',1,'b',2,'c',3)['a']"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 1 );
      e = QgsExpression( u"map('a',1,'b',2,'c',3)['b']"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( u"map('a',1,'b',2,'c',3)['c']"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 3 );
      e = QgsExpression( u"map('a',1,'bbb',2,'c',3)['b'||'b'||'b']"_s );
      QCOMPARE( e.evaluate( &context ).toInt(), 2 );
      e = QgsExpression( u"@null_variable['key']"_s );
      QVERIFY( !e.evaluate( &context ).isValid() );
      QVERIFY( !e.hasEvalError() ); // no eval error - we are tolerant to this
    }

    void testSqliteFetchAndIncrementWithTranscationMode()
    {
      QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
      QTemporaryDir tempDir;
      QFile::copy( testDataDir + u"kbs.qgs"_s, tempDir.filePath( "kbs.qgs" ) );
      QFile::copy( testDataDir + u"kbs.gpkg"_s, tempDir.filePath( "kbs.gpkg" ) );

      QgsProject *project = QgsProject::instance();
      QVERIFY( project->read( tempDir.filePath( "kbs.qgs" ) ) );

      QgsVectorLayer *zustaendigkeitskataster = project->mapLayer<QgsVectorLayer *>( u"zustaendigkeitkataster_2b5bb693_3151_4c82_967f_b49d4d348a17"_s );

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

    void testReplaceExpressionText_data()
    {
      QTest::addColumn<QString>( "input" );
      QTest::addColumn<QString>( "expected" );
      QTest::newRow( "no exp" ) << "some text" << "some text";
      QTest::newRow( "simple exp" ) << "some text [% 1 + 2 %]" << "some text 3";
      QTest::newRow( "multiple exp" ) << "some [% 3+ 7 %] text [% 1 + 2 %]" << "some 10 text 3";
      QTest::newRow( "complex" ) << "some [%map('a', 1, 'b', 2)['a']%] text [%map('a', 1, 'b', 2)['b']%]" << "some 1 text 2";
      QTest::newRow( "complex2" ) << "some [% 'my text]' %] text" << "some my text] text";
      QTest::newRow( "newline 1" ) << "some \n [% 1 + 2 %] \n text" << "some \n 3 \n text";
      QTest::newRow( "newline 2" ) << "some [% \n 1 \n + \n 2 %] \n text" << "some 3 \n text";
      QTest::newRow( "field values" ) << "[% \"string_field\" %] - [% \"non_null_int\" %] - [% \"null_int\" %]" << "string value - 5 - ";
    }

    void testReplaceExpressionText()
    {
      QFETCH( QString, input );
      QFETCH( QString, expected );

      QgsExpressionContext context;

      QgsFields fields;
      fields.append( QgsField( "string_field", QMetaType::Type::QString ) );
      fields.append( QgsField( "non_null_int", QMetaType::Type::Int ) );
      fields.append( QgsField( "null_int", QMetaType::Type::Int ) );

      QgsFeature feature( fields );
      feature.setAttributes( QgsAttributes( { QVariant( u"string value"_s ), QVariant( 5 ), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) } ) );

      context.setFeature( feature );
      context.setFields( fields );

      QCOMPARE( QgsExpression::replaceExpressionText( input, &context ), expected );
    }

    void testConcatNULLAttributeValue()
    {
      // Test that null integer values coming from provider are not transformed as 0
      // https://github.com/qgis/QGIS/issues/36112

      QgsFields fields;
      fields.append( QgsField( u"foo"_s, QMetaType::Type::Int ) );

      QgsFeature f;
      f.initAttributes( 1 );
      f.setAttribute( 0, QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );
      QgsExpression exp( u"concat('test', foo)"_s );
      QVariant res = exp.evaluate( &context );
      QCOMPARE( static_cast<QMetaType::Type>( res.userType() ), QMetaType::Type::QString );
      QCOMPARE( res.toString(), u"test"_s );

      f.setAttribute( 0, QVariant() );
      context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );
      res = exp.evaluate( &context );
      QCOMPARE( static_cast<QMetaType::Type>( res.userType() ), QMetaType::Type::QString );
      QCOMPARE( res.toString(), u"test"_s );
    }

    void testIsFieldEqualityExpression_data()
    {
      QTest::addColumn<QString>( "input" );
      QTest::addColumn<bool>( "expected" );
      QTest::addColumn<QString>( "field" );
      QTest::addColumn<QVariant>( "value" );
      QTest::newRow( "empty" ) << "" << false << QString() << QVariant();
      QTest::newRow( "invalid" ) << "a=" << false << QString() << QVariant();
      QTest::newRow( "is string" ) << "field = 'value'" << true << "field" << QVariant( u"value"_s );
      QTest::newRow( "is number" ) << "field = 5" << true << "field" << QVariant( 5 );
      QTest::newRow( "quoted field" ) << "\"my field\" = 5" << true << "my field" << QVariant( 5 );
      QTest::newRow( "not equal" ) << "field <> 5" << false << QString() << QVariant();
    }

    void testIsFieldEqualityExpression()
    {
      QFETCH( QString, input );
      QFETCH( bool, expected );
      QFETCH( QString, field );
      QFETCH( QVariant, value );

      QString resField;
      QVariant resValue;
      QCOMPARE( QgsExpression::isFieldEqualityExpression( input, resField, resValue ), expected );
      if ( expected )
      {
        QCOMPARE( resField, field );
        QCOMPARE( resValue, value );
      }
    }

    void testAttemptReduceToInClause_data()
    {
      QTest::addColumn<QStringList>( "input" );
      QTest::addColumn<bool>( "expected" );
      QTest::addColumn<QString>( "expression" );
      QTest::newRow( "OR conditions mixed IN" ) << ( QStringList() << u"field = 'value' OR field IN( 'value2', 'value3' )"_s ) << true << "\"field\" IN ('value','value2','value3')";
      QTest::newRow( "OR conditions non literal" ) << ( QStringList() << u"field = 'value' OR field = 'value2' OR field = var('qgis_version')"_s ) << false << QString();
      QTest::newRow( "OR conditions mixed IN reverse" ) << ( QStringList() << u"field IN ('value','value2') OR field = 'value3'"_s ) << true << "\"field\" IN ('value','value2','value3')";
      QTest::newRow( "OR conditions mixed IN different fields" ) << ( QStringList() << u"field2 IN ('value','value2') OR field = 'value3'"_s ) << false << QString();
      QTest::newRow( "OR conditions" ) << ( QStringList() << u"field = 'value' OR field = 'value2'"_s ) << true << "\"field\" IN ('value','value2')";
      QTest::newRow( "OR conditions different fields" ) << ( QStringList() << u"field = 'value' OR field2 = 'value2'"_s ) << false << QString();
      QTest::newRow( "OR conditions three" ) << ( QStringList() << u"field = 'value' OR field = 'value2' OR field = 'value3'"_s ) << true << "\"field\" IN ('value','value2','value3')";
      QTest::newRow( "empty" ) << QStringList() << false << QString();
      QTest::newRow( "invalid" ) << ( QStringList() << u"a="_s ) << false << QString();
      QTest::newRow( "not equality" ) << ( QStringList() << u"field <> 'value'"_s ) << false << "\"field\"";
      QTest::newRow( "one expression" ) << ( QStringList() << u"field = 'value'"_s ) << true << "\"field\" IN ('value')";
      QTest::newRow( "one IN expression" ) << ( QStringList() << u"field IN ('value', 'value2')"_s ) << true << "\"field\" IN ('value','value2')";
      QTest::newRow( "one NOT IN expression" ) << ( QStringList() << u"field NOT IN ('value', 'value2')"_s ) << false << "\"field\" NOT IN ('value','value2')";
      QTest::newRow( "one IN expression non-literal" ) << ( QStringList() << u"field IN ('value', 'value2', \"a field\")"_s ) << false << QString();
      QTest::newRow( "two expressions" ) << ( QStringList() << u"field = 'value'"_s << u"field = 'value2'"_s ) << true << "\"field\" IN ('value','value2')";
      QTest::newRow( "two expressions with IN" ) << ( QStringList() << u"field = 'value'"_s << u"field IN ('value2', 'value3')"_s ) << true << "\"field\" IN ('value','value2','value3')";
      QTest::newRow( "two expressions with a not IN" ) << ( QStringList() << u"field = 'value'"_s << u"field NOT IN ('value2', 'value3')"_s ) << false << QString();
      QTest::newRow( "two expressions with IN not literal" ) << ( QStringList() << u"field = 'value'"_s << u"field IN ('value2', 'value3', \"a field\")"_s ) << false << QString();
      QTest::newRow( "two expressions with IN different field" ) << ( QStringList() << u"field = 'value'"_s << u"field2 IN ('value2', 'value3')"_s ) << false << QString();
      QTest::newRow( "two expressions first not equality" ) << ( QStringList() << u"field <>'value'"_s << u"field == 'value2'"_s ) << false << "\"field\"";
      QTest::newRow( "two expressions second not equality" ) << ( QStringList() << u"field = 'value'"_s << u"field <> 'value2'"_s ) << false << "\"field\"";
      QTest::newRow( "three expressions" ) << ( QStringList() << u"field = 'value'"_s << u"field = 'value2'"_s << u"field = 'value3'"_s ) << true << "\"field\" IN ('value','value2','value3')";
      QTest::newRow( "three expressions with IN" ) << ( QStringList() << u"field IN ('v1', 'v2')"_s << u"field = 'value'"_s << u"field = 'value2'"_s << u"field = 'value3'"_s ) << true << "\"field\" IN ('v1','v2','value','value2','value3')";
      QTest::newRow( "three expressions different fields" ) << ( QStringList() << u"field = 'value'"_s << u"field2 = 'value2'"_s << u"\"field\" = 'value3'"_s ) << false << "field";
    }

    void testAttemptReduceToInClause()
    {
      QFETCH( QStringList, input );
      QFETCH( bool, expected );
      QFETCH( QString, expression );

      QString resExpression;
      QCOMPARE( QgsExpression::attemptReduceToInClause( input, resExpression ), expected );
      if ( expected )
      {
        QCOMPARE( resExpression, expression );
      }
    }

    void testCollapseOrValues_data()
    {
      QTest::addColumn<QString>( "expression" );
      QTest::addColumn<QString>( "expected" );


      QTest::newRow( "simple" ) << u"field = 'value' OR field = 'value2'"_s << u"field  IN ('value', 'value2')"_s;
      QTest::newRow( "simple 2" ) << u"\"field\" = 'value' OR field = 'value2' OR field = 'value3'"_s << u"field  IN ('value', 'value2', 'value3')"_s;
      QTest::newRow( "simple3 mixed" ) << u"field = 'value' OR field = 'value2' OR field2 = 'value3'"_s << u"field  IN ('value', 'value2') OR field2 = 'value3'"_s;
      QTest::newRow( "simple3 mixed 2" ) << u"field2 = 'value3' OR field = 'value1' OR field = 'value2'"_s << u"field2 = 'value3' OR field  IN ('value1', 'value2')"_s;
      QTest::newRow( "simple3 mixed 3" ) << u"field = 'value1' OR field2 = 'value3' OR field = 'value2'"_s << u"field  IN ('value1', 'value2') OR field2 = 'value3'"_s;
      QTest::newRow( "simple mixed order" ) << u"field = 'value' OR 'value2' = field OR field = 'value3'"_s << u"field  IN ('value', 'value2', 'value3')"_s;

      // test with IN
      QTest::newRow( "simple IN" ) << u"field IN ('value', 'value2') OR field = 'value3'"_s << u"field  IN ('value', 'value2', 'value3')"_s;
      QTest::newRow( "simple IN 2" ) << u"field = 'value' OR field IN ('value2', 'value3')"_s << u"field  IN ('value', 'value2', 'value3')"_s;
      // not in, should fail
      QTest::newRow( "not IN left" ) << u"field NOT IN ('value', 'value2') OR field = 'value3'"_s << u"field NOT IN ('value', 'value2') OR field = 'value3'"_s;
      QTest::newRow( "not IN right" ) << u"field IN ('value', 'value2') OR field NOT IN ('value3')"_s << u"field  IN ('value', 'value2') OR field NOT IN ('value3')"_s;
      QTest::newRow( "not IN left same" ) << u"field NOT IN ('value') OR field IN ('value')"_s << u"field NOT IN ('value') OR field  IN ('value')"_s;
      QTest::newRow( "not IN right same" ) << u"field IN ('value') OR field NOT IN ('value')"_s << u"field  IN ('value') OR field NOT IN ('value')"_s;
      QTest::newRow( "not IN three" ) << u"\"TYPE\" in ('a') OR \"TYPE\" not in ('b') OR \"TYPE\" in ('c')"_s << u"TYPE  IN ('a') OR TYPE NOT IN ('b') OR TYPE  IN ('c')"_s;

      // could be handled, but isn't right now
      QTest::newRow( "not IN both" ) << u"field NOT IN ('value', 'value2') OR field NOT IN ('value3')"_s << u"field NOT IN ('value', 'value2') OR field NOT IN ('value3')"_s;

      // test cases that should not trigger reduction
      QTest::newRow( "no reduction 1" ) << u"field = 'value' OR field2 = 'value2'"_s << u"field = 'value' OR field2 = 'value2'"_s;
      // this could theoretically be reduced, but we don't currently support this
      QTest::newRow( "no reduction 2" ) << u"field = 'value' OR field != 'value2' OR field = 'value3'"_s << u"field = 'value' OR field <> 'value2' OR field = 'value3'"_s;
    }

    void testCollapseOrValues()
    {
      QFETCH( QString, expression );
      QFETCH( QString, expected );

      QgsExpression exp( expression );
      QgsExpressionContext context;
      exp.prepare( &context );
      QCOMPARE( exp.rootNode()->effectiveNode()->dump(), expected );
    }

    void testPrecomputedNodesWithIntrospectionFunctions()
    {
      QgsFields fields;
      fields.append( QgsField( u"first_field"_s, QMetaType::Type::Int ) );
      fields.append( QgsField( u"second_field"_s, QMetaType::Type::Int ) );

      QgsExpression exp( u"attribute(@static_feature, concat('second','_',@field_name_part_var)) + x(geometry( @static_feature ))"_s );
      // initially this expression requires all attributes -- we can't determine the referenced columns in advance
      QCOMPARE( exp.referencedColumns(), QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
      QCOMPARE( exp.referencedAttributeIndexes( fields ), QSet<int>() << 0 << 1 );
      QCOMPARE( exp.referencedFunctions(), QSet<QString>() << u"attribute"_s << u"concat"_s << u"geometry"_s << u"x"_s << u"var"_s );
      QCOMPARE( exp.referencedVariables(), QSet<QString>() << u"field_name_part_var"_s << u"static_feature"_s );

      // prepare the expression using static variables
      QgsExpressionContext context;
      auto scope = std::make_unique<QgsExpressionContextScope>();
      scope->setVariable( u"field_name_part_var"_s, u"field"_s, true );

      // this feature gets added as a static variable, to emulate eg the @atlas_feature variable
      QgsFeature feature( fields );
      feature.setAttributes( QgsAttributes() << 5 << 10 );
      feature.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 27, 42 ) ) );
      scope->setVariable( u"static_feature"_s, feature, true );

      context.appendScope( scope.release() );

      QVERIFY( exp.prepare( &context ) );
      // because all parts of the expression are static, the root node should have a cached static value!
      QVERIFY( exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->cachedStaticValue().toInt(), 37 );

      // referenced columns should be empty -- we don't need ANY columns to evaluate this expression
      QVERIFY( exp.referencedColumns().empty() );
      QVERIFY( exp.referencedAttributeIndexes( fields ).empty() );
      // in contrast, referencedFunctions() and referencedVariables() should NOT be affected by pre-compiled nodes
      // as these methods are used for introspection purposes only...
      QCOMPARE( exp.referencedFunctions(), QSet<QString>() << u"attribute"_s << u"concat"_s << u"geometry"_s << u"x"_s << u"var"_s );
      QCOMPARE( exp.referencedVariables(), QSet<QString>() << u"field_name_part_var"_s << u"static_feature"_s );

      // secondary test - this one uses a mix of pre-computable nodes and non-pre-computable nodes
      QgsExpression exp2( u"(attribute(@static_feature, concat('second','_',@field_name_part_var)) + x(geometry( @static_feature ))) > \"another_field\""_s );
      QCOMPARE( exp2.referencedColumns(), QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES << u"another_field"_s );
      QCOMPARE( exp2.referencedFunctions(), QSet<QString>() << u"attribute"_s << u"concat"_s << u"geometry"_s << u"x"_s << u"var"_s );
      QCOMPARE( exp2.referencedVariables(), QSet<QString>() << u"field_name_part_var"_s << u"static_feature"_s );

      QgsFields fields2;
      fields2.append( QgsField( u"another_field"_s, QMetaType::Type::Int ) );
      context.setFields( fields2 );

      QVERIFY( exp2.prepare( &context ) );
      // because NOT all parts of the expression are static, the root node should NOT have a cached static value!
      QVERIFY( !exp2.rootNode()->hasCachedStaticValue() );

      // but the only referenced column should be "another_field", because the first half of the expression with the "attribute" function is static and has been precomputed
      QCOMPARE( exp2.referencedColumns(), QSet<QString>() << u"another_field"_s );
      QCOMPARE( exp2.referencedAttributeIndexes( fields2 ), QSet<int>() << 0 );
      QCOMPARE( exp2.referencedFunctions(), QSet<QString>() << u"attribute"_s << u"concat"_s << u"geometry"_s << u"x"_s << u"var"_s );
      QCOMPARE( exp2.referencedVariables(), QSet<QString>() << u"field_name_part_var"_s << u"static_feature"_s );
    }

    void testPrecomputedNodesWithBinaryOperators()
    {
      QgsFields fields;
      fields.append( QgsField( u"first_field"_s, QMetaType::Type::Int ) );
      fields.append( QgsField( u"second_field"_s, QMetaType::Type::Int ) );

      // OR operations:

      // the result of this expression will always be true, regardless of the field value - so we can precompile it to a static node!
      QgsExpression exp( u"\"first_field\" or (true or false)"_s );

      // prepare the expression using static variables
      QgsExpressionContext context;
      context.setFields( fields );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->cachedStaticValue().toBool(), true );

      // the result of this expression depends on the value of first_field, it can't be completely precompiled
      exp = QgsExpression( u"\"first_field\" or (true and false)"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );

      // the result of this expression will always be true, regardless of the field value
      exp = QgsExpression( u"(true or false) or \"first_field\""_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->cachedStaticValue().toBool(), true );

      // the result of this expression depends on the value of first_field, it can't be completely precompiled
      exp = QgsExpression( u"(true and false) or \"first_field\""_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );

      // AND operations:

      // the result of this expression will always be false, regardless of the field value - so we can precompile it to a static node!
      exp = QgsExpression( u"\"first_field\" AND (true AND false)"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->cachedStaticValue().toBool(), false );

      // the result of this expression depends on the value of first_field, it can't be completely precompiled
      exp = QgsExpression( u"\"first_field\" AND (true or false)"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );

      // the result of this expression will always be false, regardless of the field value
      exp = QgsExpression( u"(true and false) AND \"first_field\""_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->cachedStaticValue().toBool(), false );

      // the result of this expression depends on the value of first_field, it can't be completely precompiled
      exp = QgsExpression( u"(true or false) AND \"first_field\""_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
    }

    void testPrecomputedNodesReplacedWithEffectiveNodes()
    {
      QgsFields fields;
      fields.append( QgsField( u"first_field"_s, QMetaType::Type::Int ) );
      fields.append( QgsField( u"second_field"_s, QMetaType::Type::Int ) );
      fields.append( QgsField( u"third_field"_s, QMetaType::Type::Int ) );

      QgsFeature f( fields );
      f.setAttributes( QgsAttributes() << 11 << 20 << 300 );

      QgsExpressionContext context;
      context.setFields( fields );
      context.setFeature( f );

      // nothing we can do to optimize this expression
      QgsExpression exp( u"CASE WHEN \"first_field\" = 5 then \"second_field\" when \"first_field\" = 6 then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntCondition );
      QCOMPARE( exp.evaluate( &context ).toInt(), 60 );

      exp = QgsExpression( u"CASE WHEN \"first_field\" = 5 then \"second_field\" when \"first_field\" = 11 then \"second_field\" * 2 else 77 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntCondition );
      QCOMPARE( exp.evaluate( &context ).toInt(), 40 );

      // slightly more complex expression
      exp = QgsExpression( u"CASE WHEN (upper(\"first_field\") = 'AA') then \"second_field\" when \"first_field\" = 11 then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntCondition );
      QCOMPARE( exp.evaluate( &context ).toInt(), 40 );

      // first condition is non-static, second is static... still nothing we can do to optimize this one
      exp = QgsExpression( u"CASE WHEN (upper(\"first_field\") = 'AA') then \"second_field\" when 3 * 2 = 6 then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntCondition );
      QCOMPARE( exp.evaluate( &context ).toInt(), 40 );

      // first condition is static but false, second condition is non static ... can't optimize
      exp = QgsExpression( u"CASE WHEN 3 * 2 = 7 then \"second_field\" when \"second_field\" = 'B' then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntCondition );
      QCOMPARE( exp.evaluate( &context ).toInt(), 60 );

      // first condition is static but NULL, second condition is non static ... can't optimize
      exp = QgsExpression( u"CASE WHEN NULL then \"second_field\" when \"second_field\" = 'B' then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntCondition );
      QCOMPARE( exp.evaluate( &context ).toInt(), 60 );

      // first condition is static AND true, and THEN expression for this node is static -- yay, we CAN optimize this down to a static value for the whole node
      exp = QgsExpression( u"CASE WHEN 3 * 2 = 6 then 7 + 4 when \"second_field\" = 'B' then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->cachedStaticValue().toInt(), 11 );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntCondition );
      QCOMPARE( exp.evaluate( &context ).toInt(), 11 );

      // first condition is static AND true, but THEN expression is non-static -- yay, we CAN still optimize this, because we will ALWAYS be returning the evaluated
      // value for the THEN clause of the first condition, so we can effectively replace the entire node with the THEN expression of the first condition
      exp = QgsExpression( u"CASE WHEN 3 * 2 = 6 then \"first_field\" when \"second_field\" = 'B' then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntColumnRef );
      QCOMPARE( qgis::down_cast<const QgsExpressionNodeColumnRef *>( exp.rootNode()->effectiveNode() )->name(), u"first_field"_s );
      QCOMPARE( exp.evaluate( &context ).toInt(), 11 );

      // first condition is static AND false, second is static AND true, so we can effectively replace the entire node with the THEN expression of the second condition
      exp = QgsExpression( u"CASE WHEN 3 * 2 = 7 then \"first_field\" when 'B'='B' then \"second_field\" else \"third_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntColumnRef );
      QCOMPARE( qgis::down_cast<const QgsExpressionNodeColumnRef *>( exp.rootNode()->effectiveNode() )->name(), u"second_field"_s );
      QCOMPARE( exp.evaluate( &context ).toInt(), 20 );

      // first two conditions are static AND false, so we can effectively replace the entire node with the ELSE expression
      exp = QgsExpression( u"CASE WHEN 3 * 2 = 7 then \"first_field\" when 'B'='C' then \"second_field\" else \"third_field\" end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntColumnRef );
      QCOMPARE( qgis::down_cast<const QgsExpressionNodeColumnRef *>( exp.rootNode()->effectiveNode() )->name(), u"third_field"_s );
      QCOMPARE( exp.evaluate( &context ).toInt(), 300 );

      // slightly more complex -- second condition is static and TRUE, but uses a more complicated THEN node
      exp = QgsExpression( u"CASE WHEN 3 * 2 = 7 then \"first_field\" when 'B'='B' then upper(\"second_field\") || \"first_field\" else \"third_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntBinaryOperator );
      QCOMPARE( qgis::down_cast<const QgsExpressionNodeBinaryOperator *>( exp.rootNode()->effectiveNode() )->opLeft()->nodeType(), QgsExpressionNode::NodeType::ntFunction );
      QCOMPARE( qgis::down_cast<const QgsExpressionNodeBinaryOperator *>( exp.rootNode()->effectiveNode() )->opRight()->nodeType(), QgsExpressionNode::NodeType::ntColumnRef );
      QCOMPARE( exp.evaluate( &context ).toInt(), 2011 );

      // EVEN more complex -- second condition is static and TRUE, and uses a nested CASE as the THEN node
      // the whole root node can be replaced by a column ref to "second_field"
      exp = QgsExpression( u"CASE WHEN 3 * 2 = 7 then \"first_field\" when 'B'='B' then ( CASE WHEN 3*3=11 then \"first_field\" ELSE \"second_field\" END) else \"third_field\" end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->effectiveNode()->nodeType(), QgsExpressionNode::NodeType::ntColumnRef );
      QCOMPARE( qgis::down_cast<const QgsExpressionNodeColumnRef *>( exp.rootNode()->effectiveNode() )->name(), u"second_field"_s );
      QCOMPARE( exp.evaluate( &context ).toInt(), 20 );
    }

    void testNodeSetCachedStaticValue()
    {
      QgsFields fields;
      fields.append( QgsField( u"first_field"_s, QMetaType::Type::Int ) );
      fields.append( QgsField( u"second_field"_s, QMetaType::Type::Int ) );
      fields.append( QgsField( u"third_field"_s, QMetaType::Type::Int ) );

      QgsFeature f( fields );
      f.setAttributes( QgsAttributes() << 11 << 20 << 300 );

      QgsExpressionContext context;
      context.setFields( fields );
      context.setFeature( f );

      QgsExpression exp( u"CASE WHEN \"first_field\" = 5 then \"second_field\" when \"first_field\" = 6 then \"second_field\" * 2 else \"second_field\" * 3 end"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.rootNode()->hasCachedStaticValue() );

      // force set a cached static value
      exp.rootNode()->setCachedStaticValue( 55 );
      QVERIFY( exp.rootNode()->hasCachedStaticValue() );
      QCOMPARE( exp.rootNode()->cachedStaticValue().toInt(), 55 );
    }

    void testExpressionUtilsToLocalizedString()
    {
      const QVariant t_int( 12346 );
      QVariant t_uint = QgsVariantUtils::createNullVariant( QMetaType::Type::UInt );
      t_uint = 12346;
      QVariant t_long = QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );
      t_long = 12346;
      QVariant t_ulong = QgsVariantUtils::createNullVariant( QMetaType::Type::ULongLong );
      t_ulong = 12346;
      const QVariant t_double( 123456.801 );

      QLocale::setDefault( QLocale::English );

      qDebug() << QVariant( 123456.801 ).toString();

      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_int ), u"12,346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_uint ), u"12,346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_long ), u"12,346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_ulong ), u"12,346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_double ), u"123,456.801"_s );

      QLocale::setDefault( QLocale::Italian );

      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_int ), u"12.346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_uint ), u"12.346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_long ), u"12.346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_ulong ), u"12.346"_s );
      QCOMPARE( QgsExpressionUtils::toLocalizedString( t_double ), u"123.456,801"_s );

      QLocale::setDefault( QLocale::English );

      QCOMPARE( QgsExpressionUtils::toLocalizedString( QString( "hello world" ) ), u"hello world"_s );
    }

    void testLoadLayer()
    {
      QgsExpressionContext context;
      QgsMapLayerStore store;

      // load_layer is only available when a destination store is set
      QVERIFY( !context.hasFunction( u"load_layer"_s ) );
      QVERIFY( !context.functionNames().contains( u"load_layer"_s ) );
      QVERIFY( !context.function( u"load_layer"_s ) );

      context.setLoadedLayerStore( &store );
      QVERIFY( context.hasFunction( u"load_layer"_s ) );
      QVERIFY( context.functionNames().contains( u"load_layer"_s ) );
      QVERIFY( context.function( u"load_layer"_s ) );

      const QString pointsFileName = QStringLiteral( TEST_DATA_DIR ) + '/' + "points.shp";
      QgsExpression exp( u"layer_property(load_layer('%1', 'ogr'), 'feature_count')"_s.arg( pointsFileName ) );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.hasEvalError() );
      QCOMPARE( exp.evaluate( &context ).toInt(), 17 );

      // non-static arguments are not allowed
      QgsFields fields;
      fields.append( QgsField( u"first_field"_s, QMetaType::Type::Int ) );

      QgsFeature f( fields );
      f.setAttributes( QgsAttributes() << 11 );
      context.setFields( fields );
      context.setFeature( f );

      exp = QgsExpression( u"layer_property(load_layer('%1' || \"first_field\", 'ogr'), 'feature_count')"_s.arg( pointsFileName ) );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.hasEvalError() );
      QCOMPARE( exp.evalErrorString(), u"load_layer requires a static value for the uri argument"_s );

      exp = QgsExpression( u"layer_property(load_layer('%1', 'ogr' || \"first_field\"), 'feature_count')"_s.arg( pointsFileName ) );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.hasEvalError() );
      QCOMPARE( exp.evalErrorString(), u"load_layer requires a static value for the provider argument"_s );

      // invalid provider
      exp = QgsExpression( u"layer_property(load_layer('%1', 'magic'), 'feature_count')"_s.arg( pointsFileName ) );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.hasEvalError() );
      QCOMPARE( exp.evalErrorString(), u"Invalid provider argument for load_layer"_s );

      // invalid uri
      exp = QgsExpression( u"layer_property(load_layer('nope', 'ogr'), 'feature_count')"_s );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( exp.hasEvalError() );
      QCOMPARE( exp.evalErrorString(), u"Could not load_layer with uri: nope"_s );

      // raster layer
      const QString rasterFileName = QStringLiteral( TEST_DATA_DIR ) + '/' + "tenbytenraster.asc";
      exp = QgsExpression( u"layer_property(load_layer('%1', 'gdal'), 'type')"_s.arg( rasterFileName ) );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.hasEvalError() );
      QCOMPARE( exp.evaluate( &context ).toString(), u"Raster"_s );

      // complex expression
      exp = QgsExpression( u"with_variable('layer', load_layer('%1', 'gdal'), layer_property(@layer, 'type') || layer_property(@layer, 'type'))"_s.arg( rasterFileName ) );
      QVERIFY( exp.prepare( &context ) );
      QVERIFY( !exp.hasEvalError() );
      QCOMPARE( exp.evaluate( &context ).toString(), u"RasterRaster"_s );
    }

    void testHelpExamples()
    {
      // trigger initialization of function help
      QgsExpression::helpText( QString() );
      const HelpTextHash &functionHelp = QgsExpression::functionHelpTexts();
      for ( auto helpIt = functionHelp.constBegin(); helpIt != functionHelp.constEnd(); ++helpIt )
      {
#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 11
        if ( helpIt->mName == "concave_hull"_L1 )
          continue;
#endif

        for ( auto variantIt = helpIt->mVariants.constBegin(); variantIt != helpIt->mVariants.constEnd(); ++variantIt )
        {
          for ( const HelpExample &example : std::as_const( variantIt->mExamples ) )
          {
            const QString htmlExpression = example.mExpression;
            QTextDocument sourceDoc;
            sourceDoc.setHtml( htmlExpression );
            const QString plainTextExpression = sourceDoc.toPlainText();

            QgsExpression exampleExpression( plainTextExpression );
            QVERIFY2( !exampleExpression.hasParserError(), u"Expression: %1 for %2 has parser error"_s.arg( plainTextExpression, helpIt->mName ).toLocal8Bit() );
          }
        }
      }
    }

    void testDumpLiteralNode_data()
    {
      QTest::addColumn<QVariant>( "value" );
      QTest::addColumn<QString>( "expected" );
      QTest::newRow( "int 1" ) << QVariant( 1 ) << u"1"_s;
      QTest::newRow( "int 11231234" ) << QVariant( 11231234 ) << u"11231234"_s;
      QTest::newRow( "int -11231234" ) << QVariant( -11231234 ) << u"-11231234"_s;
      QTest::newRow( "double 11.5" ) << QVariant( 11.5 ) << u"11.5"_s;
      QTest::newRow( "double -11.5" ) << QVariant( -11.5 ) << u"-11.5"_s;
      QTest::newRow( "double 3972047.39999999990686774" ) << QVariant( 3972047.39999999990686774 ) << u"3972047.39999999990686774"_s;
      QTest::newRow( "string abc" ) << QVariant( u"abc"_s ) << u"'abc'"_s;
      QTest::newRow( "bool TRUE" ) << QVariant( true ) << u"TRUE"_s;
      QTest::newRow( "bool FALSE" ) << QVariant( false ) << u"FALSE"_s;
      QTest::newRow( "date" ) << QVariant( QDate( 2020, 5, 12 ) ) << u"'2020-05-12'"_s;
      QTest::newRow( "time" ) << QVariant( QTime( 9, 5, 12 ) ) << u"'09:05:12'"_s;
      QTest::newRow( "datetime" ) << QVariant( QDateTime( QDate( 2020, 5, 12 ), QTime( 9, 5, 12 ), Qt::UTC ) ) << u"'2020-05-12T09:05:12Z'"_s;
    }

    void testDumpLiteralNode()
    {
      QFETCH( QVariant, value );
      QFETCH( QString, expected );

      QCOMPARE( QgsExpressionNodeLiteral( value ).dump(), expected );
    }
};

QGSTEST_MAIN( TestQgsExpression )

#include "testqgsexpression.moc"
