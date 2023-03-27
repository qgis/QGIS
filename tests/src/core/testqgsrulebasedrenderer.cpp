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
#include <QDomDocument>
#include <QFile>
#include <QTemporaryFile>
//header for class being tested
#include <qgsrulebasedrenderer.h>

#include <qgsapplication.h>
#include <qgsreadwritecontext.h>
#include <qgssymbol.h>
#include <qgsvectorlayer.h>
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsmultirenderchecker.h"
#include "qgsvectorlayerfeaturecounter.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsrendererrange.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsgeometry.h"
#include "qgsembeddedsymbolrenderer.h"

typedef QgsRuleBasedRenderer::Rule RRule;

class TestQgsRuleBasedRenderer: public QgsTest
{
    Q_OBJECT

  public:

    TestQgsRuleBasedRenderer() : QgsTest( QStringLiteral( "Rule based renderer tests" ) ) {}

  private slots:

    void initTestCase()
    {
      // we need memory provider, so make sure to load providers
      QgsApplication::init();
      QgsApplication::initQgis();
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void test_load_xml()
    {
      QDomDocument doc;
      xml2domElement( QStringLiteral( "rulebasedrenderer_simple.xml" ), doc );
      QDomElement elem = doc.documentElement();

      QgsRuleBasedRenderer *r = static_cast<QgsRuleBasedRenderer *>( QgsRuleBasedRenderer::create( elem, QgsReadWriteContext() ) );
      QVERIFY( r );
      check_tree_valid( r->rootRule() );
      delete r;
    }

    void test_load_invalid_xml()
    {
      QDomDocument doc;
      xml2domElement( QStringLiteral( "rulebasedrenderer_invalid.xml" ), doc );
      QDomElement elem = doc.documentElement();

      const std::shared_ptr<QgsRuleBasedRenderer> r( static_cast<QgsRuleBasedRenderer *>( QgsRuleBasedRenderer::create( elem, QgsReadWriteContext() ) ) );
      QVERIFY( !r );
    }

    void test_willRenderFeature_symbolsForFeature()
    {
      // prepare features
      QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "point?field=fld:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      const int idx = layer->fields().indexFromName( QStringLiteral( "fld" ) );
      QVERIFY( idx != -1 );
      QgsFeature f1;
      f1.initAttributes( 1 );
      f1.setAttribute( idx, QVariant( 2 ) );
      QgsFeature f2;
      f2.initAttributes( 1 );
      f2.setAttribute( idx, QVariant( 8 ) );
      QgsFeature f3;
      f3.initAttributes( 1 );
      f3.setAttribute( idx, QVariant( 100 ) );

      // prepare renderer
      QgsSymbol *s1 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
      QgsSymbol *s2 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
      RRule *rootRule = new RRule( nullptr );
      rootRule->appendChild( new RRule( s1, 0, 0, QStringLiteral( "fld >= 5 and fld <= 20" ) ) );
      rootRule->appendChild( new RRule( s2, 0, 0, QStringLiteral( "fld <= 10" ) ) );
      QgsRuleBasedRenderer r( rootRule );

      QVERIFY( r.capabilities() & QgsFeatureRenderer::MoreSymbolsPerFeature );

      QgsRenderContext ctx; // dummy render context
      ctx.expressionContext().setFields( layer->fields() );
      r.startRender( ctx, layer->fields() );

      // test willRenderFeature
      ctx.expressionContext().setFeature( f1 );
      QVERIFY( r.willRenderFeature( f1, ctx ) );
      ctx.expressionContext().setFeature( f2 );
      QVERIFY( r.willRenderFeature( f2, ctx ) );
      ctx.expressionContext().setFeature( f3 );
      QVERIFY( !r.willRenderFeature( f3, ctx ) );

      // test symbolsForFeature
      ctx.expressionContext().setFeature( f1 );
      const QgsSymbolList lst1 = r.symbolsForFeature( f1, ctx );
      QVERIFY( lst1.count() == 1 );
      ctx.expressionContext().setFeature( f2 );
      const QgsSymbolList lst2 = r.symbolsForFeature( f2, ctx );
      QVERIFY( lst2.count() == 2 );
      ctx.expressionContext().setFeature( f3 );
      const QgsSymbolList lst3 = r.symbolsForFeature( f3, ctx );
      QVERIFY( lst3.isEmpty() );

      r.stopRender( ctx );

      delete layer;
    }

    void test_clone_ruleKey()
    {
      RRule *rootRule = new RRule( nullptr );
      RRule *sub1Rule = new RRule( nullptr, 0, 0, QStringLiteral( "fld > 1" ) );
      RRule *sub2Rule = new RRule( nullptr, 0, 0, QStringLiteral( "fld > 2" ) );
      RRule *sub3Rule = new RRule( nullptr, 0, 0, QStringLiteral( "fld > 3" ) );
      rootRule->appendChild( sub1Rule );
      sub1Rule->appendChild( sub2Rule );
      sub2Rule->appendChild( sub3Rule );
      const QgsRuleBasedRenderer r( rootRule );

      QgsRuleBasedRenderer *clone = static_cast<QgsRuleBasedRenderer *>( r.clone() );
      RRule *cloneRootRule = clone->rootRule();
      RRule *cloneSub1Rule = cloneRootRule->children()[0];
      RRule *cloneSub2Rule = cloneSub1Rule->children()[0];
      RRule *cloneSub3Rule = cloneSub2Rule->children()[0];

      QCOMPARE( rootRule->ruleKey(), cloneRootRule->ruleKey() );
      QCOMPARE( sub1Rule->ruleKey(), cloneSub1Rule->ruleKey() );
      QCOMPARE( sub2Rule->ruleKey(), cloneSub2Rule->ruleKey() );
      QCOMPARE( sub3Rule->ruleKey(), cloneSub3Rule->ruleKey() );

      delete clone;
    }

    /**
     * test_many_rules_expression_filter checks that with > 50 rules we have a binary tree (log2(N))
     */
    void test_many_rules_expression_filter()
    {

      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( QStringLiteral( "point?field=fld:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      QgsRenderContext ctx; // dummy render context
      ctx.expressionContext().setFields( layer->fields() );

      const std::function<QString( const int ruleCount )> makeFilter = [ & ]( const int rc ) -> QString
      {

        // prepare renderer
        RRule *rootRule = new RRule( nullptr );
        for ( int i = 0; i < rc; i++ )
        {
          rootRule->appendChild( new RRule( nullptr, 0, 0, QString::number( i ) ) );
        }
        QgsRuleBasedRenderer r( rootRule );
        r.startRender( ctx, layer->fields() );
        r.stopRender( ctx );
        return r.filter();
      };

      QCOMPARE( makeFilter( 1 ), QString( "(0)" ) );
      QCOMPARE( makeFilter( 2 ), QString( "(0) OR (1)" ) );
      QCOMPARE( makeFilter( 3 ), QString( "(0) OR (1) OR (2)" ) );
      QCOMPARE( makeFilter( 10 ), QString( "(0) OR (1) OR (2) OR (3) OR (4) OR (5) OR (6) OR (7) OR (8) OR (9)" ) );
      QCOMPARE( makeFilter( 51 ), QString( "(((((0) OR ((1) OR (2))) OR ((3) OR ((4) OR (5)))) OR (((6) OR ((7) OR (8))) OR ((9) OR ((10) OR (11))))) OR "
                                           "((((12) OR ((13) OR (14))) OR ((15) OR ((16) OR (17)))) OR (((18) OR ((19) OR (20))) OR (((21) OR (22)) OR ((23) OR (24)))))) OR "
                                           "(((((25) OR ((26) OR (27))) OR ((28) OR ((29) OR (30)))) OR (((31) OR ((32) OR (33))) OR (((34) OR (35)) OR ((36) OR (37))))) OR "
                                           "((((38) OR ((39) OR (40))) OR ((41) OR ((42) OR (43)))) OR (((44) OR ((45) OR (46))) OR (((47) OR (48)) OR ((49) OR (50))))))" ) );

    }

    void testElse()
    {
      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      layer->setRenderer( new QgsRuleBasedRenderer( rootrule ) );
      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );
      mapsettings.setLayers( {layer.get()} );

      QgsMultiRenderChecker renderchecker;
      renderchecker.setMapSettings( mapsettings );
      renderchecker.setControlName( QStringLiteral( "expected_rulebased_else" ) );
      const bool res = renderchecker.runTest( QStringLiteral( "rulebased_else" ) );
      if ( !res )
        mReport += renderchecker.report();
      QVERIFY( res );
    }

    void testDisabledElse()
    {
      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      r2->setActive( false );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      layer->setRenderer( new QgsRuleBasedRenderer( rootrule ) );
      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );
      mapsettings.setLayers( {layer.get()} );

      QgsMultiRenderChecker renderchecker;
      renderchecker.setMapSettings( mapsettings );
      renderchecker.setControlName( QStringLiteral( "expected_rulebased_disabled_else" ) );
      const bool res = renderchecker.runTest( QStringLiteral( "rulebased_disabled_else" ) );
      if ( !res )
        mReport += renderchecker.report();
      QVERIFY( res );
    }

    void testWillRenderFeature()
    {
      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      r2->setActive( false );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer( rootrule );
      layer->setRenderer( renderer );
      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );
      mapsettings.setLayers( {layer.get()} );

      QgsFeature f;
      QgsFeatureIterator it = layer->getFeatures();
      QVERIFY( it.nextFeature( f ) );

      QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mapsettings );
      ctx.expressionContext().setFeature( f );

