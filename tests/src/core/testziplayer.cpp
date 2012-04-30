/***************************************************************************
     testziplayer.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2012 Etienne Tourigny and Tim Sutton
    Email                : etourigny.dev@gmail.com
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
#include <QObject>
#include <QApplication>
#include <QFileInfo>

//qgis includes...
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsdataitem.h>
#include "qgsconfig.h"
#include <qgsrenderer.h>
#include <qgsuniquevaluerenderer.h>

#include <gdal.h>

/** \ingroup UnitTests
 * This is a unit test to verify that zip vector layers work
 */
class TestZipLayer: public QObject
{
    Q_OBJECT;

  private:

    QString mDataDir;
    QSettings mSettings;
    int mMaxScanZipSetting;
    int mScanZipSetting;

    // get map layer using Passthru
    QgsMapLayer * getLayer( QString myPath, QString myName, QString myProviderKey );
    bool testZipItemPassthru( QString myFileName, QString myProviderKey );
    // get map layer using QgsZipItem (only 1 child)
    QgsMapLayer * getZipLayer( QString myPath, QString myName );
    // test item(s) in zip item (supply name or test all)
    bool testZipItem( QString myFileName, QString myChildName );
    // get layer transparency to test for .qml loading
    int getLayerTransparency( QString myFileName, QString myProviderKey, int myScanZipSetting = 1 );

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    // tests
    // test for .zip and .gz files using all options
    void testPassthruVectorZip();
    void testPassthruVectorGzip();
    void testPassthruRasterZip();
    void testPassthruRasterGzip();
    // test both "Basic Scan" and "Full scan" for .zip files
    void testZipItemRaster();
    void testZipItemVector();
    void testZipItemAll();
    // test that styles are loaded from .qml files outside zip files
    void testZipItemVectorTransparency();
    void testGipItemVectorTransparency();
    void testZipItemRasterTransparency();
    void testGZipItemRasterTransparency();
};


QgsMapLayer *TestZipLayer::getLayer( QString myPath, QString myName, QString myProviderKey )
{
  if ( myName == "" )
  {
    QFileInfo myFileInfo( myPath );
    myName = myFileInfo.completeBaseName();
  }
  QgsMapLayer *myLayer = NULL;

  if ( myProviderKey == "ogr" )
  {
    myLayer = new QgsVectorLayer( myPath, myName, "ogr" );
  }
  else if ( myProviderKey == "gdal" )
  {
    myLayer = new QgsRasterLayer( myPath, myName, "gdal" );
  }
  // item should not have other provider key, but if it does will return NULL

  return myLayer;
}

QgsMapLayer *TestZipLayer::getZipLayer( QString myPath, QString myName )
{
  QgsMapLayer *myLayer = NULL;
  QgsDirectoryItem *dirItem = new QgsDirectoryItem( NULL, "/", "" );
  QgsDataItem* myItem = QgsZipItem::itemFromPath( dirItem, myPath, myName );
  if ( myItem )
  {
    QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem*>( myItem );
    if ( layerItem )
      myLayer = getLayer( layerItem->path(), layerItem->name(), layerItem->providerKey() );
  }
  delete dirItem;
  return myLayer;
}

bool TestZipLayer::testZipItemPassthru( QString myFileName, QString myProviderKey )
{
  QgsMapLayer * myLayer = getLayer( myFileName, "", myProviderKey );
  bool ok = myLayer && myLayer->isValid();
  if ( myLayer )
    delete myLayer;
  return ok;
}

bool TestZipLayer::testZipItem( QString myFileName, QString myChildName = "" )
{
  QgsDebugMsg( QString( "\n=======================================\nfile = %1 name = %2" ).arg( myFileName ).arg( myChildName ) );
  QFileInfo myFileInfo( myFileName );
  QgsZipItem *myZipItem = new QgsZipItem( NULL, myFileInfo.fileName(), myFileName );
  myZipItem->populate();
  bool ok = false;
  QVector<QgsDataItem*> myChildren = myZipItem->children();

  if ( myChildren.size() > 0 )
  {
    QgsDebugMsg( QString( "has %1 items" ).arg( myChildren.size() ) );
    foreach( QgsDataItem* item, myChildren )
    {
      QgsDebugMsg( QString( "child name=%1" ).arg( item->name() ) );
      QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem*>( item );
      if ( layerItem )
      {
        QgsDebugMsg( QString( "child name=%1 provider=%2 path=%3" ).arg( layerItem->name() ).arg( layerItem->providerKey() ).arg( layerItem->path() ) );
        if ( myChildName == "" || myChildName == item->name() )
        {
          QgsMapLayer* layer = getLayer( layerItem->path(), layerItem->name(), layerItem->providerKey() );
          if ( layer != NULL )
          {
            // we got a layer, check if it is valid and exit
            // if no child name given in argument, then pass to next one (unless current child is invalid)
            QgsDebugMsg( QString( "valid: %1" ).arg( layer->isValid() ) );
            ok = layer->isValid();
            delete layer;
            if ( ! ok )
            {
              QWARN( QString( "Invalid layer %1" ).arg( layerItem->path() ).toLocal8Bit().data() );
            }
            if ( myChildName == "" )
            {
              if ( ! ok )
                break;
            }
            else
            {
              break;
            }
          }
          else
          {
            QWARN( QString( "Invalid layer %1" ).arg( layerItem->path() ).toLocal8Bit().data() );
            break;
          }
        }
      }
      else
      {
        QWARN( QString( "Invalid layer %1" ).arg( layerItem->path() ).toLocal8Bit().data() );
        break;
      }
    }
  }
  delete myZipItem;
  return ok;
}

