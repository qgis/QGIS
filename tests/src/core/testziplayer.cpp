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
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QApplication>
#include <QFileInfo>

//qgis includes...
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsdataitem.h"
#include "qgsconfig.h"
#include "qgsrasterrenderer.h"
#include "qgssettings.h"
#include "qgsdirectoryitem.h"
#include "qgslayeritem.h"
#include "qgszipitem.h"

#include <gdal.h>

/**
 * \ingroup UnitTests
 * This is a unit test to verify that zip vector layers work
 */
class TestZipLayer: public QObject
{
    Q_OBJECT

  private:

    QString mDataDir;
    QString mScanZipSetting;
    QString mSettingsKey;

    // get map layer using Passthru
    QgsMapLayer *getLayer( const QString &myPath, const QString &myName, const QString &myProviderKey );
    bool testZipItemPassthru( const QString &myFileName, const QString &myProviderKey );
    // get map layer using QgsZipItem (only 1 child)
    QgsMapLayer *getZipLayer( const QString &myPath, const QString &myName );
    // test item(s) in zip item (supply name or test all)
    bool testZipItem( const QString &myFileName, const QString &myChildName = QString(), const QString &myDriverName = QString() );
    // get layer transparency to test for .qml loading
    int getLayerTransparency( const QString &myFileName, const QString &myProviderKey, const QString &myScanZipSetting = QStringLiteral( "basic" ) );

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


QgsMapLayer *TestZipLayer::getLayer( const QString &myPath, const QString &myName, const QString &myProviderKey )
{
  QString fullName = myName;
  if ( fullName.isEmpty() )
  {
    QFileInfo myFileInfo( myPath );
    fullName = myFileInfo.completeBaseName();
  }
  QgsMapLayer *myLayer = nullptr;

  if ( myProviderKey == QLatin1String( "ogr" ) )
  {
    QgsVectorLayer::LayerOptions options { QgsCoordinateTransformContext() };
    myLayer = new QgsVectorLayer( myPath, fullName, QStringLiteral( "ogr" ), options );
  }
  else if ( myProviderKey == QLatin1String( "gdal" ) )
  {
    myLayer = new QgsRasterLayer( myPath, fullName, QStringLiteral( "gdal" ) );
  }

  // item should not have other provider key, but if it does will return nullptr
  return myLayer;
}

QgsMapLayer *TestZipLayer::getZipLayer( const QString &myPath, const QString &myName )
{
  QgsMapLayer *myLayer = nullptr;
  QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "/" ), QString() );
  QgsDataItem *myItem = QgsZipItem::itemFromPath( dirItem, myPath, myName );
  if ( myItem )
  {
    QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem *>( myItem->children().at( 0 ) );
    if ( layerItem )
      myLayer = getLayer( layerItem->path(), layerItem->name(), layerItem->providerKey() );
  }
  delete myItem;
  delete dirItem;
  return myLayer;
}

bool TestZipLayer::testZipItemPassthru( const QString &myFileName, const QString &myProviderKey )
{
  std::unique_ptr< QgsMapLayer > layer( getLayer( myFileName, QString(), myProviderKey ) );
  return layer && layer->isValid();
}

