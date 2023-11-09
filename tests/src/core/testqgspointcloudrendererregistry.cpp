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

#include "qgspointcloudrendererregistry.h"
#include "qgspointcloudrenderer.h"
#include "qgsrendercontext.h"
#include "qgsreadwritecontext.h"

#include <QObject>
#include "qgstest.h"

//dummy renderer for testing
class DummyRenderer : public QgsPointCloudRenderer
{
  public:
    DummyRenderer() = default;
    QString type() const override { return QStringLiteral( "dummy" ); }
    QgsPointCloudRenderer *clone() const override { return new DummyRenderer(); }
    static QgsPointCloudRenderer *create( QDomElement &, const QgsReadWriteContext & ) { return new DummyRenderer(); }
    void renderBlock( const QgsPointCloudBlock *, QgsPointCloudRenderContext & ) override {}
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext & ) const override { return doc.createElement( QStringLiteral( "test" ) ); }

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
  QgsPointCloudRendererMetadata metadata = QgsPointCloudRendererMetadata( QStringLiteral( "name" ), QStringLiteral( "display name" ), DummyRenderer::create, QIcon() );
  QCOMPARE( metadata.name(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating renderer from metadata
  QDomElement elem;
  const std::unique_ptr< QgsPointCloudRenderer > renderer( metadata.createRenderer( elem, QgsReadWriteContext() ) );
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

  registry->addRenderer( new QgsPointCloudRendererMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy renderer" ), DummyRenderer::create, QIcon() ) );
  QCOMPARE( registry->renderersList().length(), previousCount + 1 );
  //try adding again, should have no effect
  QgsPointCloudRendererMetadata *dupe = new QgsPointCloudRendererMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy callout" ), DummyRenderer::create, QIcon() );
  QVERIFY( ! registry->addRenderer( dupe ) );
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

  QgsPointCloudRendererAbstractMetadata *metadata = registry->rendererMetadata( QStringLiteral( "Dummy" ) );
  QCOMPARE( metadata->name(), QString( "Dummy" ) );

  //metadata for bad renderer
  metadata = registry->rendererMetadata( QStringLiteral( "bad renderer" ) );
  QVERIFY( !metadata );
}

QGSTEST_MAIN( TestQgsPointCloudRendererRegistry )
#include "testqgspointcloudrendererregistry.moc"
