/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Frida  Nov 23  2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim@linfiniti.com
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
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QTime>
#include <QDesktopServices>

#include "cpl_conv.h"
#include "gdal.h"

//qgis includes...
#include <qgsrasterlayer.h>
#include <qgsrasterpyramid.h>
#include <qgsrasterbandstats.h>
#include <qgsrasteridentifyresult.h>
#include <qgsproject.h>
#include <qgsapplication.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgscolorramp.h>
#include <qgscptcityarchive.h>
#include "qgscolorrampshader.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastershader.h"
#include "qgsrastertransparency.h"

//qgis unit test includes
#include <qgsrenderchecker.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsRasterLayer : public QObject
{
    Q_OBJECT
  public:
    TestQgsRasterLayer() = default;
    ~TestQgsRasterLayer() override
    {
      delete mMapSettings;
    }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void isValid();
    void isSpatial();
    void pseudoColor();
    void colorRamp1();
    void colorRamp2();
    void colorRamp3();
    void colorRamp4();
    void landsatBasic();
    void landsatBasic875Qml();
    void checkDimensions();
    void checkStats();
    void checkScaleOffset();
    void buildExternalOverviews();
    void registry();
    void transparency();
    void multiBandColorRenderer();
    void setRenderer();
    void regression992(); //test for issue #992 - GeoJP2 images improperly displayed as all black
    void testRefreshRendererIfNeeded();
    void sample();


  private:
    bool render( const QString &fileName, int mismatchCount = 0 );
    bool setQml( const QString &type, QString &msg );
    void populateColorRampShader( QgsColorRampShader *colorRampShader,
                                  QgsColorRamp *colorRamp,
                                  int numberOfEntries );
    bool testColorRamp( const QString &name, QgsColorRamp *colorRamp,
                        QgsColorRampShader::Type type, int numberOfEntries );
    QString mTestDataDir;
    QgsRasterLayer *mpRasterLayer = nullptr;
    QgsRasterLayer *mpLandsatRasterLayer = nullptr;
    QgsRasterLayer *mpFloat32RasterLayer = nullptr;
    QgsRasterLayer *mPngRasterLayer = nullptr;
    QgsRasterLayer *mGeoJp2RasterLayer = nullptr;

    QgsMapSettings *mMapSettings = nullptr;
    QString mReport;
};

class TestSignalReceiver : public QObject
{
    Q_OBJECT

  public:
    TestSignalReceiver()
      : QObject( nullptr )
    {}
    bool rendererChanged =  false ;
  public slots:
    void onRendererChanged()
    {
      rendererChanged = true;
    }
};

//runs before all tests
void TestQgsRasterLayer::initTestCase()
{
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  // disable any PAM stuff to make sure stats are consistent
  CPLSetConfigOption( "GDAL_PAM_ENABLED", "NO" );
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( '\n', QLatin1String( "<br />" ) );
  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString myFileName = mTestDataDir + "tenbytenraster.asc";
  QString myLandsatFileName = mTestDataDir + "landsat.tif";
  QString myFloat32FileName = mTestDataDir + "/raster/band1_float32_noct_epsg4326.tif";
  QString pngRasterFileName = mTestDataDir + "rgb256x256.png";
  QString geoJp2RasterFileName = mTestDataDir + "rgbwcmyk01_YeGeo.jp2";

  QFileInfo myRasterFileInfo( myFileName );
  mpRasterLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
                                      myRasterFileInfo.completeBaseName() );
  qDebug() << "tenbyteraster metadata: " << mpRasterLayer->dataProvider()->htmlMetadata();

  QFileInfo myLandsatRasterFileInfo( myLandsatFileName );
  mpLandsatRasterLayer = new QgsRasterLayer( myLandsatRasterFileInfo.filePath(),
      myLandsatRasterFileInfo.completeBaseName() );
  qDebug() << "landsat metadata: " << mpLandsatRasterLayer->dataProvider()->htmlMetadata();

  QFileInfo myFloat32RasterFileInfo( myFloat32FileName );
  mpFloat32RasterLayer = new QgsRasterLayer( myFloat32RasterFileInfo.filePath(),
      myFloat32RasterFileInfo.completeBaseName() );
  qDebug() << "float32raster metadata: " << mpFloat32RasterLayer->dataProvider()->htmlMetadata();

  QFileInfo pngRasterFileInfo( pngRasterFileName );
  mPngRasterLayer = new QgsRasterLayer( pngRasterFileInfo.filePath(),
                                        pngRasterFileInfo.completeBaseName() );

  QFileInfo geoJp2RasterFileInfo( geoJp2RasterFileName );
  mGeoJp2RasterLayer = new QgsRasterLayer( geoJp2RasterFileInfo.filePath(),
      geoJp2RasterFileInfo.completeBaseName() );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpRasterLayer
    << mpLandsatRasterLayer
    << mpFloat32RasterLayer
    << mPngRasterLayer
    << mGeoJp2RasterLayer );

  // add the test layer to the maprender
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mpRasterLayer );
  mReport += QLatin1String( "<h1>Raster Layer Tests</h1>\n" );
  mReport += "<p>" + mySettings + "</p>";
}
//runs after all tests
void TestQgsRasterLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }
}

