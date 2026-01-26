/***************************************************************************
     testqgslayerpropertiesdialogs.cpp
     ------------------------------
    Date                 : October 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "annotations/qgsannotationlayerproperties.h"
#include "qgisapp.h"
#include "qgsannotationlayer.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmarkersymbol.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerproperties.h"
#include "qgsmessagebar.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerproperties.h"
#include "qgsprovidersourcewidget.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgstest.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenelayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilelayerproperties.h"

#include <QObject>

class DummySourceWidget : public QgsProviderSourceWidget
{
    Q_OBJECT
  public:
    DummySourceWidget( QWidget *parent )
      : QgsProviderSourceWidget( parent )
    {
    }

    void setSourceUri( const QString &uri ) override { Q_UNUSED( uri ); }

    QString sourceUri() const override
    {
      return newSource;
    }

    QString newSource;
};

class TestQgsLayerPropertiesDialogs : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayerPropertiesDialogs()
      : QgsTest( u"Layer properties dialogs"_s )
    {}

  private:
    QString mTestDataDir;
    QgisApp *mQgisApp = nullptr;

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      mQgisApp = new QgisApp();

      const QString myDataDir( TEST_DATA_DIR );
      mTestDataDir = myDataDir + '/';
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testValidVectorProperties()
    {
      // valid vector layer
      const QString pointFileName = mTestDataDir + "points.shp";
      const QFileInfo pointFileInfo( pointFileName );
      auto vl = std::make_unique<QgsVectorLayer>( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), u"ogr"_s );
      QVERIFY( vl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
      dialog.show();
      dialog.accept();
    }

    void testInvalidVectorProperties()
    {
      // invalid vector layer
      const QString pointFileName = mTestDataDir + "xxpoints.shp";
      const QFileInfo pointFileInfo( pointFileName );
      auto vl = std::make_unique<QgsVectorLayer>( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), u"xxogr"_s );
      QVERIFY( !vl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
      dialog.show();
      dialog.accept();
    }

    void testChangeVectorSubset()
    {
      // start with a point layer
      const QString pointFileName = mTestDataDir + "points.shp";
      const QFileInfo pointFileInfo( pointFileName );
      auto vl = std::make_unique<QgsVectorLayer>( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), u"ogr"_s );
      QVERIFY( vl->isValid() );
      vl->setSubsetString( u"\"class\"='Biplane'"_s );
      QCOMPARE( vl->subsetString(), u"\"class\"='Biplane'"_s );

      // no change to filter
      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        dialog.show();
        dialog.accept();
      }

      QCOMPARE( vl->subsetString(), u"\"class\"='Biplane'"_s );

      // change the filter to a line layer:
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        dialog.txtSubsetSQL->setText( u"\"class\"='B52'"_s );
        dialog.show();
        dialog.accept();
      }
      QCOMPARE( vl->subsetString(), u"\"class\"='B52'"_s );

      // try with BOTH a filter change and the source widget present, to check interaction of the two
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        DummySourceWidget *sourceWidget = new DummySourceWidget( &dialog );
        sourceWidget->newSource = mTestDataDir + "points.shp";
        dialog.mSourceWidget = sourceWidget;
        dialog.show();
        dialog.txtSubsetSQL->setText( u"\"class\"='Biplane'"_s );
        dialog.accept();
      }
      QCOMPARE( vl->source(), mTestDataDir + "points.shp|subset=\"class\"='Biplane'" );
      QCOMPARE( vl->subsetString(), u"\"class\"='Biplane'"_s );

      // try with BOTH a filter change AND a source change
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        DummySourceWidget *sourceWidget = new DummySourceWidget( &dialog );
        sourceWidget->newSource = mTestDataDir + "lines.shp";
        dialog.mSourceWidget = sourceWidget;
        dialog.show();
        dialog.txtSubsetSQL->setText( u"\"Name\" = 'Highway'"_s );
        dialog.accept();
      }
      QCOMPARE( vl->source(), mTestDataDir + "lines.shp|subset=\"Name\" = 'Highway'" );
      QCOMPARE( vl->subsetString(), u"\"Name\" = 'Highway'"_s );
    }

    void testChangeVectorDataSource()
    {
      // start with a point layer
      const QString pointFileName = mTestDataDir + "points.shp";
      const QFileInfo pointFileInfo( pointFileName );
      auto vl = std::make_unique<QgsVectorLayer>( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), u"ogr"_s );
      QVERIFY( vl->isValid() );
      // point layer should have a marker symbol
      vl->setRenderer( new QgsSingleSymbolRenderer( new QgsMarkerSymbol() ) );
      QCOMPARE( dynamic_cast<QgsSingleSymbolRenderer *>( vl->renderer() )->symbol()->type(), Qgis::SymbolType::Marker );

      // no change to data source
      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        dialog.show();
        dialog.accept();
      }

      // renderer should still be a marker type
      QCOMPARE( dynamic_cast<QgsSingleSymbolRenderer *>( vl->renderer() )->symbol()->type(), Qgis::SymbolType::Marker );

      // change the data source to a line layer:
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        DummySourceWidget *sourceWidget = new DummySourceWidget( &dialog );
        sourceWidget->newSource = mTestDataDir + "lines_touching.shp";
        dialog.mSourceWidget = sourceWidget;
        dialog.show();
        dialog.accept();
      }

      QCOMPARE( vl->source(), mTestDataDir + "lines_touching.shp" );
      QCOMPARE( vl->geometryType(), Qgis::GeometryType::Line );
      // single symbol renderer with marker symbol would be nonsense now, we expected a line symbol
      // ie the settings for the renderer which were present in the dialog MUST be ignored and overwritten
      // by the logic which triggers when the geometry type is changed via a data source change
      QCOMPARE( dynamic_cast<QgsSingleSymbolRenderer *>( vl->renderer() )->symbol()->type(), Qgis::SymbolType::Line );
    }

    void testValidRasterProperties()
    {
      // valid raster layer
      QTemporaryDir tmpDir;
      QFile::copy( mTestDataDir + "landsat_4326.tif", tmpDir.filePath( u"landsat_4326.tif"_s ) );
      const QString rasterFileName = tmpDir.filePath( u"landsat_4326.tif"_s );
      auto rl = std::make_unique<QgsRasterLayer>( rasterFileName, u"test"_s, u"gdal"_s );
      QVERIFY( rl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsRasterLayerProperties dialog( rl.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testInvalidRasterProperties()
    {
      // invalid raster layer
      const QString rasterFileName = mTestDataDir + "xxlandsat_4326.tif";
      auto rl = std::make_unique<QgsRasterLayer>( rasterFileName, u"test"_s, u"xxgdal"_s );
      QVERIFY( !rl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsRasterLayerProperties dialog( rl.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testValidMeshProperties()
    {
      // valid mesh layer
      QString uri( mTestDataDir + "/mesh/quad_and_triangle.2dm" );
      auto ml = std::make_unique<QgsMeshLayer>( uri, u"test"_s, u"mdal"_s );
      QVERIFY( ml->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsMeshLayerProperties dialog( ml.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testInvalidMeshProperties()
    {
      // invalid mesh layer
      QString uri( mTestDataDir + "/mesh/xxquad_and_triangle.2dm" );
      auto ml = std::make_unique<QgsMeshLayer>( uri, u"test"_s, u"xmdal"_s );
      QVERIFY( !ml->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsMeshLayerProperties dialog( ml.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testValidPointCloudProperties()
    {
      // valid point cloud layer
      auto layer = std::make_unique<QgsPointCloudLayer>( mTestDataDir + u"point_clouds/ept/sunshine-coast/ept.json"_s, u"layer"_s, u"ept"_s );
      QVERIFY( layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsPointCloudLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testInvalidPointCloudProperties()
    {
      // invalid point cloud layer
      auto layer = std::make_unique<QgsPointCloudLayer>( mTestDataDir + u"xxpoint_clouds/ept/sunshine-coast/ept.json"_s, u"layer"_s, u"xxept"_s );
      QVERIFY( !layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsPointCloudLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testValidVectorTileProperties()
    {
      // valid vector tile layer
      const QString srcMbtiles = u"type=mbtiles&url=%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR );
      auto layer = std::make_unique<QgsVectorTileLayer>( srcMbtiles );
      QVERIFY( layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorTileLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testInvalidVectorTileProperties()
    {
      // invalid vector tile layer
      const QString srcMbtiles = u"type=mbtiles&url=%1/vector_tile/xxmbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR );
      auto layer = std::make_unique<QgsVectorTileLayer>( srcMbtiles );
      QVERIFY( !layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorTileLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testInvalidTileSceneProperties()
    {
      // invalid tiled scene layer
      auto layer = std::make_unique<QgsTiledSceneLayer>( u"xxx"_s, u"test"_s, u"xxx"_s );
      QVERIFY( !layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsTiledSceneLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testValidAnnotationLayerProperties()
    {
      // valid annotation layer
      auto layer = std::make_unique<QgsAnnotationLayer>( u"xxx"_s, QgsAnnotationLayer::LayerOptions( QgsCoordinateTransformContext() ) );
      QVERIFY( layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsAnnotationLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }
};

QGSTEST_MAIN( TestQgsLayerPropertiesDialogs )
#include "testqgslayerpropertiesdialogs.moc"
