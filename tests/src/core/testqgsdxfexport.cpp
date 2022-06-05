/***************************************************************************
  testqgsdxfexport.cpp
  --------------------
  Date                 : November 2017
  Copyright            : (C) 2017 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsdxfexport.h"
#include "qgsfillsymbollayer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsfontutils.h"
#include "qgsnullsymbolrenderer.h"
#include "qgstextrenderer.h"
#include "qgspallabeling.h"
#include "qgslabelingengine.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include <QTemporaryFile>
#include <QRegularExpression>

Q_DECLARE_METATYPE( QgsDxfExport::HAlign )
Q_DECLARE_METATYPE( QgsDxfExport::VAlign )

class TestQgsDxfExport : public QObject
{
    Q_OBJECT
  public:
    TestQgsDxfExport() = default;

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testPoints();
    void testLines();
    void testPolygons();
    void testMultiSurface();
    void testMtext();
    void testMtext_data();
    void testMTextEscapeSpaces();
    void testMTextEscapeLineBreaks();
    void testText();
    void testTextAngle();
    void testTextAlign();
    void testTextAlign_data();
    void testTextQuadrant();
    void testTextQuadrant_data();
    void testGeometryGeneratorExport();
    void testCurveExport();
    void testCurveExport_data();
    void testDashedLine();
    void testTransform();

  private:
    QgsVectorLayer *mPointLayer = nullptr;
    QgsVectorLayer *mPointLayerNoSymbols = nullptr;
    QgsVectorLayer *mPointLayerGeometryGenerator = nullptr;
    QgsVectorLayer *mLineLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;

    QString mReport;

    void setDefaultLabelParams( QgsPalLayerSettings &settings );
    QString getTempFileName( const QString &file ) const;

    bool fileContainsText( const QString &path, const QString &text, QString *debugInfo = nullptr ) const;
};

void TestQgsDxfExport::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

void TestQgsDxfExport::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsDxfExport::init()
{
  QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";

  mPointLayer = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointLayer->isValid() );
  QgsProject::instance()->addMapLayer( mPointLayer );

  mPointLayerNoSymbols = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointLayerNoSymbols->isValid() );
  mPointLayerNoSymbols->setRenderer( new QgsNullSymbolRenderer() );
  mPointLayerNoSymbols->addExpressionField( QStringLiteral( "'A text with spaces'" ), QgsField( QStringLiteral( "Spacestest" ), QVariant::String ) );
  QgsProject::instance()->addMapLayer( mPointLayerNoSymbols );

  //Point layer with geometry generator symbolizer
  mPointLayerGeometryGenerator = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointLayerGeometryGenerator );

  QVariantMap ggProps;
  ggProps.insert( QStringLiteral( "SymbolType" ), QStringLiteral( "Fill" ) );
  ggProps.insert( QStringLiteral( "geometryModifier" ), QStringLiteral( "buffer( $geometry, 0.1 )" ) );
  QgsSymbolLayer *ggSymbolLayer = QgsGeometryGeneratorSymbolLayer::create( ggProps );
  QgsSymbolLayerList fillSymbolLayerList;
  fillSymbolLayerList << new QgsSimpleFillSymbolLayer();
  ggSymbolLayer->setSubSymbol( new QgsFillSymbol( fillSymbolLayerList ) );
  QgsSymbolLayerList slList;
  slList << ggSymbolLayer;
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( slList );
  QgsSingleSymbolRenderer *sr = new QgsSingleSymbolRenderer( markerSymbol );
  mPointLayerGeometryGenerator->setRenderer( sr );

  QgsProject::instance()->addMapLayer( mPointLayerGeometryGenerator );

  filename = QStringLiteral( TEST_DATA_DIR ) + "/lines.shp";
  mLineLayer = new QgsVectorLayer( filename, QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( mLineLayer->isValid() );
  QgsProject::instance()->addMapLayer( mLineLayer );
  filename = QStringLiteral( TEST_DATA_DIR ) + "/polys.shp";
  mPolygonLayer = new QgsVectorLayer( filename, QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPolygonLayer->isValid() );
  QgsProject::instance()->addMapLayer( mPolygonLayer );
}

void TestQgsDxfExport::cleanup()
{
  QgsProject::instance()->removeMapLayer( mPointLayer->id() );
  QgsProject::instance()->removeMapLayer( mPointLayerNoSymbols->id() );
  QgsProject::instance()->removeMapLayer( mPointLayerGeometryGenerator->id() );
  QgsProject::instance()->removeMapLayer( mLineLayer->id() );
  QgsProject::instance()->removeMapLayer( mPolygonLayer->id() );
  mPointLayer = nullptr;
}

void TestQgsDxfExport::testPoints()
{
  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( mPointLayer ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  const QString file = getTempFileName( "point_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  QVERIFY( !fileContainsText( file, QStringLiteral( "nan.0" ) ) );

  // reload and compare
  std::unique_ptr< QgsVectorLayer > result = std::make_unique< QgsVectorLayer >( file, "dxf" );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), mPointLayer->featureCount() );
  QCOMPARE( result->wkbType(), QgsWkbTypes::Point );
}

void TestQgsDxfExport::testLines()
{
  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( mLineLayer ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mLineLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mLineLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mLineLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  const QString file = getTempFileName( "line_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  // reload and compare
  std::unique_ptr< QgsVectorLayer > result = std::make_unique< QgsVectorLayer >( file, "dxf" );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), mLineLayer->featureCount() );
  QCOMPARE( result->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsDxfExport::testPolygons()
{
  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( mPolygonLayer ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPolygonLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPolygonLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPolygonLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  const QString file = getTempFileName( "polygon_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  // reload and compare
  std::unique_ptr< QgsVectorLayer > result = std::make_unique< QgsVectorLayer >( file, "dxf" );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 12L );
  QCOMPARE( result->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsDxfExport::testMultiSurface()
{
  QgsDxfExport d;
  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "MultiSurface" ), QString(), QStringLiteral( "memory" ) );
  const QgsGeometry g = QgsGeometry::fromWkt( "MultiSurface (Polygon ((0 0, 0 1, 1 1, 0 0)))" );
  QgsFeature f;
  f.setGeometry( g );
  vl->dataProvider()->addFeatures( QgsFeatureList() << f );
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( vl.get() ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( vl->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  const QString file = getTempFileName( "multisurface_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  // reload and compare
  std::unique_ptr< QgsVectorLayer > result = std::make_unique< QgsVectorLayer >( file, "dxf" );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 1L );
  QCOMPARE( result->wkbType(), QgsWkbTypes::LineString );
  QgsFeature f2;
  result->getFeatures().nextFeature( f2 );
  QCOMPARE( f2.geometry().asWkt(), QStringLiteral( "LineString (0 0, 0 1, 1 1, 0 0)" ) );
}

void TestQgsDxfExport::testMtext()
{
  QFETCH( QgsVectorLayer *, layer );
  QFETCH( QString, layerName );

  QVERIFY( layer );

  QgsProject::instance()->addMapLayer( layer );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  layer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  layer->setLabelsEnabled( true );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( layer ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( layer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << layer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( layer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );

  const QString file = getTempFileName( layerName );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  QString debugInfo;
  QVERIFY2( fileContainsText( file, "MTEXT\n"
                              "  5\n"
                              "**no check**\n"
                              "100\n"
                              "AcDbEntity\n"
                              "100\n"
                              "AcDbMText\n"
                              "  8\n"
                              "points\n"
                              "420\n"
                              "**no check**\n"
                              " 10\n"
                              "**no check**\n"
                              " 20\n"
                              "**no check**\n"
                              "  1\n"
                              "\\fQGIS Vera Sans|i0|b1;\\H3.81136;Biplane\n"
                              " 50\n"
                              "0.0\n"
                              " 41\n"
                              "**no check**\n"
                              " 71\n"
                              "     7\n"
                              "  7\n"
                              "STANDARD\n"
                              "  0", &debugInfo ), debugInfo.toUtf8().constData() );


  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsDxfExport::testMtext_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layer" );
  QTest::addColumn<QString>( "layerName" );

  const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";

  QgsVectorLayer *pointLayer = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointLayer->isValid() );

  QTest::newRow( "MText" )
      << pointLayer
      << QStringLiteral( "mtext_dxf" );

  QgsVectorLayer *pointLayerNoSymbols = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointLayerNoSymbols->isValid() );
  pointLayerNoSymbols->setRenderer( new QgsNullSymbolRenderer() );
  pointLayerNoSymbols->addExpressionField( QStringLiteral( "'A text with spaces'" ), QgsField( QStringLiteral( "Spacestest" ), QVariant::String ) );

  QTest::newRow( "MText No Symbology" )
      << pointLayerNoSymbols
      << QStringLiteral( "mtext_no_symbology_dxf" );
}

void TestQgsDxfExport::testMTextEscapeSpaces()
{
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Spacestest" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  mPointLayerNoSymbols->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  mPointLayerNoSymbols->setLabelsEnabled( true );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( mPointLayerNoSymbols ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayerNoSymbols->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayerNoSymbols );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayerNoSymbols->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );

  const QString file = getTempFileName( "mtext_escape_spaces" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();
  QString debugInfo;
  QVERIFY2( fileContainsText( file, "\\fQGIS Vera Sans|i0|b1;\\H3.81136;A\\~text\\~with\\~spaces", &debugInfo ), debugInfo.toUtf8().constData() );
}

void TestQgsDxfExport::testMTextEscapeLineBreaks()
{
  const int field = mPointLayerNoSymbols->addExpressionField( QStringLiteral( "'A text with ' || char(13) || char(10) || 'line break'" ), QgsField( QStringLiteral( "linebreaktest" ), QVariant::String ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "linebreaktest" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  mPointLayerNoSymbols->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  mPointLayerNoSymbols->setLabelsEnabled( true );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( mPointLayerNoSymbols ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayerNoSymbols->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayerNoSymbols );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayerNoSymbols->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );

  const QString file = getTempFileName( "mtext_escape_linebreaks" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  dxfFile.open( QIODevice::ReadOnly );
  const QString fileContent = QTextStream( &dxfFile ).readAll();
  dxfFile.close();
  QVERIFY( fileContent.contains( "A\\~text\\~with\\~\\Pline\\~break" ) );
  mPointLayerNoSymbols->removeExpressionField( field );
}

void TestQgsDxfExport::testText()
{
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  mPointLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  mPointLayer->setLabelsEnabled( true );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( mPointLayer ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );
  d.setFlags( QgsDxfExport::FlagNoMText );

  const QString file = getTempFileName( "text_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  QString debugInfo;
  QVERIFY2( fileContainsText( file, "TEXT\n"
                              "  5\n"
                              "**no check**\n"
                              "100\n"
                              "AcDbEntity\n"
                              "100\n"
                              "AcDbText\n"
                              "  8\n"
                              "points\n"
                              "420\n"
                              "**no check**\n"
                              " 10\n"
                              "**no check**\n"
                              " 20\n"
                              "**no check**\n"
                              " 40\n"
                              "**no check**\n"
                              "  1\n"
                              "Biplane\n"
                              " 50\n"
                              "0.0\n"
                              "  7\n"
                              "STANDARD\n"
                              "100\n"
                              "AcDbText", &debugInfo ), debugInfo.toUtf8().constData() );
}

void TestQgsDxfExport::testTextAngle()
{
  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:2056&field=ori:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  const QgsGeometry g = QgsGeometry::fromWkt( "Point(2684679.392 1292182.527)" );
  const QgsGeometry g2 = QgsGeometry::fromWkt( "Point(2684692.322 1292192.534)" );
  QgsFeature f( vl->fields() );
  f.setGeometry( g );
  f.setAttribute( 0, 30 );

  vl->dataProvider()->addFeatures( QgsFeatureList() << f );

  f.setGeometry( g2 );
  f.setAttribute( 0, 40 );

  vl->dataProvider()->addFeatures( QgsFeatureList() << f );

  QgsPalLayerSettings settings;
  auto ddp = settings.dataDefinedProperties();
  QgsProperty prop;
  prop.setExpressionString( QStringLiteral( "ori" ) );
  ddp.setProperty( QgsPalLayerSettings::Property::LabelRotation, prop );
  settings.setDataDefinedProperties( ddp );
  settings.fieldName = QStringLiteral( "ori" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  settings.setFormat( format );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( vl.get() ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 2684579, 1292082, 2684779, 1292282 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( vl->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );
  d.setFlags( QgsDxfExport::FlagNoMText );

  const QString file = getTempFileName( "text_dxf_angle" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  QString debugInfo;
  QVERIFY2( fileContainsText( file, "TEXT\n"
                              "  5\n"
                              "**no check**\n"
                              "100\n"
                              "AcDbEntity\n"
                              "100\n"
                              "AcDbText\n"
                              "  8\n"
                              "vl\n"
                              "420\n"
                              "**no check**\n"
                              " 10\n"
                              "**no check**\n"
                              " 20\n"
                              "**no check**\n"
                              " 40\n"
                              "**no check**\n"
                              "  1\n"
                              "40\n"
                              " 50\n"
                              "320.0\n"
                              "  7\n"
                              "STANDARD\n"
                              "100\n"
                              "AcDbText", &debugInfo ), debugInfo.toUtf8().constData() );
}

void TestQgsDxfExport::testTextAlign()
{
  QFETCH( QgsDxfExport::HAlign, dxfHali );
  QFETCH( QgsDxfExport::VAlign, dxfVali );
  QFETCH( QString, hali );
  QFETCH( QString, vali );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "text" );

  QgsPropertyCollection props = settings.dataDefinedProperties();
  QgsProperty halignProp = QgsProperty();
  halignProp.setStaticValue( hali );
  props.setProperty( QgsPalLayerSettings::Hali, halignProp );
  QgsProperty posXProp = QgsProperty();
  posXProp.setExpressionString( QStringLiteral( "x($geometry) + 1" ) );
  props.setProperty( QgsPalLayerSettings::PositionX, posXProp );
  QgsProperty valignProp = QgsProperty();
  valignProp.setStaticValue( vali );
  props.setProperty( QgsPalLayerSettings::Vali, valignProp );
  QgsProperty posYProp = QgsProperty();
  posYProp.setExpressionString( QStringLiteral( "y($geometry) + 1" ) );
  props.setProperty( QgsPalLayerSettings::PositionY, posYProp );
  settings.setDataDefinedProperties( props );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:2056&field=text:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  const QgsGeometry g = QgsGeometry::fromWkt( "Point(2684679.392 1292182.527)" );
  QgsFeature f( vl->fields() );
  f.setGeometry( g );
  f.setAttribute( 0, QStringLiteral( "--- MY TEXT ---" ) );

  vl->dataProvider()->addFeatures( QgsFeatureList() << f );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 2684658.97702550329267979, 1292165.99626861698925495, 2684711.73293229937553406, 1292188.10791716771200299 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( vl->crs() );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( vl.get() ) );
  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );
  d.setFlags( QgsDxfExport::FlagNoMText );
  d.setExtent( mapSettings.extent() );

  static int testNumber = 0;
  ++testNumber;
  const QString file = getTempFileName( QStringLiteral( "text_dxf_%1_%2" ).arg( hali, vali ) );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();
  QString debugInfo;
  QVERIFY2( fileContainsText( file, QStringLiteral( "TEXT\n"
                              "  5\n"
                              "**no check**\n"
                              "100\n"
                              "AcDbEntity\n"
                              "100\n"
                              "AcDbText\n"
                              "  8\n"
                              "vl\n"
                              "420\n"
                              "**no check**\n"
                              " 10\n"
                              "REGEX ^2684680\\.39\\d*\n"
                              " 20\n"
                              "REGEX ^1292183\\.52\\d*\n"
                              " 11\n"
                              "REGEX ^2684680\\.39\\d*\n"
                              " 21\n"
                              "REGEX ^1292183\\.52\\d*\n"
                              " 40\n"
                              "**no check**\n"
                              "  1\n"
                              "--- MY TEXT ---\n"
                              " 50\n"
                              "0.0\n"
                              " 72\n"
                              "     %1\n"
                              "  7\n"
                              "STANDARD\n"
                              "100\n"
                              "AcDbText\n"
                              " 73\n"
                              "     %2" ).arg( QString::number( static_cast<int>( dxfHali ) ), QString::number( static_cast<int>( dxfVali ) ) ), &debugInfo ), debugInfo.toUtf8().constData() );
}

void TestQgsDxfExport::testTextAlign_data()
{
  QTest::addColumn<QgsDxfExport::HAlign>( "dxfHali" );
  QTest::addColumn<QgsDxfExport::VAlign>( "dxfVali" );
  QTest::addColumn<QString>( "hali" );
  QTest::addColumn<QString>( "vali" );

  QTest::newRow( "Align left bottom" )
      << QgsDxfExport::HAlign::HLeft
      << QgsDxfExport::VAlign::VBottom
      << QStringLiteral( "Left" )
      << QStringLiteral( "Bottom" );

  QTest::newRow( "Align center bottom" )
      << QgsDxfExport::HAlign::HCenter
      << QgsDxfExport::VAlign::VBottom
      << QStringLiteral( "Center" )
      << QStringLiteral( "Bottom" );

  QTest::newRow( "Align right bottom" )
      << QgsDxfExport::HAlign::HRight
      << QgsDxfExport::VAlign::VBottom
      << QStringLiteral( "Right" )
      << QStringLiteral( "Bottom" );

  QTest::newRow( "Align left top" )
      << QgsDxfExport::HAlign::HLeft
      << QgsDxfExport::VAlign::VTop
      << QStringLiteral( "Left" )
      << QStringLiteral( "Top" );

  QTest::newRow( "Align right cap" )
      << QgsDxfExport::HAlign::HRight
      << QgsDxfExport::VAlign::VTop
      << QStringLiteral( "Right" )
      << QStringLiteral( "Cap" );

  QTest::newRow( "Align left base" )
      << QgsDxfExport::HAlign::HLeft
      << QgsDxfExport::VAlign::VBaseLine
      << QStringLiteral( "Left" )
      << QStringLiteral( "Base" );

  QTest::newRow( "Align center half" )
      << QgsDxfExport::HAlign::HCenter
      << QgsDxfExport::VAlign::VMiddle
      << QStringLiteral( "Center" )
      << QStringLiteral( "Half" );
}

void TestQgsDxfExport::testTextQuadrant()
{
  QFETCH( int, offsetQuad );
  QFETCH( QgsDxfExport::HAlign, dxfHali );
  QFETCH( QgsDxfExport::VAlign, dxfVali );
  QFETCH( double, angle );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "text" );
  settings.placement = Qgis::LabelPlacement::OverPoint;

  QgsPropertyCollection props = settings.dataDefinedProperties();
  QgsProperty offsetQuadProp = QgsProperty();
  offsetQuadProp.setStaticValue( offsetQuad );
  props.setProperty( QgsPalLayerSettings::OffsetQuad, offsetQuadProp );
  props.setProperty( QgsPalLayerSettings::Property::LabelRotation, angle );
  settings.setDataDefinedProperties( props );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:2056&field=text:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  const QgsGeometry g = QgsGeometry::fromWkt( "Point(2685025.687 1292145.297)" );
  QgsFeature f( vl->fields() );
  f.setGeometry( g );
  f.setAttribute( 0, QStringLiteral( "182" ) );

  vl->dataProvider()->addFeatures( QgsFeatureList() << f );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 2685025.687, 1292045.297, 2685125.687, 1292145.297 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( vl->crs() );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( vl.get() ) );
  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );
  d.setFlags( QgsDxfExport::FlagNoMText );
  d.setExtent( mapSettings.extent() );

  static int testNumber = 0;
  ++testNumber;
  const QString file = getTempFileName( QStringLiteral( "text_dxf_offset_quad_%1_%2" ).arg( offsetQuad ).arg( angle ) );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();
  QString debugInfo;
  QVERIFY2( fileContainsText( file, QStringLiteral( "TEXT\n"
                              "  5\n"
                              "**no check**\n"
                              "100\n"
                              "AcDbEntity\n"
                              "100\n"
                              "AcDbText\n"
                              "  8\n"
                              "vl\n"
                              "420\n"
                              "**no check**\n"
                              " 10\n"
                              "REGEX ^2685025\\.68\\d*\n"
                              " 20\n"
                              "REGEX ^1292145\\.29\\d*\n"
                              " 11\n"
                              "REGEX ^2685025\\.68\\d*\n"
                              " 21\n"
                              "REGEX ^1292145\\.29\\d*\n"
                              " 40\n"
                              "**no check**\n"
                              "  1\n"
                              "182\n"
                              " 50\n"
                              "%1\n"
                              " 72\n"
                              "     %2\n"
                              "  7\n"
                              "STANDARD\n"
                              "100\n"
                              "AcDbText\n"
                              " 73\n"
                              "     %3" ).arg( QString::number( fmod( 360 - angle, 360 ), 'f', 1 ) ).arg( QString::number( static_cast<int>( dxfHali ) ), QString::number( static_cast<int>( dxfVali ) ) ), &debugInfo ), debugInfo.toUtf8().constData() );
}

void TestQgsDxfExport::testTextQuadrant_data()
{
  QTest::addColumn<int>( "offsetQuad" );
  QTest::addColumn<QgsDxfExport::HAlign>( "dxfHali" );
  QTest::addColumn<QgsDxfExport::VAlign>( "dxfVali" );
  QTest::addColumn<double>( "angle" );

  QTest::newRow( "Above Left, no rotation" )
      << 0
      << QgsDxfExport::HAlign::HRight
      << QgsDxfExport::VAlign::VBottom
      << 0.0;

  QTest::newRow( "Above, no rotation" )
      << 1
      << QgsDxfExport::HAlign::HCenter
      << QgsDxfExport::VAlign::VBottom
      << 0.0;

  QTest::newRow( "Above Right, no rotation" )
      << 2
      << QgsDxfExport::HAlign::HLeft
      << QgsDxfExport::VAlign::VBottom
      << 0.0;

  QTest::newRow( "Left, no rotation" )
      << 3
      << QgsDxfExport::HAlign::HRight
      << QgsDxfExport::VAlign::VMiddle
      << 0.0;

  QTest::newRow( "Over, no rotation" )
      << 4
      << QgsDxfExport::HAlign::HCenter
      << QgsDxfExport::VAlign::VMiddle
      << 0.0;

  QTest::newRow( "Right, no rotation" )
      << 5
      << QgsDxfExport::HAlign::HLeft
      << QgsDxfExport::VAlign::VMiddle
      << 0.0;

  QTest::newRow( "Below Left, no rotation" )
      << 6
      << QgsDxfExport::HAlign::HRight
      << QgsDxfExport::VAlign::VTop
      << 0.0;

  QTest::newRow( "Below, no rotation" )
      << 7
      << QgsDxfExport::HAlign::HCenter
      << QgsDxfExport::VAlign::VTop
      << 0.0;

  QTest::newRow( "Below Right, no rotation" )
      << 8
      << QgsDxfExport::HAlign::HLeft
      << QgsDxfExport::VAlign::VTop
      << 0.0;

  QTest::newRow( "Below, 20°" )
      << 7
      << QgsDxfExport::HAlign::HCenter
      << QgsDxfExport::VAlign::VTop
      << 20.0;
}

void TestQgsDxfExport::testGeometryGeneratorExport()
{
  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( mPointLayerGeometryGenerator ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayerGeometryGenerator->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayerGeometryGenerator );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayerGeometryGenerator->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 6000000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );

  const QString file = getTempFileName( "geometry_generator_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  QVERIFY( fileContainsText( file, "HATCH" ) );
}

void TestQgsDxfExport::testCurveExport()
{
  QFETCH( QString, wkt );
  QFETCH( QString, wktType );
  QFETCH( QString, dxfText );

  QgsDxfExport d;
  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( wktType, QString(), QStringLiteral( "memory" ) );
  const QgsGeometry g = QgsGeometry::fromWkt( wkt );
  QgsFeature f;
  f.setGeometry( g );
  vl->dataProvider()->addFeatures( QgsFeatureList() << f );
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( vl.get() ) );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( vl->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  const QString file = getTempFileName( wktType );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  QString debugInfo;
  QVERIFY2( fileContainsText( file, dxfText, &debugInfo ), debugInfo.toUtf8().constData() );
}

void TestQgsDxfExport::testCurveExport_data()
{
  QTest::addColumn<QString>( "wkt" );
  QTest::addColumn<QString>( "wktType" );
  QTest::addColumn<QString>( "dxfText" );

  // curved segment
  QTest::newRow( "circular string" )
      << QStringLiteral( "CompoundCurve (CircularString (220236.7836819862422999 150406.56493463439983316, 220237.85162031010258943 150412.10612405074061826, 220242.38532074165414087 150409.6075513684481848))" )
      << QStringLiteral( "CircularString" )
      << QStringLiteral( "SECTION\n"
                         "  2\n"
                         "ENTITIES\n"
                         "  0\n"
                         "LWPOLYLINE\n"
                         "  5\n"
                         "82\n"
                         "  8\n"
                         "0\n"
                         "100\n"
                         "AcDbEntity\n"
                         "100\n"
                         "AcDbPolyline\n"
                         "  6\n"
                         "CONTINUOUS\n"
                         "420\n"
                         "     0\n"
                         " 90\n"
                         "     2\n"
                         " 70\n"
                         "   130\n"
                         " 43\n"
                         "-1.0\n"
                         " 10\n"
                         "220236.7836819862422999\n"
                         " 20\n"
                         "150406.56493463439983316\n"
                         " 42\n"
                         "-1.37514344818771517\n"
                         " 10\n"
                         "220242.38532074165414087\n"
                         " 20\n"
                         "150409.6075513684481848\n"
                         "  0\n"
                         "ENDSEC" );

  // Contains straight and curved segments
  QTest::newRow( "mixed curve polygon" )
      << QStringLiteral( "CurvePolygon (CompoundCurve ((-1.58053402239448748 0.39018087855297157, -1.49267872523686473 0.39362618432385876, -1.24806201550387597 0.65719207579672689),CircularString (-1.24806201550387597 0.65719207579672689, -0.63479758828596045 0.49870801033591727, -0.61584840654608097 0.32644272179155898),(-0.61584840654608097 0.32644272179155898, -1.58053402239448748 0.39018087855297157)))" )
      << QStringLiteral( "CurvePolygon" )
      << QStringLiteral( "SECTION\n"
                         "  2\n"
                         "ENTITIES\n"
                         "  0\n"
                         "LWPOLYLINE\n"
                         "  5\n"
                         "82\n"
                         "  8\n"
                         "0\n"
                         "100\n"
                         "AcDbEntity\n"
                         "100\n"
                         "AcDbPolyline\n"
                         "  6\n"
                         "CONTINUOUS\n"
                         "420\n"
                         "     0\n"
                         " 90\n"
                         "     5\n"
                         " 70\n"
                         "   131\n"
                         " 43\n"
                         "-1.0\n"
                         " 10\n"
                         "-1.58053402239448748\n"
                         " 20\n"
                         "0.39018087855297157\n"
                         " 10\n"
                         "-1.49267872523686473\n"
                         " 20\n"
                         "0.39362618432385876\n"
                         " 10\n"
                         "-1.24806201550387597\n"
                         " 20\n"
                         "0.65719207579672689\n"
                         " 42\n"
                         "-0.69027811746778556\n"
                         " 10\n"
                         "-0.61584840654608097\n"
                         " 20\n"
                         "0.32644272179155898\n"
                         " 10\n"
                         "-1.58053402239448748\n"
                         " 20\n"
                         "0.39018087855297157\n"
                         "  0\n"
                         "ENDSEC" );

}

void TestQgsDxfExport::testDashedLine()
{
  std::unique_ptr<QgsSimpleLineSymbolLayer> symbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>( QColor( 0, 0, 0 ) );
  symbolLayer->setWidth( 0.11 );
  symbolLayer->setCustomDashVector( { 0.5, 0.35 } );
  symbolLayer->setCustomDashPatternUnit( QgsUnitTypes::RenderUnit::RenderMapUnits );
  symbolLayer->setUseCustomDashPattern( true );

  QgsLineSymbol *symbol = new QgsLineSymbol();
  symbol->changeSymbolLayer( 0, symbolLayer.release() );

  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "CompoundCurve?crs=epsg:2056" ), QString(), QStringLiteral( "memory" ) );
  const QgsGeometry g = QgsGeometry::fromWkt( "CompoundCurve ((2689563.84200000017881393 1283531.23699999996460974, 2689563.42499999981373549 1283537.55499999993480742, 2689563.19900000002235174 1283540.52399999997578561, 2689562.99800000013783574 1283543.42999999993480742, 2689562.66900000022724271 1283548.56000000005587935, 2689562.43399999989196658 1283555.287999999942258))" );
  QgsFeature f;
  f.setGeometry( g );
  vl->dataProvider()->addFeatures( QgsFeatureList() << f );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( symbol );
  vl->setRenderer( renderer );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( vl.get() ) );
  d.setSymbologyExport( QgsDxfExport::SymbologyExport::SymbolLayerSymbology );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( vl->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  const QString file = getTempFileName( "dashed_line_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  QString debugInfo;

  // Make sure the style definition for the dashed line is there
  QVERIFY2( fileContainsText( file,
                              "LTYPE\n"
                              "  5\n"
                              "6c\n"
                              "100\n"
                              "AcDbSymbolTableRecord\n"
                              "100\n"
                              "AcDbLinetypeTableRecord\n"
                              "  2\n"
                              "symbolLayer0\n"
                              " 70\n"
                              "    64\n"
                              "  3\n"
                              "\n"
                              " 72\n"
                              "    65\n"
                              " 73\n"
                              "     2\n"
                              " 40\n"
                              "REGEX ^0\\.8[0-9]*\n"
                              " 49\n"
                              "0.5\n"
                              " 74\n"
                              "     0\n"
                              " 49\n"
                              "REGEX ^-0\\.3[0-9]*\n"
                              " 74\n"
                              "     0", &debugInfo ), debugInfo.toUtf8().constData() );

  // Make sure that the polyline references the style symbolLayer0
  QVERIFY2( fileContainsText( file,
                              "LWPOLYLINE\n"
                              "  5\n"
                              "83\n"
                              "  8\n"
                              "0\n"
                              "100\n"
                              "AcDbEntity\n"
                              "100\n"
                              "AcDbPolyline\n"
                              "  6\n"
                              "symbolLayer0\n"
                              "420\n"
                              "     0\n"
                              " 90\n"
                              "     6\n"
                              " 70\n"
                              "   128\n"
                              " 43\n"
                              "0.11\n"
                              " 10\n"
                              "REGEX ^2689563.84[0-9]*\n"
                              " 20\n"
                              "REGEX ^1283531.23[0-9]*\n"
                              " 10\n"
                              "REGEX ^2689563.42[0-9]*\n"
                              " 20\n"
                              "REGEX ^1283537.55[0-9]*\n"
                              " 10\n"
                              "REGEX ^2689563.19[0-9]*\n"
                              " 20\n"
                              "REGEX ^1283540.52[0-9]*\n"
                              " 10\n"
                              "REGEX ^2689562.99[0-9]*\n"
                              " 20\n"
                              "REGEX ^1283543.42[0-9]*\n"
                              " 10\n"
                              "REGEX ^2689562.66[0-9]*\n"
                              " 20\n"
                              "REGEX ^1283548.56[0-9]*\n"
                              " 10\n"
                              "REGEX ^2689562.43[0-9]*\n"
                              " 20\n"
                              "REGEX ^1283555.28[0-9]*\n"
                              "  0\n"
                              "ENDSEC"
                              , &debugInfo ), debugInfo.toUtf8().constData() );
}

void TestQgsDxfExport::testTransform()
{
  std::unique_ptr<QgsSimpleLineSymbolLayer> symbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>( QColor( 0, 0, 0 ) );
  symbolLayer->setWidth( 0.11 );
  symbolLayer->setCustomDashVector( { 0.5, 0.35 } );
  symbolLayer->setCustomDashPatternUnit( QgsUnitTypes::RenderUnit::RenderMapUnits );
  symbolLayer->setUseCustomDashPattern( true );

  QgsLineSymbol *symbol = new QgsLineSymbol();
  symbol->changeSymbolLayer( 0, symbolLayer.release() );

  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "Linestring?crs=epsg:2056" ), QString(), QStringLiteral( "memory" ) );
  QgsGeometry g = QgsGeometry::fromWkt( QStringLiteral( "LineString (2689564.82757076947018504 1283554.68540272791869938, 2689565.52996697928756475 1283531.49185784510336816)" ) );
  QgsFeature f;
  f.setGeometry( g );
  vl->dataProvider()->addFeatures( QgsFeatureList() << f );
  g = QgsGeometry::fromWkt( QStringLiteral( "LineString( 2689550.41764387069270015 1283518.10608713980764151, 2689586.27526817657053471 1283519.37654714332893491 )" ) );
  f.setGeometry( g );
  vl->dataProvider()->addFeatures( QgsFeatureList() << f );

  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( symbol );
  vl->setRenderer( renderer );

  QgsDxfExport d;
  d.addLayers( QList< QgsDxfExport::DxfLayer >() << QgsDxfExport::DxfLayer( vl.get() ) );
  d.setSymbologyExport( QgsDxfExport::SymbologyExport::SymbolLayerSymbology );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  const QString file = getTempFileName( QStringLiteral( "line_transform" ) );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile.close();

  std::unique_ptr< QgsVectorLayer > result = std::make_unique< QgsVectorLayer >( file, QStringLiteral( "res" ) );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 2L );
  QgsFeature f2;
  QgsFeatureIterator it = result->getFeatures();
  QVERIFY( it.nextFeature( f2 ) );
  QCOMPARE( f2.geometry().asWkt( 0 ), QStringLiteral( "LineString (960884 6056508, 960884 6056473)" ) );
  QVERIFY( it.nextFeature( f2 ) );
  QCOMPARE( f2.geometry().asWkt( 0 ), QStringLiteral( "LineString (960862 6056454, 960915 6056455)" ) );

  // export a subset via extent (this is in EPSG:3857 -- the destination crs
  d.setExtent( QgsRectangle( 960858.48, 6056426.49, 960918.31, 6056467.93 ) );
  const QString file2 = getTempFileName( QStringLiteral( "line_transform2" ) );
  QFile dxfFile2( file2 );
  QCOMPARE( d.writeToFile( &dxfFile2, QStringLiteral( "CP1252" ) ), QgsDxfExport::ExportResult::Success );
  dxfFile2.close();

  result = std::make_unique< QgsVectorLayer >( file2, QStringLiteral( "res" ) );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 1L );
  it = result->getFeatures();
  QVERIFY( it.nextFeature( f2 ) );
  QCOMPARE( f2.geometry().asWkt( 0 ), QStringLiteral( "LineString (960862 6056454, 960915 6056455)" ) );
}

bool TestQgsDxfExport::fileContainsText( const QString &path, const QString &text, QString *debugInfo ) const
{
  QStringList debugLines;
  const QStringList searchLines = text.split( '\n' );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
    return false;
  QTextStream in( &file );
  QString line;
  QString failedLine;
  QString failedCandidateLine;
  int maxLine = 0;
  do
  {
    bool found = true;
    int i = 0;
    for ( const QString &searchLine : searchLines )
    {
      line = in.readLine();
      if ( searchLine != QLatin1String( "**no check**" ) )
      {
        if ( line != searchLine )
        {
          bool ok = false;
          if ( searchLine.startsWith( QLatin1String( "REGEX " ) ) )
          {
            const QRegularExpression re( searchLine.mid( 6 ) );
            if ( re.match( line ).hasMatch() )
              ok = true;
          }

          if ( !ok )
          {
            if ( i == maxLine )
            {
              failedLine = searchLine;
              failedCandidateLine = line;
            }
            found = false;
            break;
          }
        }
      }
      i++;
      if ( i > maxLine )
      {
        maxLine = i;
        debugLines.append( QStringLiteral( "\n  Found line: %1" ).arg( searchLine ) );
      }
    }
    if ( found )
      return true;
  }
  while ( !line.isNull() );
  if ( debugInfo )
  {
    while ( debugLines.size() > 10 )
      debugLines.removeFirst();
    debugInfo->append( debugLines.join( QLatin1String( "" ) ) );
    debugInfo->append( QStringLiteral( "\n  Failed on line %1" ).arg( failedLine ) );
    debugInfo->append( QStringLiteral( "\n  Candidate line %1" ).arg( failedCandidateLine ) );
  }
  return false;
}

QString TestQgsDxfExport::getTempFileName( const QString &file ) const
{
  return QDir::tempPath() + '/' + file + ".dxf";
}


QGSTEST_MAIN( TestQgsDxfExport )
#include "testqgsdxfexport.moc"
