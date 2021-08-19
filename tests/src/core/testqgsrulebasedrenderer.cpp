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
//header for class being tested
#include <qgsrulebasedrenderer.h>

#include <qgsapplication.h>
#include <qgsreadwritecontext.h>
#include <qgssymbol.h>
#include <qgsvectorlayer.h>

typedef QgsRuleBasedRenderer::Rule RRule;

class TestQgsRuleBasedRenderer: public QObject
{
    Q_OBJECT
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

      QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "point?field=fld:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
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
