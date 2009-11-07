/***************************************************************************
  testqgsvectoranalyzer.cpp
  --------------------------------------
Date                 : Sun Sep 16 12:22:49 AKDT 2007
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

//header for class being tested
#include <qgsgeometryanalyzer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>

class TestQgsVectorAnalyzer: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.
    /** Our tests proper begin here */
    void singleToMulti(  );
    void multiToSingle(  );
    void extractNodes(  );
    void polygonsToLines(  );
    void exportGeometryInfo(  );
    void simplifyGeometry(  );
    void polygonCentroids(  );
    void layerExtent(  );
  private:
    QgsGeometryAnalyzer mAnalyzer;
    QgsVectorLayer * mpLineLayer;
    QgsVectorLayer * mpPolyLayer;
    QgsVectorLayer * mpPointLayer;
    
};

void  TestQgsVectorAnalyzer::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(INSTALL_PREFIX, true);
  QgsApplication::showSettings();
  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance(QgsApplication::pluginPath());

  //create some objects that will be used in all tests...
  //create a map layer that will be used in all tests...
  QString myFileName (TEST_DATA_DIR); //defined in CmakeLists.txt
  QString myEndName = "lines.shp";
  myFileName = myFileName + QDir::separator() + myEndName;
  QFileInfo myLineInfo ( myFileName );
  mpLineLayer = new QgsVectorLayer ( myLineInfo.filePath(),
            myLineInfo.completeBaseName(), "ogr" );

  myEndName = "polys.shp";
  myFileName = myFileName + QDir::separator() + myEndName;
  QFileInfo myPolyInfo ( myFileName );
  mpPolyLayer = new QgsVectorLayer ( myPolyInfo.filePath(),
            myPolyInfo.completeBaseName(), "ogr" );

  myEndName = "points.shp";
  myFileName = myFileName + QDir::separator() + myEndName;
  QFileInfo myPointInfo ( myFileName );
  mpPointLayer = new QgsVectorLayer ( myPointInfo.filePath(),
            myPointInfo.completeBaseName(), "ogr" );
}
void  TestQgsVectorAnalyzer::cleanupTestCase()
{

}
void  TestQgsVectorAnalyzer::init() 
{

}
void  TestQgsVectorAnalyzer::cleanup() 
{

}

void TestQgsVectorAnalyzer::singleToMulti(  )
{

}
void TestQgsVectorAnalyzer::multiToSingle(  )
{

}
void TestQgsVectorAnalyzer::extractNodes(  )
{

}
void TestQgsVectorAnalyzer::polygonsToLines(  )
{

}
void TestQgsVectorAnalyzer::exportGeometryInfo(  )
{
}

void TestQgsVectorAnalyzer::simplifyGeometry(  )
{
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  QString myFileName = myTmpDir +  "simplify_layer.shp";
  QVERIFY( mAnalyzer.simplify( mpLineLayer,
                             myFileName,
                             1.0 ) );
}

void TestQgsVectorAnalyzer::polygonCentroids(  )
{
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  QString myFileName = myTmpDir +  "centroid_layer.shp";
  QVERIFY( mAnalyzer.centroids( mpPolyLayer, myFileName ) );
}

void TestQgsVectorAnalyzer::layerExtent(  )
{
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  QString myFileName = myTmpDir +  "extent_layer.shp";
  QVERIFY( mAnalyzer.extent( mpPointLayer, myFileName ) );
}

QTEST_MAIN( TestQgsVectorAnalyzer )
#include "moc_testqgsvectoranalyzer.cxx"

