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
#include <QtTest/QtTest>
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
#include "qgspainteffectregistry.h"
#include "qgsvectorcolorrampv2.h"
#include "qgssymbollayerv2utils.h"

//qgis test includes
#include "qgsmultirenderchecker.h"

//dummy paint effect for testing
class DummyPaintEffect : public QgsPaintEffect
{
  public:
    DummyPaintEffect( QString prop1, QString prop2 ) : mProp1( prop1 ), mProp2( prop2 ) {}
    virtual ~DummyPaintEffect() {}
    virtual QString type() const override { return "Dummy"; }
    virtual QgsPaintEffect* clone() const override { return new DummyPaintEffect( mProp1, mProp2 ); }
    static QgsPaintEffect* create( const QgsStringMap& props ) { return new DummyPaintEffect( props["testProp"], props["testProp2"] ); }
    virtual QgsStringMap properties() const override
    {
      QgsStringMap props;
      props["testProp"] = mProp1;
      props["testProp2"] = mProp2;
      return props;
    }
    virtual void readProperties( const QgsStringMap& props ) override
    {
      mProp1 = props["testProp"];
      mProp2 = props["testProp2"];
    }

    QString prop1() { return mProp1; }
    QString prop2() { return mProp2; }

  protected:

    virtual void draw( QgsRenderContext& context ) override { Q_UNUSED( context ); }

  private:
    QString mProp1;
    QString mProp2;
};



/** \ingroup UnitTests
 * This is a unit test for paint effects
 */
class TestQgsPaintEffect: public QObject
{
    Q_OBJECT
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

    void stack();

  private:
    bool imageCheck( QString testName , QImage &image, int mismatchCount );
    QString mReport;

    QPicture* mPicture;
};


void TestQgsPaintEffect::initTestCase()
{
  mReport += "<h1>Paint Effect Tests</h1>\n";
  mPicture = 0;

  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  registry->addEffectType( new QgsPaintEffectMetadata( "Dummy", "Dummy effect", DummyPaintEffect::create ) );
}

void TestQgsPaintEffect::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
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
  DummyPaintEffect* effect = new DummyPaintEffect( "a", "b" );

  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement rootNode = doc.createElement( "qgis" );
  QDomElement noNode;
  QCOMPARE( effect->saveProperties( doc, noNode ), false );

  //test writing with node
  QDomElement effectParentElem = doc.createElement( "parent" );
  rootNode.appendChild( effectParentElem );
  QVERIFY( effect->saveProperties( doc, effectParentElem ) );

  //check if effect node was written
  QDomNodeList evalNodeList = effectParentElem.elementsByTagName( "effect" );
  QCOMPARE( evalNodeList.count(), 1 );

  QDomElement effectElem = evalNodeList.at( 0 ).toElement();
  QCOMPARE( effectElem.attribute( "type" ), QString( "Dummy" ) );

  //test reading empty node
  QgsPaintEffect* restoredEffect = QgsPaintEffectRegistry::instance()->createEffect( noNode );
  QVERIFY( !restoredEffect );

  //test reading bad node
  QDomElement badEffectElem = doc.createElement( "parent" );
  badEffectElem.setAttribute( "type", "bad" );
  restoredEffect = QgsPaintEffectRegistry::instance()->createEffect( badEffectElem );
  QVERIFY( !restoredEffect );

  //test reading node
  restoredEffect = QgsPaintEffectRegistry::instance()->createEffect( effectElem );
  QVERIFY( restoredEffect );
  DummyPaintEffect* restoredDummyEffect = dynamic_cast<DummyPaintEffect*>( restoredEffect );
  QVERIFY( restoredDummyEffect );

  //test properties
  QCOMPARE( restoredDummyEffect->prop1(), effect->prop1() );
  QCOMPARE( restoredDummyEffect->prop2(), effect->prop2() );

  delete effect;
  delete restoredEffect;
}

