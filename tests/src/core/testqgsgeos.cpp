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
    void compareGeoms( const QgsAbstractGeometry *inputGeom, const QString expectedWkt = QString() );
    void geometryConversionPoint();
    void geometryConversionMultiPoint();
    void geometryConversionLineString();
    void geometryConversionMultiLineString();
#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 15 )
    void geometryConversionCircularString();
    void geometryConversionCompoundCurve();
    void geometryConversionMultiCurve();
#endif
    void geometryConversionPolygon();
    void geometryConversionMultiPolygon();
    void geometryConversionTriangle();
#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 15 )
    void geometryConversionCurvePolygon();
    void geometryConversionMultiSurface();
    void geometryConversionGeometryCollection();
#endif

    void geosCollectionAndEmptyParts();

    void pointGeometryFromGeos_data();
    void pointGeometryFromGeos();
    void linestringGeometryFromGeos_data();
    void linestringGeometryFromGeos();
};

void TestQgsGeos::compareGeoms( const QgsAbstractGeometry *inputGeom, const QString expectedWkt )
{
  //Compares several aspects of an input QGIS geometry
  //vs. the corresponding converted GEOS version and the
  //final QGIS geometry obtained when converted from GEOS.
  geos::unique_ptr geos = QgsGeos::asGeos( inputGeom );
  QVERIFY( geos );
  GEOSContextHandle_t context = QgsGeosContext::get();
  bool hasZ = GEOSHasZ_r( context, geos.get() );
  QCOMPARE( inputGeom->is3D(), hasZ );
  bool hasM = GEOSHasM_r( context, geos.get() );
  QCOMPARE( inputGeom->isMeasure(), hasM );

  auto geom = QgsGeos::fromGeos( geos.release() );
  QCOMPARE( inputGeom->is3D(), geom->is3D() );
  QCOMPARE( inputGeom->isMeasure(), geom->isMeasure() );
  QVERIFY( geom );
  // Some types like QgsTriangle don't have 1:1 correspondence with GEOS,
  // so we do some type conversions to be able to match GEOS geometry types.
  // E.g., QgsTriangle is converted to a Polygon, and then we expect a polygon back.
  QCOMPARE( expectedWkt.isEmpty() ? inputGeom->asWkt() : expectedWkt, geom->asWkt() );
}

void TestQgsGeos::geometryConversionPoint()
{
  // compareGeoms( std::make_unique< QgsPoint >().get() ); //Empty geometry, not yet supported

  compareGeoms( std::make_unique< QgsPoint >( 5, 7 ).get() );     //Point
  compareGeoms( std::make_unique< QgsPoint >( 5, 7, 10 ).get() ); //PointZ
  //compareGeoms( std::make_unique< QgsPoint >( 5, 7, 10, 20 ).get() ); //PointZM, not yet supported
  //compareWkts( std::make_unique< QgsPoint >( Qgis::WkbType::PointM, 5, 7, 20).get() ); //PointM, not yet supported
}

void TestQgsGeos::geometryConversionMultiPoint()
{
  compareGeoms( std::make_unique< QgsMultiPoint >().get() ); //Empty geometry

  QVector< QgsPoint > points;
  points << QgsPoint( 5, 7 );
  compareGeoms( std::make_unique< QgsMultiPoint >( points ).get() ); //MultiPoint

  points.clear();
  points << QgsPoint( 8, 4, 100 ) << QgsPoint( 5, 7, 150 );
  compareGeoms( std::make_unique< QgsMultiPoint >( points ).get() ); //MultiPointZ

  //points.clear();
  //points << QgsPoint ( 8, 4, 100, 0.1 ) << QgsPoint( 5, 7, 150, 0.7 );
  //compareGeoms( std::make_unique< QgsMultiPoint >( points ).get() ); //MultiPointZM, not yet supported

  //points.clear();
  //points << QgsPoint( Qgis::WkbType::PointM, 5, 7, 20) << QgsPoint( Qgis::WkbType::PointM, 2, 9, 60);
  //compareGeoms( std::make_unique< QgsMultiPoint >( points ).get() ); //MultiPointM, not yet supported
}

