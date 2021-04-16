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
 * \ingroup UnitTests
 * This is a unit test for the QgsSettingsRegistry classes
 */
class TestQgsSettingsRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void getSettingsEntries();
    void getSettingsEntriesWithDynamicKeys();
};

void TestQgsSettingsRegistry::getSettingsEntries()
{
  QString settingsEntryBoolKey( "/qgis/testing/settingsEntryBool" );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, QgsSettings::NoSection, false );
  QString settingsEntryIntegerKey( "/qgis/testing/settingsEntryInteger" );
  QgsSettingsEntryBool settingsEntryInteger( settingsEntryIntegerKey, QgsSettings::NoSection, 123 );

  QString settingsEntryInexisting( "/qgis/testing/settingsEntryInexisting" );

  QgsSettingsRegistry settingsRegistry;
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSettingsEntry( &settingsEntryInteger );

  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryBoolKey ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryIntegerKey ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryInexisting ), nullptr );
}

void TestQgsSettingsRegistry::getSettingsEntriesWithDynamicKeys()
{
  QString settingsEntryBoolKey( "/qgis/testing/%1_settingsEntryBool" );
  QgsSettingsEntryBool settingsEntryBool( settingsEntryBoolKey, QgsSettings::NoSection, false );
  QString settingsEntryIntegerKey( "/qgis/testing/%1/settingsEntryInteger" );
  QgsSettingsEntryBool settingsEntryInteger( settingsEntryIntegerKey, QgsSettings::NoSection, 123 );
  QString settingsEntryDoubleKey( "/qgis/testing/%1/settingsEntryDouble_%2" );
  QgsSettingsEntryBool settingsEntryDouble( settingsEntryDoubleKey, QgsSettings::NoSection, 1.23 );

  QString settingsEntryInexisting( "/qgis/testing/settingsEntryInexisting%1" );

  QgsSettingsRegistry settingsRegistry;
  settingsRegistry.addSettingsEntry( &settingsEntryBool );
  settingsRegistry.addSettingsEntry( &settingsEntryInteger );
  settingsRegistry.addSettingsEntry( &settingsEntryDouble );

  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryBoolKey ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryBoolKey.replace( "%1", "1st" ) ), &settingsEntryBool );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryIntegerKey ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryIntegerKey.replace( "%1", "Second" ) ), &settingsEntryInteger );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryDoubleKey ), &settingsEntryDouble );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryDoubleKey.replace( "%1", "1st" ).replace( "%2", "2nd" ) ), &settingsEntryDouble );
  QCOMPARE( settingsRegistry.getSettingsEntry( settingsEntryInexisting ), nullptr );
}

QGSTEST_MAIN( TestQgsSettingsRegistry )
#include "testqgssettingsregistry.moc"
