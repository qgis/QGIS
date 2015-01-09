
#include <QtTest/QtTest>
#include <QObject>

#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstylemanager.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsvectorlayer.h"


class TestQgsMapLayerStyleManager : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testDefault();
    void testStyle();
    void testReadWrite();
    void testSwitchingStyles();

  private:
    QgsVectorLayer* mVL;
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
  mVL = new QgsVectorLayer( "LineString", "Line Layer", "memory" );
  QgsMapLayerRegistry::instance()->addMapLayer( mVL );
}

void TestQgsMapLayerStyleManager::cleanup()
{
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
}

void TestQgsMapLayerStyleManager::testDefault()
{
  QgsMapLayerStyleManager* mgr = mVL->styleManager();
  QVERIFY( mgr );

  QCOMPARE( mgr->styles().count(), 1 );
  QCOMPARE( mgr->style( QString() ).isValid(), true );
}

void TestQgsMapLayerStyleManager::testStyle()
{
  QgsLineSymbolV2* sym1 = new QgsLineSymbolV2();
  sym1->setColor( Qt::magenta );
  QgsLineSymbolV2* sym2 = new QgsLineSymbolV2();
  sym2->setColor( Qt::red );

  QgsMapLayerStyle st;
  QCOMPARE( st.isValid(), false );

  mVL->setRendererV2( new QgsSingleSymbolRendererV2( sym1 ) );

  QgsMapLayerStyle st1;
  st1.readFromLayer( mVL );
  QCOMPARE( st1.isValid(), true );

  qDebug( "CNT-1: %s", st1.dump().toAscii().data() );

  mVL->setRendererV2( new QgsSingleSymbolRendererV2( sym2 ) );

  QgsMapLayerStyle st2;
  st2.readFromLayer( mVL );

  qDebug( "CNT-2: %s", st2.dump().toAscii().data() );

  st1.writeToLayer( mVL );

  QgsSingleSymbolRendererV2* r1 = dynamic_cast<QgsSingleSymbolRendererV2*>( mVL->rendererV2() );
  QVERIFY( r1 );
  QCOMPARE( r1->symbol()->color(), QColor( Qt::magenta ) );

  st2.writeToLayer( mVL );

  QgsSingleSymbolRendererV2* r2 = dynamic_cast<QgsSingleSymbolRendererV2*>( mVL->rendererV2() );
  QVERIFY( r2 );
  QCOMPARE( r2->symbol()->color(), QColor( Qt::red ) );
}


void TestQgsMapLayerStyleManager::testReadWrite()
{
  QgsSingleSymbolRendererV2* r0 = dynamic_cast<QgsSingleSymbolRendererV2*>( mVL->rendererV2() );
  r0->symbol()->setColor( Qt::red );

  // create and populate the manager with one more style

  QgsMapLayerStyleManager sm0( mVL );

  sm0.addStyleFromLayer( "blue" );
  sm0.setCurrentStyle( "blue" );
  QgsSingleSymbolRendererV2* r1 = dynamic_cast<QgsSingleSymbolRendererV2*>( mVL->rendererV2() );
  r1->symbol()->setColor( Qt::blue );

  // read and write

  QDomDocument doc;
  QDomElement mgrElem = doc.createElement( "map-layer-style-manager" );
  doc.appendChild( mgrElem );
  sm0.writeXml( mgrElem );

  QString xml;
  QTextStream ts( &xml );
  doc.save( ts, 2 );
  qDebug( "%s", xml.toAscii().data() );

  QgsMapLayerStyleManager sm1( mVL );
  sm1.readXml( mgrElem );

  QCOMPARE( sm1.styles().count(), 2 );
  QCOMPARE( sm1.style( QString() ).isValid(), true );
  QCOMPARE( sm1.style( "blue" ).isValid(), true );
  QCOMPARE( sm1.currentStyle(), QString( "blue" ) );

  // now use the default style - the symbol should get red color
  sm1.setCurrentStyle( QString() );

  QgsSingleSymbolRendererV2* r2 = dynamic_cast<QgsSingleSymbolRendererV2*>( mVL->rendererV2() );
  QCOMPARE( r2->symbol()->color(), QColor( Qt::red ) );
}

static void _setVLColor( QgsVectorLayer* vl, const QColor& c )
{
  dynamic_cast<QgsSingleSymbolRendererV2*>( vl->rendererV2() )->symbol()->setColor( c );
}

static QColor _getVLColor( QgsVectorLayer* vl )
{
  return dynamic_cast<QgsSingleSymbolRendererV2*>( vl->rendererV2() )->symbol()->color();
}

void TestQgsMapLayerStyleManager::testSwitchingStyles()
{
  _setVLColor( mVL, Qt::red );

  mVL->styleManager()->addStyleFromLayer( "s1" );
  mVL->styleManager()->setCurrentStyle( "s1" );

  QCOMPARE( mVL->styleManager()->currentStyle(), QString( "s1" ) );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  _setVLColor( mVL, Qt::green );

  mVL->styleManager()->setCurrentStyle( QString() );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  mVL->styleManager()->setCurrentStyle( "s1" );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::green ) );

  _setVLColor( mVL, Qt::blue );

  mVL->styleManager()->setCurrentStyle( QString() );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::red ) );

  mVL->styleManager()->setCurrentStyle( "s1" );
  QCOMPARE( _getVLColor( mVL ), QColor( Qt::blue ) );
}


QTEST_MAIN( TestQgsMapLayerStyleManager )
#include "testqgsmaplayerstylemanager.moc"