void TestQgsRasterLayer::isValid()
{
  QVERIFY( mpRasterLayer->isValid() );
  mpRasterLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, QgsRasterMinMaxOrigin::MinMax );
  mMapSettings->setExtent( mpRasterLayer->extent() );
  QVERIFY( render( "raster" ) );
}

void TestQgsRasterLayer::isSpatial()
{
  QVERIFY( mpRasterLayer->isSpatial() );
}

void TestQgsRasterLayer::pseudoColor()
{
  QgsRasterShader *rasterShader = new QgsRasterShader();
  QgsColorRampShader *colorRampShader = new QgsColorRampShader();
  colorRampShader->setColorRampType( QgsColorRampShader::Interpolated );

  //items to imitate old pseudo color renderer
  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  QgsColorRampShader::ColorRampItem firstItem;
  firstItem.value = 0.0;
  firstItem.color = QColor( "#0000ff" );
  colorRampItems.append( firstItem );
  QgsColorRampShader::ColorRampItem secondItem;
  secondItem.value = 3.0;
  secondItem.color = QColor( "#00ffff" );
  colorRampItems.append( secondItem );
  QgsColorRampShader::ColorRampItem thirdItem;
  thirdItem.value = 6.0;
  thirdItem.color = QColor( "#ffff00" );
  colorRampItems.append( thirdItem );
  QgsColorRampShader::ColorRampItem fourthItem;
  fourthItem.value = 9.0;
  fourthItem.color = QColor( "#ff0000" );
  colorRampItems.append( fourthItem );

  colorRampShader->setColorRampItemList( colorRampItems );
  rasterShader->setRasterShaderFunction( colorRampShader );
  QgsSingleBandPseudoColorRenderer *r = new QgsSingleBandPseudoColorRenderer( mpRasterLayer->dataProvider(), 1, rasterShader );
  mpRasterLayer->setRenderer( r );
  mMapSettings->setExtent( mpRasterLayer->extent() );
  QVERIFY( render( "raster_pseudo" ) );
}

void TestQgsRasterLayer::populateColorRampShader( QgsColorRampShader *colorRampShader,
    QgsColorRamp *colorRamp,
    int numberOfEntries )

{
  // adapted from QgsSingleBandPseudoColorRendererWidget::on_mClassifyButton_clicked()
  // and TestQgsRasterLayer::pseudoColor()
  // would be better to add a more generic function to api that does this
  int bandNr = 1;
  QgsRasterBandStats myRasterBandStats = mpRasterLayer->dataProvider()->bandStatistics( bandNr );

  QList<double> entryValues;
  QVector<QColor> entryColors;
  double currentValue = myRasterBandStats.minimumValue;
  double intervalDiff;
  if ( numberOfEntries > 1 )
  {
    //because the highest value is also an entry, there are (numberOfEntries - 1)
    //intervals
    intervalDiff = ( myRasterBandStats.maximumValue - myRasterBandStats.minimumValue ) /
                   ( numberOfEntries - 1 );
  }
  else
  {
    intervalDiff = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
  }

  for ( int i = 0; i < numberOfEntries; ++i )
  {
    entryValues.push_back( currentValue );
    currentValue += intervalDiff;
    entryColors.push_back( colorRamp->color( ( ( double ) i ) / numberOfEntries ) );
  }

  //items to imitate old pseudo color renderer
  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  QList<double>::const_iterator value_it = entryValues.constBegin();
  QVector<QColor>::const_iterator color_it = entryColors.constBegin();
  for ( ; value_it != entryValues.constEnd(); ++value_it, ++color_it )
  {
    colorRampItems.append( QgsColorRampShader::ColorRampItem( *value_it, *color_it ) );
  }
  colorRampShader->setColorRampItemList( colorRampItems );
}

