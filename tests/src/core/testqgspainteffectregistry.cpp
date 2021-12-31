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
#include "qgseffectstack.h"
#include "qgsrendercontext.h"

#include <QObject>
#include "qgstest.h"

//dummy paint effect for testing
class DummyPaintEffect : public QgsPaintEffect
{
  public:
    DummyPaintEffect() = default;
    QString type() const override { return QStringLiteral( "Dummy" ); }
    QgsPaintEffect *clone() const override { return new DummyPaintEffect(); }
    static QgsPaintEffect *create( const QVariantMap & ) { return new DummyPaintEffect(); }
    QVariantMap properties() const override { return QVariantMap(); }
    void readProperties( const QVariantMap &props ) override { Q_UNUSED( props ); }
  protected:
    void draw( QgsRenderContext &context ) override { Q_UNUSED( context ); }
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
    void defaultStack(); //check creating/testing default stack

  private:

};

void TestQgsPaintEffectRegistry::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();
}

void TestQgsPaintEffectRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPaintEffectRegistry::init()
{

}

void TestQgsPaintEffectRegistry::cleanup()
{

}

void TestQgsPaintEffectRegistry::metadata()
{
  QgsPaintEffectMetadata metadata = QgsPaintEffectMetadata( QStringLiteral( "name" ), QStringLiteral( "display name" ), DummyPaintEffect::create );
  QCOMPARE( metadata.name(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating effect from metadata
  const QVariantMap map;
  QgsPaintEffect *effect = metadata.createPaintEffect( map );
  QVERIFY( effect );
  DummyPaintEffect *dummyEffect = dynamic_cast<DummyPaintEffect *>( effect );
  QVERIFY( dummyEffect );
  delete effect;
}

void TestQgsPaintEffectRegistry::createInstance()
{
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  QVERIFY( registry );
}

void TestQgsPaintEffectRegistry::instanceHasDefaultEffects()
{
  //check that effect instance is initially populated with some effects
  //(assumes that there is some default effects)
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  QVERIFY( registry->effects().length() > 0 );
}

void TestQgsPaintEffectRegistry::addEffect()
{
  //create an empty registry
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  const int previousCount = registry->effects().length();

  registry->addEffectType( new QgsPaintEffectMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy effect" ), DummyPaintEffect::create ) );
  QCOMPARE( registry->effects().length(), previousCount + 1 );
  //try adding again, should have no effect
  QgsPaintEffectMetadata *dupe = new QgsPaintEffectMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy effect" ), DummyPaintEffect::create );
  QVERIFY( ! registry->addEffectType( dupe ) );
  QCOMPARE( registry->effects().length(), previousCount + 1 );
  delete dupe;

  //try adding empty metadata
  registry->addEffectType( nullptr );
  QCOMPARE( registry->effects().length(), previousCount + 1 );
}

void TestQgsPaintEffectRegistry::fetchEffects()
{
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  const QStringList effects = registry->effects();

  QVERIFY( effects.contains( "Dummy" ) );

  QgsPaintEffectAbstractMetadata *metadata = registry->effectMetadata( QStringLiteral( "Dummy" ) );
  QCOMPARE( metadata->name(), QString( "Dummy" ) );

  //metadata for bad effect
  metadata = registry->effectMetadata( QStringLiteral( "bad effect" ) );
  QVERIFY( !metadata );
}

void TestQgsPaintEffectRegistry::createEffect()
{
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  QgsPaintEffect *effect = registry->createEffect( QStringLiteral( "Dummy" ) );

  QVERIFY( effect );
  DummyPaintEffect *dummyEffect = dynamic_cast<DummyPaintEffect *>( effect );
  QVERIFY( dummyEffect );
  delete effect;

  //try creating a bad effect
  effect = registry->createEffect( QStringLiteral( "bad effect" ) );
  QVERIFY( !effect );
}

void TestQgsPaintEffectRegistry::defaultStack()
{
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  QgsEffectStack *effect = static_cast<QgsEffectStack *>( QgsPaintEffectRegistry::defaultStack() );
  QVERIFY( registry->isDefaultStack( effect ) );
  effect->effect( 1 )->setEnabled( true );
  QVERIFY( !registry->isDefaultStack( effect ) );
  effect->effect( 1 )->setEnabled( false );
  effect->effect( 2 )->setEnabled( false ); //third effect should be enabled by default
  QVERIFY( !registry->isDefaultStack( effect ) );
  effect->effect( 2 )->setEnabled( true );
  effect->appendEffect( new QgsEffectStack() );
  QVERIFY( !registry->isDefaultStack( effect ) );
  delete effect;
  QgsPaintEffect *effect2 = new DummyPaintEffect();
  QVERIFY( !registry->isDefaultStack( effect2 ) );
  delete effect2;

  effect = static_cast<QgsEffectStack *>( QgsPaintEffectRegistry::defaultStack() );
  static_cast< QgsDrawSourceEffect * >( effect->effect( 2 ) )->setOpacity( 0.5 );
  QVERIFY( !registry->isDefaultStack( effect ) );
  static_cast< QgsDrawSourceEffect * >( effect->effect( 2 ) )->setOpacity( 1.0 );
  QVERIFY( registry->isDefaultStack( effect ) );
  static_cast< QgsDrawSourceEffect * >( effect->effect( 2 ) )->setBlendMode( QPainter::CompositionMode_Lighten );
  QVERIFY( !registry->isDefaultStack( effect ) );
}

QGSTEST_MAIN( TestQgsPaintEffectRegistry )
#include "testqgspainteffectregistry.moc"
