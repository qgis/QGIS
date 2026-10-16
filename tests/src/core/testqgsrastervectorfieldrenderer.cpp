/***************************************************************************
  testqgsrastervectorfieldrenderer.cpp
  ------------------------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by Stefanos Natsis
  Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include <QDomDocument>
#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

//qgis includes...
#include "qgsapplication.h"
#include "qgscolorrampshader.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrasterblock.h"
#include "qgsrastervectorfieldrenderer.h"
#include "qgsrastervectorfieldvaluesource.h"
#include "qgsstyle.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the raster vector field renderer
 */
class TestQgsRasterVectorFieldRenderer : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterVectorFieldRenderer()
      : QgsTest( u"Raster Vector Field Renderer Tests"_s )
    {}

  private:
    QgsRasterLayer *mLayer = nullptr;
    QgsRasterLayer *mDirectionLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;
    QgsMapSettings *mDirectionMapSettings = nullptr;

    //! Returns a renderer over the test layer, with the symbology and the magnitude range shared by the render checks
    QgsRasterVectorFieldRenderer *createRenderer( Qgis::VectorFieldSymbology symbology );

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testBands();
    void testClone();
    void testXmlRoundTrip();
    void testRegistered();

    void testDirectionEncodings();
    void testDirectionOrdinalVectors();
    void testMajorityAggregation();
    void testEncodedBands();
    void testEncodedXmlRoundTrip();

    void testRenderArrows();
    void testRenderArrowsColorRamp();
    void testRenderArrowsOnPixels();
    void testRenderWindBarbs();
    void testRenderTraces();
    void testRenderEncodedArrows();
    void testRenderEncodedStreamlinesAndTraces();
};

void TestQgsRasterVectorFieldRenderer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  const QString dataDir = QString( TEST_DATA_DIR ) + "/raster";
  mLayer = new QgsRasterLayer( dataDir + "/vector_field_uv.tif", u"vector field"_s, u"gdal"_s );
  QVERIFY( mLayer->isValid() );
  QCOMPARE( mLayer->bandCount(), 2 );

  QgsProject::instance()->addMapLayer( mLayer );

  mMapSettings = new QgsMapSettings();
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLayer );
  mMapSettings->setExtent( mLayer->extent() );
  mMapSettings->setDestinationCrs( mLayer->crs() );
  mMapSettings->setOutputSize( QSize( 400, 400 ) );
  mMapSettings->setOutputDpi( 96 );

  mDirectionLayer = new QgsRasterLayer( dataDir + "/flow_direction_d8.tif", u"flow direction"_s, u"gdal"_s );
  QVERIFY( mDirectionLayer->isValid() );
  QCOMPARE( mDirectionLayer->bandCount(), 1 );

  QgsProject::instance()->addMapLayer( mDirectionLayer );

  mDirectionMapSettings = new QgsMapSettings();
  mDirectionMapSettings->setLayers( QList<QgsMapLayer *>() << mDirectionLayer );
  mDirectionMapSettings->setExtent( mDirectionLayer->extent() );
  mDirectionMapSettings->setDestinationCrs( mDirectionLayer->crs() );
  mDirectionMapSettings->setOutputSize( QSize( 400, 400 ) );
  mDirectionMapSettings->setOutputDpi( 96 );
}

void TestQgsRasterVectorFieldRenderer::cleanupTestCase()
{
  delete mMapSettings;
  delete mDirectionMapSettings;
  QgsApplication::exitQgis();
}

QgsRasterVectorFieldRenderer *TestQgsRasterVectorFieldRenderer::createRenderer( Qgis::VectorFieldSymbology symbology )
{
  QgsRasterVectorFieldRenderer *renderer = new QgsRasterVectorFieldRenderer( mLayer->dataProvider() );
  renderer->setXBand( 1 );
  renderer->setYBand( 2 );

  // the field of the test raster is a vortex, whose magnitude grows away from the center
  renderer->setMinimumMagnitude( 0 );
  renderer->setMaximumMagnitude( 14 );

  QgsVectorFieldSettings settings = renderer->settings();
  settings.setSymbology( symbology );
  // one glyph per pixel is the default, the render tests draw on a grid coarse enough to read
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 40 );
  settings.setUserGridCellHeight( 40 );
  renderer->setSettings( settings );

  return renderer;
}

