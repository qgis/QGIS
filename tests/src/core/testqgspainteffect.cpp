/***************************************************************************
     testqgspainteffect.cpp
     --------------------------------------
    Date                 : December 2014
    Copyright            : (C) 2014 by Nyall Dawson
    Email                : nyall dot dawson at gmail.com
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
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QPicture>

#include "qgspainteffect.h"
#include "qgsblureffect.h"
#include "qgsshadoweffect.h"
#include "qgseffectstack.h"
#include "qgsgloweffect.h"
#include "qgstransformeffect.h"
#include "qgspainteffectregistry.h"
#include "qgscolorrampimpl.h"
#include "qgssymbollayerutils.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgslayout.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutpagecollection.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

//qgis test includes
#include "qgsmultirenderchecker.h"

//dummy paint effect for testing
class DummyPaintEffect : public QgsPaintEffect
{
  public:
    DummyPaintEffect( const QString &prop1, const QString &prop2 )
      : mProp1( prop1 )
      , mProp2( prop2 )
    {}
    QString type() const override { return QStringLiteral( "Dummy" ); }
    QgsPaintEffect *clone() const override { return new DummyPaintEffect( mProp1, mProp2 ); }
    static QgsPaintEffect *create( const QVariantMap &props ) { return new DummyPaintEffect( props[QStringLiteral( "testProp" )].toString(), props[QStringLiteral( "testProp2" )].toString() ); }
    QVariantMap properties() const override
    {
      QVariantMap props;
      props[QStringLiteral( "testProp" )] = mProp1;
      props[QStringLiteral( "testProp2" )] = mProp2;
      return props;
    }
    void readProperties( const QVariantMap &props ) override
    {
      mProp1 = props[QStringLiteral( "testProp" )].toString();
      mProp2 = props[QStringLiteral( "testProp2" )].toString();
    }

    QString prop1() { return mProp1; }
    QString prop2() { return mProp2; }

  protected:

    void draw( QgsRenderContext &context ) override { Q_UNUSED( context ); }

  private:
    QString mProp1;
    QString mProp2;
};



/**
 * \ingroup UnitTests
 * This is a unit test for paint effects
 */
class TestQgsPaintEffect: public QgsTest
{
    Q_OBJECT