bool TestQgsRasterLayer::testColorRamp( const QString &name, QgsColorRamp *colorRamp,
                                        QgsColorRampShader::Type type, int numberOfEntries )
{
  QgsRasterShader *rasterShader = new QgsRasterShader();
  QgsColorRampShader *colorRampShader = new QgsColorRampShader();
  colorRampShader->setColorRampType( type );

  populateColorRampShader( colorRampShader, colorRamp, numberOfEntries );

  rasterShader->setRasterShaderFunction( colorRampShader );
  QgsSingleBandPseudoColorRenderer *r = new QgsSingleBandPseudoColorRenderer( mpRasterLayer->dataProvider(), 1, rasterShader );
  mpRasterLayer->setRenderer( r );
  mMapSettings->setExtent( mpRasterLayer->extent() );
  return render( name );
}

void TestQgsRasterLayer::colorRamp1()
{
  // gradient ramp
  QgsGradientColorRamp *colorRamp = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::black ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( Qt::white ) ) );
  colorRamp->setStops( stops );

  // QVERIFY( testColorRamp( "raster_colorRamp1", colorRamp, QgsColorRampShader::Interpolated, 5 ) );
  QVERIFY( testColorRamp( "raster_colorRamp1", colorRamp, QgsColorRampShader::Discrete, 10 ) );
  delete colorRamp;
}

void TestQgsRasterLayer::colorRamp2()
{
  QgsColorBrewerColorRamp ramp( QStringLiteral( "BrBG" ), 10 );
  // ColorBrewer ramp
  QVERIFY( testColorRamp( "raster_colorRamp2",
                          &ramp,
                          QgsColorRampShader::Discrete, 10 ) );
}

void TestQgsRasterLayer::colorRamp3()
{
  // cpt-city ramp, discrete
  QgsCptCityArchive::initArchives();
  QgsCptCityColorRamp ramp( QStringLiteral( "cb/div/BrBG" ), QStringLiteral( "_10" ) );
  QVERIFY( testColorRamp( "raster_colorRamp3",
                          &ramp,
                          QgsColorRampShader::Discrete, 10 ) );
  QgsCptCityArchive::clearArchives();
}

void TestQgsRasterLayer::colorRamp4()
{
  // cpt-city ramp, continuous
  QgsCptCityColorRamp ramp( QStringLiteral( "grass/elevation" ), QString() );
  QVERIFY( testColorRamp( "raster_colorRamp4",
                          &ramp,
                          QgsColorRampShader::Discrete, 10 ) );
}

void TestQgsRasterLayer::landsatBasic()
{
  QVERIFY2( mpLandsatRasterLayer->isValid(), "landsat.tif layer is not valid!" );
  mpLandsatRasterLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, QgsRasterMinMaxOrigin::MinMax );
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mpLandsatRasterLayer );
  mMapSettings->setDestinationCrs( mpLandsatRasterLayer->crs() );
  mMapSettings->setExtent( mpLandsatRasterLayer->extent() );
  QVERIFY( render( "landsat_basic" ) );
}