void TestQgsRasterVectorFieldRenderer::testBands()
{
  QgsRasterVectorFieldRenderer renderer( mLayer->dataProvider() );

  // the renderer reads two bands, so it has no single input band
  QCOMPARE( renderer.inputBand(), -1 );
  QVERIFY( !renderer.setInputBand( 1 ) );

  QCOMPARE( renderer.xBand(), 1 );
  QCOMPARE( renderer.yBand(), 2 );
  QCOMPARE( renderer.usesBands(), QList<int>() << 1 << 2 );

  QVERIFY( renderer.setXBand( 2 ) );
  QVERIFY( renderer.setYBand( 1 ) );
  QCOMPARE( renderer.usesBands(), QList<int>() << 2 << 1 );

  // the test raster only has two bands
  QVERIFY( !renderer.setXBand( 3 ) );
  QVERIFY( !renderer.setXBand( 0 ) );
  QCOMPARE( renderer.xBand(), 2 );
}

void TestQgsRasterVectorFieldRenderer::testClone()
{
  QgsRasterVectorFieldRenderer renderer( mLayer->dataProvider() );
  renderer.setXBand( 2 );
  renderer.setYBand( 1 );
  renderer.setMinimumMagnitude( 1.5 );
  renderer.setMaximumMagnitude( 12.5 );
  renderer.setOpacity( 0.25 );

  QgsVectorFieldSettings settings = renderer.settings();
  settings.setSymbology( Qgis::VectorFieldSymbology::WindBarbs );
  settings.setColor( QColor( 255, 0, 0 ) );
  settings.setLineWidth( 1.25 );
  renderer.setSettings( settings );

  std::unique_ptr<QgsRasterVectorFieldRenderer> cloned( renderer.clone() );
  QCOMPARE( cloned->xBand(), 2 );
  QCOMPARE( cloned->yBand(), 1 );
  QCOMPARE( cloned->minimumMagnitude(), 1.5 );
  QCOMPARE( cloned->maximumMagnitude(), 12.5 );
  QCOMPARE( cloned->opacity(), 0.25 );
  QCOMPARE( cloned->settings().symbology(), Qgis::VectorFieldSymbology::WindBarbs );
  QCOMPARE( cloned->settings().color(), QColor( 255, 0, 0 ) );
  QCOMPARE( cloned->settings().lineWidth(), 1.25 );
}

