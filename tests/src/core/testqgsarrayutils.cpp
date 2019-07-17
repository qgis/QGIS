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

    void testPgArrayStringToListAndBack();
    void testUnquotedPgArrayStringToListAndBack();
    void testNumberArrayStringToListAndBack();
    void testMultidimensionalPgArrayStringToListAndBack();
};


void TestQgsArrayUtils::testPgArrayStringToListAndBack()
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
  QCOMPARE( QgsArrayUtils::parse( string ), vl );

  // and back
  QCOMPARE( QgsArrayUtils::build( vl ), string );
}

void TestQgsArrayUtils::testUnquotedPgArrayStringToListAndBack()
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

  // and back including quotes
  QString new_string = QStringLiteral( "{\"one\",\"two\",\"three\",\"four\",\"five\",\"that genius\"}" );
  QCOMPARE( QgsArrayUtils::build( vl ), new_string );
}

void TestQgsArrayUtils::testNumberArrayStringToListAndBack()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( 1 );
  vl.push_back( 2 );
  vl.push_back( 3 );
  vl.push_back( 4 );

  QString number_string = QStringLiteral( "{1,2,3,4}" );
  QCOMPARE( QgsArrayUtils::parse( number_string ), vl );

  // and back without quotes
  QCOMPARE( QgsArrayUtils::build( vl ), number_string );
}

void TestQgsArrayUtils::testMultidimensionalPgArrayStringToListAndBack()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( QStringLiteral( "{one one third,one two third,one three third}" ) );
  vl.push_back( QStringLiteral( "{\"two one third\",\"two two third\",\"two three third\"}" ) );
  vl.push_back( QStringLiteral( "{three one third,three two third,three three third}" ) );

  QString string = QStringLiteral( "{{one one third,one two third,one three third},{\"two one third\",\"two two third\",\"two three third\"},{three one third,three two third,three three third}}" );
  QCOMPARE( QgsArrayUtils::parse( string ), vl );

  // and back without quotes
  QCOMPARE( QgsArrayUtils::build( vl ), string );
}

QGSTEST_MAIN( TestQgsArrayUtils )
#include "testqgsarrayutils.moc"
