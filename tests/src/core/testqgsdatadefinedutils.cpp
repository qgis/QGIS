/***************************************************************************
     testqgsdatadefinedutils.cpp
     ------------
    Date                 : October 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <memory>

#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

#include "qgis.h"
#include "qgsdatadefinedutils.h"

/**
 * \ingroup UnitTests
 */
class TestQgsDataDefinedUtils : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsDataDefinedUtils()
      : QgsTest( u"Qgis data defined utils Tests"_s )
    {}

  private slots:
    void init() {}    // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testDataDefinedStringDecoder_data();
    void testDataDefinedStringDecoder();
};


void TestQgsDataDefinedUtils::testDataDefinedStringDecoder_data()
{
  QTest::addColumn<QString>( "candidate" );
  QTest::addColumn<bool>( "isMatch" );
  QTest::addColumn<Qgis::LabelPredefinedPointPosition>( "expected" );

  QTest::newRow( "null string" ) << QString() << false << Qgis::LabelPredefinedPointPosition::OverPoint;
  QTest::newRow( "empty string" ) << "" << false << Qgis::LabelPredefinedPointPosition::OverPoint;
  QTest::newRow( "whitespace only string" ) << u"   "_s << false << Qgis::LabelPredefinedPointPosition::OverPoint;

  QTest::newRow( "xxyyyzz" ) << u"xxyyyzz"_s << false << Qgis::LabelPredefinedPointPosition::OverPoint;
  QTest::newRow( "xxyyTopLeftyzz" ) << u"xxyyTopLeftyzz"_s << false << Qgis::LabelPredefinedPointPosition::OverPoint;
  QTest::newRow( "2a33245" ) << u"2a33245"_s << false << Qgis::LabelPredefinedPointPosition::OverPoint;
  QTest::newRow( "!@#@!#" ) << u"!@#@!#"_s << false << Qgis::LabelPredefinedPointPosition::OverPoint;
  QTest::newRow( "unicode" ) << u"🄗"_s << false << Qgis::LabelPredefinedPointPosition::OverPoint;

  QTest::newRow( "topleft" ) << u"topleft"_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "Topleft" ) << u"Topleft"_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "TopLeft" ) << u"TopLeft"_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "top_left" ) << u"top_left"_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "Top_left" ) << u"Top_left"_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "Top_Left" ) << u"Top_Left"_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "   topleft" ) << u"   topleft"_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "topleft   " ) << u"topleft   "_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;
  QTest::newRow( "   topleft   " ) << u"   topleft   "_s << true << Qgis::LabelPredefinedPointPosition::TopLeft;

  QTest::newRow( "topright" ) << u"topright"_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "Topright" ) << u"Topright"_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "TopRight" ) << u"TopRight"_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "top_right" ) << u"top_right"_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "Top_right" ) << u"Top_right"_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "Top_Right" ) << u"Top_Right"_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "   topright" ) << u"   topright"_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "topright   " ) << u"topright   "_s << true << Qgis::LabelPredefinedPointPosition::TopRight;
  QTest::newRow( "   topright   " ) << u"   topright   "_s << true << Qgis::LabelPredefinedPointPosition::TopRight;

  QTest::newRow( "bottom" ) << u"bottom"_s << true << Qgis::LabelPredefinedPointPosition::BottomMiddle;
  QTest::newRow( "BOTTOM" ) << u"BOTTOM"_s << true << Qgis::LabelPredefinedPointPosition::BottomMiddle;
  QTest::newRow( "boTTom" ) << u"boTTom"_s << true << Qgis::LabelPredefinedPointPosition::BottomMiddle;
}

void TestQgsDataDefinedUtils::testDataDefinedStringDecoder()
{
  static constexpr auto sPointPositionMap = makeDataDefinedStringDecoder<Qgis::LabelPredefinedPointPosition>(
    { { u"topleft", Qgis::LabelPredefinedPointPosition::TopLeft },
      { u"top_left", Qgis::LabelPredefinedPointPosition::TopLeft },
      { u"topright", Qgis::LabelPredefinedPointPosition::TopRight },
      { u"top_right", Qgis::LabelPredefinedPointPosition::TopRight },
      { u"bottom", Qgis::LabelPredefinedPointPosition::BottomMiddle } }
  );

  QFETCH( QString, candidate );
  QFETCH( bool, isMatch );
  QFETCH( Qgis::LabelPredefinedPointPosition, expected );

  if ( auto evaluatedPosition = sPointPositionMap.lookup( candidate ) )
  {
    QVERIFY( isMatch );
    QCOMPARE( *evaluatedPosition, expected );
  }
  else
  {
    QVERIFY( !isMatch );
  };
}


QGSTEST_MAIN( TestQgsDataDefinedUtils )
#include "testqgsdatadefinedutils.moc"
