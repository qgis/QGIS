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

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsabstractgeopdfexporter.h"
#include <QObject>
#include "qgstest.h"

class TestGeospatialPdfExporter : public QgsAbstractGeospatialPdfExporter
{

  private:

    VectorComponentDetail componentDetailForLayerId( const QString &layerId ) override
    {
      Q_UNUSED( layerId );
      VectorComponentDetail detail;
      detail.mapLayerId = layerId;
      detail.name = QStringLiteral( "name %1" ).arg( layerId );
      detail.displayAttribute = QStringLiteral( "attr %1" ).arg( layerId );
      return detail;
    }

};

class TestQgsGeospatialPdfExport : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsGeospatialPdfExport() : QgsTest( QStringLiteral( "Geospatial PDF Export Testss" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testCollectingFeatures();
    void testComposition();
    void testMetadata();
    void testGeoref();
    void testGeorefPolygon();
    void testGroups();
    void testCustomGroups();
    void compositionMode();
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
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 11 << 22 );
  f.setGeometry( QgsGeometry( new QgsPoint( 3, 4 ) ) );
  renderedBounds = QgsGeometry::fromRect( QgsRectangle( 2, 10, 7, 20 ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geospatialPdfExporter.mVectorComponents.count(), 2 );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QLatin1String( "layer1" ) )
    {
      component = it;
      break;
    }
  }
  QCOMPARE( component.mapLayerId, QStringLiteral( "layer1" ) );
  QCOMPARE( component.name, QStringLiteral( "name layer1" ) );
  // check that temporary layers were correctly written
  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layerName=%2" ).arg( component.sourceVectorPath, component.sourceVectorLayer ), QStringLiteral( "layer" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->featureCount(), 2L );
  QCOMPARE( layer->wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( layer->fields().at( 1 ).name(), QStringLiteral( "a1" ) );
  QCOMPARE( layer->fields().at( 2 ).name(), QStringLiteral( "a2" ) );
  QgsFeatureIterator it = layer->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attributes().at( 1 ).toInt(), 1 );
  QCOMPARE( f.attributes().at( 2 ).toInt(), 2 );
  QCOMPARE( f.geometry().asWkt(), QStringLiteral( "Polygon ((1 10, 6 10, 6 20, 1 20, 1 10))" ) );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attributes().at( 1 ).toInt(), 11 );
  QCOMPARE( f.attributes().at( 2 ).toInt(), 22 );
  QCOMPARE( f.geometry().asWkt(), QStringLiteral( "Polygon ((2 10, 7 10, 7 20, 2 20, 2 10))" ) );

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QLatin1String( "layer2" ) )
    {
      component = it;
      break;
    }
  }
  QCOMPARE( component.mapLayerId, QStringLiteral( "layer2" ) );
  QCOMPARE( component.name, QStringLiteral( "name layer2" ) );
  layer = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layerName=%2" ).arg( component.sourceVectorPath, component.sourceVectorLayer ), QStringLiteral( "layer" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->featureCount(), 1L );
  QCOMPARE( layer->wkbType(), Qgis::WkbType::LineString );
  QCOMPARE( layer->fields().at( 1 ).name(), QStringLiteral( "a1" ) );
  QCOMPARE( layer->fields().at( 2 ).name(), QStringLiteral( "a2" ) );
  it = layer->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attributes().at( 1 ).toInt(), 31 );
  QCOMPARE( f.attributes().at( 2 ).toInt(), 32 );
  QCOMPARE( f.geometry().asWkt(), QStringLiteral( "LineString (1 1, 2 2)" ) );
}

