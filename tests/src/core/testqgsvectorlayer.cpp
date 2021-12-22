/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
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
#include <QDesktopServices>
#include <QSignalSpy>

//qgis includes...
#include <qgsgeometry.h>
#include <qgsmaplayer.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerutils.h>
#include "qgsfeatureiterator.h"
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
//qgis test includes
#include "qgsrenderchecker.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vector layer class.
 */
class TestQgsVectorLayer : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorLayer() = default;

  private:
    bool mTestHasError =  false ;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsVectorLayer *mpLinesLayer = nullptr;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsVectorLayer *mpNonSpatialLayer = nullptr;
    QString mTestDataDir;
    QString mReport;


  private slots:

    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void nonSpatialIterator();
    void getValues();
    void setRenderer();
    void setFeatureBlendMode();
    void setLayerTransparency();
    void uniqueValues();
    void minimumValue();
    void maximumValue();
    void minimumAndMaximumValue();
    void isSpatial();
    void testAddTopologicalPoints();
    void testCopyPasteFieldConfiguration();
    void testCopyPasteFieldConfiguration_data();
    void testFieldExpression();
};

void TestQgsVectorLayer::initTestCase()
{
  mTestHasError = false;
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...

  //
  //create a non spatial layer that will be used in all tests...
  //
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
  const QString myDbfFileName = mTestDataDir + "nonspatial.dbf";
  const QFileInfo myDbfFileInfo( myDbfFileName );
  mpNonSpatialLayer = new QgsVectorLayer( myDbfFileInfo.filePath(),
                                          myDbfFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpNonSpatialLayer );
  //
  //create a point layer that will be used in all tests...
  //
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                      myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );


  //
  // Create a line layer that will be used in all tests...
  //
  const QString myLinesFileName = mTestDataDir + "lines.shp";
  const QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                     myLineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpLinesLayer );

  mReport += QLatin1String( "<h1>Vector Renderer Tests</h1>\n" );
}

void TestQgsVectorLayer::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }
  QgsApplication::exitQgis();
}

void TestQgsVectorLayer::nonSpatialIterator()
{
  QgsFeature f;
  QgsAttributeList myList;
  myList << 0 << 1 << 2 << 3;
  int myCount = 0;
  QgsFeatureIterator fit = mpNonSpatialLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( myList ) );
  while ( fit.nextFeature( f ) )
  {
    qDebug( "Getting non-spatial feature from layer" );
    myCount++;
  }
  QVERIFY( myCount == 3 );
}

void TestQgsVectorLayer::getValues()
{
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:real" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );
  QgsFeature f1( layer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 1 );
  QgsFeature f2( layer->dataProvider()->fields(), 2 );
  f2.setAttribute( QStringLiteral( "col1" ), 2 );
  QgsFeature f3( layer->dataProvider()->fields(), 3 );
  f3.setAttribute( QStringLiteral( "col1" ), 3 );
  QgsFeature f4( layer->dataProvider()->fields(), 4 );
  f4.setAttribute( QStringLiteral( "col1" ), QVariant() );
  layer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  //make a selection
  QgsFeatureIds ids;
  ids << f2.id() << f3.id();
  layer->selectByIds( ids );

  bool ok;
  QList<QVariant> varList = QgsVectorLayerUtils::getValues( layer, QStringLiteral( "col1" ), ok );
  QVERIFY( ok );
  QCOMPARE( varList.length(), 4 );
  std::sort( varList.begin(), varList.end() );
  QCOMPARE( varList.at( 0 ), QVariant() );
  QCOMPARE( varList.at( 1 ), QVariant( 1 ) );
  QCOMPARE( varList.at( 2 ), QVariant( 2 ) );
  QCOMPARE( varList.at( 3 ), QVariant( 3 ) );

  //check with selected features
  varList = QgsVectorLayerUtils::getValues( layer, QStringLiteral( "col1" ), ok, true );
  QVERIFY( ok );
  QCOMPARE( varList.length(), 2 );
  std::sort( varList.begin(), varList.end() );
  QCOMPARE( varList.at( 0 ), QVariant( 2 ) );
  QCOMPARE( varList.at( 1 ), QVariant( 3 ) );

  int nulls = 0;
  QList<double> doubleList = QgsVectorLayerUtils::getDoubleValues( layer, QStringLiteral( "col1" ), ok, false, &nulls );
  QVERIFY( ok );
  QCOMPARE( doubleList.length(), 3 );
  std::sort( doubleList.begin(), doubleList.end() );
  QCOMPARE( doubleList.at( 0 ), 1.0 );
  QCOMPARE( doubleList.at( 1 ), 2.0 );
  QCOMPARE( doubleList.at( 2 ), 3.0 );
  QCOMPARE( nulls, 1 );

  //check with selected features
  doubleList = QgsVectorLayerUtils::getDoubleValues( layer, QStringLiteral( "col1" ), ok, true, &nulls );
  QVERIFY( ok );
  std::sort( doubleList.begin(), doubleList.end() );
  QCOMPARE( doubleList.length(), 2 );
  QCOMPARE( doubleList.at( 0 ), 2.0 );
  QCOMPARE( doubleList.at( 1 ), 3.0 );
  QCOMPARE( nulls, 0 );

  QList<QVariant> expVarList = QgsVectorLayerUtils::getValues( layer, QStringLiteral( "tostring(col1) || ' '" ), ok );
  QVERIFY( ok );
  QCOMPARE( expVarList.length(), 4 );
  std::sort( expVarList.begin(), expVarList.end() );
  QCOMPARE( expVarList.at( 0 ), QVariant() );
  QCOMPARE( expVarList.at( 1 ).toString(), QString( "1 " ) );
  QCOMPARE( expVarList.at( 2 ).toString(), QString( "2 " ) );
  QCOMPARE( expVarList.at( 3 ).toString(), QString( "3 " ) );

  QList<double> expDoubleList = QgsVectorLayerUtils::getDoubleValues( layer, QStringLiteral( "col1 * 2" ), ok, false, &nulls );
  QVERIFY( ok );
  std::sort( expDoubleList.begin(), expDoubleList.end() );
  QCOMPARE( expDoubleList.length(), 3 );
  QCOMPARE( expDoubleList.at( 0 ), 2.0 );
  QCOMPARE( expDoubleList.at( 1 ), 4.0 );
  QCOMPARE( expDoubleList.at( 2 ), 6.0 );
  QCOMPARE( nulls, 1 );

  delete layer;
}