void TestQgsGeos::geometryConversionLineString()
{
  //compareGeoms( std::make_unique< QgsLineString >().get() ); //Empty geometry, not yet supported

  compareGeoms( std::make_unique< QgsLineString >( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ) ).get() );       //LineString
  compareGeoms( std::make_unique< QgsLineString >( QgsPoint( 0, 0, 7 ), QgsPoint( 1, 1, 8 ) ).get() ); //LineStringZ
  //compareGeoms( std::make_unique< QgsLineString >( QgsPoint( 0, 0, 7, 2 ), QgsPoint( 1, 1, 8, 3 ) ).get() ); //LineStringZM, not yet supported
}

void TestQgsGeos::geometryConversionMultiLineString()
{
  compareGeoms( std::make_unique< QgsMultiLineString >().get() ); //Empty geometry

  QList< QgsLineString > lines;
  lines << QgsLineString( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ) );
  compareGeoms( std::make_unique< QgsMultiLineString >( lines ).get() ); //MultiLineString

  lines.clear();
  lines << QgsLineString( QgsPoint( 0, 0, 100 ), QgsPoint( 1, 1, 200 ) );
  compareGeoms( std::make_unique< QgsMultiLineString >( lines ).get() ); //MultiLineStringZ

  //lines.clear();
  //lines << QgsLineString( QgsPoint( 0, 0, 100, 0.1 ), QgsPoint( 1, 1, 200, 0.7 ) );
  //compareGeoms( std::make_unique< QgsMultiLineString >( lines ).get() ); //MultiLineStringZM, not yet supported

  //lines.clear();
  //lines << QgsLineString( QgsPoint( Qgis::WkbType::PointM, 5, 7, 20 ), QgsPoint( Qgis::WkbType::PointM, 2, 9, 60 ) );
  //compareGeoms( std::make_unique< QgsMultiLineString >( lines ).get() ); //MultiLineStringM, not yet supported
}

#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 15 )
void TestQgsGeos::geometryConversionCircularString()
{
  compareGeoms( std::make_unique< QgsCircularString >().get() );                                                                //Empty geometry
  compareGeoms( std::make_unique< QgsCircularString >( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 1, 0 ) ).get() );          //CircularString
  compareGeoms( std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 7 ), QgsPoint( 1, 1, 8 ), QgsPoint( 1, 0, 9 ) ).get() ); //CircularStringZ
  //compareGeoms( std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 7, 2 ), QgsPoint( 1, 1, 8, 3 ), QgsPoint( 1, 0, 9, 4 ) ).get() ); //CircularStringZM, not yet supported
}

void TestQgsGeos::geometryConversionCompoundCurve()
{
  auto cc = std::make_unique< QgsCompoundCurve >();
  compareGeoms( cc.get() ); //Empty geometry

  auto cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 1, 0 ) );
  auto ls = std::make_unique< QgsLineString >( QgsPoint( 1, 0 ), QgsPoint( 2, 2 ) );
  cc->addCurve( cs->clone() );
  cc->addCurve( ls->clone() );
  compareGeoms( cc.get() ); //CompoundCurve

  cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 100 ), QgsPoint( 1, 1, 200 ), QgsPoint( 1, 0, 150 ) );
  ls = std::make_unique< QgsLineString >( QgsPoint( 1, 0, 150 ), QgsPoint( 2, 2, 175 ) );
  cc->clear();
  cc->addCurve( cs->clone() );
  cc->addCurve( ls->clone() );
  compareGeoms( cc.get() ); //CompoundCurveZ

  auto ls2 = std::make_unique< QgsLineString >( QgsPoint( 2, 2, 175 ), QgsPoint( 2, 0, 100 ) );
  cc->addCurve( ls2->clone(), true ); //extendPrevious
  compareGeoms( cc.get() );

  /*
  cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 100, 2 ), QgsPoint( 1, 1, 200, 4 ), QgsPoint( 1, 0, 150, 6 ) );
  ls = std::make_unique< QgsLineString >( QgsPoint( 1, 0, 150, 6 ), QgsPoint( 2, 2, 175, 8 ) );
  cc->clear();
  cc->addCurve( cs->clone() );
  cc->addCurve( ls->clone() );
  compareGeoms( cc.get() ); //CompoundCurveZM, not yet supported
  */
}

