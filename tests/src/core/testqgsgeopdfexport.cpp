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
  QString composition = geoPdfExporter.createCompositionXml( renderedLayers );
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

QGSTEST_MAIN( TestQgsGeoPdfExport )
#include "testqgsgeopdfexport.moc"
