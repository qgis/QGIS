/***************************************************************************
  testalignraster.cpp
  --------------------------------------
  Date                 : June 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsalignraster.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"

#include <QDir>

#include <gdal.h>



static QString _tempFile( const QString &name )
{
  return QStringLiteral( "%1/aligntest-%2.tif" ).arg( QDir::tempPath(), name );
}


class TestAlignRaster : public QObject
{
    Q_OBJECT

    QString SRC_FILE;
  private slots:

    void initTestCase()
    {
      GDALAllRegister();

      SRC_FILE = QStringLiteral( TEST_DATA_DIR ) + "/float1-16.tif";

      QgsApplication::init(); // needed for CRS database
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testRasterInfo()
    {
      const QgsAlignRaster::RasterInfo out( SRC_FILE );
      QVERIFY( out.isValid() );
      QCOMPARE( out.cellSize(), QSizeF( 0.2, 0.2 ) );
      QCOMPARE( out.gridOffset(), QPointF( 0.0, 0.0 ) );
      QCOMPARE( out.extent(), QgsRectangle( 106.0, -7.0, 106.8, -6.2 ) );
    }

    void testClip()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "clip" ) ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      align.setClipExtent( 106.3, -6.65, 106.35, -6.5 );
      const bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 1, 2 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.2, 0.2 ) );
      QCOMPARE( out.identify( 106.3, -6.7 ), 10. );
      QCOMPARE( out.identify( 106.3, -6.5 ), 6. );
    }

    void testClipOutside()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "clip-outside" ) ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      align.setClipExtent( 1, 1, 2, 2 );
      const bool res = align.run();
      QVERIFY( !res );
    }

    void testChangeGridOffsetNN()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "change-grid-offset-nn" ) ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      QPointF offset = align.gridOffset();
      offset.rx() += 0.25;
      align.setGridOffset( offset );
      const bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 3, 4 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.2, 0.2 ) );
      QCOMPARE( out.identify( 106.1, -6.9 ), 13. );
      QCOMPARE( out.identify( 106.2, -6.9 ), 13. );
      QCOMPARE( out.identify( 106.3, -6.9 ), 14. );
    }

    void testChangeGridOffsetBilinear()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "change-grid-offset-bilinear" ) ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Bilinear;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      QPointF offset = align.gridOffset();
      offset.rx() += 0.1;
      align.setGridOffset( offset );
      const bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 3, 4 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.2, 0.2 ) );
      QCOMPARE( out.identify( 106.2, -6.9 ), 13.5 );
    }

    void testSmallerCellSize()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "smaller-cell-size" ) ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Bilinear;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      align.setCellSize( 0.1, 0.1 );
      const bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 8, 8 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.1, 0.1 ) );
      QCOMPARE( out.identify( 106.15, -6.35 ), 2.25 );

    }


    void testBiggerCellSize()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "bigger-cell-size" ) ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Average;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE, QString(), QSizeF( 0.4, 0.4 ) );
      const bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 2, 2 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.4, 0.4 ) );
      QCOMPARE( out.identify( 106.2, -6.9 ), 11.5 );
    }


    void testRescaleBiggerCellSize()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "rescale-bigger-cell-size" ) ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Average;
      rasters[0].rescaleValues = true;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE, QString(), QSizeF( 0.4, 0.4 ) );
      const bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 2, 2 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.4, 0.4 ) );
      QCOMPARE( out.identify( 106.2, -6.4 ), 14. ); // = (1+2+5+6)
    }

    void testReprojectToOtherCRS()
    {
      const QString tmpFile( _tempFile( QStringLiteral( "reproject-utm-47n" ) ) );

      // reproject from WGS84 to UTM zone 47N
      // (the true UTM zone for this raster is 48N, but here it is
      // more obvious the different shape of the resulting raster)
      const QgsCoordinateReferenceSystem destCRS( QStringLiteral( "EPSG:32647" ) );
      QVERIFY( destCRS.isValid() );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE, destCRS.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) );
      const bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      const QgsCoordinateReferenceSystem outCRS( out.crs() );
      QCOMPARE( outCRS, destCRS );
      QCOMPARE( out.rasterSize(), QSize( 4, 4 ) );
      // tolerance of 1 to keep the test more robust
      QGSCOMPARENEAR( out.cellSize().width(), 22293, 1 ); // ~ 22293.256065
      QGSCOMPARENEAR( out.cellSize().height(), 22293, 1 ); // ~ 22293.256065
      QGSCOMPARENEAR( out.gridOffset().x(), 4327, 1 ); // ~ 4327.168434
      QGSCOMPARENEAR( out.gridOffset().y(), 637, 1 ); // ~ 637.007990
      QCOMPARE( out.identify( 1308405, -746611 ), 10. );
    }

    void testSuggestedReferenceLayer()
    {
      QgsAlignRaster align;

      QCOMPARE( align.suggestedReferenceLayer(), -1 );

      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, QString() );
      align.setRasters( rasters );

      QCOMPARE( align.suggestedReferenceLayer(), 0 );

    }

};

QGSTEST_MAIN( TestAlignRaster )

#include "testqgsalignraster.moc"
