/***************************************************************************
                         TestQgsMaterialRegistry.cpp
                         -----------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgs3d.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsmaterial.h"
#include "qgsmaterialregistry.h"
#include "qgstest.h"

#include <QObject>

//dummy material for testing
class DummyMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    DummyMaterialSettings() = default;
    QString type() const override { return u"Dummy"_s; }
    static QgsAbstractMaterialSettings *create() { return new DummyMaterialSettings(); }
    DummyMaterialSettings *clone() const override { return new DummyMaterialSettings(); }
    static bool supportsTechnique( QgsMaterialSettingsRenderingTechnique ) { return true; }
    void readXml( const QDomElement &, const QgsReadWriteContext & ) override {}
    void writeXml( QDomElement &, const QgsReadWriteContext & ) const override {}
    void addParametersToEffect( Qt3DRender::QEffect *, const QgsMaterialContext & ) const override {}
    QgsMaterial *toMaterial( QgsMaterialSettingsRenderingTechnique, const QgsMaterialContext & ) const override { return nullptr; }
    QMap<QString, QString> toExportParameters() const override { return QMap<QString, QString>(); }
    QByteArray dataDefinedVertexColorsAsByte( const QgsExpressionContext & ) const override { return QByteArray(); }
    bool equals( const QgsAbstractMaterialSettings * ) const override { return true; }
};

class TestQgsMaterialRegistry : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMaterialRegistry()
      : QgsTest( u"3D Material Registry Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void metadata();
    void createInstance();
    void instanceHasDefaultMaterials();
    void addMaterialSettings();
    void fetchTypes();
    void createMaterial();

  private:
};

void TestQgsMaterialRegistry::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();
}

void TestQgsMaterialRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMaterialRegistry::init()
{
}

void TestQgsMaterialRegistry::cleanup()
{
}

void TestQgsMaterialRegistry::metadata()
{
  QgsMaterialSettingsMetadata metadata = QgsMaterialSettingsMetadata( u"name"_s, u"display name"_s, DummyMaterialSettings::create, DummyMaterialSettings::supportsTechnique );
  QCOMPARE( metadata.type(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating material settings from metadata
  const std::unique_ptr<QgsAbstractMaterialSettings> material( metadata.create() );
  QVERIFY( material );
  DummyMaterialSettings *dummyMaterial = dynamic_cast<DummyMaterialSettings *>( material.get() );
  QVERIFY( dummyMaterial );
}

void TestQgsMaterialRegistry::createInstance()
{
  QgsMaterialRegistry *registry = Qgs3D::materialRegistry();
  QVERIFY( registry );
}

void TestQgsMaterialRegistry::instanceHasDefaultMaterials()
{
  //check that material registry is initially populated with some symbols
  //(assumes that there is some default materials)
  QgsMaterialRegistry *registry = Qgs3D::materialRegistry();

  // should be empty until initialized
  QVERIFY( registry->materialSettingsTypes().empty() );
  Qgs3D::initialize();
  QVERIFY( registry->materialSettingsTypes().length() > 0 );
}

void TestQgsMaterialRegistry::addMaterialSettings()
{
  QgsMaterialRegistry *registry = Qgs3D::materialRegistry();
  const int previousCount = registry->materialSettingsTypes().length();

  registry->addMaterialSettingsType( new QgsMaterialSettingsMetadata( u"Dummy"_s, u"Dummy material"_s, DummyMaterialSettings::create, DummyMaterialSettings::supportsTechnique ) );
  QCOMPARE( registry->materialSettingsTypes().length(), previousCount + 1 );
  //try adding again, should have no effect
  QgsMaterialSettingsMetadata *dupe = new QgsMaterialSettingsMetadata( u"Dummy"_s, u"Dummy material"_s, DummyMaterialSettings::create, DummyMaterialSettings::supportsTechnique );
  QVERIFY( !registry->addMaterialSettingsType( dupe ) );
  QCOMPARE( registry->materialSettingsTypes().length(), previousCount + 1 );
  delete dupe;

  //try adding empty metadata
  registry->addMaterialSettingsType( nullptr );
  QCOMPARE( registry->materialSettingsTypes().length(), previousCount + 1 );
}

void TestQgsMaterialRegistry::fetchTypes()
{
  QgsMaterialRegistry *registry = Qgs3D::materialRegistry();
  const QStringList types = registry->materialSettingsTypes();

  QVERIFY( types.contains( "Dummy" ) );

  QgsMaterialSettingsAbstractMetadata *metadata = registry->materialSettingsMetadata( u"Dummy"_s );
  QCOMPARE( metadata->type(), QString( "Dummy" ) );

  //metadata for bad material
  metadata = registry->materialSettingsMetadata( u"bad material"_s );
  QVERIFY( !metadata );
}

void TestQgsMaterialRegistry::createMaterial()
{
  QgsMaterialRegistry *registry = Qgs3D::materialRegistry();
  std::unique_ptr<QgsAbstractMaterialSettings> material( registry->createMaterialSettings( u"Dummy"_s ) );

  QVERIFY( material.get() );
  DummyMaterialSettings *dummySymbol = dynamic_cast<DummyMaterialSettings *>( material.get() );
  QVERIFY( dummySymbol );

  //try creating a bad material
  material.reset( registry->createMaterialSettings( u"bad material"_s ) );
  QVERIFY( !material.get() );
}

QGSTEST_MAIN( TestQgsMaterialRegistry )
#include "testqgsmaterialregistry.moc"
