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
#include <iostream>

#include <QApplication>

#include <qgsvectorlayer.h> //defines QgsFieldMap 
#include <qgsvectorfilewriter.h> //logic for writing shpfiles
#include <qgsfeature.h> //we will need to pass a bunch of these for each rec
#include <qgsgeometry.h> //each feature needs a geometry
#include <qgspoint.h> //we will use point geometry
#include <qgsspatialrefsys.h> //needed for creating a srs
#include <qgsapplication.h> //search path for srs.db
#include <qgsfield.h>
#include <qgis.h> //defines GEOWKT

/** \ingroup UnitTests
 * This is a unit test for the QgsVectorFileWriter class.
 *
 *  Possible QVariant::Type s
 *   QVariant::String
 *   QVariant::Int
 *   QVariant::Double
 *   
 *   Allowed ogr prvider typeNames:
 *   Integer
 *   Real
 *   String
 * 
 *   Constructor for QgsField:
 *   QgsField::QgsField(QString name, 
 *                      QVariant::Type type, 
 *                      QString typeName, 
 *                      int len, 
 *                      int prec, 
 *                      QString comment)
 */
class TestQgsVectorFileWriter: public QObject
{
  Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(){};// will be called after the last testfunction was executed.
    void init(){};// will be called before each testfunction is executed.
    void cleanup(){};// will be called after every testfunction.

    /** This method tests writing a point to a shapefile */
    void createPoint();
    /** This method tests writing a polyline to a shapefile */
    void createLine();
    /** This method tests writing a polygon to a shapefile */
    void createPolygon();
    /** This method test writing multiple features to a shapefile */
    void polygonGridTest();
    /** As above but using a projected SRS*/
    void projectedPlygonGridTest();
    
  private:
    // a little util fn used by all tests
    bool cleanupFile(QString theFileBase);
    QString mEncoding;
    QgsVectorFileWriter::WriterError mError;
    QgsSpatialRefSys mSRS;
    QgsFieldMap mFields;
    QgsPoint mPoint1;
    QgsPoint mPoint2;
    QgsPoint mPoint3;
};

void TestQgsVectorFileWriter::initTestCase()
{
  qDebug("\n\n **************\n"
      "Note: if you get a message like \n"
      "ERROR 1: /tmp/testpt.shp is not a directory.\n"
      "It is caused by the /tmp/testshp.* files already existing\n"
      "(the ERROR comes from OGR and is not very intuitive)\n"
      "******************\n");
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(qgisPath, TRUE);
  //create some objects that will be used in all tests...

  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;

  mEncoding = "UTF-8";
  QgsField myField1("Field1",QVariant::String,"String",10,0,"Field 1 comment");
  mFields.insert(0, myField1);
  mSRS = QgsSpatialRefSys(GEOWKT);
  mPoint1 = QgsPoint(10.0,10.0);
  mPoint2 = QgsPoint(15.0,10.0);
  mPoint3 = QgsPoint(15.0,12.0);
}


