/***************************************************************************
  testqgsgcptransformer.cpp
  --------------------------------------
  Date                 : February 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsgcptransformer.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgsgcpgeometrytransformer.h"
#include "qgsgeometry.h"

#include <QDir>

#include <gdal.h>

class TestQgsGcpTransformer : public QObject
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

    void testLinear()
    {
      QgsLinearGeorefTransform transform;
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                          << QgsPointXY( 2352, 716 )
                                          << QgsPointXY( 2067, 2398 )
                                          << QgsPointXY( 1102, 2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 288;
      double y = 1000;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 135047, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, 1000, 1 );

      x = 2352;
      y = 1150;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 141437, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, 1150, 1 );

      x = 2067;
      y = 2500;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 198947, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, 2500, 1 );

      x = 1102;
      y = 2300;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 190427, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, 2300, 1 );
    }

    void testLinearRasterYAxis()
    {
      QgsLinearGeorefTransform transform;
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, -716 )
                                          << QgsPointXY( 2352, -1126 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 1102, -2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 288;
      double y = -716;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, -716, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, -1126, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, -2398, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, -2209, 1 );
    }

    void testLinearExact()
    {
      QgsLinearGeorefTransform transform;
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    void testLinearExactRasterYAxis()
    {
      QgsLinearGeorefTransform transform;
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, -123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, -140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, -194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, -186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }


    //
    // Helmert
    //

    void testHelmert()
    {
      QgsHelmertGeorefTransform transform;
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                          << QgsPointXY( 2352, 716 )
                                          << QgsPointXY( 2067, 2398 )
                                          << QgsPointXY( 1102, 2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 288;
      double y = 1000;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 302325, 1 );
      QGSCOMPARENEAR( y, 145676, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, 1000, 1 );

      x = 2352;
      y = 1150;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 340495, 1 );
      QGSCOMPARENEAR( y, 155540, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, 1150, 1 );

      x = 2067;
      y = 2500;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 330540, 1 );
      QGSCOMPARENEAR( y, 179867, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, 2500, 1 );

      x = 1102;
      y = 2300;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 313138, 1 );
      QGSCOMPARENEAR( y, 172822, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, 2300, 1 );
    }

    void testHelmertRasterYAxis()
    {
      QgsHelmertGeorefTransform transform;
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, -716 )
                                          << QgsPointXY( 2352, -1126 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 1102, -2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 288;
      double y = -716;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 334590, 1 );
      QGSCOMPARENEAR( y, 115324, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, -716, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 296200, 1 );
      QGSCOMPARENEAR( y, 115340, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, -1126, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 296768, 1 );
      QGSCOMPARENEAR( y, 91566, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, -2398, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 314707, 1 );
      QGSCOMPARENEAR( y, 91510, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, -2209, 1 );
    }

    void testHelmertExact()
    {
      QgsHelmertGeorefTransform transform;
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    void testHelmertExactRasterYAxis()
    {
      QgsHelmertGeorefTransform transform;
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, -123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, -140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, -194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, -186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }



    //
    // Polynomial order 1
    //

    void testPolyOrder1()
    {
      QgsGDALGeorefTransform transform( false, 1 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                          << QgsPointXY( 2352, 716 )
                                          << QgsPointXY( 2067, 2398 )
                                          << QgsPointXY( 1102, 2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 288;
      double y = 1000;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321209, 1 );
      QGSCOMPARENEAR( y, 128732, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, 1017, 1 );

      x = 2352;
      y = 1150;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322700, 1 );
      QGSCOMPARENEAR( y, 146403, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, 1164, 1 );

      x = 2067;
      y = 2500;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322501, 1 );
      QGSCOMPARENEAR( y, 202230, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, 2474, 1 );

      x = 1102;
      y = 2300;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321803, 1 );
      QGSCOMPARENEAR( y, 188448, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, 2279, 1 );
    }

    void testPolyOrder1RasterYAxis()
    {
      QgsGDALGeorefTransform transform( false, 1 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, -716 )
                                          << QgsPointXY( 2352, -1126 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 1102, -2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 288;
      double y = -716;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321201, 1 );
      QGSCOMPARENEAR( y, 63463, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, -716, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322686, 1 );
      QGSCOMPARENEAR( y, 24965, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, -1126, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322474, 1 );
      QGSCOMPARENEAR( y, -31687, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, -2398, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321780, 1 );
      QGSCOMPARENEAR( y, -13813, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, -2209, 1 );
    }

    void testPolyOrder1Exact()
    {
      QgsGDALGeorefTransform transform( false, 1 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    void testPolyOrder1ExactRasterYAxis()
    {
      QgsGDALGeorefTransform transform( false, 1 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, -123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, -140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, -194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, -186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }


    //
    // Polynomial order 2
    //

    void testPolyOrder2()
    {
      QgsGDALGeorefTransform transform( false, 2 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                          << QgsPointXY( 2352, 716 )
                                          << QgsPointXY( 2067, 2398 )
                                          << QgsPointXY( 2100, 1500 )
                                          << QgsPointXY( 1102, 2209 )
                                          << QgsPointXY( 300, 1550 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322550, 149979 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321310, 145979 ) );

      double x = 288;
      double y = 1000;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321175, 1 );
      QGSCOMPARENEAR( y, 126837, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 265, 1 );
      QGSCOMPARENEAR( y, 1025, 1 );

      x = 2352;
      y = 1150;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322763, 1 );
      QGSCOMPARENEAR( y, 135026, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2336, 1 );
      QGSCOMPARENEAR( y, 936, 1 );

      x = 2067;
      y = 2500;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322489, 1 );
      QGSCOMPARENEAR( y, 198409, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2097, 1 );
      QGSCOMPARENEAR( y, 2445, 1 );

      x = 1102;
      y = 2300;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321803, 1 );
      QGSCOMPARENEAR( y, 197778, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1123, 1 );
      QGSCOMPARENEAR( y, 2225, 1 );
    }

    void testPolyOrder2RasterYAxis()
    {
      QgsGDALGeorefTransform transform( false, 2 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, -1126 )
                                          << QgsPointXY( 2352, -716 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 2100, -1500 )
                                          << QgsPointXY( 1102, -2209 )
                                          << QgsPointXY( 300, -1550 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322550, 149979 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321310, 145979 ), true );

      double x = 288;
      double y = -716;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 320404, 1 );
      QGSCOMPARENEAR( y, 131061, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -1913, 1 );
      QGSCOMPARENEAR( y, 1625, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322022, 1 );
      QGSCOMPARENEAR( y, 176342, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1435, 1 );
      QGSCOMPARENEAR( y, 2087, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 320831, 1 );
      QGSCOMPARENEAR( y, 273376, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -310, 1 );
      QGSCOMPARENEAR( y, 134, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 319958, 1 );
      QGSCOMPARENEAR( y, 243375, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -3994, 1 );
      QGSCOMPARENEAR( y, 1158, 1 );
    }

    void testPolyOrder2Exact()
    {
      QgsGDALGeorefTransform transform( false, 2 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 ) );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    void testPolyOrder2ExactRasterYAxis()
    {
      QgsGDALGeorefTransform transform( false, 2 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 ), true );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, -123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, -140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, -194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, -186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }


    //
    // Polynomial order 3
    //

    void testPolyOrder3()
    {
      QgsGDALGeorefTransform transform( false, 3 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                          << QgsPointXY( 2352, 716 )
                                          << QgsPointXY( 2067, 2398 )
                                          << QgsPointXY( 2100, 1500 )
                                          << QgsPointXY( 1102, 2209 )
                                          << QgsPointXY( 300, 1550 )
                                          << QgsPointXY( 300, 850 )
                                          << QgsPointXY( 1000, 830 )
                                          << QgsPointXY( 900, 1450 )
                                          << QgsPointXY( 700, 1550 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322550, 149979 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321310, 145979 )
                                          << QgsPointXY( 321310, 131979 )
                                          << QgsPointXY( 321703, 131579 )
                                          << QgsPointXY( 321603, 145979 )
                                          << QgsPointXY( 321003, 146179 )
                                        );

      double x = 288;
      double y = 1000;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321202, 1 );
      QGSCOMPARENEAR( y, 129364, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 237, 1 );
      QGSCOMPARENEAR( y, 1319, 1 );

      x = 2352;
      y = 1150;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 318595, 1 );
      QGSCOMPARENEAR( y, 101765, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 138412, 1 );
      QGSCOMPARENEAR( y, 15444, 1 );

      x = 2067;
      y = 2500;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321488, 1 );
      QGSCOMPARENEAR( y, 185766, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -1237, 1 );
      QGSCOMPARENEAR( y, 5315, 1 );

      x = 1102;
      y = 2300;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 320879, 1 );
      QGSCOMPARENEAR( y, 189391, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -4000, 1 );
      QGSCOMPARENEAR( y, 2621, 1 );
    }

    void testPolyOrder3RasterYAxis()
    {
      QgsGDALGeorefTransform transform( false, 3 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, -1126 )
                                          << QgsPointXY( 2352, -716 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 2100, -1500 )
                                          << QgsPointXY( 1102, -2209 )
                                          << QgsPointXY( 300, -1550 )
                                          << QgsPointXY( 300, -850 )
                                          << QgsPointXY( 1000, -830 )
                                          << QgsPointXY( 900, -1450 )
                                          << QgsPointXY( 700, -1550 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322550, 149979 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321310, 145979 )
                                          << QgsPointXY( 321310, 131979 )
                                          << QgsPointXY( 321703, 131579 )
                                          << QgsPointXY( 321603, 145979 )
                                          << QgsPointXY( 321003, 146179 ), true );

      // these values look like nonsense... I can't verify them, except to say that at the time
      // these tests were written the raster georeferencer worked correctly with polynomial order 3, so I can
      // only assume they are correct...!
      double x = 288;
      double y = -716;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 368688, 1 );
      QGSCOMPARENEAR( y, 828425, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -476460174, 1 );
      QGSCOMPARENEAR( y, -43384555, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 456596, 1 );
      QGSCOMPARENEAR( y, 1609794, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -11099349683, 1 );
      QGSCOMPARENEAR( y, -130456708, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 731809, 1 );
      QGSCOMPARENEAR( y, 4861338, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -313632670265, 1 );
      QGSCOMPARENEAR( y, -7338651765, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 620013, 1 );
      QGSCOMPARENEAR( y, 3875762, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, -121400397631, 1 );
      QGSCOMPARENEAR( y, -5274501643, 1 );
    }

    void testPolyOrder3Exact()
    {
      QgsGDALGeorefTransform transform( false, 3 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 )
                                          << QgsPointXY( 322010, 182061 )
                                          << QgsPointXY( 322010, 132061 )
                                          << QgsPointXY( 321050, 162061 )
                                          << QgsPointXY( 321050, 172061 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 )
                                          << QgsPointXY( 322010, 182061 )
                                          << QgsPointXY( 322010, 132061 )
                                          << QgsPointXY( 321050, 162061 )
                                          << QgsPointXY( 321050, 172061 ) );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    void testPolyOrder3ExactRasterYAxis()
    {
      QgsGDALGeorefTransform transform( false, 3 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 )
                                          << QgsPointXY( 322010, 182061 )
                                          << QgsPointXY( 322010, 132061 )
                                          << QgsPointXY( 321050, 162061 )
                                          << QgsPointXY( 321050, 172061 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 322501, 152061 )
                                          << QgsPointXY( 321803, 192198 )
                                          << QgsPointXY( 321210, 152061 )
                                          << QgsPointXY( 322010, 182061 )
                                          << QgsPointXY( 322010, 132061 )
                                          << QgsPointXY( 321050, 162061 )
                                          << QgsPointXY( 321050, 172061 ), true );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, -123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, -140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, -194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, -186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }


    //
    // Thin plate spline
    //

    void testTPSReports()
    {
      QgsGDALGeorefTransform transform( true, 1 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                          << QgsPointXY( 2352, 716 )
                                          << QgsPointXY( 2067, 2398 )
                                          << QgsPointXY( 1102, 2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 288;
      double y = 1000;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321209, 1 );
      QGSCOMPARENEAR( y, 124248, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 290, 1 );
      QGSCOMPARENEAR( y, 680, 1 );

      x = 2352;
      y = 1150;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322700, 1 );
      QGSCOMPARENEAR( y, 146609, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2347, 1 );
      QGSCOMPARENEAR( y, 1636, 1 );

      x = 2067;
      y = 2500;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322501, 1 );
      QGSCOMPARENEAR( y, 196160, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2068, 1 );
      QGSCOMPARENEAR( y, 2273, 1 );

      x = 1102;
      y = 2300;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321803, 1 );
      QGSCOMPARENEAR( y, 195999, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1103, 1 );
      QGSCOMPARENEAR( y, 2177, 1 );
    }

    void testTPSRasterYAxis()
    {
      QgsGDALGeorefTransform transform( true, 1 );
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, -716 )
                                          << QgsPointXY( 2352, -1126 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 1102, -2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 288;
      double y = -716;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321202, 1 );
      QGSCOMPARENEAR( y, 63505, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 297, 1 );
      QGSCOMPARENEAR( y, -705, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322686, 1 );
      QGSCOMPARENEAR( y, 24959, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2361, 1 );
      QGSCOMPARENEAR( y, -1116, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322474, 1 );
      QGSCOMPARENEAR( y, -31676, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2078, 1 );
      QGSCOMPARENEAR( y, -2385, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321780, 1 );
      QGSCOMPARENEAR( y, -13787, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1114, 1 );
      QGSCOMPARENEAR( y, -2195, 1 );
    }

    void testTPSExact()
    {
      QgsGDALGeorefTransform transform( true, 1 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    void testTPSExactRasterYAxis()
    {
      QgsGDALGeorefTransform transform( true, 1 );
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, -123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, -140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, -194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, -186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    //
    // Projective
    //

    void testProjective()
    {
      QgsProjectiveGeorefTransform transform;
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                          << QgsPointXY( 2352, 716 )
                                          << QgsPointXY( 2067, 2398 )
                                          << QgsPointXY( 1102, 2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 288;
      double y = 1000;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321256, 1 );
      QGSCOMPARENEAR( y, 123764, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, 1000, 1 );

      x = 2352;
      y = 1150;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322687, 1 );
      QGSCOMPARENEAR( y, 142908, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, 1150, 1 );

      x = 2067;
      y = 2500;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322494, 1 );
      QGSCOMPARENEAR( y, 197069, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, 2500, 1 );

      x = 1102;
      y = 2300;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321781, 1 );
      QGSCOMPARENEAR( y, 198051, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, 2300, 1 );
    }

    void testProjectiveRasterYAxis()
    {
      QgsProjectiveGeorefTransform transform;
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 288, -716 )
                                          << QgsPointXY( 2352, -1126 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 1102, -2209 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 288;
      double y = -716;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321203, 1 );
      QGSCOMPARENEAR( y, 63965, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, -716, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322681, 1 );
      QGSCOMPARENEAR( y, 25366, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, -1126, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322467, 1 );
      QGSCOMPARENEAR( y, -30635, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, -2398, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321777, 1 );
      QGSCOMPARENEAR( y, -12647, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, -2209, 1 );
    }

    void testProjectiveExact()
    {
      QgsProjectiveGeorefTransform transform;
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ) );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }

    void testProjectiveExactRasterYAxis()
    {
      QgsProjectiveGeorefTransform transform;
      //this is a identity transform!
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 192198 ), true );

      double x = 321212;
      double y = 123003;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, -123003, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 123003, 1 );

      x = 322702;
      y = 140444;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, -140444, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 140444, 1 );

      x = 322496;
      y = 194554;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, -194554, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 194554, 1 );

      x = 321800;
      y = 186514;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, -186514, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 186514, 1 );
    }


    // geometry transformer
    void testGeometryTransformer()
    {
      QgsGcpGeometryTransformer transformer( QgsGcpTransformerInterface::TransformMethod::Projective,
                                             QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                             << QgsPointXY( 2352, 716 )
                                             << QgsPointXY( 2067, 2398 )
                                             << QgsPointXY( 1102, 2209 ),
                                             QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                             << QgsPointXY( 322698, 129979 )
                                             << QgsPointXY( 322501, 192061 )
                                             << QgsPointXY( 321803, 192198 ) );

      const QgsGeometry geom = QgsGeometry::fromWkt( QStringLiteral( "LineString(288 1000, 2352 1150, 2067 2500, 1102 2300)" ) );
      bool ok = false;
      QCOMPARE( transformer.transform( geom, ok ).asWkt( 0 ), QStringLiteral( "LineString (321256 123764, 322688 142909, 322495 197069, 321782 198051)" ) );
      QVERIFY( ok );

      // invalid parameters -- not enough GCPs
      QgsGcpGeometryTransformer transformer2( QgsGcpTransformerInterface::TransformMethod::PolynomialOrder2,
                                              QVector< QgsPointXY >() << QgsPointXY( 288, 1126 )
                                              << QgsPointXY( 2352, 716 )
                                              << QgsPointXY( 2067, 2398 )
                                              << QgsPointXY( 1102, 2209 ),
                                              QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                              << QgsPointXY( 322698, 129979 )
                                              << QgsPointXY( 322501, 192061 )
                                              << QgsPointXY( 321803, 192198 ) );
      QCOMPARE( transformer2.transform( geom, ok ).asWkt( 0 ), QStringLiteral( "LineString (288 1000, 2352 1150, 2067 2500, 1102 2300)" ) );
      QVERIFY( !ok );
    }


};

QGSTEST_MAIN( TestQgsGcpTransformer )

#include "testqgsgcptransformer.moc"
