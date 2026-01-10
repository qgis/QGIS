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

#include "qgspostgresstringutils.h"
#include "qgstest.h"

#include <QString>

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
  vl.push_back( u"one"_s );
  vl.push_back( u"}two{"_s );
  vl.push_back( u"thr\"ee"_s );
  vl.push_back( u"fo,ur"_s );
  vl.push_back( u"fiv'e"_s );
  vl.push_back( 6 );
  vl.push_back( u"and 7garcìa]["_s );
  vl.push_back( u"...all the etceteras"_s );

  const QString string = u"{\"one\",\"}two{\",\"thr\\\"ee\",\"fo,ur\",\"fiv'e\",6,\"and 7garcìa][\",\"...all the etceteras\"}"_s;
  QCOMPARE( QgsPostgresStringUtils::parseArray( string ), vl );

  // and back
  QCOMPARE( QgsPostgresStringUtils::buildArray( vl ), string );
}

void TestQgsPostgresStringUtils::testUnquotedPgArrayStringToListAndBack()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( u"one"_s );
  vl.push_back( u"two"_s );
  vl.push_back( u"three"_s );
  vl.push_back( u"four"_s );
  vl.push_back( u"five"_s );
  vl.push_back( u"that genius"_s );

  const QString fallback_string = u"{one,two,three,four,five,that genius}"_s;
  QCOMPARE( QgsPostgresStringUtils::parseArray( fallback_string ), vl );

  // and back including quotes
  const QString new_string = u"{\"one\",\"two\",\"three\",\"four\",\"five\",\"that genius\"}"_s;
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

  const QString number_string = u"{1,2,3,4}"_s;
  QCOMPARE( QgsPostgresStringUtils::parseArray( number_string ), vl );

  // and back without quotes
  QCOMPARE( QgsPostgresStringUtils::buildArray( vl ), number_string );
}

void TestQgsPostgresStringUtils::testMultidimensionalPgArrayStringToListAndBack()
{
  //there might have been used
  QVariantList vl;
  vl.push_back( u"{one one third,one two third,one three third}"_s );
  vl.push_back( u"{\"two one third\",\"two two third\",\"two three third\"}"_s );
  vl.push_back( u"{three one third,three two third,three three third}"_s );

  const QString string = u"{{one one third,one two third,one three third},{\"two one third\",\"two two third\",\"two three third\"},{three one third,three two third,three three third}}"_s;
  QCOMPARE( QgsPostgresStringUtils::parseArray( string ), vl );

  // and back without quotes
  QCOMPARE( QgsPostgresStringUtils::buildArray( vl ), string );
}

QGSTEST_MAIN( TestQgsPostgresStringUtils )
#include "testqgspostgresstringutils.moc"