void TestQgsVectorLayer::setRenderer()
{
  const QSignalSpy spy( mpPointsLayer, &QgsVectorLayer::rendererChanged );

  QgsSingleSymbolRenderer *symbolRenderer = new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );

  mpPointsLayer->setRenderer( symbolRenderer );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( mpPointsLayer->renderer(), symbolRenderer );
}

void TestQgsVectorLayer::setFeatureBlendMode()
{
  const QSignalSpy spy( mpPointsLayer, &QgsVectorLayer::featureBlendModeChanged );

  mpPointsLayer->setFeatureBlendMode( QPainter::CompositionMode_Screen );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), static_cast< int >( QPainter::CompositionMode_Screen ) );
  QCOMPARE( mpPointsLayer->featureBlendMode(), QPainter::CompositionMode_Screen );
  mpPointsLayer->setFeatureBlendMode( QPainter::CompositionMode_Screen );
  QCOMPARE( spy.count(), 1 );

  mpPointsLayer->setFeatureBlendMode( QPainter::CompositionMode_Darken );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).toInt(), static_cast< int >( QPainter::CompositionMode_Darken ) );
  QCOMPARE( mpPointsLayer->featureBlendMode(), QPainter::CompositionMode_Darken );
}

void TestQgsVectorLayer::setLayerTransparency()
{
  const QSignalSpy spy( mpPointsLayer, &QgsMapLayer::opacityChanged );

  mpPointsLayer->setOpacity( 0.5 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).toDouble(), 0.5 );
  QCOMPARE( mpPointsLayer->opacity(), 0.5 );
  mpPointsLayer->setOpacity( 0.5 );
  QCOMPARE( spy.count(), 1 );
  mpPointsLayer->setOpacity( 1.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).toDouble(), 1.0 );
  QCOMPARE( mpPointsLayer->opacity(), 1.0 );
}

void TestQgsVectorLayer::uniqueValues()
{
  //test with invalid field
  const QSet<QVariant> values = mpPointsLayer->uniqueValues( 1000 );
  QCOMPARE( values.count(), 0 );
}

void TestQgsVectorLayer::minimumValue()
{
  //test with invalid field
  QCOMPARE( mpPointsLayer->minimumValue( 1000 ), QVariant() );
}

void TestQgsVectorLayer::maximumValue()
{
  //test with invalid field
  QCOMPARE( mpPointsLayer->maximumValue( 1000 ), QVariant() );
}

void TestQgsVectorLayer::minimumAndMaximumValue()
{
  //test with invalid field
  QVariant min;
  QVariant max;
  mpPointsLayer->minimumAndMaximumValue( 1000, min, max );
  QCOMPARE( min, QVariant() );
  QCOMPARE( max, QVariant() );
}

void TestQgsVectorLayer::isSpatial()
{
  QVERIFY( mpPointsLayer->isSpatial() );
  QVERIFY( mpPolysLayer->isSpatial() );
  QVERIFY( mpLinesLayer->isSpatial() );
  QVERIFY( !mpNonSpatialLayer->isSpatial() );
}