void TestQgsPaintEffect::stackSaveRestore()
{
  QgsEffectStack* stack = new QgsEffectStack();
  //add two effects to stack
  QgsBlurEffect* blur = new QgsBlurEffect();
  QgsDropShadowEffect* shadow = new QgsDropShadowEffect();
  stack->appendEffect( blur );
  stack->appendEffect( shadow );
  stack->setEnabled( false );

  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement rootNode = doc.createElement( "qgis" );
  QDomElement noNode;
  QCOMPARE( stack->saveProperties( doc, noNode ), false );

  //test writing with node
  QDomElement effectParentElem = doc.createElement( "parent" );
  rootNode.appendChild( effectParentElem );
  QVERIFY( stack->saveProperties( doc, effectParentElem ) );

  //check if stack effect node was written
  QDomNodeList evalNodeList = effectParentElem.childNodes();
  QCOMPARE( evalNodeList.length(), ( unsigned int )1 );
  QDomElement effectElem = evalNodeList.at( 0 ).toElement();
  QCOMPARE( effectElem.attribute( "type" ), stack->type() );

  //should be two effect child nodes
  QDomNodeList childNodeList = effectElem.elementsByTagName( "effect" );
  QCOMPARE( childNodeList.length(), ( unsigned int )2 );
  QCOMPARE( childNodeList.at( 0 ).toElement().attribute( "type" ), blur->type() );
  QCOMPARE( childNodeList.at( 1 ).toElement().attribute( "type" ), shadow->type() );

  //test reading node
  QgsPaintEffect* restoredEffect = QgsPaintEffectRegistry::instance()->createEffect( effectElem );
  QVERIFY( restoredEffect );
  QgsEffectStack* restoredStack = dynamic_cast<QgsEffectStack*>( restoredEffect );
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
  QgsDrawSourceEffect* effect = new QgsDrawSourceEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setTransparency( 0.5 );
  QCOMPARE( effect->transparency(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsDrawSourceEffect* copy = new QgsDrawSourceEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->transparency(), effect->transparency() );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->drawMode(), effect->drawMode() );
  delete copy;

  //clone
  QgsPaintEffect* clone = effect->clone();
  QgsDrawSourceEffect* cloneCast = dynamic_cast<QgsDrawSourceEffect* >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->transparency(), effect->transparency() );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->drawMode(), effect->drawMode() );
  delete cloneCast;

  //read/write
  QgsStringMap props = effect->properties();
  QgsPaintEffect* readEffect = QgsDrawSourceEffect::create( props );
  QgsDrawSourceEffect* readCast = dynamic_cast<QgsDrawSourceEffect* >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->transparency(), effect->transparency() );
  QCOMPARE( readCast->enabled(), effect->enabled() );
  QCOMPARE( readCast->drawMode(), effect->drawMode() );
  delete readCast;

  delete effect;

  //test render
  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsSymbolLayerV2Utils::createRenderContext( &painter );

  effect = new QgsDrawSourceEffect();
  effect->render( *mPicture, context );
  painter.end();

  bool result = imageCheck( QString( "painteffect_drawsource" ), image, 0 );

  delete effect;
  QVERIFY( result );
}

void TestQgsPaintEffect::blur()
{
  //create
  QgsBlurEffect* effect = new QgsBlurEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setTransparency( 0.5 );
  QCOMPARE( effect->transparency(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setBlurLevel( 6 );
  QCOMPARE( effect->blurLevel(), 6 );
  effect->setBlurMethod( QgsBlurEffect::GaussianBlur );
  QCOMPARE( effect->blurMethod(), QgsBlurEffect::GaussianBlur );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsBlurEffect* copy = new QgsBlurEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->transparency(), effect->transparency() );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->blurLevel(), effect->blurLevel() );
  QCOMPARE( copy->blurMethod(), effect->blurMethod() );
  QCOMPARE( copy->drawMode(), effect->drawMode() );
  delete copy;

  //clone
  QgsPaintEffect* clone = effect->clone();
  QgsBlurEffect* cloneCast = dynamic_cast<QgsBlurEffect* >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->transparency(), effect->transparency() );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( cloneCast->blurMethod(), effect->blurMethod() );
  QCOMPARE( cloneCast->drawMode(), effect->drawMode() );
  delete cloneCast;

  //read/write
  QgsStringMap props = effect->properties();
  QgsPaintEffect* readEffect = QgsBlurEffect::create( props );
  QgsBlurEffect* readCast = dynamic_cast<QgsBlurEffect* >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->transparency(), effect->transparency() );
  QCOMPARE( readCast->enabled(), effect->enabled() );
  QCOMPARE( readCast->blurLevel(), effect->blurLevel() );
  QCOMPARE( readCast->blurMethod(), effect->blurMethod() );
  QCOMPARE( readCast->drawMode(), effect->drawMode() );
  delete readCast;

  delete effect;

  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsSymbolLayerV2Utils::createRenderContext( &painter );

  effect = new QgsBlurEffect();
  effect->render( *mPicture, context );
  painter.end();

  bool result = imageCheck( QString( "painteffect_blur" ), image, 0 );

  delete effect;
  QVERIFY( result );
}