  public:
    TestQgsPaintEffect() : QgsTest( QStringLiteral( "Paint Effect Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void saveRestore();
    void stackSaveRestore();

    //specific effects
    void drawSource();
    void blur();
    void dropShadow();
    void glow();
    void transform();

    void stack();

    //rendering
    void layerEffectPolygon();
    void layerEffectLine();
    void layerEffectMarker();
    void vectorLayerEffect();
    void mapUnits();
    void layout();

  private:
    bool imageCheck( const QString &testName, QImage &image, int mismatchCount = 0 );
    bool mapRenderCheck( const QString &testName, QgsMapSettings &mapSettings, int mismatchCount = 0 );

    QString mTestDataDir;

    QPicture *mPicture = nullptr;
};

void TestQgsPaintEffect::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mPicture = nullptr;

  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  registry->addEffectType( new QgsPaintEffectMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy effect" ), DummyPaintEffect::create ) );

  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
}

void TestQgsPaintEffect::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPaintEffect::init()
{
  mPicture = new QPicture();
  QPainter painter;
  painter.begin( mPicture );
  painter.setPen( Qt::red );
  painter.setBrush( QColor( 255, 50, 50, 200 ) );
  painter.drawEllipse( QPoint( 50, 50 ), 30, 30 );
  painter.end();
}

void TestQgsPaintEffect::cleanup()
{
  delete mPicture;
}

void TestQgsPaintEffect::saveRestore()
{
  DummyPaintEffect *effect = new DummyPaintEffect( QStringLiteral( "a" ), QStringLiteral( "b" ) );

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement noNode;
  QCOMPARE( effect->saveProperties( doc, noNode ), false );

  //test writing with node
  QDomElement effectParentElem = doc.createElement( QStringLiteral( "parent" ) );
  rootNode.appendChild( effectParentElem );
  QVERIFY( effect->saveProperties( doc, effectParentElem ) );

  //check if effect node was written
  const QDomNodeList evalNodeList = effectParentElem.elementsByTagName( QStringLiteral( "effect" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  const QDomElement effectElem = evalNodeList.at( 0 ).toElement();
  QCOMPARE( effectElem.attribute( "type" ), QString( "Dummy" ) );

  //test reading empty node
  QgsPaintEffect *restoredEffect = QgsApplication::paintEffectRegistry()->createEffect( noNode );
  QVERIFY( !restoredEffect );

  //test reading bad node
  QDomElement badEffectElem = doc.createElement( QStringLiteral( "parent" ) );
  badEffectElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "bad" ) );
  restoredEffect = QgsApplication::paintEffectRegistry()->createEffect( badEffectElem );
  QVERIFY( !restoredEffect );

  //test reading node
  restoredEffect = QgsApplication::paintEffectRegistry()->createEffect( effectElem );
  QVERIFY( restoredEffect );
  DummyPaintEffect *restoredDummyEffect = dynamic_cast<DummyPaintEffect *>( restoredEffect );
  QVERIFY( restoredDummyEffect );

  //test properties
  QCOMPARE( restoredDummyEffect->prop1(), effect->prop1() );
  QCOMPARE( restoredDummyEffect->prop2(), effect->prop2() );

  delete effect;
  delete restoredEffect;
}

void TestQgsPaintEffect::stackSaveRestore()
{
  QgsEffectStack *stack = new QgsEffectStack();
  //add two effects to stack
  QgsBlurEffect *blur = new QgsBlurEffect();
  QgsDropShadowEffect *shadow = new QgsDropShadowEffect();
  stack->appendEffect( blur );
  stack->appendEffect( shadow );
  stack->setEnabled( false );

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement noNode;
  QCOMPARE( stack->saveProperties( doc, noNode ), false );

  //test writing with node
  QDomElement effectParentElem = doc.createElement( QStringLiteral( "parent" ) );
  rootNode.appendChild( effectParentElem );
  QVERIFY( stack->saveProperties( doc, effectParentElem ) );

  //check if stack effect node was written
  const QDomNodeList evalNodeList = effectParentElem.childNodes();
  QCOMPARE( evalNodeList.count(), 1 );
  const QDomElement effectElem = evalNodeList.at( 0 ).toElement();
  QCOMPARE( effectElem.attribute( "type" ), stack->type() );

  //should be two effect child nodes
  const QDomNodeList childNodeList = effectElem.elementsByTagName( QStringLiteral( "effect" ) );
  QCOMPARE( childNodeList.count(), 2 );
  QCOMPARE( childNodeList.at( 0 ).toElement().attribute( "type" ), blur->type() );
  QCOMPARE( childNodeList.at( 1 ).toElement().attribute( "type" ), shadow->type() );

  //test reading node
  QgsPaintEffect *restoredEffect = QgsApplication::paintEffectRegistry()->createEffect( effectElem );
  QVERIFY( restoredEffect );
  QgsEffectStack *restoredStack = dynamic_cast<QgsEffectStack *>( restoredEffect );
  QVERIFY( restoredStack );
  QCOMPARE( restoredStack->enabled(), stack->enabled() );

  //test that child effects were restored
  QCOMPARE( restoredStack->effectList()->length(), 2 );
  QCOMPARE( restoredStack->effectList()->at( 0 )->type(), blur->type() );
  QCOMPARE( restoredStack->effectList()->at( 1 )->type(), shadow->type() );

  delete stack;
  delete restoredStack;
}

void TestQgsPaintEffect::drawSource()
{
  //create
  QgsDrawSourceEffect *effect = new QgsDrawSourceEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setOpacity( 0.5 );
  QCOMPARE( effect->opacity(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsDrawSourceEffect *copy = new QgsDrawSourceEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->opacity(), effect->opacity() );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->drawMode(), effect->drawMode() );
  delete copy;

  //clone
  QgsPaintEffect *clone = effect->clone();
  QgsDrawSourceEffect *cloneCast = dynamic_cast<QgsDrawSourceEffect * >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->opacity(), effect->opacity() );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->drawMode(), effect->drawMode() );
  delete cloneCast;

  //read/write
  const QVariantMap props = effect->properties();
  QgsPaintEffect *readEffect = QgsDrawSourceEffect::create( props );
  QgsDrawSourceEffect *readCast = dynamic_cast<QgsDrawSourceEffect * >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->opacity(), effect->opacity() );
  QCOMPARE( readCast->enabled(), effect->enabled() );
  QCOMPARE( readCast->drawMode(), effect->drawMode() );
  delete readCast;

  delete effect;

  //test render
  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.setDotsPerMeterX( 96 / 25.4 * 1000 );
  image.setDotsPerMeterY( 96 / 25.4 * 1000 );
  image.fill( Qt::transparent );

  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  effect = new QgsDrawSourceEffect();
  effect->render( *mPicture, context );
  painter.end();

  const bool result = imageCheck( QStringLiteral( "painteffect_drawsource" ), image, 0 );

  delete effect;
  QVERIFY( result );
}

