
/***************************************************************************
     testqgsogcutils.cpp
     --------------------------------------
    Date                 : March 2013
    Copyright            : (C) 2013 Martin Dobias
    Email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest>

//qgis includes...
#include <qgsgeometry.h>
#include <qgsogcutils.h>


/** \ingroup UnitTests
 * This is a unit test for OGC utilities
 */
class TestQgsOgcUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testGeometryFromGML();
    void testGeometryToGML();
};


void TestQgsOgcUtils::testGeometryFromGML()
{
  QgsGeometry* geom = QgsOgcUtils::geometryFromGML2( "<Point><coordinates>123,456</coordinates></Point>" );
  QVERIFY( geom );
  QVERIFY( geom->wkbType() == QGis::WKBPoint );
  QVERIFY( geom->asPoint() == QgsPoint( 123, 456 ) );

  QgsGeometry* geomBox = QgsOgcUtils::geometryFromGML2( "<gml:Box srsName=\"foo\"><gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>" );
  QVERIFY( geomBox );
  QVERIFY( geomBox->wkbType() == QGis::WKBPolygon );

  geom = QgsOgcUtils::geometryFromGML3( "<Point><pos>123 456</pos></Point>" );
  QVERIFY( geom );
  QVERIFY( geom->wkbType() == QGis::WKBPoint );
  QVERIFY( geom->asPoint() == QgsPoint( 123, 456 ) );

  geomBox = QgsOgcUtils::geometryFromGML3( "<gml:Envelope srsName=\"foo\"><gml:lowerCorner>135.2239 34.4879</gml:lowerCorner><gml:upperCorner>135.8578 34.8471</gml:upperCorner></gml:Envelope>" );
  QVERIFY( geomBox );
  QVERIFY( geomBox->wkbType() == QGis::WKBPolygon );

  delete geom;
  delete geomBox;
}

void TestQgsOgcUtils::testGeometryToGML()
{
  QDomDocument doc;

  QDomElement elemInvalid = QgsOgcUtils::geometryToGML2( 0, doc );
  QVERIFY( elemInvalid.isNull() );

  QgsGeometry* geomPoint = QgsGeometry::fromPoint( QgsPoint( 111, 222 ) );
  QDomElement elemPoint = QgsOgcUtils::geometryToGML2( geomPoint, doc );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:coordinates cs=\",\" ts=\" \">111.0,222.0</gml:coordinates></gml:Point>" ) );
  doc.removeChild( elemPoint );

  elemInvalid = QgsOgcUtils::geometryToGML3( 0, doc );
  QVERIFY( elemInvalid.isNull() );

  elemPoint = QgsOgcUtils::geometryToGML3( geomPoint, doc );
  QVERIFY( !elemPoint.isNull() );

  doc.appendChild( elemPoint );
  QCOMPARE( doc.toString( -1 ), QString( "<gml:Point><gml:pos srsDimension=\"2\">111.0 222.0</gml:pos></gml:Point>" ) );
  doc.removeChild( elemPoint );

  delete geomPoint;
}


QTEST_MAIN( TestQgsOgcUtils )
#include "moc_testqgsogcutils.cxx"
