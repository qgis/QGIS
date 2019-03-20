/***************************************************************************
     testqgswmsparameters.cpp
     --------------------------------------
    Date                 : 20 Mar 2019
    Copyright            : (C) 2019 by Paul Blottiere
    Email                : paul dot blottiere @ oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgswmsparameters.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS parameters parsing
 */
class TestQgsWmsParameters : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    // fake
    void dxfOptions();
};

void TestQgsWmsParameters::initTestCase()
{
}

void TestQgsWmsParameters::cleanupTestCase()
{
}

void TestQgsWmsParameters::dxfOptions()
{
  const QString key = "FORMAT_OPTIONS";
  const QString value = "MODE:SYMBOLLAYERSYMBOLOGY;SCALE:250;USE_TITLE_AS_LAYERNAME:TRUE;LAYERATTRIBUTES:pif,paf,pouf";

  QUrlQuery query;
  query.addQueryItem( key, value );

  QgsWms::QgsWmsParameters parameters( query );

  QCOMPARE( parameters.dxfScale(), 250 );
  QCOMPARE( parameters.dxfUseLayerTitleAsName(), true );
  QCOMPARE( parameters.dxfMode(), QgsDxfExport::SymbolLayerSymbology );
  QCOMPARE( parameters.dxfLayerAttributes().size(), 3 );
  QCOMPARE( parameters.dxfLayerAttributes()[0], "pif" );
  QCOMPARE( parameters.dxfLayerAttributes()[1], "paf" );
  QCOMPARE( parameters.dxfLayerAttributes()[2], "pouf" );
}

QGSTEST_MAIN( TestQgsWmsParameters )
#include "testqgswmsparameters.moc"
