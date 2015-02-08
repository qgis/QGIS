/***************************************************************************
     testqgsmapsettings.cpp
     --------------------------------------
    Date                 : Tue  6 Feb 2015
    Copyright            : (C) 2014 by Sandro Santilli
    Email                : strk@keybit.net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QObject>
//header for class being tested
#include <qgsrectangle.h>
#include <qgsmapsettings.h>
#include <qgspoint.h>
#include <math.h>
#include "qgslogger.h"

class TestQgsMapSettings: public QObject
{
    Q_OBJECT
  private slots:
    void visibleExtent();
    void mapUnitsPerPixel();
    void visiblePolygon();
  private:
    QString toString( const QPolygonF& p, int decimalPlaces = 2 ) const;
};

QString TestQgsMapSettings::toString( const QPolygonF& p, int dec ) const
{
  QString s;
  const char *sep = "";
  double r = pow( 10.0, dec );
  for ( int i = 0; i < p.size(); ++i )
  {
    s += QString( "%1%2 %3" ).arg( sep )
         .arg( int( p[i].x() * r ) / r )
         .arg( int( p[i].y() * r ) / r );
    sep = ",";
  }

  return s;
}

void TestQgsMapSettings::visibleExtent()
{
  QgsMapSettings ms;

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,0 : 150,100" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 100 ) );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "0,0 : 100,100" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 50, 100 ) );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "0,-50 : 100,150" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 50, 100 ) );
  ms.setRotation( 90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,0 : 150,100" ) );
  ms.setRotation( -90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,0 : 150,100" ) );

  ms.setExtent( QgsRectangle( 0, 0, 100, 50 ) );
  ms.setOutputSize( QSize( 50, 100 ) );
  ms.setRotation( 0 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "0,-75 : 100,125" ) );
  ms.setRotation( 90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,-25 : 150,75" ) );
  ms.setRotation( -90 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-50,-25 : 150,75" ) );
  ms.setRotation( 45 );
  QCOMPARE( ms.visibleExtent().toString( 0 ), QString( "-56,-81 : 156,131" ) );
}

void TestQgsMapSettings::mapUnitsPerPixel()
{
  QgsMapSettings ms;
  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );

  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 2.0 );

  ms.setOutputSize( QSize( 100, 100 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 1.0 );

  ms.setOutputSize( QSize( 50, 100 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 2.0 );

  ms.setOutputSize( QSize( 5000, 1000 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 0.1 );

  ms.setOutputSize( QSize( 1000, 500 ) );
  QCOMPARE( ms.mapUnitsPerPixel(), 0.2 );
}

void TestQgsMapSettings::visiblePolygon()
{
  QgsMapSettings ms;

  ms.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "-50 100,150 100,150 0,-50 0" ) );

  ms.setExtent( QgsRectangle( 0, -50, 100, 0 ) );
  ms.setOutputSize( QSize( 100, 50 ) );
  ms.setRotation( 90 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "25 -75,25 25,75 25,75 -75" ) );
  ms.setRotation( -90 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "75 25,75 -75,25 -75,25 25" ) );
  ms.setRotation( 30 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "-5.8 -28.34,80.8 21.65,105.8 -21.65,19.19 -71.65" ) );
  ms.setRotation( -30 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "19.19 21.65,105.8 -28.34,80.8 -71.65,-5.8 -21.65" ) );
  ms.setRotation( 45 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "-3.03 -42.67,67.67 28.03,103.03 -7.32,32.32 -78.03" ) );
  ms.setRotation( -45 );
  QCOMPARE( toString( ms.visiblePolygon() ),
            QString( "32.32 28.03,103.03 -42.67,67.67 -78.03,-3.03 -7.32" ) );
}

QTEST_MAIN( TestQgsMapSettings )
#include "testqgsmapsettings.moc"