void TestQgsRasterLayer::landsatBasic875Qml()
{
  QVERIFY2( mpLandsatRasterLayer->isValid(), "landsat.tif layer is not valid!" );
  //a qml that orders the rgb bands as 8,7,5
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mpLandsatRasterLayer );
  mMapSettings->setExtent( mpLandsatRasterLayer->extent() );
  QString msg;
  bool result = setQml( QStringLiteral( "875" ), msg );
  QVERIFY2( result, msg.toLocal8Bit().constData() );
  QVERIFY( render( "landsat_875" ) );
}
void TestQgsRasterLayer::checkDimensions()
{
  mReport += QLatin1String( "<h2>Check Dimensions</h2>\n" );
  QVERIFY( mpRasterLayer->width() == 10 );
  QVERIFY( mpRasterLayer->height() == 10 );
  // regression check for ticket #832
  // note bandStatistics call is base 1
  // TODO: elementCount is not collected by GDAL, use other stats.
  //QVERIFY( mpRasterLayer->dataProvider()->bandStatistics( 1 ).elementCount == 100 );
  mReport += QLatin1String( "<p>Passed</p>" );
}
void TestQgsRasterLayer::checkStats()
{

  mReport += QLatin1String( "<h2>Check Stats</h2>\n" );
  QgsRasterBandStats myStatistics = mpRasterLayer->dataProvider()->bandStatistics( 1,
                                    QgsRasterBandStats::Min | QgsRasterBandStats::Max |
                                    QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev );
  QCOMPARE( mpRasterLayer->width(), 10 );
  QCOMPARE( mpRasterLayer->height(), 10 );
  //QCOMPARE( myStatistics.elementCount, 100 );
  QCOMPARE( myStatistics.minimumValue, 0.0 );
  QCOMPARE( myStatistics.maximumValue, 9.0 );
  QGSCOMPARENEAR( myStatistics.mean, 4.5, 4 * std::numeric_limits<double>::epsilon() );
  double stdDev = 2.87228132326901431;
  // TODO: verify why GDAL stdDev is so different from generic (2.88675)
  mReport += QStringLiteral( "stdDev = %1 expected = %2<br>\n" ).arg( myStatistics.stdDev ).arg( stdDev );
  QGSCOMPARENEAR( myStatistics.stdDev, stdDev, 0.00000000000001 );
  mReport += QLatin1String( "<p>Passed</p>" );

  // limited extent
  myStatistics = mpRasterLayer->dataProvider()->bandStatistics( 1,
                 QgsRasterBandStats::Min | QgsRasterBandStats::Max |
                 QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, QgsRectangle( 1535400, 5083280, 1535450, 5083320 ) );

  QCOMPARE( myStatistics.minimumValue, 2.0 );
  QCOMPARE( myStatistics.maximumValue, 7.0 );
  QGSCOMPARENEAR( myStatistics.mean, 4.5, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( myStatistics.stdDev, 1.507557, 0.00001 );

  // with sample size
  myStatistics = mpRasterLayer->dataProvider()->bandStatistics( 1,
                 QgsRasterBandStats::Min | QgsRasterBandStats::Max |
                 QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, QgsRectangle( 1535400, 5083280, 1535450, 5083320 ), 10 );
  QCOMPARE( myStatistics.minimumValue, 2.0 );
  QCOMPARE( myStatistics.maximumValue, 7.0 );
  QCOMPARE( myStatistics.elementCount, 12ULL );
  QGSCOMPARENEAR( myStatistics.mean, 4.5, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( myStatistics.stdDev, 2.153222, 0.00001 );

  // extremely limited extent - ~1 px size
  myStatistics = mpRasterLayer->dataProvider()->bandStatistics( 1,
                 QgsRasterBandStats::Min | QgsRasterBandStats::Max |
                 QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, QgsRectangle( 1535400, 5083280, 1535412, 5083288 ) );
  QCOMPARE( myStatistics.minimumValue, 2.0 );
  QCOMPARE( myStatistics.maximumValue, 3.0 );
  QGSCOMPARENEAR( myStatistics.mean, 2.600000, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( myStatistics.stdDev, 0.492366, 0.00001 );

  // extremely limited extent - ~1 px size - with sample size
  myStatistics = mpRasterLayer->dataProvider()->bandStatistics( 1,
                 QgsRasterBandStats::Min | QgsRasterBandStats::Max |
                 QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, QgsRectangle( 1535400, 5083280, 1535412, 5083288 ), 6 );
  QCOMPARE( myStatistics.minimumValue, 2.0 );
  QCOMPARE( myStatistics.maximumValue, 3.0 );
  QCOMPARE( myStatistics.elementCount, 2ULL );
  QGSCOMPARENEAR( myStatistics.mean, 2.500000, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( myStatistics.stdDev, 0.707107, 0.00001 );
}

// test scale_factor and offset - uses netcdf file which may not be supported
// see https://issues.qgis.org/issues/8417
void TestQgsRasterLayer::checkScaleOffset()
{
  mReport += QLatin1String( "<h2>Check Stats with scale/offset</h2>\n" );

  QFileInfo myRasterFileInfo( mTestDataDir + "scaleoffset.tif" );
  QgsRasterLayer *myRasterLayer = nullptr;
  myRasterLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
                                      myRasterFileInfo.completeBaseName() );
  QVERIFY( myRasterLayer );
  if ( ! myRasterLayer->isValid() )
  {
    qDebug() << QStringLiteral( "raster layer %1 invalid" ).arg( myRasterFileInfo.filePath() );
    mReport += QStringLiteral( "raster layer %1 invalid" ).arg( myRasterFileInfo.filePath() );
    delete myRasterLayer;
    QVERIFY( false );
    return;
  }

  QFile::remove( myRasterFileInfo.filePath() + ".aux.xml" ); // remove cached stats
  QgsRasterBandStats myStatistics = myRasterLayer->dataProvider()->bandStatistics( 1,
                                    QgsRasterBandStats::Min | QgsRasterBandStats::Max |
                                    QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev );
  mReport += QStringLiteral( "raster min: %1 max: %2 mean: %3" ).arg( myStatistics.minimumValue ).arg( myStatistics.maximumValue ).arg( myStatistics.mean );
  QVERIFY( myRasterLayer->width() == 10 );
  QVERIFY( myRasterLayer->height() == 10 );
  //QVERIFY( myStatistics.elementCount == 100 );
  double minVal = 0.0;
  mReport += QStringLiteral( "min = %1 expected = %2 diff = %3<br>\n" ).arg( myStatistics.minimumValue ).arg( minVal ).arg( std::fabs( myStatistics.minimumValue - minVal ) );
  double maxVal = 9.0;
  mReport += QStringLiteral( "max = %1 expected = %2 diff = %3<br>\n" ).arg( myStatistics.maximumValue ).arg( maxVal ).arg( std::fabs( myStatistics.maximumValue - maxVal ) );
  double meanVal = 4.5;
  mReport += QStringLiteral( "min = %1 expected = %2 diff = %3<br>\n" ).arg( myStatistics.mean ).arg( meanVal ).arg( std::fabs( myStatistics.mean - meanVal ) );
  QVERIFY( std::fabs( myStatistics.minimumValue - minVal ) < 0.0000001 );
  QVERIFY( std::fabs( myStatistics.maximumValue - maxVal ) < 0.0000001 );
  QVERIFY( std::fabs( myStatistics.mean - meanVal ) < 0.0000001 );

  double stdDev = 2.87228615;
  // TODO: verify why GDAL stdDev is so different from generic (2.88675)
  mReport += QStringLiteral( "stdDev = %1 expected = %2 diff = %3<br>\n" ).arg( myStatistics.stdDev ).arg( stdDev ).arg( std::fabs( myStatistics.stdDev - stdDev ) );
  QVERIFY( std::fabs( myStatistics.stdDev - stdDev ) < 0.0000001 );

  QgsRasterDataProvider *myProvider = myRasterLayer->dataProvider();
  QgsPointXY myPoint( 1535030, 5083350 );
  QgsRectangle myRect( 1535030 - 5, 5083350 - 5, 1535030 + 5, 5083350 + 5 );
  QgsRasterIdentifyResult identifyResult = myProvider->identify( myPoint, QgsRaster::IdentifyFormatValue, myRect, 1, 1 );

  if ( identifyResult.isValid() )
  {
    QMap<int, QVariant> values = identifyResult.results();
    Q_FOREACH ( int bandNo, values.keys() )
    {
      QString valueString;
      if ( values.value( bandNo ).isNull() )
      {
        valueString = tr( "no data" );
        mReport += QStringLiteral( " %1 = %2 <br>\n" ).arg( myProvider->generateBandName( bandNo ), valueString );
        delete myRasterLayer;
        QVERIFY( false );
        return;
      }
      else
      {
        double expected = 0.99995432;
        double value = values.value( bandNo ).toDouble();
        valueString = QgsRasterBlock::printValue( value );
        mReport += QStringLiteral( " %1 = %2 <br>\n" ).arg( myProvider->generateBandName( bandNo ), valueString );
        mReport += QStringLiteral( " value = %1 expected = %2 diff = %3 <br>\n" ).arg( value ).arg( expected ).arg( std::fabs( value - expected ) );
        QVERIFY( std::fabs( value - expected ) < 0.0000001 );
      }
    }
  }
  else
  {
    delete myRasterLayer;
    QVERIFY( false );
    return;
  }

  mReport += QLatin1String( "<p>Passed</p>" );
  delete myRasterLayer;
}

void TestQgsRasterLayer::buildExternalOverviews()
{
  //before we begin delete any old ovr file (if it exists)
  //and make a copy of the landsat raster into the temp dir
  QString myTempPath = QDir::tempPath() + '/';
  QFile::remove( myTempPath + "landsat.tif.ovr" );
  QFile::remove( myTempPath + "landsat.tif" );
  QVERIFY( QFile::copy( mTestDataDir + "landsat.tif", myTempPath + "landsat.tif" ) );
  QFileInfo myRasterFileInfo( myTempPath + "landsat.tif" );
  QgsRasterLayer *mypLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
      myRasterFileInfo.completeBaseName() );

  QVERIFY( mypLayer->isValid() );

  //
  // OK now we can go on to test
  //

  QgsRaster::RasterPyramidsFormat myFormatFlag = QgsRaster::PyramidsGTiff;
  QList< QgsRasterPyramid > myPyramidList = mypLayer->dataProvider()->buildPyramidList();
  for ( int myCounterInt = 0; myCounterInt < myPyramidList.count(); myCounterInt++ )
  {
    //mark to be pyramided
    myPyramidList[myCounterInt].build = true;
  }
  //now actually make the pyramids
  QString myResult =
    mypLayer->dataProvider()->buildPyramids( myPyramidList, QStringLiteral( "NEAREST" ), myFormatFlag );
  qDebug( "%s", myResult.toLocal8Bit().constData() );
  QVERIFY( myResult.isEmpty() );
  //
  // Lets verify we have pyramids now...
  //
  myPyramidList = mypLayer->dataProvider()->buildPyramidList();
  for ( int myCounterInt = 0; myCounterInt < myPyramidList.count(); myCounterInt++ )
  {
    //mark to be pyramided
    QVERIFY( myPyramidList.at( myCounterInt ).exists );
  }

  //
  // And that they were indeed in an external file...
  //
  QVERIFY( QFile::exists( myTempPath + "landsat.tif.ovr" ) );

  //cleanup
  delete mypLayer;

  QFile::remove( myTempPath + "landsat.tif.ovr" );
  mypLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
                                 myRasterFileInfo.completeBaseName() );
  myPyramidList = mypLayer->dataProvider()->buildPyramidList();
  for ( int myCounterInt = 0; myCounterInt < myPyramidList.count(); myCounterInt++ )
  {
    //mark to be pyramided
    myPyramidList[myCounterInt].build = true;
  }

  // Test with options
  QStringList optionList;
  optionList << QStringLiteral( "COMPRESS_OVERVIEW=DEFLATE" );
  optionList << QStringLiteral( "invalid" );

  myResult =
    mypLayer->dataProvider()->buildPyramids( myPyramidList, QStringLiteral( "NEAREST" ), myFormatFlag, optionList );
  qDebug( "%s", myResult.toLocal8Bit().constData() );
  QVERIFY( myResult.isEmpty() );
  QVERIFY( QFile::exists( myTempPath + "landsat.tif.ovr" ) );

  //cleanup
  delete mypLayer;

  // Check that the overview is Deflate compressed
  QString ovrFilename( myTempPath + "landsat.tif.ovr" );
  GDALDatasetH hDS = GDALOpen( ovrFilename.toLocal8Bit().constData(), GA_ReadOnly );
  QVERIFY( hDS );
  const char *pszCompression = GDALGetMetadataItem( hDS, "COMPRESSION", "IMAGE_STRUCTURE" );
  QVERIFY( pszCompression && EQUAL( pszCompression, "DEFLATE" ) );
  GDALClose( hDS );

  mReport += QLatin1String( "<h2>Check Overviews</h2>\n" );
  mReport += QLatin1String( "<p>Passed</p>" );
}


void TestQgsRasterLayer::registry()
{
  QString myTempPath = QDir::tempPath() + '/';
  QFile::remove( myTempPath + "landsat.tif.ovr" );
  QFile::remove( myTempPath + "landsat.tif" );
  QVERIFY( QFile::copy( mTestDataDir + "landsat.tif", myTempPath + "landsat.tif" ) );
  QFileInfo myRasterFileInfo( myTempPath + "landsat.tif" );
  QgsRasterLayer *mypLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
      myRasterFileInfo.completeBaseName() );
  QVERIFY( mypLayer->isValid() );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mypLayer, false );
  QgsProject::instance()->removeMapLayers(
    QStringList() << mypLayer->id() );
}

