/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim @ linfiniti.com
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
#include <QPainter>
#include <QSettings>
#include <QTime>
#include <iostream>

#include <QApplication>
#include <QDesktopServices>

//qgis includes...
#include <qgsvectorlayer.h> //defines QgsFieldMap 
#include <qgsvectorfilewriter.h> //logic for writing shpfiles
#include <qgsfeature.h> //we will need to pass a bunch of these for each rec
#include <qgsgeometry.h> //each feature needs a geometry
#include <qgspoint.h> //we will use point geometry
#include <qgscoordinatereferencesystem.h> //needed for creating a srs
#include <qgsapplication.h> //search path for srs.db
#include <qgsfield.h>
#include <qgis.h> //defines GEOWkt
#include <qgsmaprenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>

//qgs unit test utility class
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for the QgsMapRenderer class.
 * It will do some performance testing too
 *
 */
class TestQgsMapRenderer: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    /** This method tests render perfomance */
    void performanceTest();

  private:
    QString mEncoding;
    QgsVectorFileWriter::WriterError mError;
    QgsCoordinateReferenceSystem mCRS;
    QgsFieldMap mFields;
    QgsMapRenderer * mpMapRenderer;
    QgsMapLayer * mpPolysLayer;
    QString mReport;
};

void TestQgsMapRenderer::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath();
  QgsApplication::setPrefixPath( INSTALL_PREFIX, true );
  QgsApplication::showSettings();
  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );


  //create some objects that will be used in all tests...
  mEncoding = "UTF-8";
  QgsField myField1( "Value", QVariant::Int, "int", 10, 0, "Value on lon" );
  mFields.insert( 0, myField1 );
  mCRS = QgsCoordinateReferenceSystem( GEOWkt );
  //
  // Create the test dataset if it doesnt exist
  //
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataDir = myDataDir + QDir::separator();
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  QString myFileName = myTmpDir +  "maprender_testdata.shp";
  //copy over the default qml for our generated layer
  QString myQmlFileName = myTestDataDir +  "maprender_testdata.qml";
  QFile::copy( myQmlFileName, myTmpDir + "maprender_testdata.qml" );
  qDebug( "Checking test dataset exists...\n%s", myFileName.toLocal8Bit().constData() );
  if ( !QFile::exists( myFileName ) )
  {
    qDebug( "Creating test dataset: " );

    QgsVectorFileWriter myWriter( myFileName,
                                  mEncoding,
                                  mFields,
                                  QGis::WKBPolygon,
                                  &mCRS );
    double myInterval = 0.5;
    for ( double i = -180.0; i <= 180.0; i += myInterval )
    {
      for ( double j = -90.0; j <= 90.0; j += myInterval )
      {
        //
        // Create a polygon feature
        //
        QgsPolyline myPolyline;
        QgsPoint myPoint1 = QgsPoint( i, j );
        QgsPoint myPoint2 = QgsPoint( i + myInterval, j );
        QgsPoint myPoint3 = QgsPoint( i + myInterval, j + myInterval );
        QgsPoint myPoint4 = QgsPoint( i, j + myInterval );
        myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
        QgsPolygon myPolygon;
        myPolygon << myPolyline;
        //polygon: first item of the list is outer ring,
        // inner rings (if any) start from second item
        //
        // NOTE: don't delete this pointer again -
        // ownership is passed to the feature which will
        // delete it in its dtor!
        QgsGeometry * mypPolygonGeometry = QgsGeometry::fromPolygon( myPolygon );
        QgsFeature myFeature;
        myFeature.setTypeName( "WKBPolygon" );
        myFeature.setGeometry( mypPolygonGeometry );
        myFeature.addAttribute( 0, i );
        //
        // Write the feature to the filewriter
        // and check for errors
        //
        QVERIFY( myWriter.addFeature( myFeature ) );
        mError = myWriter.hasError();
        if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
        {
          std::cout << "Driver not found error" << std::endl;
        }
        else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
        {
          std::cout << "Create data source error" << std::endl;
        }
        else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
        {
          std::cout << "Create layer error" << std::endl;
        }
        QVERIFY( mError == QgsVectorFileWriter::NoError );
      }
    }
  } //file exists
  //
  //create a poly layer that will be used in all tests...
  //
  QFileInfo myPolyFileInfo( myFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer( mpPolysLayer );
  // add the test layer to the maprender
  mpMapRenderer = new QgsMapRenderer();
  QStringList myLayers;
  myLayers << mpPolysLayer->getLayerID();
  mpMapRenderer->setLayerSet( myLayers );
  mReport += "<h1>Map Render Tests</h1>\n";
}


void TestQgsMapRenderer::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "maprendertest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    QDesktopServices::openUrl( "file://" + myReportFile );
  }

}



void TestQgsMapRenderer::performanceTest()
{
  mpMapRenderer->setExtent( mpPolysLayer->extent() );
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataDir = myDataDir + QDir::separator();
  QgsRenderChecker myChecker;
  myChecker.setExpectedImage( myTestDataDir + "expected_maprender.png" );
  myChecker.setMapRenderer( mpMapRenderer );
  bool myResultFlag = myChecker.runTest( "maprender" );
  mReport += myChecker.report();
  QVERIFY( myResultFlag );
}


QTEST_MAIN( TestQgsMapRenderer )
#include "moc_testqgsmaprenderer.cxx"


