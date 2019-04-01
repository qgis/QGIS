/***************************************************************************
     testqgsprojutils.cpp
     --------------------------------------
    Date                 : March 2019
    Copyright            : (C) 2019 Nyall Dawson
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
#include <QPixmap>

#include "qgsapplication.h"
#include "qgslogger.h"

//header for class being tested
#include "qgsprojutils.h"
#include <QtConcurrent>

class TestQgsProjUtils: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void threadSafeContext();
    void usesAngularUnits();

};


void TestQgsProjUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::createDatabase();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsProjUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


struct ProjContextWrapper
{
  explicit ProjContextWrapper()
  {}

  void operator()( int )
  {
    QVERIFY( QgsProjContext::get() );
    // TODO - do something with the context?
  }
};

void TestQgsProjUtils::threadSafeContext()
{
  // smash proj context generation over many threads
  QVector< int > list;
  list.resize( 100 );
  QtConcurrent::blockingMap( list, ProjContextWrapper() );
}

void TestQgsProjUtils::usesAngularUnits()
{
#if PROJ_VERSION_MAJOR>=6
  QVERIFY( !QgsProjUtils::usesAngularUnit( QString() ) );
  QVERIFY( !QgsProjUtils::usesAngularUnit( QString( "" ) ) );
  QVERIFY( !QgsProjUtils::usesAngularUnit( QStringLiteral( "x" ) ) );
  QVERIFY( QgsProjUtils::usesAngularUnit( QStringLiteral( "+proj=longlat +ellps=WGS60 +no_defs" ) ) );
  QVERIFY( !QgsProjUtils::usesAngularUnit( QStringLiteral( "+proj=tmerc +lat_0=0 +lon_0=147 +k_0=0.9996 +x_0=500000 +y_0=10000000 +ellps=GRS80 +units=m +no_defs" ) ) );
#endif
}

QGSTEST_MAIN( TestQgsProjUtils )
#include "testqgsprojutils.moc"
