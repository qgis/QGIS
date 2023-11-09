/***************************************************************************
     testqgspostgresstringutils.cpp
     --------------------------------------
    Date                 : July 2019
    Copyright            : (C) 2019 David Signer
    email                : david at opengis dot ch
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
#include "qgspostgresstringutils.h"

class TestQgsPostgresStringUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testPgArrayStringToListAndBack();
    void testUnquotedPgArrayStringToListAndBack();
    void testNumberArrayStringToListAndBack();
    void testMultidimensionalPgArrayStringToListAndBack();
};


void TestQgsPostgresStringUtils::testPgArrayStringToListAndBack()
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

  const QString string = QStringLiteral( "{\"one\",\"}two{\",\"thr\\\"ee\",\"fo,ur\",\"fiv'e\",6,\"and 7garcìa][\",\"...all the etceteras\"}" );
  QCOMPARE( QgsPostgresStringUtils::parseArray( string ), vl );

  // and back
  QCOMPARE( QgsPostgresStringUtils::buildArray( vl ), string );
}

void TestQgsPostgresStringUtils::testUnquotedPgArrayStringToListAndBack()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( QStringLiteral( "one" ) );
  vl.push_back( QStringLiteral( "two" ) );
  vl.push_back( QStringLiteral( "three" ) );
  vl.push_back( QStringLiteral( "four" ) );
  vl.push_back( QStringLiteral( "five" ) );
  vl.push_back( QStringLiteral( "that genius" ) );

  const QString fallback_string = QStringLiteral( "{one,two,three,four,five,that genius}" );
  QCOMPARE( QgsPostgresStringUtils::parseArray( fallback_string ), vl );

  // and back including quotes
  const QString new_string = QStringLiteral( "{\"one\",\"two\",\"three\",\"four\",\"five\",\"that genius\"}" );
  QCOMPARE( QgsPostgresStringUtils::buildArray( vl ), new_string );
}

void TestQgsPostgresStringUtils::testNumberArrayStringToListAndBack()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( 1 );
  vl.push_back( 2 );
  vl.push_back( 3 );
  vl.push_back( 4 );

  const QString number_string = QStringLiteral( "{1,2,3,4}" );
  QCOMPARE( QgsPostgresStringUtils::parseArray( number_string ), vl );

  // and back without quotes
  QCOMPARE( QgsPostgresStringUtils::buildArray( vl ), number_string );
}

void TestQgsPostgresStringUtils::testMultidimensionalPgArrayStringToListAndBack()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( QStringLiteral( "{one one third,one two third,one three third}" ) );
  vl.push_back( QStringLiteral( "{\"two one third\",\"two two third\",\"two three third\"}" ) );
  vl.push_back( QStringLiteral( "{three one third,three two third,three three third}" ) );

  const QString string = QStringLiteral( "{{one one third,one two third,one three third},{\"two one third\",\"two two third\",\"two three third\"},{three one third,three two third,three three third}}" );
  QCOMPARE( QgsPostgresStringUtils::parseArray( string ), vl );

  // and back without quotes
  QCOMPARE( QgsPostgresStringUtils::buildArray( vl ), string );
}

QGSTEST_MAIN( TestQgsPostgresStringUtils )
#include "testqgspostgresstringutils.moc"
