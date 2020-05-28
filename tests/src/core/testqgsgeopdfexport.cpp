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
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgslayoutitemmap.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsabstractgeopdfexporter.h"
#include <QObject>
#include "qgstest.h"

class TestGeoPdfExporter : public QgsAbstractGeoPdfExporter
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

class TestQgsGeoPdfExport : public QObject
{
    Q_OBJECT

  public:
    TestQgsGeoPdfExport() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testCollectingFeatures();
    void testComposition();
    void testMetadata();
    void testGeoref();
    void testGeorefPolygon();
    void testGroups();
    void testCustomGroups();

  private:

    QString mReport;
};

void TestQgsGeoPdfExport::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport = QStringLiteral( "<h1>GeoPDF Export Tests</h1>\n" );
}

void TestQgsGeoPdfExport::cleanupTestCase()
{
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

void TestQgsGeoPdfExport::init()
{
}

void TestQgsGeoPdfExport::cleanup()
{
}

void TestQgsGeoPdfExport::testCollectingFeatures()
{
  if ( !QgsAbstractGeoPdfExporter::geoPDFCreationAvailable() )
  {
    QSKIP( "This test requires GeoPDF creation abilities", SkipSingle );
  }

  TestGeoPdfExporter geoPdfExporter;
  // no features, no crash
  QVERIFY( geoPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geoPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 11 << 22 );
  f.setGeometry( QgsGeometry( new QgsPoint( 3, 4 ) ) );
  renderedBounds = QgsGeometry::fromRect( QgsRectangle( 2, 10, 7, 20 ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geoPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geoPdfExporter.mVectorComponents.count(), 2 );
  QgsAbstractGeoPdfExporter::VectorComponentDetail component;
  for ( const auto &it : qgis::as_const( geoPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QStringLiteral( "layer1" ) )
    {
      component = it;
      break;
    }
  }
  QCOMPARE( component.mapLayerId, QStringLiteral( "layer1" ) );
  QCOMPARE( component.name, QStringLiteral( "name layer1" ) );
  // check that temporary layers were correctly written
  std::unique_ptr< QgsVectorLayer > layer = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layerName=%2" ).arg( component.sourceVectorPath, component.sourceVectorLayer ), QStringLiteral( "layer" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->featureCount(), 2L );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::Polygon );
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

  for ( const auto &it : qgis::as_const( geoPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QStringLiteral( "layer2" ) )
    {
      component = it;
      break;
    }
  }
  QCOMPARE( component.mapLayerId, QStringLiteral( "layer2" ) );
  QCOMPARE( component.name, QStringLiteral( "name layer2" ) );
  layer = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layerName=%2" ).arg( component.sourceVectorPath, component.sourceVectorLayer ), QStringLiteral( "layer" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->featureCount(), 1L );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( layer->fields().at( 1 ).name(), QStringLiteral( "a1" ) );
  QCOMPARE( layer->fields().at( 2 ).name(), QStringLiteral( "a2" ) );
  it = layer->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attributes().at( 1 ).toInt(), 31 );
  QCOMPARE( f.attributes().at( 2 ).toInt(), 32 );
  QCOMPARE( f.geometry().asWkt(), QStringLiteral( "LineString (1 1, 2 2)" ) );
}

void TestQgsGeoPdfExport::testComposition()
{
  TestGeoPdfExporter geoPdfExporter;
  // no features, no crash
  QVERIFY( geoPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geoPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geoPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeoPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : qgis::as_const( geoPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QStringLiteral( "layer1" ) )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == QStringLiteral( "layer2" ) )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > renderedLayers; // no extra layers for now
  QgsAbstractGeoPdfExporter::ExportDetails details;
  QString composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
  QDomDocument doc;
  doc.setContent( composition );
  QDomNodeList ifLayerOnList = doc.elementsByTagName( QStringLiteral( "IfLayerOn" ) );
  QCOMPARE( ifLayerOnList.count(), 2 );

  int layer1Idx = ifLayerOnList.at( 0 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QStringLiteral( "layer1" ) ? 0 : 1;
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

  layer1Idx = layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "id" ) ) == QStringLiteral( "layer1" ) ? 0 : 1;
  layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer1" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "name layer1" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer2" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "name layer2" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

}

