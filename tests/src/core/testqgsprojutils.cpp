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

QGSTEST_MAIN( TestQgsProjUtils )
#include "testqgsprojutils.moc"
