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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

#include <iostream>
//qgis includes...
#include <qgsmaprenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgssinglesymbolrendererv2.h>
//qgis test includes
#include "qgsrenderchecker.h"

class TestSignalReceiver : public QObject
{
    Q_OBJECT

  public:
    TestSignalReceiver() : QObject( 0 ),
        rendererChanged( false ),
        featureBlendMode( QPainter::CompositionMode( 0 ) ),
        transparency( 0 )
    {}
    bool rendererChanged;
    QPainter::CompositionMode featureBlendMode;
    int transparency;
  public slots:
    void onRendererChanged()
    {
      rendererChanged = true;
    }
    void onFeatureBlendModeChanged( const QPainter::CompositionMode blendMode )
    {
      featureBlendMode = blendMode;
    }
    void onLayerTransparencyChanged( int layerTransparency )
    {
      transparency = layerTransparency;
    }
};

/** \ingroup UnitTests
 * This is a unit test for the vector layer class.
 */
class TestQgsVectorLayer : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorLayer()
        : mTestHasError( false )
        , mpMapRenderer( 0 )
        , mpPointsLayer( 0 )
        , mpLinesLayer( 0 )
        , mpPolysLayer( 0 )
        , mpNonSpatialLayer( 0 )
    {}

  private:
    bool mTestHasError;
    QgsMapRenderer * mpMapRenderer;
    QgsMapLayer * mpPointsLayer;
    QgsMapLayer * mpLinesLayer;
    QgsMapLayer * mpPolysLayer;
    QgsVectorLayer * mpNonSpatialLayer;
    QString mTestDataDir;
    QString mReport;

  private slots:


    // will be called before the first testfunction is executed.
    void initTestCase()
    {
      mTestHasError = false;
      QgsApplication::init();
      QgsApplication::initQgis();
      QgsApplication::showSettings();

      //create some objects that will be used in all tests...

      //
      //create a non spatial layer that will be used in all tests...
      //
      QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
      mTestDataDir = myDataDir + QDir::separator();
      QString myDbfFileName = mTestDataDir + "nonspatial.dbf";
      QFileInfo myDbfFileInfo( myDbfFileName );
      mpNonSpatialLayer = new QgsVectorLayer( myDbfFileInfo.filePath(),
                                              myDbfFileInfo.completeBaseName(), "ogr" );
      // Register the layer with the registry
      QgsMapLayerRegistry::instance()->addMapLayers(
        QList<QgsMapLayer *>() << mpNonSpatialLayer );
      //
      //create a point layer that will be used in all tests...
      //
      QString myPointsFileName = mTestDataDir + "points.shp";
      QFileInfo myPointFileInfo( myPointsFileName );
      mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                          myPointFileInfo.completeBaseName(), "ogr" );
      // Register the layer with the registry
      QgsMapLayerRegistry::instance()->addMapLayers(
        QList<QgsMapLayer *>() << mpPointsLayer );

      //
      //create a poly layer that will be used in all tests...
      //
      QString myPolysFileName = mTestDataDir + "polys.shp";
      QFileInfo myPolyFileInfo( myPolysFileName );
      mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                         myPolyFileInfo.completeBaseName(), "ogr" );
      // Register the layer with the registry
      QgsMapLayerRegistry::instance()->addMapLayers(
        QList<QgsMapLayer *>() << mpPolysLayer );


      //
      // Create a line layer that will be used in all tests...
      //
      QString myLinesFileName = mTestDataDir + "lines.shp";
      QFileInfo myLineFileInfo( myLinesFileName );
      mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                         myLineFileInfo.completeBaseName(), "ogr" );
      // Register the layer with the registry
      QgsMapLayerRegistry::instance()->addMapLayers(
        QList<QgsMapLayer *>() << mpLinesLayer );
      //
      // We only need maprender instead of mapcanvas
      // since maprender does not require a qui
      // and is more light weight
      //
      mpMapRenderer = new QgsMapRenderer();
      QStringList myLayers;
      myLayers << mpPointsLayer->id();
      myLayers << mpPolysLayer->id();
      myLayers << mpLinesLayer->id();
      mpMapRenderer->setLayerSet( myLayers );
      mReport += "<h1>Vector Renderer Tests</h1>\n";
    }
    // will be called after the last testfunction was executed.
    void cleanupTestCase()
    {
      QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
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
    void init() {}    // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void QgsVectorLayerNonSpatialIterator()
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

    void QgsVectorLayerGetValues()
    {
      QgsVectorLayer* layer = new QgsVectorLayer( "Point?field=col1:real", "layer", "memory" );
      QVERIFY( layer->isValid() );
      QgsFeature f1( layer->dataProvider()->fields(), 1 );
      f1.setAttribute( "col1", 1 );
      QgsFeature f2( layer->dataProvider()->fields(), 2 );
      f2.setAttribute( "col1", 2 );
      QgsFeature f3( layer->dataProvider()->fields(), 3 );
      f3.setAttribute( "col1", 3 );
      QgsFeature f4( layer->dataProvider()->fields(), 4 );
      f4.setAttribute( "col1", QVariant() );
      layer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

      //make a selection
      QgsFeatureIds ids;
      ids << f2.id() << f3.id();
      layer->setSelectedFeatures( ids );

      bool ok;
      QList<QVariant> varList = layer->getValues( "col1", ok );
      QVERIFY( ok );
      QCOMPARE( varList.length(), 4 );
      QCOMPARE( varList.at( 0 ), QVariant( 1 ) );
      QCOMPARE( varList.at( 1 ), QVariant( 2 ) );
      QCOMPARE( varList.at( 2 ), QVariant( 3 ) );
      QCOMPARE( varList.at( 3 ), QVariant() );

      //check with selected features
      varList = layer->getValues( "col1", ok, true );
      QVERIFY( ok );
      QCOMPARE( varList.length(), 2 );
      QCOMPARE( varList.at( 0 ), QVariant( 2 ) );
      QCOMPARE( varList.at( 1 ), QVariant( 3 ) );

      int nulls = 0;
      QList<double> doubleList = layer->getDoubleValues( "col1", ok, false, &nulls );
      QVERIFY( ok );
      QCOMPARE( doubleList.length(), 3 );
      QCOMPARE( doubleList.at( 0 ), 1.0 );
      QCOMPARE( doubleList.at( 1 ), 2.0 );
      QCOMPARE( doubleList.at( 2 ), 3.0 );
      QCOMPARE( nulls, 1 );

      //check with selected features
      doubleList = layer->getDoubleValues( "col1", ok, true, &nulls );
      QVERIFY( ok );
      QCOMPARE( doubleList.length(), 2 );
      QCOMPARE( doubleList.at( 0 ), 2.0 );
      QCOMPARE( doubleList.at( 1 ), 3.0 );
      QCOMPARE( nulls, 0 );

      QList<QVariant> expVarList = layer->getValues( "tostring(col1) || ' '", ok );
      QVERIFY( ok );
      QCOMPARE( expVarList.length(), 4 );
      QCOMPARE( expVarList.at( 0 ).toString(), QString( "1 " ) );
      QCOMPARE( expVarList.at( 1 ).toString(), QString( "2 " ) );
      QCOMPARE( expVarList.at( 2 ).toString(), QString( "3 " ) );
      QCOMPARE( expVarList.at( 3 ), QVariant() );

      QList<double> expDoubleList = layer->getDoubleValues( "col1 * 2", ok, false, &nulls );
      QVERIFY( ok );
      QCOMPARE( expDoubleList.length(), 3 );
      QCOMPARE( expDoubleList.at( 0 ), 2.0 );
      QCOMPARE( expDoubleList.at( 1 ), 4.0 );
      QCOMPARE( expDoubleList.at( 2 ), 6.0 );
      QCOMPARE( nulls, 1 );

      delete layer;
    }

    void QgsVectorLayerstorageType() {}
    void QgsVectorLayercapabilitiesString() {}
    void QgsVectorLayerdataComment() {}
    void QgsVectorLayerproviderType() {}
    void QgsVectorLayersetDisplayField() {}
    void QgsVectorLayerdrawLabels() {}
    void QgsVectorLayerdrawLineString() {}
    void QgsVectorLayerdrawPolygon() {}
    void QgsVectorLayerdrawRendererV2() {}
    void QgsVectorLayerdrawRendererV2Levels() {}
    void QgsVectorLayerreload() {}
    void QgsVectorLayerdraw() {}
    void QgsVectorLayerdeleteCachedGeometries() {}
    void QgsVectorLayerdrawVertexMarker() {}
    void QgsVectorLayerselect() {}
    void QgsVectorLayerinvertSelection() {}
    void QgsVectorLayerinvertSelectionInRectangle() {}
    void QgsVectorLayerremoveSelection() {}
    void QgsVectorLayertriggerRepaint() {}
    void QgsVectorLayerdataProvider() {}
    void QgsVectorLayersetProviderEncoding() {}
    void QgsVectorLayerrenderer() {}
    void QgsVectorLayersetRenderer() {}
    void QgsVectorLayergeometryType() {}
    void QgsVectorLayerwkbType() {}
    void QgsVectorLayerboundingBoxOfSelected() {}
    void QgsVectorLayerfeatureCount() {}
    void QgsVectorLayerupdateFeatureCount() {}
    void QgsVectorLayerupdateExtents() {}
    void QgsVectorLayersubsetString() {}
    void QgsVectorLayersetSubsetString() {}
    void QgsVectorLayerupdateFeatureAttributes() {}
    void QgsVectorLayerupdateFeatureGeometry() {}
    void QgsVectorLayernextFeature() {}
    void QgsVectorLayerfeatureAtId() {}
    void QgsVectorLayeraddFeature() {}
    void QgsVectorLayerinsertVertex() {}
    void QgsVectorLayermoveVertex() {}
    void QgsVectorLayerdeleteVertex() {}
    void QgsVectorLayerdeleteSelectedFeatures() {}
    void QgsVectorLayeraddRing() {}
    void QgsVectorLayeraddIsland() {}
    void QgsVectorLayertranslateFeature() {}
    void QgsVectorLayersplitFeatures() {}
    void QgsVectorLayerremovePolygonIntersections() {}
    void QgsVectorLayeraddTopologicalPoints() {}
    void QgsVectorLayerlabel() {}
    void QgsVectorLayerenableLabels() {}
    void QgsVectorLayerhasLabelsEnabled() {}
    void QgsVectorLayerstartEditing() {}
    void QgsVectorLayerreadXml() {}
    void QgsVectorLayersetDataProvider() {}
    void QgsVectorLayerwriteXml() {}
    void QgsVectorLayerreadSymbology() {}
    void QgsVectorLayerwriteSymbology() {}
    void QgsVectorLayerchangeGeometry() {}
    void QgsVectorLayerchangeAttributeValue() {}
    void QgsVectorLayeraddAttribute() {}
    void QgsVectorLayeraddAttributeAlias() {}
    void QgsVectorLayerattributeAlias() {}
    void QgsVectorLayerattributeDisplayName() {}
    void QgsVectorLayerdeleteAttribute() {}
    void QgsVectorLayerdeleteFeature() {}
    void QgsVectorLayerpendingFields() {}
    void QgsVectorLayerpendingAllAttributesList() {}
    void QgsVectorLayerpendingFeatureCount() {}
    void QgsVectorLayercommitChanges() {}
    void QgsVectorLayercommitErrors() {}
    void QgsVectorLayerrollBack() {}
    void QgsVectorLayersetSelectedFeatures() {}
    void QgsVectorLayerselectedFeatureCount() {}
    void QgsVectorLayerselectedFeaturesIds() {}
    void QgsVectorLayerselectedFeatures() {}
    void QgsVectorLayeraddFeatures() {}
    void QgsVectorLayercopySymbologySettings() {}
    void QgsVectorLayerhasCompatibleSymbology() {}
    void QgsVectorLayersnapPoint() {}
    void QgsVectorLayersnapWithContext() {}
    void QgsVectorLayersnapToGeometry() {}
    void QgsVectorLayerinsertSegmentVerticesForSnap() {}
    void QgsVectorLayerboundingBoxFromPointList() {}
    void QgsVectorLayercurrentVertexMarkerType() {}
    void QgsVectorLayercurrentVertexMarkerSize() {}
    void QgsVectorLayerdrawFeature() {}
    void QgsVectorLayersetCoordinateSystem() {}
    void QgsVectorLayertransformPoint() {}
    void QgsVectorLayertransformPoints() {}
    void QgsVectorLayerdisplayField() {}
    void QgsVectorLayerisEditable() {}
    void QgsVectorLayerisModified() {}
    void QgsVectorLayersetModified() {}
    void QgsVectorLayereditType() {}
    void QgsVectorLayersetEditType() {}
    void QgsVectorLayereditForm() {}
    void QgsVectorLayersetEditForm() {}
    void QgsVectorLayersetAnnotationForm() {}
    void QgsVectorLayereditFormInit() {}
    void QgsVectorLayersetEditFormInit() {}
    void QgsVectorLayervalueMap() {}
    void QgsVectorLayerrange() {}
    void QgsVectorLayeraddOverlay() {}
    void QgsVectorLayerremoveOverlay() {}
    void QgsVectorLayervectorOverlays() {}
    void QgsVectorLayerfindOverlayByType() {}
    void QgsVectorLayerrendererV2() {}

    void QgsVectorLayersetRendererV2()
    {
      QgsVectorLayer* vLayer = static_cast< QgsVectorLayer * >( mpPointsLayer );
      TestSignalReceiver receiver;
      QObject::connect( vLayer, SIGNAL( rendererChanged() ),
                        &receiver, SLOT( onRendererChanged() ) );
      QgsSingleSymbolRendererV2* symbolRenderer = new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( QGis::Point ) );

      QCOMPARE( receiver.rendererChanged, false );
      vLayer->setRendererV2( symbolRenderer );
      QCOMPARE( receiver.rendererChanged, true );
      QCOMPARE( vLayer->rendererV2(), symbolRenderer );
    }

    void QgsVectorLayerisUsingRendererV2() {}
    void QgsVectorLayersetUsingRendererV2() {}
    void QgsVectorLayereditGeometryChange() {}
    void QgsVectorLayereditFeatureAdd() {}
    void QgsVectorLayereditFeatureDelete() {}
    void QgsVectorLayereditAttributeChange() {}
    void QgsVectorLayerbeginEditCommand() {}
    void QgsVectorLayerendEditCommand() {}
    void QgsVectorLayerdestroyEditCommand() {}
    void QgsVectorLayerredoEditCommand() {}
    void QgsVectorLayerundoEditCommand() {}
    void QgsVectorLayersetCheckedState() {}
    void QgsVectorLayercheckedState() {}
    void QgsVectorLayerfieldNameIndex() {}
    void QgsVectorLayerstopRendererV2() {}

    void QgsVectorLayersetFeatureBlendMode()
    {
      QgsVectorLayer *vLayer = static_cast< QgsVectorLayer * >( mpPointsLayer );
      TestSignalReceiver receiver;
      QObject::connect( vLayer, SIGNAL( featureBlendModeChanged( const QPainter::CompositionMode ) ),
                        &receiver, SLOT( onFeatureBlendModeChanged( const QPainter::CompositionMode ) ) );

      QCOMPARE( int( receiver.featureBlendMode ), 0 );
      vLayer->setFeatureBlendMode( QPainter::CompositionMode_Screen );
      QCOMPARE( receiver.featureBlendMode, QPainter::CompositionMode_Screen );
      QCOMPARE( vLayer->featureBlendMode(), QPainter::CompositionMode_Screen );
    }

    void QgsVectorLayersetLayerTransparency()
    {
      QgsVectorLayer* vLayer = static_cast< QgsVectorLayer * >( mpPointsLayer );
      TestSignalReceiver receiver;
      QObject::connect( vLayer, SIGNAL( layerTransparencyChanged( int ) ),
                        &receiver, SLOT( onLayerTransparencyChanged( int ) ) );

      QCOMPARE( receiver.transparency, 0 );
      vLayer->setLayerTransparency( 50 );
      QCOMPARE( receiver.transparency, 50 );
      QCOMPARE( vLayer->layerTransparency(), 50 );
    }
};

QTEST_MAIN( TestQgsVectorLayer )
#include "testqgsvectorlayer.moc"
