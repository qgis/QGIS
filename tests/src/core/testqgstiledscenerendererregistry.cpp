/***************************************************************************
                         testqgstiledscenerendererregistry.cpp
                         -----------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgstiledscenerendererregistry.h"
#include "qgstiledscenerenderer.h"
#include "qgsreadwritecontext.h"

#include <QObject>
#include "qgstest.h"

//dummy renderer for testing
class DummyRenderer : public QgsTiledSceneRenderer
{
  public:
    DummyRenderer() = default;
    QString type() const override { return QStringLiteral( "dummy" ); }
    QgsTiledSceneRenderer *clone() const override { return new DummyRenderer(); }
    static QgsTiledSceneRenderer *create( QDomElement &, const QgsReadWriteContext & ) { return new DummyRenderer(); }
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext & ) const override { return doc.createElement( QStringLiteral( "test" ) ); }
    void renderTriangle( QgsTiledSceneRenderContext &, const QPolygonF & ) override {};
    void renderLine( QgsTiledSceneRenderContext &, const QPolygonF & ) override {};
};

class TestQgsTiledSceneRendererRegistry : public QObject
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

void TestQgsTiledSceneRendererRegistry::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();
}

void TestQgsTiledSceneRendererRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTiledSceneRendererRegistry::init()
{
}

void TestQgsTiledSceneRendererRegistry::cleanup()
{
}

void TestQgsTiledSceneRendererRegistry::metadata()
{
  QgsTiledSceneRendererMetadata metadata = QgsTiledSceneRendererMetadata( QStringLiteral( "name" ), QStringLiteral( "display name" ), DummyRenderer::create, QIcon() );
  QCOMPARE( metadata.name(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating renderer from metadata
  QDomElement elem;
  const std::unique_ptr<QgsTiledSceneRenderer> renderer( metadata.createRenderer( elem, QgsReadWriteContext() ) );
  QVERIFY( renderer );
  DummyRenderer *dummyRenderer = dynamic_cast<DummyRenderer *>( renderer.get() );
  QVERIFY( dummyRenderer );
}

void TestQgsTiledSceneRendererRegistry::createInstance()
{
  QgsTiledSceneRendererRegistry *registry = QgsApplication::tiledSceneRendererRegistry();
  QVERIFY( registry );
}

void TestQgsTiledSceneRendererRegistry::instanceHasDefaultRenderers()
{
  //check that callout registry is initially populated with some renderers
  //(assumes that there is some default renderers)
  QgsTiledSceneRendererRegistry *registry = QgsApplication::tiledSceneRendererRegistry();
  QVERIFY( registry->renderersList().length() > 0 );
}

void TestQgsTiledSceneRendererRegistry::addRenderer()
{
  QgsTiledSceneRendererRegistry *registry = QgsApplication::tiledSceneRendererRegistry();
  const int previousCount = registry->renderersList().length();

  registry->addRenderer( new QgsTiledSceneRendererMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy renderer" ), DummyRenderer::create, QIcon() ) );
  QCOMPARE( registry->renderersList().length(), previousCount + 1 );
  //try adding again, should have no effect
  QgsTiledSceneRendererMetadata *dupe = new QgsTiledSceneRendererMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy callout" ), DummyRenderer::create, QIcon() );
  QVERIFY( !registry->addRenderer( dupe ) );
  QCOMPARE( registry->renderersList().length(), previousCount + 1 );
  delete dupe;

  //try adding empty metadata
  registry->addRenderer( nullptr );
  QCOMPARE( registry->renderersList().length(), previousCount + 1 );
}

void TestQgsTiledSceneRendererRegistry::fetchTypes()
{
  QgsTiledSceneRendererRegistry *registry = QgsApplication::tiledSceneRendererRegistry();
  const QStringList types = registry->renderersList();

  QVERIFY( types.contains( "Dummy" ) );

  QgsTiledSceneRendererAbstractMetadata *metadata = registry->rendererMetadata( QStringLiteral( "Dummy" ) );
  QCOMPARE( metadata->name(), QString( "Dummy" ) );

  //metadata for bad renderer
  metadata = registry->rendererMetadata( QStringLiteral( "bad renderer" ) );
  QVERIFY( !metadata );
}

QGSTEST_MAIN( TestQgsTiledSceneRendererRegistry )
#include "testqgstiledscenerendererregistry.moc"