void TestQgsRasterVectorFieldRenderer::testXmlRoundTrip()
{
  QgsRasterVectorFieldRenderer renderer( mLayer->dataProvider() );
  renderer.setXBand( 2 );
  renderer.setYBand( 1 );
  renderer.setMinimumMagnitude( 0.5 );
  renderer.setMaximumMagnitude( 9.5 );

  QgsVectorFieldSettings settings = renderer.settings();
  settings.setSymbology( Qgis::VectorFieldSymbology::Streamlines );
  settings.setColor( QColor( 0, 128, 255 ) );
  settings.setLineWidth( 0.75 );
  settings.setFilterMin( 2 );
  settings.setFilterMax( 8 );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 25 );
  settings.setUserGridCellHeight( 35 );

  QgsVectorFieldStreamlineSettings streamlineSettings = settings.streamLinesSettings();
  streamlineSettings.setSeedingMethod( Qgis::VectorFieldSeedingMethod::Random );
  streamlineSettings.setSeedingDensity( 0.25 );
  settings.setStreamLinesSettings( streamlineSettings );

  QgsVectorFieldArrowSettings arrowSettings = settings.arrowSettings();
  arrowSettings.setShaftLengthMethod( Qgis::VectorFieldArrowScalingMethod::Fixed );
  arrowSettings.setFixedShaftLength( 13 );
  settings.setArrowsSettings( arrowSettings );

  renderer.setSettings( settings );

  QDomDocument doc( u"style"_s );
  QDomElement rootElem = doc.createElement( u"qgis"_s );
  doc.appendChild( rootElem );
  renderer.writeXml( doc, rootElem );

  const QDomElement rendererElem = rootElem.firstChildElement( u"rasterrenderer"_s );
  QVERIFY( !rendererElem.isNull() );
  QCOMPARE( rendererElem.attribute( u"type"_s ), u"vectorfield"_s );

  std::unique_ptr<QgsRasterRenderer> restored( QgsRasterVectorFieldRenderer::create( rendererElem, mLayer->dataProvider() ) );
  QgsRasterVectorFieldRenderer *restoredVectorField = dynamic_cast<QgsRasterVectorFieldRenderer *>( restored.get() );
  QVERIFY( restoredVectorField );

  QCOMPARE( restoredVectorField->xBand(), 2 );
  QCOMPARE( restoredVectorField->yBand(), 1 );
  QCOMPARE( restoredVectorField->minimumMagnitude(), 0.5 );
  QCOMPARE( restoredVectorField->maximumMagnitude(), 9.5 );

  const QgsVectorFieldSettings restoredSettings = restoredVectorField->settings();
  QCOMPARE( restoredSettings.symbology(), Qgis::VectorFieldSymbology::Streamlines );
  QCOMPARE( restoredSettings.color(), QColor( 0, 128, 255 ) );
  QCOMPARE( restoredSettings.lineWidth(), 0.75 );
  QCOMPARE( restoredSettings.filterMin(), 2.0 );
  QCOMPARE( restoredSettings.filterMax(), 8.0 );
  QVERIFY( restoredSettings.isOnUserDefinedGrid() );
  QCOMPARE( restoredSettings.userGridCellWidth(), 25 );
  QCOMPARE( restoredSettings.userGridCellHeight(), 35 );
  QCOMPARE( restoredSettings.streamLinesSettings().seedingMethod(), Qgis::VectorFieldSeedingMethod::Random );
  QCOMPARE( restoredSettings.streamLinesSettings().seedingDensity(), 0.25 );
  QCOMPARE( restoredSettings.arrowSettings().shaftLengthMethod(), Qgis::VectorFieldArrowScalingMethod::Fixed );
  QCOMPARE( restoredSettings.arrowSettings().fixedShaftLength(), 13.0 );

  // an unset magnitude range is not written, so that it stays calculated from the data
  QgsRasterVectorFieldRenderer automatic( mLayer->dataProvider() );
  QVERIFY( std::isnan( automatic.minimumMagnitude() ) );
  QVERIFY( std::isnan( automatic.maximumMagnitude() ) );

  QDomDocument automaticDoc( u"style"_s );
  QDomElement automaticRoot = automaticDoc.createElement( u"qgis"_s );
  automaticDoc.appendChild( automaticRoot );
  automatic.writeXml( automaticDoc, automaticRoot );

  std::unique_ptr<QgsRasterRenderer> restoredAutomatic( QgsRasterVectorFieldRenderer::create( automaticRoot.firstChildElement( u"rasterrenderer"_s ), mLayer->dataProvider() ) );
  QgsRasterVectorFieldRenderer *restoredAutomaticVectorField = dynamic_cast<QgsRasterVectorFieldRenderer *>( restoredAutomatic.get() );
  QVERIFY( restoredAutomaticVectorField );
  QVERIFY( std::isnan( restoredAutomaticVectorField->minimumMagnitude() ) );
  QVERIFY( std::isnan( restoredAutomaticVectorField->maximumMagnitude() ) );
}

void TestQgsRasterVectorFieldRenderer::testRegistered()
{
  QgsRasterRendererRegistryEntry entry;
  QVERIFY( QgsApplication::rasterRendererRegistry()->rendererData( u"vectorfield"_s, entry ) );
  QVERIFY( entry.rendererCreateFunction );
  QVERIFY( entry.capabilities & Qgis::RasterRendererCapability::UsesMultipleBands );
}

