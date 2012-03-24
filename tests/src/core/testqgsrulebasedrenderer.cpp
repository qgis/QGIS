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
#include <QtTest>
#include <QDomDocument>
#include <QFile>
//header for class being tested
#include <qgsrulebasedrendererv2.h>

#include <qgsapplication.h>
#include <qgssymbolv2.h>
#include <qgsvectorlayer.h>

#if QT_VERSION < 0x40701
// See http://hub.qgis.org/issues/4284
Q_DECLARE_METATYPE( QVariant )
#endif

typedef QgsRuleBasedRendererV2::Rule RRule;

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

    void test_load_xml()
    {
      QDomDocument doc;
      xml2domElement( "rulebasedrenderer_simple.xml", doc );
      QDomElement elem = doc.documentElement();

      QgsRuleBasedRendererV2* r = static_cast<QgsRuleBasedRendererV2*>( QgsRuleBasedRendererV2::create( elem ) );
      QVERIFY( r );
      check_tree_valid( r->rootRule() );
      delete r;
    }

    void test_load_invalid_xml()
    {
      QDomDocument doc;
      xml2domElement( "rulebasedrenderer_invalid.xml", doc );
      QDomElement elem = doc.documentElement();

      QgsRuleBasedRendererV2* r = static_cast<QgsRuleBasedRendererV2*>( QgsRuleBasedRendererV2::create( elem ) );
      QVERIFY( r == NULL );
    }

    void test_willRenderFeature_symbolsForFeature()
    {
      // prepare features
      QgsVectorLayer* layer = new QgsVectorLayer( "point?field=fld:int", "x", "memory" );
      int idx = layer->fieldNameIndex( "fld" );
      QVERIFY( idx != -1 );
      QgsFeature f1; f1.addAttribute( idx, QVariant( 2 ) );
      QgsFeature f2; f2.addAttribute( idx, QVariant( 8 ) );
      QgsFeature f3; f3.addAttribute( idx, QVariant( 100 ) );

      // prepare renderer
      QgsSymbolV2* s1 = QgsSymbolV2::defaultSymbol( QGis::Point );
      QgsSymbolV2* s2 = QgsSymbolV2::defaultSymbol( QGis::Point );
      RRule* rootRule = new RRule( NULL );
      rootRule->appendChild( new RRule( s1, 0, 0, "fld >= 5 and fld <= 20" ) );
      rootRule->appendChild( new RRule( s2, 0, 0, "fld <= 10" ) );
      QgsRuleBasedRendererV2 r( rootRule );

      QVERIFY( r.capabilities() & QgsFeatureRendererV2::MoreSymbolsPerFeature );

      QgsRenderContext ctx; // dummy render context
      r.startRender( ctx, layer );

      // test willRenderFeature
      QVERIFY( r.willRenderFeature( f1 ) == true );
      QVERIFY( r.willRenderFeature( f2 ) == true );
      QVERIFY( r.willRenderFeature( f3 ) == false );

      // test symbolsForFeature
      QgsSymbolV2List lst1 = r.symbolsForFeature( f1 );
      QVERIFY( lst1.count() == 1 );
      QgsSymbolV2List lst2 = r.symbolsForFeature( f2 );
      QVERIFY( lst2.count() == 2 );
      QgsSymbolV2List lst3 = r.symbolsForFeature( f3 );
      QVERIFY( lst3.count() == 0 );

      r.stopRender( ctx );

      delete layer;
    }

  private:
    void xml2domElement( QString testFile, QDomDocument& doc )
    {
      QString fileName = QString( TEST_DATA_DIR ) + QDir::separator() + testFile;
      QFile f( fileName );
      bool fileOpen = f.open( QIODevice::ReadOnly );
      QVERIFY( fileOpen );

      QString msg;
      int line, col;
      bool parse = doc.setContent( &f, &msg, &line, &col );
      QVERIFY( parse );
    }

    void check_tree_valid( QgsRuleBasedRendererV2::Rule* root )
    {
      // root must always exist (although it does not have children)
      QVERIFY( root );
      // and does not have a parent
      QVERIFY( root->parent() == NULL );

      foreach( QgsRuleBasedRendererV2::Rule* node, root->children() )
      check_non_root_rule( node );
    }

    void check_non_root_rule( QgsRuleBasedRendererV2::Rule* node )
    {
      qDebug() << node->dump();
      // children must not be NULL
      QVERIFY( node );
      // and must have a parent
      QVERIFY( node->parent() );
      // check that all children are okay
      foreach( QgsRuleBasedRendererV2::Rule* child, node->children() )
      check_non_root_rule( child );
    }

};

QTEST_MAIN( TestQgsRuleBasedRenderer )

#include "moc_testqgsrulebasedrenderer.cxx"

