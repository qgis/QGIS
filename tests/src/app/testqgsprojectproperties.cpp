/***************************************************************************
     testqgsprojectproperties.cpp
     -------------------------
    Date                 : 2018-11-21
    Copyright            : (C) 2018 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsprojectproperties.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the project properties dialog
 */
class TestQgsProjectProperties : public QObject
{
    Q_OBJECT
  public:
    TestQgsProjectProperties();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testProjectPropertiesDirty();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsProjectProperties::TestQgsProjectProperties() = default;

//runs before all tests
void TestQgsProjectProperties::initTestCase()
{
  qDebug() << "TestQgsProjectProperties::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsProjectProperties::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProjectProperties::testProjectPropertiesDirty()
{
  // create a temporary layer
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "none?field=code:int&field=regular:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  // add layer to project, to insure presence of layer-related project settings
  QgsProject::instance()->addMapLayer( tempLayer.get() );

  // opening the project properties for the first time in a new project does write new entries
  // call apply twice here to test that subsequent opening will not dirty project
  QgsProjectProperties *pp = new QgsProjectProperties( mQgisApp->mapCanvas() );
  pp->apply();
  delete pp;
  QgsProject::instance()->setDirty( false );
  pp = new QgsProjectProperties( mQgisApp->mapCanvas() );
  pp->apply();
  delete pp;
  QCOMPARE( QgsProject::instance()->isDirty(), false );
}

QGSTEST_MAIN( TestQgsProjectProperties )

#include "testqgsprojectproperties.moc"
