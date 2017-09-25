/***************************************************************************
                         testqgscoordinatetransform.cpp
                         -----------------------
    begin                : October 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscoordinatetransform.h"
#include "qgsapplication.h"
#include "qgsrectangle.h"
#include <QObject>
#include "qgstest.h"

class TestQgsCoordinateTransform: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void transformBoundingBox();
    void copy();
    void assignment();
    void isValid();
    void isShortCircuited();

  private:

};


void TestQgsCoordinateTransform::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsCoordinateTransform::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCoordinateTransform::copy()
{
  QgsCoordinateTransform uninitialized;
  QgsCoordinateTransform uninitializedCopy( uninitialized );
  QVERIFY( !uninitializedCopy.isValid() );

  QgsCoordinateReferenceSystem source;
  source.createFromId( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsCoordinateReferenceSystem destination;
  destination.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );

  QgsCoordinateTransform original( source, destination );
  QVERIFY( original.isValid() );

  QgsCoordinateTransform copy( original );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.sourceCrs().authid(), original.sourceCrs().authid() );
  QCOMPARE( copy.destinationCrs().authid(), original.destinationCrs().authid() );

  // force detachement of copy
  QgsCoordinateReferenceSystem newDest;
  newDest.createFromId( 3857, QgsCoordinateReferenceSystem::EpsgCrsId );
  copy.setDestinationCrs( newDest );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.destinationCrs().authid(), QString( "EPSG:3857" ) );
  QCOMPARE( original.destinationCrs().authid(), QString( "EPSG:4326" ) );
}

void TestQgsCoordinateTransform::assignment()
{
  QgsCoordinateTransform uninitialized;
  QgsCoordinateTransform uninitializedCopy;
  uninitializedCopy = uninitialized;
  QVERIFY( !uninitializedCopy.isValid() );

  QgsCoordinateReferenceSystem source;
  source.createFromId( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsCoordinateReferenceSystem destination;
  destination.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );

  QgsCoordinateTransform original( source, destination );
  QVERIFY( original.isValid() );

  QgsCoordinateTransform copy;
  copy = original;
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.sourceCrs().authid(), original.sourceCrs().authid() );
  QCOMPARE( copy.destinationCrs().authid(), original.destinationCrs().authid() );

  // force detachement of copy
  QgsCoordinateReferenceSystem newDest;
  newDest.createFromId( 3857, QgsCoordinateReferenceSystem::EpsgCrsId );
  copy.setDestinationCrs( newDest );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.destinationCrs().authid(), QString( "EPSG:3857" ) );
  QCOMPARE( original.destinationCrs().authid(), QString( "EPSG:4326" ) );

  // test assigning back to invalid
  copy = uninitialized;
  QVERIFY( !copy.isValid() );
  QVERIFY( original.isValid() );
}

void TestQgsCoordinateTransform::isValid()
{
  QgsCoordinateTransform tr;
  QVERIFY( !tr.isValid() );

  QgsCoordinateReferenceSystem srs1;
  srs1.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem srs2;
  srs2.createFromSrid( 4326 );

  // valid source, invalid destination
  QgsCoordinateTransform tr2( srs1, QgsCoordinateReferenceSystem() );
  QVERIFY( !tr2.isValid() );

  // invalid source, valid destination
  QgsCoordinateTransform tr3( QgsCoordinateReferenceSystem(), srs2 );
  QVERIFY( !tr3.isValid() );

  // valid source, valid destination
  QgsCoordinateTransform tr4( srs1, srs2 );
  QVERIFY( tr4.isValid() );

  // try to invalidate by setting source as invalid
  tr4.setSourceCrs( QgsCoordinateReferenceSystem() );
  QVERIFY( !tr4.isValid() );

  QgsCoordinateTransform tr5( srs1, srs2 );
  // try to invalidate by setting destination as invalid
  tr5.setDestinationCrs( QgsCoordinateReferenceSystem() );
  QVERIFY( !tr5.isValid() );
}

void TestQgsCoordinateTransform::isShortCircuited()
{
  QgsCoordinateTransform tr;
  //invalid transform shortcircuits
  QVERIFY( tr.isShortCircuited() );

  QgsCoordinateReferenceSystem srs1;
  srs1.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem srs2;
  srs2.createFromSrid( 4326 );

  // valid source, invalid destination
  QgsCoordinateTransform tr2( srs1, QgsCoordinateReferenceSystem() );
  QVERIFY( tr2.isShortCircuited() );

  // invalid source, valid destination
  QgsCoordinateTransform tr3( QgsCoordinateReferenceSystem(), srs2 );
  QVERIFY( tr3.isShortCircuited() );

  // equal, valid source and destination
  QgsCoordinateTransform tr4( srs1, srs1 );
  QVERIFY( tr4.isShortCircuited() );

  // valid but different source and destination
  QgsCoordinateTransform tr5( srs1, srs2 );
  QVERIFY( !tr5.isShortCircuited() );

  // try to short circuit by changing dest
  tr5.setDestinationCrs( srs1 );
  QVERIFY( tr5.isShortCircuited() );
}


void TestQgsCoordinateTransform::transformBoundingBox()
{
  //test transforming a bounding box which crosses the 180 degree longitude line
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4326 );

  QgsCoordinateTransform tr( sourceSrs, destSrs );
  QgsRectangle crossingRect( 6374985, -3626584, 7021195, -3272435 );
  QgsRectangle resultRect = tr.transformBoundingBox( crossingRect, QgsCoordinateTransform::ForwardTransform, true );
  QgsRectangle expectedRect;
  expectedRect.setXMinimum( 175.771 );
  expectedRect.setYMinimum( -39.7222 );
  expectedRect.setXMaximum( -176.549 );
  expectedRect.setYMaximum( -36.3951 );

  qDebug( "BBox transform x min: %.17f", resultRect.xMinimum() );
  qDebug( "BBox transform x max: %.17f", resultRect.xMaximum() );
  qDebug( "BBox transform y min: %.17f", resultRect.yMinimum() );
  qDebug( "BBox transform y max: %.17f", resultRect.yMaximum() );

  QGSCOMPARENEAR( resultRect.xMinimum(), expectedRect.xMinimum(), 0.001 );
  QGSCOMPARENEAR( resultRect.yMinimum(), expectedRect.yMinimum(), 0.001 );
  QGSCOMPARENEAR( resultRect.xMaximum(), expectedRect.xMaximum(), 0.001 );
  QGSCOMPARENEAR( resultRect.yMaximum(), expectedRect.yMaximum(), 0.001 );
}

QGSTEST_MAIN( TestQgsCoordinateTransform )
#include "testqgscoordinatetransform.moc"
