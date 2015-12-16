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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
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
    Q_OBJECT

  private:

    QString mDataDir;
    QString mScanZipSetting;
    QStringList mScanZipSettings;
    QString mSettingsKey;

    // get map layer using Passthru
    QgsMapLayer * getLayer( const QString& myPath, const QString& myName, const QString& myProviderKey );
    bool testZipItemPassthru( const QString& myFileName, const QString& myProviderKey );
    // get map layer using QgsZipItem (only 1 child)
    QgsMapLayer * getZipLayer( const QString& myPath, const QString& myName );
    // test item(s) in zip item (supply name or test all)
    bool testZipItem( const QString& myFileName, const QString& myChildName = "", const QString& myDriverName = "" );
    // get layer transparency to test for .qml loading
    int getLayerTransparency( const QString& myFileName, const QString& myProviderKey, const QString& myScanZipSetting = "basic" );
    bool testZipItemTransparency( const QString& myFileName, const QString& myProviderKey, int myTarget );

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    // tests
    // test for .zip and .gz files using all options
    void testPassthruVectorZip();
    void testPassthruVectorTar();
    void testPassthruVectorGzip();
    void testPassthruRasterZip();
    void testPassthruRasterTar();
    void testPassthruRasterGzip();
    // test both "Basic Scan" and "Full scan" for .zip files
    void testZipItemRaster();
    void testTarItemRaster();
    void testZipItemVector();
    void testTarItemVector();
    void testZipItemAll();
    void testTarItemAll();
    // test that styles are loaded from .qml files outside zip files
#if 0
    // TODO - find a simple way to test vector style loading
    void testZipItemVectorTransparency();
    void testTarItemVectorTransparency();
    void testGzipItemVectorTransparency();
#endif
    void testZipItemRasterTransparency();
    void testTarItemRasterTransparency();
    void testGzipItemRasterTransparency();
    //make sure items inside subfolders can be read
    void testZipItemSubfolder();
    void testTarItemSubfolder();
    //make sure .vrt items are loaded by proper provider (gdal/ogr)
    void testZipItemVRT();
};


QgsMapLayer *TestZipLayer::getLayer( const QString& myPath, const QString& myName, const QString& myProviderKey )
{
  QString fullName = myName;
  if ( fullName == "" )
  {
    QFileInfo myFileInfo( myPath );
    fullName = myFileInfo.completeBaseName();
  }
  QgsMapLayer *myLayer = nullptr;

  if ( myProviderKey == "ogr" )
  {
    myLayer = new QgsVectorLayer( myPath, fullName, "ogr" );
  }
  else if ( myProviderKey == "gdal" )
  {
    myLayer = new QgsRasterLayer( myPath, fullName, QString( "gdal" ) );
  }

  // item should not have other provider key, but if it does will return nullptr
  return myLayer;
}

QgsMapLayer *TestZipLayer::getZipLayer( const QString& myPath, const QString& myName )
{
  QgsMapLayer *myLayer = nullptr;
  QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, "/", "" );
  QgsDataItem* myItem = QgsZipItem::itemFromPath( dirItem, myPath, myName );
  if ( myItem )
  {
    QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem*>( myItem );
    if ( layerItem )
      myLayer = getLayer( layerItem->path(), layerItem->name(), layerItem->providerKey() );
  }
  delete myItem;
  delete dirItem;
  return myLayer;
}

bool TestZipLayer::testZipItemPassthru( const QString& myFileName, const QString& myProviderKey )
{
  QgsMapLayer * myLayer = getLayer( myFileName, "", myProviderKey );
  bool ok = myLayer && myLayer->isValid();
  if ( myLayer )
    delete myLayer;
  return ok;
}

