/***************************************************************************
     testqgssettingsentry.cpp
     --------------------------------------
    Date                 : 01.04.2021
    Copyright            : (C) 2021 by Damiano Lombardi
    Email                : damiano@opengis.ch
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
#include "qgssettingsentry.h"
#include "qgsunittypes.h"
#include "qgsmaplayerproxymodel.h"
#include "qgstest.h"


/**
 * \ingroup UnitTests
 * This is a unit test for QgsSettingsEntry classes
 * \note Most functions are covered in the python test
 */
class TestQgsSettingsEntry : public QObject
{
    Q_OBJECT

  private slots:
    void settingsKey();
    void enumValue();
    void flagValue();
};

void TestQgsSettingsEntry::settingsKey()
{
  QgsSettings settings;

  {
    QString key( QStringLiteral( "/qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 42 );

    QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::NoSection, 0 );
    QCOMPARE( settingsEntryInteger.value(), 42 );
  }

  {
    QString key( QStringLiteral( "qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 43 );

    QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::NoSection, 0 );
    QCOMPARE( settingsEntryInteger.value(), 43 );
  }

  {
    QString key( QStringLiteral( "/qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key, QgsSettings::Core );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 44, QgsSettings::Core );

    QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::Core, 0 );
    QCOMPARE( settingsEntryInteger.value(), 44 );
  }

  {
    QString key( QStringLiteral( "qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key, QgsSettings::Core );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 45, QgsSettings::Core );

    QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::Core, 0 );
    QCOMPARE( settingsEntryInteger.value(), 45 );
  }
}

void TestQgsSettingsEntry::enumValue()
{
  QString settingsKey( QStringLiteral( "qgis/testing/my_enum_value_for_units" ) );

  // Make sure the setting is not existing
  QgsSettings().remove( settingsKey, QgsSettings::NoSection );

  QgsSettingsEntryEnumFlag settingsEntryEnum( settingsKey, QgsSettings::NoSection, QgsUnitTypes::LayoutMeters, QStringLiteral( "Layout unit" ) );

  // Check default value
  QCOMPARE( settingsEntryEnum.defaultValue(), QgsUnitTypes::LayoutMeters );

  // Check set value
  {
    bool success = settingsEntryEnum.setValue( QgsUnitTypes::LayoutFeet );
    QCOMPARE( success, true );
    QgsUnitTypes::LayoutUnit qgsSettingsValue = QgsSettings().enumValue( settingsKey, QgsUnitTypes::LayoutMeters, QgsSettings::NoSection );
    QCOMPARE( qgsSettingsValue, QgsUnitTypes::LayoutFeet );
  }

  // Check get value
  QgsSettings().setEnumValue( settingsKey, QgsUnitTypes::LayoutPicas, QgsSettings::NoSection );
  QCOMPARE( settingsEntryEnum.value(), QgsUnitTypes::LayoutPicas );

  // Check settings type
  QCOMPARE( settingsEntryEnum.settingsType(), QgsSettingsEntryBase::SettingsType::EnumFlag );

  // assign to inexisting value
  {
    bool success = settingsEntryEnum.setValue( static_cast<QgsUnitTypes::LayoutUnit>( -1 ) );
    QCOMPARE( success, false );

    // Current value should not have changed
    QgsUnitTypes::LayoutUnit qgsSettingsValue = QgsSettings().enumValue( settingsKey, QgsUnitTypes::LayoutMeters, QgsSettings::NoSection );
    QCOMPARE( qgsSettingsValue, QgsUnitTypes::LayoutPicas );
  }

  // check that value is stored as string
  QCOMPARE( settingsEntryEnum.valueAsVariant().toString(), QMetaEnum::fromType<QgsUnitTypes::LayoutUnit>().key( QgsUnitTypes::LayoutPicas ) );

  // auto conversion of old settings (int to str)
  QSettings().setValue( settingsKey, static_cast<int>( QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( settingsEntryEnum.valueAsVariant().toInt(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( settingsEntryEnum.value(), QgsUnitTypes::LayoutCentimeters );
}

void TestQgsSettingsEntry::flagValue()
{
  QString settingsKey( QStringLiteral( "qgis/testing/my_flag_value_for_units" ) );
  QgsMapLayerProxyModel::Filters pointAndLine = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::PointLayer | QgsMapLayerProxyModel::LineLayer );
  QgsMapLayerProxyModel::Filters pointAndPolygon = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::PointLayer | QgsMapLayerProxyModel::PolygonLayer );
  QgsMapLayerProxyModel::Filters hasGeometry = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::HasGeometry );

  // Make sure the setting is not existing
  QgsSettings().remove( settingsKey, QgsSettings::NoSection );

  QgsSettingsEntryEnumFlag settingsEntryFlag( settingsKey, QgsSettings::NoSection, pointAndLine, QStringLiteral( "Filters" ) );

  // Check default value
  QCOMPARE( settingsEntryFlag.defaultValue(), pointAndLine );

  // Check set value
  {
    bool success = settingsEntryFlag.setValue( hasGeometry );
    QCOMPARE( success, true );
    QgsMapLayerProxyModel::Filters qgsSettingsValue = QgsSettings().flagValue( settingsKey, pointAndLine, QgsSettings::NoSection );
    QCOMPARE( qgsSettingsValue, hasGeometry );
  }

  // Check get value
  QgsSettings().setFlagValue( settingsKey, pointAndLine, QgsSettings::NoSection );
  QCOMPARE( settingsEntryFlag.value(), pointAndLine );

  // Check settings type
  QCOMPARE( settingsEntryFlag.settingsType(), QgsSettingsEntryBase::SettingsType::EnumFlag );

  // check that value is stored as string
  QCOMPARE( settingsEntryFlag.valueAsVariant().toByteArray(), QMetaEnum::fromType<QgsMapLayerProxyModel::Filters>().valueToKeys( pointAndLine ) );

  // auto conversion of old settings (int to str)
  QSettings().setValue( settingsKey, static_cast<int>( pointAndPolygon ) );
  QCOMPARE( settingsEntryFlag.valueAsVariant().toInt(), pointAndPolygon );
  QCOMPARE( settingsEntryFlag.value(), pointAndPolygon );
}


QGSTEST_MAIN( TestQgsSettingsEntry )
#include "testqgssettingsentry.moc"
