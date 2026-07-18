/***************************************************************************
     testqgsgeos.cpp
     --------------------------------------
    Date                 : July 2026
    Copyright            : (C) 2026 by Germán Carrillo
    Email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <geos_c.h>

#include "qgscircularstring.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgstest.h"

#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsGeos : public QObject
{
    Q_OBJECT
  private slots:
    void compareGeoms( const QgsAbstractGeometry *inputGeom );
    void geometryConversionPoint();
    //void geometryConversionMultiPoint();
    void geometryConversionLineString();
    //void geometryConversionMultiLineString();
#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 15 )
    void geometryConversionCircularString();
    //void geometryConversionCompoundCurve();
    //void geometryConversionMultiCurve();
#endif
    //void geometryConversionPolygon();
    //void geometryConversionMultiPolygon();
    //void geometryConversionTriangle();
    //void geometryConversionCurvePolygon();
    //void geometryConversionMultiSurface();
};

void TestQgsGeos::compareGeoms( const QgsAbstractGeometry *inputGeom )
{
  geos::unique_ptr geos = QgsGeos::asGeos( inputGeom );
  GEOSContextHandle_t context = QgsGeosContext::get();
  bool hasZ = GEOSHasZ_r( context, geos.get() );
  QCOMPARE( inputGeom->is3D(), hasZ );
  bool hasM = GEOSHasM_r( context, geos.get() );
  QCOMPARE( inputGeom->isMeasure(), hasM );

  auto geom = QgsGeos::fromGeos( geos.release() );
  QCOMPARE( inputGeom->is3D(), geom->is3D() );
  QCOMPARE( inputGeom->isMeasure(), geom->isMeasure() );
  QVERIFY( geom );
  QCOMPARE( inputGeom->asWkt(), geom->asWkt() );
}

void TestQgsGeos::geometryConversionPoint()
{
  compareGeoms( std::make_unique< QgsPoint >( 5, 7 ).get() );
  compareGeoms( std::make_unique< QgsPoint >( 5, 7, 10 ).get() );
  //compareGeoms( std::make_unique< QgsPoint >( 5, 7, 10, 20 ).get() ); Not yet supported
  //compareWkts( std::make_unique< QgsPoint >( Qgis::WkbType::PointM, 5, 7, 20).get() ); Not yet supported
}

void TestQgsGeos::geometryConversionLineString()
{
  compareGeoms( std::make_unique< QgsLineString >( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ) ).get() );
  compareGeoms( std::make_unique< QgsLineString >( QgsPoint( 0, 0, 7 ), QgsPoint( 1, 1, 8 ) ).get() );
  //compareGeoms( std::make_unique< QgsLineString >( QgsPoint( 0, 0, 7, 2 ), QgsPoint( 1, 1, 8, 3 ) ).get() );  Not yet supported
}

#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 15 )
void TestQgsGeos::geometryConversionCircularString()
{
  compareGeoms( std::make_unique< QgsCircularString >( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 1, 0 ) ).get() );
  compareGeoms( std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 7 ), QgsPoint( 1, 1, 8 ), QgsPoint( 1, 0, 9 ) ).get() );
  //compareGeoms( std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 7, 2 ), QgsPoint( 1, 1, 8, 3 ), QgsPoint( 1, 0, 9, 4 ) ).get() );  Not yet supported
}
#endif

QGSTEST_MAIN( TestQgsGeos )
#include "testqgsgeos.moc"
