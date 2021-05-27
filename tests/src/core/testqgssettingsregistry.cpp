/***************************************************************************
     testqgssettingregistry.cpp
     --------------------------------------
    Date                 : 08.04.2021
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

#include "qgssettingsentry.h"
#include "qgssettingsregistry.h"
#include "qgsunittypes.h"
#include "qgsmaplayerproxymodel.h"
#include "qgstest.h"

/**
 * This is a helper class to test protected methods of QgsSettingsRegistry
 */
class SettingsRegistryTest : public QgsSettingsRegistry
{
  public:

    void addSettingsEntry( const QgsSettingsEntryBase *settingsEntry )
    {
      QgsSettingsRegistry::addSettingsEntry( settingsEntry );
    }
};

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsSettingsRegistry classes
 */
class TestQgsSettingsRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void getSettingsEntries();
    void getSettingsEntriesWithDynamicKeys();
    void childRegistry();
};

void TestQgsSettingsRegistry::getSettingsEntries()
{
  QString settingsEntryBoolKey( QStringLiteral( "/qgis/testing/settingsEntryBool" ) );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, QgsSettings::NoSection, false );
  QString settingsEntryIntegerKey( QStringLiteral( "/qgis/testing/settingsEntryInteger" ) );
  QgsSettingsEntryBool settingsEntryInteger( settingsEntryIntegerKey, QgsSettings::NoSection, 123 );

  QString settingsEntryInexisting( QStringLiteral( "/qgis/testing/settingsEntryInexisting" ) );

  SettingsRegistryTest settingsRegistry;
  settingsRegistry.addSettingsEntry( nullptr ); // should not crash
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSettingsEntry( &settingsEntryInteger );

  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryBoolKey ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryIntegerKey ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryInexisting ), nullptr );
}

void TestQgsSettingsRegistry::getSettingsEntriesWithDynamicKeys()
{
  QString settingsEntryBoolKey( QStringLiteral( "/qgis/testing/%1_settingsEntryBool" ) );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, QgsSettings::NoSection, false );
  QString settingsEntryIntegerKey( QStringLiteral( "/qgis/testing/%1/settingsEntryInteger" ) );
  QgsSettingsEntryInteger settingsEntryInteger( settingsEntryIntegerKey, QgsSettings::NoSection, 123 );
  QString settingsEntryDoubleKey( QStringLiteral( "/qgis/testing/%1/settingsEntryDouble_%2" ) );
  QgsSettingsEntryDouble settingsEntryDouble( settingsEntryDoubleKey, QgsSettings::NoSection, 1.23 );

  QString settingsEntryInexisting( QStringLiteral( "/qgis/testing/settingsEntryInexisting%1" ) );

  SettingsRegistryTest settingsRegistry;
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSettingsEntry( &settingsEntryInteger );
  settingsRegistry.addSettingsEntry( &settingsEntryDouble );

  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryBoolKey ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryBoolKey.replace( QLatin1String( "%1" ), QLatin1String( "1st" ) ) ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryIntegerKey ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryIntegerKey.replace( QLatin1String( "%1" ), QLatin1String( "Second" ) ) ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryDoubleKey ), &settingsEntryDouble );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryDoubleKey.replace( QLatin1String( "%1" ), QLatin1String( "1st" ) ).replace( QLatin1String( "%2" ), QLatin1String( "2nd" ) ) ), &settingsEntryDouble );
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryInexisting ), nullptr );
}

void TestQgsSettingsRegistry::childRegistry()
{
  QString settingsEntryBoolKey( QStringLiteral( "/qgis/testing/settingsEntryBool" ) );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, QgsSettings::NoSection, false );
  QString settingsEntryIntegerKey( QStringLiteral( "/qgis/testing/settingsEntryInteger" ) );
  QgsSettingsEntryInteger settingsEntryInteger( settingsEntryIntegerKey, QgsSettings::NoSection, 123 );

  SettingsRegistryTest settingsRegistryChild;
  settingsRegistryChild.addSettingsEntry( &settingsEntryInteger );

  SettingsRegistryTest settingsRegistry;
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSubRegistry( nullptr ); // should not crash
  settingsRegistry.addSubRegistry( &settingsRegistryChild );

  // Search only in parent
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryIntegerKey, false ), nullptr );

  // Search including child registries
  QCOMPARE( settingsRegistry.settingsEntry( settingsEntryIntegerKey, true ), &settingsEntryInteger );
}

QGSTEST_MAIN( TestQgsSettingsRegistry )
#include "testqgssettingsregistry.moc"