//
// Helper methods
//


bool TestQgsRasterLayer::render( const QString &testType, int mismatchCount )
{
  mReport += "<h2>" + testType + "</h2>\n";
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  bool myResultFlag = myChecker.runTest( testType, mismatchCount );
  mReport += "\n\n\n" + myChecker.report();
  return myResultFlag;
}

bool TestQgsRasterLayer::setQml( const QString &type, QString &msg )
{
  //load a qml style and apply to our layer
  if ( !mpLandsatRasterLayer->isValid() )
  {
    msg = QStringLiteral( " **** setQml -> mpLandsatRasterLayer is invalid" );
    return false;
  }

  QString myFileName = mTestDataDir + "landsat_" + type + ".qml";
  bool myStyleFlag = false;
  QString loadStyleMsg;
  loadStyleMsg = mpLandsatRasterLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    msg = QStringLiteral( "Loading QML %1 failed: %2" ).arg( myFileName, loadStyleMsg );
    return false;
  }
  return true;
}

void TestQgsRasterLayer::transparency()
{

  QVERIFY( mpFloat32RasterLayer->isValid() );
  QgsSingleBandGrayRenderer *renderer = new QgsSingleBandGrayRenderer( mpRasterLayer->dataProvider(), 1 );
  mpFloat32RasterLayer->setRenderer( renderer );
  mpFloat32RasterLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, QgsRasterMinMaxOrigin::MinMax );

  qDebug( "contrastEnhancement.minimumValue = %.17g", renderer->contrastEnhancement()->minimumValue() );
  qDebug( "contrastEnhancement.maximumValue = %.17g", renderer->contrastEnhancement()->maximumValue() );

  QList<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList;
  QgsRasterTransparency *rasterTransparency = new QgsRasterTransparency();
  QgsRasterTransparency::TransparentSingleValuePixel myTransparentPixel;

  myTransparentPixel.min = -2.5840000772112106e+38;
  myTransparentPixel.max = -1.0879999684602689e+38;
  myTransparentPixel.percentTransparent = 50;
  myTransparentSingleValuePixelList.append( myTransparentPixel );

  myTransparentPixel.min = 1.359999960575336e+37;
  myTransparentPixel.max = 9.520000231087593e+37;
  myTransparentPixel.percentTransparent = 70;
  myTransparentSingleValuePixelList.append( myTransparentPixel );

  rasterTransparency->setTransparentSingleValuePixelList( myTransparentSingleValuePixelList );

  QgsRasterRenderer *rasterRenderer = mpFloat32RasterLayer->renderer();
  QVERIFY( rasterRenderer != nullptr );
  rasterRenderer->setRasterTransparency( rasterTransparency );

  mMapSettings->setLayers( QList<QgsMapLayer *>() << mpFloat32RasterLayer );
  mMapSettings->setDestinationCrs( mpFloat32RasterLayer->crs() );
  mMapSettings->setExtent( mpFloat32RasterLayer->extent() );
  QVERIFY( render( "raster_transparency" ) );
}