void TestQgsVectorLayer::testAddTopologicalPoints()
{
  // create a simple linestring layer

  QgsVectorLayer *layerLine = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( layerLine->isValid() );

  QgsPolylineXY line1;
  line1 << QgsPointXY( 2, 1 ) << QgsPointXY( 1, 1 ) << QgsPointXY( 1, 5 );
  QgsFeature lineF1;
  lineF1.setGeometry( QgsGeometry::fromPolylineXY( line1 ) );

  layerLine->startEditing();
  layerLine->addFeature( lineF1 );
  const QgsFeatureId fidLineF1 = lineF1.id();
  QCOMPARE( layerLine->featureCount(), ( long )1 );

  QCOMPARE( layerLine->undoStack()->index(), 1 );

  // outside of the linestring - nothing should happen
  int result = layerLine->addTopologicalPoints( QgsPoint( 2, 2 ) );
  QCOMPARE( result, 2 );

  QCOMPARE( layerLine->undoStack()->index(), 1 );
  QCOMPARE( layerLine->getFeature( fidLineF1 ).geometry().asWkt(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 5)" ).asWkt() );

  // add point at an existing vertex
  result = layerLine->addTopologicalPoints( QgsPoint( 1, 1 ) );
  QCOMPARE( result, 2 );

  QCOMPARE( layerLine->undoStack()->index(), 1 );
  QCOMPARE( layerLine->getFeature( fidLineF1 ).geometry().asWkt(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 5)" ).asWkt() );

  // add point on segment of linestring
  result = layerLine->addTopologicalPoints( QgsPoint( 1, 2 ) );
  QCOMPARE( result, 0 );

  QCOMPARE( layerLine->undoStack()->index(), 2 );
  QCOMPARE( layerLine->getFeature( fidLineF1 ).geometry().asWkt(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 2, 1 5)" ).asWkt() );

  // add points from disjoint geometry - nothing should happen
  result = layerLine->addTopologicalPoints( QgsGeometry::fromWkt( "LINESTRING(2 0, 2 1, 2 2)" ) );
  QCOMPARE( result, 2 );

  QCOMPARE( layerLine->undoStack()->index(), 2 );
  QCOMPARE( layerLine->getFeature( fidLineF1 ).geometry().asWkt(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 2, 1 5)" ).asWkt() );

  // add 2 out of 3 points from intersecting geometry
  result = layerLine->addTopologicalPoints( QgsGeometry::fromWkt( "LINESTRING(2 0, 2 1, 2 3, 1 3, 0 3, 0 4, 1 4)" ) );
  QCOMPARE( result, 0 );

  QCOMPARE( layerLine->undoStack()->index(), 2 );
  QCOMPARE( layerLine->getFeature( fidLineF1 ).geometry().asWkt(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 2, 1 3, 1 4, 1 5)" ).asWkt() );

  delete layerLine;

  // Test error results -1: layer error, 1: geometry error
  QgsVectorLayer *nonSpatialLayer = new QgsVectorLayer( QStringLiteral( "None" ), QStringLiteral( "non spatial layer" ), QStringLiteral( "memory" ) );
  QVERIFY( nonSpatialLayer->isValid() );

  result = nonSpatialLayer->addTopologicalPoints( QgsPoint( 2, 2 ) );
  QCOMPARE( result, -1 );  // Non editable

  nonSpatialLayer->startEditing();
  result = nonSpatialLayer->addTopologicalPoints( QgsPoint( 2, 2 ) );
  QCOMPARE( result, 1 );  // Non spatial

  delete nonSpatialLayer;

  QgsVectorLayer *layerPoint = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:27700" ), QStringLiteral( "layer point" ), QStringLiteral( "memory" ) );
  QVERIFY( layerPoint->isValid() );

  layerPoint->startEditing();
  result = layerPoint->addTopologicalPoints( QgsGeometry() );
  QCOMPARE( result, 1 );  // Null geometry

  delete layerPoint;

  QgsVectorLayer *layerInvalid = new QgsVectorLayer( QString(), QStringLiteral( "layer invalid" ), QStringLiteral( "none" ) );
  QVERIFY( !layerInvalid->isValid() );

  result = layerInvalid->addTopologicalPoints( QgsPoint( 2, 2 ) );
  QCOMPARE( result, -1 );  // Invalid layer

  delete layerInvalid;
}

void TestQgsVectorLayer::testCopyPasteFieldConfiguration_data()
{
  QTest::addColumn<QgsMapLayer::StyleCategories>( "categories" );
  // QTest::addColumn<double>( "flagExpected" );
  // QTest::addColumn<double>( "widgetExpected" );

  QTest::newRow( "forms_and_fields" ) << ( QgsMapLayer::Fields | QgsMapLayer::Forms );
  QTest::newRow( "forms" ) << QgsMapLayer::StyleCategories( QgsMapLayer::Forms );
  QTest::newRow( "fields" ) << QgsMapLayer::StyleCategories( QgsMapLayer::Fields );
  QTest::newRow( "none" ) << QgsMapLayer::StyleCategories( QgsMapLayer::StyleCategories() );
}

void TestQgsVectorLayer::testCopyPasteFieldConfiguration()
{
  QFETCH( QgsMapLayer::StyleCategories, categories );

  QgsVectorLayer layer1( QStringLiteral( "Point?field=name:string" ), QStringLiteral( "layer1" ), QStringLiteral( "memory" ) );
  QVERIFY( layer1.isValid() );
  QVERIFY( layer1.editorWidgetSetup( 0 ).type().isEmpty() );
  QCOMPARE( layer1.fieldConfigurationFlags( 0 ), QgsField::ConfigurationFlags() );

  layer1.setEditorWidgetSetup( 0, QgsEditorWidgetSetup( "ValueMap", QVariantMap() ) );
  QCOMPARE( layer1.editorWidgetSetup( 0 ).type(), QStringLiteral( "ValueMap" ) );
  layer1.setFieldConfigurationFlags( 0, QgsField::ConfigurationFlag::NotSearchable );
  QCOMPARE( layer1.fieldConfigurationFlags( 0 ), QgsField::ConfigurationFlag::NotSearchable );

  // export given categories, import all
  QString errorMsg;
  QDomDocument doc( QStringLiteral( "qgis" ) );
  const QgsReadWriteContext context;
  layer1.exportNamedStyle( doc, errorMsg, context, categories );
  QVERIFY( errorMsg.isEmpty() );

  QgsVectorLayer layer2( QStringLiteral( "Point?field=name:string" ), QStringLiteral( "layer2" ), QStringLiteral( "memory" ) );
  QVERIFY( layer2.isValid() );
  QVERIFY( layer2.editorWidgetSetup( 0 ).type().isEmpty() );
  QCOMPARE( layer2.fieldConfigurationFlags( 0 ), QgsField::ConfigurationFlags() );

  QVERIFY( layer2.importNamedStyle( doc, errorMsg ) );
  QCOMPARE( layer2.editorWidgetSetup( 0 ).type(), categories.testFlag( QgsMapLayer::Forms ) ? QStringLiteral( "ValueMap" ) : QString( "" ) );
  QCOMPARE( layer2.fieldConfigurationFlags( 0 ), categories.testFlag( QgsMapLayer::Fields ) ? QgsField::ConfigurationFlag::NotSearchable : QgsField::ConfigurationFlags() );

  // export all, import given categories
  QDomDocument doc2( QStringLiteral( "qgis" ) );
  layer1.exportNamedStyle( doc2, errorMsg, context );
  QVERIFY( errorMsg.isEmpty() );

  QgsVectorLayer layer3( QStringLiteral( "Point?field=name:string" ), QStringLiteral( "layer3" ), QStringLiteral( "memory" ) );
  QVERIFY( layer3.isValid() );
  QVERIFY( layer3.editorWidgetSetup( 0 ).type().isEmpty() );
  QCOMPARE( layer3.fieldConfigurationFlags( 0 ), QgsField::ConfigurationFlags() );

  QVERIFY( layer3.importNamedStyle( doc2, errorMsg, categories ) );
  QCOMPARE( layer3.editorWidgetSetup( 0 ).type(), categories.testFlag( QgsMapLayer::Forms ) ? QStringLiteral( "ValueMap" ) : QString( "" ) );
  QCOMPARE( layer3.fieldConfigurationFlags( 0 ), categories.testFlag( QgsMapLayer::Fields ) ? QgsField::ConfigurationFlag::NotSearchable : QgsField::ConfigurationFlags() );
}

void TestQgsVectorLayer::testFieldExpression()
{
  QgsVectorLayer layer1( QStringLiteral( "Point?field=name:string" ), QStringLiteral( "layer1" ), QStringLiteral( "memory" ) );
  QVERIFY( layer1.isValid() );

  layer1.addExpressionField( QStringLiteral( "'abc'" ), QgsField( QStringLiteral( "virtual_field" ), QVariant::String ) );

  QCOMPARE( layer1.expressionField( layer1.fields().lookupField( QStringLiteral( "virtual_field" ) ) ),  QStringLiteral( "'abc'" ) );
  QCOMPARE( layer1.expressionField( layer1.fields().lookupField( QStringLiteral( "name" ) ) ),  QString() );
}

QGSTEST_MAIN( TestQgsVectorLayer )
#include "testqgsvectorlayer.moc"