void TestQgsGeospatialPdfExport::testComposition()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // no features, no crash
  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geospatialPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QLatin1String( "layer1" ) )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == QLatin1String( "layer2" ) )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList< QgsAbstractGeospatialPdfExporter::ComponentLayerDetail > renderedLayers;
  QgsAbstractGeospatialPdfExporter::ComponentLayerDetail detail;
  detail.mapLayerId = QStringLiteral( "layer3" );
  detail.opacity = 0.7;
  detail.compositionMode = QPainter::CompositionMode_Screen;
  detail.sourcePdfPath = QStringLiteral( "a pdf.pdf" );
  renderedLayers << detail;

  QgsAbstractGeospatialPdfExporter::ExportDetails details;

  details.layerIdToPdfLayerTreeNameMap.insert( QStringLiteral( "layer1" ), QStringLiteral( "my first layer" ) );
  details.initialLayerVisibility.insert( QStringLiteral( "layer2" ), false );
  details.layerOrder = QStringList() << QStringLiteral( "layer2" );
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QDomNodeList ifLayerOnList = doc.elementsByTagName( QStringLiteral( "IfLayerOn" ) );
  QCOMPARE( ifLayerOnList.count(), 3 );

  int layer1Idx = ifLayerOnList.at( 0 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QLatin1String( "layer1" ) ? 0 :
                  ifLayerOnList.at( 1 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QLatin1String( "layer1" ) ? 1 : 2;
  int layer2Idx = ifLayerOnList.at( 0 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QLatin1String( "layer2" ) ? 0 :
                  ifLayerOnList.at( 1 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QLatin1String( "layer2" ) ? 1 : 2;
  int layer3Idx = ifLayerOnList.at( 0 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QLatin1String( "layer3" ) ? 0 :
                  ifLayerOnList.at( 1 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QLatin1String( "layer3" ) ? 1 : 2;
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().attribute( QStringLiteral( "layerId" ) ), QStringLiteral( "layer1" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "dataset" ) ), layer1Path );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "layer" ) ), layer1Layer );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "visible" ) ), QStringLiteral( "false" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "fieldToDisplay" ) ), QStringLiteral( "attr layer1" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "displayLayerName" ) ), QStringLiteral( "name layer1" ) );

  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().attribute( QStringLiteral( "layerId" ) ), QStringLiteral( "layer2" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "dataset" ) ), layer2Path );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "layer" ) ), layer2Layer );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "visible" ) ), QStringLiteral( "false" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "fieldToDisplay" ) ), QStringLiteral( "attr layer2" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "displayLayerName" ) ), QStringLiteral( "name layer2" ) );

  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().attribute( QStringLiteral( "layerId" ) ), QStringLiteral( "layer3" ) );
  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().elementsByTagName( QStringLiteral( "PDF" ) ).at( 0 ).toElement().attribute( QStringLiteral( "dataset" ) ), QStringLiteral( "a pdf.pdf" ) );
  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().elementsByTagName( QStringLiteral( "PDF" ) ).at( 0 ).toElement().elementsByTagName( QStringLiteral( "Blending" ) ).at( 0 ).toElement().attribute( QStringLiteral( "opacity" ) ).toDouble(), 0.7 );
  QCOMPARE( ifLayerOnList.at( layer3Idx ).toElement().elementsByTagName( QStringLiteral( "PDF" ) ).at( 0 ).toElement().elementsByTagName( QStringLiteral( "Blending" ) ).at( 0 ).toElement().attribute( QStringLiteral( "function" ) ), QStringLiteral( "Screen" ) );

  QDomNodeList layerTreeList = doc.elementsByTagName( QStringLiteral( "LayerTree" ) ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 3 );

  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer1" ) );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "my first layer" ) );
  QCOMPARE( layerTreeList.at( 1 ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer2" ) );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "name layer2" ) );
  QCOMPARE( layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "false" ) );

  QCOMPARE( layerTreeList.at( 2 ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer3" ) );
}

void TestQgsGeospatialPdfExport::testMetadata()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // test creation of the composition xml with metadata

  QList< QgsAbstractGeospatialPdfExporter::ComponentLayerDetail > renderedLayers;
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Author" ) ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Producer" ) ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Creator" ) ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "CreationDate" ) ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Subject" ) ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Title" ) ).count(), 0 );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Keywords" ) ).count(), 0 );

  // with metadata
  details.author = QStringLiteral( "my author" );
  details.producer = QStringLiteral( "my producer" );
  details.creator = QStringLiteral( "my creator" );
  details.creationDateTime = QDateTime( QDate( 2010, 3, 5 ), QTime( 12, 34 ), Qt::UTC );
  details.subject = QStringLiteral( "my subject" );
  details.title = QStringLiteral( "my title" );
  details.keywords.insert( QStringLiteral( "k1" ), QStringList() << QStringLiteral( "v1" ) << QStringLiteral( "v2" ) );

  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Author" ) ).at( 0 ).toElement().text(), QStringLiteral( "my author" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Producer" ) ).at( 0 ).toElement().text(), QStringLiteral( "my producer" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Creator" ) ).at( 0 ).toElement().text(), QStringLiteral( "my creator" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "CreationDate" ) ).at( 0 ).toElement().text(), QStringLiteral( "D:20100305123400+0'0'" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Subject" ) ).at( 0 ).toElement().text(), QStringLiteral( "my subject" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Title" ) ).at( 0 ).toElement().text(), QStringLiteral( "my title" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Keywords" ) ).at( 0 ).toElement().text(), QStringLiteral( "k1: v1,v2" ) );

}

