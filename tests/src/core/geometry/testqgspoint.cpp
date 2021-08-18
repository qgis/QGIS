/***************************************************************************
     testqgspoint.cpp
     --------------------------------------
    Date                 : August 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
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
#include <QString>
//header for class being tested
#include "qgsproject.h"
#include "qgspoint.h"
#include "qgslinestring.h"
#include "testtransformer.h"
#include "testgeometryutils.h"

class TestQgsPoint: public QObject
{
    Q_OBJECT
  private slots:
    void point();
};

void TestQgsPoint::point()
{
  //test QgsPointV2
  QgsPoint pEmpty;
  QVERIFY( pEmpty.isEmpty() );
  QCOMPARE( pEmpty.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( pEmpty.asWkt(), QStringLiteral( "Point EMPTY" ) );
  pEmpty.setX( 1.0 );
  QVERIFY( pEmpty.isEmpty() );
  QCOMPARE( pEmpty.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( pEmpty.asWkt(), QStringLiteral( "Point EMPTY" ) );
  pEmpty.setY( 2.0 );
  QVERIFY( !pEmpty.isEmpty() );
  QCOMPARE( pEmpty.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( pEmpty.asWkt(), QStringLiteral( "Point (1 2)" ) );

  //test constructors
  QgsPoint p1( 5.0, 6.0 );
  QCOMPARE( p1.x(), 5.0 );
  QCOMPARE( p1.y(), 6.0 );
  QVERIFY( !p1.isEmpty() );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( p1.wktTypeStr(), QString( "Point" ) );

  QgsPoint p2( QgsPointXY( 3.0, 4.0 ) );
  QCOMPARE( p2.x(), 3.0 );
  QCOMPARE( p2.y(), 4.0 );
  QVERIFY( !p2.isEmpty() );
  QVERIFY( !p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWkbTypes::Point );

  QgsPoint p3( QPointF( 7.0, 9.0 ) );
  QCOMPARE( p3.x(), 7.0 );
  QCOMPARE( p3.y(), 9.0 );
  QVERIFY( !p3.isEmpty() );
  QVERIFY( !p3.is3D() );
  QVERIFY( !p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWkbTypes::Point );

  QgsPoint p4( QgsWkbTypes::Point, 11.0, 13.0 );
  QCOMPARE( p4.x(), 11.0 );
  QCOMPARE( p4.y(), 13.0 );
  QVERIFY( !p4.isEmpty() );
  QVERIFY( !p4.is3D() );
  QVERIFY( !p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWkbTypes::Point );

  QgsPoint p5( QgsWkbTypes::PointZ, 11.0, 13.0, 15.0 );
  QCOMPARE( p5.x(), 11.0 );
  QCOMPARE( p5.y(), 13.0 );
  QCOMPARE( p5.z(), 15.0 );
  QVERIFY( !p5.isEmpty() );
  QVERIFY( p5.is3D() );
  QVERIFY( !p5.isMeasure() );
  QCOMPARE( p5.wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( p5.wktTypeStr(), QString( "PointZ" ) );

  QgsPoint p6( QgsWkbTypes::PointM, 11.0, 13.0, 0.0, 17.0 );
  QCOMPARE( p6.x(), 11.0 );
  QCOMPARE( p6.y(), 13.0 );
  QCOMPARE( p6.m(), 17.0 );
  QVERIFY( !p6.isEmpty() );
  QVERIFY( !p6.is3D() );
  QVERIFY( p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::PointM );
  QCOMPARE( p6.wktTypeStr(), QString( "PointM" ) );

  QgsPoint p7( QgsWkbTypes::PointZM, 11.0, 13.0, 0.0, 17.0 );
  QCOMPARE( p7.x(), 11.0 );
  QCOMPARE( p7.y(), 13.0 );
  QCOMPARE( p7.m(), 17.0 );
  QVERIFY( !p7.isEmpty() );
  QVERIFY( p7.is3D() );
  QVERIFY( p7.isMeasure() );
  QCOMPARE( p7.wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( p7.wktTypeStr(), QString( "PointZM" ) );

  QgsPoint noZ( QgsWkbTypes::PointM, 11.0, 13.0, 15.0, 17.0 );
  QCOMPARE( noZ.x(), 11.0 );
  QCOMPARE( noZ.y(), 13.0 );
  QVERIFY( std::isnan( noZ.z() ) );
  QCOMPARE( noZ.m(), 17.0 );
  QCOMPARE( noZ.wkbType(), QgsWkbTypes::PointM );

  QgsPoint noM( QgsWkbTypes::PointZ, 11.0, 13.0, 17.0, 18.0 );
  QCOMPARE( noM.x(), 11.0 );
  QCOMPARE( noM.y(), 13.0 );
  QVERIFY( std::isnan( noM.m() ) );
  QCOMPARE( noM.z(), 17.0 );
  QCOMPARE( noM.wkbType(), QgsWkbTypes::PointZ );

  QgsPoint p8( QgsWkbTypes::Point25D, 21.0, 23.0, 25.0 );
  QCOMPARE( p8.x(), 21.0 );
  QCOMPARE( p8.y(), 23.0 );
  QCOMPARE( p8.z(), 25.0 );
  QVERIFY( !p8.isEmpty() );
  QVERIFY( p8.is3D() );
  QVERIFY( !p8.isMeasure() );
  QCOMPARE( p8.wkbType(), QgsWkbTypes::Point25D );

  QgsPoint pp( QgsWkbTypes::Point );
  QVERIFY( !pp.is3D() );
  QVERIFY( !pp.isMeasure() );

  QgsPoint ppz( QgsWkbTypes::PointZ );
  QVERIFY( ppz.is3D() );
  QVERIFY( !ppz.isMeasure() );

  QgsPoint ppm( QgsWkbTypes::PointM );
  QVERIFY( !ppm.is3D() );
  QVERIFY( ppm.isMeasure() );

  QgsPoint ppzm( QgsWkbTypes::PointZM );
  QVERIFY( ppzm.is3D() );
  QVERIFY( ppzm.isMeasure() );

#if 0 //should trigger an assert
  //try creating a point with a nonsense WKB type
  QgsPoint p9( QgsWkbTypes::PolygonZM, 11.0, 13.0, 9.0, 17.0 );
  QCOMPARE( p9.wkbType(), QgsWkbTypes::Unknown );
#endif

  //test equality operator

  QgsPoint pRight, pLeft;
  QVERIFY( pRight.isEmpty() );
  QVERIFY( pLeft.isEmpty() );
  QVERIFY( pLeft == pRight );
  pRight.setX( 1 );
  pLeft.setY( 1 );
  QVERIFY( pRight.isEmpty() );
  QVERIFY( pLeft.isEmpty() );
  QVERIFY( pLeft != pRight );
  pRight.setY( 1 );
  pLeft.setX( 1 );
  QVERIFY( !pRight.isEmpty() );
  QVERIFY( !pLeft.isEmpty() );
  QVERIFY( pLeft == pRight );


  QVERIFY( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 2 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 1 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 0.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 2 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) );
  QVERIFY( QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) ) );
  //test inequality operator
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) != QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) != QgsPoint( QgsWkbTypes::PointZ, 2 / 3.0, 1 / 3.0 ) );

  QgsLineString nonPoint;
  QVERIFY( p8 != nonPoint );
  QVERIFY( !( p8 == nonPoint ) );

  //test setters and getters
  //x
  QgsPoint p10( QgsWkbTypes::PointZM );
  p10.setX( 5.0 );
  QCOMPARE( p10.x(), 5.0 );
  QCOMPARE( p10.rx(), 5.0 );
  p10.rx() = 9.0;
  QCOMPARE( p10.x(), 9.0 );
  //y
  p10.setY( 7.0 );
  QCOMPARE( p10.y(), 7.0 );
  QCOMPARE( p10.ry(), 7.0 );
  p10.ry() = 3.0;
  QCOMPARE( p10.y(), 3.0 );
  //z
  p10.setZ( 17.0 );
  QCOMPARE( p10.is3D(), true );
  QCOMPARE( p10.z(), 17.0 );
  QCOMPARE( p10.rz(), 17.0 );
  p10.rz() = 13.0;
  QCOMPARE( p10.z(), 13.0 );
  //m
  p10.setM( 27.0 );
  QCOMPARE( p10.m(), 27.0 );
  QCOMPARE( p10.rm(), 27.0 );
  p10.rm() = 23.0;
  QCOMPARE( p10.m(), 23.0 );

  //other checks
  QCOMPARE( p10.geometryType(), QString( "Point" ) );
  QCOMPARE( p10.dimension(), 0 );

  //clone
  std::unique_ptr< QgsPoint >clone( p10.clone() );
  QVERIFY( p10 == *clone );

  //toCurveType
  clone.reset( p10.toCurveType() );
  QVERIFY( p10 == *clone );

  //assignment
  QgsPoint original( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QgsPoint assigned( 6.0, 7.0 );
  assigned = original;
  QVERIFY( assigned == original );

  //clear
  QgsPoint p11( 5.0, 6.0 );
  p11.clear();
  QCOMPARE( p11.wkbType(), QgsWkbTypes::Point );
  QVERIFY( std::isnan( p11.x() ) );
  QVERIFY( std::isnan( p11.y() ) );

  //toQPointF
  QgsPoint p11a( 5.0, 9.0 );
  QPointF result = p11a.toQPointF();
  QGSCOMPARENEAR( result.x(), 5.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( result.y(), 9.0, 4 * std::numeric_limits<double>::epsilon() );

  //to/from WKB
  QgsPoint p12( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QByteArray wkb12 = p12.asWkb();
  QCOMPARE( wkb12.size(), p12.wkbSize() );
  QgsPoint p13;
  QgsConstWkbPtr wkb12ptr( wkb12 );
  p13.fromWkb( wkb12ptr );
  QVERIFY( p13 == p12 );

  //bad WKB - check for no crash
  p13 = QgsPoint( 1, 2 );
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !p13.fromWkb( nullPtr ) );
  QCOMPARE( p13.wkbType(), QgsWkbTypes::Point );
  QgsLineString line;
  p13 = QgsPoint( 1, 2 );
  QByteArray wkbLine = line.asWkb();
  QCOMPARE( wkbLine.size(), line.wkbSize() );
  QgsConstWkbPtr wkbLinePtr( wkbLine );
  QVERIFY( !p13.fromWkb( wkbLinePtr ) );
  QCOMPARE( p13.wkbType(), QgsWkbTypes::Point );

  //to/from WKT
  p13 = QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QString wkt = p13.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsPoint p14;
  QVERIFY( p14.fromWkt( wkt ) );
  QVERIFY( p14 == p13 );

  //bad WKT
  QVERIFY( !p14.fromWkt( "Polygon()" ) );
  QVERIFY( !p14.fromWkt( "Point(1 )" ) );

  //asGML2
  QgsPoint exportPoint( 1, 2 );
  QgsPoint exportPointFloat( 1 / 3.0, 2 / 3.0 );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,2</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPoint.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1 2</pos></Point>" ) );
  QCOMPARE( elemToString( exportPoint.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.333 0.667</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QgsPoint exportPointZ( 1, 2, 3 );
  QString expectedGML3Z( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"3\">1 2 3</pos></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointZ.asGml3( doc, 3 ) ), expectedGML3Z );

  //asGML2 inverted axis
  QgsPoint exportPointInvertedAxis( 1, 2 );
  QgsPoint exportPointFloatInvertedAxis( 1 / 3.0, 2 / 3.0 );
  QDomDocument docInvertedAxis( QStringLiteral( "gml" ) );
  QString expectedGML2InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2,1</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointInvertedAxis.asGml2( docInvertedAxis, 17, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML2InvertedAxis );
  QString expectedGML2prec3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.333</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointFloatInvertedAxis.asGml2( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML2prec3InvertedAxis );

  //asGML3 inverted axis
  QString expectedGML3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">2 1</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointInvertedAxis.asGml3( docInvertedAxis, 17, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3InvertedAxis );
  QString expectedGML3prec3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.667 0.333</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointFloatInvertedAxis.asGml3( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3prec3InvertedAxis );
  QgsPoint exportPointZInvertedAxis( 1, 2, 3 );
  QString expectedGML3ZInvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"3\">2 1 3</pos></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointZInvertedAxis.asGml3( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3ZInvertedAxis );


  //asJSON
  QString expectedJson( QStringLiteral( "{\"coordinates\":[1.0,2.0],\"type\":\"Point\"}" ) );
  QCOMPARE( exportPoint.asJson(), expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[0.333,0.667],\"type\":\"Point\"}" ) );
  QCOMPARE( exportPointFloat.asJson( 3 ), expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<Point><coordinates>1,2</coordinates></Point>" ) );
  QCOMPARE( exportPoint.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<Point><coordinates>0.333,0.667</coordinates></Point>" ) );
  QCOMPARE( exportPointFloat.asKml( 3 ), expectedKmlPrec3 );

  //bounding box
  QgsPoint p15( 1.0, 2.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 1.0, 2.0, 1.0, 2.0 ) );
  //modify points and test that bounding box is updated accordingly
  p15.setX( 3.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 3.0, 2.0, 3.0, 2.0 ) );
  p15.setY( 6.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 3.0, 6.0, 3.0, 6.0 ) );
  p15.rx() = 4.0;
  QCOMPARE( p15.boundingBox(), QgsRectangle( 4.0, 6.0, 4.0, 6.0 ) );
  p15.ry() = 9.0;
  QCOMPARE( p15.boundingBox(), QgsRectangle( 4.0, 9.0, 4.0, 9.0 ) );
  p15.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 11.0, 13.0 ) );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 11.0, 13.0, 11.0, 13.0 ) );
  p15 = QgsPoint( 21.0, 23.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 21.0, 23.0, 21.0, 23.0 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );
  QgsPoint p16( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 );
  p16.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( p16.x(), 175.771, 0.001 );
  QGSCOMPARENEAR( p16.y(), -39.724, 0.001 );
  QGSCOMPARENEAR( p16.z(), 1.0, 0.001 );
  QCOMPARE( p16.m(), 2.0 );
  p16.transform( tr, QgsCoordinateTransform::ReverseTransform );
  QGSCOMPARENEAR( p16.x(), 6374985, 1 );
  QGSCOMPARENEAR( p16.y(), -3626584, 1 );
  QGSCOMPARENEAR( p16.z(), 1.0, 0.001 );
  QCOMPARE( p16.m(), 2.0 );
#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //test with z transform
  p16.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  QGSCOMPARENEAR( p16.z(), -19.249, 0.001 );
  p16.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  QGSCOMPARENEAR( p16.z(), 1.0, 0.001 );
#endif
  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsPoint p17( QgsWkbTypes::PointZM, 10, 20, 30, 40 );
  p17.transform( qtr );
  QVERIFY( p17 == QgsPoint( QgsWkbTypes::PointZM, 20, 60, 30, 40 ) );
  p17.transform( QTransform::fromScale( 1, 1 ), 11, 2, 3, 4 );
  QVERIFY( p17 == QgsPoint( QgsWkbTypes::PointZM, 20, 60, 71, 163 ) );

  //coordinateSequence
  QgsPoint p18( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QgsCoordinateSequence coord = p18.coordinateSequence();
  QCOMPARE( coord.count(), 1 );
  QCOMPARE( coord.at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).at( 0 ), p18 );

  //low level editing
  //insertVertex should have no effect
  QgsPoint p19( QgsWkbTypes::PointZM, 3.0, 4.0, 6.0, 7.0 );
  p19.insertVertex( QgsVertexId( 1, 2, 3 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 6.0, 7.0 ) );

  //moveVertex
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  //invalid vertex id, should not crash
  p19.moveVertex( QgsVertexId( 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  //move PointZM using Point
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 11.0, 12.0, 1.0, 2.0 ) );
  //move PointZM using PointZ
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 21.0, 22.0, 23.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 21.0, 22.0, 23.0, 2.0 ) );
  //move PointZM using PointM
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointM, 31.0, 32.0, 0.0, 43.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 31.0, 32.0, 23.0, 43.0 ) );
  //move Point using PointZM (z/m should be ignored)
  QgsPoint p20( 3.0, 4.0 );
  p20.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( p20, QgsPoint( 2.0, 3.0 ) );

  //deleteVertex - should do nothing, but not crash
  p20.deleteVertex( QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p20, QgsPoint( 2.0, 3.0 ) );

  // closestSegment
  QgsPoint closest;
  QgsVertexId after;
  // return error - points have no segments
  QVERIFY( p20.closestSegment( QgsPoint( 4.0, 6.0 ), closest, after ) < 0 );

  //nextVertex
  QgsPoint p21( 3.0, 4.0 );
  QgsPoint p22;
  QgsVertexId v( 0, 0, -1 );
  QVERIFY( p21.nextVertex( v, p22 ) );
  QCOMPARE( p22, p21 );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  //no more vertices
  QVERIFY( !p21.nextVertex( v, p22 ) );
  v = QgsVertexId( 0, 1, -1 ); //test that ring number is maintained
  QVERIFY( p21.nextVertex( v, p22 ) );
  QCOMPARE( p22, p21 );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );
  v = QgsVertexId( 1, 0, -1 ); //test that part number is maintained
  QVERIFY( p21.nextVertex( v, p22 ) );
  QCOMPARE( p22, p21 );
  QCOMPARE( v, QgsVertexId( 1, 0, 0 ) );

  //adjacent vertices - both should be invalid
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  p21.adjacentVertices( v, prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );

  // vertex iterator
  QgsAbstractGeometry::vertex_iterator it1 = p21.vertices_begin();
  QgsAbstractGeometry::vertex_iterator it1end = p21.vertices_end();
  QCOMPARE( *it1, p21 );
  QCOMPARE( it1.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it1;
  QCOMPARE( it1, it1end );

  // Java-style iterator
  QgsVertexIterator it2( &p21 );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), p21 );
  QVERIFY( !it2.hasNext() );

  //vertexAt - will always be same as point
  QCOMPARE( p21.vertexAt( QgsVertexId() ), p21 );
  QCOMPARE( p21.vertexAt( QgsVertexId( 0, 0, 0 ) ), p21 );

  //vertexAngle - undefined, but check that it doesn't crash
  ( void )p21.vertexAngle( QgsVertexId() );

  //counts
  QCOMPARE( p20.vertexCount(), 1 );
  QCOMPARE( p20.ringCount(), 1 );
  QCOMPARE( p20.partCount(), 1 );

  //measures and other abstract geometry methods
  QCOMPARE( p20.length(), 0.0 );
  QCOMPARE( p20.perimeter(), 0.0 );
  QCOMPARE( p20.area(), 0.0 );
  QCOMPARE( p20.centroid(), p20 );
  QVERIFY( !p20.hasCurvedSegments() );
  std::unique_ptr< QgsPoint >segmented( static_cast< QgsPoint *>( p20.segmentize() ) );
  QCOMPARE( *segmented, p20 );

  //addZValue
  QgsPoint p23( 1.0, 2.0 );
  QVERIFY( p23.addZValue( 5.0 ) );
  QCOMPARE( p23, QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 5.0 ) );
  QVERIFY( !p23.addZValue( 6.0 ) );

  //addMValue
  QgsPoint p24( 1.0, 2.0 );
  QVERIFY( p24.addMValue( 5.0 ) );
  QCOMPARE( p24, QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 5.0 ) );
  QVERIFY( !p24.addMValue( 6.0 ) );

  //dropZ
  QgsPoint p25( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 );
  QVERIFY( p25.dropZValue() );
  QCOMPARE( p25, QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !p25.dropZValue() );
  QgsPoint p26( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( p26.dropZValue() );
  QCOMPARE( p26, QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 4.0 ) );
  QVERIFY( !p26.dropZValue() );
  QgsPoint p26a( QgsWkbTypes::Point25D, 1.0, 2.0, 3.0 );
  QVERIFY( p26a.dropZValue() );
  QCOMPARE( p26a, QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) );
  QVERIFY( !p26a.dropZValue() );

  //dropM
  QgsPoint p27( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 );
  QVERIFY( p27.dropMValue() );
  QCOMPARE( p27, QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !p27.dropMValue() );
  QgsPoint p28( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( p28.dropMValue() );
  QCOMPARE( p28, QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0, 0.0 ) );
  QVERIFY( !p28.dropMValue() );

  //convertTo
  QgsPoint p29( 1.0, 2.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::Point ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::Point );
  QVERIFY( p29.convertTo( QgsWkbTypes::PointZ ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::PointZ );
  p29.setZ( 5.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::Point25D ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::Point25D );
  QCOMPARE( p29.z(), 5.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::PointZM ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( p29.z(), 5.0 );
  p29.setM( 9.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::PointM ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::PointM );
  QVERIFY( std::isnan( p29.z() ) );
  QCOMPARE( p29.m(), 9.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::Point ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::Point );
  QVERIFY( std::isnan( p29.z() ) );
  QVERIFY( std::isnan( p29.m() ) );
  QVERIFY( !p29.convertTo( QgsWkbTypes::Polygon ) );

  //boundary
  QgsPoint p30( 1.0, 2.0 );
  QVERIFY( !p30.boundary() );

  // distance
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 2, 2 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 3, 2 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 3, 2 ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 1, 3 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 1, 4 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 1, 4 ), 2.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distance( QgsPoint( 1, -4 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distance( 1, -4 ), 2.0 );

  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 2, 2 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 3, 2 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 3, 2 ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 1, 3 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 1, 4 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 1, 4 ), 4.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distanceSquared( QgsPoint( 1, -4 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distanceSquared( 1, -4 ), 4.0 );

  // distance 3D
  QCOMPARE( QgsPoint( 0, 0 ).distanceSquared3D( QgsPoint( 1, 1 ) ), 2.0 );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( 1, 1, 0 ) ) );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ) ) ) );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( 2, 2, 2 ) ) );
  QVERIFY( std::isnan( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( 1, 1, 0 ), 6.0 );
  QVERIFY( std::isnan( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( QgsPoint( 0, 0 ) ) ) );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( 0, 0, 0 ), 12.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ) ), 48.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( 2, 2, 2 ), 48.0 );


  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 3, 2, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( 1, 3, 2 ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 4, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( 1, 1, 4 ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -4, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -2, 0 ).distance3D( 1, 1, -4 ), 2.0 );

  // azimuth
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 1, 2 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 1, 3 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 2, 2 ) ), 90.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 1, 0 ) ), 180.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 0, 2 ) ), -90.0 );

  // operators
  QgsPoint p31( 1, 2 );
  QgsPoint p32( 3, 5 );
  QCOMPARE( p32 - p31, QgsVector( 2, 3 ) );
  QCOMPARE( p31 - p32, QgsVector( -2, -3 ) );

  p31 = QgsPoint( 1, 2 );
  QCOMPARE( p31 + QgsVector( 3, 5 ), QgsPoint( 4, 7 ) );
  p31 += QgsVector( 3, 5 );
  QCOMPARE( p31, QgsPoint( 4, 7 ) );

  QCOMPARE( p31 - QgsVector( 3, 5 ), QgsPoint( 1, 2 ) );
  p31 -= QgsVector( 3, 5 );
  QCOMPARE( p31, QgsPoint( 1, 2 ) );

  // test projecting a point
  // 2D
  QgsPoint p33 = QgsPoint( 1, 2 );
  QCOMPARE( p33.project( 1, 0 ), QgsPoint( 1, 3 ) );
  QCOMPARE( p33.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );
  QCOMPARE( p33.project( 1.5, 90 ), QgsPoint( 2.5, 2 ) );
  QCOMPARE( p33.project( 1.5, 90, 90 ), QgsPoint( 2.5, 2 ) ); // stay QgsWkbTypes::Point
  QCOMPARE( p33.project( 2, 180 ), QgsPoint( 1, 0 ) );
  QCOMPARE( p33.project( 5, 270 ), QgsPoint( -4, 2 ) );
  QCOMPARE( p33.project( 6, 360 ), QgsPoint( 1, 8 ) );
  QCOMPARE( p33.project( 5, 450 ), QgsPoint( 6, 2 ) );
  QCOMPARE( p33.project( 5, 450, 450 ), QgsPoint( 6, 2 ) );  // stay QgsWkbTypes::Point
  QCOMPARE( p33.project( -1, 0 ), QgsPoint( 1, 1 ) );
  QCOMPARE( p33.project( 1.5, -90 ), QgsPoint( -0.5, 2 ) );
  p33.addZValue( 0 );
  QCOMPARE( p33.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 1 ) );
  QCOMPARE( p33.project( 2, 180, 180 ), QgsPoint( QgsWkbTypes::PointZ,  1, 2, -2 ) );
  QCOMPARE( p33.project( 5, 270, 270 ), QgsPoint( QgsWkbTypes::PointZ,  6, 2, 0 ) );
  QCOMPARE( p33.project( 6, 360, 360 ), QgsPoint( QgsWkbTypes::PointZ,  1, 2, 6 ) );
  QCOMPARE( p33.project( -1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, -1 ) );
  QCOMPARE( p33.project( 1.5, -90, -90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 0 ) );

  // PointM
  p33.dropZValue();
  p33.addMValue( 5.0 );
  QCOMPARE( p33.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 3, 0, 5 ) );
  QCOMPARE( p33.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointM, 6, 2, 0, 5 ) );

  p33.addZValue( 0 );
  QCOMPARE( p33.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 1, 5 ) );

  // 3D
  QgsPoint p34 = QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 );
  QCOMPARE( p34.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 3, 2 ) );
  QCOMPARE( p34.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p34.project( 1.5, 90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  QCOMPARE( p34.project( 1.5, 90, 90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  QCOMPARE( p34.project( 2, 180 ), QgsPoint( QgsWkbTypes::PointZ, 1, 0, 2 ) );
  QCOMPARE( p34.project( 2, 180, 180 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 ) );
  QCOMPARE( p34.project( 5, 270 ), QgsPoint( QgsWkbTypes::PointZ, -4, 2, 2 ) );
  QCOMPARE( p34.project( 5, 270, 270 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( p34.project( 6, 360 ), QgsPoint( QgsWkbTypes::PointZ, 1, 8, 2 ) );
  QCOMPARE( p34.project( 6, 360, 360 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 8 ) );
  QCOMPARE( p34.project( 5, 450 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( p34.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( p34.project( -1, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2 ) );
  QCOMPARE( p34.project( -1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 1 ) );
  QCOMPARE( p34.project( 1.5, -90 ), QgsPoint( QgsWkbTypes::PointZ, -0.5, 2, 2 ) );
  QCOMPARE( p34.project( 1.5, -90, -90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  // PointM
  p34.addMValue( 5.0 );
  QCOMPARE( p34.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 3, 2, 5 ) );
  QCOMPARE( p34.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( p34.project( 5, 450 ), QgsPoint( QgsWkbTypes::PointZM, 6, 2, 2, 5 ) );
  QCOMPARE( p34.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointZM, 6, 2, 2, 5 ) );

  // inclination
  QCOMPARE( QgsPoint( 1, 2 ).inclination( QgsPoint( 1, 2 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 90 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, -90 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 0 ) ), 0.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 180 ) ), 180.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, -180 ) ), 180.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 720 ) ), 0.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 45 ) ), 45.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 135 ) ), 135.0 );

  // vertex number
  QCOMPARE( QgsPoint( 1, 2 ).vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( QgsPoint( 1, 2 ).vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  QCOMPARE( QgsPoint( 1, 2 ).vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );

  //segmentLength
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( -1, 0, 1 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );

  // remove duplicate points
  QgsPoint p = QgsPoint( 1, 2 );
  QVERIFY( !p.removeDuplicateNodes() );
  QCOMPARE( p.x(), 1.0 );
  QCOMPARE( p.y(), 2.0 );

  // swap xy
  p = QgsPoint( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  p.swapXy();
  QCOMPARE( p.x(), 2.2 );
  QCOMPARE( p.y(), 1.1 );
  QCOMPARE( p.z(), 3.3 );
  QCOMPARE( p.m(), 4.4 );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );

  // filter vertex
  p = QgsPoint( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  p.filterVertices( []( const QgsPoint & )-> bool { return false; } );
  QCOMPARE( p.x(), 1.1 );
  QCOMPARE( p.y(), 2.2 );
  QCOMPARE( p.z(), 3.3 );
  QCOMPARE( p.m(), 4.4 );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );

  // transform vertex
  p = QgsPoint( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  p.transformVertices( []( const QgsPoint & p )-> QgsPoint
  {
    return QgsPoint( p.x() + 2, p.y() + 3, p.z() + 1, p.m() + 8 );
  } );
  QCOMPARE( p.x(), 3.1 );
  QCOMPARE( p.y(), 5.2 );
  QCOMPARE( p.z(), 4.3 );
  QCOMPARE( p.m(), 12.4 );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );
  // no dimensionality change allowed
  p.transformVertices( []( const QgsPoint & p )-> QgsPoint
  {
    return QgsPoint( p.x() + 2, p.y() + 3 );
  } );
  QCOMPARE( p.x(), 5.1 );
  QCOMPARE( p.y(), 8.2 );
  QVERIFY( std::isnan( p.z() ) );
  QVERIFY( std::isnan( p.m() ) );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );
  p = QgsPoint( 2, 3 );
  p.transformVertices( []( const QgsPoint & p )-> QgsPoint
  {
    return QgsPoint( p.x() + 2, p.y() + 3, 7, 8 );
  } );
  QCOMPARE( p.x(), 4.0 );
  QCOMPARE( p.y(), 6.0 );
  QVERIFY( std::isnan( p.z() ) );
  QVERIFY( std::isnan( p.m() ) );
  QCOMPARE( p.wkbType(), QgsWkbTypes::Point );

  // transform using class
  p = QgsPoint( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  TestTransformer transformer;
  QVERIFY( p.transform( &transformer ) );
  QCOMPARE( p.x(), 3.3 );
  QCOMPARE( p.y(), 16.2 );
  QCOMPARE( p.z(), 8.3 );
  QCOMPARE( p.m(), 3.4 );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );

  TestFailTransformer failTransformer;
  QVERIFY( !p.transform( &failTransformer ) );

  // test bounding box intersects
  QVERIFY( QgsPoint( 1, 2 ).boundingBoxIntersects( QgsRectangle( 0, 0.5, 1.5, 3 ) ) );
  QVERIFY( !QgsPoint( 1, 2 ).boundingBoxIntersects( QgsRectangle( 3, 0.5, 3.5, 3 ) ) );
  QVERIFY( !QgsPoint().boundingBoxIntersects( QgsRectangle( 0, 0.5, 3.5, 3 ) ) );
}

QGSTEST_MAIN( TestQgsPoint )
#include "testqgspoint.moc"




