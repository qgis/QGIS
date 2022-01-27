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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <cmath>

//header for class being tested
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgsrectangle.h"
#include "qgsmapsettings.h"
#include "qgspointxy.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsrendercontext.h"
#include "qgsgrouplayer.h"

class TestHandler : public QgsRenderedFeatureHandlerInterface
{
  public:

    void handleRenderedFeature( const QgsFeature &, const QgsGeometry &, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext & ) override {}

};

class TestQgsMapSettings: public QObject
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
    void testComputeExtentForScale();
    void testComputeScaleForExtent();
    void testLayersWithGroupLayers();

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
    s += QStringLiteral( "%1%2 %3" ).arg( sep )
         .arg( int( p[i].x() * r ) / r )
         .arg( int( p[i].y() * r ) / r );
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
  QCOMPARE( ms.simplifyMethod().simplifyHints(), QgsVectorSimplifyMethod::NoSimplification );
  QgsVectorSimplifyMethod simplify;
  simplify.setSimplifyHints( QgsVectorSimplifyMethod::GeometrySimplification );
  ms.setSimplifyMethod( simplify );
  QCOMPARE( ms.simplifyMethod().simplifyHints(), QgsVectorSimplifyMethod::GeometrySimplification );

  QVERIFY( ms.zRange().isInfinite() );
  ms.setZRange( QgsDoubleRange( 1, 10 ) );
  QCOMPARE( ms.zRange(), QgsDoubleRange( 1, 10 ) );
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
  QCOMPARE( scale * 1.5, ms.scale() );
}

void TestQgsMapSettings::visiblePolygon()
{
  QgsMapSettings ms;

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "-50 100,150 100,150 0,-50 0" ) );

  ms.setExtent( QgsRectangle( 0, -50, 100, 0 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  ms.setRotation( 90 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "25 -75,25 25,75 25,75 -75" ) );
  ms.setRotation( -90 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "75 25,75 -75,25 -75,25 25" ) );
  ms.setRotation( 30 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "-5.8 -28.34,80.8 21.65,105.8 -21.65,19.19 -71.65" ) );
  ms.setRotation( -30 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "19.19 21.65,105.8 -28.34,80.8 -71.65,-5.8 -21.65" ) );
  ms.setRotation( 45 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "-3.03 -42.67,67.67 28.03,103.03 -7.32,32.32 -78.03" ) );
  ms.setRotation( -45 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "32.32 28.03,103.03 -42.67,67.67 -78.03,-3.03 -7.32" ) );
}

void TestQgsMapSettings::visiblePolygonWithBuffer()
{
  QgsMapSettings ms;

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ),
            QString( "-50 100,150 100,150 0,-50 0" ) );

  ms.setExtentBuffer( 10 );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ),
            QString( "-70 120,170 120,170 -20,-70 -20" ) );

  ms.setExtent( QgsRectangle( 0, -50, 100, 0 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  ms.setRotation( 90 );
  ms.setExtentBuffer( 0 );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ),
            QString( "25 -75,25 25,75 25,75 -75" ) );

  ms.setExtentBuffer( 10 );
  QCOMPARE( toString( ms.visiblePolygonWithBuffer() ),
            QString( "15 -85,15 35,85 35,85 -85" ) );
}