void TestQgsPaintEffect::blur()
{
  //create
  QgsBlurEffect *effect = new QgsBlurEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setOpacity( 0.5 );
  QCOMPARE( effect->opacity(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setBlurLevel( 6.0 );
  QCOMPARE( effect->blurLevel(), 6.0 );
  effect->setBlurMethod( QgsBlurEffect::GaussianBlur );
  QCOMPARE( effect->blurMethod(), QgsBlurEffect::GaussianBlur );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsBlurEffect *copy = new QgsBlurEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->opacity(), effect->opacity() );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->blurLevel(), effect->blurLevel() );
  QCOMPARE( copy->blurMethod(), effect->blurMethod() );
  QCOMPARE( copy->drawMode(), effect->drawMode() );
  delete copy;

  //clone
  QgsPaintEffect *clone = effect->clone();
  QgsBlurEffect *cloneCast = dynamic_cast<QgsBlurEffect * >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->opacity(), effect->opacity() );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( cloneCast->blurMethod(), effect->blurMethod() );
  QCOMPARE( cloneCast->drawMode(), effect->drawMode() );
  delete cloneCast;

  //read/write
  const QVariantMap props = effect->properties();
  QgsPaintEffect *readEffect = QgsBlurEffect::create( props );
  QgsBlurEffect *readCast = dynamic_cast<QgsBlurEffect * >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->opacity(), effect->opacity() );
  QCOMPARE( readCast->enabled(), effect->enabled() );
  QCOMPARE( readCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( readCast->blurMethod(), effect->blurMethod() );
  QCOMPARE( readCast->drawMode(), effect->drawMode() );
  delete readCast;

  delete effect;

  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.setDotsPerMeterX( 96 / 25.4 * 1000 );
  image.setDotsPerMeterY( 96 / 25.4 * 1000 );
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  effect = new QgsBlurEffect();
  effect->render( *mPicture, context );
  painter.end();

  const bool result = imageCheck( QStringLiteral( "painteffect_blur" ), image, 0 );

  delete effect;
  QVERIFY( result );
}

void TestQgsPaintEffect::dropShadow()
{
  //create
  QgsDropShadowEffect *effect = new QgsDropShadowEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setOpacity( 0.5 );
  QCOMPARE( effect->opacity(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setBlurLevel( 6.0 );
  QCOMPARE( effect->blurLevel(), 6.0 );
  effect->setOffsetAngle( 77 );
  QCOMPARE( effect->offsetAngle(), 77 );
  effect->setOffsetDistance( 9.7 );
  QCOMPARE( effect->offsetDistance(), 9.7 );
  effect->setOffsetUnit( QgsUnitTypes::RenderMapUnits );
  QCOMPARE( effect->offsetUnit(), QgsUnitTypes::RenderMapUnits );
  effect->setOffsetMapUnitScale( QgsMapUnitScale( 1.0, 2.0 ) );
  QCOMPARE( effect->offsetMapUnitScale().minScale, 1.0 );
  QCOMPARE( effect->offsetMapUnitScale().maxScale, 2.0 );
  effect->setColor( QColor( 56, 67, 89 ) );
  QCOMPARE( effect->color(), QColor( 56, 67, 89 ) );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsDropShadowEffect *copy = new QgsDropShadowEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->opacity(), effect->opacity() );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->blurLevel(), effect->blurLevel() );
  QCOMPARE( copy->offsetAngle(), effect->offsetAngle() );
  QCOMPARE( copy->offsetDistance(), effect->offsetDistance() );
  QCOMPARE( copy->offsetUnit(), effect->offsetUnit() );
  QCOMPARE( copy->offsetMapUnitScale().minScale, effect->offsetMapUnitScale().minScale );
  QCOMPARE( copy->offsetMapUnitScale().maxScale, effect->offsetMapUnitScale().maxScale );
  QCOMPARE( copy->color(), effect->color() );
  QCOMPARE( copy->drawMode(), effect->drawMode() );
  delete copy;

  //clone
  QgsPaintEffect *clone = effect->clone();
  QgsDropShadowEffect *cloneCast = dynamic_cast<QgsDropShadowEffect * >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->opacity(), effect->opacity() );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( cloneCast->offsetAngle(), effect->offsetAngle() );
  QCOMPARE( cloneCast->offsetDistance(), effect->offsetDistance() );
  QCOMPARE( cloneCast->offsetUnit(), effect->offsetUnit() );
  QCOMPARE( cloneCast->offsetMapUnitScale().minScale, effect->offsetMapUnitScale().minScale );
  QCOMPARE( cloneCast->offsetMapUnitScale().maxScale, effect->offsetMapUnitScale().maxScale );
  QCOMPARE( cloneCast->color(), effect->color() );
  QCOMPARE( cloneCast->drawMode(), effect->drawMode() );
  delete cloneCast;

  //read/write
  const QVariantMap props = effect->properties();
  QgsPaintEffect *readEffect = QgsDropShadowEffect::create( props );
  QgsDropShadowEffect *readCast = dynamic_cast<QgsDropShadowEffect * >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->opacity(), effect->opacity() );
  QCOMPARE( readCast->enabled(), effect->enabled() );
  QCOMPARE( readCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( readCast->offsetAngle(), effect->offsetAngle() );
  QCOMPARE( readCast->offsetDistance(), effect->offsetDistance() );
  QCOMPARE( readCast->offsetUnit(), effect->offsetUnit() );
  QCOMPARE( readCast->offsetMapUnitScale().minScale, effect->offsetMapUnitScale().minScale );
  QCOMPARE( readCast->offsetMapUnitScale().maxScale, effect->offsetMapUnitScale().maxScale );
  QCOMPARE( readCast->color(), effect->color() );
  QCOMPARE( readCast->drawMode(), effect->drawMode() );
  delete readCast;

  delete effect;

  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.setDotsPerMeterX( 96 / 25.4 * 1000 );
  image.setDotsPerMeterY( 96 / 25.4 * 1000 );
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  effect = new QgsDropShadowEffect();
  effect->render( *mPicture, context );
  painter.end();

  const bool result = imageCheck( QStringLiteral( "painteffect_dropshadow" ), image, 0 );
  delete effect;
  QVERIFY( result );
}