void TestQgsRasterVectorFieldRenderer::testDirectionEncodings()
{
  // the ordinals run clockwise from north, 0 being north and 7 north west. These tables are the
  // whole correctness surface of the encoded directions, so every code of every scheme is asserted
  const auto ordinal = []( Qgis::RasterDirectionEncoding encoding, double value ) { return QgsRasterVectorFieldEncodedDirectionValueSource::directionOrdinal( encoding, value ); };

  // Esri, powers of two clockwise from east
  constexpr Qgis::RasterDirectionEncoding esri = Qgis::RasterDirectionEncoding::Esri;
  QCOMPARE( ordinal( esri, 64 ), 0 );  // N
  QCOMPARE( ordinal( esri, 128 ), 1 ); // NE
  QCOMPARE( ordinal( esri, 1 ), 2 );   // E
  QCOMPARE( ordinal( esri, 2 ), 3 );   // SE
  QCOMPARE( ordinal( esri, 4 ), 4 );   // S
  QCOMPARE( ordinal( esri, 8 ), 5 );   // SW
  QCOMPARE( ordinal( esri, 16 ), 6 );  // W
  QCOMPARE( ordinal( esri, 32 ), 7 );  // NW

  QCOMPARE( ordinal( esri, 0 ), -1 );
  QCOMPARE( ordinal( esri, 3 ), -1 );
  QCOMPARE( ordinal( esri, 255 ), -1 );
  QCOMPARE( ordinal( esri, -1 ), -1 );

  // GRASS, counter clockwise from north east
  constexpr Qgis::RasterDirectionEncoding grass = Qgis::RasterDirectionEncoding::Grass;
  QCOMPARE( ordinal( grass, 2 ), 0 ); // N
  QCOMPARE( ordinal( grass, 1 ), 1 ); // NE
  QCOMPARE( ordinal( grass, 8 ), 2 ); // E
  QCOMPARE( ordinal( grass, 7 ), 3 ); // SE
  QCOMPARE( ordinal( grass, 6 ), 4 ); // S
  QCOMPARE( ordinal( grass, 5 ), 5 ); // SW
  QCOMPARE( ordinal( grass, 4 ), 6 ); // W
  QCOMPARE( ordinal( grass, 3 ), 7 ); // NW
  // a negative value leaves the region in the direction of its absolute value
  QCOMPARE( ordinal( grass, -2 ), 0 );
  QCOMPARE( ordinal( grass, -1 ), 1 );
  QCOMPARE( ordinal( grass, -8 ), 2 );
  QCOMPARE( ordinal( grass, -3 ), 7 );
  // zero is a depression
  QCOMPARE( ordinal( grass, 0 ), -1 );
  QCOMPARE( ordinal( grass, 9 ), -1 );

  // SAGA, clockwise from north, which is the ordinal itself
  constexpr Qgis::RasterDirectionEncoding saga = Qgis::RasterDirectionEncoding::Saga;
  for ( int code = 0; code <= 7; ++code )
    QCOMPARE( ordinal( saga, code ), code );
  QCOMPARE( ordinal( saga, -1 ), -1 );
  QCOMPARE( ordinal( saga, 8 ), -1 );

  // PCRaster, the numeric keypad with 5 in the middle
  constexpr Qgis::RasterDirectionEncoding ldd = Qgis::RasterDirectionEncoding::PcRaster;
  QCOMPARE( ordinal( ldd, 8 ), 0 ); // N
  QCOMPARE( ordinal( ldd, 9 ), 1 ); // NE
  QCOMPARE( ordinal( ldd, 6 ), 2 ); // E
  QCOMPARE( ordinal( ldd, 3 ), 3 ); // SE
  QCOMPARE( ordinal( ldd, 2 ), 4 ); // S
  QCOMPARE( ordinal( ldd, 1 ), 5 ); // SW
  QCOMPARE( ordinal( ldd, 4 ), 6 ); // W
  QCOMPARE( ordinal( ldd, 7 ), 7 ); // NW
  // five is a pit, a cell without a local drain direction
  QCOMPARE( ordinal( ldd, 5 ), -1 );
  QCOMPARE( ordinal( ldd, 0 ), -1 );
  QCOMPARE( ordinal( ldd, 10 ), -1 );

  // the codes are classes, a value between two of them names no direction
  QCOMPARE( ordinal( saga, 2.5 ), -1 );
  QCOMPARE( ordinal( esri, std::numeric_limits<double>::quiet_NaN() ), -1 );
}

