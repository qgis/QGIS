/***************************************************************************
     testqgssettings.cpp
     --------------------------------------
    Date                 : 17.02.2018
    Copyright            : (C) 2018 by Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QObject>


#include "qgssettings.h"
#include "qgsmaplayerproxymodel.h"
#include "qgstest.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the enum and flag functions of QgsSettings
 */
class TestQgsSettings : public QObject
{
    Q_OBJECT

  private slots:
    void enumValue();
    void flagValue();
};


void TestQgsSettings::enumValue()
{
  QgsSettings settings;

  // assign to inexisting value
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units" ), -1 );
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units_as_string" ), QStringLiteral( "myString" ) );
  // just to be sure it really doesn't exist
  QVERIFY( static_cast<int>( Qgis::LayoutUnit::Meters ) != -1 );

  // standard method returns invalid value
  const int v1 = settings.value( QStringLiteral( "qgis/testing/my_value_for_units" ), static_cast<int>( Qgis::LayoutUnit::Meters ) ).toInt();
  QCOMPARE( v1, -1 );

  // enum method returns default value if current setting is incorrect
  const Qgis::LayoutUnit v2 = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), Qgis::LayoutUnit::Meters );
  QCOMPARE( v2, Qgis::LayoutUnit::Meters );
  const Qgis::LayoutUnit v2s = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units_as_string" ), Qgis::LayoutUnit::Meters );
  QCOMPARE( v2s, Qgis::LayoutUnit::Meters );

  // test a different value than default
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units" ), static_cast<int>( Qgis::LayoutUnit::Centimeters ) );
  const Qgis::LayoutUnit v3 = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), Qgis::LayoutUnit::Meters );
  QCOMPARE( v3, Qgis::LayoutUnit::Centimeters );
  settings.setEnumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), Qgis::LayoutUnit::Centimeters );
  // auto conversion of old settings (int to str)
  QCOMPARE( settings.value( "qgis/testing/my_value_for_units" ).toString(), QStringLiteral( "Centimeters" ) );
  const Qgis::LayoutUnit v3s = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), Qgis::LayoutUnit::Meters );
  QCOMPARE( v3s, Qgis::LayoutUnit::Centimeters );
  const QString v3ss = settings.value( QStringLiteral( "qgis/testing/my_value_for_units" ), QStringLiteral( "myDummyValue" ) ).toString();
  QCOMPARE( v3ss, QStringLiteral( "Centimeters" ) );
}

void TestQgsSettings::flagValue()
{
  QgsSettings settings;
  const Qgis::LayerFilters pointAndLine = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::LineLayer );
  const Qgis::LayerFilters pointAndPolygon = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::PolygonLayer );
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), 1e8 ); // invalid
  const Qgis::LayerFilters v4 = settings.flagValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), pointAndLine );
  QCOMPARE( v4, pointAndLine );

  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), static_cast<int>( pointAndPolygon ) );
  const Qgis::LayerFilters v5 = settings.flagValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), pointAndLine, QgsSettings::NoSection );
  QCOMPARE( v5, pointAndPolygon );
  // auto conversion of old settings (int to str)
  QCOMPARE( settings.value( "qgis/testing/my_value_for_a_flag" ).toString(), QStringLiteral( "PointLayer|PolygonLayer" ) );

  settings.setFlagValue( QStringLiteral( "qgis/testing/my_value_for_a_flag_as_string" ), pointAndPolygon, QgsSettings::NoSection );
  const Qgis::LayerFilters v5s = settings.flagValue( QStringLiteral( "qgis/testing/my_value_for_a_flag_as_string" ), pointAndLine, QgsSettings::NoSection );
  QCOMPARE( v5s, pointAndPolygon );
  const QString v5ss = settings.value( QStringLiteral( "qgis/testing/my_value_for_a_flag_as_string" ), QStringLiteral( "myDummyString" ), QgsSettings::NoSection ).toString();
  QCOMPARE( v5ss, QStringLiteral( "PointLayer|PolygonLayer" ) );
}


QGSTEST_MAIN( TestQgsSettings )
#include "testqgssettings.moc"