void TestQgsRasterLayer::multiBandColorRenderer()
{
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mPngRasterLayer->dataProvider(), 1, 2, 3 );
  mPngRasterLayer->setRenderer( rasterRenderer );
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mPngRasterLayer );
  mMapSettings->setDestinationCrs( mPngRasterLayer->crs() );
  mMapSettings->setExtent( mPngRasterLayer->extent() );
  QVERIFY( render( "raster_multibandrenderer" ) );
}

void TestQgsRasterLayer::setRenderer()
{
  TestSignalReceiver receiver;
  QObject::connect( mpRasterLayer, SIGNAL( rendererChanged() ),
                    &receiver, SLOT( onRendererChanged() ) );
  QgsRasterRenderer *renderer = ( QgsRasterRenderer * ) mpRasterLayer->renderer()->clone();
  QCOMPARE( receiver.rendererChanged, false );
  mpRasterLayer->setRenderer( renderer );
  QCOMPARE( receiver.rendererChanged, true );
  QCOMPARE( mpRasterLayer->renderer(), renderer );
}

void TestQgsRasterLayer::regression992()
{
  if ( ! mGeoJp2RasterLayer->isValid() )
  {
    QSKIP( "This test requires the JPEG2000 GDAL driver", SkipAll );
  }

  mMapSettings->setDestinationCrs( mGeoJp2RasterLayer->crs() );
  mMapSettings->setExtent( mGeoJp2RasterLayer->extent() );
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mGeoJp2RasterLayer );
  QVERIFY( render( "raster_geojp2", 400 ) );
}

