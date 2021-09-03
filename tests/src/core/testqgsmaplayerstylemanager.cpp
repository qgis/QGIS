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

#include "qgstest.h"
#include <QObject>

#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsmaplayerstylemanager.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"
#include "qgslinesymbol.h"

class TestQgsMapLayerStyleManager : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapLayerStyleManager() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

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
  mVL = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "Line Layer" ), QStringLiteral( "memory" ) );
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
  QCOMPARE( mgr->style( QStringLiteral( "default" ) ).isValid(), true );
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

  sm0.addStyleFromLayer( QStringLiteral( "blue" ) );
  sm0.setCurrentStyle( QStringLiteral( "blue" ) );
  QgsSingleSymbolRenderer *r1 = dynamic_cast<QgsSingleSymbolRenderer *>( mVL->renderer() );
  QVERIFY( r1 );
  r1->symbol()->setColor( Qt::blue );

  // read and write

  QDomDocument doc;
  QDomElement mgrElem = doc.createElement( QStringLiteral( "map-layer-style-manager" ) );
  doc.appendChild( mgrElem );
  sm0.writeXml( mgrElem );

  QString xml;
  QTextStream ts( &xml );
  doc.save( ts, 2 );
  qDebug( "%s", xml.toLatin1().data() );

  QgsMapLayerStyleManager sm1( mVL );
  sm1.readXml( mgrElem );

  QCOMPARE( sm1.styles().count(), 2 );
  QCOMPARE( sm1.style( QStringLiteral( "default" ) ).isValid(), true );
  QCOMPARE( sm1.style( "blue" ).isValid(), true );
  QCOMPARE( sm1.currentStyle(), QString( "blue" ) );

  // now use the default style - the symbol should get red color
  sm1.setCurrentStyle( QStringLiteral( "default" ) );

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

  mVL->styleManager()->addStyleFromLayer( QStringLiteral( "s1" ) );
  mVL->styleManager()->setCurrentStyle( QStringLiteral( "s1" ) );

  QCOMPARE( mVL->styleManager()->currentStyle(), QString( "s1" ) );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  _setVLColor( mVL, Qt::green );

  mVL->styleManager()->setCurrentStyle( QStringLiteral( "default" ) );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  mVL->styleManager()->setCurrentStyle( QStringLiteral( "s1" ) );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::green ) );

  _setVLColor( mVL, Qt::blue );

  mVL->styleManager()->setCurrentStyle( QStringLiteral( "default" ) );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  mVL->styleManager()->setCurrentStyle( QStringLiteral( "s1" ) );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::blue ) );
}

void TestQgsMapLayerStyleManager::testCopyStyles()
{
  std::unique_ptr<QgsVectorLayer> lines = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineString" ), QStringLiteral( "Line Layer" ), QStringLiteral( "memory" ) );
  std::unique_ptr<QgsVectorLayer> lines2 = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineString" ), QStringLiteral( "Line Layer" ), QStringLiteral( "memory" ) );

  QgsMapLayerStyleManager *sm = lines->styleManager();

  sm->addStyleFromLayer( QStringLiteral( "style2" ) );

  QgsMapLayerStyleManager *sm2 = lines2->styleManager();

  sm2->copyStylesFrom( sm );
  sm2->addStyleFromLayer( "style3" );

  QVERIFY( sm2->styles().contains( "style2" ) );
  QVERIFY( sm2->styles().contains( "style3" ) );
  QVERIFY( sm2->styles().contains( "default" ) );
}


QGSTEST_MAIN( TestQgsMapLayerStyleManager )
#include "testqgsmaplayerstylemanager.moc"
