/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
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
#include <QPainter>
#include <QSettings>
#include <QTime>
#include <iostream>

#include <QApplication>

//qgis includes...
#include <qgsvectorlayer.h> //defines QgsFieldMap 
#include <qgsvectorfilewriter.h> //logic for writing shpfiles
#include <qgsfeature.h> //we will need to pass a bunch of these for each rec
#include <qgsgeometry.h> //each feature needs a geometry
#include <qgspoint.h> //we will use point geometry
#include <qgsspatialrefsys.h> //needed for creating a srs
#include <qgsapplication.h> //search path for srs.db
#include <qgsfield.h>
#include <qgis.h> //defines GEOWKT
#include <qgsmaprender.h> 
#include <qgsmaplayer.h> 
#include <qgsvectorlayer.h> 
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>

/** \ingroup UnitTests
 * This is a unit test for the QgsMapRender class.
 * It will do some performance testing too
 *
 */
class TestQgsMapRender: public QObject
{
  Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void init(){};// will be called before each testfunction is executed.
    void cleanup(){};// will be called after every testfunction.

    /** This method tests render perfomance */
    void performanceTest();
    
  private:
    QString mEncoding;
    QgsVectorFileWriter::WriterError mError;
    QgsSpatialRefSys mSRS;
    QgsFieldMap mFields;
    QgsMapRender * mpMapRenderer;
    QgsMapLayer * mpPolysLayer;
};

void TestQgsMapRender::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(qgisPath, TRUE);
#ifdef Q_OS_LINUX
  QgsApplication::setPkgDataPath(qgisPath + "/../share/qgis");
  QgsApplication::setPluginPath(qgisPath + "/../lib/qgis");
#endif
  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance(QgsApplication::pluginPath());
  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;

  //create some objects that will be used in all tests...
  mEncoding = "UTF-8";
  QgsField myField1("Field1",QVariant::String,"String",10,0,"Field 1 comment");
  mFields.insert(0, myField1);
  mSRS = QgsSpatialRefSys(GEOWKT);
  //
  // Create the test dataset if it doesnt exist
  //
  QString myDataDir (TEST_DATA_DIR); //defined in CmakeLists.txt
  QString myTestDataDir = myDataDir + QDir::separator();
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  QString myFileName = myTmpDir +  "maprender_testdata.shp";
  qDebug ( "Checking test dataset exists...");
  qDebug ( myFileName );
  if (!QFile::exists(myFileName))
  {
    qDebug ( "Creating test dataset: ");
    
    QgsVectorFileWriter myWriter (myFileName,
        mEncoding,
        mFields,
        QGis::WKBPolygon,
        &mSRS);
    double myInterval=0.5;
    for (double i=-180.0;i<=180.0;i+=myInterval)
    {
      for (double j=-90.0;j<=90.0;j+=myInterval)
      {
        //
        // Create a polygon feature
        //
        QgsPolyline myPolyline;
        QgsPoint myPoint1 = QgsPoint(i,j);
        QgsPoint myPoint2 = QgsPoint(i+myInterval,j);
        QgsPoint myPoint3 = QgsPoint(i+myInterval,j+myInterval);
        QgsPoint myPoint4 = QgsPoint(i,j+myInterval);
        myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
        QgsPolygon myPolygon;
        myPolygon << myPolyline;
        //polygon: first item of the list is outer ring, 
        // inner rings (if any) start from second item 
        //
        // NOTE: dont delete this pointer again - 
        // ownership is passed to the feature which will
        // delete it in its dtor!
        QgsGeometry * mypPolygonGeometry = QgsGeometry::fromPolygon(myPolygon);
        QgsFeature myFeature;
        myFeature.setTypeName("WKBPolygon");
        myFeature.setGeometry(mypPolygonGeometry);
        myFeature.addAttribute(0,"HelloWorld");
        //
        // Write the feature to the filewriter
        // and check for errors
        //
        QVERIFY(myWriter.addFeature(myFeature));
        mError = myWriter.hasError();
        if(mError==QgsVectorFileWriter::ErrDriverNotFound)
        {
          std::cout << "Driver not found error" << std::endl;
        }
        else if (mError==QgsVectorFileWriter::ErrCreateDataSource)
        {
          std::cout << "Create data source error" << std::endl;
        }
        else if (mError==QgsVectorFileWriter::ErrCreateLayer)
        {
          std::cout << "Create layer error" << std::endl;
        }
        QVERIFY(mError==QgsVectorFileWriter::NoError);
      }
    }
  } //file exists
  //
  //create a poly layer that will be used in all tests...
  //
  QFileInfo myPolyFileInfo ( myFileName );
  mpPolysLayer = new QgsVectorLayer ( myPolyFileInfo.filePath(),
      myPolyFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer(mpPolysLayer);
  // add the test layer to the maprender
  mpMapRenderer = new QgsMapRender();
  QStringList myLayers;
  myLayers << mpPolysLayer->getLayerID();
  mpMapRenderer->setLayerSet(myLayers);
}

void TestQgsMapRender::performanceTest()
{

  //
  // Now render our layers onto a pixmap 
  //
  QPixmap myPixmap( 1800,900 );
  myPixmap.fill ( QColor ( "#98dbf9" ) );
  QPainter myPainter( &myPixmap );
  mpMapRenderer->setOutputSize( QSize ( 1800,900 ),72 ); 
  mpMapRenderer->setExtent(mpPolysLayer->extent());
  qDebug ("Extents set to:");
  qDebug (mpPolysLayer->extent().stringRep());
  QTime myTime;
  myTime.start();
  mpMapRenderer->render( &myPainter );
  qDebug ("Elapsed time in ms for render job: " + 
      QString::number ( myTime.elapsed() ).toLocal8Bit()); 
  myPainter.end();
  //
  // Save the pixmap to disk so the user can make a 
  // visual assessment if needed
  //
  myPixmap.save (QDir::tempPath() + QDir::separator() + "maprender_result.png");
}


QTEST_MAIN(TestQgsMapRender)
#include "moc_testqgsmaprender.cxx"