void TestQgsGeospatialPdfExport::testGeoref()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // test creation of the composition xml with georeferencing

  QList< QgsAbstractGeospatialPdfExporter::ComponentLayerDetail > renderedLayers;
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Georeferencing" ) ).count(), 0 );

  // with points
  QgsAbstractGeospatialPdfExporter::GeoReferencedSection section;
  section.crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4283" ) );
  section.pageBoundsMm = QgsRectangle( 0, 0, 253.2, 222.25 );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 0, 0 ), QgsPointXY( -122.4, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 253.2, 0 ), QgsPointXY( -78, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 253.2, 222.25 ), QgsPointXY( -78, 14.6 ) ) );
  section.controlPoints.append( QgsAbstractGeospatialPdfExporter::ControlPoint( QgsPointXY( 0, 222.25 ), QgsPointXY( -122.4, 14.6 ) ) );
  details.georeferencedSections << section;

  composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "SRS" ) ).at( 0 ).toElement().text(), QStringLiteral( "EPSG:4283" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "x1" ) ), QStringLiteral( "0" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "x2" ) ).left( 9 ), QStringLiteral( "717.73228" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "y1" ) ), QStringLiteral( "0" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "y2" ) ), QStringLiteral( "630" ) );

  QDomNodeList cps = doc.elementsByTagName( QStringLiteral( "ControlPoint" ) );
  QCOMPARE( cps.count(), 4 );
  QDomElement cp1;
  for ( int i = 0; i < 4; ++i )
  {
    const QString x = cps.at( i ).toElement().attribute( QStringLiteral( "GeoX" ) ).left( 10 );
    const QString y = cps.at( i ).toElement().attribute( QStringLiteral( "GeoY" ) ).left( 10 );
    if ( x == QLatin1String( "-122.40000" ) && y == QLatin1String( "53.6000000" ) )
    {
      cp1 = cps.at( i ).toElement();
      break;
    }
  }
  QVERIFY( !cp1.isNull() );
  QCOMPARE( cp1.attribute( QStringLiteral( "x" ) ), QStringLiteral( "0" ) );
  QCOMPARE( cp1.attribute( QStringLiteral( "y" ) ).left( 10 ), QStringLiteral( "-2.8346456" ) );
}

void TestQgsGeospatialPdfExport::testGeorefPolygon()
{
  // test georeferencing a region using polygon bounds
  TestGeospatialPdfExporter geospatialPdfExporter;

  QList< QgsAbstractGeospatialPdfExporter::ComponentLayerDetail > renderedLayers;
  QgsAbstractGeospatialPdfExporter::ExportDetails details;

  // with points
  QgsAbstractGeospatialPdfExporter::GeoReferencedSection section;
  section.crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4283" ) );
  section.pageBoundsMm = QgsRectangle( 0, 0, 253.2, 222.25 );
  QgsPolygon p;
  p.fromWkt( QStringLiteral( "Polygon((30 5, 250 15, 240 200, 50 190, 30 5))" ) );
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
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "SRS" ) ).at( 0 ).toElement().text(), QStringLiteral( "EPSG:4283" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingPolygon" ) ).at( 0 ).toElement().text(), QStringLiteral( "Polygon ((60 -12, 500 -32, 480 -402, 100 -382, 60 -12))" ) );

  QDomNodeList cps = doc.elementsByTagName( QStringLiteral( "ControlPoint" ) );
  QCOMPARE( cps.count(), 4 );
  QDomElement cp1;
  for ( int i = 0; i < 4; ++i )
  {
    const QString x = cps.at( i ).toElement().attribute( QStringLiteral( "GeoX" ) ).left( 10 );
    const QString y = cps.at( i ).toElement().attribute( QStringLiteral( "GeoY" ) ).left( 10 );
    if ( x == QLatin1String( "-122.40000" ) && y == QLatin1String( "53.6000000" ) )
    {
      cp1 = cps.at( i ).toElement();
      break;
    }
  }
  QVERIFY( !cp1.isNull() );
  QCOMPARE( cp1.attribute( QStringLiteral( "x" ) ), QStringLiteral( "0" ) );
  QCOMPARE( cp1.attribute( QStringLiteral( "y" ) ).left( 10 ), QStringLiteral( "-2.8346456" ) );
}