void TestQgsRasterVectorFieldRenderer::testDirectionOrdinalVectors()
{
  // a positive y points north, the engine flips the axis for the painter itself
  const QgsVector north = QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 0 );
  QGSCOMPARENEAR( north.x(), 0.0, 0.0001 );
  QGSCOMPARENEAR( north.y(), 1.0, 0.0001 );

  const QgsVector east = QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 2 );
  QGSCOMPARENEAR( east.x(), 1.0, 0.0001 );
  QGSCOMPARENEAR( east.y(), 0.0, 0.0001 );

  const QgsVector southWest = QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 5 );
  QGSCOMPARENEAR( southWest.x(), -M_SQRT1_2, 0.0001 );
  QGSCOMPARENEAR( southWest.y(), -M_SQRT1_2, 0.0001 );

  // every direction is a unit vector, the encodings carry no magnitude
  for ( int i = 0; i < 8; ++i )
    QGSCOMPARENEAR( QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( i ).length(), 1.0, 0.0001 );

  // an ordinal outside of the compass has no vector at all
  QVERIFY( std::isnan( QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( -1 ).x() ) );
  QVERIFY( std::isnan( QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 8 ).x() ) );
}

void TestQgsRasterVectorFieldRenderer::testMajorityAggregation()
{
  // a 4 by 4 block of Esri codes covering a unit square, so a cell is 0.25 wide
  const QgsRectangle blockExtent( 0, 0, 1, 1 );
  const auto makeSource = [&blockExtent]( const QVector<int> &codes ) {
    auto block = std::make_shared<QgsRasterBlock>( Qgis::DataType::Float64, 4, 4 );
    for ( int row = 0; row < 4; ++row )
      for ( int column = 0; column < 4; ++column )
        block->setValue( row, column, codes.at( row * 4 + column ) );

    return std::make_unique<QgsRasterVectorFieldEncodedDirectionValueSource>( block, Qgis::RasterDirectionEncoding::Esri, blockExtent, blockExtent );
  };

  // ten cells point north east and six point north, so the whole block is north east
  // clang-format off
  const QVector<int> mixed = {
    128, 128, 128, 128,
    128, 128, 128, 128,
    128, 128,  64,  64,
     64,  64,  64,  64
  };
  // clang-format on

  std::unique_ptr<QgsRasterVectorFieldEncodedDirectionValueSource> source = makeSource( mixed );

  // without a window a sample reads the cell it falls in, whatever its neighbours hold
  QCOMPARE( source->vectorValue( QgsPointXY( 0.125, 0.125 ) ), QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 0 ) ); // bottom left, N
  QCOMPARE( source->vectorValue( QgsPointXY( 0.125, 0.875 ) ), QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 1 ) ); // top left, NE

  // a window covering the whole block returns the direction most of it holds
  source->setSamplingWindow( 1, 1 );
  QCOMPARE( source->vectorValue( QgsPointXY( 0.5, 0.5 ) ), QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 1 ) );

  // the codes are never averaged: north east and north would otherwise combine into a code of
  // their own, and a direction which is in neither the data nor the encoding
  // clang-format off
  const QVector<int> opposed = {
    1, 1, 16, 16,
    1, 1, 16, 16,
    1, 1, 16, 16,
    1, 1, 16, 16
  };
  // clang-format on
  source = makeSource( opposed );
  source->setSamplingWindow( 1, 1 );

  // east and west are equally common, and their resultant cancels out, so the lowest ordinal wins.
  // Either way the answer is a direction which is actually in the data
  const QgsVector tied = source->vectorValue( QgsPointXY( 0.5, 0.5 ) );
  QVERIFY( tied == QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 2 ) || tied == QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 6 ) );

  // a tie is broken towards the resultant of the whole window: seven cells point east, seven west,
  // and the two remaining ones lean north, so north east is the more representative of the two
  // clang-format off
  const QVector<int> leaning = {
    1,  1, 16, 16,
    1,  1, 16, 16,
    1,  1, 16, 16,
    1, 64, 64, 16
  };
  // clang-format on
  source = makeSource( leaning );
  source->setSamplingWindow( 1, 1 );
  QCOMPARE( source->vectorValue( QgsPointXY( 0.5, 0.5 ) ), QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( 2 ) );

  // a window holding nothing but sinks and undecided cells has no direction at all
  // clang-format off
  const QVector<int> sinks = {
    0, 0, 0, 0,
    0, 3, 3, 0,
    0, 3, 3, 0,
    0, 0, 0, 0
  };
  // clang-format on
  source = makeSource( sinks );
  source->setSamplingWindow( 1, 1 );
  QVERIFY( std::isnan( source->vectorValue( QgsPointXY( 0.5, 0.5 ) ).x() ) );

  // and so does a point outside of the block
  QVERIFY( std::isnan( source->vectorValue( QgsPointXY( 5, 5 ) ).x() ) );
}