void TestQgsGeoPdfExport::testMetadata()
{
  TestGeoPdfExporter geoPdfExporter;
  // test creation of the composition xml with metadata

  QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > renderedLayers;
  QgsAbstractGeoPdfExporter::ExportDetails details;
  QString composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
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

  composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Author" ) ).at( 0 ).toElement().text(), QStringLiteral( "my author" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Producer" ) ).at( 0 ).toElement().text(), QStringLiteral( "my producer" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Creator" ) ).at( 0 ).toElement().text(), QStringLiteral( "my creator" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "CreationDate" ) ).at( 0 ).toElement().text(), QStringLiteral( "D:20100305123400+0'0'" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Subject" ) ).at( 0 ).toElement().text(), QStringLiteral( "my subject" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Title" ) ).at( 0 ).toElement().text(), QStringLiteral( "my title" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Keywords" ) ).at( 0 ).toElement().text(), QStringLiteral( "k1: v1,v2" ) );

}

void TestQgsGeoPdfExport::testGeoref()
{
  TestGeoPdfExporter geoPdfExporter;
  // test creation of the composition xml with georeferencing

  QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > renderedLayers;
  QgsAbstractGeoPdfExporter::ExportDetails details;
  QString composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
  QDomDocument doc;
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "Georeferencing" ) ).count(), 0 );

  // with points
  QgsAbstractGeoPdfExporter::GeoReferencedSection section;
  section.crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4283" ) );
  section.pageBoundsMm = QgsRectangle( 0, 0, 253.2, 222.25 );
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 0, 0 ), QgsPointXY( -122.4, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 253.2, 0 ), QgsPointXY( -78, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 253.2, 222.25 ), QgsPointXY( -78, 14.6 ) ) );
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 0, 222.25 ), QgsPointXY( -122.4, 14.6 ) ) );
  details.georeferencedSections << section;

  composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "SRS" ) ).at( 0 ).toElement().text(), QStringLiteral( "EPSG:4283" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "x1" ) ), QStringLiteral( "0" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "x2" ) ), QStringLiteral( "717.732" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "y1" ) ), QStringLiteral( "0" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingBox" ) ).at( 0 ).toElement().attribute( QStringLiteral( "y2" ) ), QStringLiteral( "630" ) );

  QDomNodeList cps = doc.elementsByTagName( QStringLiteral( "ControlPoint" ) );
  QCOMPARE( cps.count(), 4 );
  QDomElement cp1;
  for ( int i = 0; i < 4; ++i )
  {
    if ( cps.at( i ).toElement().attribute( QStringLiteral( "GeoX" ) ) == QStringLiteral( "-122.4" )
         && cps.at( i ).toElement().attribute( QStringLiteral( "GeoY" ) ) == QStringLiteral( "53.6" ) )
    {
      cp1 = cps.at( i ).toElement();
      break;
    }
  }
  QVERIFY( !cp1.isNull() );
  QCOMPARE( cp1.attribute( QStringLiteral( "x" ) ), QStringLiteral( "0" ) );
  QCOMPARE( cp1.attribute( QStringLiteral( "y" ) ), QStringLiteral( "-2.83465" ) );
}

void TestQgsGeoPdfExport::testGeorefPolygon()
{
  // test georeferencing a region using polygon bounds
  TestGeoPdfExporter geoPdfExporter;

  QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > renderedLayers;
  QgsAbstractGeoPdfExporter::ExportDetails details;

  // with points
  QgsAbstractGeoPdfExporter::GeoReferencedSection section;
  section.crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4283" ) );
  section.pageBoundsMm = QgsRectangle( 0, 0, 253.2, 222.25 );
  QgsPolygon p;
  p.fromWkt( QStringLiteral( "Polygon((30 5, 250 15, 240 200, 50 190, 30 5))" ) );
  section.pageBoundsPolygon = p;
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 0, 0 ), QgsPointXY( -122.4, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 253.2, 0 ), QgsPointXY( -78, 53.6 ) ) );
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 253.2, 222.25 ), QgsPointXY( -78, 14.6 ) ) );
  section.controlPoints.append( QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 0, 222.25 ), QgsPointXY( -122.4, 14.6 ) ) );
  details.georeferencedSections << section;

  QString composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
  QDomDocument doc;
  doc.setContent( composition );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "SRS" ) ).at( 0 ).toElement().text(), QStringLiteral( "EPSG:4283" ) );
  QCOMPARE( doc.elementsByTagName( QStringLiteral( "BoundingPolygon" ) ).at( 0 ).toElement().text(), QStringLiteral( "Polygon ((60 -12, 500 -32, 480 -402, 100 -382, 60 -12))" ) );

  QDomNodeList cps = doc.elementsByTagName( QStringLiteral( "ControlPoint" ) );
  QCOMPARE( cps.count(), 4 );
  QDomElement cp1;
  for ( int i = 0; i < 4; ++i )
  {
    if ( cps.at( i ).toElement().attribute( QStringLiteral( "GeoX" ) ) == QStringLiteral( "-122.4" )
         && cps.at( i ).toElement().attribute( QStringLiteral( "GeoY" ) ) == QStringLiteral( "53.6" ) )
    {
      cp1 = cps.at( i ).toElement();
      break;
    }
  }
  QVERIFY( !cp1.isNull() );
  QCOMPARE( cp1.attribute( QStringLiteral( "x" ) ), QStringLiteral( "0" ) );
  QCOMPARE( cp1.attribute( QStringLiteral( "y" ) ), QStringLiteral( "-2.83465" ) );
}

