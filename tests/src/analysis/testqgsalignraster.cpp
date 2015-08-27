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

#include <QtTest/QtTest>

#include "qgsalignraster.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"

#include <QDir>

#include <gdal.h>



static QString _tempFile( const QString& name )
{
  return QString( "%1/aligntest-%2.tif" ).arg( QDir::tempPath() ).arg( name );
}


class TestAlignRaster : public QObject
{
    Q_OBJECT

    QString SRC_FILE;
  private slots:

    void initTestCase()
    {
      GDALAllRegister();

      SRC_FILE = QString( TEST_DATA_DIR ) + "/float1-16.tif";

      QgsApplication::init(); // needed for CRS database
    }

    void testRasterInfo()
    {
      QgsAlignRaster::RasterInfo out( SRC_FILE );
      QVERIFY( out.isValid() );
      QCOMPARE( out.cellSize(), QSizeF( 0.2, 0.2 ) );
      QCOMPARE( out.gridOffset(), QPointF( 0.0, 0.0 ) );
      QCOMPARE( out.extent(), QgsRectangle( 106.0, -7.0, 106.8, -6.2 ) );
    }

    void testClip()
    {
      QString tmpFile( _tempFile( "clip" ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      align.setClipExtent( 106.3, -6.65, 106.35, -6.5 );
      bool res = align.run();
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
      QString tmpFile( _tempFile( "clip-outside" ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      align.setClipExtent( 1, 1, 2, 2 );
      bool res = align.run();
      QVERIFY( !res );
    }

    void testChangeGridOffsetNN()
    {
      QString tmpFile( _tempFile( "change-grid-offset-nn" ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      QPointF offset = align.gridOffset();
      offset.rx() += 0.25;
      align.setGridOffset( offset );
      bool res = align.run();
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
      QString tmpFile( _tempFile( "change-grid-offset-bilinear" ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Bilinear;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      QPointF offset = align.gridOffset();
      offset.rx() += 0.1;
      align.setGridOffset( offset );
      bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 3, 4 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.2, 0.2 ) );
      QCOMPARE( out.identify( 106.2, -6.9 ), 13.5 );
    }

    void testSmallerCellSize()
    {
      QString tmpFile( _tempFile( "smaller-cell-size" ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Bilinear;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE );
      align.setCellSize( 0.1, 0.1 );
      bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 8, 8 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.1, 0.1 ) );
      QCOMPARE( out.identify( 106.15, -6.35 ), 2.25 );

    }


    void testBiggerCellSize()
    {
      QString tmpFile( _tempFile( "bigger-cell-size" ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Bilinear;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE, QString(), QSizeF( 0.4, 0.4 ) );
      bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 2, 2 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.4, 0.4 ) );
      QCOMPARE( out.identify( 106.2, -6.9 ), 11.5 );
    }


    void testRescaleBiggerCellSize()
    {
      QString tmpFile( _tempFile( "rescale-bigger-cell-size" ) );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      rasters[0].resampleMethod = QgsAlignRaster::RA_Bilinear;
      rasters[0].rescaleValues = true;
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE, QString(), QSizeF( 0.4, 0.4 ) );
      bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QCOMPARE( out.rasterSize(), QSize( 2, 2 ) );
      QCOMPARE( out.cellSize(), QSizeF( 0.4, 0.4 ) );
      QCOMPARE( out.identify( 106.2, -6.4 ), 14. ); // = (1+2+5+6)
    }

    void testReprojectToOtherCRS()
    {
      QString tmpFile( _tempFile( "reproject-utm-47n" ) );

      // reproject from WGS84 to UTM zone 47N
      // (the true UTM zone for this raster is 48N, but here it is
      // more obvious the different shape of the resulting raster)
      QgsCoordinateReferenceSystem destCRS( "EPSG:32647" );
      QVERIFY( destCRS.isValid() );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE, destCRS.toWkt() );
      bool res = align.run();
      QVERIFY( res );

      QgsAlignRaster::RasterInfo out( tmpFile );
      QVERIFY( out.isValid() );
      QgsCoordinateReferenceSystem outCRS( out.crs() );
      QCOMPARE( outCRS, destCRS );
      QCOMPARE( out.rasterSize(), QSize( 4, 4 ) );
      // let's stick to integers to keep the test more robust
      QCOMPARE( qRound( out.cellSize().width() ), 22293 ); // ~ 22293.256065
      QCOMPARE( qRound( out.cellSize().height() ), 22293 ); // ~ 22293.256065
      QCOMPARE( qRound( out.gridOffset().x() ), 4327 ); // ~ 4327.168434
      QCOMPARE( qRound( out.gridOffset().y() ), 637 ); // ~ 637.007990
      QCOMPARE( out.identify( 1308405, -746611 ), 10. );
    }

    void testInvalidReprojection()
    {
      QString tmpFile( _tempFile( "reproject-invalid" ) );

      // reprojection to British National Grid with raster in Jakarta area clearly cannot work
      QgsCoordinateReferenceSystem destCRS( "EPSG:27700" );
      QVERIFY( destCRS.isValid() );

      QgsAlignRaster align;
      QgsAlignRaster::List rasters;
      rasters << QgsAlignRaster::Item( SRC_FILE, tmpFile );
      align.setRasters( rasters );
      align.setParametersFromRaster( SRC_FILE, destCRS.toWkt() );
      bool res = align.run();
      QVERIFY( !res );
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

QTEST_MAIN( TestAlignRaster )

#include "testqgsalignraster.moc"
