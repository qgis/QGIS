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
#include "qgsmaplayerproxymodel.h"
#include "qgssettingsentry.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgstest.h"

#include <QObject>
#include <QSettings>
#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup UnitTests
 * This is a unit test for QgsSettingsEntry classes
 * \note Most functions are covered in the python test
 */
class TestQgsSettingsEntry : public QObject
{
    Q_OBJECT

  private:
    const QString mSettingsSection = u"settingsEntryBool"_s;

  private slots:
    void settingsKey();
    void enumValue();
    void flagValue();
    void testFormerValue();
    void testChanged();
};

void TestQgsSettingsEntry::settingsKey()
{
  QSettings &settings = QgsSettingsEntryBase::userSettings();

  {
    const QString key( u"/settingsKey"_s );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QSettings and QgsSettingsEntry
    settings.setValue( key, 42 );

    const QgsSettingsEntryInteger settingsEntryInteger( key, 0 );
    QCOMPARE( settingsEntryInteger.value(), 42 );
  }

  {
    const QString key( u"settingsKey"_s );

    // Be sure that settings does not exist already
    settings.remove( key );

    // Check that keys are handled same way for QSettings and QgsSettingsEntry
    settings.setValue( key, 43 );

    const QgsSettingsEntryInteger settingsEntryInteger( key, 0 );
    QCOMPARE( settingsEntryInteger.value(), 43 );
  }
}

void TestQgsSettingsEntry::enumValue()
{
  const QString settingsKey( u"my_enum_value_for_units"_s );

  // Make sure the setting is not existing
  QgsSettingsEntryBase::userSettings().remove( settingsKey );

  const QgsSettingsEntryEnumFlag settingsEntryEnum( settingsKey, mSettingsSection, Qgis::LayoutUnit::Meters, u"Layout unit"_s );

  // Check default value
  QCOMPARE( settingsEntryEnum.defaultValue(), Qgis::LayoutUnit::Meters );

  // Check set value
  {
    const bool success = settingsEntryEnum.setValue( Qgis::LayoutUnit::Feet );
    QCOMPARE( success, true );
    const Qgis::LayoutUnit value = settingsEntryEnum.value();
    QCOMPARE( value, Qgis::LayoutUnit::Feet );
  }

  // Check get value
  QgsSettingsEntryBase::userSettings().setValue( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), u"Picas"_s );
  QCOMPARE( settingsEntryEnum.value(), Qgis::LayoutUnit::Picas );

  // Check settings type
  QCOMPARE( settingsEntryEnum.settingsType(), Qgis::SettingsType::EnumFlag );

  // assign to inexisting value
  {
    const bool success = settingsEntryEnum.setValue( static_cast<Qgis::LayoutUnit>( -1 ) );
    QCOMPARE( success, false );

    // Current value should not have changed
    QCOMPARE( settingsEntryEnum.value(), Qgis::LayoutUnit::Picas );
  }

  // check that value is stored as string
  QCOMPARE( settingsEntryEnum.valueAsVariant().toString(), u"Picas"_s );

  // auto conversion of old settings (int to str)
  QSettings().setValue( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), static_cast<int>( Qgis::LayoutUnit::Centimeters ) );
  QCOMPARE( settingsEntryEnum.valueAsVariant().toInt(), static_cast<int>( Qgis::LayoutUnit::Centimeters ) );
  QCOMPARE( settingsEntryEnum.value(), Qgis::LayoutUnit::Centimeters );

  // save as int instead of string
  const QgsSettingsEntryEnumFlag settingsEntryEnumAsInteger( settingsKey, mSettingsSection, Qgis::LayoutUnit::Meters, u"Layout unit"_s, Qgis::SettingsOption::SaveEnumFlagAsInt );
  settingsEntryEnumAsInteger.remove();
  {
    int qgsSettingsValue = static_cast<int>( settingsEntryEnumAsInteger.value() );
    QCOMPARE( qgsSettingsValue, static_cast<int>( Qgis::LayoutUnit::Meters ) );
    const bool success = settingsEntryEnumAsInteger.setValue( Qgis::LayoutUnit::Feet );
    QCOMPARE( success, true );
    qgsSettingsValue = QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), static_cast<int>( Qgis::LayoutUnit::Meters ) ).toInt();
    QCOMPARE( qgsSettingsValue, static_cast<int>( Qgis::LayoutUnit::Feet ) );
  }
}

