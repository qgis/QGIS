/***************************************************************************
                         testqgspointcloudrendererregistry.cpp
                         -----------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudrenderer.h"
#include "qgspointcloudrendererregistry.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgstest.h"

#include <QObject>

//dummy renderer for testing
class DummyRenderer : public QgsPointCloudRenderer
{
  public:
    DummyRenderer() = default;
    QString type() const override { return u"dummy"_s; }
    QgsPointCloudRenderer *clone() const override { return new DummyRenderer(); }
    static QgsPointCloudRenderer *create( QDomElement &, const QgsReadWriteContext & ) { return new DummyRenderer(); }
    void renderBlock( const QgsPointCloudBlock *, QgsPointCloudRenderContext & ) override {}
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext & ) const override { return doc.createElement( u"test"_s ); }
};

class TestQgsPointCloudRendererRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void metadata();
    void createInstance();
    void instanceHasDefaultRenderers();
    void addRenderer();
    void fetchTypes();

  private:
};

void TestQgsPointCloudRendererRegistry::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();
}

void TestQgsPointCloudRendererRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPointCloudRendererRegistry::init()
{
}

void TestQgsPointCloudRendererRegistry::cleanup()
{
}

void TestQgsPointCloudRendererRegistry::metadata()
{
  QgsPointCloudRendererMetadata metadata = QgsPointCloudRendererMetadata( u"name"_s, u"display name"_s, DummyRenderer::create, QIcon() );
  QCOMPARE( metadata.name(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating renderer from metadata
  QDomElement elem;
  const std::unique_ptr<QgsPointCloudRenderer> renderer( metadata.createRenderer( elem, QgsReadWriteContext() ) );
  QVERIFY( renderer );
  DummyRenderer *dummyRenderer = dynamic_cast<DummyRenderer *>( renderer.get() );
  QVERIFY( dummyRenderer );
}

void TestQgsPointCloudRendererRegistry::createInstance()
{
  QgsPointCloudRendererRegistry *registry = QgsApplication::pointCloudRendererRegistry();
  QVERIFY( registry );
}

void TestQgsPointCloudRendererRegistry::instanceHasDefaultRenderers()
{
  //check that callout registry is initially populated with some renderers
  //(assumes that there is some default callouts)
  QgsPointCloudRendererRegistry *registry = QgsApplication::pointCloudRendererRegistry();
  QVERIFY( registry->renderersList().length() > 0 );
}

void TestQgsPointCloudRendererRegistry::addRenderer()
{
  QgsPointCloudRendererRegistry *registry = QgsApplication::pointCloudRendererRegistry();
  const int previousCount = registry->renderersList().length();

  registry->addRenderer( new QgsPointCloudRendererMetadata( u"Dummy"_s, u"Dummy renderer"_s, DummyRenderer::create, QIcon() ) );
  QCOMPARE( registry->renderersList().length(), previousCount + 1 );
  //try adding again, should have no effect
  QgsPointCloudRendererMetadata *dupe = new QgsPointCloudRendererMetadata( u"Dummy"_s, u"Dummy callout"_s, DummyRenderer::create, QIcon() );
  QVERIFY( !registry->addRenderer( dupe ) );
  QCOMPARE( registry->renderersList().length(), previousCount + 1 );
  delete dupe;

  //try adding empty metadata
  registry->addRenderer( nullptr );
  QCOMPARE( registry->renderersList().length(), previousCount + 1 );
}

void TestQgsPointCloudRendererRegistry::fetchTypes()
{
  QgsPointCloudRendererRegistry *registry = QgsApplication::pointCloudRendererRegistry();
  const QStringList types = registry->renderersList();

  QVERIFY( types.contains( "Dummy" ) );

  QgsPointCloudRendererAbstractMetadata *metadata = registry->rendererMetadata( u"Dummy"_s );
  QCOMPARE( metadata->name(), QString( "Dummy" ) );

  //metadata for bad renderer
  metadata = registry->rendererMetadata( u"bad renderer"_s );
  QVERIFY( !metadata );
}

QGSTEST_MAIN( TestQgsPointCloudRendererRegistry )
#include "testqgspointcloudrendererregistry.moc"
