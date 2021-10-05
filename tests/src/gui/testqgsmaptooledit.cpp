/***************************************************************************
    testqgsmaptooledit.cpp
     --------------------------------------
    Date                 : 6.2.2017
    Copyright            : (C) 2017 Alexander Lisovenko
    Email                : alexander.lisovenko@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QCoreApplication>

#include "qgstest.h"
#include "qgsguiutils.h"
#include "qgsmaptooledit.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"

class TestQgsMapToolEdit : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolEdit() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void checkDefaultZValue();
    void checkDefaultMValue();
    void checkLayers();

  private:
    QgsMapCanvas *mCanvas = nullptr;

};

void TestQgsMapToolEdit::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsMapToolEdit::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolEdit::init()
{
  mCanvas = new QgsMapCanvas();
}

void TestQgsMapToolEdit::cleanup()
{
  delete mCanvas;
}

void TestQgsMapToolEdit::checkDefaultZValue()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.remove();

  QgsMapToolEdit *tool = new QgsMapToolEdit( mCanvas );
  QCOMPARE( tool->defaultZValue(), Qgis::DEFAULT_Z_COORDINATE );

  const double z_value_for_test = Qgis::DEFAULT_Z_COORDINATE + 1;
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( z_value_for_test );

  QCOMPARE( tool->defaultZValue(), z_value_for_test );
}

void TestQgsMapToolEdit::checkDefaultMValue()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "/qgis/digitizing/default_m_value" ) );

  QgsMapToolEdit *tool = new QgsMapToolEdit( mCanvas );
  QCOMPARE( tool->defaultMValue(), Qgis::DEFAULT_M_COORDINATE );

  const double m_value_for_test = Qgis::DEFAULT_M_COORDINATE + 1;
  settings.setValue( QStringLiteral( "/qgis/digitizing/default_m_value" ), m_value_for_test );

  QCOMPARE( tool->defaultMValue(), m_value_for_test );
}

void TestQgsMapToolEdit::checkLayers()
{
  QgsProject::instance()->clear();
  //set up canvas with a mix of project and non-project layers
  QgsVectorLayer *vl1 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=halig:string&field=valig:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QVERIFY( vl1->isValid() );
  QgsProject::instance()->addMapLayer( vl1 );

  std::unique_ptr< QgsVectorLayer > vl2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:3946&field=halig:string&field=valig:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( vl2->isValid() );

  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  canvas->setLayers( { vl1, vl2.get() } );

  std::unique_ptr< QgsMapToolEdit > tool = std::make_unique< QgsMapToolEdit >( canvas.get() );

  // retrieving layer by id should work for both layers from the project AND for freestanding layers
  QCOMPARE( tool->layer( vl1->id() ), vl1 );
  QCOMPARE( tool->layer( vl2->id() ), vl2.get() );
  QCOMPARE( tool->layer( QStringLiteral( "xxx" ) ), nullptr );
}

QGSTEST_MAIN( TestQgsMapToolEdit )
#include "testqgsmaptooledit.moc"