bool TestZipLayer::testZipItem( const QString &myFileName, const QString &myChildName, const QString &myProviderName )
{
  QgsDebugMsgLevel( QStringLiteral( "\n=======================================\nfile = %1 name = %2 provider = %3"
                                  ).arg( myFileName, myChildName, myProviderName ), 2 );
  QFileInfo myFileInfo( myFileName );
  QgsZipItem *myZipItem = new QgsZipItem( nullptr, myFileInfo.fileName(), myFileName );
  myZipItem->populate();
  // wait until populated in separate thread
  QElapsedTimer time;
  time.start();
  while ( myZipItem->state() != Qgis::BrowserItemState::Populated && time.elapsed() < 5000 )
  {
    QTest::qSleep( 100 );
    QCoreApplication::processEvents();
  }
  QgsDebugMsgLevel( QStringLiteral( "time.elapsed() = %1 ms" ).arg( time.elapsed() ), 2 );
  bool ok = false;

  QVector<QgsDataItem *> myChildren = myZipItem->children();

  QgsDebugMsgLevel( QStringLiteral( "has %1 items" ).arg( myChildren.size() ), 2 );
  if ( !myChildren.isEmpty() )
  {
    for ( QgsDataItem *item : std::as_const( myChildren ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "child name=%1" ).arg( item->name() ), 2 );
      QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem *>( item );
      if ( layerItem )
      {
        QgsDebugMsgLevel( QStringLiteral( "child name=%1 provider=%2 path=%3" ).arg( layerItem->name(), layerItem->providerKey(), layerItem->path() ), 2 );
        if ( myChildName.isEmpty() || myChildName == item->name() )
        {
          QgsMapLayer *layer = getLayer( layerItem->path(), layerItem->name(), layerItem->providerKey() );
          if ( layer )
          {
            // we got a layer, check if it is valid and exit
            // if no child name given in argument, then pass to next one (unless current child is invalid)
            QgsDebugMsgLevel( QStringLiteral( "valid: %1" ).arg( layer->isValid() ), 2 );
            ok = layer->isValid();
            delete layer;
            if ( ! ok )
            {
              QWARN( QString( "Invalid layer %1" ).arg( layerItem->path() ).toLocal8Bit().data() );
            }
            if ( myChildName.isEmpty() )
            {
              if ( ! ok )
                break;
            }
            else
            {
              //verify correct provider was used
              if ( !myProviderName.isEmpty() )
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
    }
  }
  delete myZipItem;
  return ok;
}

int TestZipLayer::getLayerTransparency( const QString &myFileName, const QString &myProviderKey, const QString &myScanZipSetting )
{
  int myTransparency = -1;
  QgsSettings settings;
  settings.setValue( mSettingsKey, myScanZipSetting );
  if ( myScanZipSetting != settings.value( mSettingsKey ).toString() )
    return myTransparency;

  QgsMapLayer *myLayer = nullptr;
  if ( myFileName.endsWith( QLatin1String( ".gz" ), Qt::CaseInsensitive ) )
    myLayer = getLayer( myFileName, QString(), myProviderKey );
  else
    myLayer = getZipLayer( myFileName, QString() );
  if ( myLayer && myLayer->isValid() )
  {
    // myTransparency = myLayer->getTransparency();
    if ( myLayer->type() == QgsMapLayerType::RasterLayer )
    {
      QgsRasterLayer *layer = dynamic_cast<QgsRasterLayer *>( myLayer );
      if ( layer && layer->renderer() )
      {
        myTransparency = std::ceil( layer->renderer()->opacity() * 255 );
      }
    }
  }
  else
    QWARN( QString( "Could not open filename %1 using %2 provider" ).arg( myFileName, myProviderKey ).toLocal8Bit().data() );
  if ( myLayer )
    delete myLayer;
  return myTransparency;
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
  mDataDir = QStringLiteral( TEST_DATA_DIR ) + "/zip/";
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  // save current zipSetting value
  QgsSettings settings;
  mSettingsKey = QStringLiteral( "/qgis/scanZipInBrowser2" );
  mScanZipSetting = settings.value( mSettingsKey, "" ).toString();
}

void TestZipLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();

  // restore zipSetting
  QgsSettings settings;
  settings.setValue( mSettingsKey, mScanZipSetting );
}

void TestZipLayer::testPassthruVectorZip()
{
  QgsSettings settings;
  QString myFileName = mDataDir + "points2.zip";

  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItemPassthru( myFileName, "ogr" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItemPassthru( myFileName, "ogr" ) );
}

void TestZipLayer::testPassthruVectorTar()
{
  QgsSettings settings;
  QString myFileName = mDataDir + "points2.tar";

  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItemPassthru( myFileName, "ogr" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItemPassthru( myFileName, "ogr" ) );
}

void TestZipLayer::testPassthruVectorGzip()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "points3.geojson.gz", "ogr" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "points3.geojson.gz", "ogr" ) );
}

void TestZipLayer::testPassthruRasterZip()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.zip", "gdal" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.zip", "gdal" ) );
}

