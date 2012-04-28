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
#include <QtTest>
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
//qgis test includes
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for the vector layer class.
 */
class TestQgsVectorLayer: public QObject
{
    Q_OBJECT;
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

    }
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void QgsVectorLayerNonSpatialIterator()
    {
      QgsVectorDataProvider * myProvider = mpNonSpatialLayer->dataProvider();
      QgsFeature f;
      QgsAttributeList myList;
      myList << 0 << 1 << 2 << 3;
      int myCount = 0;
      myProvider->select( myList );
      while ( myProvider->nextFeature( f ) )
      {
        qDebug( "Getting feature from provider" );
        myCount++;
      }
      QVERIFY( myCount == 3 );
    };

    void QgsVectorLayerstorageType()
    {

    };
    void QgsVectorLayercapabilitiesString()
    {

    };
    void QgsVectorLayerdataComment()
    {

    };
    void QgsVectorLayerproviderType()
    {

    };
    void QgsVectorLayersetDisplayField()
    {

    };
    void QgsVectorLayerdrawLabels()
    {

    };
    void QgsVectorLayerdrawLineString()
    {

    };
    void QgsVectorLayerdrawPolygon()
    {

    };
    void QgsVectorLayerdrawRendererV2()
    {

    };
    void QgsVectorLayerdrawRendererV2Levels()
    {

    };
    void QgsVectorLayerreload()
    {

    };
    void QgsVectorLayerdraw()
    {

    };
    void QgsVectorLayerdeleteCachedGeometries()
    {

    };
    void QgsVectorLayerdrawVertexMarker()
    {

    };
    void QgsVectorLayerselect()
    {

    };
    void QgsVectorLayerinvertSelection()
    {

    };
    void QgsVectorLayerinvertSelectionInRectangle()
    {

    };
    void QgsVectorLayerremoveSelection()
    {

    };
    void QgsVectorLayertriggerRepaint()
    {

    };
    void QgsVectorLayerdataProvider()
    {

    };
    void QgsVectorLayersetProviderEncoding()
    {

    };
    void QgsVectorLayerrenderer()
    {

    };
    void QgsVectorLayersetRenderer()
    {

    };
    void QgsVectorLayergeometryType()
    {

    };
    void QgsVectorLayerwkbType()
    {

    };
    void QgsVectorLayerboundingBoxOfSelected()
    {

    };
    void QgsVectorLayerfeatureCount()
    {

    };
    void QgsVectorLayerupdateFeatureCount()
    {

    };
    void QgsVectorLayerupdateExtents()
    {

    };
    void QgsVectorLayersubsetString()
    {

    };
    void QgsVectorLayersetSubsetString()
    {

    };
    void QgsVectorLayerupdateFeatureAttributes()
    {

    };
    void QgsVectorLayerupdateFeatureGeometry()
    {

    };
    void QgsVectorLayernextFeature()
    {

    };
    void QgsVectorLayerfeatureAtId()
    {

    };
    void QgsVectorLayeraddFeature()
    {

    };
    void QgsVectorLayerinsertVertex()
    {

    };
    void QgsVectorLayermoveVertex()
    {

    };
    void QgsVectorLayerdeleteVertex()
    {

    };
    void QgsVectorLayerdeleteSelectedFeatures()
    {

    };
    void QgsVectorLayeraddRing()
    {

    };
    void QgsVectorLayeraddIsland()
    {

    };
    void QgsVectorLayertranslateFeature()
    {

    };
    void QgsVectorLayersplitFeatures()
    {

    };
    void QgsVectorLayerremovePolygonIntersections()
    {

    };
    void QgsVectorLayeraddTopologicalPoints()
    {

    };
    void QgsVectorLayerlabel()
    {

    };
    void QgsVectorLayerenableLabels()
    {

    };
    void QgsVectorLayerhasLabelsEnabled()
    {

    };
    void QgsVectorLayerstartEditing()
    {

    };
    void QgsVectorLayerreadXml()
    {

    };
    void QgsVectorLayersetDataProvider()
    {

    };
    void QgsVectorLayerwriteXml()
    {

    };
    void QgsVectorLayerreadSymbology()
    {

    };
    void QgsVectorLayerwriteSymbology()
    {

    };
    void QgsVectorLayerchangeGeometry()
    {

    };
    void QgsVectorLayerchangeAttributeValue()
    {

    };
    void QgsVectorLayeraddAttribute()
    {

    };
    void QgsVectorLayeraddAttributeAlias()
    {

    };
    void QgsVectorLayerattributeAlias()
    {

    };
    void QgsVectorLayerattributeDisplayName()
    {

    };
    void QgsVectorLayerdeleteAttribute()
    {

    };
    void QgsVectorLayerdeleteFeature()
    {

    };
    void QgsVectorLayerpendingFields()
    {

    };
    void QgsVectorLayerpendingAllAttributesList()
    {

    };
    void QgsVectorLayerpendingFeatureCount()
    {

    };
    void QgsVectorLayercommitChanges()
    {

    };
    void QgsVectorLayercommitErrors()
    {

    };
    void QgsVectorLayerrollBack()
    {

    };
    void QgsVectorLayersetSelectedFeatures()
    {

    };
    void QgsVectorLayerselectedFeatureCount()
    {

    };
    void QgsVectorLayerselectedFeaturesIds()
    {

    };
    void QgsVectorLayerselectedFeatures()
    {

    };
    void QgsVectorLayeraddFeatures()
    {

    };
    void QgsVectorLayercopySymbologySettings()
    {

    };
    void QgsVectorLayerhasCompatibleSymbology()
    {

    };
    void QgsVectorLayersnapPoint()
    {

    };
    void QgsVectorLayersnapWithContext()
    {

    };
    void QgsVectorLayersnapToGeometry()
    {

    };
    void QgsVectorLayerinsertSegmentVerticesForSnap()
    {

    };
    void QgsVectorLayerboundingBoxFromPointList()
    {

    };
    void QgsVectorLayercurrentVertexMarkerType()
    {

    };
    void QgsVectorLayercurrentVertexMarkerSize()
    {

    };
    void QgsVectorLayerdrawFeature()
    {

    };
    void QgsVectorLayersetCoordinateSystem()
    {

    };
    void QgsVectorLayertransformPoint()
    {

    };
    void QgsVectorLayertransformPoints()
    {

    };
    void QgsVectorLayerdisplayField()
    {

    };
    void QgsVectorLayerisEditable()
    {

    };
    void QgsVectorLayerisModified()
    {

    };
    void QgsVectorLayersetModified()
    {

    };
    void QgsVectorLayereditType()
    {

    };
    void QgsVectorLayersetEditType()
    {

    };
    void QgsVectorLayereditForm()
    {

    };
    void QgsVectorLayersetEditForm()
    {

    };
    void QgsVectorLayersetAnnotationForm()
    {

    };
    void QgsVectorLayereditFormInit()
    {

    };
    void QgsVectorLayersetEditFormInit()
    {

    };
    void QgsVectorLayervalueMap()
    {

    };
    void QgsVectorLayerrange()
    {

    };
    void QgsVectorLayeraddOverlay()
    {

    };
    void QgsVectorLayerremoveOverlay()
    {

    };
    void QgsVectorLayervectorOverlays()
    {

    };
    void QgsVectorLayerfindOverlayByType()
    {

    };
    void QgsVectorLayerrendererV2()
    {

    };
    void QgsVectorLayersetRendererV2()
    {

    };
    void QgsVectorLayerisUsingRendererV2()
    {

    };
    void QgsVectorLayersetUsingRendererV2()
    {

    };
    void QgsVectorLayereditGeometryChange()
    {

    };
    void QgsVectorLayereditFeatureAdd()
    {

    };
    void QgsVectorLayereditFeatureDelete()
    {

    };
    void QgsVectorLayereditAttributeChange()
    {

    };
    void QgsVectorLayerbeginEditCommand()
    {

    };
    void QgsVectorLayerendEditCommand()
    {

    };
    void QgsVectorLayerdestroyEditCommand()
    {

    };
    void QgsVectorLayerredoEditCommand()
    {

    };
    void QgsVectorLayerundoEditCommand()
    {

    };
    void QgsVectorLayersetCheckedState()
    {

    };
    void QgsVectorLayercheckedState()
    {

    };
    void QgsVectorLayerfieldNameIndex()
    {

    };
    void QgsVectorLayerstopRendererV2()
    {

    };

};

QTEST_MAIN( TestQgsVectorLayer )
#include "moc_testqgsvectorlayer.cxx"