void TestQgsPaintEffect::glow()
{
  //create
  QgsOuterGlowEffect *effect = new QgsOuterGlowEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setOpacity( 0.5 );
  QCOMPARE( effect->opacity(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setBlurLevel( 6.0 );
  QCOMPARE( effect->blurLevel(), 6.0 );
  effect->setSpread( 7.8 );
  QCOMPARE( effect->spread(), 7.8 );
  effect->setSpreadUnit( QgsUnitTypes::RenderMapUnits );
  QCOMPARE( effect->spreadUnit(), QgsUnitTypes::RenderMapUnits );
  effect->setSpreadMapUnitScale( QgsMapUnitScale( 1.0, 2.0 ) );
  QCOMPARE( effect->spreadMapUnitScale().minScale, 1.0 );
  QCOMPARE( effect->spreadMapUnitScale().maxScale, 2.0 );
  effect->setRamp( new QgsGradientColorRamp( QColor( 255, 0, 0 ), QColor( 0, 255, 0 ) ) );
  QCOMPARE( effect->ramp()->color( 0 ), QColor( 255, 0, 0 ) );
  QCOMPARE( effect->ramp()->color( 1.0 ), QColor( 0, 255, 0 ) );
  effect->setColorType( QgsGlowEffect::ColorRamp );
  QCOMPARE( effect->colorType(), QgsGlowEffect::ColorRamp );
  effect->setColor( QColor( 56, 67, 89 ) );
  QCOMPARE( effect->color(), QColor( 56, 67, 89 ) );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsOuterGlowEffect *copy = new QgsOuterGlowEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->opacity(), effect->opacity() );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->blurLevel(), effect->blurLevel() );
  QCOMPARE( copy->spread(), effect->spread() );
  QCOMPARE( copy->spreadUnit(), effect->spreadUnit() );
  QCOMPARE( copy->spreadMapUnitScale().minScale, effect->spreadMapUnitScale().minScale );
  QCOMPARE( copy->spreadMapUnitScale().maxScale, effect->spreadMapUnitScale().maxScale );
  QCOMPARE( copy->colorType(), effect->colorType() );
  QCOMPARE( copy->ramp()->color( 0 ), effect->ramp()->color( 0 ) );
  QCOMPARE( copy->color(), effect->color() );
  QCOMPARE( copy->drawMode(), effect->drawMode() );
  delete copy;

  //clone
  QgsPaintEffect *clone = effect->clone();
  QgsOuterGlowEffect *cloneCast = dynamic_cast<QgsOuterGlowEffect * >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->opacity(), effect->opacity() );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( cloneCast->spread(), effect->spread() );
  QCOMPARE( cloneCast->spreadUnit(), effect->spreadUnit() );
  QCOMPARE( cloneCast->spreadMapUnitScale().minScale, effect->spreadMapUnitScale().minScale );
  QCOMPARE( cloneCast->spreadMapUnitScale().maxScale, effect->spreadMapUnitScale().maxScale );
  QCOMPARE( cloneCast->colorType(), effect->colorType() );
  QCOMPARE( cloneCast->ramp()->color( 0 ), effect->ramp()->color( 0 ) );
  QCOMPARE( cloneCast->color(), effect->color() );
  QCOMPARE( cloneCast->drawMode(), effect->drawMode() );
  delete cloneCast;

  //read/write
  const QVariantMap props = effect->properties();
  QgsPaintEffect *readEffect = QgsOuterGlowEffect::create( props );
  QgsOuterGlowEffect *readCast = dynamic_cast<QgsOuterGlowEffect * >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->opacity(), effect->opacity() );
  QCOMPARE( readCast->enabled(), effect->enabled() );
  QCOMPARE( readCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( readCast->spread(), effect->spread() );
  QCOMPARE( readCast->spreadUnit(), effect->spreadUnit() );
  QCOMPARE( readCast->spreadMapUnitScale().minScale, effect->spreadMapUnitScale().minScale );
  QCOMPARE( readCast->spreadMapUnitScale().maxScale, effect->spreadMapUnitScale().maxScale );
  QCOMPARE( readCast->colorType(), effect->colorType() );
  QCOMPARE( readCast->ramp()->color( 0 ), effect->ramp()->color( 0 ) );
  QCOMPARE( readCast->color(), effect->color() );
  QCOMPARE( readCast->drawMode(), effect->drawMode() );
  delete readCast;

  delete effect;

  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.setDotsPerMeterX( 96 / 25.4 * 1000 );
  image.setDotsPerMeterY( 96 / 25.4 * 1000 );
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  effect = new QgsOuterGlowEffect();
  effect->setSpread( 20 );
  effect->render( *mPicture, context );
  painter.end();

  delete effect;

  const bool result = imageCheck( QStringLiteral( "painteffect_outerglow" ), image, 0 );
  QVERIFY( result );

  //TODO - inner glow

}

