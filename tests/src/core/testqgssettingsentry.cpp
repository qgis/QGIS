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

  private:
    const QString mSettingsSection = QStringLiteral( "settingsEntryBool" );

  private slots:
    void settingsKey();
    void enumValue();
    void flagValue();
    void testFormerValue();
    void testChanged();
};

void TestQgsSettingsEntry::settingsKey()
{
  QgsSettings settings;

  {
    const QString key( QStringLiteral( "/settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 42 );

    const QgsSettingsEntryInteger settingsEntryInteger( key, 0 );
    QCOMPARE( settingsEntryInteger.value(), 42 );
  }

  {
    const QString key( QStringLiteral( "settingsKey" ) );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QgsSettings and QgsSettingsEntry
    settings.setValue( key, 43 );

    const QgsSettingsEntryInteger settingsEntryInteger( key, 0 );
    QCOMPARE( settingsEntryInteger.value(), 43 );
  }
}

void TestQgsSettingsEntry::enumValue()
{
  const QString settingsKey( QStringLiteral( "my_enum_value_for_units" ) );

  // Make sure the setting is not existing
  QgsSettings().remove( settingsKey );

  const QgsSettingsEntryEnumFlag settingsEntryEnum( settingsKey, mSettingsSection, Qgis::LayoutUnit::Meters, QStringLiteral( "Layout unit" ) );

  // Check default value
  QCOMPARE( settingsEntryEnum.defaultValue(), Qgis::LayoutUnit::Meters );

  // Check set value
  {
    const bool success = settingsEntryEnum.setValue( Qgis::LayoutUnit::Feet );
    QCOMPARE( success, true );
    const Qgis::LayoutUnit qgsSettingsValue = QgsSettings().enumValue( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), Qgis::LayoutUnit::Meters );
    QCOMPARE( qgsSettingsValue, Qgis::LayoutUnit::Feet );
  }

  // Check get value
  QgsSettings().setEnumValue( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), Qgis::LayoutUnit::Picas );
  QCOMPARE( settingsEntryEnum.value(), Qgis::LayoutUnit::Picas );

  // Check settings type
  QCOMPARE( settingsEntryEnum.settingsType(), Qgis::SettingsType::EnumFlag );

  // assign to inexisting value
  {
    const bool success = settingsEntryEnum.setValue( static_cast<Qgis::LayoutUnit>( -1 ) );
    QCOMPARE( success, false );

    // Current value should not have changed
    const Qgis::LayoutUnit qgsSettingsValue = QgsSettings().enumValue( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), Qgis::LayoutUnit::Meters );
    QCOMPARE( qgsSettingsValue, Qgis::LayoutUnit::Picas );
  }

  // check that value is stored as string
  QCOMPARE( settingsEntryEnum.valueAsVariant().toString(), QMetaEnum::fromType<Qgis::LayoutUnit>().key( static_cast<int>( Qgis::LayoutUnit::Picas ) ) );

  // auto conversion of old settings (int to str)
  QSettings().setValue( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), static_cast<int>( Qgis::LayoutUnit::Centimeters ) );
  QCOMPARE( settingsEntryEnum.valueAsVariant().toInt(), static_cast<int>( Qgis::LayoutUnit::Centimeters ) );
  QCOMPARE( settingsEntryEnum.value(), Qgis::LayoutUnit::Centimeters );

  // save as int instead of string
  const QgsSettingsEntryEnumFlag settingsEntryEnumAsInteger( settingsKey, mSettingsSection, Qgis::LayoutUnit::Meters, QStringLiteral( "Layout unit" ), Qgis::SettingsOption::SaveEnumFlagAsInt );
  settingsEntryEnumAsInteger.remove();
  {
    int qgsSettingsValue = static_cast<int>( settingsEntryEnumAsInteger.value() );
    QCOMPARE( qgsSettingsValue, static_cast<int>( Qgis::LayoutUnit::Meters ) );
    const bool success = settingsEntryEnumAsInteger.setValue( Qgis::LayoutUnit::Feet );
    QCOMPARE( success, true );
    qgsSettingsValue = QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), static_cast<int>( Qgis::LayoutUnit::Meters ) ).toInt();
    QCOMPARE( qgsSettingsValue, static_cast<int>( Qgis::LayoutUnit::Feet ) );
  }
}