void TestZipLayer::testPassthruRasterTar()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.tar", "gdal" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.tar", "gdal" ) );
}

void TestZipLayer::testPassthruRasterGzip()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.tif.gz", "gdal" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItemPassthru( mDataDir + "landsat_b1.tif.gz", "gdal" ) );
}

void TestZipLayer::testZipItemRaster()
{
  QgsSettings settings;

  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "landsat_b1.tif" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "landsat_b1.tif" ) );
}

void TestZipLayer::testTarItemRaster()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "landsat_b1.tif" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "landsat_b1.tif" ) );
}

void TestZipLayer::testZipItemVector()
{
  QgsSettings settings;

  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "points.shp" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "points.shp" ) );
}

void TestZipLayer::testTarItemVector()
{
  QgsSettings settings;

  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "points.shp" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "points.shp" ) );
}

void TestZipLayer::testZipItemAll()
{
  // test for all items inside zip, using zipSetting 3 (Full Scan) which will ignore invalid items
  // using zipSetting 2 (Basic Scan) would raise errors, because QgsZipItem would not test for valid items
  // and return child names of the invalid items
  // test file does not contain invalid items (some of dash tests failed because of them)
  QgsSettings settings;
  settings.setValue( mSettingsKey, "full" );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "" ) );
}

void TestZipLayer::testTarItemAll()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, "full" );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "" ) );
}

#if 0
void TestZipLayer::testZipItemVectorTransparency()
{
  QVERIFY( testZipItemTransparency( mDataDir + "points2.zip", "ogr", 250 ) );
}

void TestZipLayer::testTarItemVectorTransparency()
{
  QVERIFY( testZipItemTransparency( mDataDir + "points2.tar", "ogr", 250 ) );
}

void TestZipLayer::testGzipItemVectorTransparency()
{
  QVERIFY( testZipItemTransparency( mDataDir + "points3.geojson.gz", "ogr", 250 ) );
}
#endif

void TestZipLayer::testZipItemRasterTransparency()
{
  QCOMPARE( getLayerTransparency( mDataDir + "landsat_b1.zip", "gdal", QStringLiteral( "basic" ) ), 250 );
  QCOMPARE( getLayerTransparency( mDataDir + "landsat_b1.zip", "gdal", QStringLiteral( "full" ) ), 250 );
}

void TestZipLayer::testTarItemRasterTransparency()
{
  QCOMPARE( getLayerTransparency( mDataDir + "landsat_b1.tar", "gdal", QStringLiteral( "basic" ) ), 250 );
  QCOMPARE( getLayerTransparency( mDataDir + "landsat_b1.tar", "gdal", QStringLiteral( "full" ) ), 250 );
}

void TestZipLayer::testGzipItemRasterTransparency()
{
  QCOMPARE( getLayerTransparency( mDataDir + "landsat_b1.tif.gz", "gdal", QStringLiteral( "basic" ) ), 250 );
  QCOMPARE( getLayerTransparency( mDataDir + "landsat_b1.tif.gz", "gdal", QStringLiteral( "full" ) ), 250 );
}

void TestZipLayer::testZipItemSubfolder()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "folder/folder2/landsat_b2.tif" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "folder/folder2/landsat_b2.tif" ) );
}

void TestZipLayer::testTarItemSubfolder()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "folder/folder2/landsat_b2.tif" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItem( mDataDir + "testtar.tgz", "folder/folder2/landsat_b2.tif" ) );
}


void TestZipLayer::testZipItemVRT()
{
  QgsSettings settings;

  settings.setValue( mSettingsKey, QStringLiteral( "basic" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "landsat_b1.vrt", "gdal" ) );
  // this file is buggy with gdal svn - skip for now
  // QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "points.vrt", "ogr" ) );

  settings.setValue( mSettingsKey, QStringLiteral( "full" ) );
  QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "landsat_b1.vrt", "gdal" ) );
  // this file is buggy with gdal svn - skip for now
  // QVERIFY( testZipItem( QDir::tempPath() + "/testzip.zip", "points.vrt", "ogr" ) );

}

QGSTEST_MAIN( TestZipLayer )
#include "testziplayer.moc"