void TestQgsGeos::geometryConversionMultiCurve()
{
  auto mc = std::make_unique< QgsMultiCurve >();
  compareGeoms( mc.get() ); //Empty geometry

  auto cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 1, 0 ) );
  auto ls = std::make_unique< QgsLineString >( QgsPoint( 1, 0 ), QgsPoint( 2, 2 ) );
  auto cc = std::make_unique< QgsCompoundCurve >();
  cc->addCurve( cs->clone() );
  cc->addCurve( ls->clone() );
  mc->addGeometry( cs->clone() );
  mc->addGeometry( ls->clone() );
  mc->addGeometry( cc->clone() );
  compareGeoms( mc.get() ); //MultiCurve

  cc->clear();
  mc->clear();
  cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 100 ), QgsPoint( 1, 1, 200 ), QgsPoint( 1, 0, 150 ) );
  ls = std::make_unique< QgsLineString >( QgsPoint( 1, 0, 150 ), QgsPoint( 2, 2, 175 ) );
  cc->addCurve( cs->clone() );
  cc->addCurve( ls->clone() );
  mc->addGeometry( cs->clone() );
  mc->addGeometry( ls->clone() );
  mc->addGeometry( cc->clone() );
  compareGeoms( mc.get() ); //MultiCurveZ

  /*
  cc->clear();
  mc->clear();
  cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 100, 2 ), QgsPoint( 1, 1, 200, 4 ), QgsPoint( 1, 0, 150, 6 ) );
  ls = std::make_unique< QgsLineString >( QgsPoint( 1, 0, 150, 6 ), QgsPoint( 2, 2, 175, 8 ) );
  cc->addCurve( cs->clone() );
  cc->addCurve( ls->clone() );
  mc->addGeometry( cs->clone() );
  mc->addGeometry( ls->clone() );
  mc->addGeometry( cc->clone() );
  compareGeoms( mc.get() ); //MultiCurveZM, not yet supported
  */
}
#endif

void TestQgsGeos::geometryConversionPolygon()
{
  auto polygon = std::make_unique< QgsPolygon >();
  polygon->fromWkt( u"POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10))"_s );
  compareGeoms( polygon.get() ); //Polygon

  //polygon->fromWkt( u"POLYGON ( EMPTY )"_s );
  //compareGeoms( polygon.get() ); //Empty geometry, not yet supported

  polygon->fromWkt( u"POLYGON ((30 10 100, 10 20 150, 20 40 200, 40 40 80, 30 10 100))"_s );
  compareGeoms( polygon.get() ); //PolygonZ

  // polygon->fromWkt( u"POLYGON ((30 10 100 2, 10 20 150 4, 20 40 200 8, 40 40 80 5, 30 10 100 2))"_s );
  // compareGeoms( polygon.get() ); //PolygonZM, not yet supported
}