void TestQgsPaintEffect::transform()
{
  //create
  std::unique_ptr< QgsTransformEffect > effect( new QgsTransformEffect() );
  QVERIFY( effect.get() );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setTranslateX( 6 );
  QCOMPARE( effect->translateX(), 6.0 );
  effect->setTranslateY( 77 );
  QCOMPARE( effect->translateY(), 77.0 );
  effect->setTranslateUnit( QgsUnitTypes::RenderMapUnits );
  QCOMPARE( effect->translateUnit(), QgsUnitTypes::RenderMapUnits );
  effect->setTranslateMapUnitScale( QgsMapUnitScale( 1.0, 2.0 ) );
  QCOMPARE( effect->translateMapUnitScale().minScale, 1.0 );
  QCOMPARE( effect->translateMapUnitScale().maxScale, 2.0 );
  effect->setScaleX( 0.5 );
  QCOMPARE( effect->scaleX(), 0.5 );
  effect->setScaleY( 1.5 );
  QCOMPARE( effect->scaleY(), 1.5 );
  effect->setRotation( 45.5 );
  QCOMPARE( effect->rotation(), 45.5 );
  effect->setShearX( 1.2 );
  QCOMPARE( effect->shearX(), 1.2 );
  effect->setShearY( 0.6 );
  QCOMPARE( effect->shearY(), 0.6 );
  effect->setReflectX( true );
  QCOMPARE( effect->reflectX(), true );
  effect->setReflectY( true );
  QCOMPARE( effect->reflectY(), true );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  std::unique_ptr< QgsTransformEffect > copy( new QgsTransformEffect( *effect ) );
  QVERIFY( copy.get() );
  QCOMPARE( copy->enabled(), false );
  QCOMPARE( copy->translateX(), 6.0 );
  QCOMPARE( copy->translateY(), 77.0 );
  QCOMPARE( copy->translateUnit(), QgsUnitTypes::RenderMapUnits );
  QCOMPARE( copy->translateMapUnitScale().minScale, 1.0 );
  QCOMPARE( copy->translateMapUnitScale().maxScale, 2.0 );
  QCOMPARE( copy->scaleX(), 0.5 );
  QCOMPARE( copy->scaleY(), 1.5 );
  QCOMPARE( copy->rotation(), 45.5 );
  QCOMPARE( copy->shearX(), 1.2 );
  QCOMPARE( copy->shearY(), 0.6 );
  QCOMPARE( copy->reflectX(), true );
  QCOMPARE( copy->reflectY(), true );
  QCOMPARE( copy->drawMode(), QgsPaintEffect::Modifier );
  copy.reset( nullptr );

  //clone
  std::unique_ptr< QgsPaintEffect > clone( effect->clone() );
  QgsTransformEffect *cloneCast = dynamic_cast<QgsTransformEffect * >( clone.get() );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->enabled(), false );
  QCOMPARE( cloneCast->translateX(), 6.0 );
  QCOMPARE( cloneCast->translateY(), 77.0 );
  QCOMPARE( cloneCast->translateUnit(), QgsUnitTypes::RenderMapUnits );
  QCOMPARE( cloneCast->translateMapUnitScale().minScale, 1.0 );
  QCOMPARE( cloneCast->translateMapUnitScale().maxScale, 2.0 );
  QCOMPARE( cloneCast->scaleX(), 0.5 );
  QCOMPARE( cloneCast->scaleY(), 1.5 );
  QCOMPARE( cloneCast->rotation(), 45.5 );
  QCOMPARE( cloneCast->shearX(), 1.2 );
  QCOMPARE( cloneCast->shearY(), 0.6 );
  QCOMPARE( cloneCast->reflectX(), true );
  QCOMPARE( cloneCast->reflectY(), true );
  QCOMPARE( cloneCast->drawMode(), QgsPaintEffect::Modifier );
  clone.reset( nullptr );

  //read/write
  const QVariantMap props = effect->properties();
  const std::unique_ptr< QgsPaintEffect > readEffect( QgsTransformEffect::create( props ) );
  QgsTransformEffect *readCast = dynamic_cast<QgsTransformEffect * >( readEffect.get() );
  QVERIFY( readCast );
  QCOMPARE( readCast->enabled(), false );
  QCOMPARE( readCast->translateX(), 6.0 );
  QCOMPARE( readCast->translateY(), 77.0 );
  QCOMPARE( readCast->translateUnit(), QgsUnitTypes::RenderMapUnits );
  QCOMPARE( readCast->translateMapUnitScale().minScale, 1.0 );
  QCOMPARE( readCast->translateMapUnitScale().maxScale, 2.0 );
  QCOMPARE( readCast->scaleX(), 0.5 );
  QCOMPARE( readCast->scaleY(), 1.5 );
  QCOMPARE( readCast->rotation(), 45.5 );
  QCOMPARE( readCast->shearX(), 1.2 );
  QCOMPARE( readCast->shearY(), 0.6 );
  QCOMPARE( readCast->reflectX(), true );
  QCOMPARE( readCast->reflectY(), true );
  QCOMPARE( readCast->drawMode(), QgsPaintEffect::Modifier );
}