void TestQgsRasterVectorFieldRenderer::testEncodedBands()
{
  QgsRasterVectorFieldRenderer renderer( mDirectionLayer->dataProvider() );

  // the component bands are the default, so that a project written before the encodings reads back
  // exactly as it was saved
  QCOMPARE( renderer.sourceMode(), Qgis::RasterVectorFieldSourceMode::CartesianComponents );
  QCOMPARE( renderer.directionBand(), 1 );
  QCOMPARE( renderer.directionEncoding(), Qgis::RasterDirectionEncoding::Esri );

  renderer.setSourceMode( Qgis::RasterVectorFieldSourceMode::EncodedDirection );

  // only the direction band is read in this mode, whatever the component bands are set to
  QCOMPARE( renderer.usesBands(), QList<int>() << 1 );

  // the test raster only has one band
  QVERIFY( !renderer.setDirectionBand( 2 ) );
  QVERIFY( !renderer.setDirectionBand( 0 ) );
  QCOMPARE( renderer.directionBand(), 1 );

  // the bands still depend on the mode, so there is no single input band to offer
  QCOMPARE( renderer.inputBand(), -1 );
  QVERIFY( !renderer.setInputBand( 1 ) );
}

void TestQgsRasterVectorFieldRenderer::testEncodedXmlRoundTrip()
{
  QgsRasterVectorFieldRenderer renderer( mDirectionLayer->dataProvider() );
  renderer.setSourceMode( Qgis::RasterVectorFieldSourceMode::EncodedDirection );
  renderer.setDirectionBand( 1 );
  renderer.setDirectionEncoding( Qgis::RasterDirectionEncoding::PcRaster );

  QDomDocument doc( u"style"_s );
  QDomElement rootElem = doc.createElement( u"qgis"_s );
  doc.appendChild( rootElem );
  renderer.writeXml( doc, rootElem );

  const QDomElement rendererElem = rootElem.firstChildElement( u"rasterrenderer"_s );
  QVERIFY( !rendererElem.isNull() );

  std::unique_ptr<QgsRasterRenderer> restored( QgsRasterVectorFieldRenderer::create( rendererElem, mDirectionLayer->dataProvider() ) );
  QgsRasterVectorFieldRenderer *restoredVectorField = dynamic_cast<QgsRasterVectorFieldRenderer *>( restored.get() );
  QVERIFY( restoredVectorField );
  QCOMPARE( restoredVectorField->sourceMode(), Qgis::RasterVectorFieldSourceMode::EncodedDirection );
  QCOMPARE( restoredVectorField->directionBand(), 1 );
  QCOMPARE( restoredVectorField->directionEncoding(), Qgis::RasterDirectionEncoding::PcRaster );

  std::unique_ptr<QgsRasterVectorFieldRenderer> cloned( restoredVectorField->clone() );
  QCOMPARE( cloned->sourceMode(), Qgis::RasterVectorFieldSourceMode::EncodedDirection );
  QCOMPARE( cloned->directionBand(), 1 );
  QCOMPARE( cloned->directionEncoding(), Qgis::RasterDirectionEncoding::PcRaster );
}

