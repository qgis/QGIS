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
#include <QSignalSpy>
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
#include "qgsdirectoryitem.h"
#include "qgslayeritem.h"

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
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testValid();
    void testDirItem();
    void testDirItemChildren();
    void testDirItemMonitoring();
    void testDirItemMonitoringSlowDrive();
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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
  // save current scanItemsSetting value
  QgsSettings settings;
  settings.clear();
  mScanItemsSetting = settings.value( u"/qgis/scanItemsInBrowser2"_s, QVariant( "" ) ).toString();

  //create a directory item that will be used in all tests...
  mDirItem = new QgsDirectoryItem( nullptr, u"Test"_s, TEST_DATA_DIR );
}

void TestQgsDataItem::cleanupTestCase()
{
  // restore scanItemsSetting
  QgsSettings settings;
  settings.setValue( u"/qgis/scanItemsInBrowser2"_s, mScanItemsSetting );
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
    QgsDebugMsgLevel( u"dirItem has %1 children"_s.arg( mDirItem->rowCount() ), 1 );
  }
  QVERIFY( isValidDirItem( mDirItem ) );
}

void TestQgsDataItem::testDirItem()
{
  auto dirItem = std::make_unique<QgsDirectoryItem>( nullptr, u"Test"_s, TEST_DATA_DIR );
  QCOMPARE( dirItem->dirPath(), QStringLiteral( TEST_DATA_DIR ) );
  QCOMPARE( dirItem->name(), u"Test"_s );

  QVERIFY( dirItem->hasDragEnabled() );
  QgsMimeDataUtils::Uri mime = dirItem->mimeUris().isEmpty() ? QgsMimeDataUtils::Uri() : dirItem->mimeUris().first();
  QVERIFY( mime.isValid() );
  QCOMPARE( mime.uri, QStringLiteral( TEST_DATA_DIR ) );
  QCOMPARE( mime.layerType, u"directory"_s );
}