void TestQgsPaintEffect::stack()
{
  //create
  QgsEffectStack *effect = new QgsEffectStack();
  QVERIFY( effect );
  effect->appendEffect( new QgsDrawSourceEffect() );
  effect->appendEffect( new QgsDrawSourceEffect() );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  QCOMPARE( effect->count(), 2 );

  //copy constructor
  QgsEffectStack *copy = new QgsEffectStack( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->count(), effect->count() );
  delete copy;

  //clone
  QgsPaintEffect *clone = effect->clone();
  QgsEffectStack *cloneCast = dynamic_cast<QgsEffectStack * >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->count(), effect->count() );
  delete cloneCast;

  delete effect;

  //stack from single effect
  const QgsBlurEffect singleEffect;
  effect = new QgsEffectStack( singleEffect );
  QVERIFY( effect );
  QCOMPARE( effect->count(), 1 );
  QgsPaintEffect *resultEffect = effect->effect( 0 );
  QgsBlurEffect *blurEffect = dynamic_cast<QgsBlurEffect *>( resultEffect );
  QVERIFY( blurEffect );
  delete effect;

  //rendering

  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.setDotsPerMeterX( 96 / 25.4 * 1000 );
  image.setDotsPerMeterY( 96 / 25.4 * 1000 );
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  effect = new QgsEffectStack();
  QgsBlurEffect *blur = new QgsBlurEffect();
  effect->effectList()->append( blur );
  QgsDrawSourceEffect *source = new QgsDrawSourceEffect();
  effect->effectList()->append( source );

  effect->render( *mPicture, context );
  painter.end();

  const bool result = imageCheck( QStringLiteral( "painteffect_stack" ), image, 0 );

  delete effect;
  QVERIFY( result );
}

