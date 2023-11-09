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

#include <QString>
#include "qgstest.h"
#include "qgshstoreutils.h"

class TestQgsHstoreUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testHstore();
};


void TestQgsHstoreUtils::testHstore()
{
  QVariantMap map;
  map[QStringLiteral( "1" )] = "one";
  map[QStringLiteral( "2" )] = "two";
  map[QStringLiteral( "3" )] = "three";

  QCOMPARE( QgsHstoreUtils::parse( QStringLiteral( "1=>one,2=>two,3=>three" ) ), map );
  QCOMPARE( QgsHstoreUtils::build( map ), QStringLiteral( "\"1\"=>\"one\",\"2\"=>\"two\",\"3\"=>\"three\"" ) );

  map.clear();
  map[QStringLiteral( "1" )] = "one";
  // if a key is missing its closing quote, the map construction process will stop and a partial map is returned
  QCOMPARE( QgsHstoreUtils::parse( QStringLiteral( "\"1\"=>\"one\",\"2=>\"two\"" ) ), QVariantMap( map ) );
}

QGSTEST_MAIN( TestQgsHstoreUtils )
#include "testqgshstoreutils.moc"
