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
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QStringList>

//qgis includes...
#include "qgsdataitem.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgssettings.h"

/**
 * \ingroup UnitTests
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
    void testLayerItemType();
    void testProjectItemCreation();

  private:
    QgsDirectoryItem *mDirItem = nullptr;
    QString mScanItemsSetting;
    QString mTestDataDir;
    bool isValidDirItem( QgsDirectoryItem *item );
};

TestQgsDataItem::TestQgsDataItem() = default;

void TestQgsDataItem::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = dataDir + '/';

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
  // save current scanItemsSetting value
  QgsSettings settings;
  mScanItemsSetting = settings.value( QStringLiteral( "/qgis/scanItemsInBrowser2" ), QVariant( "" ) ).toString();

  //create a directory item that will be used in all tests...
  mDirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), TEST_DATA_DIR );
}

void TestQgsDataItem::cleanupTestCase()
{
  // restore scanItemsSetting
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/qgis/scanItemsInBrowser2" ), mScanItemsSetting );
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
    QgsDebugMsg( QStringLiteral( "dirItem has %1 children" ).arg( mDirItem->rowCount() ) );
  }
  QVERIFY( isValidDirItem( mDirItem ) );
}

void TestQgsDataItem::testDirItemChildren()
{
  QgsSettings settings;
  QStringList tmpSettings;
  tmpSettings << QString() << QStringLiteral( "contents" ) << QStringLiteral( "extension" );
  Q_FOREACH ( const QString &tmpSetting, tmpSettings )
  {
    settings.setValue( QStringLiteral( "/qgis/scanItemsInBrowser2" ), tmpSetting );
    QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), TEST_DATA_DIR );
    QVERIFY( isValidDirItem( dirItem ) );

    QVector<QgsDataItem *> children = dirItem->createChildren();
    for ( int i = 0; i < children.size(); i++ )
    {
      QgsDataItem *dataItem = children[i];
      QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem *>( dataItem );
      if ( ! layerItem )
        continue;

      // test .vrt and .gz files are not loaded by gdal and ogr
      QFileInfo info( layerItem->path() );
      QString lFile = info.fileName();
      QString lProvider = layerItem->providerKey();
      QString errStr = QStringLiteral( "layer #%1 - %2 provider = %3 tmpSetting = %4" ).arg( i ).arg( lFile, lProvider, tmpSetting );

      QgsDebugMsg( QStringLiteral( "testing child name=%1 provider=%2 path=%3 tmpSetting = %4" ).arg( layerItem->name(), lProvider, lFile, tmpSetting ) );

      if ( lFile == QLatin1String( "landsat.tif" ) )
      {
        QVERIFY2( lProvider == "gdal", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == QLatin1String( "points.vrt" ) )
      {
        QVERIFY2( lProvider == "ogr", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == QLatin1String( "landsat.vrt" ) )
      {
        QVERIFY2( lProvider == "gdal", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == QLatin1String( "landsat_b1.tif.gz" ) )
      {
        QVERIFY2( lProvider == "gdal", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == QLatin1String( "points3.geojson.gz" ) )
      {
        QVERIFY2( lProvider == "ogr", errStr.toLocal8Bit().constData() );
      }

      // test layerName() does not include extension for gdal and ogr items (bug #5621)
      QString lName = layerItem->layerName();
      errStr = QStringLiteral( "layer #%1 - %2 lName = %3 tmpSetting = %4" ).arg( i ).arg( lFile, lName, tmpSetting );

      if ( lFile == QLatin1String( "landsat.tif" ) )
      {
        QVERIFY2( lName == "landsat", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == QLatin1String( "points.shp" ) )
      {
        QVERIFY2( lName == "points", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == QLatin1String( "landsat_b1.tif.gz" ) )
      {
        QVERIFY2( lName == "landsat_b1", errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == QLatin1String( "points3.geojson.gz" ) )
      {
        QVERIFY2( lName == "points3", errStr.toLocal8Bit().constData() );
      }

    }
    qDeleteAll( children );

    delete dirItem;
  }
}

void TestQgsDataItem::testLayerItemType()
{
  std::unique_ptr< QgsMapLayer > layer = qgis::make_unique< QgsVectorLayer >( mTestDataDir + "polys.shp",
                                         QString(), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), QgsLayerItem::Polygon );

  layer = qgis::make_unique< QgsVectorLayer >( mTestDataDir + "points.shp",
          QString(), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), QgsLayerItem::Point );

  layer = qgis::make_unique< QgsVectorLayer >( mTestDataDir + "lines.shp",
          QString(), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), QgsLayerItem::Line );

  layer = qgis::make_unique< QgsVectorLayer >( mTestDataDir + "nonspatial.dbf",
          QString(), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), QgsLayerItem::TableLayer );

  layer = qgis::make_unique< QgsVectorLayer >( mTestDataDir + "invalid.dbf",
          QString(), QStringLiteral( "ogr" ) );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), QgsLayerItem::Vector );

  layer = qgis::make_unique< QgsRasterLayer >( mTestDataDir + "rgb256x256.png",
          QString(), QStringLiteral( "gdal" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), QgsLayerItem::Raster );

  layer = qgis::make_unique< QgsMeshLayer >( mTestDataDir + "mesh/quad_and_triangle.2dm",
          QString(), QStringLiteral( "mdal" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), QgsLayerItem::Mesh );
}


class TestProjectDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return QStringLiteral( "project_test" ); }
    int capabilities() override { return QgsDataProvider::File; }
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override
    {
      QFileInfo fileInfo( path );
      if ( fileInfo.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) == 0 || fileInfo.suffix().compare( QLatin1String( "qgz" ), Qt::CaseInsensitive ) == 0 )
      {
        return new QgsDataItem( QgsDataItem::Custom, parentItem, path, path );
      }
      return nullptr;
    }
};

void TestQgsDataItem::testProjectItemCreation()
{
  QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), mTestDataDir + QStringLiteral( "qgis_server/" ) );
  QVector<QgsDataItem *> children = dirItem->createChildren();

  // ensure that QgsProjectItem items were created
  bool foundQgsProject = false;
  bool foundQgzProject = false;
  for ( QgsDataItem *child : children )
  {
    if ( child->type() == QgsDataItem::Project && child->path() == mTestDataDir + QStringLiteral( "qgis_server/test_project.qgs" ) )
    {
      foundQgsProject = true;
      continue;
    }
    if ( child->type() == QgsDataItem::Project && child->path() == mTestDataDir + QStringLiteral( "qgis_server/test_project.qgz" ) )
    {
      foundQgzProject = true;
      continue;
    }
  }
  QVERIFY( foundQgsProject );
  QVERIFY( foundQgzProject );
  delete dirItem;

  // now, add a specific provider which handles project files
  QgsApplication::dataItemProviderRegistry()->addProvider( new TestProjectDataItemProvider() );

  dirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), mTestDataDir + QStringLiteral( "qgis_server/" ) );
  children = dirItem->createChildren();

  // ensure that QgsProjectItem items were NOT created -- our test provider should have created custom items instead
  foundQgsProject = false;
  foundQgzProject = false;
  bool foundCustomQgsProject = false;
  bool foundCustomQgzProject = false;
  for ( QgsDataItem *child : children )
  {
    if ( child->type() == QgsDataItem::Project && child->path() == mTestDataDir + QStringLiteral( "qgis_server/test_project.qgs" ) )
    {
      foundQgsProject = true;
      continue;
    }
    if ( child->type() == QgsDataItem::Project && child->path() == mTestDataDir + QStringLiteral( "qgis_server/test_project.qgz" ) )
    {
      foundQgzProject = true;
      continue;
    }
    if ( child->type() == QgsDataItem::Custom && child->path() == mTestDataDir + QStringLiteral( "qgis_server/test_project.qgs" ) )
    {
      foundCustomQgsProject = true;
      continue;
    }
    if ( child->type() == QgsDataItem::Custom && child->path() == mTestDataDir + QStringLiteral( "qgis_server/test_project.qgz" ) )
    {
      foundCustomQgzProject = true;
      continue;
    }
  }
  QVERIFY( !foundQgsProject );
  QVERIFY( !foundQgzProject );
  QVERIFY( foundCustomQgsProject );
  QVERIFY( foundCustomQgzProject );
  delete dirItem;
}

QGSTEST_MAIN( TestQgsDataItem )
#include "testqgsdataitem.moc"