void TestQgsPaintEffect::layerEffectPolygon()
{
  // test rendering a polygon symbol layer with a paint effect

  const QString polysFileName = mTestDataDir + "polys.shp";
  const QFileInfo polyFileInfo( polysFileName );
  QgsVectorLayer *polysLayer = new QgsVectorLayer( polyFileInfo.filePath(),
      polyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  polysLayer->setSimplifyMethod( simplifyMethod );

  QgsMapSettings ms;
  QgsSimpleFillSymbolLayer *fill = new QgsSimpleFillSymbolLayer;
  fill->setColor( QColor( 255, 0, 0 ) );
  QgsDropShadowEffect *effect = new QgsDropShadowEffect();
  fill->setPaintEffect( effect );

  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, fill );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( fillSymbol );

  polysLayer->setRenderer( renderer );
  ms.setLayers( QList<QgsMapLayer *>() << polysLayer );
  ms.setExtent( polysLayer->extent() );

  const bool result = mapRenderCheck( QStringLiteral( "painteffect_poly" ), ms );
  QVERIFY( result );
  delete polysLayer;
}

void TestQgsPaintEffect::layerEffectLine()
{
  // test rendering a line symbol layer with a paint effect
  const QString linesFileName = mTestDataDir + "lines.shp";
  const QFileInfo lineFileInfo( linesFileName );
  QgsVectorLayer *lineLayer = new QgsVectorLayer( lineFileInfo.filePath(),
      lineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  lineLayer->setSimplifyMethod( simplifyMethod );

  QgsMapSettings ms;
  QgsSimpleLineSymbolLayer *line = new QgsSimpleLineSymbolLayer;
  line->setColor( QColor( 255, 0, 0 ) );
  line->setWidth( 1.0 );
  QgsDropShadowEffect *effect = new QgsDropShadowEffect();
  line->setPaintEffect( effect );

  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, line );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( lineSymbol );

  lineLayer->setRenderer( renderer );
  ms.setLayers( QList<QgsMapLayer *>() << lineLayer );
  ms.setExtent( lineLayer->extent() );

  const bool result = mapRenderCheck( QStringLiteral( "painteffect_line" ), ms );
  QVERIFY( result );
  delete lineLayer;
}

void TestQgsPaintEffect::layerEffectMarker()
{
  // test rendering a marker symbol layer with a paint effect
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );
  QgsVectorLayer *pointLayer = new QgsVectorLayer( pointFileInfo.filePath(),
      pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsMapSettings ms;
  QgsSimpleMarkerSymbolLayer *marker = new QgsSimpleMarkerSymbolLayer;
  marker->setColor( QColor( 255, 0, 0 ) );
  QgsDropShadowEffect *effect = new QgsDropShadowEffect();
  marker->setPaintEffect( effect );

  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol();
  markerSymbol->changeSymbolLayer( 0, marker );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( markerSymbol );

  pointLayer->setRenderer( renderer );
  ms.setLayers( QList<QgsMapLayer *>() << pointLayer );
  ms.setExtent( pointLayer->extent() );

  const bool result = mapRenderCheck( QStringLiteral( "painteffect_marker" ), ms );
  QVERIFY( result );
  delete pointLayer;
}

void TestQgsPaintEffect::vectorLayerEffect()
{
  // test rendering a whole vector layer with a layer-wide effect
  const QString polysFileName = mTestDataDir + "polys.shp";
  const QFileInfo polyFileInfo( polysFileName );
  QgsVectorLayer *polysLayer = new QgsVectorLayer( polyFileInfo.filePath(),
      polyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  polysLayer->setSimplifyMethod( simplifyMethod );

  QgsMapSettings ms;
  QgsSimpleFillSymbolLayer *fill = new QgsSimpleFillSymbolLayer;
  fill->setColor( QColor( 255, 0, 0 ) );

  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, fill );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( fillSymbol );

  QgsOuterGlowEffect *effect = new QgsOuterGlowEffect();
  effect->setSpread( 30.0 );
  effect->setColor( QColor( 255, 0, 0 ) );
  renderer->setPaintEffect( effect );

  polysLayer->setRenderer( renderer );

  ms.setLayers( QList<QgsMapLayer *>() << polysLayer );
  ms.setExtent( polysLayer->extent() );

  const bool result = mapRenderCheck( QStringLiteral( "painteffect_layer" ), ms );
  QVERIFY( result );
  delete polysLayer;
}