void TestQgsGeoPdfExport::testGroups()
{
  TestGeoPdfExporter geoPdfExporter;
  // no features, no crash
  QVERIFY( geoPdfExporter.saveTemporaryLayers() );
  QCOMPARE( geoPdfExporter.mVectorComponents.count(), 0 );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geoPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeoPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : qgis::as_const( geoPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QStringLiteral( "layer1" ) )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == QStringLiteral( "layer2" ) )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > renderedLayers; // no extra layers for now
  QgsAbstractGeoPdfExporter::ExportDetails details;
  QString composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
  QDomDocument doc;
  doc.setContent( composition );
  QDomNodeList ifLayerOnList = doc.elementsByTagName( QStringLiteral( "IfLayerOn" ) );
  QCOMPARE( ifLayerOnList.count(), 2 );

  int layer1Idx = ifLayerOnList.at( 0 ).toElement().attribute( QStringLiteral( "layerId" ) ) == QStringLiteral( "layer1" ) ? 0 : 1;
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

  layer1Idx = layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "id" ) ) == QStringLiteral( "layer1" ) ? 0 : 1;
  layer2Idx = layer1Idx == 0 ? 1 : 0;
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer1" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "name layer1" ) );
  QCOMPARE( layerTreeList.at( layer1Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "id" ) ), QStringLiteral( "layer2" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "name" ) ), QStringLiteral( "name layer2" ) );
  QCOMPARE( layerTreeList.at( layer2Idx ).toElement().attribute( QStringLiteral( "initiallyVisible" ) ), QStringLiteral( "true" ) );

}

void TestQgsGeoPdfExport::testCustomGroups()
{
  TestGeoPdfExporter geoPdfExporter;

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "a1" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "a2" ), QVariant::Int ) );
  QgsFeature f( fields );

  f.setAttributes( QgsAttributes() << 1 << 2 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  QgsGeometry renderedBounds( QgsGeometry::fromRect( QgsRectangle( 1, 10, 6, 20 ) ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer1" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );
  f.setAttributes( QgsAttributes() << 31 << 32 );
  f.setGeometry( QgsGeometry( new QgsPoint( 4, 5 ) ) );
  renderedBounds = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 2 2)" ) );
  geoPdfExporter.pushRenderedFeature( QStringLiteral( "layer2" ), QgsAbstractGeoPdfExporter::RenderedFeature( f, renderedBounds ) );

  QVERIFY( geoPdfExporter.saveTemporaryLayers() );
  QgsAbstractGeoPdfExporter::VectorComponentDetail component;
  QString layer1Path;
  QString layer1Layer;
  QString layer2Path;
  QString layer2Layer;

  for ( const auto &it : qgis::as_const( geoPdfExporter.mVectorComponents ) )
  {
    if ( it.mapLayerId == QStringLiteral( "layer1" ) )
    {
      layer1Path = it.sourceVectorPath;
      layer1Layer = it.sourceVectorLayer;
    }
    else if ( it.mapLayerId == QStringLiteral( "layer2" ) )
    {
      layer2Path = it.sourceVectorPath;
      layer2Layer = it.sourceVectorLayer;
    }
  }

  // test creation of the composition xml
  QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > renderedLayers; // no extra layers for now
  QgsAbstractGeoPdfExporter::ExportDetails details;
  details.customLayerTreeGroups.insert( QStringLiteral( "layer1" ), QStringLiteral( "my group" ) );
  details.customLayerTreeGroups.insert( QStringLiteral( "layer2" ), QStringLiteral( "my group2" ) );

  QString composition = geoPdfExporter.createCompositionXml( renderedLayers, details );
  QgsDebugMsg( composition );
  QDomDocument doc;
  doc.setContent( composition );

  QDomNodeList layerTreeList = doc.elementsByTagName( QStringLiteral( "LayerTree" ) ).at( 0 ).toElement().childNodes();
  QCOMPARE( layerTreeList.count(), 2 );

  int layer1Idx = layerTreeList.at( 0 ).toElement().attribute( QStringLiteral( "name" ) ) == QStringLiteral( "my group" ) ? 0 : 1;
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

QGSTEST_MAIN( TestQgsGeoPdfExport )
#include "testqgsgeopdfexport.moc"