      renderer->rootRule()->children()[0]->setActive( false );
      renderer->rootRule()->children()[1]->setActive( true );
      renderer->rootRule()->children()[2]->setActive( true );

      renderer->startRender( ctx, layer->fields() ); // build mActiveChildren
      bool rendered = renderer->willRenderFeature( f, ctx );
      renderer->stopRender( ctx );
      renderer->rootRule()->children()[0]->setActive( true );
      QVERIFY( !rendered );

      renderer->startRender( ctx, layer->fields() ); // build mActiveChildren
      rendered = renderer->willRenderFeature( f, ctx );
      renderer->stopRender( ctx );
      QVERIFY( rendered );
    }

    void testGroupAndElseRules()
    {
      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *rx1 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, "\"id\" < 3" );
      QgsRuleBasedRenderer::Rule *rx2 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      QgsRuleBasedRenderer::Rule *subrx1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *subrx2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      rx1->appendChild( subrx1 );
      rx1->appendChild( subrx2 );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( rx1 );
      rootrule->appendChild( rx2 );

      rootrule->children()[0]->children()[0]->setActive( false );
      rootrule->children()[0]->children()[1]->setActive( false );

      layer->setRenderer( new QgsRuleBasedRenderer( rootrule ) );

      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );
      mapsettings.setLayers( {layer.get()} );

      QgsMultiRenderChecker renderchecker;
      renderchecker.setMapSettings( mapsettings );
      renderchecker.setControlName( QStringLiteral( "expected_rulebased_group_else" ) );
      const bool res = renderchecker.runTest( QStringLiteral( "rulebased_group_else" ) );
      if ( !res )
        mReport += renderchecker.report();
      QVERIFY( res );
    }

    void testWillRenderFeatureNestedElse()
    {
      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      r3->appendChild( r1 );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer( rootrule );
      layer->setRenderer( renderer );
      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );
      mapsettings.setLayers( {layer.get()} );

      QgsFeature ft = layer->getFeature( 0 );

      QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mapsettings );
      ctx.expressionContext().setFeature( ft );

      // Render with else rule and all activated
      renderer->startRender( ctx, layer->fields() );
      QVERIFY( renderer->willRenderFeature( ft, ctx ) );
      renderer->stopRender( ctx );

      // Render with else rule where else is deactivated
      renderer->rootRule()->children()[1]->setActive( false );
      renderer->startRender( ctx, layer->fields() );
      QVERIFY( !renderer->willRenderFeature( ft, ctx ) );
      renderer->stopRender( ctx );
    }

    void testWillRenderFeatureTwoElse()
    {
      // Regression #21287, also test rulesForFeature since there were no tests any where and I've found a couple of issues

      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 200" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 1000, 100000000, "ELSE" ); // < match this!
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 1, 999, "ELSE" );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer( rootrule );
      layer->setRenderer( renderer );
      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );
      mapsettings.setLayers( {layer.get()} );

      QgsFeature f = layer->getFeature( 0 ); // 'id' = 1

      QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mapsettings );
      ctx.expressionContext().setFeature( f );

      renderer->startRender( ctx, layer->fields() ); // build mActiveChildren
      QVERIFY( renderer->willRenderFeature( f, ctx ) );
      renderer->stopRender( ctx );

      // No scale in context? All ELSE rules should match
      QgsRenderContext noScaleContext;
      noScaleContext.setRendererScale( 0 );
      renderer->startRender( noScaleContext, layer->fields() ); // build mActiveChildren
      QCOMPARE( renderer->rootRule()->rulesForFeature( f, &noScaleContext ).size(), 2 );
      QVERIFY( renderer->rootRule()->rulesForFeature( f, &noScaleContext ).contains( r2 ) );
      QVERIFY( renderer->rootRule()->rulesForFeature( f, &noScaleContext ).contains( r3 ) );
      renderer->stopRender( noScaleContext );

      // With context: only the matching one
      renderer->startRender( ctx, layer->fields() );
      QCOMPARE( renderer->rootRule()->rulesForFeature( f, &ctx ).size(), 1 );
      QCOMPARE( renderer->rootRule()->rulesForFeature( f, &ctx )[0], r2 );
      renderer->stopRender( ctx );
    }

    void testUsedAttributes()
    {
      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 200" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 1000, 100000000, "ELSE" );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );

      std::unique_ptr< QgsRuleBasedRenderer > renderer = std::make_unique< QgsRuleBasedRenderer >( rootrule );

      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );

      QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mapsettings );
      QCOMPARE( renderer->usedAttributes( ctx ), QSet<QString> { QStringLiteral( "id" )} );
    }

    void testPointsUsedAttributes()
    {
      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );

      // Create rulebased style
      QgsMarkerSymbol *sym1 = new QgsMarkerSymbol();
      QgsSimpleMarkerSymbolLayer *l1 = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Triangle, 5 );
      l1->setColor( QColor( 255, 0, 0 ) );
      l1->setStrokeStyle( Qt::NoPen );
      l1->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromField( QStringLiteral( "Heading" ) ) );
      sym1->changeSymbolLayer( 0, l1 );

      QgsMarkerSymbol *sym2 = new QgsMarkerSymbol();
      QgsSimpleMarkerSymbolLayer *l2 = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Triangle, 5 );
      l2->setColor( QColor( 0, 255, 0 ) );
      l2->setStrokeStyle( Qt::NoPen );
      l2->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromField( QStringLiteral( "Heading" ) ) );
      sym2->changeSymbolLayer( 0, l2 );

      QgsMarkerSymbol *sym3 = new QgsMarkerSymbol();
      QgsSimpleMarkerSymbolLayer *l3 = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Triangle, 5 );
      l3->setColor( QColor( 0, 0, 255 ) );
      l3->setStrokeStyle( Qt::NoPen );
      l3->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromField( QStringLiteral( "Heading" ) ) );
      sym3->changeSymbolLayer( 0, l3 );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"Class\" = 'B52'" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"Class\" = 'Biplane'" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "\"Class\" = 'Jet'" );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer( rootrule );
      layer->setRenderer( renderer );

      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -133, 22, -70, 52 ) );
      mapsettings.setLayers( { layer.get() } );

      QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mapsettings );
      ctx.expressionContext().appendScope( layer->createExpressionContextScope() );

      // for symbol layer
      QCOMPARE( l1->usedAttributes( ctx ), QSet<QString>( {"Heading"} ) );
      // for symbol
      QCOMPARE( sym1->usedAttributes( ctx ), QSet<QString>( {"Heading"} ) );
      // for symbol renderer
      QCOMPARE( renderer->usedAttributes( ctx ), QSet<QString>( {"Class", "Heading"} ) );
    }

    void testFeatureCount()
    {
      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      QgsRuleBasedRenderer::Rule *rootrule = new QgsRuleBasedRenderer::Rule( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer( rootrule );
      layer->setRenderer( renderer );
      QgsMapSettings mapsettings;
      mapsettings.setOutputSize( QSize( 400, 400 ) );
      mapsettings.setOutputDpi( 96 );
      mapsettings.setExtent( QgsRectangle( -163, 22, -70, 52 ) );
      mapsettings.setLayers( {layer.get()} );

      QgsFeature ft = layer->getFeature( 2 ); // 'id' = 3 => ELSE

      QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mapsettings );
      ctx.expressionContext().setFeature( ft );

      QgsVectorLayerFeatureCounter *counter = layer->countSymbolFeatures();
      counter->waitForFinished();

      renderer->startRender( ctx, layer->fields() );
      QgsRuleBasedRenderer::Rule *elseRule = nullptr;
      for ( QgsRuleBasedRenderer::Rule *rule : renderer->rootRule()->children() )
      {
        if ( rule->filterExpression() == QLatin1String( "ELSE" ) )
        {
          elseRule = rule;
          break;
        }
      }
      QVERIFY( elseRule );

      const long long count = counter->featureCount( elseRule->ruleKey() );
      QCOMPARE( count, 1LL );
    }

    void testRefineWithCategories()
    {
      // Test refining rule with categories (refs #10815)

      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      // First, try with a field based category (id)
      QList< QgsRendererCategory > cats;
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "id 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), QString() ) );
      cats.append( QgsRendererCategory( QVariant(), new QgsMarkerSymbol(), QString() ) );
      std::unique_ptr< QgsCategorizedSymbolRenderer > c = std::make_unique< QgsCategorizedSymbolRenderer >( "id", cats );

      QgsRuleBasedRenderer::refineRuleCategories( r2, c.get() );
      QCOMPARE( r2->children()[0]->filterExpression(), "\"id\" = 1" );
      QCOMPARE( r2->children()[1]->filterExpression(), "\"id\" = 2" );
      QCOMPARE( r2->children()[2]->filterExpression(), "\"id\" IS NULL" );
      QCOMPARE( r2->children()[0]->label(), "id 1" );
      QCOMPARE( r2->children()[1]->label(), "2" );
      QCOMPARE( r2->children()[2]->label(), QString() );

      // Next try with an expression based category
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "result 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "result 2" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "id + 1", cats );

      QgsRuleBasedRenderer::refineRuleCategories( r1, c.get() );
      QCOMPARE( r1->children()[0]->filterExpression(), "id + 1 = 1" );
      QCOMPARE( r1->children()[1]->filterExpression(), "id + 1 = 2" );
      QCOMPARE( r1->children()[0]->label(), "result 1" );
      QCOMPARE( r1->children()[1]->label(), "result 2" );

      // Last try with an expression which is just a quoted field name
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "result 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "result 2" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "\"id\"", cats );

      QgsRuleBasedRenderer::refineRuleCategories( r3, c.get() );
      QCOMPARE( r3->children()[0]->filterExpression(), "\"id\" = 1" );
      QCOMPARE( r3->children()[1]->filterExpression(), "\"id\" = 2" );
      QCOMPARE( r3->children()[0]->label(), "result 1" );
      QCOMPARE( r3->children()[1]->label(), "result 2" );
    }

    void testRefineWithRanges()
    {
      // Test refining rule with ranges (refs #10815)

      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      // First, try with a field based category (id)
      QList< QgsRendererRange > ranges;
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      std::unique_ptr< QgsGraduatedSymbolRenderer > c = std::make_unique< QgsGraduatedSymbolRenderer >( "id", ranges );

      QgsRuleBasedRenderer::refineRuleRanges( r2, c.get() );
      QCOMPARE( r2->children()[0]->filterExpression(), "\"id\" >= 0.0000 AND \"id\" <= 1.0000" );
      QCOMPARE( r2->children()[1]->filterExpression(), "\"id\" > 1.0000 AND \"id\" <= 2.0000" );

      // Next try with an expression based range
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "id / 2", ranges );

      QgsRuleBasedRenderer::refineRuleRanges( r1, c.get() );
      QCOMPARE( r1->children()[0]->filterExpression(), "(id / 2) >= 0.0000 AND (id / 2) <= 1.0000" );
      QCOMPARE( r1->children()[1]->filterExpression(), "(id / 2) > 1.0000 AND (id / 2) <= 2.0000" );

      // Last try with an expression which is just a quoted field name
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "\"id\"", ranges );

      QgsRuleBasedRenderer::refineRuleRanges( r3, c.get() );
      QCOMPARE( r3->children()[0]->filterExpression(), "\"id\" >= 0.0000 AND \"id\" <= 1.0000" );
      QCOMPARE( r3->children()[1]->filterExpression(), "\"id\" > 1.0000 AND \"id\" <= 2.0000" );
    }

    void testConvertFromCategorisedRenderer()
    {
      // Test converting categorised renderer to rule based

      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      std::unique_ptr<QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule>( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      // First, try with a field based category (id)
      QList< QgsRendererCategory > cats;
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "id 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "id 2" ) );
      cats.append( QgsRendererCategory( "a\'b", new QgsMarkerSymbol(), "id a'b" ) );
      cats.append( QgsRendererCategory( "a\nb", new QgsMarkerSymbol(), "id a\\nb" ) );
      cats.append( QgsRendererCategory( "a\\b", new QgsMarkerSymbol(), "id a\\\\b" ) );
      cats.append( QgsRendererCategory( "a\tb", new QgsMarkerSymbol(), "id a\\tb" ) );
      cats.append( QgsRendererCategory( QVariantList( {"c", "d"} ), new QgsMarkerSymbol(), "c/d" ) );
      std::unique_ptr< QgsCategorizedSymbolRenderer > c = std::make_unique< QgsCategorizedSymbolRenderer >( "id", cats );

      std::unique_ptr< QgsRuleBasedRenderer > r( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 7 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" = 2" );
      QCOMPARE( r->rootRule()->children()[2]->filterExpression(), "\"id\" = \'a\'\'b\'" );
      QCOMPARE( r->rootRule()->children()[3]->filterExpression(), "\"id\" = \'a\\nb\'" );
      QCOMPARE( r->rootRule()->children()[4]->filterExpression(), "\"id\" = \'a\\\\b\'" );
      QCOMPARE( r->rootRule()->children()[5]->filterExpression(), "\"id\" = \'a\\tb\'" );
      QCOMPARE( r->rootRule()->children()[6]->filterExpression(), "\"id\" IN (\'c\',\'d\')" );


      // Next try with an expression based category
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "result 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "result 2" ) );
      cats.append( QgsRendererCategory( QVariantList( {3, 4} ), new QgsMarkerSymbol(), "result 3/4" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "id + 1", cats );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 3 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "id + 1 = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "id + 1 = 2" );
      QCOMPARE( r->rootRule()->children()[2]->filterExpression(), "id + 1 IN (3,4)" );

      // Last try with an expression which is just a quoted field name
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "result 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "result 2" ) );
      cats.append( QgsRendererCategory( QVariantList( {3, 4} ), new QgsMarkerSymbol(), "result 3/4" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "\"id\"", cats );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" = 2" );
      QCOMPARE( r->rootRule()->children()[2]->filterExpression(), "\"id\" IN (3,4)" );

      // Next try with a complex name
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "fa_cy-fie+ld 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "fa_cy-fie+ld 2" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "fa_cy-fie+ld", cats );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"fa_cy-fie+ld\" = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"fa_cy-fie+ld\" = 2" );
    }

    void testConvertFromCategorisedRendererNoLayer()
    {
      // Test converting categorised renderer to rule based

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      // First, try with a field based category (id)
      QList< QgsRendererCategory > cats;
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "id 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "id 2" ) );
      cats.append( QgsRendererCategory( "a\'b", new QgsMarkerSymbol(), "id a'b" ) );
      cats.append( QgsRendererCategory( "a\nb", new QgsMarkerSymbol(), "id a\\nb" ) );
      cats.append( QgsRendererCategory( "a\\b", new QgsMarkerSymbol(), "id a\\\\b" ) );
      cats.append( QgsRendererCategory( "a\tb", new QgsMarkerSymbol(), "id a\\tb" ) );
      cats.append( QgsRendererCategory( QVariantList( {"c", "d"} ), new QgsMarkerSymbol(), "c/d" ) );
      std::unique_ptr< QgsCategorizedSymbolRenderer > c = std::make_unique< QgsCategorizedSymbolRenderer >( "id", cats );

      std::unique_ptr< QgsRuleBasedRenderer > r( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 7 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" = 2" );
      QCOMPARE( r->rootRule()->children()[2]->filterExpression(), "\"id\" = \'a\'\'b\'" );
      QCOMPARE( r->rootRule()->children()[3]->filterExpression(), "\"id\" = \'a\\nb\'" );
      QCOMPARE( r->rootRule()->children()[4]->filterExpression(), "\"id\" = \'a\\\\b\'" );
      QCOMPARE( r->rootRule()->children()[5]->filterExpression(), "\"id\" = \'a\\tb\'" );
      QCOMPARE( r->rootRule()->children()[6]->filterExpression(), "\"id\" IN (\'c\',\'d\')" );


      // Next try with an expression based category
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "result 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "result 2" ) );
      cats.append( QgsRendererCategory( QVariantList( {3, 4} ), new QgsMarkerSymbol(), "result 3/4" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "id + 1", cats );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 3 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "id + 1 = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "id + 1 = 2" );
      QCOMPARE( r->rootRule()->children()[2]->filterExpression(), "id + 1 IN (3,4)" );

      // Last try with an expression which is just a quoted field name
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "result 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "result 2" ) );
      cats.append( QgsRendererCategory( QVariantList( {3, 4} ), new QgsMarkerSymbol(), "result 3/4" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "\"id\"", cats );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" = 2" );
      QCOMPARE( r->rootRule()->children()[2]->filterExpression(), "\"id\" IN (3,4)" );

      // Next try with a complex name -- in this case since we don't have a layer or
      // actual field names available, we must assume the complex field name is actually an expression
      cats.clear();
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "fa_cy-fie+ld 1" ) );
      cats.append( QgsRendererCategory( 2, new QgsMarkerSymbol(), "fa_cy-fie+ld 2" ) );
      c = std::make_unique< QgsCategorizedSymbolRenderer >( "fa_cy-fie+ld", cats );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "fa_cy-fie+ld = 1" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "fa_cy-fie+ld = 2" );
    }

    void testConvertFromGraduatedRenderer()
    {
      // Test converting graduated renderer to rule based

      const QString shpFile = TEST_DATA_DIR + QStringLiteral( "/rectangles.shp" );
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( shpFile, QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
      QVERIFY( layer->isValid() );
      QgsField vfield = QgsField( QStringLiteral( "fa_cy-fie+ld" ), QVariant::Int );
      layer->addExpressionField( QStringLiteral( "\"id\"" ), vfield );

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      // First, try with a field based category (id)
      QList< QgsRendererRange > ranges;
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      std::unique_ptr< QgsGraduatedSymbolRenderer > c = std::make_unique< QgsGraduatedSymbolRenderer >( "id", ranges );

      std::unique_ptr< QgsRuleBasedRenderer > r( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" >= 0.000000 AND \"id\" <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" > 1.000000 AND \"id\" <= 2.000000" );

      // Next try with an expression based range
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "id / 2", ranges );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "(id / 2) >= 0.000000 AND (id / 2) <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "(id / 2) > 1.000000 AND (id / 2) <= 2.000000" );

      // Last try with an expression which is just a quoted field name
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "\"id\"", ranges );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" >= 0.000000 AND \"id\" <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" > 1.000000 AND \"id\" <= 2.000000" );

      // Next try with a complex name
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "fa_cy-fie+ld", ranges );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get(), layer.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"fa_cy-fie+ld\" >= 0.000000 AND \"fa_cy-fie+ld\" <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"fa_cy-fie+ld\" > 1.000000 AND \"fa_cy-fie+ld\" <= 2.000000" );
    }

    void testConvertFromGraduatedRendererNoLayer()
    {
      // Test converting graduated renderer to rule based

      // Create rulebased style
      QgsSymbol *sym1 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#fdbf6f"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym2 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#71bd6c"}, {"outline_color", "black"}} ) );
      QgsSymbol *sym3 = QgsFillSymbol::createSimple( QVariantMap( {{"color", "#1f78b4"}, {"outline_color", "black"}} ) );

      QgsRuleBasedRenderer::Rule *r1 = new QgsRuleBasedRenderer::Rule( sym1, 0, 0, "\"id\" = 1" );
      QgsRuleBasedRenderer::Rule *r2 = new QgsRuleBasedRenderer::Rule( sym2, 0, 0, "\"id\" = 2" );
      QgsRuleBasedRenderer::Rule *r3 = new QgsRuleBasedRenderer::Rule( sym3, 0, 0, "ELSE" );

      std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
      rootrule->appendChild( r1 );
      rootrule->appendChild( r2 );
      rootrule->appendChild( r3 );

      // First, try with a field based category (id)
      QList< QgsRendererRange > ranges;
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      std::unique_ptr< QgsGraduatedSymbolRenderer > c = std::make_unique< QgsGraduatedSymbolRenderer >( "id", ranges );

      std::unique_ptr< QgsRuleBasedRenderer > r( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" >= 0.000000 AND \"id\" <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" > 1.000000 AND \"id\" <= 2.000000" );

      // Next try with an expression based range
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "id / 2", ranges );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "(id / 2) >= 0.000000 AND (id / 2) <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "(id / 2) > 1.000000 AND (id / 2) <= 2.000000" );

      // Last try with an expression which is just a quoted field name
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "\"id\"", ranges );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "\"id\" >= 0.000000 AND \"id\" <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "\"id\" > 1.000000 AND \"id\" <= 2.000000" );

      // Next try with a complex name -- in this case since we don't have a layer or
      // actual field names available, we must assume the complex field name is actually an expression
      ranges.clear();
      ranges.append( QgsRendererRange( 0, 1, new QgsMarkerSymbol(), "0-1" ) );
      ranges.append( QgsRendererRange( 1, 2, new QgsMarkerSymbol(), "1-2" ) );
      c = std::make_unique< QgsGraduatedSymbolRenderer >( "fa_cy-fie+ld", ranges );

      r.reset( QgsRuleBasedRenderer::convertFromRenderer( c.get() ) );
      QCOMPARE( r->rootRule()->children().size(), 2 );
      QCOMPARE( r->rootRule()->children()[0]->filterExpression(), "(fa_cy-fie+ld) >= 0.000000 AND (fa_cy-fie+ld) <= 1.000000" );
      QCOMPARE( r->rootRule()->children()[1]->filterExpression(), "(fa_cy-fie+ld) > 1.000000 AND (fa_cy-fie+ld) <= 2.000000" );
    }

    void testConvertFromEmbedded()
    {
      // Test converting an embedded symbol renderer to a rule based renderer
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "points" ), QStringLiteral( "memory" ) );
      QVERIFY( layer->isValid() );

      QgsFeature f;
      f.setGeometry( QgsGeometry::fromWkt( "Point(-100 30)" ) );
      f.setEmbeddedSymbol(
      QgsMarkerSymbol::createSimple( {{"name", "triangle"}, {"size", 10}, {"color", "#ff0000"}, {"outline_style", "no"}} ) );
      QVERIFY( layer->dataProvider()->addFeature( f ) );
      f.setGeometry( QgsGeometry::fromWkt( "Point(-110 40)" ) );
      f.setEmbeddedSymbol(
      QgsMarkerSymbol::createSimple( {{"name", "square"}, { "size", 7}, { "color", "#00ff00"}, { "outline_style", "no"} } ) );
      QVERIFY( layer->dataProvider()->addFeature( f ) );
      f.setGeometry( QgsGeometry::fromWkt( "Point(-90 50)" ) );
      f.setEmbeddedSymbol( nullptr );
      QVERIFY( layer->dataProvider()->addFeature( f ) );

      QgsEmbeddedSymbolRenderer *renderer = new QgsEmbeddedSymbolRenderer( QgsMarkerSymbol::createSimple( {{"name", "star"}, {"size", 10}, {"color", "#ff00ff"}, {"outline_style", "no"}} ) );
      layer->setRenderer( renderer );

      std::unique_ptr< QgsRuleBasedRenderer > rule_based( QgsRuleBasedRenderer::convertFromRenderer( renderer, layer.get() ) );
      QCOMPARE( rule_based->rootRule()->children().size(), 3 );
      QgsRuleBasedRenderer::Rule *rule_0 = rule_based->rootRule()->children()[0];
      QCOMPARE( rule_0->filterExpression(), "$id=1" );
      QCOMPARE( rule_0->label(), "1" );
      QCOMPARE( rule_0->symbol()->color().name(), "#ff0000" );
      QgsRuleBasedRenderer::Rule *rule_1 = rule_based->rootRule()->children()[1];
      QCOMPARE( rule_1->filterExpression(), "$id=2" );
      QCOMPARE( rule_1->label(), "2" );
      QCOMPARE( rule_1->symbol()->color().name(), "#00ff00" );
      QgsRuleBasedRenderer::Rule *rule_2 = rule_based->rootRule()->children()[2];
      QCOMPARE( rule_2->filterExpression(), "ELSE" );
      QCOMPARE( rule_2->label(), "All other features" );
      QCOMPARE( rule_2->symbol()->color().name(), "#ff00ff" );
    }

    void testNullsCount()
    {
      std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:4326&field=number:int" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
      QVERIFY( layer->isValid() );

      QgsFeature f( layer->fields() );
      f.setAttribute( 0, 0 );
      f.setGeometry( QgsGeometry::fromWkt( "point(7 45)" ) );
      QVERIFY( layer->dataProvider()->addFeature( f ) );
      f = QgsFeature( layer->fields() );
      f.setGeometry( QgsGeometry::fromWkt( "point(7 45)" ) );
      f.setAttribute( 0, 1 );
      QVERIFY( layer->dataProvider()->addFeature( f ) );
      f.setGeometry( QgsGeometry::fromWkt( "point(7 45)" ) );
      f = QgsFeature( layer->fields() );
      QVERIFY( layer->dataProvider()->addFeature( f ) );

      QList< QgsRendererCategory > cats;
      cats.append( QgsRendererCategory( 1, new QgsMarkerSymbol(), "one" ) );
      cats.append( QgsRendererCategory( 0, new QgsMarkerSymbol(), "zero" ) );
      cats.append( QgsRendererCategory( QVariant(), new QgsMarkerSymbol(), "NULL" ) );
      QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer( "number", cats );
      layer->setRenderer( renderer );

      QgsVectorLayerFeatureCounter *counter = layer->countSymbolFeatures();
      counter->waitForFinished();

      QCOMPARE( counter->featureCount( "0" ), 1LL );
      QCOMPARE( counter->featureCount( "1" ), 1LL );
      QCOMPARE( counter->featureCount( "2" ), 1LL );
    }

    void testLegendKeyToExpression()
    {
      QgsRuleBasedRenderer::Rule *rootRule = new QgsRuleBasedRenderer::Rule( nullptr );
      std::unique_ptr< QgsRuleBasedRenderer > renderer = std::make_unique< QgsRuleBasedRenderer >( rootRule );

      bool ok = false;
      QString exp = renderer->legendKeyToExpression( "xxxx", nullptr, ok );
      QVERIFY( !ok );

      exp = renderer->legendKeyToExpression( rootRule->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "TRUE" );

      QgsRuleBasedRenderer::Rule *rule2 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, "\"field_name\" = 5" );
      QgsRuleBasedRenderer::Rule *rule3 = new QgsRuleBasedRenderer::Rule( nullptr, 2000, 0, "\"field_name\" = 6" );
      QgsRuleBasedRenderer::Rule *rule4 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 1000, "\"field_name\" = 7" );
      QgsRuleBasedRenderer::Rule *rule5 = new QgsRuleBasedRenderer::Rule( nullptr, 1000, 3000 );

      rootRule->appendChild( rule2 );
      rootRule->appendChild( rule3 );
      rootRule->appendChild( rule4 );
      rootRule->appendChild( rule5 );

      exp = renderer->legendKeyToExpression( rootRule->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "TRUE" );

      exp = renderer->legendKeyToExpression( rule2->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "\"field_name\" = 5" );

      exp = renderer->legendKeyToExpression( rule3->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(\"field_name\" = 6) AND (@map_scale >= 2000)" );

      exp = renderer->legendKeyToExpression( rule4->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(\"field_name\" = 7) AND (@map_scale <= 1000)" );

      exp = renderer->legendKeyToExpression( rule5->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(@map_scale <= 3000) AND (@map_scale >= 1000)" );

      QgsRuleBasedRenderer::Rule *rule6 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, "\"field_name\" = 'a'" );
      rule4->appendChild( rule6 );

      exp = renderer->legendKeyToExpression( rule6->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(\"field_name\" = 'a') AND ((\"field_name\" = 7) AND (@map_scale <= 1000))" );

      // group only rule
      QgsRuleBasedRenderer::Rule *rule7 = new QgsRuleBasedRenderer::Rule( nullptr );
      rule3->appendChild( rule7 );

      QgsRuleBasedRenderer::Rule *rule8 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, "\"field_name\" = 'c'" );
      rule7->appendChild( rule8 );

      exp = renderer->legendKeyToExpression( rule7->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(\"field_name\" = 6) AND (@map_scale >= 2000)" );

      exp = renderer->legendKeyToExpression( rule8->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(\"field_name\" = 'c') AND ((\"field_name\" = 6) AND (@map_scale >= 2000))" );

      // else rules
      rootRule = new QgsRuleBasedRenderer::Rule( nullptr );
      renderer = std::make_unique< QgsRuleBasedRenderer >( rootRule );

      rule2 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, "\"field_name\" = 5" );
      rule3 = new QgsRuleBasedRenderer::Rule( nullptr, 2000, 0, "\"field_name\" = 6" );
      rule4 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, QString(), QString(), QString(), true );

      rootRule->appendChild( rule2 );
      rootRule->appendChild( rule3 );
      rootRule->appendChild( rule4 );

      exp = renderer->legendKeyToExpression( rootRule->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "TRUE" );

      exp = renderer->legendKeyToExpression( rule2->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "\"field_name\" = 5" );

      exp = renderer->legendKeyToExpression( rule3->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(\"field_name\" = 6) AND (@map_scale >= 2000)" );

      exp = renderer->legendKeyToExpression( rule4->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "NOT ((\"field_name\" = 5) OR ((\"field_name\" = 6) AND (@map_scale >= 2000)))" );

      rule5 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, "\"field_name\" = 11" );
      rule4->appendChild( rule5 );

      exp = renderer->legendKeyToExpression( rule5->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(\"field_name\" = 11) AND (NOT ((\"field_name\" = 5) OR ((\"field_name\" = 6) AND (@map_scale >= 2000))))" );

      // isolated ELSE rule, with no siblings

      rootRule = new QgsRuleBasedRenderer::Rule( nullptr );
      renderer = std::make_unique< QgsRuleBasedRenderer >( rootRule );

      rule2 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, "\"field_name\" = 5" );
      rule3 = new QgsRuleBasedRenderer::Rule( nullptr, 2000, 0, "\"field_name\" = 6" );

      rootRule->appendChild( rule2 );
      rootRule->appendChild( rule3 );

      rule4 = new QgsRuleBasedRenderer::Rule( nullptr, 0, 0, QString(), QString(), QString(), true );
      rule3->appendChild( rule4 );

      exp = renderer->legendKeyToExpression( rule4->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "(TRUE) AND ((\"field_name\" = 6) AND (@map_scale >= 2000))" );
    }

    void testElseRuleSld()
    {
      QgsRuleBasedRenderer::Rule *rootRule = new QgsRuleBasedRenderer::Rule( nullptr );
      std::unique_ptr< QgsRuleBasedRenderer > renderer = std::make_unique< QgsRuleBasedRenderer >( rootRule );

      QgsRuleBasedRenderer::Rule *rule1 = new QgsRuleBasedRenderer::Rule( QgsSymbol::defaultSymbol( QgsWkbTypes::GeometryType::PointGeometry ), 0, 0, "\"field_name\" = 1" );
      QgsRuleBasedRenderer::Rule *rule2 = new QgsRuleBasedRenderer::Rule( QgsSymbol::defaultSymbol( QgsWkbTypes::GeometryType::PointGeometry ), 0, 0, "\"field_name\" = 6" );
      QgsRuleBasedRenderer::Rule *ruleElse = new QgsRuleBasedRenderer::Rule( QgsSymbol::defaultSymbol( QgsWkbTypes::GeometryType::PointGeometry ), 0, 0, "ELSE" );

      Q_ASSERT( ruleElse->isElse() );

      rootRule->appendChild( rule1 );
      rootRule->appendChild( rule2 );
      rootRule->appendChild( ruleElse );

      bool ok;

      QString exp = renderer->legendKeyToExpression( ruleElse->ruleKey(), nullptr, ok );
      QVERIFY( ok );
      QCOMPARE( exp, "NOT ((\"field_name\" = 1) OR (\"field_name\" = 6))" );

      QgsFields fields;
      std::unique_ptr<QgsVectorLayer> vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?crs=epsg:4326&field=field_name:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
      vl->setRenderer( renderer.release() );
      QString error;
      QDomDocument dom;
      vl->exportSldStyle( dom, error );

      const QString sld = dom.toString();

      Q_ASSERT( sld.contains( QStringLiteral( "<se:ElseFilter" ) ) );

      QTemporaryFile sldFile;
      sldFile.open();
      sldFile.write( sld.toUtf8() );
      sldFile.close();

      // Recreate the test layer for round trip test
      vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?crs=epsg:4326&field=field_name:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
      vl->loadSldStyle( sldFile.fileName(), ok );

      Q_ASSERT( ok );

      QgsRuleBasedRenderer *renderer2 = static_cast<QgsRuleBasedRenderer *>( vl->renderer() );
      ruleElse = renderer2->rootRule()->children().last();
      Q_ASSERT( ruleElse->isElse() );

    }


  private:
    void xml2domElement( const QString &testFile, QDomDocument &doc )
    {
      const QString fileName = QStringLiteral( TEST_DATA_DIR ) + '/' + testFile;
      QFile f( fileName );
      const bool fileOpen = f.open( QIODevice::ReadOnly );
      QVERIFY( fileOpen );

      QString msg;
      int line, col;
      const bool parse = doc.setContent( &f, &msg, &line, &col );
      QVERIFY( parse );
    }

    void check_tree_valid( QgsRuleBasedRenderer::Rule *root )
    {
      // root must always exist (although it does not have children)
      QVERIFY( root );
      // and does not have a parent
      QVERIFY( !root->parent() );

      for ( QgsRuleBasedRenderer::Rule *node : root->children() )
        check_non_root_rule( node );
    }

    void check_non_root_rule( QgsRuleBasedRenderer::Rule *node )
    {
      qDebug() << node->dump();
      // children must not be nullptr
      QVERIFY( node );
      // and must have a parent
      QVERIFY( node->parent() );
      // check that all children are okay
      for ( QgsRuleBasedRenderer::Rule *child : node->children() )
        check_non_root_rule( child );
    }

};

QGSTEST_MAIN( TestQgsRuleBasedRenderer )

#include "testqgsrulebasedrenderer.moc"
