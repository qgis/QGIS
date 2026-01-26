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
#include "qgssettingsentry.h"
#include "qgssettingsregistry.h"
#include "qgstest.h"

#include <QObject>

/**
 * This is a helper class to test protected methods of QgsSettingsRegistry
 */
Q_NOWARN_DEPRECATED_PUSH
class SettingsRegistryTest : public QgsSettingsRegistry
{
  public:
    void addSettingsEntry( const QgsSettingsEntryBase *settingsEntry )
    {
      QgsSettingsRegistry::addSettingsEntry( settingsEntry );
    }
};
Q_NOWARN_DEPRECATED_POP


/**
 * \ingroup UnitTests
 * This is a unit test for the QgsSettingsRegistry classes
 */
class TestQgsSettingsRegistry : public QObject
{
    Q_OBJECT

  private:
    const QString mSettingsSection = u"settingsEntryBool"_s;


  private slots:
    void getSettingsEntries();
    void getSettingsEntriesWithDynamicKeys();
    void childRegistry();
};

void TestQgsSettingsRegistry::getSettingsEntries()
{
  const QString settingsEntryBoolKey( u"settingsEntryBool"_s );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, mSettingsSection, false );
  const QString settingsEntryIntegerKey( u"settingsEntryInteger"_s );
  QgsSettingsEntryInteger settingsEntryInteger( settingsEntryIntegerKey, mSettingsSection, 123 );

  const QString settingsEntryInexisting( u"settingsEntryInexisting"_s );

  SettingsRegistryTest settingsRegistry;
  settingsRegistry.addSettingsEntry( nullptr ); // should not crash
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSettingsEntry( &settingsEntryInteger );

  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryBoolKey ) ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryIntegerKey ) ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryInexisting ) ), nullptr );
}

void TestQgsSettingsRegistry::getSettingsEntriesWithDynamicKeys()
{
  QString settingsEntryBoolKey( u"%1_settingsEntryBool"_s );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, mSettingsSection, false );
  QString settingsEntryIntegerKey( u"%1/settingsEntryInteger"_s );
  QgsSettingsEntryInteger settingsEntryInteger( settingsEntryIntegerKey, mSettingsSection, 123 );
  QString settingsEntryDoubleKey( u"%1/settingsEntryDouble_%2"_s );
  QgsSettingsEntryDouble settingsEntryDouble( settingsEntryDoubleKey, mSettingsSection, 1.23 );

  const QString settingsEntryInexisting( u"settingsEntryInexisting%1"_s );

  SettingsRegistryTest settingsRegistry;
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSettingsEntry( &settingsEntryInteger );
  settingsRegistry.addSettingsEntry( &settingsEntryDouble );

  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryBoolKey ) ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryBoolKey ).replace( "%1"_L1, "1st"_L1 ) ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryIntegerKey ) ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryIntegerKey ).replace( "%1"_L1, "Second"_L1 ) ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryDoubleKey ) ), &settingsEntryDouble );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryDoubleKey ).replace( "%1"_L1, "1st"_L1 ).replace( "%2"_L1, "2nd"_L1 ) ), &settingsEntryDouble );
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryInexisting ) ), nullptr );
}

void TestQgsSettingsRegistry::childRegistry()
{
  const QString settingsEntryBoolKey( u"settingsEntryBool"_s );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, mSettingsSection, false );
  const QString settingsEntryIntegerKey( u"settingsEntryInteger"_s );
  QgsSettingsEntryInteger settingsEntryInteger( settingsEntryIntegerKey, mSettingsSection, 123 );

  SettingsRegistryTest settingsRegistryChild;
  settingsRegistryChild.addSettingsEntry( &settingsEntryInteger );

  SettingsRegistryTest settingsRegistry;
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSubRegistry( nullptr ); // should not crash
  settingsRegistry.addSubRegistry( &settingsRegistryChild );

  // Search only in parent
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryIntegerKey ), false ), nullptr );

  // Search including child registries
  QCOMPARE( settingsRegistry.settingsEntry( u"%1/%2"_s.arg( mSettingsSection, settingsEntryIntegerKey ), true ), &settingsEntryInteger );
}

QGSTEST_MAIN( TestQgsSettingsRegistry )
#include "testqgssettingsregistry.moc"