void TestQgsRasterLayer::testRefreshRendererIfNeeded()
{
  QVERIFY2( mpLandsatRasterLayer->isValid(), "landsat.tif layer is not valid!" );
  mpLandsatRasterLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, QgsRasterMinMaxOrigin::MinMax );
  QVERIFY( dynamic_cast<QgsMultiBandColorRenderer *>( mpLandsatRasterLayer->renderer() ) );
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mpLandsatRasterLayer );
  mMapSettings->setExtent( mpLandsatRasterLayer->extent() );
  double initMinVal = static_cast<QgsMultiBandColorRenderer *>( mpLandsatRasterLayer->renderer() )->redContrastEnhancement()->minimumValue();

  // Should do nothing
  QgsRectangle newExtent = QgsRectangle( 785000, 3340000, 785100, 3340100 );
  mpLandsatRasterLayer->refreshRendererIfNeeded( mpLandsatRasterLayer->renderer(), newExtent );
  QCOMPARE( mpLandsatRasterLayer->renderer()->minMaxOrigin().limits(), QgsRasterMinMaxOrigin::MinMax );
  double minVal = static_cast<QgsMultiBandColorRenderer *>( mpLandsatRasterLayer->renderer() )->redContrastEnhancement()->minimumValue();
  QGSCOMPARENEAR( initMinVal, minVal, 1e-5 );

  // Change to UpdatedCanvas
  QgsRasterMinMaxOrigin mmo = mpLandsatRasterLayer->renderer()->minMaxOrigin();
  mmo.setExtent( QgsRasterMinMaxOrigin::UpdatedCanvas );
  mmo.setStatAccuracy( QgsRasterMinMaxOrigin::Exact );
  mpLandsatRasterLayer->renderer()->setMinMaxOrigin( mmo );
  QCOMPARE( mpLandsatRasterLayer->renderer()->minMaxOrigin().extent(), QgsRasterMinMaxOrigin::UpdatedCanvas );
  mpLandsatRasterLayer->refreshRendererIfNeeded( mpLandsatRasterLayer->renderer(), newExtent );
  double newMinVal = static_cast<QgsMultiBandColorRenderer *>( mpLandsatRasterLayer->renderer() )->redContrastEnhancement()->minimumValue();
  QGSCOMPARENOTNEAR( initMinVal, newMinVal, 1e-5 );
}