void TestQgsMapSettings::testIsLayerVisible()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "b" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vlC = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "c" ), QStringLiteral( "memory" ) );
  vlC->setScaleBasedVisibility( true );
  vlC->setMinimumScale( 100 );
  vlC->setMaximumScale( 0 );

  QList<QgsMapLayer *> layers;
  layers << vlA << vlB << vlC;

  QgsProject::instance()->addMapLayers( layers );

  QgsMapSettings ms;
  ms.setLayers( layers );
  ms.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3148" ) ) );
  ms.setExtent( QgsRectangle( 100, 100, 200, 200 ) );
  ms.setOutputSize( QSize( 100, 100 ) ); // this results in a scale roughly equal to 3779

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::mapSettingsScope( ms );

  // test checking for visible layer by id
  QgsExpression e( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlA->id() ) );
  QVariant r = e.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer by direct map layer object
  QgsExpression e4( QStringLiteral( "is_layer_visible(array_get( @map_layers, 0 ) )" ) );
  r = e4.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer by name
  QgsExpression e2( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlB->name() ) );
  r = e2.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer taking into account scale-based visibility
  QgsExpression e5( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlC->name() ) );
  r = e5.evaluate( &context );
  QCOMPARE( r.toBool(), false );
  vlC->setMinimumScale( 100000 );
  QgsExpressionContext context2;
  context2 << QgsExpressionContextUtils::mapSettingsScope( ms );
  QgsExpression e6( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlC->name() ) );
  r = e6.evaluate( &context2 );
  QCOMPARE( r.toBool(), true );
  vlC->setMinimumScale( 0 );
  vlC->setMaximumScale( 5000 );
  QgsExpressionContext context3;
  context3 << QgsExpressionContextUtils::mapSettingsScope( ms );
  QgsExpression e7( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlC->name() ) );
  r = e7.evaluate( &context3 );
  QCOMPARE( r.toBool(), false );
  vlC->setMaximumScale( 200 );
  QgsExpressionContext context4;
  context4 << QgsExpressionContextUtils::mapSettingsScope( ms );
  QgsExpression e8( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlC->name() ) );
  r = e8.evaluate( &context4 );
  QCOMPARE( r.toBool(), true );

  // test checking for non-existent layer
  QgsExpression e3( QStringLiteral( "is_layer_visible( 'non matching name' )" ) );
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
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "b" ), QStringLiteral( "memory" ) );

  QList<QgsMapLayer *> listRawSource;
  listRawSource << vlA << vlB;

  QgsMapLayer *l = _qgis_findLayer( listRawSource, QStringLiteral( "a" ) );
  QCOMPARE( l, vlA );

  l = _qgis_findLayer( listRawSource, QStringLiteral( "z" ) );
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

  QCOMPARE( listQPointer.count(), 2 );  // still two items but one is invalid

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
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement element = doc.createElement( QStringLiteral( "s" ) );

  //create a map settings object
  QgsMapSettings s1;
  s1.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );

  //write to xml
  s1.writeXml( element, doc );

  // read a copy from xml
  QgsMapSettings s2;
  s2.readXml( element );

  QCOMPARE( s2.destinationCrs().authid(), QStringLiteral( "EPSG:3111" ) );

  // test writing a map settings without a valid CRS
  element = doc.createElement( QStringLiteral( "s" ) );
  s1.setDestinationCrs( QgsCoordinateReferenceSystem() );
  s1.writeXml( element, doc );
  s2.readXml( element );
  QVERIFY( !s2.destinationCrs().isValid() );
}

void TestQgsMapSettings::testSetLayers()
{
  const std::unique_ptr<  QgsVectorLayer  > vlA = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );
  const std::unique_ptr<  QgsVectorLayer  > vlB = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "b" ), QStringLiteral( "memory" ) );
  const std::unique_ptr<  QgsVectorLayer  > nonSpatial = std::make_unique< QgsVectorLayer >( QStringLiteral( "none" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );

  QgsMapSettings ms;
  ms.setLayers( QList< QgsMapLayer * >() << vlA.get() );
  QCOMPARE( ms.layers(), QList< QgsMapLayer * >() << vlA.get() );
  ms.setLayers( QList< QgsMapLayer * >() << vlB.get() << vlA.get() );
  QCOMPARE( ms.layers(), QList< QgsMapLayer * >() << vlB.get() << vlA.get() );

  // non spatial and null layers should be stripped
  ms.setLayers( QList< QgsMapLayer * >() << vlA.get() << nonSpatial.get() << nullptr << vlB.get() );
  QCOMPARE( ms.layers(), QList< QgsMapLayer * >() << vlA.get() << vlB.get() );
}

void TestQgsMapSettings::testLabelBoundary()
{
  QgsMapSettings ms;
  QVERIFY( ms.labelBoundaryGeometry().isNull() );
  ms.setLabelBoundaryGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon(( 0 0, 1 0, 1 1, 0 1, 0 0 ))" ) ) );
  QCOMPARE( ms.labelBoundaryGeometry().asWkt(), QStringLiteral( "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))" ) );
}