void TestQgsGeospatialPdfExport::testGroups()
{
  TestGeospatialPdfExporter geospatialPdfExporter;
  // no features, no crash
  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geospatialPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QLatin1String( "layer1" ) )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == QLatin1String( "layer2" ) )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList< QgsAbstractGeospatialPdfExporter::ComponentLayerDetail > renderedLayers; // no extra layers for now
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );
  QDomNodeList ifLayerOnList = doc.elementsByTagName( QStringLiteral( "IfLayerOn" ) );
  QCOMPARE( ifLayerOnList.count(), 2 );

  int layer1Idx = ifLayerOnList.at( 0 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QLatin1String( "layer1" ) ? 0 : 1;
  int layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().attribute( QStringLiteral( "layerId" ) ), QStringLiteral( "layer1" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "dataset" ) ), layer1Path );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "layer" ) ), layer1Layer );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "visible" ) ), QStringLiteral( "false" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "fieldToDisplay" ) ), QStringLiteral( "attr layer1" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "displayLayerName" ) ), QStringLiteral( "name layer1" ) );

  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().attribute( QStringLiteral( "layerId" ) ), QStringLiteral( "layer2" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "dataset" ) ), layer2Path );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "layer" ) ), layer2Layer );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "visible" ) ), QStringLiteral( "false" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "fieldToDisplay" ) ), QStringLiteral( "attr layer2" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "displayLayerName" ) ), QStringLiteral( "name layer2" ) );


  QDomNodeList layerTreeList = doc.elementsByTagName( QStringLiteral( "LayerTree" ) ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );

  layer1Idx = layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "id" ) ) == QLatin1String( "layer1" ) ? 0 : 1;
  layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer1" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "name layer1" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer2" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "name layer2" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

}

void TestQgsGeospatialPdfExport::testCustomGroups()
{
  TestGeospatialPdfExporter geospatialPdfExporter;

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geospatialPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeospatialPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geospatialPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeospatialPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : std::as_const( geospatialPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QLatin1String( "layer1" ) )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == QLatin1String( "layer2" ) )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList< QgsAbstractGeospatialPdfExporter::ComponentLayerDetail > renderedLayers; // no extra layers for now
  QgsAbstractGeospatialPdfExporter::ExportDetails details;
  details.customLayerTreeGroups.insert( QStringLiteral( "layer1" ), QStringLiteral( "my group" ) );
  details.customLayerTreeGroups.insert( QStringLiteral( "layer2" ), QStringLiteral( "my group2" ) );

  QString composition = geospatialPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsgLevel( composition, 1 );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( QStringLiteral( "LayerTree" ) ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );

  int layer1Idx = layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "name" ) ) == QLatin1String( "my group" ) ? 0 : 1;
  int layer2Idx = layer1Idx == 0 ? 1 : 0;

  QString group1Id = layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "id" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "my group" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

  QString group2Id = layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "id" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "my group2" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

  QDomNodeList ifLayerOnList = doc.elementsByTagName( QStringLiteral( "IfLayerOn" ) );
  QCOMPARE( ifLayerOnList.count(), 2 );

  layer1Idx = ifLayerOnList.at( 0 ).toElement().attribute( QStringLiteral( "layerId" ) ) == group1Id ? 0 : 1;
  layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().attribute( QStringLiteral( "layerId" ) ), group1Id );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "dataset" ) ), layer1Path );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "layer" ) ), layer1Layer );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "visible" ) ), QStringLiteral( "false" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "fieldToDisplay" ) ), QStringLiteral( "attr layer1" ) );
  QCOMPARE( ifLayerOnList.at( layer1Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "displayLayerName" ) ), QStringLiteral( "name layer1" ) );

  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().attribute( QStringLiteral( "layerId" ) ), group2Id );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "dataset" ) ), layer2Path );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "layer" ) ), layer2Layer );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "Vector" ) ).at( 0 ).toElement().attribute( QStringLiteral( "visible" ) ), QStringLiteral( "false" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "fieldToDisplay" ) ), QStringLiteral( "attr layer2" ) );
  QCOMPARE( ifLayerOnList.at( layer2Idx ).toElement().elementsByTagName( QStringLiteral( "LogicalStructure" ) ).at( 0 ).toElement().attribute( QStringLiteral( "displayLayerName" ) ), QStringLiteral( "name layer2" ) );
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

  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_SourceOver ), QStringLiteral( "Normal" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Multiply ), QStringLiteral( "Multiply" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Screen ), QStringLiteral( "Screen" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Overlay ), QStringLiteral( "Overlay" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Darken ), QStringLiteral( "Darken" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Lighten ), QStringLiteral( "Lighten" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_ColorDodge ), QStringLiteral( "ColorDodge" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_ColorBurn ), QStringLiteral( "ColorBurn" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_HardLight ), QStringLiteral( "HardLight" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_SoftLight ), QStringLiteral( "SoftLight" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Difference ), QStringLiteral( "Difference" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Exclusion ), QStringLiteral( "Exclusion" ) );
  QCOMPARE( QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode_Plus ), QStringLiteral( "Normal" ) );
}

QGSTEST_MAIN( TestQgsGeospatialPdfExport )
#include "testqgsgeopdfexport.moc"