bool TestZipLayer::testZipItem( const QString& myFileName, const QString& myChildName, const QString& myProviderName )
{
  QgsDebugMsg( QString( "\n=======================================\nfile = %1 name = %2 provider = %3"
                      ).arg( myFileName, myChildName, myProviderName ) );
  QFileInfo myFileInfo( myFileName );
  QgsZipItem *myZipItem = new QgsZipItem( nullptr, myFileInfo.fileName(), myFileName );
  myZipItem->populate();
  // wait until populated in separate thread
  QTime time;
  time.start();
  while ( myZipItem->state() != QgsDataItem::Populated && time.elapsed() < 5000 )
  {
    QTest::qSleep( 100 );
    QCoreApplication::processEvents();
  }
  QgsDebugMsg( QString( "time.elapsed() = %1 ms" ).arg( time.elapsed() ) );
  bool ok = false;

  QVector<QgsDataItem*> myChildren = myZipItem->children();

  QgsDebugMsg( QString( "has %1 items" ).arg( myChildren.size() ) );
  if ( !myChildren.isEmpty() )
  {
    Q_FOREACH ( QgsDataItem* item, myChildren )
    {
      QgsDebugMsg( QString( "child name=%1" ).arg( item->name() ) );
      QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem*>( item );
      if ( layerItem )
      {
        QgsDebugMsg( QString( "child name=%1 provider=%2 path=%3" ).arg( layerItem->name(), layerItem->providerKey(), layerItem->path() ) );
        if ( myChildName == "" || myChildName == item->name() )
        {
          QgsMapLayer* layer = getLayer( layerItem->path(), layerItem->name(), layerItem->providerKey() );
          if ( layer )
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
              //verify correct provider was used
              if ( myProviderName != "" )
              {
                ok = ( myProviderName == layerItem->providerKey() );
                if ( ! ok )
                {
                  QWARN( QString( "Layer %1 opened by provider %2, expecting %3"
                                ).arg( layerItem->path(), layerItem->providerKey(), myProviderName ).toLocal8Bit().data() );
                }
              }
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
        QWARN( QString( "Invalid layer %1" ).arg( layerItem ? layerItem->path() : "(null)" ).toLocal8Bit().data() );
        break;
      }
    }
  }
  delete myZipItem;
  return ok;
}

int TestZipLayer::getLayerTransparency( const QString& myFileName, const QString& myProviderKey, const QString& myScanZipSetting )
{
  int myTransparency = -1;
  QSettings settings;
  settings.setValue( mSettingsKey, myScanZipSetting );
  if ( myScanZipSetting != settings.value( mSettingsKey ).toString() )
    return myTransparency;

  QgsMapLayer * myLayer = nullptr;
  if ( myFileName.endsWith( ".gz", Qt::CaseInsensitive ) )
    myLayer = getLayer( myFileName, "", myProviderKey );
  else
    myLayer = getZipLayer( myFileName, "" );
  if ( myLayer && myLayer->isValid() )
  {
    // myTransparency = myLayer->getTransparency();
    if ( myLayer->type() == QgsMapLayer::RasterLayer )
    {
      QgsRasterLayer* layer = dynamic_cast<QgsRasterLayer*>( myLayer );
      if ( layer && layer->renderer() )
      {
        myTransparency = ceil( layer->renderer()->opacity() * 255 );
      }
    }
  }
  else
    QWARN( QString( "Could not open filename %1 using %2 provider" ).arg( myFileName, myProviderKey ).toLocal8Bit().data() );
  if ( myLayer )
    delete myLayer;
  return myTransparency;
}

bool TestZipLayer::testZipItemTransparency( const QString& myFileName, const QString& myProviderKey, int myTarget )
{
  int myTransparency;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    myTransparency = getLayerTransparency( myFileName, myProviderKey, s );
    if ( myTransparency != myTarget )
    {
      QWARN( QString( "Transparency of %1 is %2, should be %3" ).arg( myFileName ).arg( myTransparency ).arg( myTarget ).toLocal8Bit().data() );
      return false;
    }
  }
  return true;
}


// slots
void TestZipLayer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // output test environment
  QgsApplication::showSettings();
  qDebug() << "GDAL version (build):   " << GDAL_RELEASE_NAME;
  qDebug() << "GDAL version (runtime): " << GDALVersionInfo( "RELEASE_NAME" );

  // save data dir
  QFile::remove( QDir::tempPath() + "/testzip.zip" );
  QVERIFY( QFile::copy( QString( TEST_DATA_DIR ) + "/zip/" + "testzip.zip", QDir::tempPath() + "/testzip.zip" ) );
  mDataDir = QString( TEST_DATA_DIR ) + "/zip/";
  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  // save current zipSetting value
  QSettings settings;
  mSettingsKey = "/qgis/scanZipInBrowser2";
  mScanZipSetting = settings.value( mSettingsKey, "" ).toString();
  mScanZipSettings << "" << "basic" << "full";
}

void TestZipLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();

  // restore zipSetting
  QSettings settings;
  settings.setValue( mSettingsKey, mScanZipSetting );
}

void TestZipLayer::testPassthruVectorZip()
{
  QSettings settings;
  QString myFileName = mDataDir + "points2.zip";
  QgsDebugMsg( "GDAL: " + QString( GDAL_RELEASE_NAME ) );
#if GDAL_VERSION_NUM < 1800
  myFileName = "/vsizip/" + myFileName + "/points.shp";
#endif
  QgsDebugMsg( "FILE: " + QString( myFileName ) );
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItemPassthru( myFileName, "ogr" ) );
  }
}

