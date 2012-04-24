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

    bool testPassthruVector( QString myFileName );
    bool testPassthruRaster( QString myFileName );
    bool testZipItem( QString myFileName, QString myChildName );

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

};

bool TestZipLayer::testPassthruVector( QString myFileName )
{
  QFileInfo myFileInfo( myFileName );
  QgsVectorLayer * myVectorLayer;
  myVectorLayer = new QgsVectorLayer( myFileInfo.filePath(),
                                      myFileInfo.completeBaseName(), "ogr" );
  bool ok = myVectorLayer->isValid();
  delete myVectorLayer;
  return ok;
}

bool TestZipLayer::testPassthruRaster( QString myFileName )
{
  QFileInfo myFileInfo( myFileName );
  QgsRasterLayer * myRasterLayer;
  myRasterLayer = new QgsRasterLayer( myFileInfo.filePath(),
                                      myFileInfo.completeBaseName(), "gdal" );
  bool ok = myRasterLayer->isValid();
  delete myRasterLayer;
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
          QgsMapLayer* myLayer = NULL;
          if ( layerItem->providerKey() == "ogr" )
          {
            myLayer = new QgsVectorLayer( item->path(), item->name(), "ogr" );
          }
          else if ( layerItem->providerKey() == "gdal" )
          {
            myLayer = new QgsRasterLayer( item->path(), item->name(), "gdal" );
          }
          else
          {
            // item should not have other provider key, but if it does the test will fail
            ok = false;
            QWARN( QString( "Invalid provider %1" ).arg( layerItem->providerKey() ).toLocal8Bit().data() );
            break;
          }
          if ( myLayer != NULL )
          {
            // we got a layer, check if it is valid and exit
            QgsDebugMsg( QString( "valid: %1" ).arg( myLayer->isValid() ) );
            ok = myLayer->isValid();
            delete myLayer;
            if ( ! ok )
            {
              QWARN( QString( "Invalid item %1" ).arg( layerItem->path() ).toLocal8Bit().data() );
            }
            // if no child name given, then pass to next one (unless current child is invalid)
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
            QWARN( QString( "Invalid item %1" ).arg( layerItem->path() ).toLocal8Bit().data() );
            break;
          }
        }
      }
      else
      {
        QWARN( QString( "Invalid item %1" ).arg( layerItem->path() ).toLocal8Bit().data() );
        break;
      }
    }
  }
  delete myZipItem;
  return ok;
}

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
  QString myFileName = mDataDir + "points.zip";
  QgsDebugMsg( "GDAL: " + QString( GDAL_RELEASE_NAME ) );
#if GDAL_VERSION_NUM < 1800
  myFileName = "/vsizip/" + myFileName + "/points.shp";
#endif
  QgsDebugMsg( "FILE: " + QString( myFileName ) );
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testPassthruVector( myFileName ) );
  }
}

void TestZipLayer::testPassthruVectorGzip()
{
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testPassthruVector( mDataDir + "points.geojson.gz" ) );
  }
}

void TestZipLayer::testPassthruRasterZip()
{
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testPassthruRaster( mDataDir + "landsat_b1.zip" ) );
  }
}

void TestZipLayer::testPassthruRasterGzip()
{
  for ( int i = 1 ; i <= mMaxScanZipSetting ; i++ )
  {
    mSettings.setValue( "/qgis/scanZipInBrowser", i );
    QVERIFY( testPassthruRaster( mDataDir + "landsat_b1.tif.gz" ) );
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

QTEST_MAIN( TestZipLayer )
#include "moc_testziplayer.cxx"




