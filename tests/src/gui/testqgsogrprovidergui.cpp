/***************************************************************************
  testqgsogrprovidergui.cpp - TestQgsOgrProviderGui

 ---------------------
 begin                : 10.11.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QSignalSpy>
#include <QTemporaryFile>

#include "qgsdataitemguiprovider.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsgeopackagedataitems.h"
#include "qgsgui.h"
#include "qgsvectorlayer.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the ogr provider GUI
 */
class TestQgsOgrProviderGui : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void providersRegistered();
    //! Test GPKG data items rename
    void testGpkgDataItemRename();

  private:
    QString mTestDataDir;
};


//runs before all tests
void TestQgsOgrProviderGui::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
}

void TestQgsOgrProviderGui::providersRegistered()
{
  const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
  bool hasOgrProvider = false;
  bool hasGpkgProvider = false;
  for ( QgsDataItemGuiProvider *provider : providers )
  {
    if ( provider->name() == QLatin1String( "ogr_items" ) )
      hasOgrProvider = true;
    if ( provider->name() == QLatin1String( "geopackage_items" ) )
      hasGpkgProvider = true;
  }
  QVERIFY( hasOgrProvider );
  QVERIFY( hasGpkgProvider );
}

//runs after all tests
void TestQgsOgrProviderGui::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsOgrProviderGui::testGpkgDataItemRename()
{
  QTemporaryFile f( QStringLiteral( "qgis-XXXXXX.gpkg" ) );
  f.open();
  f.close();
  const QString fileName { f.fileName( ) };
  f.remove();
  QVERIFY( QFile::copy( QStringLiteral( "%1/provider/bug_21227-rename-styles.gpkg" ).arg( mTestDataDir ),  fileName ) );

  // create geopackage item and populate it with layers
  QgsGeoPackageCollectionItem gpkgItem( nullptr, QStringLiteral( "test gpkg" ), QStringLiteral( "gpkg:/%1" ).arg( fileName ) );
  gpkgItem.populate( true );
  const QVector<QgsDataItem *> items = gpkgItem.children();
  QgsDataItem *itemLayer1 = nullptr;
  for ( QgsDataItem *item : items )
  {
    if ( item->name() == QLatin1String( "layer 1" ) )
      itemLayer1 = item;
  }
  QVERIFY( itemLayer1 );

  QSignalSpy spyDataChanged( &gpkgItem, &QgsDataItem::dataChanged );

  // try to rename
  const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
  bool success = false;
  for ( QgsDataItemGuiProvider *provider : providers )
  {
    if ( provider->rename( itemLayer1, QStringLiteral( "layer 3" ), QgsDataItemGuiContext() ) )
    {
      success = true;
      // also check it was the correct provider
      QCOMPARE( provider->name(), QStringLiteral( "geopackage_items" ) );
    }
  }
  QVERIFY( success );

  // gpkg item gets refreshed in the background and there will be multiple dataChanged signals
  // emitted unfortunately, so let's just wait until no more data changes signals are coming.
  // Animation of "loading" icon also triggers dataChanged() signals, making even the number
  // of signals unpredictable...
  while ( spyDataChanged.wait( 500 ) )
    ;

  // Check that the style is still available
  const QgsVectorLayer metadataLayer( QStringLiteral( "/%1|layername=layer_styles" ).arg( fileName ) );
  QVERIFY( metadataLayer.isValid() );
  QgsFeature feature;
  QgsFeatureIterator it = metadataLayer.getFeatures( QgsFeatureRequest( QgsExpression( QStringLiteral( "\"f_table_name\" = 'layer 3'" ) ) ) );
  QVERIFY( it.nextFeature( feature ) );
  QVERIFY( feature.isValid() );
  QCOMPARE( feature.attribute( QStringLiteral( "styleName" ) ).toString(), QString( "style for layer 1" ) );
  it = metadataLayer.getFeatures( QgsFeatureRequest( QgsExpression( QStringLiteral( "\"f_table_name\" = 'layer 1' " ) ) ) );
  QVERIFY( !it.nextFeature( feature ) );
  it = metadataLayer.getFeatures( QgsFeatureRequest( QgsExpression( QStringLiteral( "\"f_table_name\" = 'layer 2' " ) ) ) );
  QVERIFY( it.nextFeature( feature ) );
  QVERIFY( feature.isValid() );
  QCOMPARE( feature.attribute( QStringLiteral( "styleName" ) ).toString(), QString( "style for layer 2" ) );
}


QGSTEST_MAIN( TestQgsOgrProviderGui )
#include "testqgsogrprovidergui.moc"
