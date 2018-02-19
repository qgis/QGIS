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
#include "qgsunittypes.h"
#include "qgstest.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the operations on curve geometries
 */
class TestQgsSettings : public QObject
{
    Q_OBJECT

  private slots:
    void enumSettingValue();
};


void TestQgsSettings::enumSettingValue()
{
  QgsSettings settings;

  // assign to inexisting value
  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units" ), -1 );
  // just to be sure it really doesn't exist
  QVERIFY( static_cast<int>( QgsUnitTypes::LayoutMeters ) != -1 );

  // standard method returns invalid value
  int v1 = settings.value( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutMeters ).toInt();
  QCOMPARE( v1, -1 );

  // enum method returns default value if current setting is incorrect
  QgsUnitTypes::LayoutUnit v2 = settings.enumSettingValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutMeters );
  QCOMPARE( v2, QgsUnitTypes::LayoutMeters );

  settings.setValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutCentimeters );
  QgsUnitTypes::LayoutUnit v3 = settings.enumSettingValue( QStringLiteral( "qgis/testing/my_value_for_units" ), QgsUnitTypes::LayoutMeters );
  QCOMPARE( v3, QgsUnitTypes::LayoutCentimeters );
}


QGSTEST_MAIN( TestQgsSettings )
#include "testqgssettings.moc"
