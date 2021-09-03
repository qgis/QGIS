/***************************************************************************
  testqgsinterpolator.cpp
  -----------------------
Date                 : August 2020
Copyright            : (C) 2020 by Vincent Cloarec
Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsdualedgetriangulation.h"
#include "qgsmeshtriangulation.h"
#include "qgsvectorlayer.h"

#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

class TestQgsTriangulation : public QObject
{
    Q_OBJECT

  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.
    void dualEdge();

    void meshTriangulation();
    void meshTriangulationWithOnlyBreakLine();
    void meshTriangulationPointAndBreakLineBreakLine();

  private:
};

void  TestQgsTriangulation::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsTriangulation::cleanupTestCase()
{}

void TestQgsTriangulation::init()
{}

void TestQgsTriangulation::cleanup()
{}

void TestQgsTriangulation::dualEdge()
{
  //3 points
  QgsDualEdgeTriangulation triangulation;
  // Add colinear points
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  QgsMesh mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 2 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 3 );
  QCOMPARE( mesh.faceCount(), 1 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 2, 1} ) ) );

  //4 points
  triangulation = QgsDualEdgeTriangulation();
  // Add colinear points
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 2 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  triangulation.addPoint( QgsPoint( 2, 3, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 2, 1} ) ) );

  //3 first colinear points
  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 3 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {3, 2, 1} ) ) );
  triangulation.addPoint( QgsPoint( 2, 3, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 5 );
  QCOMPARE( mesh.faceCount(), 3 );

  //3 first colinear points with different order
  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 2, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 3 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 2} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {1, 2, 3} ) ) );
  triangulation.addPoint( QgsPoint( 2, 3, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 5 );
  QCOMPARE( mesh.faceCount(), 3 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 2} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {1, 3, 4} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 2 ), QgsMeshFace( {1, 2, 3} ) ) );

  //4 first colinear points
  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 2, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 3, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 4, 1, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 1, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 5 );
  QCOMPARE( mesh.faceCount(), 3 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 6 );
  QCOMPARE( mesh.faceCount(), 4 );

  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 2, 0, 1 ) );
  triangulation.addPoint( QgsPoint( 0, 2, 1 ) );
  triangulation.addPoint( QgsPoint( 2, 4, 1 ) );
  triangulation.addPoint( QgsPoint( 4, 2, 1 ) );

  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.faceCount(), 2 );
  QCOMPARE( mesh.vertexCount(), 4 );

  //add point exactly on existing edge
  triangulation.addPoint( QgsPoint( 2, 2, 1 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.faceCount(), 4 );
  QCOMPARE( mesh.vertexCount(), 5 );
}

void TestQgsTriangulation::meshTriangulation()
{
  QgsMeshTriangulation meshTri;

  meshTri.setCrs( QgsCoordinateReferenceSystem( "EPSG:32620" ) );

  QgsVectorLayer *mLayerPointZ = new QgsVectorLayer( QStringLiteral( "PointZ?crs=EPSG:32620" ),
      QStringLiteral( "point Z" ),
      QStringLiteral( "memory" ) );

  const QString wkt1 = "PointZ (684486.0 1761297.0 1)";
  const QString wkt2 = "PointZ (684276.0 1761309.0 2)";
  const QString wkt3 = "PointZ (684098.0 1761401.0 3)";
  const QString wkt4 = "PointZ (684292.0 1761406.0 4)";

  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );
  QgsFeature f3;
  f3.setGeometry( QgsGeometry::fromWkt( wkt3 ) );
  QgsFeature f4;
  f4.setGeometry( QgsGeometry::fromWkt( wkt4 ) );

  QgsFeatureList flist;
  flist << f1 << f2 << f3 << f4;
  mLayerPointZ->dataProvider()->addFeatures( flist );

  const QgsCoordinateTransformContext transformContext;
  QgsCoordinateTransform transform( mLayerPointZ->crs(),
                                    QgsCoordinateReferenceSystem( "EPSG:32620" ),
                                    transformContext );

  QgsFeatureIterator fIt = mLayerPointZ->getFeatures();
  meshTri.addVertices( fIt, -1, transform );

  QgsMesh mesh = meshTri.triangulatedMesh();

  QCOMPARE( mesh.vertexCount(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );

  QCOMPARE( mesh.vertex( 0 ), QgsMeshVertex( 684486.0, 1761297.0, 1 ) );
  QCOMPARE( mesh.vertex( 1 ), QgsMeshVertex( 684276.0, 1761309.0, 2 ) );
  QCOMPARE( mesh.vertex( 2 ), QgsMeshVertex( 684098.0, 1761401.0, 3 ) );
  QCOMPARE( mesh.vertex( 3 ), QgsMeshVertex( 684292.0, 1761406.0, 4 ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {1, 3, 2} ) ) );

  const QString wkt5 = "LineStringZ (684098.0 1761401.0 3,684210.24 1761347.92 7,684343.8 1761373.4 8,684486.0 1761297.0 1)";

  QgsVectorLayer *mLayerBreakLine = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:32620" ),
      QStringLiteral( "line" ),
      QStringLiteral( "memory" ) );

  QgsFeature f5;
  f5.setGeometry( QgsGeometry::fromWkt( wkt5 ) );
  mLayerBreakLine->dataProvider()->addFeature( f5 );

  transform = QgsCoordinateTransform( mLayerBreakLine->crs(),
                                      QgsCoordinateReferenceSystem( "EPSG:32620" ),
                                      transformContext );
  fIt = mLayerBreakLine->getFeatures();
  meshTri.addBreakLines( fIt, -1, transform );

  mesh = meshTri.triangulatedMesh();

  QCOMPARE( mesh.vertexCount(), 6 );
  QCOMPARE( mesh.faceCount(), 6 );

  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 5} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {0, 5, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 2 ), QgsMeshFace( {1, 4, 2} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 3 ), QgsMeshFace( {3, 4, 5} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 4 ), QgsMeshFace( {1, 5, 4} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 5 ), QgsMeshFace( {2, 4, 3} ) ) );
}

void TestQgsTriangulation::meshTriangulationWithOnlyBreakLine()
{
  QgsMeshTriangulation meshTri;

  QgsVectorLayer *mLayerLineZ = new QgsVectorLayer( QStringLiteral( "LineStringZ" ),
      QStringLiteral( "break line Z" ),
      QStringLiteral( "memory" ) );

  QStringList wktLines;

  wktLines << QStringLiteral( "LineStringZ (315377.05605000001378357 5839566.94189499784260988 24.94718200000000152, 315374.77223399997455999 5839565.11973000038415194 24.04360499999999945)" )
           << QStringLiteral( "LineStringZ (315369.53268400009255856 5839567.42751600034534931 25.41215299999999999, 315369.31927300000097603 5839570.36336500104516745 25.47851700000000008)" )
           << QStringLiteral( "LineStringZ (315369.31927300000097603 5839570.36336500104516745 25.47851700000000008, 315377.62744900002144277 5839568.60983499884605408 24.98952099999999987)" )
           << QStringLiteral( "LineStringZ (315369.53268400009255856 5839567.42751600034534931 25.41215299999999999, 315377.05605000001378357 5839566.94189499784260988 24.94718200000000152)" )
           << QStringLiteral( "LineStringZ (315374.77223399997455999 5839565.11973000038415194 24.04360499999999945, 315370.67597402411047369 5839565.22503056097775698 24.04360499999999945)" )
           << QStringLiteral( "LineStringZ (315370.67597402411047369 5839565.22503056097775698 24.04360499999999945, 315369.53268400009255856 5839567.42751600034534931 25.41215299999999999)" )
           << QStringLiteral( "LineStringZ (315369.31927300000097603 5839570.36336500104516745 25.47851700000000008, 315371.93385799997486174 5839571.38528699986636639 24.06699300000000008)" )
           << QStringLiteral( "LineStringZ (315371.93385799997486174 5839571.38528699986636639 24.06699300000000008, 315376.77400700020371005 5839570.69979299977421761 24.0794150000000009)" )
           << QStringLiteral( "LineStringZ (315376.77400700020371005 5839570.69979299977421761 24.0794150000000009, 315377.62744900002144277 5839568.60983499884605408 24.98952099999999987)" )
           << QStringLiteral( "LineStringZ (315377.62744900002144277 5839568.60983499884605408 24.98952099999999987, 315377.05605000001378357 5839566.94189499784260988 24.94718200000000152)" );

  QgsFeatureList flist;
  for ( const QString &wkt : wktLines )
  {
    QgsFeature feat;
    feat.setGeometry( QgsGeometry::fromWkt( wkt ) );
    flist << feat;
  }

  mLayerLineZ->dataProvider()->addFeatures( flist );

  const QgsCoordinateTransformContext transformContext;
  const QgsCoordinateTransform transform( mLayerLineZ->crs(),
                                          QgsCoordinateReferenceSystem(),
                                          transformContext );

  QgsFeatureIterator fIt = mLayerLineZ->getFeatures();
  meshTri.addBreakLines( fIt, -1, transform );

  const QgsMesh mesh = meshTri.triangulatedMesh();

  QCOMPARE( mesh.vertexCount(), 8 );
  QCOMPARE( mesh.faceCount(), 6 );

  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {2, 0, 4} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {0, 2, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 2 ), QgsMeshFace( {1, 2, 5} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 3 ), QgsMeshFace( {2, 4, 3} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 4 ), QgsMeshFace( {4, 6, 3} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 5 ), QgsMeshFace( {4, 7, 6} ) ) );
}

void TestQgsTriangulation::meshTriangulationPointAndBreakLineBreakLine()
{
  QgsMeshTriangulation meshTri;

  QgsVectorLayer *mLayerPointsZ = new QgsVectorLayer( QStringLiteral( "PointZ" ),
      QStringLiteral( "points Z" ),
      QStringLiteral( "memory" ) );

  for ( int i = 0; i < 4; ++i )
  {
    for ( int j = 0 ; j < 10; ++j )
    {
      QgsFeature feat;
      feat.setGeometry( QgsGeometry( new QgsPoint( i * 10.0, j * 10.0 ) ) );
      mLayerPointsZ->dataProvider()->addFeature( feat );
    }
  }

  const QgsCoordinateTransformContext transformContext;
  const QgsCoordinateTransform transform( mLayerPointsZ->crs(),
                                          QgsCoordinateReferenceSystem(),
                                          transformContext );

  QgsFeatureIterator fIt = mLayerPointsZ->getFeatures();
  meshTri.addVertices( fIt, -1, transform );

  QgsMesh mesh = meshTri.triangulatedMesh();

  QCOMPARE( mesh.vertexCount(), 40 );
  QCOMPARE( mesh.faceCount(), 54 );

  QgsVectorLayer *mLayerLineZ = new QgsVectorLayer( QStringLiteral( "LineStringZ" ),
      QStringLiteral( "break line Z" ),
      QStringLiteral( "memory" ) );


  QgsFeature feat;
  feat.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineStringZ (5 25 1, 95 25 2, 95 15 3, 5 15 4)" ) ) );
  mLayerLineZ->dataProvider()->addFeature( feat );
  fIt = mLayerLineZ->getFeatures();
  meshTri.addBreakLines( fIt, -1, transform );

  mesh = meshTri.triangulatedMesh();

  QCOMPARE( mesh.vertexCount(), 44 );
  QCOMPARE( mesh.faceCount(), 68 );
}

QGSTEST_MAIN( TestQgsTriangulation )
#include "testqgstriangulation.moc"