void TestQgsDataItem::testDirItemChildren()
{
  QgsSettings settings;
  QStringList tmpSettings;
  tmpSettings << QString() << u"contents"_s << u"extension"_s;
  for ( const QString &tmpSetting : std::as_const( tmpSettings ) )
  {
    settings.setValue( u"/qgis/scanItemsInBrowser2"_s, tmpSetting );
    QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, u"Test"_s, TEST_DATA_DIR );
    QVERIFY( isValidDirItem( dirItem ) );

    QVector<QgsDataItem *> children = dirItem->createChildren();
    for ( int i = 0; i < children.size(); i++ )
    {
      QgsDataItem *dataItem = children[i];
      QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( dataItem );
      if ( !layerItem )
        continue;

      // test .vrt and .gz files are not loaded by gdal and ogr
      QFileInfo info( layerItem->path() );
      QString lFile = info.fileName();
      QString lProvider = layerItem->providerKey();
      QString errStr = u"layer #%1 - %2 provider = %3 tmpSetting = %4"_s.arg( i ).arg( lFile, lProvider, tmpSetting );

      QgsDebugMsgLevel( u"testing child name=%1 provider=%2 path=%3 tmpSetting = %4"_s.arg( layerItem->name(), lProvider, lFile, tmpSetting ), 1 );

      if ( lFile == "landsat.tif"_L1 )
      {
        QVERIFY2( lProvider == "gdal"_L1, errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points.vrt"_L1 )
      {
        QVERIFY2( lProvider == "ogr"_L1, errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "landsat.vrt"_L1 )
      {
        QVERIFY2( lProvider == "gdal"_L1, errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "landsat_b1.tif.gz"_L1 )
      {
        QVERIFY2( lProvider == "gdal"_L1, errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points3.geojson.gz"_L1 )
      {
        QVERIFY2( lProvider == "ogr"_L1, errStr.toLocal8Bit().constData() );
      }

      // test layerName() does not include extension for gdal and ogr items (bug #5621)
      QString lName = layerItem->layerName();
      errStr = u"layer #%1 - %2 lName = %3 tmpSetting = %4"_s.arg( i ).arg( lFile, lName, tmpSetting );

      if ( lFile == "landsat.tif"_L1 )
      {
        QVERIFY2( lName == "landsat"_L1, errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points.shp"_L1 )
      {
        QVERIFY2( lName == "points"_L1, errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "landsat_b1.tif.gz"_L1 )
      {
        QVERIFY2( lName == "landsat_b1"_L1, errStr.toLocal8Bit().constData() );
      }
      else if ( lFile == "points3.geojson.gz"_L1 )
      {
        QVERIFY2( lName == "points3"_L1, errStr.toLocal8Bit().constData() );
      }
    }
    qDeleteAll( children );

    delete dirItem;
  }
}

void TestQgsDataItem::testDirItemMonitoring()
{
  QTemporaryDir dir;

  const QString parentDir = dir.path();
  const QString child1 = parentDir + u"/child1"_s;
  const QString child2 = parentDir + u"/child2"_s;
  QVERIFY( QDir().mkpath( child1 ) );
  QVERIFY( QDir().mkpath( child2 ) );

  auto dirItem = std::make_unique<QgsDirectoryItem>( nullptr, u"parent name"_s, parentDir, parentDir + '/' );
  QCOMPARE( dirItem->path(), parentDir + '/' );
  QCOMPARE( dirItem->dirPath(), parentDir );

  dirItem->populate( true );
  QCOMPARE( dirItem->rowCount(), 2 );
  QPointer<QgsDirectoryItem> childItem1( qobject_cast<QgsDirectoryItem *>( dirItem->children().at( 0 ) ) );
  QPointer<QgsDirectoryItem> childItem2( qobject_cast<QgsDirectoryItem *>( dirItem->children().at( 1 ) ) );
  QVERIFY( childItem1 );
  QCOMPARE( childItem1->path(), child1 );
  QCOMPARE( childItem1->dirPath(), child1 );
  QVERIFY( childItem2 );
  QCOMPARE( childItem2->path(), child2 );
  QCOMPARE( childItem2->dirPath(), child2 );

  QCOMPARE( dirItem->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QCOMPARE( childItem1->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QCOMPARE( childItem2->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );

  QVERIFY( dirItem->isMonitored() );
  QVERIFY( childItem1->isMonitored() );
  QVERIFY( childItem2->isMonitored() );

  QVERIFY( dirItem->mFileSystemWatcher );
  // file system watchers aren't created until items are populated
  QVERIFY( !childItem1->mFileSystemWatcher );
  QVERIFY( !childItem2->mFileSystemWatcher );

  // avoid the normal required timeout between population
  dirItem->mLastScan = QDateTime( QDate( 2000, 1, 1 ), QTime( 0, 0, 0 ) );
  const QString child3 = parentDir + u"/child3"_s;
  QVERIFY( QDir().mkpath( child3 ) );
  QSignalSpy spy( dirItem.get(), &QgsDataItem::endInsertItems );
  spy.wait();
  QCOMPARE( dirItem->rowCount(), 3 );
  QVERIFY( childItem1 );
  QVERIFY( childItem2 );
  QPointer<QgsDirectoryItem> childItem3( qobject_cast<QgsDirectoryItem *>( dirItem->children().at( 2 ) ) );
  QCOMPARE( childItem3->dirPath(), child3 );

  QCOMPARE( childItem3->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QVERIFY( childItem3->isMonitored() );

  childItem1->populate( true );
  childItem2->populate( true );
  QVERIFY( childItem1->mFileSystemWatcher );
  QVERIFY( childItem2->mFileSystemWatcher );

  // block monitoring of childItem3
  childItem3->populate( true );
  QVERIFY( childItem3->mFileSystemWatcher );
  QVERIFY( !childItem3->hasChildren() );
  childItem3->setMonitoring( Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QCOMPARE( childItem3->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !childItem3->isMonitored() );
  QVERIFY( !childItem3->mFileSystemWatcher );

  // explicitly force monitoring of childItem2
  childItem2->populate( true );
  QVERIFY( childItem2->mFileSystemWatcher );
  QVERIFY( !childItem2->hasChildren() );
  childItem2->setMonitoring( Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QCOMPARE( childItem2->monitoring(), Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( childItem2->isMonitored() );
  QVERIFY( childItem2->mFileSystemWatcher );

  // turn off monitoring of parent item
  QVERIFY( dirItem->mFileSystemWatcher );
  dirItem->setMonitoring( Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QCOMPARE( dirItem->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !dirItem->isMonitored() );
  QVERIFY( !dirItem->mFileSystemWatcher );

  // child dir without an explicit setting should inherit parent dir setting, others should not
  QCOMPARE( childItem1->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QVERIFY( !childItem1->isMonitored() );
  QVERIFY( !childItem1->mFileSystemWatcher );
  QCOMPARE( childItem2->monitoring(), Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( childItem2->isMonitored() );
  QVERIFY( childItem2->mFileSystemWatcher );
  QCOMPARE( childItem3->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !childItem3->isMonitored() );
  QVERIFY( !childItem3->mFileSystemWatcher );

  // turn on monitoring of parent item
  QVERIFY( !dirItem->mFileSystemWatcher );
  dirItem->setMonitoring( Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QCOMPARE( dirItem->monitoring(), Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( dirItem->isMonitored() );
  QVERIFY( dirItem->mFileSystemWatcher );

  QCOMPARE( childItem1->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QVERIFY( childItem1->isMonitored() );
  QVERIFY( childItem1->mFileSystemWatcher );
  QCOMPARE( childItem2->monitoring(), Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( childItem2->isMonitored() );
  QVERIFY( childItem2->mFileSystemWatcher );
  QCOMPARE( childItem3->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !childItem3->isMonitored() );
  QVERIFY( !childItem3->mFileSystemWatcher );

  // turn back off monitoring
  dirItem->setMonitoring( Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QCOMPARE( dirItem->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !dirItem->isMonitored() );
  QVERIFY( !dirItem->mFileSystemWatcher );
  QCOMPARE( childItem1->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QVERIFY( !childItem1->isMonitored() );
  QVERIFY( !childItem1->mFileSystemWatcher );
  QCOMPARE( childItem2->monitoring(), Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( childItem2->isMonitored() );
  QVERIFY( childItem2->mFileSystemWatcher );
  QCOMPARE( childItem3->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !childItem3->isMonitored() );
  QVERIFY( !childItem3->mFileSystemWatcher );

  // now switch parent back to default behavior
  QVERIFY( !dirItem->mFileSystemWatcher );
  dirItem->setMonitoring( Qgis::BrowserDirectoryMonitoring::Default );
  QCOMPARE( dirItem->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QVERIFY( dirItem->isMonitored() );
  QVERIFY( dirItem->mFileSystemWatcher );

  QCOMPARE( childItem1->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QVERIFY( childItem1->isMonitored() );
  QVERIFY( childItem1->mFileSystemWatcher );
  QCOMPARE( childItem2->monitoring(), Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( childItem2->isMonitored() );
  QVERIFY( childItem2->mFileSystemWatcher );
  QCOMPARE( childItem3->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !childItem3->isMonitored() );
  QVERIFY( !childItem3->mFileSystemWatcher );

  // turn off monitoring
  QgsSettings().setValue( u"/qgis/monitorDirectoriesInBrowser"_s, false );
  dirItem->reevaluateMonitoring();
  QVERIFY( !dirItem->isMonitored() );
  QVERIFY( !dirItem->mFileSystemWatcher );
  QCOMPARE( childItem1->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  QVERIFY( !childItem1->isMonitored() );
  QVERIFY( !childItem1->mFileSystemWatcher );
  QCOMPARE( childItem2->monitoring(), Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( childItem2->isMonitored() );
  QVERIFY( childItem2->mFileSystemWatcher );
  QCOMPARE( childItem3->monitoring(), Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  QVERIFY( !childItem3->isMonitored() );
  QVERIFY( !childItem3->mFileSystemWatcher );
  QgsSettings().setValue( u"/qgis/monitorDirectoriesInBrowser"_s, true );
}

void TestQgsDataItem::testDirItemMonitoringSlowDrive()
{
  QTemporaryDir dir;

  // fake a directory on a slow drive -- this is a special hardcoded path which always reports to be on a slow drive. See QgsFileUtils::pathIsSlowDevice
  const QString parentDir = dir.path() + u"/fake_slow_path_for_unit_tests"_s;
  const QString child1 = parentDir + u"/child1"_s;
  QVERIFY( QDir().mkpath( child1 ) );
  const QString child2 = parentDir + u"/child2"_s;
  QVERIFY( QDir().mkpath( child2 ) );
  const QString child2child = child2 + u"/child"_s;
  QVERIFY( QDir().mkpath( child2child ) );

  QVERIFY( !QgsDirectoryItem::pathShouldByMonitoredByDefault( parentDir ) );
  QVERIFY( !QgsDirectoryItem::pathShouldByMonitoredByDefault( child1 ) );
  QVERIFY( !QgsDirectoryItem::pathShouldByMonitoredByDefault( child2 ) );
  QVERIFY( !QgsDirectoryItem::pathShouldByMonitoredByDefault( child2child ) );

  auto dirItem = std::make_unique<QgsDirectoryItem>( nullptr, u"parent name"_s, parentDir, u"/"_s + parentDir );
  // user has not explicitly set the path to be monitored or not, so Default should be returned here:
  QCOMPARE( dirItem->monitoring(), Qgis::BrowserDirectoryMonitoring::Default );
  // but directory should NOT be monitored
  QVERIFY( !dirItem->isMonitored() );

  dirItem->populate( true );
  QVERIFY( !dirItem->mFileSystemWatcher );

  QCOMPARE( dirItem->rowCount(), 2 );
  QPointer<QgsDirectoryItem> childItem1( qobject_cast<QgsDirectoryItem *>( dirItem->children().at( 0 ) ) );
  QPointer<QgsDirectoryItem> childItem2( qobject_cast<QgsDirectoryItem *>( dirItem->children().at( 1 ) ) );
  QVERIFY( childItem1 );
  QVERIFY( childItem2 );
  // neither of these should be monitored either!
  QVERIFY( !childItem1->isMonitored() );
  QVERIFY( !childItem2->isMonitored() );
  childItem1->populate( true );
  childItem2->populate( true );
  QVERIFY( !childItem1->mFileSystemWatcher );
  QVERIFY( !childItem2->mFileSystemWatcher );

  // explicitly opt in to monitoring a directory on a slow drive
  childItem2->setMonitoring( Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
  QVERIFY( childItem2->mFileSystemWatcher );

  // this means that subdirectories of childItem2 should now return true to being monitored by default
  QVERIFY( !QgsDirectoryItem::pathShouldByMonitoredByDefault( parentDir ) );
  QVERIFY( !QgsDirectoryItem::pathShouldByMonitoredByDefault( child1 ) );
  QVERIFY( !QgsDirectoryItem::pathShouldByMonitoredByDefault( child2 ) );
  QVERIFY( QgsDirectoryItem::pathShouldByMonitoredByDefault( child2child ) );
}

void TestQgsDataItem::testLayerItemType()
{
  std::unique_ptr<QgsMapLayer> layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp", QString(), u"ogr"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), Qgis::BrowserLayerType::Polygon );

  layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "points.shp", QString(), u"ogr"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), Qgis::BrowserLayerType::Point );

  layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "lines.shp", QString(), u"ogr"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), Qgis::BrowserLayerType::Line );

  layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "nonspatial.dbf", QString(), u"ogr"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), Qgis::BrowserLayerType::TableLayer );

  layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "invalid.dbf", QString(), u"ogr"_s );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), Qgis::BrowserLayerType::Vector );

  layer = std::make_unique<QgsRasterLayer>( mTestDataDir + "rgb256x256.png", QString(), u"gdal"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), Qgis::BrowserLayerType::Raster );

  layer = std::make_unique<QgsMeshLayer>( mTestDataDir + "mesh/quad_and_triangle.2dm", QString(), u"mdal"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( QgsLayerItem::typeFromMapLayer( layer.get() ), Qgis::BrowserLayerType::Mesh );
}


class TestProjectDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return u"project_test"_s; }
    Qgis::DataItemProviderCapabilities capabilities() const override { return Qgis::DataItemProviderCapability::Files; }
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override
    {
      QFileInfo fileInfo( path );
      if ( fileInfo.suffix().compare( "qgs"_L1, Qt::CaseInsensitive ) == 0 || fileInfo.suffix().compare( "qgz"_L1, Qt::CaseInsensitive ) == 0 )
      {
        return new QgsDataItem( Qgis::BrowserItemType::Custom, parentItem, path, path );
      }
      return nullptr;
    }
};

void TestQgsDataItem::testProjectItemCreation()
{
  QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, u"Test"_s, mTestDataDir + u"qgis_server/"_s );
  QVector<QgsDataItem *> children = dirItem->createChildren();

  // ensure that QgsProjectItem items were created
  bool foundQgsProject = false;
  bool foundQgzProject = false;
  for ( QgsDataItem *child : children )
  {
    if ( child->type() == Qgis::BrowserItemType::Project && child->path() == mTestDataDir + u"qgis_server/test_project.qgs"_s )
    {
      foundQgsProject = true;
      continue;
    }
    if ( child->type() == Qgis::BrowserItemType::Project && child->path() == mTestDataDir + u"qgis_server/test_project.qgz"_s )
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

  dirItem = new QgsDirectoryItem( nullptr, u"Test"_s, mTestDataDir + u"qgis_server/"_s );
  children = dirItem->createChildren();

  // ensure that QgsProjectItem items were NOT created -- our test provider should have created custom items instead
  foundQgsProject = false;
  foundQgzProject = false;
  bool foundCustomQgsProject = false;
  bool foundCustomQgzProject = false;
  for ( QgsDataItem *child : std::as_const( children ) )
  {
    if ( child->type() == Qgis::BrowserItemType::Project && child->path() == mTestDataDir + u"qgis_server/test_project.qgs"_s )
    {
      foundQgsProject = true;
      continue;
    }
    if ( child->type() == Qgis::BrowserItemType::Project && child->path() == mTestDataDir + u"qgis_server/test_project.qgz"_s )
    {
      foundQgzProject = true;
      continue;
    }
    if ( child->type() == Qgis::BrowserItemType::Custom && child->path() == mTestDataDir + u"qgis_server/test_project.qgs"_s )
    {
      foundCustomQgsProject = true;
      continue;
    }
    if ( child->type() == Qgis::BrowserItemType::Custom && child->path() == mTestDataDir + u"qgis_server/test_project.qgz"_s )
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
