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
#include "qgsarrayutils.h"

class TestQgsArrayUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testListToPgArrayStringAndBack();
    void testFallbackPgArrayStringToList();
};


void TestQgsArrayUtils::testListToPgArrayStringAndBack()
{

  QVariantList vl;
  vl.push_back( QStringLiteral( "one" ) );
  vl.push_back( QStringLiteral( "}two{" ) );
  vl.push_back( QStringLiteral( "thr\"ee" ) );
  vl.push_back( QStringLiteral( "fo,ur" ) );
  vl.push_back( QStringLiteral( "fiv'e" ) );
  vl.push_back( 6 );
  vl.push_back( QStringLiteral( "and 7garcìa][" ) );
  vl.push_back( QStringLiteral( "...all the etceteras" ) );

  QString string = QStringLiteral( "{\"one\",\"}two{\",\"thr\\\"ee\",\"fo,ur\",\"fiv'e\",6,\"and 7garcìa][\",\"...all the etceteras\"}" );

  QCOMPARE( QgsArrayUtils::build( vl ), string );

  // and back
  QCOMPARE( QgsArrayUtils::parse( string ), vl );
}

void TestQgsArrayUtils::testFallbackPgArrayStringToList()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( QStringLiteral( "one" ) );
  vl.push_back( QStringLiteral( "two" ) );
  vl.push_back( QStringLiteral( "three" ) );
  vl.push_back( QStringLiteral( "four" ) );
  vl.push_back( QStringLiteral( "five" ) );
  vl.push_back( QStringLiteral( "that genius" ) );

  QString fallback_string = QStringLiteral( "{one,two,three,four,five,that genius}" );

  QCOMPARE( QgsArrayUtils::parse( fallback_string ), vl );

  // and back with quotes
  QString new_string = QStringLiteral( "{\"one\",\"two\",\"three\",\"four\",\"five\",\"that genius\"}" );
  QCOMPARE( QgsArrayUtils::build( vl ), new_string );
}

QGSTEST_MAIN( TestQgsArrayUtils )
#include "testqgsarrayutils.moc"