void TestQgsPaintEffect::mapUnits()
{
  //test rendering an effect which utilizes map units
  const QString linesFileName = mTestDataDir + "lines.shp";
  const QFileInfo lineFileInfo( linesFileName );
  QgsVectorLayer *lineLayer = new QgsVectorLayer( lineFileInfo.filePath(),
      lineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  lineLayer->setSimplifyMethod( simplifyMethod );

  QgsMapSettings ms;
  QgsSimpleLineSymbolLayer *line = new QgsSimpleLineSymbolLayer;
  line->setColor( QColor( 255, 0, 0 ) );
  line->setWidth( 1.0 );

  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, line );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( lineSymbol );
  QgsOuterGlowEffect *effect = new QgsOuterGlowEffect();
  effect->setColor( QColor( 255, 0, 0 ) );
  effect->setSpread( 3 );
  effect->setSpreadUnit( QgsUnitTypes::RenderMapUnits );
  renderer->setPaintEffect( effect );

  lineLayer->setRenderer( renderer );
  ms.setLayers( QList<QgsMapLayer *>() << lineLayer );
  ms.setExtent( lineLayer->extent() );

  const bool result = mapRenderCheck( QStringLiteral( "painteffect_mapunits" ), ms );
  QVERIFY( result );
  delete lineLayer;
}

void TestQgsPaintEffect::layout()
{
  //test rendering an effect inside a layout (tests DPI scaling of effects)

  const QString linesFileName = mTestDataDir + "lines.shp";
  const QFileInfo lineFileInfo( linesFileName );
  QgsVectorLayer *lineLayer = new QgsVectorLayer( lineFileInfo.filePath(),
      lineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  lineLayer->setSimplifyMethod( simplifyMethod );

  QgsSimpleLineSymbolLayer *line = new QgsSimpleLineSymbolLayer;
  line->setColor( QColor( 255, 0, 0 ) );
  line->setWidth( 1.0 );

  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, line );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( lineSymbol );
  QgsEffectStack *effect = new QgsEffectStack();
  effect->appendEffect( new QgsDropShadowEffect() );
  effect->appendEffect( new QgsDrawSourceEffect() );
  renderer->setPaintEffect( effect );

  lineLayer->setRenderer( renderer );

  QgsLayout l( QgsProject::instance() );
  std::unique_ptr< QgsLayoutItemPage > page = std::make_unique< QgsLayoutItemPage >( &l );
  page->setPageSize( QgsLayoutSize( 50, 50 ) );
  l.pageCollection()->addPage( page.release() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 1, 1, 48, 48 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( lineLayer->extent() );
  map->setLayers( QList<QgsMapLayer *>() << lineLayer );

  QImage outputImage( 591, 591, QImage::Format_RGB32 );
  outputImage.setDotsPerMeterX( 300 / 25.4 * 1000 );
  outputImage.setDotsPerMeterY( 300 / 25.4 * 1000 );
  QgsMultiRenderChecker::drawBackground( &outputImage );
  QPainter p( &outputImage );
  const QgsLayoutExporter exporter( &l );
  exporter.renderPage( &p, 0 );
  p.end();

  const bool result = imageCheck( QStringLiteral( "painteffect_composer" ), outputImage );
  QVERIFY( result );
  delete lineLayer;
}


//
// Private helper functions not called directly by CTest
//

bool TestQgsPaintEffect::imageCheck( const QString &testName, QImage &image, int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  const QString tempDir = QDir::tempPath() + '/';
  const QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "effects" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  const bool resultFlag = checker.compareImages( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}

bool TestQgsPaintEffect::mapRenderCheck( const QString &testName, QgsMapSettings &mapSettings, int mismatchCount )
{
  QgsMultiRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "effects" ) );
  mapSettings.setOutputDpi( 96 );
  checker.setControlName( "expected_" + testName );
  checker.setMapSettings( mapSettings );
  checker.setColorTolerance( 20 );
  const bool result = checker.runTest( testName, mismatchCount );
  mReport += checker.report();
  return result;
}

QGSTEST_MAIN( TestQgsPaintEffect )
#include "testqgspainteffect.moc"