void TestQgsGeos::geometryConversionMultiPolygon()
{
  auto multiPolygon = std::make_unique< QgsMultiPolygon >();
  compareGeoms( multiPolygon.get() ); //Empty geometry

  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints(
    QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 5, 50, 1 ) << QgsPoint( Qgis::WkbType::PointZ, 6, 61, 3 ) << QgsPoint( Qgis::WkbType::PointZ, 9, 71, 4 ) << QgsPoint( Qgis::WkbType::PointZ, 5, 71, 4 )
  );
  part.setExteriorRing( ring.clone() );
  multiPolygon->addGeometry( part.clone() );
  compareGeoms( multiPolygon.get() ); //MultiPolygonZ, single PolygonZ inside

  multiPolygon->addGeometry( part.clone() );
  compareGeoms( multiPolygon.get() ); //MultiPolygonZ, several PolygonZs inside

  /*
  multiPolygon->clear();
  ring.setPoints(
    QgsPointSequence()
    << QgsPoint( Qgis::WkbType::PointZM, 5, 50, 1, 4 )
    << QgsPoint( Qgis::WkbType::PointZM, 6, 61, 3, 5 )
    << QgsPoint( Qgis::WkbType::PointZM, 9, 71, 4, 15 )
    << QgsPoint( Qgis::WkbType::PointZM, 5, 71, 4, 6 )
    );
  part.setExteriorRing( ring.clone() );
  multiPolygon->addGeometry( part.clone() );
  compareGeoms( multiPolygon.get() ); //MultiPolygonZM, singlePolygonZs inside, not yet supported

  multiPolygon->addGeometry( part.clone() );
  compareGeoms( multiPolygon.get() ); //MultiPolygonZM, several PolygonZs inside, not yet supported
  */
}

