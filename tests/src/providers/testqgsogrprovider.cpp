/***************************************************************************
     testqgsogrprovider.cpp
     --------------------------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by Alessandro Pasotti
    Email                : apasotti@boundlessgeo.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include <qgis.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsvectordataprovider.h>

/** \ingroup UnitTests
 * This is a unit test for the gdal provider
 */
class TestQgsOgrProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void shapefileOpen();
    void geopackageSubLayers();
    void geopackageOpen();

  private:
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsOgrProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>OGR Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsOgrProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsOgrProvider::shapefileOpen()
{
  QString dataPath = QStringLiteral( TEST_DATA_DIR ) + "/provider/shapefile.shp";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "ogr" ),  dataPath );
  QgsVectorDataProvider *p = dynamic_cast< QgsVectorDataProvider * >( provider );
  QVERIFY( p );
  QVERIFY( p->isValid() );
  // Ogr provider returns 1 using OGR_DS_GetLayerCount
  // note that gdal returns 0 querying metadata SUBDATASETS when called
  // on a raster.
  QCOMPARE( p->subLayerCount(), 1u );
  QVERIFY( p->featureCount() > 0 );
  delete provider;
}


void TestQgsOgrProvider::geopackageSubLayers()
{
  QString dataPath = QStringLiteral( TEST_DATA_DIR ) + "/provider/gdal_sample_v1.2_no_extensions.gpkg";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "ogr" ),  dataPath );
  QgsVectorDataProvider *p = dynamic_cast< QgsVectorDataProvider * >( provider );
  QVERIFY( p );
  QVERIFY( p->isValid() );
  QCOMPARE( p->featureCount(), 2l );
  // Check sublayers
  QCOMPARE( p->subLayerCount(), 17u );
  QStringList subDataSets = p->subLayers();
  subDataSets.sort();
  QCOMPARE( subDataSets.join( '#' ), QStringLiteral( "0:point2d:2:Point"
            "#10:polygon3d:2:Polygon25D"
            "#11:multipoint3d:2:MultiPoint25D"
            "#12:multilinestring3d:2:MultiLineString25D"
            "#13:multipolygon3d:2:MultiPolygon25D"
            "#14:geomcollection3d:5:GeometryCollection25D"
            "#15:geometry3d:1:GeometryCollection25D"
            "#15:geometry3d:2:LineString25D"
            "#15:geometry3d:2:Point25D"
            "#15:geometry3d:2:Polygon25D"
            "#16:attribute_table:1:None"
            "#1:linestring2d:2:LineString"
            "#2:polygon2d:2:Polygon"
            "#3:multipoint2d:2:MultiPoint"
            "#4:multilinestring2d:2:MultiLineString"
            "#5:multipolygon2d:2:MultiPolygon"
            "#6:geomcollection2d:5:GeometryCollection"
            "#7:geometry2d:1:GeometryCollection"
            "#7:geometry2d:2:LineString"
            "#7:geometry2d:2:Point"
            "#7:geometry2d:2:Polygon"
            "#8:point3d:2:Point25D"
            "#9:linestring3d:2:LineString25D" ) ) ;
  // We only get the first layer!
  QgsFeatureIterator fi = p->getFeatures( );
  QgsFeature f;
  fi.nextFeature( f );
  QVERIFY( f.geometry().type() == QgsWkbTypes::GeometryType::PointGeometry );
  fi.nextFeature( f );
  QVERIFY( f.geometry().type() == QgsWkbTypes::GeometryType::UnknownGeometry );

  delete provider;
}


void TestQgsOgrProvider::geopackageOpen()
{
  QString dataPath = QStringLiteral( TEST_DATA_DIR ) + "/provider/gdal_sample_v1.2_no_extensions.gpkg";
  // Open the linestring from the collection layer
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "ogr" ), QStringLiteral( "%1|layerid=7|geometrytype=LineString" ).arg( dataPath ) );
  QVERIFY( provider->isValid() );
  QgsVectorDataProvider *p = dynamic_cast< QgsVectorDataProvider * >( provider );
  QVERIFY( p );
  if ( p )
  {
    QCOMPARE( p->featureCount(), 2l );
    QCOMPARE( p->subLayerCount(), 17u );
    QgsFeatureIterator fi = p->getFeatures( );
    QgsFeature f;
    while ( fi.nextFeature( f ) )
    {
      QVERIFY( f.geometry().type() == QgsWkbTypes::GeometryType::LineGeometry );
    }
  }
  delete provider;
}

QGSTEST_MAIN( TestQgsOgrProvider )
#include "testqgsogrprovider.moc"
