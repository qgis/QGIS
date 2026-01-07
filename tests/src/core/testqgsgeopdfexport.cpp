/***************************************************************************
                         testqgsgeopdfexport.cpp
                         ----------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractgeopdfexporter.h"
#include "qgsapplication.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QObject>

class TestGeospatialPdfExporter : public QgsAbstractGeospatialPdfExporter
{
  private:
    VectorComponentDetail componentDetailForLayerId( const QString &layerId ) override
    {
      Q_UNUSED( layerId );
      VectorComponentDetail detail;
      detail.mapLayerId = layerId;
      detail.name = u"name %1"_s.arg( layerId );
      detail.displayAttribute = u"attr %1"_s.arg( layerId );
      return detail;
    }
};

class TestQgsGeospatialPdfExport : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsGeospatialPdfExport()
      : QgsTest( u"Geospatial PDF Export Testss"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testCollectingFeatures();
    void testComposition();
    void testMetadata();
    void testGeoref();
    void testGeorefPolygon();
    void testGroups();
    void testCustomGroups();
    void testGroupOrder();
    void testLayerOrder();
    void compositionMode();
    void testMutuallyExclusiveGroupsLayers();
    void testMutuallyExclusiveGroupsCustom();
};

void TestQgsGeospatialPdfExport::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsGeospatialPdfExport::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsGeospatialPdfExport::testCollectingFeatures()
{
  if ( !QgsAbstractGeospatialPdfExporter::geospatialPDFCreationAvailable() )
  {
    QSKIP( "This test requires geospatial PDF creation abilities", SkipSingle );
  }

  TestGeospatialPdfExporter geospatialPdfExporter;
  // no features, no crash
  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geospatialPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( u"a1"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"a2"_s, QMetaType::Type::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 11 << 22 );
  f.setGeometry( QgsGeometry( new QgsPoint( 3, 4 ) ) );
  renderedBounds = QgsGeometry::fromRect( QgsRectangle( 2, 10, 7, 20 ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(1 1, 2 2)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer2"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geospatialPdfExporter.mVectorComponents.count(), 2 );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == "layer1"_L1 )
    {
      component = it;
      break;
    }
  }
  QCOMPARE( component.mapLayerId, u"layer1"_s );
  QCOMPARE( component.name, u"name layer1"_s );
  // check that temporary layers were correctly written
  auto layer = std::make_unique<QgsVectorLayer>( u"%1|layerName=%2"_s.arg( component.sourceVectorPath, component.sourceVectorLayer ), u"layer"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->featureCount(), 2L );
  QCOMPARE( layer->wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( layer->fields().at( 1 ).name(), u"a1"_s );
  QCOMPARE( layer->fields().at( 2 ).name(), u"a2"_s );
  QgsFeatureIterator it = layer->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attributes().at( 1 ).toInt(), 1 );
  QCOMPARE( f.attributes().at( 2 ).toInt(), 2 );
  QCOMPARE( f.geometry().asWkt(), u"Polygon ((1 10, 6 10, 6 20, 1 20, 1 10))"_s );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attributes().at( 1 ).toInt(), 11 );
  QCOMPARE( f.attributes().at( 2 ).toInt(), 22 );
  QCOMPARE( f.geometry().asWkt(), u"Polygon ((2 10, 7 10, 7 20, 2 20, 2 10))"_s );

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == "layer2"_L1 )
    {
      component = it;
      break;
    }
  }
  QCOMPARE( component.mapLayerId, u"layer2"_s );
  QCOMPARE( component.name, u"name layer2"_s );
  layer = std::make_unique<QgsVectorLayer>( u"%1|layerName=%2"_s.arg( component.sourceVectorPath, component.sourceVectorLayer ), u"layer"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->featureCount(), 1L );
  QCOMPARE( layer->wkbType(), Qgis::WkbType::LineString );
  QCOMPARE( layer->fields().at( 1 ).name(), u"a1"_s );
  QCOMPARE( layer->fields().at( 2 ).name(), u"a2"_s );
  it = layer->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attributes().at( 1 ).toInt(), 31 );
  QCOMPARE( f.attributes().at( 2 ).toInt(), 32 );
  QCOMPARE( f.geometry().asWkt(), u"LineString (1 1, 2 2)"_s );
}

void TestQgsGeospatialPdfExport::testComposition()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // no features, no crash
  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geospatialPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( u"a1"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"a2"_s, QMetaType::Type::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(1 1, 2 2)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer2"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == "layer1"_L1 )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == "layer2"_L1 )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers;
  QgsAbstractGeospatialPdfExporter::ComponentLayerDetail detail;
  detail.mapLayerId = u"layer3"_s;
  detail.name = u"xxx"_s;
  detail.opacity = 0.7;
  detail.compositionMode = QPainter::CompositionMode_Screen;
  detail.sourcePdfPath = u"a pdf.pdf"_s;
  renderedLayers << detail;

  QgsAbstractGeospatialPdfExporter::ExportDetails details;

  details.layerIdToPdfLayerTreeNameMap.insert( u"layer1"_s, u"my first layer"_s );
  details.initialLayerVisibility.insert( u"layer2"_s, false );
  details.layerOrder = QStringList() << u"layer2"_s;
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 3 );

  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"id"_s ), u"layer2"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"name layer2"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"initiallyVisible"_s ), u"false"_s );

  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"id"_s ), u"layer1"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my first layer"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );

  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"id"_s ), u"layer3"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"name"_s ), u"xxx"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );

  QDomNodeList ifLayerOnList = doc.elementsByTagName( u"IfLayerOn"_s );
  QCOMPARE( ifLayerOnList.count(), 3 );

  int layer1Idx = ifLayerOnList.at( 0 ).toElement().attribute( u"layerId"_s ) == "layer1"_L1 ? 0 : ifLayerOnList.at( 1 ).toElement().attribute( u"layerId"_s ) == "layer1"_L1 ? 1
                                                                                                                                                                              : 2;
  int layer2Idx = ifLayerOnList.at( 0 ).toElement().attribute( u"layerId"_s ) == "layer2"_L1 ? 0 : ifLayerOnList.at( 1 ).toElement().attribute( u"layerId"_s ) == "layer2"_L1 ? 1
                                                                                                                                                                              : 2;
  int layer3Idx = ifLayerOnList.at( 0 ).toElement().attribute( u"layerId"_s ) == "layer3"_L1 ? 0 : ifLayerOnList.at( 1 ).toElement().attribute( u"layerId"_s ) == "layer3"_L1 ? 1
                                                                                                                                                                              : 2;
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().attribute( u"layerId"_s ), u"layer1"_s );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer1Path );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer1Layer );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer1"_s );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer1"_s );

  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().attribute( u"layerId"_s ), u"layer2"_s );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer2Path );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer2Layer );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer2"_s );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer2"_s );

  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().attribute( u"layerId"_s ), u"layer3"_s );
  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().elementsByTagName( u"PDF"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), u"a pdf.pdf"_s );
  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().elementsByTagName( u"PDF"_s ).at( 0 ).toElement().elementsByTagName( u"Blending"_s ).at( 0 ).toElement().attribute( u"opacity"_s ).toDouble(), 0.7 );
  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().elementsByTagName( u"PDF"_s ).at( 0 ).toElement().elementsByTagName( u"Blending"_s ).at( 0 ).toElement().attribute( u"function"_s ), u"Screen"_s );
}

void TestQgsGeospatialPdfExport::testMetadata()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // test creation of the composition xml with metadata

  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers;
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( u"Author"_s ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( u"Producer"_s ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( u"Creator"_s ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( u"CreationDate"_s ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( u"Subject"_s ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( u"Title"_s ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( u"Keywords"_s ).count(), 0 );

  // with metadata
  details.author = u"my author"_s;
  details.producer = u"my producer"_s;
  details.creator = u"my creator"_s;
  details.creationDateTime = QDateTime( QDate( 2010, 3, 5 ), QTime( 12, 34 ), Qt::UTC );
  details.subject = u"my subject"_s;
  details.title = u"my title"_s;
  details.keywords.insert( u"k1"_s, QStringList() << u"v1"_s << u"v2"_s );

  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( u"Author"_s ).at( 0 ).toElement().text(), u"my author"_s );
  QCOMPARE( doc.elementsByTagName( u"Producer"_s ).at( 0 ).toElement().text(), u"my producer"_s );
  QCOMPARE( doc.elementsByTagName( u"Creator"_s ).at( 0 ).toElement().text(), u"my creator"_s );
  QCOMPARE( doc.elementsByTagName( u"CreationDate"_s ).at( 0 ).toElement().text(), u"D:20100305123400+0'0'"_s );
  QCOMPARE( doc.elementsByTagName( u"Subject"_s ).at( 0 ).toElement().text(), u"my subject"_s );
  QCOMPARE( doc.elementsByTagName( u"Title"_s ).at( 0 ).toElement().text(), u"my title"_s );
  QCOMPARE( doc.elementsByTagName( u"Keywords"_s ).at( 0 ).toElement().text(), u"k1: v1,v2"_s );
}

void TestQgsGeospatialPdfExport::testGeoref()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // test creation of the composition xml with georeferencing

  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers;
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( u"Georeferencing"_s ).count(), 0 );

  // with points
  QgsAbstractGeospatialPdfExporter::GeoReferencedSection section;
  section.crs = QgsCoordinateReferenceSystem( u"EPSG:4283"_s );
  section.pageBoundsMm = QgsRectangle( 0, 0, 253.2, 222.25 );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 0, 0 ), QgsPointXY( -122.4, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 253.2, 0 ), QgsPointXY( -78, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 253.2, 222.25 ), QgsPointXY( -78, 14.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 0, 222.25 ), QgsPointXY( -122.4, 14.6 ) ) );
  details.georeferencedSections << section;

  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( u"SRS"_s ).at( 0 ).toElement().text(), u"EPSG:4283"_s );
  QCOMPARE( doc.elementsByTagName( u"BoundingBox"_s ).at( 0 ).toElement().attribute( u"x1"_s ), u"0"_s );
  QCOMPARE( doc.elementsByTagName( u"BoundingBox"_s ).at( 0 ).toElement().attribute( u"x2"_s ).left( 9 ), u"717.73228"_s );
  QCOMPARE( doc.elementsByTagName( u"BoundingBox"_s ).at( 0 ).toElement().attribute( u"y1"_s ), u"0"_s );
  QCOMPARE( doc.elementsByTagName( u"BoundingBox"_s ).at( 0 ).toElement().attribute( u"y2"_s ), u"630"_s );

  QDomNodeList cps = doc.elementsByTagName( u"ControlPoint"_s );
  QCOMPARE( cps.count(), 4 );
  QDomElement cp1;
  for ( int i = 0; i < 4; ++i )
  {
    const QString x = cps.at( i ).toElement().attribute( u"GeoX"_s ).left( 10 );
    const QString y = cps.at( i ).toElement().attribute( u"GeoY"_s ).left( 10 );
    if ( x == "-122.40000"_L1 && y == "53.6000000"_L1 )
    {
      cp1 = cps.at( i ).toElement();
      break;
    }
  }
  QVERIFY( !cp1.isNull() );
  QCOMPARE( cp1.attribute( u"x"_s ), u"0"_s );
  QCOMPARE( cp1.attribute( u"y"_s ).left( 10 ), u"-2.8346456"_s );
}

void TestQgsGeospatialPdfExport::testGeorefPolygon()
{
  // test georeferencing a region using polygon bounds
  TestGeospatialPdfExporter geospatialPdfExporter;

  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers;
  QgsAbstractGeospatialPdfExporter::ExportDetails details;

  // with points
  QgsAbstractGeospatialPdfExporter::GeoReferencedSection section;
  section.crs = QgsCoordinateReferenceSystem( u"EPSG:4283"_s );
  section.pageBoundsMm = QgsRectangle( 0, 0, 253.2, 222.25 );
  QgsPolygon p;
  p.fromWkt( u"Polygon((30 5, 250 15, 240 200, 50 190, 30 5))"_s );
  section.pageBoundsPolygon = p;
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 0, 0 ), QgsPointXY( -122.4, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 253.2, 0 ), QgsPointXY( -78, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 253.2, 222.25 ), QgsPointXY( -78, 14.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 0, 222.25 ), QgsPointXY( -122.4, 14.6 ) ) );
  details.georeferencedSections << section;

  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( u"SRS"_s ).at( 0 ).toElement().text(), u"EPSG:4283"_s );
  QCOMPARE( doc.elementsByTagName( u"BoundingPolygon"_s ).at( 0 ).toElement().text(), u"Polygon ((60 -12, 500 -32, 480 -402, 100 -382, 60 -12))"_s );

  QDomNodeList cps = doc.elementsByTagName( u"ControlPoint"_s );
  QCOMPARE( cps.count(), 4 );
  QDomElement cp1;
  for ( int i = 0; i < 4; ++i )
  {
    const QString x = cps.at( i ).toElement().attribute( u"GeoX"_s ).left( 10 );
    const QString y = cps.at( i ).toElement().attribute( u"GeoY"_s ).left( 10 );
    if ( x == "-122.40000"_L1 && y == "53.6000000"_L1 )
    {
      cp1 = cps.at( i ).toElement();
      break;
    }
  }
  QVERIFY( !cp1.isNull() );
  QCOMPARE( cp1.attribute( u"x"_s ), u"0"_s );
  QCOMPARE( cp1.attribute( u"y"_s ).left( 10 ), u"-2.8346456"_s );
}

void TestQgsGeospatialPdfExport::testGroups()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // no features, no crash
  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geospatialPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( u"a1"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"a2"_s, QMetaType::Type::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(1 1, 2 2)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer2"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == "layer1"_L1 )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == "layer2"_L1 )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers; // no extra layers for now
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QDomNodeList ifLayerOnList = doc.elementsByTagName( u"IfLayerOn"_s );
  QCOMPARE( ifLayerOnList.count(), 2 );

  int layer1Idx = ifLayerOnList.at( 0 ).toElement().attribute( u"layerId"_s ) == "layer1"_L1 ? 0 : 1;
  int layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().attribute( u"layerId"_s ), u"layer1"_s );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer1Path );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer1Layer );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer1"_s );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer1"_s );

  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().attribute( u"layerId"_s ), u"layer2"_s );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer2Path );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer2Layer );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer2"_s );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer2"_s );


  QDomNodeList layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );

  layer1Idx = layerTreeList.at( 0 ).toElement().attribute( u"id"_s ) == "layer1"_L1 ? 0 : 1;
  layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( u"id"_s ), u"layer1"_s );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( u"name"_s ), u"name layer1"_s );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );

  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( u"id"_s ), u"layer2"_s );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( u"name"_s ), u"name layer2"_s );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );
}

void TestQgsGeospatialPdfExport::testCustomGroups()
{
  TestGeospatialPdfExporter geospatialPdfExporter;

  QgsFields fields;
  fields.append( QgsField( u"a1"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"a2"_s, QMetaType::Type::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(1 1, 2 2)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer2"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == "layer1"_L1 )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == "layer2"_L1 )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers; // no extra layers for now
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  details.customLayerTreeGroups.insert( u"layer1"_s, u"my group"_s );
  details.customLayerTreeGroups.insert( u"layer2"_s, u"my group2"_s );

  // this group is empty, since layer3 doesn't exist
  details.customLayerTreeGroups.insert( u"layer3"_s, u"my empty group"_s );

  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );

  QString group1Id = layerTreeList.at( 0 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"my group"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().tagName(), u"Layer"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().attribute( u"name"_s ), u"name layer1"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().attribute( u"id"_s ), u"my group_layer1"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().attribute( u"initiallyVisible"_s ), u"true"_s );

  QString group2Id = layerTreeList.at( 1 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my group2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().firstChildElement().attribute( u"name"_s ), u"name layer2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().firstChildElement().attribute( u"id"_s ), u"my group2_layer2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().firstChildElement().attribute( u"initiallyVisible"_s ), u"true"_s );

  QDomNodeList contentList = doc.documentElement().firstChildElement( u"Page"_s ).firstChildElement( u"Content"_s ).childNodes();
  QCOMPARE( contentList.count(), 2 );

  int layer1Idx = contentList.at( 0 ).toElement().attribute( u"layerId"_s ) == group1Id ? 0 : 1;
  int layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( contentList.at( layer1Idx ).toElement().attribute( u"layerId"_s ), group1Id );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().tagName(), u"IfLayerOn"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().attribute( u"layerId"_s ), u"my group_layer1"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer1Path );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer1Layer );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer1"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer1"_s );

  QCOMPARE( contentList.at( layer2Idx ).toElement().attribute( u"layerId"_s ), group2Id );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().tagName(), u"IfLayerOn"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().attribute( u"layerId"_s ), u"my group2_layer2"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer2Path );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer2Layer );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer2"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer2"_s );
}

void TestQgsGeospatialPdfExport::testGroupOrder()
{
  TestGeospatialPdfExporter geospatialPdfExporter;

  QgsFields fields;
  fields.append( QgsField( u"a1"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"a2"_s, QMetaType::Type::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(1 1, 2 2)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer2"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == "layer1"_L1 )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == "layer2"_L1 )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers; // no extra layers for now
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  details.customLayerTreeGroups.insert( u"layer1"_s, u"my group"_s );
  details.customLayerTreeGroups.insert( u"layer2"_s, u"my group2"_s );

  details.layerTreeGroupOrder = QStringList { u"my group2"_s, u"my group"_s };

  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"my group2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my group"_s );

  details.layerTreeGroupOrder = QStringList { u"my group"_s, u"my group2"_s };

  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );

  layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"my group"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my group2"_s );

  // incomplete list
  details.layerTreeGroupOrder = QStringList { u"my group2"_s };

  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );

  layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"my group2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my group"_s );

  // list with extra groups which don't have content
  details.layerTreeGroupOrder = QStringList { u"aaa"_s, u"my group"_s };

  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );

  layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"my group"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my group2"_s );
}

void TestQgsGeospatialPdfExport::testLayerOrder()
{
  TestGeospatialPdfExporter geospatialPdfExporter;

  QgsFields fields;
  fields.append( QgsField( u"a1"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"a2"_s, QMetaType::Type::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(1 1, 2 2)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer2"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );

  // test creation of the composition xml
  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers; // no extra layers for now
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  details.layerOrder = QStringList { "layer2", "layer1" };

  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"name layer2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"name layer1"_s );

  details.layerOrder = QStringList { "layer1", "layer2" };
  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );

  layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"name layer1"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"name layer2"_s );

  // layer without attributes
  QgsAbstractGeospatialPdfExporter::ComponentLayerDetail detail;
  detail.mapLayerId = u"layer3"_s;
  detail.name = u"xxx layer 3"_s;
  detail.opacity = 0.7;
  detail.compositionMode = QPainter::CompositionMode_Screen;
  detail.sourcePdfPath = u"a pdf.pdf"_s;
  renderedLayers << detail;

  details.layerOrder = QStringList { "layer3", "layer1", "layer2" };
  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );

  layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 3 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"xxx layer 3"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"name layer1"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"name"_s ), u"name layer2"_s );

  details.layerOrder = QStringList { "layer1", "layer2", "layer3" };
  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );

  layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 3 );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"name layer1"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"name layer2"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"name"_s ), u"xxx layer 3"_s );
}

void TestQgsGeospatialPdfExport::testMutuallyExclusiveGroupsLayers()
{
  TestGeospatialPdfExporter geospatialPdfExporter;

  QgsFields fields;
  fields.append( QgsField( u"a1"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"a2"_s, QMetaType::Type::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( u"layer1"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(1 1, 2 2)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer2"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 41 << 42 );
  f.setGeometry( QgsGeometry( new QgsPoint( 6, 7 ) ) );
  renderedBounds = QgsGeometry::fromWkt( u"LineString(6 6, 7 7)"_s );
  geospatialPdfExporter.pushRenderedFeature( u"layer3"_s, QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;
  QString layer3Path;
  QString layer3Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == "layer1"_L1 )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == "layer2"_L1 )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == "layer3"_L1 )
    {
      layer3Path = it.sourceVectorPath;
      layer3Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList<QgsAbstractGeospatialPdfExporter::ComponentLayerDetail> renderedLayers; // no extra layers for now
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  details.customLayerTreeGroups.insert( u"layer1"_s, u"my group"_s );
  details.customLayerTreeGroups.insert( u"layer2"_s, u"my group2"_s );
  details.customLayerTreeGroups.insert( u"layer3"_s, u"my group3"_s );
  // groups 1 & 2 should be mutually exclusive
  details.mutuallyExclusiveGroups.insert( u"my group"_s );
  details.mutuallyExclusiveGroups.insert( u"my group2"_s );

  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 3 );

  QString group1Id = layerTreeList.at( 0 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"my group"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"mutuallyExclusiveGroupId"_s ), u"__mutually_exclusive_groups__"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().tagName(), u"Layer"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().attribute( u"name"_s ), u"name layer1"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().firstChildElement().attribute( u"id"_s ), u"my group_layer1"_s );

  QString group2Id = layerTreeList.at( 1 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my group2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"initiallyVisible"_s ), u"false"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"mutuallyExclusiveGroupId"_s ), u"__mutually_exclusive_groups__"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().firstChildElement().tagName(), u"Layer"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().firstChildElement().attribute( u"name"_s ), u"name layer2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().firstChildElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().firstChildElement().attribute( u"id"_s ), u"my group2_layer2"_s );

  QString group3Id = layerTreeList.at( 2 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"name"_s ), u"my group3"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"mutuallyExclusiveGroupId"_s ), QString() );
  QCOMPARE( layerTreeList.at( 2 ).toElement().firstChildElement().tagName(), u"Layer"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().firstChildElement().attribute( u"name"_s ), u"name layer3"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().firstChildElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().firstChildElement().attribute( u"id"_s ), u"my group3_layer3"_s );

  QDomNodeList contentList = doc.documentElement().firstChildElement( u"Page"_s ).firstChildElement( u"Content"_s ).childNodes();
  QCOMPARE( contentList.count(), 3 );

  int layer1Idx = contentList.at( 0 ).toElement().attribute( u"layerId"_s ) == group1Id   ? 0
                  : contentList.at( 1 ).toElement().attribute( u"layerId"_s ) == group1Id ? 1
                                                                                          : 2;
  int layer2Idx = contentList.at( 0 ).toElement().attribute( u"layerId"_s ) == group2Id   ? 0
                  : contentList.at( 1 ).toElement().attribute( u"layerId"_s ) == group2Id ? 1
                                                                                          : 2;
  int layer3Idx = contentList.at( 0 ).toElement().attribute( u"layerId"_s ) == group3Id   ? 0
                  : contentList.at( 1 ).toElement().attribute( u"layerId"_s ) == group3Id ? 1
                                                                                          : 2;
  QCOMPARE( contentList.at( layer1Idx ).toElement().attribute( u"layerId"_s ), group1Id );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().tagName(), u"IfLayerOn"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().attribute( u"layerId"_s ), u"my group_layer1"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer1Path );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer1Layer );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer1"_s );
  QCOMPARE( contentList.at( layer1Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer1"_s );

  QCOMPARE( contentList.at( layer2Idx ).toElement().attribute( u"layerId"_s ), group2Id );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().tagName(), u"IfLayerOn"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().attribute( u"layerId"_s ), u"my group2_layer2"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer2Path );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer2Layer );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer2"_s );
  QCOMPARE( contentList.at( layer2Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer2"_s );

  QCOMPARE( contentList.at( layer3Idx ).toElement().attribute( u"layerId"_s ), group3Id );
  QCOMPARE( contentList.at( layer3Idx ).toElement().firstChildElement().tagName(), u"IfLayerOn"_s );
  QCOMPARE( contentList.at( layer3Idx ).toElement().firstChildElement().attribute( u"layerId"_s ), u"my group3_layer3"_s );
  QCOMPARE( contentList.at( layer3Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"dataset"_s ), layer3Path );
  QCOMPARE( contentList.at( layer3Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"layer"_s ), layer3Layer );
  QCOMPARE( contentList.at( layer3Idx ).toElement().firstChildElement().elementsByTagName( u"Vector"_s ).at( 0 ).toElement().attribute( u"visible"_s ), u"false"_s );
  QCOMPARE( contentList.at( layer3Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"fieldToDisplay"_s ), u"attr layer3"_s );
  QCOMPARE( contentList.at( layer3Idx ).toElement().firstChildElement().elementsByTagName( u"LogicalStructure"_s ).at( 0 ).toElement().attribute( u"displayLayerName"_s ), u"name layer3"_s );
}

void TestQgsGeospatialPdfExport::testMutuallyExclusiveGroupsCustom()
{
  TestGeospatialPdfExporter geospatialPdfExporter;

  // test creation of the composition xml
  QgsAbstractGeospatialPdfExporter::ComponentLayerDetail group1;
  group1.group = u"my group"_s;
  group1.sourcePdfPath = u"group1.pdf"_s;
  QgsAbstractGeospatialPdfExporter::ComponentLayerDetail group2;
  group2.group = u"my group2"_s;
  group2.sourcePdfPath = u"group2.pdf"_s;
  QgsAbstractGeospatialPdfExporter::ComponentLayerDetail group3;
  group3.group = u"my group3"_s;
  group3.sourcePdfPath = u"group3.pdf"_s;
  QgsAbstractGeospatialPdfExporter::ComponentLayerDetail object4;
  object4.sourcePdfPath = u"object4.pdf"_s;

  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  // groups 1 & 2 should be mutually exclusive
  details.mutuallyExclusiveGroups.insert( u"my group"_s );
  details.mutuallyExclusiveGroups.insert( u"my group2"_s );

  QString composition = geospatialPdfExporter.createCompositionXml( { group1, group2, group3, object4 }, details );

  QgsDebugError( composition );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( u"LayerTree"_s ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 3 );

  QString group1Id = layerTreeList.at( 0 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"name"_s ), u"my group"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( u"mutuallyExclusiveGroupId"_s ), u"__mutually_exclusive_groups__"_s );
  QCOMPARE( layerTreeList.at( 0 ).toElement().childNodes().count(), 0 );

  QString group2Id = layerTreeList.at( 1 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"name"_s ), u"my group2"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"initiallyVisible"_s ), u"false"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( u"mutuallyExclusiveGroupId"_s ), u"__mutually_exclusive_groups__"_s );
  QCOMPARE( layerTreeList.at( 1 ).toElement().childNodes().count(), 0 );

  QString group3Id = layerTreeList.at( 2 ).toElement().attribute( u"id"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"name"_s ), u"my group3"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"initiallyVisible"_s ), u"true"_s );
  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( u"mutuallyExclusiveGroupId"_s ), QString() );
  QCOMPARE( layerTreeList.at( 2 ).toElement().childNodes().count(), 0 );

  QDomNodeList contentList = doc.documentElement().firstChildElement( u"Page"_s ).firstChildElement( u"Content"_s ).childNodes();
  QCOMPARE( contentList.count(), 4 );

  QCOMPARE( contentList.at( 0 ).toElement().attribute( u"layerId"_s ), group1Id );
  QCOMPARE( contentList.at( 0 ).toElement().childNodes().at( 0 ).toElement().tagName(), u"PDF"_s );
  QCOMPARE( contentList.at( 0 ).toElement().childNodes().at( 0 ).toElement().attribute( u"dataset"_s ), u"group1.pdf"_s );

  QCOMPARE( contentList.at( 1 ).toElement().attribute( u"layerId"_s ), group2Id );
  QCOMPARE( contentList.at( 1 ).toElement().childNodes().at( 0 ).toElement().tagName(), u"PDF"_s );
  QCOMPARE( contentList.at( 1 ).toElement().childNodes().at( 0 ).toElement().attribute( u"dataset"_s ), u"group2.pdf"_s );

  QCOMPARE( contentList.at( 2 ).toElement().attribute( u"layerId"_s ), group3Id );
  QCOMPARE( contentList.at( 2 ).toElement().childNodes().at( 0 ).toElement().tagName(), u"PDF"_s );
  QCOMPARE( contentList.at( 2 ).toElement().childNodes().at( 0 ).toElement().attribute( u"dataset"_s ), u"group3.pdf"_s );

  QCOMPARE( contentList.at( 3 ).toElement().tagName(), u"PDF"_s );
  QCOMPARE( contentList.at( 3 ).toElement().attribute( u"dataset"_s ), u"object4.pdf"_s );
}

void TestQgsGeospatialPdfExport::compositionMode()
{
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_SourceOver ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Multiply ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Screen ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Overlay ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Darken ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Lighten ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_ColorDodge ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_ColorBurn ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_HardLight ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_SoftLight ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Difference ) );
  QVERIFY( QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Exclusion ) );
  QVERIFY( !QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode_Plus ) );

  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_SourceOver ), u"Normal"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Multiply ), u"Multiply"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Screen ), u"Screen"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Overlay ), u"Overlay"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Darken ), u"Darken"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Lighten ), u"Lighten"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_ColorDodge ), u"ColorDodge"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_ColorBurn ), u"ColorBurn"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_HardLight ), u"HardLight"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_SoftLight ), u"SoftLight"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Difference ), u"Difference"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Exclusion ), u"Exclusion"_s );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Plus ), u"Normal"_s );
}


QGSTEST_MAIN( TestQgsGeospatialPdfExport )
#include "testqgsgeopdfexport.moc"