void TestQgsGeos::geometryConversionTriangle()
{
  auto triangle = std::make_unique< QgsTriangle >();
  //compareGeoms( triangle.get() ); //Empty geometry, not yet supported

  triangle = std::make_unique< QgsTriangle >( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  compareGeoms( triangle.get(), u"Polygon ((0 0, 0 10, 10 10, 0 0))"_s ); //Triangle

  triangle = std::make_unique< QgsTriangle >( QgsPoint( 0, 0, 100 ), QgsPoint( 0, 10, 150 ), QgsPoint( 10, 10, 200 ) );
  compareGeoms( triangle.get(), u"Polygon Z ((0 0 100, 0 10 150, 10 10 200, 0 0 100))"_s ); //TriangleZ

  //triangle = std::make_unique< QgsTriangle >( QgsPoint( 0, 0, 100, 2), QgsPoint( 0, 10, 150, 4), QgsPoint( 10, 10, 200, 6 ) );
  //compareGeoms( triangle.get(), u"Polygon Z ((0 0 100 2, 0 10 150 4, 10 10 200 6, 0 0 100 2))"_s ); //TriangleZM, not yet supported
}

#if GEOS_VERSION_MAJOR > 3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 15 )
void TestQgsGeos::geometryConversionCurvePolygon()
{
  auto cp = std::make_unique< QgsCurvePolygon >();
  //compareGeoms( cp.get() ); //Empty geometry, not yet supported

  auto ls = std::make_unique< QgsLineString >();
  ls->setPoints( QgsPointSequence() << QgsPoint( 2, 0 ) << QgsPoint( 0, 0 ) << QgsPoint( 2, 2 ) );
  auto cs = std::make_unique< QgsCircularString >( QgsPoint( 2, 2 ), QgsPoint( 3, 1 ), QgsPoint( 2, 0 ) );
  auto cc = std::make_unique< QgsCompoundCurve >();
  cc->addCurve( ls->clone() );
  cc->addCurve( cs->clone() );
  cp->setExteriorRing( cc->clone() );
  compareGeoms( cp.get() ); //CurvePolygon

  ls = std::make_unique< QgsLineString >();
  ls->setPoints( QgsPointSequence() << QgsPoint( 2, 0, 100 ) << QgsPoint( 0, 0, 150 ) << QgsPoint( 2, 2, 200 ) );
  cs = std::make_unique< QgsCircularString >( QgsPoint( 2, 2, 200 ), QgsPoint( 3, 1, 50 ), QgsPoint( 2, 0, 100 ) );
  cc = std::make_unique< QgsCompoundCurve >();
  cp = std::make_unique< QgsCurvePolygon >();
  cc->addCurve( ls->clone() );
  cc->addCurve( cs->clone() );
  cp->setExteriorRing( cc->clone() );
  compareGeoms( cp.get() ); //CurvePolygonZ

  /*
  ls = std::make_unique< QgsLineString >();
  ls->setPoints( QgsPointSequence() <<  QgsPoint( 2, 0, 100, 0.5 ) << QgsPoint( 0, 0, 150, 0.6 ) << QgsPoint( 2, 2, 200, 0.7 ) );
  cs = std::make_unique< QgsCircularString >( QgsPoint( 2, 2, 200, 0.7 ), QgsPoint( 3, 1, 50, 0.8 ), QgsPoint( 2, 0, 100, 0.9 ) );
  cc = std::make_unique< QgsCompoundCurve >();
  cp = std::make_unique< QgsCurvePolygon >();
  cc->addCurve( ls->clone() );
  cc->addCurve( cs->clone() );
  cp->setExteriorRing( cc->clone() );
  compareGeoms( cp.get() ); //CurvePolygonZM, not yet supported
  */
}

void TestQgsGeos::geometryConversionMultiSurface()
{
  auto multiSurface = std::make_unique< QgsMultiSurface >();
  compareGeoms( multiSurface.get() ); //Empty geometry

  QgsPolygon polygon;
  QgsLineString ring;

  ring.setPoints(
    QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 5, 50, 1 ) << QgsPoint( Qgis::WkbType::PointZ, 6, 61, 3 ) << QgsPoint( Qgis::WkbType::PointZ, 9, 71, 4 ) << QgsPoint( Qgis::WkbType::PointZ, 5, 71, 4 )
  );
  polygon.setExteriorRing( ring.clone() );
  multiSurface->addGeometry( polygon.clone() );
  compareGeoms( multiSurface.get() ); //Single PolygonZ

  auto ls = std::make_unique< QgsLineString >();
  ls->setPoints( QgsPointSequence() << QgsPoint( 2, 0, 100 ) << QgsPoint( 0, 0, 150 ) << QgsPoint( 2, 2, 200 ) );
  auto cs = std::make_unique< QgsCircularString >( QgsPoint( 2, 2, 200 ), QgsPoint( 3, 1, 50 ), QgsPoint( 2, 0, 100 ) );
  auto cc = std::make_unique< QgsCompoundCurve >();
  auto cp = std::make_unique< QgsCurvePolygon >();
  cc->addCurve( ls->clone() );
  cc->addCurve( cs->clone() );
  cp->setExteriorRing( cc->clone() );
  multiSurface->addGeometry( cp->clone() );
  compareGeoms( multiSurface.get() ); //PolygonZ and CurvePolygonZ

  /*
  multiSurface->clear();
  ring.setPoints(
    QgsPointSequence()
    << QgsPoint( Qgis::WkbType::PointZM, 5, 50, 1, 4 )
    << QgsPoint( Qgis::WkbType::PointZM, 6, 61, 3, 5 )
    << QgsPoint( Qgis::WkbType::PointZM, 9, 71, 4, 15 )
    << QgsPoint( Qgis::WkbType::PointZM, 5, 71, 4, 6 )
    );
  polygon.setExteriorRing( ring.clone() );
  multiSurface->addGeometry( polygon.clone() );
  compareGeoms( multiSurface.get() ); //Single PolygonZM, not yet supported

  ls = std::make_unique< QgsLineString >();
  ls->setPoints( QgsPointSequence() <<  QgsPoint( 2, 0, 100 ) << QgsPoint( 0, 0, 150 ) << QgsPoint( 2, 2, 200 ) );
  cs = std::make_unique< QgsCircularString >( QgsPoint( 2, 2, 200 ), QgsPoint( 3, 1, 50 ), QgsPoint( 2, 0, 100 ) );
  cc = std::make_unique< QgsCompoundCurve >();
  cp = std::make_unique< QgsCurvePolygon >();
  cc->addCurve( ls->clone() );
  cc->addCurve( cs->clone() );
  cp->setExteriorRing( cc->clone() );
  multiSurface->addGeometry( cp->clone() );
  compareGeoms( multiSurface.get() ); //PolygonZM and CurvePolygonZM, not yet supported
  */
}