int TestZipLayer::getLayerTransparency( QString myFileName, QString myProviderKey, int myScanZipSetting )
{
  int myTransparency = -1;
  mSettings.setValue( "/qgis/scanZipInBrowser", myScanZipSetting );
  QgsMapLayer * myLayer = NULL;
  if ( myScanZipSetting == 1 )
    myLayer = getLayer( myFileName, "", myProviderKey );
  else
    myLayer = getZipLayer( myFileName, "" );
  if ( myLayer && myLayer->isValid() )
    myTransparency = myLayer->getTransparency();
  if ( myLayer )
    delete myLayer;
  return myTransparency;
}


// slots
void TestZipLayer::initTestCase()
{
  QgsApplication::init();
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );
  // save data dir
  mDataDir = QString( TEST_DATA_DIR ) + QDir::separator();
  // set zipSetting to 1 (Passthru) and save current value
  mScanZipSetting = mSettings.value( "/qgis/scanZipInBrowser", 1 ).toInt();
  mSettings.setValue( "/qgis/scanZipInBrowser", 1 );
  // max zipSetting value, depending on zlib presence
  mMaxScanZipSetting = 1;
#ifdef HAVE_ZLIB
  mMaxScanZipSetting = 3;
#endif

}

void TestZipLayer::cleanupTestCase()
{
  // restore zipSetting
  mSettings.setValue( "/qgis/scanZipInBrowser", mScanZipSetting );
}


void TestZipLayer::testPassthruVectorZip()
{
  QString myFileName = mDataDir + "points2.zip";
  QgsDebugMsg( "GDAL: " + QString( GDAL_RELEASE_NAME ) );
#if GDAL_VERSION_NUM < 1800
  myFileName = "/vsizip/" + myFileName + "/points.shp";
#endif
  QgsDebugMsg( "FILE: " + QString( myFileName ) );
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testZipItemPassthru( myFileName, "ogr" ) );
  }
}

void TestZipLayer::testPassthruVectorGzip()
{
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testZipItemPassthru( mDataDir + "points3.geojson.gz", "ogr" ) );
  }
}

void TestZipLayer::testPassthruRasterZip()
{
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.zip", "gdal" ) );
  }
}

void TestZipLayer::testPassthruRasterGzip()
{
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.tif.gz", "gdal" ) );
  }
}

void TestZipLayer::testZipItemRaster()
{
#ifndef HAVE_ZLIB
  QSKIP( "This test requires ZLIB", SkipSingle );
#endif

  for ( int i = 2 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testZipItem( mDataDir + "testzip.zip", "landsat_b1.tif" ) );
  }
}

void TestZipLayer::testZipItemVector()
{
#ifndef HAVE_ZLIB
  QSKIP( "This test requires ZLIB", SkipSingle );
#endif
  for ( int i = 2 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testZipItem( mDataDir + "testzip.zip", "points.shp" ) );
  }
}

void TestZipLayer::testZipItemAll()
{
#ifndef HAVE_ZLIB
  QSKIP( "This test requires ZLIB", SkipSingle );
#endif
  // test file contains invalid items (tmp1.tif, tmp1.txt and tmp1.xml)
  // test for all items inside zip, using zipSetting 3 (Full Scan) which will ignore invalid items
  // using zipSetting 2 (Basic Scan) would raise errors, because QgsZipItem would not test for valid items
  // and return child names of the invalid items
  mSettings.setValue( "/qgis/scanZipInBrowser", 3 );
  QVERIFY( testZipItem( mDataDir + "testzip.zip", "" ) );
}


void TestZipLayer::testZipItemVectorTransparency()
{
  int myTarget = 250;
  int myTransparency = getLayerTransparency( mDataDir + "points2.zip", "ogr", 1 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
  myTransparency = getLayerTransparency( mDataDir + "points2.zip", "ogr", 2 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
}

void TestZipLayer::testGipItemVectorTransparency()
{
  int myTarget = 250;
  int myTransparency = getLayerTransparency( mDataDir + "points3.geojson.gz", "ogr", 1 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
  myTransparency = getLayerTransparency( mDataDir + "points3.geojson.gz", "ogr", 2 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
}

void TestZipLayer::testZipItemRasterTransparency()
{
  int myTarget = 250;
  int myTransparency = getLayerTransparency( mDataDir + "landsat_b1.zip", "gdal", 1 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
  myTransparency = getLayerTransparency( mDataDir + "landsat_b1.zip", "gdal", 2 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
}

void TestZipLayer::testGZipItemRasterTransparency()
{
  int myTarget = 250;
  int myTransparency = getLayerTransparency( mDataDir + "landsat_b1.tif.gz", "gdal", 1 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
  myTransparency = getLayerTransparency( mDataDir + "landsat_b1.tif.gz", "gdal", 2 );
  QVERIFY2(( myTransparency == myTarget ), QString( "Transparency is %1, should be %2" ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
}


QTEST_MAIN( TestZipLayer )
#include "moc_testziplayer.cxx"
