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
#include <QObject>
#include <QtTest/QtTest>

class TestQgsCoordinateTransform: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void transformBoundingBox();

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

  QVERIFY( qgsDoubleNear( resultRect.xMinimum(), expectedRect.xMinimum(), 0.001 ) );
  QVERIFY( qgsDoubleNear( resultRect.yMinimum(), expectedRect.yMinimum(), 0.001 ) );
  QVERIFY( qgsDoubleNear( resultRect.xMaximum(), expectedRect.xMaximum(), 0.001 ) );
  QVERIFY( qgsDoubleNear( resultRect.yMaximum(), expectedRect.yMaximum(), 0.001 ) );
}

QTEST_MAIN( TestQgsCoordinateTransform )
#include "testqgscoordinatetransform.moc"
