/***************************************************************************
     testqgshstoreutils.cpp
     --------------------------------------
    Date                 : October 2018
    Copyright            : (C) 2018 Etienne Trimaille
    Email                : etienne dot trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshstoreutils.h"
#include "qgstest.h"

#include <QString>

class TestQgsHstoreUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testHstore();
};


void TestQgsHstoreUtils::testHstore()
{
  QVariantMap map;
  map[u"1"_s] = "one";
  map[u"2"_s] = "two";
  map[u"3"_s] = "three";

  QCOMPARE( QgsHstoreUtils::parse( u"1=>one,2=>two,3=>three"_s ), map );
  QCOMPARE( QgsHstoreUtils::build( map ), u"\"1\"=>\"one\",\"2\"=>\"two\",\"3\"=>\"three\""_s );

  map.clear();
  map[u"1"_s] = "one";
  // if a key is missing its closing quote, the map construction process will stop and a partial map is returned
  QCOMPARE( QgsHstoreUtils::parse( u"\"1\"=>\"one\",\"2=>\"two\""_s ), QVariantMap( map ) );
}

QGSTEST_MAIN( TestQgsHstoreUtils )
#include "testqgshstoreutils.moc"
