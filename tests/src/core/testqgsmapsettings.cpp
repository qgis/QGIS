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
#include "qgsmaplayerlistutils.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"

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
    void mapUnitsPerPixel();
    void testDevicePixelRatio();
    void visiblePolygon();
    void testIsLayerVisible();
    void testMapLayerListUtils();
    void testXmlReadWrite();
    void testSetLayers();
    void testLabelBoundary();
    void testExpressionContext();

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
  double r = std::pow( 10.0, dec );
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
  QgsMapSettings ms;
  QCOMPARE( ms.destinationCrs(), QgsCoordinateReferenceSystem() );
}

void TestQgsMapSettings::testGettersSetters()
{
  // basic getter/setter tests
  QgsMapSettings ms;

  ms.setTextRenderFormat( QgsRenderContext::TextFormatAlwaysText );
  QCOMPARE( ms.textRenderFormat(), QgsRenderContext::TextFormatAlwaysText );
  ms.setTextRenderFormat( QgsRenderContext::TextFormatAlwaysOutlines );
  QCOMPARE( ms.textRenderFormat(), QgsRenderContext::TextFormatAlwaysOutlines );
}

void TestQgsMapSettings::testLabelingEngineSettings()
{
  // test that setting labeling engine settings for QgsMapSettings works
  QgsMapSettings ms;
  QgsLabelingEngineSettings les;
  les.setNumCandidatePositions( 4, 8, 15 ); // 23, 42... ;)
  ms.setLabelingEngineSettings( les );
  int c1, c2, c3;
  ms.labelingEngineSettings().numCandidatePositions( c1, c2, c3 );
  QCOMPARE( c1, 4 );
  QCOMPARE( c2, 8 );
  QCOMPARE( c3, 15 );

  // ensure that setting labeling engine settings also sets text format
  les.setDefaultTextRenderFormat( QgsRenderContext::TextFormatAlwaysText );
  ms.setLabelingEngineSettings( les );
  QCOMPARE( ms.textRenderFormat(), QgsRenderContext::TextFormatAlwaysText );
  les.setDefaultTextRenderFormat( QgsRenderContext::TextFormatAlwaysOutlines );
  ms.setLabelingEngineSettings( les );
  QCOMPARE( ms.textRenderFormat(), QgsRenderContext::TextFormatAlwaysOutlines );
  // but we should be able to override this manually
  ms.setTextRenderFormat( QgsRenderContext::TextFormatAlwaysText );
  QCOMPARE( ms.textRenderFormat(), QgsRenderContext::TextFormatAlwaysText );
  QCOMPARE( ms.labelingEngineSettings().defaultTextRenderFormat(), QgsRenderContext::TextFormatAlwaysText );
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
  double scale = ms.scale();
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

void TestQgsMapSettings::testIsLayerVisible()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "b" ), QStringLiteral( "memory" ) );

  QList<QgsMapLayer *> layers;
  layers << vlA << vlB;

  QgsProject::instance()->addMapLayers( layers );

  QgsMapSettings ms;
  ms.setLayers( layers );
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::mapSettingsScope( ms );

  // test checking for visible layer by id
  QgsExpression e( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlA-> id() ) );
  QVariant r = e.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer by direct map layer object
  QgsExpression e4( QStringLiteral( "is_layer_visible(array_get( @map_layers, 0 ) )" ) );
  r = e4.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for visible layer by name
  QgsExpression e2( QStringLiteral( "is_layer_visible( '%1' )" ).arg( vlB-> name() ) );
  r = e2.evaluate( &context );
  QCOMPARE( r.toBool(), true );

  // test checking for non-existent layer
  QgsExpression e3( QStringLiteral( "is_layer_visible( 'non matching name' )" ) );
  r = e3.evaluate( &context );
  QCOMPARE( r.toBool(), false );

  QgsProject::instance()->removeMapLayer( vlA );
  r = e.evaluate( &context );
  QCOMPARE( r.toBool(), false ); // layer is deleted
  r = e2.evaluate( &context );
  QCOMPARE( r.toBool(), true ); // layer still exists

  QgsProject::instance()->removeMapLayer( vlB );
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
  QDomDocumentType documentType =
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
  std::unique_ptr<  QgsVectorLayer  > vlA = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );
  std::unique_ptr<  QgsVectorLayer  > vlB = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "b" ), QStringLiteral( "memory" ) );
  std::unique_ptr<  QgsVectorLayer  > nonSpatial = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "none" ), QStringLiteral( "a" ), QStringLiteral( "memory" ) );

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

  e = QgsExpression( QStringLiteral( "@map_crs_proj4" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "+proj=longlat +datum=WGS84 +no_defs" ) );

  e = QgsExpression( QStringLiteral( "@map_crs_wkt" ) );
  r = e.evaluate( &c );
  QVERIFY( r.toString().length() > 15 );

  e = QgsExpression( QStringLiteral( "@map_crs_ellipsoid" ) );
  r = e.evaluate( &c );
  QCOMPARE( r.toString(), QStringLiteral( "WGS84" ) );
}

QGSTEST_MAIN( TestQgsMapSettings )
#include "testqgsmapsettings.moc"
