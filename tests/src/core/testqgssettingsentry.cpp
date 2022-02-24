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
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
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
    void testFormerValue();
};

void TestQgsSettingsEntry::settingsKey()
{
  QgsSettings settings;

  {
    const QString key( QStringLiteral( "/qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 42 );

    const QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::NoSection, 0 );
    QCOMPARE( settingsEntryInteger.value(), 42 );
  }

  {
    const QString key( QStringLiteral( "qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 43 );

    const QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::NoSection, 0 );
    QCOMPARE( settingsEntryInteger.value(), 43 );
  }

  {
    const QString key( QStringLiteral( "/qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key, QgsSettings::Core );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 44, QgsSettings::Core );

    const QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::Core, 0 );
    QCOMPARE( settingsEntryInteger.value(), 44 );
  }

  {
    const QString key( QStringLiteral( "qgis/testing/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key, QgsSettings::Core );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 45, QgsSettings::Core );

    const QgsSettingsEntryInteger settingsEntryInteger( key, QgsSettings::Core, 0 );
    QCOMPARE( settingsEntryInteger.value(), 45 );
  }
}

void TestQgsSettingsEntry::enumValue()
{
  const QString settingsKey( QStringLiteral( "qgis/testing/my_enum_value_for_units" ) );

  // Make sure the setting is not existing
  QgsSettings().remove( settingsKey, QgsSettings::NoSection );

  const QgsSettingsEntryEnumFlag settingsEntryEnum( settingsKey, QgsSettings::NoSection, QgsUnitTypes::LayoutMeters, QStringLiteral( "Layout unit" ) );

  // Check default value
  QCOMPARE( settingsEntryEnum.defaultValue(), QgsUnitTypes::LayoutMeters );

  // Check set value
  {
    const bool success = settingsEntryEnum.setValue( QgsUnitTypes::LayoutFeet );
    QCOMPARE( success, true );
    const QgsUnitTypes::LayoutUnit qgsSettingsValue = QgsSettings().enumValue( settingsKey, QgsUnitTypes::LayoutMeters, QgsSettings::NoSection );
    QCOMPARE( qgsSettingsValue, QgsUnitTypes::LayoutFeet );
  }

  // Check get value
  QgsSettings().setEnumValue( settingsKey, QgsUnitTypes::LayoutPicas, QgsSettings::NoSection );
  QCOMPARE( settingsEntryEnum.value(), QgsUnitTypes::LayoutPicas );

  // Check settings type
  QCOMPARE( settingsEntryEnum.settingsType(), Qgis::SettingsType::EnumFlag );

  // assign to inexisting value
  {
    const bool success = settingsEntryEnum.setValue( static_cast<QgsUnitTypes::LayoutUnit>( -1 ) );
    QCOMPARE( success, false );

    // Current value should not have changed
    const QgsUnitTypes::LayoutUnit qgsSettingsValue = QgsSettings().enumValue( settingsKey, QgsUnitTypes::LayoutMeters, QgsSettings::NoSection );
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
  const QString settingsKey( QStringLiteral( "qgis/testing/my_flag_value_for_units" ) );
  const QgsMapLayerProxyModel::Filters pointAndLine = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::PointLayer | QgsMapLayerProxyModel::LineLayer );
  const QgsMapLayerProxyModel::Filters pointAndPolygon = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::PointLayer | QgsMapLayerProxyModel::PolygonLayer );
  const QgsMapLayerProxyModel::Filters hasGeometry = QgsMapLayerProxyModel::Filters( QgsMapLayerProxyModel::HasGeometry );

  // Make sure the setting is not existing
  QgsSettings().remove( settingsKey, QgsSettings::NoSection );

  const QgsSettingsEntryEnumFlag settingsEntryFlag( settingsKey, QgsSettings::NoSection, pointAndLine, QStringLiteral( "Filters" ) );

  // Check default value
  QCOMPARE( settingsEntryFlag.defaultValue(), pointAndLine );

  // Check set value
  {
    const bool success = settingsEntryFlag.setValue( hasGeometry );
    QCOMPARE( success, true );
    const QgsMapLayerProxyModel::Filters qgsSettingsValue = QgsSettings().flagValue( settingsKey, pointAndLine, QgsSettings::NoSection );
    QCOMPARE( qgsSettingsValue, hasGeometry );
  }

  // Check get value
  QgsSettings().setFlagValue( settingsKey, pointAndLine, QgsSettings::NoSection );
  QCOMPARE( settingsEntryFlag.value(), pointAndLine );

  // Check settings type
  QCOMPARE( settingsEntryFlag.settingsType(), Qgis::SettingsType::EnumFlag );

  // check that value is stored as string
  QCOMPARE( settingsEntryFlag.valueAsVariant().toByteArray(), QMetaEnum::fromType<QgsMapLayerProxyModel::Filters>().valueToKeys( pointAndLine ) );

  // auto conversion of old settings (int to str)
  QSettings().setValue( settingsKey, static_cast<int>( pointAndPolygon ) );
  QCOMPARE( settingsEntryFlag.valueAsVariant().toInt(), pointAndPolygon );
  QCOMPARE( settingsEntryFlag.value(), pointAndPolygon );
}

void TestQgsSettingsEntry::testFormerValue()
{
  const QString settingsKey( QStringLiteral( "qgis/testing/settingsEntryInteger/integer-former-value" ) );
  const QString settingsKeyFormer = settingsKey + ( QStringLiteral( "_formervalue" ) );

  QgsSettings().remove( settingsKey, QgsSettings::NoSection );
  QgsSettings().remove( settingsKeyFormer, QgsSettings::NoSection );
  int defaultValue = 111;
  int defaultFormerValue = 222;

  QgsSettingsEntryInteger settingsEntryInteger = QgsSettingsEntryInteger( settingsKey, QgsSettings::NoSection, defaultValue, QString(), Qgis::SettingsOption::SaveFormerValue );

  QCOMPARE( settingsEntryInteger.formerValue(), defaultValue );
  QCOMPARE( QgsSettings().value( settingsKey, defaultValue, QgsSettings::NoSection ), defaultValue );
  QCOMPARE( QgsSettings().value( settingsKeyFormer, defaultFormerValue, QgsSettings::NoSection ), defaultFormerValue );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( QgsSettings().value( settingsKey, defaultValue, QgsSettings::NoSection ), 2 );
  QCOMPARE( QgsSettings().value( settingsKeyFormer, defaultFormerValue, QgsSettings::NoSection ), defaultFormerValue );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( QgsSettings().value( settingsKey, defaultValue, QgsSettings::NoSection ), 2 );
  QCOMPARE( QgsSettings().value( settingsKeyFormer, defaultFormerValue, QgsSettings::NoSection ), defaultFormerValue );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 3 );
  QCOMPARE( QgsSettings().value( settingsKey, defaultValue, QgsSettings::NoSection ), 3 );
  QCOMPARE( QgsSettings().value( settingsKeyFormer, defaultFormerValue, QgsSettings::NoSection ).toLongLong(), 2 );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( settingsEntryInteger.formerValue(), 3 );
}


QGSTEST_MAIN( TestQgsSettingsEntry )
#include "testqgssettingsentry.moc"
