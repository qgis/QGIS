/***************************************************************************
     testqgselevationprofilewidget.cpp
     --------------------------------------
    Date                 : February 2026
    Copyright            : (C) 2026 by Germán Carrillo
    Email                : german@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "elevation/qgselevationprofilewidget.h"
#include "qgisapp.h"
#include "qgsabstractprofilesource.h"
#include "qgselevationprofile.h"
#include "qgslayertree.h"
#include "qgsprofilesourceregistry.h"
#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

class MyDummyProfileSource : public QgsAbstractProfileSource
{
  public:
    virtual QgsAbstractProfileGenerator *createProfileGenerator( const QgsProfileRequest & ) override { return nullptr; }
    virtual QString profileSourceName() const override { return u"My Dummy Profile"_s; }
    virtual QString profileSourceId() const override { return u"my-dummy-profile"_s; }
};

/**
 * \ingroup UnitTests
 * This is a unit test for the QgisApp elevation profile widget.
 */
class TestQgsAppElevationProfileWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsAppElevationProfileWidget();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void registerCustomProfileAddsCustomNode();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsAppElevationProfileWidget::TestQgsAppElevationProfileWidget() = default;

//runs before all tests
void TestQgsAppElevationProfileWidget::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  qDebug() << "TestQgsAppElevationProfileWidget::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsAppElevationProfileWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAppElevationProfileWidget::registerCustomProfileAddsCustomNode()
{
  QgsElevationProfile *profile = new QgsElevationProfile( QgsProject::instance() );
  QgsElevationProfileWidget::applyDefaultSettingsToProfile( profile );
  QgsElevationProfileWidget *profileWidget = new QgsElevationProfileWidget( profile, mQgisApp->mapCanvas() );
  QVERIFY( profileWidget->profile() );
  QCOMPARE( profile->layerTree()->children().size(), 0 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 0 );

  // Register a custom profile and check that there is a custom node in the tree view
  MyDummyProfileSource *source = new MyDummyProfileSource();
  QVERIFY( QgsApplication::profileSourceRegistry()->registerProfileSource( source ) );
  QVERIFY( profile->layerTree() );
  QCOMPARE( profile->layerTree()->children().size(), 1 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 1 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().at( 0 ), "my-dummy-profile"_L1 );

  // Unregister the custom profile and check that the custom node was removed
  QVERIFY( QgsApplication::profileSourceRegistry()->unregisterProfileSource( u"my-dummy-profile"_s ) );
  QCOMPARE( profile->layerTree()->children().size(), 0 );
  QCOMPARE( profile->layerTree()->findCustomNodeIds().size(), 0 );
}

QGSTEST_MAIN( TestQgsAppElevationProfileWidget )
#include "testqgselevationprofilewidget.moc"