void TestQgsGeos::geometryConversionGeometryCollection()
{
  auto collection = std::make_unique< QgsGeometryCollection >();
  compareGeoms( collection.get() ); //Empty geometry

  QVector< QgsPoint > points;
  points << QgsPoint( 8, 4 ) << QgsPoint( 5, 7 );
  collection->addGeometry( std::make_unique< QgsMultiPoint >( points ).release() );
  compareGeoms( collection.get() ); //Multi geometry inside collection

  auto cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 1, 0 ) );
  collection->addGeometry( cs->clone() );
  compareGeoms( collection.get() ); //Multi geometry and simple curved geometry inside collection

  auto ls = std::make_unique< QgsLineString >();
  ls->setPoints( QgsPointSequence() << QgsPoint( 2, 0 ) << QgsPoint( 0, 0 ) << QgsPoint( 2, 2 ) );
  cs = std::make_unique< QgsCircularString >( QgsPoint( 2, 2 ), QgsPoint( 3, 1 ), QgsPoint( 2, 0 ) );
  auto cc = std::make_unique< QgsCompoundCurve >();
  auto cp = std::make_unique< QgsCurvePolygon >();
  cc->addCurve( ls->clone() );
  cc->addCurve( cs->clone() );
  cp->setExteriorRing( cc->clone() );
  collection->addGeometry( cp->clone() );
  compareGeoms( collection.get() ); //Multi geom, CircularString and CurvePolygon inside collection

  /*
  collection->clear();
  points.clear();
  points << QgsPoint ( 8, 4, 100 ) << QgsPoint( 5, 7, 150 );
  collection->addGeometry( std::make_unique< QgsMultiPoint >( points ).release() );
  compareGeoms( collection.get() ); //Multi geometry Z inside collection, not yet supported

  cs = std::make_unique< QgsCircularString >( QgsPoint( 0, 0, 7 ), QgsPoint( 1, 1, 8 ), QgsPoint( 1, 0, 9 ) );
  collection->addGeometry( cs->clone() );
  compareGeoms( collection.get() ); //Multi geometry Z and simple curved geometry Z inside collection, not yet supported

  ls = std::make_unique< QgsLineString >();
  ls->setPoints( QgsPointSequence() <<  QgsPoint( 2, 0, 100 ) << QgsPoint( 0, 0, 150 ) << QgsPoint( 2, 2, 400 ) );
  cs = std::make_unique< QgsCircularString >( QgsPoint( 2, 2, 200 ), QgsPoint( 3, 1, 220 ), QgsPoint( 2, 0, 230 ) );
  cc->clear();
  cc->addCurve( ls->clone() );
  cc->addCurve( cs->clone() );
  cp->setExteriorRing( cc->clone() );
  collection->addGeometry( cp->clone() );
  compareGeoms( collection.get() ); //Multi geomZ, CircularStringZ and CurvePolygonZ inside collection, not yet supported
  */
}
#endif