void TestQgsRasterLayer::sample()
{
  QString fileName = mTestDataDir + "landsat-f32-b1.tif";

  QFileInfo rasterFileInfo( fileName );
  std::unique_ptr< QgsRasterLayer > rl = qgis::make_unique< QgsRasterLayer> ( rasterFileInfo.filePath(),
                                         rasterFileInfo.completeBaseName() );
  QVERIFY( rl->isValid() );
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 0, 0 ), 1 ) ) );
  bool ok = false;
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 0, 0 ), 1, &ok ) ) );
  QVERIFY( !ok );
  QCOMPARE( rl->dataProvider()->sample( QgsPointXY( 788461, 3344957 ), 1 ), 125.0 );
  QCOMPARE( rl->dataProvider()->sample( QgsPointXY( 788461, 3344957 ), 1, &ok ), 125.0 );
  QVERIFY( ok );
  // bad bands
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 788461, 3344957 ), 0 ) ) );
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 788461, 3344957 ), 0, &ok ) ) );
  QVERIFY( !ok );
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 788461, 3344957 ), 10, &ok ) ) );
  QVERIFY( !ok );

  fileName = mTestDataDir + "landsat_4326.tif";
  rasterFileInfo = QFileInfo( fileName );
  rl = qgis::make_unique< QgsRasterLayer> ( rasterFileInfo.filePath(),
       rasterFileInfo.completeBaseName() );
  QVERIFY( rl->isValid() );
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 0, 0 ), 1 ) ) );
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 0, 0 ), 1, &ok ) ) );
  QVERIFY( !ok );
  QCOMPARE( rl->dataProvider()->sample( QgsPointXY( 17.943731, 30.230791 ), 1, &ok ), 125.0 );
  QVERIFY( ok );
  QCOMPARE( rl->dataProvider()->sample( QgsPointXY( 17.943731, 30.230791 ), 2 ), 139.0 );
  QCOMPARE( rl->dataProvider()->sample( QgsPointXY( 17.943731, 30.230791 ), 3 ), 111.0 );

  // src no data
  rl->dataProvider()->setNoDataValue( 3, 111.0 );
  ok = false;
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 17.943731, 30.230791 ), 3, &ok ) ) );
  QVERIFY( !ok );
  rl->dataProvider()->setUseSourceNoDataValue( 3, false );
  QCOMPARE( rl->dataProvider()->sample( QgsPointXY( 17.943731, 30.230791 ), 3 ), 111.0 );

  rl->dataProvider()->setUserNoDataValue( 2, QgsRasterRangeList() << QgsRasterRange( 130, 140 ) );
  QVERIFY( std::isnan( rl->dataProvider()->sample( QgsPointXY( 17.943731, 30.230791 ), 2, &ok ) ) );
  QVERIFY( !ok );
}

QGSTEST_MAIN( TestQgsRasterLayer )
#include "testqgsrasterlayer.moc"
