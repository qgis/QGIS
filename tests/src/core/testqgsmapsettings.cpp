/***************************************************************************
     testqgsmapsettings.cpp
     --------------------------------------
    Date                 : Tue  6 Feb 2015
    Copyright            : (C) 2014 by Sandro Santilli
    Email                : strk@keybit.net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>

#include "qgstest.h"

#include <QObject>
#include <QString>

//header for class being tested
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgsrectangle.h"
#include "qgsmapsettings.h"
#include "qgspointxy.h"
#include "qgsapplication.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsgrouplayer.h"

class TestHandler : public QgsRenderedFeatureHandlerInterface
{
  public:
    void handleRenderedFeature( const QgsFeature &, const QgsGeometry &, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext & ) override {}
};

class TestQgsMapSettings : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDefaults();
    void testGettersSetters();
    void testLabelingEngineSettings();
    void visibleExtent();
    void extentBuffer();
    void mapUnitsPerPixel();
    void testDevicePixelRatio();
    void visiblePolygon();
    void visiblePolygonWithBuffer();
    void testIsLayerVisible();
    void testMapLayerListUtils();
    void testXmlReadWrite();
    void testSetLayers();
    void testLabelBoundary();
    void testExpressionContext();
    void testRenderedFeatureHandlers();
    void testCustomRenderingFlags();
    void testClippingRegions();
    void testScale();
    void testComputeExtentForScale();
    void testComputeScaleForExtent();
    void testLayersWithGroupLayers();
    void testMaskRenderSettings();
    void testDeprecatedFlagsRasterizePolicy();

  private:
    QString toString( const QPolygonF &p, int decimalPlaces = 2 ) const;
};

void TestQgsMapSettings::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsMapSettings::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QString TestQgsMapSettings::toString( const QPolygonF &p, int dec ) const
{
  QString s;
  const char *sep = "";
  const double r = std::pow( 10.0, dec );
  for ( int i = 0; i < p.size(); ++i )
  {
    s += u"%1%2 %3"_s.arg( sep ).arg( int( p[i].x() * r ) / r ).arg( int( p[i].y() * r ) / r );
    sep = ",";
  }

  return s;
}

void TestQgsMapSettings::testDefaults()
{
  const QgsMapSettings ms;
  QCOMPARE( ms.destinationCrs(), QgsCoordinateReferenceSystem() );
}

void TestQgsMapSettings::testGettersSetters()
{
  // basic getter/setter tests
  QgsMapSettings ms;

  ms.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  QCOMPARE( ms.textRenderFormat(), Qgis::TextRenderFormat::AlwaysText );
  ms.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysOutlines );
  QCOMPARE( ms.textRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );

  // must default to no simplification
  QCOMPARE( ms.simplifyMethod().simplifyHints(), Qgis::VectorRenderingSimplificationFlags() );
  QgsVectorSimplifyMethod simplify;
  simplify.setSimplifyHints( Qgis::VectorRenderingSimplificationFlag::GeometrySimplification );
  ms.setSimplifyMethod( simplify );
  QCOMPARE( ms.simplifyMethod().simplifyHints(), Qgis::VectorRenderingSimplificationFlag::GeometrySimplification );

  QVERIFY( ms.zRange().isInfinite() );
  ms.setZRange( QgsDoubleRange( 1, 10 ) );
  QCOMPARE( ms.zRange(), QgsDoubleRange( 1, 10 ) );

  QCOMPARE( ms.frameRate(), -1.0 );
  ms.setFrameRate( 30.0 );
  QCOMPARE( ms.frameRate(), 30.0 );

  QCOMPARE( ms.currentFrame(), -1 );
  ms.setCurrentFrame( 6 );
  QCOMPARE( ms.currentFrame(), 6LL );
}

void TestQgsMapSettings::testLabelingEngineSettings()
{
  // test that setting labeling engine settings for QgsMapSettings works
  QgsMapSettings ms;
  QgsLabelingEngineSettings les;
  les.setMaximumLineCandidatesPerCm( 4 );
  les.setMaximumPolygonCandidatesPerCmSquared( 8.0 );
  ms.setLabelingEngineSettings( les );
  QCOMPARE( ms.labelingEngineSettings().maximumLineCandidatesPerCm(), 4.0 );
  QCOMPARE( ms.labelingEngineSettings().maximumPolygonCandidatesPerCmSquared(), 8.0 );

  // ensure that setting labeling engine settings also sets text format
  les.setDefaultTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  ms.setLabelingEngineSettings( les );
  QCOMPARE( ms.textRenderFormat(), Qgis::TextRenderFormat::AlwaysText );
  les.setDefaultTextRenderFormat( Qgis::TextRenderFormat::AlwaysOutlines );
  ms.setLabelingEngineSettings( les );
  QCOMPARE( ms.textRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );
  // but we should be able to override this manually
  ms.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  QCOMPARE( ms.textRenderFormat(), Qgis::TextRenderFormat::AlwaysText );
  QCOMPARE( ms.labelingEngineSettings().defaultTextRenderFormat(), Qgis::TextRenderFormat::AlwaysText );
}

void TestQgsMapSettings::visibleExtent()
{
  QgsMapSettings ms;

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,0 : 150,100" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 100 ) );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "0,0 : 100,100" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 50, 100 ) );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "0,-50 : 100,150" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 50, 100 ) );
  ms.setRotation( 90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,0 : 150,100" ) );
  ms.setRotation( -90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,0 : 150,100" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 50 ) );
  ms.setOutputSize( QSize( 50, 100 ) );
  ms.setRotation( 0 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "0,-75 : 100,125" ) );
  ms.setRotation( 90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,-25 : 150,75" ) );
  ms.setRotation( -90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,-25 : 150,75" ) );
  ms.setRotation( 45 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-56,-81 : 156,131" ) );
}

void TestQgsMapSettings::extentBuffer()
{
  QgsMapSettings ms;
  ms.setExtent( QgsRectangle( 50, 50, 100, 100 ) );
  ms.setOutputSize( QSize( 50, 50 ) );
  ms.setExtentBuffer( 10 );
  QgsRectangle visibleExtent = ms.visibleExtent();
  visibleExtent.grow( ms.extentBuffer() );
  QCOMPARE( visibleExtent.toString( 0 ), QString( "40,40 : 110,110" ) );
}

void TestQgsMapSettings::mapUnitsPerPixel()
{
  QgsMapSettings ms;
  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );

  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 2.0 );

  ms.setOutputSize( QSize( 100, 100 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 1.0 );

  ms.setOutputSize( QSize( 50, 100 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 2.0 );

  ms.setOutputSize( QSize( 5000, 1000 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 0.1 );

  ms.setOutputSize( QSize( 1000, 500 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 0.2 );
}

void TestQgsMapSettings::testDevicePixelRatio()
{
  QgsMapSettings ms;
  ms.setOutputSize( QSize( 100, 50 ) );
  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setDevicePixelRatio( 1 );
  const double scale = ms.scale();
  ms.setDevicePixelRatio( 1.5 );
  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  QCOMPARE( ms.outputSize() * 1.5, ms.deviceOutputSize() );
  QCOMPARE( scale, ms.scale() );
}

void TestQgsMapSettings::visiblePolygon()
{
  QgsMapSettings ms;

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( toString( ms.visiblePolygon() ), QString( "-50 100,150 100,150 0,-50 0" ) );

  ms.setExtent( QgsRectangle( 0, -50, 100, 0 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  ms.setRotation( 90 );
  QCOMPARE( toString( ms.visiblePolygon() ), QString( "25 -75,25 25,75 25,75 -75" ) );
  ms.setRotation( -90 );
  QCOMPARE( toString( ms.visiblePolygon() ), QString( "75 25,75 -75,25 -75,25 25" ) );
  ms.setRotation( 30 );
  QCOMPARE( toString( ms.visiblePolygon() ), QString( "-5.8 -28.34,80.8 21.65,105.8 -21.65,19.19 -71.65" ) );
  ms.setRotation( -30 );
  QCOMPARE( toString( ms.visiblePolygon() ), QString( "19.19 21.65,105.8 -28.34,80.8 -71.65,-5.8 -21.65" ) );
  ms.setRotation( 45 );
  QCOMPARE( toString( ms.visiblePolygon() ), QString( "-3.03 -42.67,67.67 28.03,103.03 -7.32,32.32 -78.03" ) );
  ms.setRotation( -45 );
  QCOMPARE( toString( ms.visiblePolygon() ), QString( "32.32 28.03,103.03 -42.67,67.67 -78.03,-3.03 -7.32" ) );
}

void TestQgsMapSettings::visiblePolygonWithBuffer()
{
  QgsMapSettings ms;

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ), QString( "-50 100,150 100,150 0,-50 0" ) );

  ms.setExtentBuffer( 10 );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ), QString( "-70 120,170 120,170 -20,-70 -20" ) );

  ms.setExtent( QgsRectangle( 0, -50, 100, 0 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  ms.setRotation( 90 );
  ms.setExtentBuffer( 0 );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ), QString( "25 -75,25 25,75 25,75 -75" ) );

  ms.setExtentBuffer( 10 );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ), QString( "15 -85,15 35,85 35,85 -85" ) );
}

void TestQgsMapSettings::testIsLayerVisible()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( u"Point"_s, u"a"_s, u"memory"_s );
  QgsVectorLayer *vlB = new QgsVectorLayer( u"Point"_s, u"b"_s, u"memory"_s );
  QgsVectorLayer *vlC = new QgsVectorLayer( u"Point"_s, u"c"_s, u"memory"_s );
  vlC->setScaleBasedVisibility( true );
  vlC->setMinimumScale( 100 );
  vlC->setMaximumScale( 0 );

  QList<QgsMapLayer *> layers;
  layers << vlA << vlB << vlC;

  QgsProject::instance()->addMapLayers( layers );

  QgsMapSettings ms;
  ms.setLayers( layers );
  ms.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3148"_s ) );
  ms.setExtent( QgsRectangle( 100, 100, 200, 200 ) );
  ms.setOutputSize( QSize( 100, 100 ) ); // this results in a scale roughly equal to 3779

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::mapSettingsScope( ms );

  // test checking for visible layer by id
  QgsExpression e( u"is_layer_visible( '%1' )"_s.arg( vlA->id() ) );
  QVariant r = e.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer by direct map layer object
  QgsExpression e4( u"is_layer_visible(array_get( @map_layers, 0 ) )"_s );
  r = e4.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer by name
  QgsExpression e2( u"is_layer_visible( '%1' )"_s.arg( vlB->name() ) );
  r = e2.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer taking into account scale-based visibility
  QgsExpression e5( u"is_layer_visible( '%1' )"_s.arg( vlC->name() ) );
  r = e5.evaluate( &context );
  QCOMPARE( r.toBool(), false );
  vlC->setMinimumScale( 100000 );
  QgsExpressionContext context2;
  context2 << QgsExpressionContextUtils::mapSettingsScope( ms );
  QgsExpression e6( u"is_layer_visible( '%1' )"_s.arg( vlC->name() ) );
  r = e6.evaluate( &context2 );
  QCOMPARE( r.toBool(), true );
  vlC->setMinimumScale( 0 );
  vlC->setMaximumScale( 5000 );
  QgsExpressionContext context3;
  context3 << QgsExpressionContextUtils::mapSettingsScope( ms );
  QgsExpression e7( u"is_layer_visible( '%1' )"_s.arg( vlC->name() ) );
  r = e7.evaluate( &context3 );
  QCOMPARE( r.toBool(), false );
  vlC->setMaximumScale( 200 );
  QgsExpressionContext context4;
  context4 << QgsExpressionContextUtils::mapSettingsScope( ms );
  QgsExpression e8( u"is_layer_visible( '%1' )"_s.arg( vlC->name() ) );
  r = e8.evaluate( &context4 );
  QCOMPARE( r.toBool(), true );

  // test checking for non-existent layer
  QgsExpression e3( u"is_layer_visible( 'non matching name' )"_s );
  r = e3.evaluate( &context );
  QCOMPARE( r.toBool(), false );

  QgsProject::instance()->removeMapLayer( vlA );
  e.prepare( &context );
  r = e.evaluate( &context );
  QCOMPARE( r.toBool(), false ); // layer is deleted
  e2.prepare( &context );
  r = e2.evaluate( &context );
  QCOMPARE( r.toBool(), true ); // layer still exists

  QgsProject::instance()->removeMapLayer( vlB );
  e2.prepare( &context );
  r = e2.evaluate( &context );
  QCOMPARE( r.toBool(), false ); // layer is deleted
}

void TestQgsMapSettings::testMapLayerListUtils()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( u"Point"_s, u"a"_s, u"memory"_s );
  QgsVectorLayer *vlB = new QgsVectorLayer( u"Point"_s, u"b"_s, u"memory"_s );

  QList<QgsMapLayer *> listRawSource;
  listRawSource << vlA << vlB;

  QgsMapLayer *l = _qgis_findLayer( listRawSource, u"a"_s );
  QCOMPARE( l, vlA );

  l = _qgis_findLayer( listRawSource, u"z"_s );
  QCOMPARE( !l, true );

  QgsWeakMapLayerPointerList listQPointer = _qgis_listRawToQPointer( listRawSource );

  QCOMPARE( listQPointer.count(), 2 );
  QCOMPARE( listQPointer[0].data(), vlA );
  QCOMPARE( listQPointer[1].data(), vlB );

  QList<QgsMapLayer *> listRaw = _qgis_listQPointerToRaw( listQPointer );

  QCOMPARE( listRaw.count(), 2 );
  QCOMPARE( listRaw[0], vlA );
  QCOMPARE( listRaw[1], vlB );

  QStringList listIDs = _qgis_listQPointerToIDs( listQPointer );

  QCOMPARE( listIDs.count(), 2 );
  QCOMPARE( listIDs[0], vlA->id() );
  QCOMPARE( listIDs[1], vlB->id() );

  // now delete one layer!
  // QPointer to vlA must get invalidated
  delete vlA;

  QCOMPARE( listQPointer.count(), 2 ); // still two items but one is invalid

  QList<QgsMapLayer *> listRaw2 = _qgis_listQPointerToRaw( listQPointer );

  QCOMPARE( listRaw2.count(), 1 );
  QCOMPARE( listRaw2[0], vlB );

  QStringList listIDs2 = _qgis_listQPointerToIDs( listQPointer );

  QCOMPARE( listIDs2.count(), 1 );
  QCOMPARE( listIDs2[0], vlB->id() );

  delete vlB;
}

void TestQgsMapSettings::testXmlReadWrite()
{
  //create a test dom element
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s
  );
  QDomDocument doc( documentType );
  QDomElement element = doc.createElement( u"s"_s );

  //create a map settings object
  QgsMapSettings s1;
  s1.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );

  //write to xml
  s1.writeXml( element, doc );

  // read a copy from xml
  QgsMapSettings s2;
  s2.readXml( element );

  QCOMPARE( s2.destinationCrs().authid(), u"EPSG:3111"_s );

  // test writing a map settings without a valid CRS
  element = doc.createElement( u"s"_s );
  s1.setDestinationCrs( QgsCoordinateReferenceSystem() );
  s1.writeXml( element, doc );
  s2.readXml( element );
  QVERIFY( !s2.destinationCrs().isValid() );
}

void TestQgsMapSettings::testSetLayers()
{
  const std::unique_ptr<QgsVectorLayer> vlA = std::make_unique<QgsVectorLayer>( u"Point"_s, u"a"_s, u"memory"_s );
  const std::unique_ptr<QgsVectorLayer> vlB = std::make_unique<QgsVectorLayer>( u"Point"_s, u"b"_s, u"memory"_s );
  const std::unique_ptr<QgsVectorLayer> nonSpatial = std::make_unique<QgsVectorLayer>( u"none"_s, u"a"_s, u"memory"_s );

  QgsMapSettings ms;
  ms.setLayers( QList<QgsMapLayer *>() << vlA.get() );
  QCOMPARE( ms.layers(), QList<QgsMapLayer *>() << vlA.get() );
  ms.setLayers( QList<QgsMapLayer *>() << vlB.get() << vlA.get() );
  QCOMPARE( ms.layers(), QList<QgsMapLayer *>() << vlB.get() << vlA.get() );

  // non spatial and null layers should be stripped
  ms.setLayers( QList<QgsMapLayer *>() << vlA.get() << nonSpatial.get() << nullptr << vlB.get() );
  QCOMPARE( ms.layers(), QList<QgsMapLayer *>() << vlA.get() << vlB.get() );
}

void TestQgsMapSettings::testLabelBoundary()
{
  QgsMapSettings ms;
  QVERIFY( ms.labelBoundaryGeometry().isNull() );
  ms.setLabelBoundaryGeometry( QgsGeometry::fromWkt( u"Polygon(( 0 0, 1 0, 1 1, 0 1, 0 0 ))"_s ) );
  QCOMPARE( ms.labelBoundaryGeometry().asWkt(), u"Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"_s );
}

void TestQgsMapSettings::testExpressionContext()
{
  QgsMapSettings ms;
  QgsExpressionContext c;
  QVariant r;

  ms.setOutputSize( QSize( 5000, 5000 ) );
  ms.setExtent( QgsRectangle( -1, 0, 2, 2 ) );
  ms.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  ms.setRotation( -32 );
  c << QgsExpressionContextUtils::mapSettingsScope( ms );

  QgsExpression e( u"@map_scale"_s );
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 247990, 10.0 );

  e = QgsExpression( u"@zoom_level"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 10.0 );

  e = QgsExpression( u"@vector_tile_zoom"_s );
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 10.1385606747, 0.0001 );

  // The old $scale function should silently map to @map_scale, so that older projects work without change
  e = QgsExpression( u"$scale"_s );
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 247990, 10.0 );

  // no map settings scope -- $scale is meaningless
  e = QgsExpression( u"$scale"_s );
  r = e.evaluate( nullptr );
  QVERIFY( !r.isValid() );

  e = QgsExpression( u"@map_id"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"canvas"_s );

  e = QgsExpression( u"@map_rotation"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), -32.0 );

  ms.setRotation( 0 );
  c << QgsExpressionContextUtils::mapSettingsScope( ms );

  e = QgsExpression( u"geom_to_wkt( @map_extent )"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"Polygon ((-1 -0.5, 2 -0.5, 2 2.5, -1 2.5, -1 -0.5))"_s );

  e = QgsExpression( u"@map_extent_width"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 3.0 );

  e = QgsExpression( u"@map_extent_height"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 3.0 );

  e = QgsExpression( u"geom_to_wkt( @map_extent_center )"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"Point (0.5 1)"_s );

  e = QgsExpression( u"@map_crs"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"EPSG:4326"_s );

  e = QgsExpression( u"@map_crs_definition"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"+proj=longlat +datum=WGS84 +no_defs"_s );

  e = QgsExpression( u"@map_units"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"degrees"_s );

  e = QgsExpression( u"@map_crs_description"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"WGS 84"_s );

  e = QgsExpression( u"@map_crs_acronym"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"longlat"_s );

  QgsExpression e6a( u"@map_crs_projection"_s );
  r = e6a.evaluate( &c );
  QCOMPARE( r.toString(), QString( "Lat/long (Geodetic alias)" ) );

  e = QgsExpression( u"@map_crs_proj4"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), u"+proj=longlat +datum=WGS84 +no_defs"_s );

  e = QgsExpression( u"@map_crs_wkt"_s );
  r = e.evaluate( &c );
  QVERIFY( r.toString().length() > 15 );

  e = QgsExpression( u"@map_crs_ellipsoid"_s );
  r = e.evaluate( &c );

  QCOMPARE( r.toString(), u"EPSG:7030"_s );

  e = QgsExpression( u"@map_z_range_lower"_s );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );
  e = QgsExpression( u"@map_z_range_upper"_s );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );

  ms.setZRange( QgsDoubleRange( 0.5, 100.5 ) );
  c = QgsExpressionContext();
  c << QgsExpressionContextUtils::mapSettingsScope( ms );
  e = QgsExpression( u"@map_z_range_lower"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 0.5 );
  e = QgsExpression( u"@map_z_range_upper"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 100.5 );

  e = QgsExpression( u"@map_start_time"_s );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );
  e = QgsExpression( u"@map_end_time"_s );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );
  e = QgsExpression( u"@map_interval"_s );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );

  QVERIFY( !c.variable( u"frame_rate"_s ).isValid() );
  QVERIFY( !c.variable( u"frame_number"_s ).isValid() );

  ms.setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2002, 3, 4 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2010, 6, 7 ), QTime( 0, 0, 0 ) ) ) );
  c = QgsExpressionContext();
  c << QgsExpressionContextUtils::mapSettingsScope( ms );
  e = QgsExpression( u"@map_start_time"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDateTime(), QDateTime( QDate( 2002, 3, 4 ), QTime( 0, 0, 0 ) ) );
  e = QgsExpression( u"@map_end_time"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDateTime(), QDateTime( QDate( 2010, 6, 7 ), QTime( 0, 0, 0 ) ) );
  e = QgsExpression( u"@map_interval"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.value<QgsInterval>(), QgsInterval( QDateTime( QDate( 2010, 6, 7 ), QTime( 0, 0, 0 ) ) - QDateTime( QDate( 2002, 3, 4 ), QTime( 0, 0, 0 ) ) ) );

  QVERIFY( !c.variable( u"frame_rate"_s ).isValid() );
  QVERIFY( !c.variable( u"frame_number"_s ).isValid() );

  ms.setFrameRate( 30 );
  ms.setCurrentFrame( 5 );
  c = QgsExpressionContext();
  c << QgsExpressionContextUtils::mapSettingsScope( ms );
  QVERIFY( c.variable( u"frame_rate"_s ).isValid() );
  QVERIFY( c.variable( u"frame_number"_s ).isValid() );
  e = QgsExpression( u"@frame_rate"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 30.0 );
  e = QgsExpression( u"@frame_number"_s );
  r = e.evaluate( &c );
  QCOMPARE( r.toLongLong(), 5LL );
}

void TestQgsMapSettings::testRenderedFeatureHandlers()
{
  const std::unique_ptr<TestHandler> testHandler = std::make_unique<TestHandler>();
  const std::unique_ptr<TestHandler> testHandler2 = std::make_unique<TestHandler>();

  auto mapSettings = std::make_unique<QgsMapSettings>();
  QVERIFY( mapSettings->renderedFeatureHandlers().isEmpty() );
  mapSettings->addRenderedFeatureHandler( testHandler.get() );
  mapSettings->addRenderedFeatureHandler( testHandler2.get() );
  QCOMPARE( mapSettings->renderedFeatureHandlers(), QList<QgsRenderedFeatureHandlerInterface *>() << testHandler.get() << testHandler2.get() );

  //ownership should NOT be transferred, i.e. it won't delete the registered handlers upon QgsMapSettings destruction
  mapSettings.reset();
  // should be no double-delete here
}

void TestQgsMapSettings::testCustomRenderingFlags()
{
  QgsMapSettings settings;
  settings.setCustomRenderingFlag( u"myexport"_s, true );
  settings.setCustomRenderingFlag( u"omitgeometries"_s, u"points"_s );
  QVERIFY( settings.customRenderingFlags()[u"myexport"_s].toBool() == true );
  QVERIFY( settings.customRenderingFlags()[u"omitgeometries"_s].toString() == "points"_L1 );

  // Test deprecated API
  Q_NOWARN_DEPRECATED_PUSH
  settings.setCustomRenderFlags( u"myexport;omitpoints"_s );
  QVERIFY( settings.customRenderFlags().split( ";" ).contains( u"myexport"_s ) );
  QVERIFY( settings.customRenderFlags().split( ";" ).contains( u"omitpoints"_s ) );
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsMapSettings::testClippingRegions()
{
  QgsMapSettings settings;
  QVERIFY( settings.clippingRegions().isEmpty() );

  const QgsMapClippingRegion region( QgsGeometry::fromWkt( u"Polygon(( 0 0, 1 0 , 1 1 , 0 1, 0 0 ))"_s ) );
  settings.addClippingRegion( region );
  QCOMPARE( settings.clippingRegions().size(), 1 );
  QCOMPARE( settings.clippingRegions().at( 0 ).geometry().asWkt(), u"Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"_s );
  const QgsMapClippingRegion region2( QgsGeometry::fromWkt( u"Polygon(( 10 0, 11 0 , 11 1 , 10 1, 10 0 ))"_s ) );
  settings.addClippingRegion( region2 );
  QCOMPARE( settings.clippingRegions().size(), 2 );
  QCOMPARE( settings.clippingRegions().at( 0 ).geometry().asWkt(), u"Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"_s );
  QCOMPARE( settings.clippingRegions().at( 1 ).geometry().asWkt(), u"Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))"_s );

  const QgsMapSettings settings2( settings );
  QCOMPARE( settings2.clippingRegions().size(), 2 );
  QCOMPARE( settings2.clippingRegions().at( 0 ).geometry().asWkt(), u"Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"_s );
  QCOMPARE( settings2.clippingRegions().at( 1 ).geometry().asWkt(), u"Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))"_s );

  QgsMapSettings settings3;
  settings3 = settings;
  QCOMPARE( settings3.clippingRegions().size(), 2 );
  QCOMPARE( settings3.clippingRegions().at( 0 ).geometry().asWkt(), u"Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"_s );
  QCOMPARE( settings3.clippingRegions().at( 1 ).geometry().asWkt(), u"Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))"_s );

  settings.setClippingRegions( QList<QgsMapClippingRegion>() << region2 );
  QCOMPARE( settings.clippingRegions().size(), 1 );
  QCOMPARE( settings.clippingRegions().at( 0 ).geometry().asWkt(), u"Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))"_s );
}

void TestQgsMapSettings::testScale()
{
  QgsMapSettings settings;
  settings.setOutputSize( QSize( 1000, 1000 ) );
  settings.setOutputDpi( 96 );

  // projected CRS
  settings.setExtent( QgsRectangle( 10346011, -5940976, 19905957, -578962 ) );
  settings.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  QGSCOMPARENEAR( settings.scale(), 36132079, 10000 );
  // method should not impact calculations, we are using a projected CRS
  settings.setScaleMethod( Qgis::ScaleCalculationMethod::HorizontalTop );
  QGSCOMPARENEAR( settings.scale(), 36132079, 10000 );
  settings.setScaleMethod( Qgis::ScaleCalculationMethod::HorizontalBottom );
  QGSCOMPARENEAR( settings.scale(), 36132079, 10000 );
  settings.setScaleMethod( Qgis::ScaleCalculationMethod::AtEquator );
  QGSCOMPARENEAR( settings.scale(), 36132079, 10000 );

  // geographic CRS
  settings.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  settings.setExtent( QgsRectangle( -49.42, 102.1, -4.44, 161.56 ) );

  settings.setScaleMethod( Qgis::ScaleCalculationMethod::HorizontalMiddle );
  QGSCOMPARENEAR( settings.scale(), 11614145, 10000 );
  settings.setScaleMethod( Qgis::ScaleCalculationMethod::HorizontalTop );
  QGSCOMPARENEAR( settings.scale(), 19779538, 10000 );
  settings.setScaleMethod( Qgis::ScaleCalculationMethod::HorizontalBottom );
  QGSCOMPARENEAR( settings.scale(), 3372951, 10000 );
  settings.setScaleMethod( Qgis::ScaleCalculationMethod::HorizontalAverage );
  QGSCOMPARENEAR( settings.scale(), 11588878, 10000 );
  settings.setScaleMethod( Qgis::ScaleCalculationMethod::AtEquator );
  QGSCOMPARENEAR( settings.scale(), 24851905, 10000 );
}

void TestQgsMapSettings::testComputeExtentForScale()
{
  QgsMapSettings settings;
  settings.setExtent( QgsRectangle( -500., -500., 500., 500. ) ); // Just to ensure settings are valid
  settings.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );

  settings.setOutputSize( QSize( 1000, 1000 ) );

  const QgsRectangle rect = settings.computeExtentForScale( QgsPoint( 0, 0 ), 500 );

  //                   [                   output width in inches                   ] * [scale]
  const double widthInches = settings.outputSize().width() / double( settings.outputDpi() ) * 500;
  const double widthMapUnits = widthInches * QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit::Feet, settings.mapUnits() ) / 12;
  QGSCOMPARENEARRECTANGLE( rect, QgsRectangle( -0.5 * widthMapUnits, -0.5 * widthMapUnits, 0.5 * widthMapUnits, 0.5 * widthMapUnits ), 0.0001 );
}

void TestQgsMapSettings::testComputeScaleForExtent()
{
  QgsMapSettings settings;
  settings.setExtent( QgsRectangle( -500., -500., 500., 500. ) ); // Just to ensure settings are valid
  settings.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );

  settings.setOutputSize( QSize( 1000, 1000 ) );

  const double scale = settings.computeScaleForExtent( QgsRectangle( -500., -500., 500., 500. ) );

  const double widthInches = 1000 * QgsUnitTypes::fromUnitToUnitFactor( settings.mapUnits(), Qgis::DistanceUnit::Feet ) * 12;
  const double testScale = widthInches * settings.outputDpi() / double( settings.outputSize().width() );
  QGSCOMPARENEAR( scale, testScale, 0.001 );
}

void TestQgsMapSettings::testLayersWithGroupLayers()
{
  // test retrieving layers from map settings when a QgsGroupLayer is present
  QgsMapSettings settings;

  auto vlA = std::make_unique<QgsVectorLayer>( u"Point"_s, u"a"_s, u"memory"_s );
  auto vlB = std::make_unique<QgsVectorLayer>( u"Point"_s, u"b"_s, u"memory"_s );
  auto vlC = std::make_unique<QgsVectorLayer>( u"Point"_s, u"c"_s, u"memory"_s );

  QgsGroupLayer::LayerOptions options( ( QgsCoordinateTransformContext() ) );
  QgsGroupLayer groupLayer( u"group"_s, options );
  groupLayer.setChildLayers( { vlB.get(), vlC.get() } );
  settings.setLayers( { vlA.get(), &groupLayer } );

  // without expanding groups
  QCOMPARE( settings.layers().size(), 2 );
  QCOMPARE( settings.layers().at( 0 ), vlA.get() );
  QCOMPARE( settings.layers().at( 1 ), &groupLayer );

  QCOMPARE( settings.layerIds().size(), 2 );
  QCOMPARE( settings.layerIds().at( 0 ), vlA->id() );
  QCOMPARE( settings.layerIds().at( 1 ), groupLayer.id() );

  // with expanding groups
  QCOMPARE( settings.layers( true ).size(), 3 );
  QCOMPARE( settings.layers( true ).at( 0 ), vlA.get() );
  QCOMPARE( settings.layers( true ).at( 1 ), vlB.get() );
  QCOMPARE( settings.layers( true ).at( 2 ), vlC.get() );

  QCOMPARE( settings.layerIds( true ).size(), 3 );
  QCOMPARE( settings.layerIds( true ).at( 0 ), vlA->id() );
  QCOMPARE( settings.layerIds( true ).at( 1 ), vlB->id() );
  QCOMPARE( settings.layerIds( true ).at( 2 ), vlC->id() );
}

void TestQgsMapSettings::testMaskRenderSettings()
{
  QgsMapSettings settings;
  settings.maskSettings().setSimplificationTolerance( 10 );
  QCOMPARE( settings.maskSettings().simplifyTolerance(), 10 );

  QgsMaskRenderSettings maskSettings;
  maskSettings.setSimplificationTolerance( 11 );
  settings.setMaskSettings( maskSettings );
  QCOMPARE( settings.maskSettings().simplifyTolerance(), 11 );

  QgsMapSettings settings2 = settings;
  QCOMPARE( settings2.maskSettings().simplifyTolerance(), 11 );

  QgsMapSettings settings3( settings );
  QCOMPARE( settings3.maskSettings().simplifyTolerance(), 11 );
}

void TestQgsMapSettings::testDeprecatedFlagsRasterizePolicy()
{
  QgsMapSettings settings;

  // test translation of rasterize policies to flags
  settings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::ForceVector );
  QVERIFY( settings.testFlag( Qgis::MapSettingsFlag::ForceVectorOutput ) );
  QVERIFY( !settings.testFlag( Qgis::MapSettingsFlag::UseAdvancedEffects ) );

  settings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
  QVERIFY( settings.testFlag( Qgis::MapSettingsFlag::ForceVectorOutput ) );
  QVERIFY( settings.testFlag( Qgis::MapSettingsFlag::UseAdvancedEffects ) );

  settings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
  QVERIFY( !settings.testFlag( Qgis::MapSettingsFlag::ForceVectorOutput ) );
  QVERIFY( settings.testFlag( Qgis::MapSettingsFlag::UseAdvancedEffects ) );

  settings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
  QCOMPARE( settings.rasterizedRenderingPolicy(), Qgis::RasterizedRenderingPolicy::PreferVector );
  settings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, false );
  QCOMPARE( settings.rasterizedRenderingPolicy(), Qgis::RasterizedRenderingPolicy::Default );

  settings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
  settings.setFlag( Qgis::MapSettingsFlag::UseAdvancedEffects, false );
  QCOMPARE( settings.rasterizedRenderingPolicy(), Qgis::RasterizedRenderingPolicy::ForceVector );
}

QGSTEST_MAIN( TestQgsMapSettings )
#include "testqgsmapsettings.moc"