void TestQgsRasterVectorFieldRenderer::testRenderArrows()
{
  mLayer->setRenderer( createRenderer( Qgis::VectorFieldSymbology::Arrows ) );

  QGSVERIFYRENDERMAPSETTINGSCHECK( u"raster_vector_field_arrows"_s, u"raster_vector_field_arrows"_s, *mMapSettings, 0, 15 );
}

void TestQgsRasterVectorFieldRenderer::testRenderArrowsColorRamp()
{
  QgsRasterVectorFieldRenderer *renderer = createRenderer( Qgis::VectorFieldSymbology::Arrows );

  QgsVectorFieldSettings settings = renderer->settings();
  settings.setColoringMethod( QgsInterpolatedLineColor::ColorRamp );
  QgsColorRampShader shader( 0, 14, QgsStyle::defaultStyle()->colorRamp( u"Viridis"_s ).release() );
  shader.classifyColorRamp( 5, -1 );
  settings.setColorRampShader( shader );
  renderer->setSettings( settings );

  mLayer->setRenderer( renderer );

  QGSVERIFYRENDERMAPSETTINGSCHECK( u"raster_vector_field_arrows_color_ramp"_s, u"raster_vector_field_arrows_color_ramp"_s, *mMapSettings, 0, 15 );
}

void TestQgsRasterVectorFieldRenderer::testRenderArrowsOnPixels()
{
  QgsRasterVectorFieldRenderer *renderer = createRenderer( Qgis::VectorFieldSymbology::Arrows );

  // one arrow per raster pixel instead of the user defined grid
  QgsVectorFieldSettings settings = renderer->settings();
  settings.setOnUserDefinedGrid( false );
  renderer->setSettings( settings );

  mLayer->setRenderer( renderer );

  QGSVERIFYRENDERMAPSETTINGSCHECK( u"raster_vector_field_arrows_on_pixels"_s, u"raster_vector_field_arrows_on_pixels"_s, *mMapSettings, 0, 15 );
}

void TestQgsRasterVectorFieldRenderer::testRenderWindBarbs()
{
  QgsRasterVectorFieldRenderer *renderer = createRenderer( Qgis::VectorFieldSymbology::WindBarbs );

  QgsVectorFieldSettings settings = renderer->settings();
  QgsVectorFieldWindBarbSettings windBarbSettings = settings.windBarbSettings();
  windBarbSettings.setShaftLength( 20 );
  windBarbSettings.setShaftLengthUnits( Qgis::RenderUnit::Pixels );
  windBarbSettings.setMagnitudeUnits( Qgis::WindSpeedUnit::Knots );
  settings.setWindBarbSettings( windBarbSettings );
  renderer->setSettings( settings );

  mLayer->setRenderer( renderer );

  QGSVERIFYRENDERMAPSETTINGSCHECK( u"raster_vector_field_wind_barbs"_s, u"raster_vector_field_wind_barbs"_s, *mMapSettings, 0, 15 );
}

