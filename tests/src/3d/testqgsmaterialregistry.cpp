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

#include "qgsmaterialregistry.h"
#include "qgsabstractmaterialsettings.h"

#include <QObject>
#include "qgstest.h"
#include "qgs3d.h"

//dummy material for testing
class DummyMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    DummyMaterialSettings() = default;
    QString type() const override { return QStringLiteral( "Dummy" ); }
    static QgsAbstractMaterialSettings *create() { return new DummyMaterialSettings(); }
    DummyMaterialSettings *clone() const override { return new DummyMaterialSettings(); }
    static bool supportsTechnique( QgsMaterialSettingsRenderingTechnique ) { return true; }
    void readXml( const QDomElement &, const QgsReadWriteContext & ) override { }
    void writeXml( QDomElement &, const QgsReadWriteContext & ) const override {}
    void addParametersToEffect( Qt3DRender::QEffect * ) const override {}
    Qt3DRender::QMaterial *toMaterial( QgsMaterialSettingsRenderingTechnique, const QgsMaterialContext & ) const override { return nullptr; }
    QMap<QString, QString> toExportParameters() const override { return QMap<QString, QString>(); }
    QByteArray dataDefinedVertexColorsAsByte( const QgsExpressionContext & ) const override {return QByteArray();}
};

class TestQgsMaterialRegistry : public QObject
{
    Q_OBJECT

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
  QgsMaterialSettingsMetadata metadata = QgsMaterialSettingsMetadata( QStringLiteral( "name" ), QStringLiteral( "display name" ),
                                         DummyMaterialSettings::create, DummyMaterialSettings::supportsTechnique );
  QCOMPARE( metadata.type(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating material settings from metadata
  const std::unique_ptr< QgsAbstractMaterialSettings > material( metadata.create() );
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

  registry->addMaterialSettingsType( new QgsMaterialSettingsMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy material" ),
                                     DummyMaterialSettings::create, DummyMaterialSettings::supportsTechnique ) );
  QCOMPARE( registry->materialSettingsTypes().length(), previousCount + 1 );
  //try adding again, should have no effect
  QgsMaterialSettingsMetadata *dupe = new QgsMaterialSettingsMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy material" ),
      DummyMaterialSettings::create, DummyMaterialSettings::supportsTechnique );
  QVERIFY( ! registry->addMaterialSettingsType( dupe ) );
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

  QgsMaterialSettingsAbstractMetadata *metadata = registry->materialSettingsMetadata( QStringLiteral( "Dummy" ) );
  QCOMPARE( metadata->type(), QString( "Dummy" ) );

  //metadata for bad material
  metadata = registry->materialSettingsMetadata( QStringLiteral( "bad material" ) );
  QVERIFY( !metadata );
}

void TestQgsMaterialRegistry::createMaterial()
{
  QgsMaterialRegistry *registry = Qgs3D::materialRegistry();
  std::unique_ptr< QgsAbstractMaterialSettings > material( registry->createMaterialSettings( QStringLiteral( "Dummy" ) ) );

  QVERIFY( material.get() );
  DummyMaterialSettings *dummySymbol = dynamic_cast<DummyMaterialSettings *>( material.get() );
  QVERIFY( dummySymbol );

  //try creating a bad material
  material.reset( registry->createMaterialSettings( QStringLiteral( "bad material" ) ) );
  QVERIFY( !material.get() );
}

QGSTEST_MAIN( TestQgsMaterialRegistry )
#include "testqgsmaterialregistry.moc"