void TestZipLayer::testPassthruVectorTar()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QSettings settings;
  QString myFileName = mDataDir + "points2.tar";
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItemPassthru( myFileName, "ogr" ) );
  }
}

void TestZipLayer::testPassthruVectorGzip()
{
#if GDAL_VERSION_NUM < 1700
  QSKIP( "This test requires GDAL >= 1.7", SkipSingle );
#endif
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItemPassthru( mDataDir + "points3.geojson.gz", "ogr" ) );
  }
}

void TestZipLayer::testPassthruRasterZip()
{
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.zip", "gdal" ) );
  }
}

void TestZipLayer::testPassthruRasterTar()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.tar", "gdal" ) );
  }
}

void TestZipLayer::testPassthruRasterGzip()
{
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.tif.gz", "gdal" ) );
  }
}

void TestZipLayer::testZipItemRaster()
{
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "landsat_b1.tif" ) );
  }
}

void TestZipLayer::testTarItemRaster()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItem( mDataDir + "testtar.tgz", "landsat_b1.tif" ) );
  }
}

void TestZipLayer::testZipItemVector()
{
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "points.shp" ) );
  }
}

void TestZipLayer::testTarItemVector()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItem( mDataDir + "testtar.tgz", "points.shp" ) );
  }
}

void TestZipLayer::testZipItemAll()
{
  // test for all items inside zip, using zipSetting 3 (Full Scan) which will ignore invalid items
  // using zipSetting 2 (Basic Scan) would raise errors, because QgsZipItem would not test for valid items
  // and return child names of the invalid items
  // test file does not contain invalid items (some of dash tests failed because of them)
  QSettings settings;
  settings.setValue( mSettingsKey, "full" );
  QVERIFY( "full" == settings.value( mSettingsKey ).toString() );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "" ) );
}

void TestZipLayer::testTarItemAll()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QSettings settings;
  settings.setValue( mSettingsKey, "full" );
  QVERIFY( "full" == settings.value( mSettingsKey ).toString() );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "" ) );
}

#if 0
void TestZipLayer::testZipItemVectorTransparency()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QVERIFY( testZipItemTransparency( mDataDir + "points2.zip", "ogr", 250 ) );
}

void TestZipLayer::testTarItemVectorTransparency()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QVERIFY( testZipItemTransparency( mDataDir + "points2.tar", "ogr", 250 ) );
}

void TestZipLayer::testGzipItemVectorTransparency()
{
#if GDAL_VERSION_NUM < 1700
  QSKIP( "This test requires GDAL >= 1.7", SkipSingle );
#endif
  QVERIFY( testZipItemTransparency( mDataDir + "points3.geojson.gz", "ogr", 250 ) );
}
#endif

void TestZipLayer::testZipItemRasterTransparency()
{
  QVERIFY( testZipItemTransparency( mDataDir + "landsat_b1.zip", "gdal", 250 ) );
}

void TestZipLayer::testTarItemRasterTransparency()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QVERIFY( testZipItemTransparency( mDataDir + "landsat_b1.tar", "gdal", 250 ) );
}

void TestZipLayer::testGzipItemRasterTransparency()
{
  QVERIFY( testZipItemTransparency( mDataDir + "landsat_b1.tif.gz", "gdal", 250 ) );
}

void TestZipLayer::testZipItemSubfolder()
{
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "folder/folder2/landsat_b2.tif" ) );
  }
}

void TestZipLayer::testTarItemSubfolder()
{
#if GDAL_VERSION_NUM < 1800
  QSKIP( "This test requires GDAL >= 1.8", SkipSingle );
#endif
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItem( mDataDir + "testtar.tgz", "folder/folder2/landsat_b2.tif" ) );
  }
}


void TestZipLayer::testZipItemVRT()
{
#if GDAL_VERSION_NUM < 1700
  QSKIP( "This test requires GDAL >= 1.7", SkipSingle );
#endif
  QSettings settings;
  Q_FOREACH ( const QString& s, mScanZipSettings )
  {
    settings.setValue( mSettingsKey, s );
    QVERIFY( s == settings.value( mSettingsKey ).toString() );
    QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "landsat_b1.vrt", "gdal" ) );
    // this file is buggy with gdal svn - skip for now
    // QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "points.vrt", "ogr" ) );
  }
}

QTEST_MAIN( TestZipLayer )
#include "testziplayer.moc"