void TestQgsGeos::geosCollectionAndEmptyParts()
{
  // test GEOS conversion utils

  // empty parts should NOT be added to a GEOS collection -- it can cause crashes in GEOS
  QgsMultiPolygon polyWithEmptyParts;
  geos::unique_ptr asGeos( QgsGeos::asGeos( &polyWithEmptyParts ) );
  QgsGeometry res( QgsGeos::fromGeos( asGeos.get() ) );
  QCOMPARE( res.asWkt(), u"MultiPolygon EMPTY"_s );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString() ) );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) ) ) );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString() ) );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString( QVector<QgsPoint>() << QgsPoint( 10, 0 ) << QgsPoint( 10, 1 ) << QgsPoint( 11, 1 ) << QgsPoint( 10, 0 ) ) ) );
  asGeos = QgsGeos::asGeos( &polyWithEmptyParts );
  QCOMPARE( GEOSGetNumGeometries_r( QgsGeosContext::get(), asGeos.get() ), 2 );
  res = QgsGeometry( QgsGeos::fromGeos( asGeos.get() ) );
  QCOMPARE( res.asWkt(), u"MultiPolygon (((0 0, 0 1, 1 1, 0 0)),((10 0, 10 1, 11 1, 10 0)))"_s );

  // Empty geometry
  QgsPoint point;
  asGeos = QgsGeos::asGeos( &point );
  // should be treated as a null geometry, not an empty point in order to maintain api compatibility with
  // earlier QGIS 3.x releases
  QVERIFY( !QgsGeos::fromGeos( asGeos.get() ) );
}

void TestQgsGeos::pointGeometryFromGeos_data()
{
  QTest::addColumn<double>( "x" );
  QTest::addColumn<double>( "y" );

  QTest::newRow( "point_geos_1" ) << 30.0 << 10.0;
}

void TestQgsGeos::pointGeometryFromGeos()
{
  QFETCH( double, x );
  QFETCH( double, y );

  GEOSContextHandle_t context = QgsGeosContext::get();
  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( context, 1, 2 );
  GEOSCoordSeq_setX_r( context, coord, 0, x );
  GEOSCoordSeq_setY_r( context, coord, 0, y );
  GEOSGeometry *geosPt = GEOSGeom_createPoint_r( context, coord );

  const QgsGeometry geom = QgsGeos::geometryFromGeos( geosPt );
  QVERIFY( geom.wkbType() == Qgis::WkbType::Point );

  const QgsPointXY geomPt = geom.asPoint();

  QGSCOMPARENEAR( x, geomPt.x(), 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( y, geomPt.y(), 4 * std::numeric_limits<double>::epsilon() );
}

void TestQgsGeos::linestringGeometryFromGeos_data()
{
  QTest::addColumn<QVariantList>( "line" );
  QVariantList line;
  line << QVariant( QPointF( 30.0, 10.0 ) ) << QVariant( QPointF( 10.0, 30.0 ) ) << QVariant( QPointF( 40.0, 40.0 ) );
  QTest::newRow( "linestring_geos_1" ) << line;
}

void TestQgsGeos::linestringGeometryFromGeos()
{
  QFETCH( QVariantList, line );
  GEOSContextHandle_t context = QgsGeosContext::get();

  //create geos coord sequence first
  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( context, line.count(), 2 );
  for ( int i = 0; i < line.count(); i++ )
  {
    const QPointF pt = line.at( i ).toPointF();
    GEOSCoordSeq_setX_r( context, coord, i, pt.x() );
    GEOSCoordSeq_setY_r( context, coord, i, pt.y() );
  }
  GEOSGeometry *geosLine = GEOSGeom_createLineString_r( context, coord );
  const QgsGeometry geom = QgsGeos::geometryFromGeos( geosLine );
  QVERIFY( geom.wkbType() == Qgis::WkbType::LineString );

  const QgsPolylineXY polyline = geom.asPolyline();

  QCOMPARE( polyline.size(), line.size() );
  for ( int i = 0; i < polyline.size(); ++i )
  {
    const QgsPointXY &polylinePt = polyline.at( i );
    const QPointF linePt = line.at( i ).toPointF();
    QGSCOMPARENEAR( polylinePt.x(), linePt.x(), 4 * std::numeric_limits<double>::epsilon() );
    QGSCOMPARENEAR( polylinePt.y(), linePt.y(), 4 * std::numeric_limits<double>::epsilon() );
  }
}

QGSTEST_MAIN( TestQgsGeos )
#include "testqgsgeos.moc"