void TestQgsVectorFileWriter::createPoint()
{

  //
  // Remove old copies that may be lying around
  //
  QString myFileBase = "/tmp/testpt";
  QVERIFY(cleanupFile(myFileBase));
  QString myFileName = myFileBase + ".shp";
  QgsVectorFileWriter myWriter (myFileName,
      mEncoding,
      mFields,
      QGis::WKBPoint,
      &mSRS);
  //
  // Create a feature
  //
  //
  // NOTE: dont delete this pointer again - 
  // ownership is passed to the feature which will
  // delete it in its dtor!
  QgsGeometry * mypPointGeometry = QgsGeometry::fromPoint(mPoint1);
  QgsFeature myFeature;
  myFeature.setGeometry(mypPointGeometry);
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

void TestQgsVectorFileWriter::createLine()
{
  //
  // Remove old copies that may be lying around
  //
  QString myFileBase = "/tmp/testln";
  QVERIFY(cleanupFile(myFileBase));
  QString myFileName = myFileBase + ".shp";
  QgsVectorFileWriter myWriter (myFileName,
      mEncoding,
      mFields,
      QGis::WKBLineString,
      &mSRS);
  //
  // Create a feature
  //
  QgsPolyline myPolyline;
  myPolyline << mPoint1 << mPoint2 << mPoint3;
  //
  // NOTE: dont delete this pointer again - 
  // ownership is passed to the feature which will
  // delete it in its dtor!
  QgsGeometry * mypLineGeometry = QgsGeometry::fromPolyline(myPolyline);
  QgsFeature myFeature;
  myFeature.setTypeName("WKBLineString");
  myFeature.setGeometry(mypLineGeometry);
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

void TestQgsVectorFileWriter::createPolygon()
{

  //
  // Remove old copies that may be lying around
  //
  QString myFileBase = "/tmp/testply";
  QVERIFY(cleanupFile(myFileBase));
  QString myFileName = myFileBase + ".shp";
  QgsVectorFileWriter myWriter (myFileName,
      mEncoding,
      mFields,
      QGis::WKBPolygon,
      &mSRS);
  //
  // Create a polygon feature
  //
  QgsPolyline myPolyline;
  myPolyline << mPoint1 << mPoint2 << mPoint3 << mPoint1;
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
void TestQgsVectorFileWriter::polygonGridTest()
{
  //
  // Remove old copies that may be lying around
  //
  QString myFileBase = "/tmp/testgrid";
  QVERIFY(cleanupFile(myFileBase));
  QString myFileName = myFileBase + ".shp";
  QgsVectorFileWriter myWriter (myFileName,
      mEncoding,
      mFields,
      QGis::WKBPolygon,
      &mSRS);
  double myInterval=5;
  for (int i=-180;i<=180;i+=myInterval)
  {
    for (int j=-90;j<=90;j+=myInterval)
    {
      //
      // Create a polygon feature
      //
      QgsPolyline myPolyline;
      QgsPoint myPoint1 = QgsPoint(static_cast<float>(i),static_cast<float>(j));
      QgsPoint myPoint2 = QgsPoint(static_cast<float>(i+myInterval),static_cast<float>(j));
      QgsPoint myPoint3 = QgsPoint(static_cast<float>(i+myInterval),static_cast<float>(j+myInterval));
      QgsPoint myPoint4 = QgsPoint(static_cast<float>(i),static_cast<float>(j+myInterval));
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
}

void TestQgsVectorFileWriter::projectedPlygonGridTest()
{
  //
  // Remove old copies that may be lying around
  //
  QString myFileBase = "/tmp/testprjgrid";
  QVERIFY(cleanupFile(myFileBase));
  QString myFileName = myFileBase + ".shp";
  //
  // We are testing projected coordinate 
  // system vector writing to lets use something fun...
  // Jamaica National Grid
  // QGIS SRSID: 1286
  // PostGIS SRID: 24200
  // +proj=lcc +lat_1=18 +lat_0=18 +lon_0=-77 +k_0=1 +x_0=250000 
  // +y_0=150000 +ellps=clrk66 +units=m +no_defs
  //
  mSRS = QgsSpatialRefSys(1286,QgsSpatialRefSys::QGIS_SRSID);
  QgsVectorFileWriter myWriter (myFileName,
      mEncoding,
      mFields,
      QGis::WKBPolygon,
      &mSRS);
  double myInterval=1000; //1km2
  for (int i=0;i<=10000;i+=myInterval) //10km
  {
    for (int j=0;j<=10000;j+=myInterval)//10km
    {
      //
      // Create a polygon feature
      //
      QgsPolyline myPolyline;
      QgsPoint myPoint1 = QgsPoint(static_cast<float>(i),static_cast<float>(j));
      QgsPoint myPoint2 = QgsPoint(static_cast<float>(i+myInterval),static_cast<float>(j));
      QgsPoint myPoint3 = QgsPoint(static_cast<float>(i+myInterval),static_cast<float>(j+myInterval));
      QgsPoint myPoint4 = QgsPoint(static_cast<float>(i),static_cast<float>(j+myInterval));
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
}
bool TestQgsVectorFileWriter::cleanupFile(QString theFileBase)
{
  //
  // Remove old copies that may be lying around
  //
  QFileInfo myInfo(theFileBase + ".shp");
  if (myInfo.exists())
  {
    if(!QFile::remove(theFileBase + ".shp"))
    {
      qDebug("Removing file failed : " + theFileBase.toLocal8Bit() + ".shp");
      return false;
    }
  }
  myInfo.setFile(theFileBase + ".shx");
  if (myInfo.exists())
  {
    if(!QFile::remove(theFileBase + ".shx"));
    {
      qDebug("Removing file failed : " + theFileBase.toLocal8Bit() + ".shx");
      return false;
    }
  }
  myInfo.setFile(theFileBase + ".dbf");
  if (myInfo.exists())
  {
    if(!QFile::remove(theFileBase + ".dbf"));
    {
      qDebug("Removing file failed : " + theFileBase.toLocal8Bit() + ".dbf");
      return false;
    }
  }
  myInfo.setFile(theFileBase + ".prj");
  if (myInfo.exists())
  {
    if(!QFile::remove(theFileBase + ".prj"));
    {
      qDebug("Removing file failed : " + theFileBase.toLocal8Bit() + ".prj");
      return false;
    }
  }
  return true;
}

QTEST_MAIN(TestQgsVectorFileWriter)
#include "moc_testqgsvectorfilewriter.cxx"

