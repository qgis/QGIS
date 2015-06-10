/***************************************************************************
                         testqgspainteffectregistry.cpp
                         -----------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#include "qgspainteffectregistry.h"
#include "qgspainteffect.h"
#include <QObject>
#include <QtTest/QtTest>

//dummy paint effect for testing
class DummyPaintEffect : public QgsPaintEffect
{
  public:
    DummyPaintEffect() {}
    virtual ~DummyPaintEffect() {}
    virtual QString type() const override { return "Dummy"; }
    virtual QgsPaintEffect* clone() const override { return new DummyPaintEffect(); }
    static QgsPaintEffect* create( const QgsStringMap& ) { return new DummyPaintEffect(); }
    virtual QgsStringMap properties() const override { return QgsStringMap(); }
    virtual void readProperties( const QgsStringMap& props ) override { Q_UNUSED( props ); }
  protected:
    virtual void draw( QgsRenderContext& context ) override { Q_UNUSED( context ); }
};

class TestQgsPaintEffectRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void metadata(); //test metadata
    void createInstance(); // create global instance of QgsPaintEffectRegistry
    void instanceHasDefaultEffects(); // check that global instance is populated with default effects
    void addEffect(); // check adding an effect to an empty registry
    void fetchEffects(); //check fetching effects
    void createEffect(); //check creating effect

  private:

};

void TestQgsPaintEffectRegistry::initTestCase()
{

}

void TestQgsPaintEffectRegistry::cleanupTestCase()
{

}

void TestQgsPaintEffectRegistry::init()
{

}

void TestQgsPaintEffectRegistry::cleanup()
{

}

void TestQgsPaintEffectRegistry::metadata()
{
  QgsPaintEffectMetadata metadata = QgsPaintEffectMetadata( "name", "display name", DummyPaintEffect::create );
  QCOMPARE( metadata.name(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating effect from metadata
  QgsStringMap map;
  QgsPaintEffect* effect = metadata.createPaintEffect( map );
  QVERIFY( effect );
  DummyPaintEffect* dummyEffect = dynamic_cast<DummyPaintEffect*>( effect );
  QVERIFY( dummyEffect );
  delete effect;
}

void TestQgsPaintEffectRegistry::createInstance()
{
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  QVERIFY( registry );
}

void TestQgsPaintEffectRegistry::instanceHasDefaultEffects()
{
  //check that effect instance is initially populated with some effects
  //(assumes that there is some default effects)
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  QVERIFY( registry->effects().length() > 0 );
}

void TestQgsPaintEffectRegistry::addEffect()
{
  //create an empty registry
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  int previousCount = registry->effects().length();

  registry->addEffectType( new QgsPaintEffectMetadata( "Dummy", "Dummy effect", DummyPaintEffect::create ) );
  QCOMPARE( registry->effects().length(), previousCount + 1 );
  //try adding again, should have no effect
  registry->addEffectType( new QgsPaintEffectMetadata( "Dummy", "Dummy effect", DummyPaintEffect::create ) );
  QCOMPARE( registry->effects().length(), previousCount + 1 );

  //try adding empty metadata
  registry->addEffectType( NULL );
  QCOMPARE( registry->effects().length(), previousCount + 1 );
}

void TestQgsPaintEffectRegistry::fetchEffects()
{
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  QStringList effects = registry->effects();

  QVERIFY( effects.contains( "Dummy" ) );

  QgsPaintEffectAbstractMetadata* metadata = registry->effectMetadata( QString( "Dummy" ) );
  QCOMPARE( metadata->name(), QString( "Dummy" ) );

  //metadata for bad effect
  metadata = registry->effectMetadata( QString( "bad effect" ) );
  QVERIFY( !metadata );
}

void TestQgsPaintEffectRegistry::createEffect()
{
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  QgsPaintEffect* effect = registry->createEffect( QString( "Dummy" ) );

  QVERIFY( effect );
  DummyPaintEffect* dummyEffect = dynamic_cast<DummyPaintEffect*>( effect );
  QVERIFY( dummyEffect );
  delete effect;
  effect = 0;

  //try creating a bad effect
  effect = registry->createEffect( QString( "bad effect" ) );
  QVERIFY( !effect );
}

QTEST_MAIN( TestQgsPaintEffectRegistry )
#include "testqgspainteffectregistry.moc"
