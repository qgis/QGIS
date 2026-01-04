/***************************************************************************
    testqgsmaplayerstylemanager.cpp
    ---------------------
    begin                : January 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgslinesymbol.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgssinglesymbolrenderer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QObject>

class TestQgsMapLayerStyleManager : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapLayerStyleManager() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testDefault();
    void testStyle();
    void testReadWrite();
    void testSwitchingStyles();
    void testCopyStyles();

  private:
    QgsVectorLayer *mVL = nullptr;
};

void TestQgsMapLayerStyleManager::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsMapLayerStyleManager::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapLayerStyleManager::init()
{
  mVL = new QgsVectorLayer( u"LineString"_s, u"Line Layer"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( mVL );
}

void TestQgsMapLayerStyleManager::cleanup()
{
  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsMapLayerStyleManager::testDefault()
{
  QgsMapLayerStyleManager *mgr = mVL->styleManager();
  QVERIFY( mgr );

  QCOMPARE( mgr->styles().count(), 1 );
  QCOMPARE( mgr->style( u"default"_s ).isValid(), true );
}

void TestQgsMapLayerStyleManager::testStyle()
{
  const QgsMapLayerStyle st;
  QCOMPARE( st.isValid(), false );

  QgsLineSymbol *sym1 = new QgsLineSymbol();
  sym1->setColor( Qt::magenta );
  mVL->setRenderer( new QgsSingleSymbolRenderer( sym1 ) );

  QgsMapLayerStyle st1;
  st1.readFromLayer( mVL );
  QCOMPARE( st1.isValid(), true );

  qDebug( "CNT-1: %s", st1.xmlData().toLatin1().data() );

  QgsLineSymbol *sym2 = new QgsLineSymbol();
  sym2->setColor( Qt::red );
  mVL->setRenderer( new QgsSingleSymbolRenderer( sym2 ) );

  QgsMapLayerStyle st2;
  st2.readFromLayer( mVL );

  qDebug( "CNT-2: %s", st2.xmlData().toLatin1().data() );

  st1.writeToLayer( mVL );

  QgsSingleSymbolRenderer *r1 = dynamic_cast<QgsSingleSymbolRenderer *>( mVL->renderer() );
  QVERIFY( r1 );
  QCOMPARE( r1->symbol()->color(), QColor( Qt::magenta ) );

  st2.writeToLayer( mVL );

  QgsSingleSymbolRenderer *r2 = dynamic_cast<QgsSingleSymbolRenderer *>( mVL->renderer() );
  QVERIFY( r2 );
  QCOMPARE( r2->symbol()->color(), QColor( Qt::red ) );
}


void TestQgsMapLayerStyleManager::testReadWrite()
{
  QgsSingleSymbolRenderer *r0 = dynamic_cast<QgsSingleSymbolRenderer *>( mVL->renderer() );
  QVERIFY( r0 );
  r0->symbol()->setColor( Qt::red );

  // create and populate the manager with one more style

  QgsMapLayerStyleManager sm0( mVL );

  sm0.addStyleFromLayer( u"blue"_s );
  sm0.setCurrentStyle( u"blue"_s );
  QgsSingleSymbolRenderer *r1 = dynamic_cast<QgsSingleSymbolRenderer *>( mVL->renderer() );
  QVERIFY( r1 );
  r1->symbol()->setColor( Qt::blue );

  // read and write

  QDomDocument doc;
  QDomElement mgrElem = doc.createElement( u"map-layer-style-manager"_s );
  doc.appendChild( mgrElem );
  sm0.writeXml( mgrElem );

  QString xml;
  QTextStream ts( &xml );
  doc.save( ts, 2 );
  qDebug( "%s", xml.toLatin1().data() );

  QgsMapLayerStyleManager sm1( mVL );
  sm1.readXml( mgrElem );

  QCOMPARE( sm1.styles().count(), 2 );
  QCOMPARE( sm1.style( u"default"_s ).isValid(), true );
  QCOMPARE( sm1.style( "blue" ).isValid(), true );
  QCOMPARE( sm1.currentStyle(), QString( "blue" ) );

  // now use the default style - the symbol should get red color
  sm1.setCurrentStyle( u"default"_s );

  QgsSingleSymbolRenderer *r2 = dynamic_cast<QgsSingleSymbolRenderer *>( mVL->renderer() );
  QVERIFY( r2 );
  QCOMPARE( r2->symbol()->color(), QColor( Qt::red ) );
}

static void _setVLColor( QgsVectorLayer *vl, const QColor &c )
{
  QgsSingleSymbolRenderer *renderer = dynamic_cast<QgsSingleSymbolRenderer *>( vl->renderer() );
  if ( renderer )
    renderer->symbol()->setColor( c );
}

static QColor _getVLColor( QgsVectorLayer *vl )
{
  QgsSingleSymbolRenderer *renderer = dynamic_cast<QgsSingleSymbolRenderer *>( vl->renderer() );
  if ( renderer )
    return renderer->symbol()->color();
  else
    return QColor();
}

void TestQgsMapLayerStyleManager::testSwitchingStyles()
{
  _setVLColor( mVL, Qt::red );

  mVL->styleManager()->addStyleFromLayer( u"s1"_s );
  mVL->styleManager()->setCurrentStyle( u"s1"_s );

  QCOMPARE( mVL->styleManager()->currentStyle(), QString( "s1" ) );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  _setVLColor( mVL, Qt::green );

  mVL->styleManager()->setCurrentStyle( u"default"_s );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  mVL->styleManager()->setCurrentStyle( u"s1"_s );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::green ) );

  _setVLColor( mVL, Qt::blue );

  mVL->styleManager()->setCurrentStyle( u"default"_s );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  mVL->styleManager()->setCurrentStyle( u"s1"_s );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::blue ) );
}

void TestQgsMapLayerStyleManager::testCopyStyles()
{
  auto lines = std::make_unique<QgsVectorLayer>( u"LineString"_s, u"Line Layer"_s, u"memory"_s );
  auto lines2 = std::make_unique<QgsVectorLayer>( u"LineString"_s, u"Line Layer"_s, u"memory"_s );

  QgsMapLayerStyleManager *sm = lines->styleManager();

  sm->addStyleFromLayer( u"style2"_s );

  QgsMapLayerStyleManager *sm2 = lines2->styleManager();

  sm2->copyStylesFrom( sm );
  sm2->addStyleFromLayer( "style3" );

  QVERIFY( sm2->styles().contains( "style2" ) );
  QVERIFY( sm2->styles().contains( "style3" ) );
  QVERIFY( sm2->styles().contains( "default" ) );
}


QGSTEST_MAIN( TestQgsMapLayerStyleManager )
#include "testqgsmaplayerstylemanager.moc"