void TestQgsRasterVectorFieldRenderer::testRenderEncodedArrows()
{
  auto renderer = std::make_unique<QgsRasterVectorFieldRenderer>( mDirectionLayer->dataProvider() );
  renderer->setSourceMode( Qgis::RasterVectorFieldSourceMode::EncodedDirection );
  renderer->setDirectionBand( 1 );
  renderer->setDirectionEncoding( Qgis::RasterDirectionEncoding::Esri );

  QgsVectorFieldSettings settings = renderer->settings();
  settings.setSymbology( Qgis::VectorFieldSymbology::Arrows );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 40 );
  settings.setUserGridCellHeight( 40 );

  // the encodings carry no magnitude, so only a fixed shaft length can draw them
  QgsVectorFieldArrowSettings arrowSettings = settings.arrowSettings();
  arrowSettings.setShaftLengthMethod( Qgis::VectorFieldArrowScalingMethod::Fixed );
  arrowSettings.setFixedShaftLength( 6 );
  settings.setArrowsSettings( arrowSettings );
  renderer->setSettings( settings );

  mDirectionLayer->setRenderer( renderer.release() );

  // the test raster drains towards its center, so the arrows converge on it, and the nodata hole
  // and the sinks around the center are left empty
  QGSVERIFYRENDERMAPSETTINGSCHECK( u"raster_vector_field_encoded_arrows"_s, u"raster_vector_field_encoded_arrows"_s, *mDirectionMapSettings, 0, 15 );
}

void TestQgsRasterVectorFieldRenderer::testRenderEncodedStreamlinesAndTraces()
{
  // the glyph symbologies reduce a whole grid cell to one direction, but the streamline and trace
  // integrators walk the field point by point and sample the nearest cell instead. This checks that
  // path draws something rather than comparing an image, as a trace render is not reproducible
  const QVector<Qgis::VectorFieldSymbology> symbologies = { Qgis::VectorFieldSymbology::Streamlines, Qgis::VectorFieldSymbology::Traces };

  for ( Qgis::VectorFieldSymbology symbology : symbologies )
  {
    auto renderer = std::make_unique<QgsRasterVectorFieldRenderer>( mDirectionLayer->dataProvider() );
    renderer->setSourceMode( Qgis::RasterVectorFieldSourceMode::EncodedDirection );
    renderer->setDirectionEncoding( Qgis::RasterDirectionEncoding::Esri );

    QgsVectorFieldSettings settings = renderer->settings();
    settings.setSymbology( symbology );
    renderer->setSettings( settings );

    mDirectionLayer->setRenderer( renderer.release() );

    QgsMapRendererSequentialJob job( *mDirectionMapSettings );
    job.start();
    job.waitForFinished();

    const QImage image = job.renderedImage();
    QVERIFY( !image.isNull() );

    int drawn = 0;
    for ( int y = 0; y < image.height(); ++y )
    {
      for ( int x = 0; x < image.width(); ++x )
      {
        if ( qAlpha( image.pixel( x, y ) ) > 0 )
          ++drawn;
      }
    }

    QVERIFY2( drawn > 0, u"symbology %1 drew nothing"_s.arg( static_cast<int>( symbology ) ).toUtf8().constData() );
  }
}

void TestQgsRasterVectorFieldRenderer::testRenderTraces()
{
  QgsRasterVectorFieldRenderer *renderer = createRenderer( Qgis::VectorFieldSymbology::Traces );

  QgsVectorFieldSettings settings = renderer->settings();
  QgsVectorFieldTracesSettings tracesSettings = settings.tracesSettings();
  tracesSettings.setParticlesCount( 200 );
  tracesSettings.setMaximumTailLength( 40 );
  tracesSettings.setMaximumTailLengthUnit( Qgis::RenderUnit::Pixels );
  settings.setTracesSettings( tracesSettings );
  renderer->setSettings( settings );

  mLayer->setRenderer( renderer );

  // the particles are seeded at random, so a reference image would never match. Only check that
  // the symbology did draw something over the data.
  QgsMapRendererSequentialJob job( *mMapSettings );
  job.start();
  job.waitForFinished();

  const QImage image = job.renderedImage();
  QVERIFY( !image.isNull() );

  const QRgb background = mMapSettings->backgroundColor().rgb() & 0x00ffffff;
  int drawnPixels = 0;
  for ( int y = 0; y < image.height(); ++y )
  {
    for ( int x = 0; x < image.width(); ++x )
    {
      if ( ( image.pixel( x, y ) & 0x00ffffff ) != background )
        ++drawnPixels;
    }
  }
  QVERIFY( drawnPixels > 0 );
}

QGSTEST_MAIN( TestQgsRasterVectorFieldRenderer )
#include "testqgsrastervectorfieldrenderer.moc"