void TestQgsSettingsEntry::flagValue()
{
  const QString settingsKey( u"my_flag_value_for_units"_s );
  const Qgis::LayerFilters pointAndLine = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::LineLayer );
  const Qgis::LayerFilters pointAndPolygon = Qgis::LayerFilters( Qgis::LayerFilter::PointLayer | Qgis::LayerFilter::PolygonLayer );
  const Qgis::LayerFilters hasGeometry = Qgis::LayerFilters( Qgis::LayerFilter::HasGeometry );

  // Make sure the setting is not existing
  QgsSettingsEntryBase::userSettings().remove( u"%1/%2"_s.arg( mSettingsSection, settingsKey ) );

  const QgsSettingsEntryEnumFlag settingsEntryFlag( settingsKey, mSettingsSection, Qgis::LayerFilters(), u"Filters"_s );

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
    QCOMPARE( settingsEntryFlag.value(), hasGeometry );
  }

  // Check get value
  QgsSettingsEntryBase::userSettings().setValue( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), u"PointLayer|LineLayer"_s );
  QCOMPARE( settingsEntryFlag.value(), pointAndLine );

  // Check settings type
  QCOMPARE( settingsEntryFlag.settingsType(), Qgis::SettingsType::EnumFlag );

  // check that value is stored as string
  QCOMPARE( settingsEntryFlag.valueAsVariant().toString(), u"PointLayer|LineLayer"_s );

  // auto conversion of old settings (int to str)
  QSettings().setValue( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), static_cast<int>( pointAndPolygon ) );
  QCOMPARE( settingsEntryFlag.valueAsVariant().toInt(), pointAndPolygon );
  QCOMPARE( settingsEntryFlag.value(), pointAndPolygon );
}

void TestQgsSettingsEntry::testFormerValue()
{
  const QString settingsKey( u"settingsEntryInteger/integer-former-value"_s );
  const QString settingsKeyFormer = settingsKey + ( u"_formervalue"_s );

  QgsSettingsEntryBase::userSettings().remove( u"%1/%2"_s.arg( mSettingsSection, settingsKey ) );
  QgsSettingsEntryBase::userSettings().remove( u"%1/%2"_s.arg( mSettingsSection, settingsKeyFormer ) );
  int defaultValue = 111;
  int defaultFormerValue = 222;

  QgsSettingsEntryInteger settingsEntryInteger = QgsSettingsEntryInteger( settingsKey, mSettingsSection, defaultValue, QString(), Qgis::SettingsOption::SaveFormerValue );

  QCOMPARE( settingsEntryInteger.formerValue(), defaultValue );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), defaultValue ), defaultValue );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ), defaultFormerValue );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), defaultValue ), 2 );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ), defaultFormerValue );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), defaultValue ), 2 );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ), defaultFormerValue );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 3 );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKey ), defaultValue ), 3 );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"%1/%2"_s.arg( mSettingsSection, settingsKeyFormer ), defaultFormerValue ).toLongLong(), 2 );
  QCOMPARE( settingsEntryInteger.formerValue(), 2 );

  settingsEntryInteger.setValue( 2 );
  QCOMPARE( settingsEntryInteger.formerValue(), 3 );
}

void TestQgsSettingsEntry::testChanged()
{
  const QString settingsKey( u"settingsEntryInteger/integer-value"_s );
  QgsSettingsEntryBase::userSettings().remove( u"%1/%2"_s.arg( mSettingsSection, settingsKey ) );
  int defaultValue = 111;

  QgsSettingsEntryBase::userSettings().setValue( u"testSetting"_s, 1 );

  QgsSettingsEntryInteger settingsEntryInteger = QgsSettingsEntryInteger( settingsKey, mSettingsSection, defaultValue );
  QVERIFY( !settingsEntryInteger.hasChanged() );
  settingsEntryInteger.copyValueToKeyIfChanged( u"testSetting"_s );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"testSetting"_s ).toInt(), 1 );

  settingsEntryInteger.setValue( 11111 );
  QVERIFY( settingsEntryInteger.hasChanged() );

  settingsEntryInteger.copyValueToKeyIfChanged( u"testSetting"_s );
  QCOMPARE( QgsSettingsEntryBase::userSettings().value( u"testSetting"_s ).toInt(), 11111 );
}


QGSTEST_MAIN( TestQgsSettingsEntry )
#include "testqgssettingsentry.moc"