void TestQgsSettingsEntry::flagValue()
{
  const QString settingsKey( QStringLiteral( "my_flag_value_for_units" ) );
  const Qgis::LayerFilters pointAndLine = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::LineLayer );
  const Qgis::LayerFilters pointAndPolygon = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::PolygonLayer );
  const Qgis::LayerFilters hasGeometry = Qgis::LayerFilters( Qgis::LayerFilter::HasGeometry );

  // Make sure the setting is not existing
  QgsSettings().remove( settingsKey );

  const QgsSettingsEntryEnumFlag settingsEntryFlag( settingsKey, mSettingsSection, Qgis::LayerFilters(), QStringLiteral( "Filters" ) );

  // Check default value
  QCOMPARE( settingsEntryFlag.defaultValue(), Qgis::LayerFilters() );

  // check no value
  QCOMPARE( settingsEntryFlag.exists(), false );
  QCOMPARE( settingsEntryFlag.value(), Qgis::LayerFilters() );

  QCOMPARE( settingsEntryFlag.valueWithDefaultOverride( pointAndLine ), pointAndLine );

  // Check set value
  {
    const bool success = settingsEntryFlag.setValue( hasGeometry );
    QCOMPARE( success, true );
    const Qgis::LayerFilters qgsSettingsValue = QgsSettings().flagValue( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), pointAndLine );
    QCOMPARE( qgsSettingsValue, hasGeometry );
  }

  // Check get value
  QgsSettings().setFlagValue( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), pointAndLine );
  QCOMPARE( settingsEntryFlag.value(), pointAndLine );

  // Check settings type
  QCOMPARE( settingsEntryFlag.settingsType(), Qgis::SettingsType::EnumFlag );

  // check that value is stored as string
  QCOMPARE( settingsEntryFlag.valueAsVariant().toByteArray(), QMetaEnum::fromType<Qgis::LayerFilters>().valueToKeys( pointAndLine ) );

  // auto conversion of old settings (int to str)
  QSettings().setValue( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), static_cast<int>( pointAndPolygon ) );
  QCOMPARE( settingsEntryFlag.valueAsVariant().toInt(), pointAndPolygon );
  QCOMPARE( settingsEntryFlag.value(), pointAndPolygon );
}

void TestQgsSettingsEntry::testFormerValue()
{
  const QString settingsKey( QStringLiteral( "settingsEntryInteger/integer-former-value" ) );
  const QString settingsKeyFormer = settingsKey + ( QStringLiteral( "_formervalue" ) );

  QgsSettings().remove( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ) );
  QgsSettings().remove( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKeyFormer ) );
  int defaultValue = 111;
  int defaultFormerValue = 222;

  QgsSettingsEntryInteger settingsEntryInteger = QgsSettingsEntryInteger( settingsKey, mSettingsSection, defaultValue, QString(), Qgis::SettingsOption::SaveFormerValue );

  QCOMPARE( settingsEntryInteger.formerValue(), defaultValue );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), defaultValue ), defaultValue );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ), defaultFormerValue );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), defaultValue ), 2 );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ), defaultFormerValue );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), defaultValue ), 2 );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ), defaultFormerValue );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 3 );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ), defaultValue ), 3 );
  QCOMPARE( QgsSettings().value( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ).toLongLong(), 2 );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( settingsEntryInteger.formerValue(), 3 );
}

void TestQgsSettingsEntry::testChanged()
{
  const QString settingsKey( QStringLiteral( "settingsEntryInteger/integer-value" ) );
  QgsSettings().remove( QStringLiteral( "%1/%2" ).arg( mSettingsSection, settingsKey ) );
  int defaultValue = 111;

  QgsSettings().setValue( QStringLiteral( "testSetting" ), 1 );

  QgsSettingsEntryInteger settingsEntryInteger = QgsSettingsEntryInteger( settingsKey, mSettingsSection, defaultValue );
  QVERIFY( !settingsEntryInteger.hasChanged() );
  settingsEntryInteger.copyValueToKeyIfChanged( QStringLiteral( "testSetting" ) );
  QCOMPARE( QgsSettings().value( QStringLiteral( "testSetting" ) ).toInt(), 1 );

  settingsEntryInteger.setValue( 11111 );
  QVERIFY( settingsEntryInteger.hasChanged() );

  settingsEntryInteger.copyValueToKeyIfChanged( QStringLiteral( "testSetting" ) );
  QCOMPARE( QgsSettings().value( QStringLiteral( "testSetting" ) ).toInt(), 11111 );
}


QGSTEST_MAIN( TestQgsSettingsEntry )
#include "testqgssettingsentry.moc"