void TestQgsPaintEffect::dropShadow()
{
  //create
  QgsDropShadowEffect* effect = new QgsDropShadowEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setTransparency( 0.5 );
  QCOMPARE( effect->transparency(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setBlurLevel( 6 );
  QCOMPARE( effect->blurLevel(), 6 );
  effect->setOffsetAngle( 77 );
  QCOMPARE( effect->offsetAngle(), 77 );
  effect->setOffsetDistance( 9.7 );
  QCOMPARE( effect->offsetDistance(), 9.7 );
  effect->setOffsetUnit( QgsSymbolV2::MapUnit );
  QCOMPARE( effect->offsetUnit(), QgsSymbolV2::MapUnit );
  effect->setOffsetMapUnitScale( QgsMapUnitScale( 1.0, 2.0 ) );
  QCOMPARE( effect->offsetMapUnitScale().minScale, 1.0 );
  QCOMPARE( effect->offsetMapUnitScale().maxScale, 2.0 );
  effect->setColor( QColor( 56, 67, 89 ) );
  QCOMPARE( effect->color(), QColor( 56, 67, 89 ) );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsDropShadowEffect* copy = new QgsDropShadowEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->transparency(), effect->transparency() );
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
  QgsPaintEffect* clone = effect->clone();
  QgsDropShadowEffect* cloneCast = dynamic_cast<QgsDropShadowEffect* >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->transparency(), effect->transparency() );
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
  QgsStringMap props = effect->properties();
  QgsPaintEffect* readEffect = QgsDropShadowEffect::create( props );
  QgsDropShadowEffect* readCast = dynamic_cast<QgsDropShadowEffect* >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->transparency(), effect->transparency() );
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
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsSymbolLayerV2Utils::createRenderContext( &painter );

  effect = new QgsDropShadowEffect();
  effect->render( *mPicture, context );
  painter.end();

  bool result = imageCheck( QString( "painteffect_dropshadow" ), image, 0 );
  delete effect;
  QVERIFY( result );
}

