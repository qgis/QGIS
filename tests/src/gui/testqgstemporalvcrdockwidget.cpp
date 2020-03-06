/***************************************************************************
                         testqgstemporalvcrdockwidget.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsprojecttimesettings.h"
#include "qgsgui.h"

#include "qgstemporalvcrdockwidget.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterLayerTemporalProperties class.
 */
class TestQgsTemporalVcrDockWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsTemporalVcrDockWidget() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testSettingDateTimes();

  private:
    // QgisApp *mQgisApp = nullptr;
    QgsTemporalVcrDockWidget *temporalVcrWidget = nullptr;
    QgsRasterLayer *layer = nullptr;
};

void TestQgsTemporalVcrDockWidget::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  // mQgisApp = new QgisApp();
  QgsApplication::showSettings();

}

void TestQgsTemporalVcrDockWidget::init()
{
//  temporalVcrWidget = new QgsTemporalVcrDockWidget();
//  mQgisApp->addDockWidget( Qt::BottomDockWidgetArea, temporalVcrWidget );

//  layer = new QgsRasterLayer( "", "test", "wms" );
//  QgsProject::instance()->addMapLayer( layer );
}

void TestQgsTemporalVcrDockWidget::cleanup()
{
}

void TestQgsTemporalVcrDockWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTemporalVcrDockWidget::testSettingDateTimes()
{
  QgsDateTimeRange range = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ) ),
                           QDateTime( QDate( 2020, 1, 31 ) ) );
  if ( QgsProject::instance()->timeSettings() )
    QgsProject::instance()->timeSettings()->setTemporalRange( range );

  //  QCOMPARE( temporalVcrWidget->dateTimes().size(), 31 );
}

QGSTEST_MAIN( TestQgsTemporalVcrDockWidget )
#include "testqgstemporalvcrdockwidget.moc"
