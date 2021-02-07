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
#include "qgssettingsregistry.h"
#include "qgsunittypes.h"
#include "qgsmaplayerproxymodel.h"
#include "qgstest.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the operations on curve geometries
 */
class TestQgsSettingsRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void variantValue();
    void enumValue();
    void flagValue();
};

void TestQgsSettingsRegistry::variantValue()
{
  QgsSettingsRegistry settingsRegistry(QgsSettings::NoSection,
                                       this);

  QString settingsName( "qgis/testing/settings_registry/my_variant_value" );
  QVariant defaultValue( 42 );
  QString settingsDescription( "Variant value for testing of 'QgsSettingsRegistry'" );
  settingsRegistry.registerValue( settingsName,
                                  defaultValue.type(),
                                  defaultValue,
                                  settingsDescription );

  // Check default value
  QCOMPARE( settingsRegistry.value(settingsName), defaultValue );

  // Set new value
  QVariant newValue( 1234 );
  settingsRegistry.setValue( settingsName,
                             newValue );

  // Check new value with QgsSettings
  QCOMPARE( QgsSettings().value( settingsName, defaultValue ), newValue );

  // Check new value with QgsSettingsRegistry
  QCOMPARE( settingsRegistry.value( settingsName ), newValue);

  // Check set on unregistered settings name
  settingsRegistry.unregister( settingsName );
  QCOMPARE( settingsRegistry.setValue( settingsName, QVariant() ), false );
}

void TestQgsSettingsRegistry::enumValue()
{
  QgsSettings settings;

  // assign to inexisting value
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units" ), -1 );
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units_as_string" ), QStringLiteral( "myString" ) );
  // just to be sure it really doesn't exist
  QVERIFY( static_cast<int>( QgsUnitTypes::LayoutMeters ) != -1 );

  // standard method returns invalid value
  int v1 = settings.value( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutMeters ).toInt();
  QCOMPARE( v1, -1 );

  // enum method returns default value if current setting is incorrect
  QgsUnitTypes::LayoutUnit v2 = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutMeters );
  QCOMPARE( v2, QgsUnitTypes::LayoutMeters );
  QgsUnitTypes::LayoutUnit v2s = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units_as_string" ), QgsUnitTypes::LayoutMeters );
  QCOMPARE( v2s, QgsUnitTypes::LayoutMeters );

  // test a different value than default
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutCentimeters );
  QgsUnitTypes::LayoutUnit v3 = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutMeters );
  QCOMPARE( v3, QgsUnitTypes::LayoutCentimeters );
  settings.setEnumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutCentimeters );
  // auto conversion of old settings (int to str)
  QCOMPARE( settings.value( "qgis/testing/my_value_for_units" ).toString(), QStringLiteral( "LayoutCentimeters" ) );
  QgsUnitTypes::LayoutUnit v3s = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutMeters );
  QCOMPARE( v3s, QgsUnitTypes::LayoutCentimeters );
  QString v3ss = settings.value( QStringLiteral( "qgis/testing/my_value_for_units" ), QStringLiteral( "myDummyValue" ) ).toString();
  QCOMPARE( v3ss, QStringLiteral( "LayoutCentimeters" ) );
}

void TestQgsSettingsRegistry::flagValue()
{
  QgsSettings settings;
  QgsMapLayerProxyModel::Filters pointAndLine = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::PointLayer | QgsMapLayerProxyModel::LineLayer );
  QgsMapLayerProxyModel::Filters pointAndPolygon = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::PointLayer | QgsMapLayerProxyModel::PolygonLayer );
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), 1e8 ); // invalid
  QgsMapLayerProxyModel::Filters v4 = settings.enumValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), pointAndLine );
  QCOMPARE( v4, pointAndLine );

  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), static_cast<int>( pointAndPolygon ) );
  QgsMapLayerProxyModel::Filters v5 = settings.flagValue( QStringLiteral( "qgis/testing/my_value_for_a_flag" ), pointAndLine, QgsSettings::NoSection );
  QCOMPARE( v5, pointAndPolygon );
  // auto conversion of old settings (int to str)
  QCOMPARE( settings.value( "qgis/testing/my_value_for_a_flag" ).toString(), QStringLiteral( "PointLayer|PolygonLayer" ) );

  settings.setFlagValue( QStringLiteral( "qgis/testing/my_value_for_a_flag_as_string" ), pointAndPolygon, QgsSettings::NoSection );
  QgsMapLayerProxyModel::Filters v5s = settings.flagValue( QStringLiteral( "qgis/testing/my_value_for_a_flag_as_string" ), pointAndLine, QgsSettings::NoSection );
  QCOMPARE( v5s, pointAndPolygon );
  QString v5ss = settings.value( QStringLiteral( "qgis/testing/my_value_for_a_flag_as_string" ), QStringLiteral( "myDummyString" ), QgsSettings::NoSection ).toString();
  QCOMPARE( v5ss, QStringLiteral( "PointLayer|PolygonLayer" ) );
}


QGSTEST_MAIN( TestQgsSettingsRegistry )
#include "testqgssettingsregistry.moc"