void TestQgsMapSettings::testExpressionContext()
{
  QgsMapSettings ms;
  QgsExpressionContext c;
  QVariant r;

  ms.setOutputSize( QSize( 5000, 5000 ) );
  ms.setExtent( QgsRectangle( -1, 0, 2, 2 ) );
  ms.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  ms.setRotation( -32 );
  c << QgsExpressionContextUtils::mapSettingsScope( ms );

  QgsExpression e( QStringLiteral( "@map_scale" ) );
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 247990, 10.0 );

  e = QgsExpression( QStringLiteral( "@zoom_level" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 10.0 );

  e = QgsExpression( QStringLiteral( "@vector_tile_zoom" ) );
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 10.1385606747, 0.0001 );

  // The old $scale function should silently map to @map_scale, so that older projects work without change
  e = QgsExpression( QStringLiteral( "$scale" ) );
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 247990, 10.0 );

  // no map settings scope -- $scale is meaningless
  e = QgsExpression( QStringLiteral( "$scale" ) );
  r = e.evaluate( nullptr );
  QVERIFY( !r.isValid() );

  e = QgsExpression( QStringLiteral( "@map_id" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "canvas" ) );

  e = QgsExpression( QStringLiteral( "@map_rotation" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), -32.0 );

  ms.setRotation( 0 );
  c << QgsExpressionContextUtils::mapSettingsScope( ms );

  e = QgsExpression( QStringLiteral( "geom_to_wkt( @map_extent )" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "Polygon ((-1 -0.5, 2 -0.5, 2 2.5, -1 2.5, -1 -0.5))" ) );

  e = QgsExpression( QStringLiteral( "@map_extent_width" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 3.0 );

  e = QgsExpression( QStringLiteral( "@map_extent_height" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toDouble(), 3.0 );

  e = QgsExpression( QStringLiteral( "geom_to_wkt( @map_extent_center )" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "Point (0.5 1)" ) );

  e = QgsExpression( QStringLiteral( "@map_crs" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "EPSG:4326" ) );

  e = QgsExpression( QStringLiteral( "@map_crs_definition" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "+proj=longlat +datum=WGS84 +no_defs" ) );

  e = QgsExpression( QStringLiteral( "@map_units" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "degrees" ) );

  e = QgsExpression( QStringLiteral( "@map_crs_description" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "WGS 84" ) );

  e = QgsExpression( QStringLiteral( "@map_crs_acronym" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "longlat" ) );

  QgsExpression e6a( QStringLiteral( "@map_crs_projection" ) );
  r = e6a.evaluate( &c );
  QCOMPARE( r.toString(), QString( "Lat/long (Geodetic alias)" ) );

  e = QgsExpression( QStringLiteral( "@map_crs_proj4" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "+proj=longlat +datum=WGS84 +no_defs" ) );

  e = QgsExpression( QStringLiteral( "@map_crs_wkt" ) );
  r = e.evaluate( &c );
  QVERIFY( r.toString().length() > 15 );

  e = QgsExpression( QStringLiteral( "@map_crs_ellipsoid" ) );
  r = e.evaluate( &c );

  QCOMPARE( r.toString(), QStringLiteral( "EPSG:7030" ) );

  e = QgsExpression( QStringLiteral( "@map_start_time" ) );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );
  e = QgsExpression( QStringLiteral( "@map_end_time" ) );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );
  e = QgsExpression( QStringLiteral( "@map_interval" ) );
  r = e.evaluate( &c );
  QVERIFY( !r.isValid() );

  ms.setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2002, 3, 4 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2010, 6, 7 ), QTime( 0, 0, 0 ) ) ) );
  c = QgsExpressionContext();
  c << QgsExpressionContextUtils::mapSettingsScope( ms );
  e = QgsExpression( QStringLiteral( "@map_start_time" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toDateTime(), QDateTime( QDate( 2002, 3, 4 ), QTime( 0, 0, 0 ) ) );
  e = QgsExpression( QStringLiteral( "@map_end_time" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toDateTime(), QDateTime( QDate( 2010, 6, 7 ), QTime( 0, 0, 0 ) ) );
  e = QgsExpression( QStringLiteral( "@map_interval" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.value< QgsInterval >(), QgsInterval( QDateTime( QDate( 2010, 6, 7 ), QTime( 0, 0, 0 ) ) - QDateTime( QDate( 2002, 3, 4 ), QTime( 0, 0, 0 ) ) ) );
}

void TestQgsMapSettings::testRenderedFeatureHandlers()
{
  const std::unique_ptr< TestHandler > testHandler = std::make_unique< TestHandler >();
  const std::unique_ptr< TestHandler > testHandler2 = std::make_unique< TestHandler >();

  std::unique_ptr< QgsMapSettings> mapSettings = std::make_unique< QgsMapSettings >();
  QVERIFY( mapSettings->renderedFeatureHandlers().isEmpty() );
  mapSettings->addRenderedFeatureHandler( testHandler.get() );
  mapSettings->addRenderedFeatureHandler( testHandler2.get() );
  QCOMPARE( mapSettings->renderedFeatureHandlers(), QList< QgsRenderedFeatureHandlerInterface * >() << testHandler.get() << testHandler2.get() );

  //ownership should NOT be transferred, i.e. it won't delete the registered handlers upon QgsMapSettings destruction
  mapSettings.reset();
  // should be no double-delete here
}

void TestQgsMapSettings::testCustomRenderingFlags()
{
  QgsMapSettings settings;
  settings.setCustomRenderingFlag( QStringLiteral( "myexport" ), true );
  settings.setCustomRenderingFlag( QStringLiteral( "omitgeometries" ), QStringLiteral( "points" ) );
  QVERIFY( settings.customRenderingFlags()[ QStringLiteral( "myexport" ) ].toBool() == true );
  QVERIFY( settings.customRenderingFlags()[ QStringLiteral( "omitgeometries" ) ].toString() == QLatin1String( "points" ) );

  // Test deprecated API
  Q_NOWARN_DEPRECATED_PUSH
  settings.setCustomRenderFlags( QStringLiteral( "myexport;omitpoints" ) );
  QVERIFY( settings.customRenderFlags().split( ";" ).contains( QStringLiteral( "myexport" ) ) );
  QVERIFY( settings.customRenderFlags().split( ";" ).contains( QStringLiteral( "omitpoints" ) ) );
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsMapSettings::testClippingRegions()
{
  QgsMapSettings settings;
  QVERIFY( settings.clippingRegions().isEmpty() );

  const QgsMapClippingRegion region( QgsGeometry::fromWkt( QStringLiteral( "Polygon(( 0 0, 1 0 , 1 1 , 0 1, 0 0 ))" ) ) );
  settings.addClippingRegion( region );
  QCOMPARE( settings.clippingRegions().size(), 1 );
  QCOMPARE( settings.clippingRegions().at( 0 ).geometry().asWkt(), QStringLiteral( "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))" ) );
  const QgsMapClippingRegion region2( QgsGeometry::fromWkt( QStringLiteral( "Polygon(( 10 0, 11 0 , 11 1 , 10 1, 10 0 ))" ) ) );
  settings.addClippingRegion( region2 );
  QCOMPARE( settings.clippingRegions().size(), 2 );
  QCOMPARE( settings.clippingRegions().at( 0 ).geometry().asWkt(), QStringLiteral( "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))" ) );
  QCOMPARE( settings.clippingRegions().at( 1 ).geometry().asWkt(), QStringLiteral( "Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))" ) );

  const QgsMapSettings settings2( settings );
  QCOMPARE( settings2.clippingRegions().size(), 2 );
  QCOMPARE( settings2.clippingRegions().at( 0 ).geometry().asWkt(), QStringLiteral( "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))" ) );
  QCOMPARE( settings2.clippingRegions().at( 1 ).geometry().asWkt(), QStringLiteral( "Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))" ) );

  QgsMapSettings settings3;
  settings3 = settings;
  QCOMPARE( settings3.clippingRegions().size(), 2 );
  QCOMPARE( settings3.clippingRegions().at( 0 ).geometry().asWkt(), QStringLiteral( "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))" ) );
  QCOMPARE( settings3.clippingRegions().at( 1 ).geometry().asWkt(), QStringLiteral( "Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))" ) ) ;

  settings.setClippingRegions( QList< QgsMapClippingRegion >() << region2 );
  QCOMPARE( settings.clippingRegions().size(), 1 );
  QCOMPARE( settings.clippingRegions().at( 0 ).geometry().asWkt(), QStringLiteral( "Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))" ) ) ;
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
  const double widthMapUnits = widthInches * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::DistanceFeet, settings.mapUnits() ) / 12;
  QGSCOMPARENEARRECTANGLE( rect, QgsRectangle( - 0.5 * widthMapUnits, - 0.5 * widthMapUnits, 0.5 * widthMapUnits, 0.5 * widthMapUnits ), 0.0001 );

}

void TestQgsMapSettings::testComputeScaleForExtent()
{
  QgsMapSettings settings;
  settings.setExtent( QgsRectangle( -500., -500., 500., 500. ) ); // Just to ensure settings are valid
  settings.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );

  settings.setOutputSize( QSize( 1000, 1000 ) );

  const double scale = settings.computeScaleForExtent( QgsRectangle( -500., -500., 500., 500. ) );

  const double widthInches = 1000 * QgsUnitTypes::fromUnitToUnitFactor( settings.mapUnits(), QgsUnitTypes::DistanceFeet ) * 12;
  const double testScale = widthInches * settings.outputDpi() / double( settings.outputSize().width() );
  QGSCOMPARENEAR( scale, testScale, 0.001 );
}

void TestQgsMapSettings::testLayersWithGroupLayers()
{
  // test retrieving layers from map settings when a QgsGroupLayer is present
  QgsMapSettings settings;

  std::unique_ptr< QgsVectorLayer > vlA = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );
  std::unique_ptr< QgsVectorLayer > vlB = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "b" ), QStringLiteral( "memory" ) );
  std::unique_ptr< QgsVectorLayer > vlC = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "c" ), QStringLiteral( "memory" ) );

  QgsGroupLayer::LayerOptions options( ( QgsCoordinateTransformContext() ) );
  QgsGroupLayer groupLayer( QStringLiteral( "group" ), options );
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

QGSTEST_MAIN( TestQgsMapSettings )
#include "testqgsmapsettings.moc"