void TestQgsPaintEffect::glow()
{
  //create
  QgsOuterGlowEffect* effect = new QgsOuterGlowEffect();
  QVERIFY( effect );
  effect->setBlendMode( QPainter::CompositionMode_ColorBurn );
  QCOMPARE( effect->blendMode(), QPainter::CompositionMode_ColorBurn );
  effect->setTransparency( 0.5 );
  QCOMPARE( effect->transparency(), 0.5 );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  effect->setBlurLevel( 6 );
  QCOMPARE( effect->blurLevel(), 6 );
  effect->setSpread( 7.8 );
  QCOMPARE( effect->spread(), 7.8 );
  effect->setSpreadUnit( QgsSymbolV2::MapUnit );
  QCOMPARE( effect->spreadUnit(), QgsSymbolV2::MapUnit );
  effect->setSpreadMapUnitScale( QgsMapUnitScale( 1.0, 2.0 ) );
  QCOMPARE( effect->spreadMapUnitScale().minScale, 1.0 );
  QCOMPARE( effect->spreadMapUnitScale().maxScale, 2.0 );
  effect->setRamp( new QgsVectorGradientColorRampV2( QColor( 255, 0, 0 ), QColor( 0, 255, 0 ) ) );
  QCOMPARE( effect->ramp()->color( 0 ), QColor( 255, 0, 0 ) );
  QCOMPARE( effect->ramp()->color( 1.0 ), QColor( 0, 255, 0 ) );
  effect->setColorType( QgsGlowEffect::ColorRamp );
  QCOMPARE( effect->colorType(), QgsGlowEffect::ColorRamp );
  effect->setColor( QColor( 56, 67, 89 ) );
  QCOMPARE( effect->color(), QColor( 56, 67, 89 ) );
  effect->setDrawMode( QgsPaintEffect::Modifier );
  QCOMPARE( effect->drawMode(), QgsPaintEffect::Modifier );

  //copy constructor
  QgsOuterGlowEffect* copy = new QgsOuterGlowEffect( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->blendMode(), effect->blendMode() );
  QCOMPARE( copy->transparency(), effect->transparency() );
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
  QgsPaintEffect* clone = effect->clone();
  QgsOuterGlowEffect* cloneCast = dynamic_cast<QgsOuterGlowEffect* >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->blendMode(), effect->blendMode() );
  QCOMPARE( cloneCast->transparency(), effect->transparency() );
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
  QgsStringMap props = effect->properties();
  QgsPaintEffect* readEffect = QgsOuterGlowEffect::create( props );
  QgsOuterGlowEffect* readCast = dynamic_cast<QgsOuterGlowEffect* >( readEffect );
  QVERIFY( readCast );
  QCOMPARE( readCast->blendMode(), effect->blendMode() );
  QCOMPARE( readCast->transparency(), effect->transparency() );
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
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsSymbolLayerV2Utils::createRenderContext( &painter );

  effect = new QgsOuterGlowEffect();
  effect->setSpread( 20 );
  effect->render( *mPicture, context );
  painter.end();

  delete effect;

  bool result = imageCheck( QString( "painteffect_outerglow" ), image, 0 );
  QVERIFY( result );

  //TODO - inner glow

}

void TestQgsPaintEffect::stack()
{
  //create
  QgsEffectStack* effect = new QgsEffectStack();
  QVERIFY( effect );
  effect->appendEffect( new QgsDrawSourceEffect() );
  effect->appendEffect( new QgsDrawSourceEffect() );
  effect->setEnabled( false );
  QCOMPARE( effect->enabled(), false );
  QCOMPARE( effect->count(), 2 );

  //copy constructor
  QgsEffectStack* copy = new QgsEffectStack( *effect );
  QVERIFY( copy );
  QCOMPARE( copy->enabled(), effect->enabled() );
  QCOMPARE( copy->count(), effect->count() );
  delete copy;

  //clone
  QgsPaintEffect* clone = effect->clone();
  QgsEffectStack* cloneCast = dynamic_cast<QgsEffectStack* >( clone );
  QVERIFY( cloneCast );
  QCOMPARE( cloneCast->enabled(), effect->enabled() );
  QCOMPARE( cloneCast->count(), effect->count() );
  delete cloneCast;

  delete effect;

  //stack from single effect
  QgsBlurEffect singleEffect;
  effect = new QgsEffectStack( singleEffect );
  QVERIFY( effect );
  QCOMPARE( effect->count(), 1 );
  QgsPaintEffect* resultEffect = effect->effect( 0 );
  QgsBlurEffect* blurEffect = dynamic_cast<QgsBlurEffect*>( resultEffect );
  QVERIFY( blurEffect );
  delete effect;

  //rendering

  QImage image( 100, 100, QImage::Format_ARGB32 );
  image.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &image );
  QgsRenderContext context = QgsSymbolLayerV2Utils::createRenderContext( &painter );

  effect = new QgsEffectStack();
  QgsBlurEffect* blur = new QgsBlurEffect();
  effect->effectList()->append( blur );
  QgsDrawSourceEffect* source = new QgsDrawSourceEffect();
  effect->effectList()->append( source );

  effect->render( *mPicture, context );
  painter.end();

  bool result = imageCheck( QString( "painteffect_stack" ), image, 0 );

  delete effect;
  QVERIFY( result );
}


//
// Private helper functions not called directly by CTest
//

bool TestQgsPaintEffect::imageCheck( QString testName, QImage &image, int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  mReport += "<h2>" + testName + "</h2>\n";
  QString tempDir = QDir::tempPath() + QDir::separator();
  QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsRenderChecker checker;
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  bool resultFlag = checker.compareImages( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}

QTEST_MAIN( TestQgsPaintEffect )
#include "testqgspainteffect.moc"
