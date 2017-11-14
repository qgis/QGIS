/***************************************************************************
                         testqgscomposerscalebar.cpp
                         ---------------------------
    begin                : August 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgsmultirenderchecker.h"
#include "qgscomposermap.h"
#include "qgscomposerscalebar.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsfontutils.h"
#include "qgsproperty.h"
#include "qgsproject.h"

#include <QLocale>
#include <QObject>
#include "qgstest.h"

class TestQgsComposerScaleBar : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerScaleBar() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void singleBox();
    void singleBoxAlpha();
    void doubleBox();
    void numeric();
    void tick();
    void dataDefined();

  private:
    QgsComposition *mComposition = nullptr;
    QgsComposerMap *mComposerMap = nullptr;
    QgsComposerScaleBar *mComposerScaleBar = nullptr;
    QgsRasterLayer *mRasterLayer = nullptr;
    QString mReport;
};

void TestQgsComposerScaleBar::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // the scale denominator is formatted in a locale aware manner
  // so 10000 is rendered as "10,000" in C (or en_US) locale, however
  // other locales may render the number differently (e.g. "10 000" in cs_CZ)
  // so we enforce C locale to make sure we get expected result
  QLocale::setDefault( QLocale::c() );

  //reproject to WGS84
  QgsCoordinateReferenceSystem destCRS;
  destCRS.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsProject::instance()->setCrs( destCRS );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  //create composition with composer map

  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 150, 150 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );
  mComposerMap->setNewExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );
  mComposerMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );

  mComposerScaleBar = new QgsComposerScaleBar( mComposition );
  mComposerScaleBar->setSceneRect( QRectF( 20, 180, 50, 20 ) );
  mComposition->addComposerScaleBar( mComposerScaleBar );
  mComposerScaleBar->setComposerMap( mComposerMap );
  mComposerScaleBar->setFont( QgsFontUtils::getStandardTestFont() );
  mComposerScaleBar->setUnits( QgsUnitTypes::DistanceMeters );
  mComposerScaleBar->setNumUnitsPerSegment( 2000 );
  mComposerScaleBar->setNumSegmentsLeft( 0 );
  mComposerScaleBar->setNumSegments( 2 );
  mComposerScaleBar->setHeight( 5 );
  mComposerScaleBar->setLineWidth( 1.0 );

  qWarning() << "scalebar font: " << mComposerScaleBar->font().toString() << " exactMatch:" << mComposerScaleBar->font().exactMatch();

  mReport = QStringLiteral( "<h1>Composer Scalebar Tests</h1>\n" );
}

void TestQgsComposerScaleBar::cleanupTestCase()
{
  delete mComposition;
  delete mRasterLayer;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsComposerScaleBar::init()
{

}

void TestQgsComposerScaleBar::cleanup()
{

}

void TestQgsComposerScaleBar::singleBox()
{
  mComposerScaleBar->setStyle( QStringLiteral( "Single Box" ) );
  QgsCompositionChecker checker( QStringLiteral( "composerscalebar_singlebox" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_scalebar" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerScaleBar::singleBoxAlpha()
{
  mComposerScaleBar->setStyle( QStringLiteral( "Single Box" ) );
  mComposerScaleBar->setFillColor( QColor( 255, 0, 0, 100 ) );
  mComposerScaleBar->setFillColor2( QColor( 0, 255, 0, 50 ) );
  mComposerScaleBar->setLineColor( QColor( 0, 0, 255, 150 ) );
  mComposerScaleBar->setFontColor( QColor( 255, 0, 255, 100 ) );
  mComposerScaleBar->setLineWidth( 1.0 );
  QgsCompositionChecker checker( QStringLiteral( "composerscalebar_singlebox_alpha" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_scalebar" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerScaleBar::doubleBox()
{
  // cleanup singleBoxAlpha
  mComposerScaleBar->setFillColor( Qt::black );
  mComposerScaleBar->setFillColor2( Qt::white );
  mComposerScaleBar->setLineColor( Qt::black );
  mComposerScaleBar->setLineWidth( 1.0 );
  mComposerScaleBar->setFontColor( Qt::black );
  mComposerScaleBar->setStyle( QStringLiteral( "Double Box" ) );

  QgsCompositionChecker checker( QStringLiteral( "composerscalebar_doublebox" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_scalebar" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerScaleBar::numeric()
{
  QFont f = mComposerScaleBar->font();

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  mComposerScaleBar->setFont( newFont );

  mComposerScaleBar->setStyle( QStringLiteral( "Numeric" ) );
  mComposerScaleBar->setSceneRect( QRectF( 20, 180, 50, 20 ) );
  QgsCompositionChecker checker( QStringLiteral( "composerscalebar_numeric" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_scalebar" ) );
  bool result = checker.testComposition( mReport, 0, 0 );
  mComposerScaleBar->setFont( f );
  QVERIFY( result );
}

void TestQgsComposerScaleBar::tick()
{
  mComposerScaleBar->setStyle( QStringLiteral( "Line Ticks Up" ) );
  mComposerScaleBar->setLineWidth( 1.0 );
  QgsCompositionChecker checker( QStringLiteral( "composerscalebar_tick" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_scalebar" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerScaleBar::dataDefined()
{
  mComposerScaleBar->dataDefinedProperties().setProperty( QgsComposerObject::ScalebarFillColor, QgsProperty::fromExpression( QStringLiteral( "'red'" ) ) );
  mComposerScaleBar->dataDefinedProperties().setProperty( QgsComposerObject::ScalebarFillColor2, QgsProperty::fromExpression( QStringLiteral( "'blue'" ) ) );
  mComposerScaleBar->dataDefinedProperties().setProperty( QgsComposerObject::ScalebarLineColor, QgsProperty::fromExpression( QStringLiteral( "'yellow'" ) ) );
  mComposerScaleBar->dataDefinedProperties().setProperty( QgsComposerObject::ScalebarLineWidth, QgsProperty::fromExpression( QStringLiteral( "1.2" ) ) );
  mComposerScaleBar->refreshDataDefinedProperty();
  QCOMPARE( mComposerScaleBar->brush().color().name(), QColor( 255, 0, 0 ).name() );
  QCOMPARE( mComposerScaleBar->brush2().color().name(), QColor( 0, 0, 255 ).name() );
  QCOMPARE( mComposerScaleBar->pen().color().name(), QColor( 255, 255, 0 ).name() );
  QCOMPARE( mComposerScaleBar->pen().widthF(), 1.2 );
  mComposerScaleBar->setDataDefinedProperties( QgsPropertyCollection() );
  mComposerScaleBar->setLineWidth( 1.0 );
}

QGSTEST_MAIN( TestQgsComposerScaleBar )
#include "testqgscomposerscalebar.moc"
