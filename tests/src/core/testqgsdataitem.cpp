/***************************************************************************
     testqgsdataitem.cpp
     --------------------------------------
    Date                 : Thu May 24 10:44:50 BRT 2012
    Copyright            : (C) 2012 Etienne Tourigny
    Email                : etourigny.dev at gmail dot com
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
#include <QSettings>

//qgis includes...
#include <qgsdataitem.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgslogger.h>

/** \ingroup UnitTests
 * This is a unit test for the QgsDataItem class.
 */
class TestQgsDataItem : public QObject
{
    Q_OBJECT

  public:
    TestQgsDataItem();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testValid();
    void testDirItemChildren();

  private:
    QgsDirectoryItem* mDirItem;
    QString mScanItemsSetting;
    bool isValidDirItem( QgsDirectoryItem *item );
};

TestQgsDataItem::TestQgsDataItem()
    : mDirItem( NULL )
{

}

void TestQgsDataItem::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );
  // save current scanItemsSetting value
  QSettings settings;
  mScanItemsSetting = settings.value( "/qgis/scanItemsInBrowser2", QVariant( "" ) ).toString();

  //create a directory item that will be used in all tests...
  mDirItem = new QgsDirectoryItem( 0, "Test", TEST_DATA_DIR );
}

void TestQgsDataItem::cleanupTestCase()
{
  // restore scanItemsSetting
  QSettings settings;
  settings.setValue( "/qgis/scanItemsInBrowser2", mScanItemsSetting );
  if ( mDirItem )
    delete mDirItem;

  QgsApplication::exitQgis();
}

bool TestQgsDataItem::isValidDirItem( QgsDirectoryItem *item )
{
  return ( item && item->hasChildren() );
}

void TestQgsDataItem::testValid()
{
  if ( mDirItem )
  {
    QgsDebugMsg( QString( "dirItem has %1 children" ).arg( mDirItem->rowCount() ) );
  }
  QVERIFY( isValidDirItem( mDirItem ) );
}

void TestQgsDataItem::testDirItemChildren()
{
  QSettings settings;
  QStringList tmpSettings;
  tmpSettings << "" << "contents" << "extension";
  Q_FOREACH ( const QString& tmpSetting, tmpSettings )
  {
    settings.setValue( "/qgis/scanItemsInBrowser2", tmpSetting );
    QgsDirectoryItem* dirItem = new QgsDirectoryItem( 0, "Test", TEST_DATA_DIR );
    QVERIFY( isValidDirItem( dirItem ) );

    QVector<QgsDataItem*> children = dirItem->createChildren();
    for ( int i = 0; i < children.size(); i++ )
    {
      QgsDataItem* dataItem = children[i];
      QgsLayerItem* layerItem = dynamic_cast<QgsLayerItem*>( dataItem );
      if ( ! layerItem )
        continue;

      // test .vrt and .gz files are not loaded by gdal and ogr
      QFileInfo info( layerItem->path() );
      QString lFile = info.fileName();
      QString lProvider = layerItem->providerKey();
      QString errStr = QString( "layer #%1 - %2 provider = %3 tmpSetting = %4" ).arg( i ).arg( lFile ).arg( lProvider ).arg( tmpSetting );

      QgsDebugMsg( QString( "testing child name=%1 provider=%2 path=%3 tmpSetting = %4" ).arg( layerItem->name() ).arg( lProvider ).arg( lFile ).arg( tmpSetting ) );

      if ( lFile == "landsat.tif" )
      {
        QVERIFY2( lProvider == "gdal", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points.vrt" )
      {
        QVERIFY2( lProvider == "ogr", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "landsat.vrt" )
      {
        QVERIFY2( lProvider == "gdal", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "landsat_b1.tif.gz" )
      {
        QVERIFY2( lProvider == "gdal", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points3.geojson.gz" )
      {
        QVERIFY2( lProvider == "ogr", errStr.toLocal8Bit().constData() );
      }

      // test layerName() does not include extension for gdal and ogr items (bug #5621)
      QString lName = layerItem->layerName();
      errStr = QString( "layer #%1 - %2 lName = %3 tmpSetting = %4" ).arg( i ).arg( lFile ).arg( lName ).arg( tmpSetting );

      if ( lFile == "landsat.tif" )
      {
        QVERIFY2( lName == "landsat", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points.shp" )
      {
        QVERIFY2( lName == "points", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "landsat_b1.tif.gz" )
      {
        QVERIFY2( lName == "landsat_b1", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points3.geojson.gz" )
      {
        QVERIFY2( lName == "points3", errStr.toLocal8Bit().constData() );
      }

    }

    delete dirItem;
  }
}

QTEST_MAIN( TestQgsDataItem )
#include "testqgsdataitem.moc"
