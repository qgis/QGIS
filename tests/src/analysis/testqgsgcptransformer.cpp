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
      transform.updateParametersFromGcps( QVector< QgsPointXY >() << QgsPointXY( 321210, 130280 )
                                          << QgsPointXY( 322698, 129979 )
                                          << QgsPointXY( 322501, 192061 )
                                          << QgsPointXY( 321803, 129198 ),
                                          QVector< QgsPointXY >() << QgsPointXY( 288, -716 )
                                          << QgsPointXY( 2352, -1126 )
                                          << QgsPointXY( 2067, -2398 )
                                          << QgsPointXY( 1102, -2209 ) );

      double x = 288;
      double y = -71;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321212, 1 );
      QGSCOMPARENEAR( y, 108597, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 288, 1 );
      QGSCOMPARENEAR( y, -71, 1 );

      x = 2352;
      y = -1126;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322702, 1 );
      QGSCOMPARENEAR( y, 133775, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2352, 1 );
      QGSCOMPARENEAR( y, -1126, 1 );

      x = 2067;
      y = -2398;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 322496, 1 );
      QGSCOMPARENEAR( y, 164131, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 2067, 1 );
      QGSCOMPARENEAR( y, -2398, 1 );

      x = 1102;
      y = -2209;
      QVERIFY( transform.transform( x, y ) );
      QGSCOMPARENEAR( x, 321800, 1 );
      QGSCOMPARENEAR( y, 159620, 1 );
      QVERIFY( transform.transform( x, y, true ) );
      QGSCOMPARENEAR( x, 1102, 1 );
      QGSCOMPARENEAR( y, -2209, 1 );
    }

};

QGSTEST_MAIN( TestQgsGcpTransformer )

#include "testqgsgcptransformer.moc"
