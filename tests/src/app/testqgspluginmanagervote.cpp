/***************************************************************************
    testqgspluginmanagervote.cpp
    ---------------------------
    Date                 : August 2026
    Copyright            : (C) 2026 by Kevin Lo
    Email                : veeman961 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pluginmanager/qgspluginmanager.h"
#include "pluginmanager/qgspluginsortfilterproxymodel.h"
#include "qgsapplication.h"
#include "qgstest.h"

#include <QPushButton>
#include <QSlider>
#include <QStandardItem>

using namespace Qt::StringLiterals;

class TestQgsPluginManagerVote : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void voteRequiresUserInteraction();
};

void TestQgsPluginManagerVote::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsPluginManagerVote::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPluginManagerVote::voteRequiresUserInteraction()
{
  QgsPluginManager manager;
  manager.addPluginMetadata( u"test_plugin"_s, {
                                                     { u"plugin_id"_s, u"123"_s },
                                                     { u"average_vote"_s, u"4.5"_s },
                                                   } );

  QStandardItem item;
  item.setData( u"test_plugin"_s, PLUGIN_BASE_NAME_ROLE );
  manager.showPluginDetails( &item );

  QSlider *slider = manager.findChild<QSlider *>( u"voteSlider"_s );
  QPushButton *submit = manager.findChild<QPushButton *>( u"voteSubmit"_s );
  QVERIFY( slider );
  QVERIFY( submit );
  QCOMPARE_NE( slider->value(), 4 );
  QCOMPARE_NE( slider->value(), 5 );
  QVERIFY( !submit->isEnabled() );

  slider->setFocus();
  QTest::keyClick( slider, Qt::Key_Right );
  QVERIFY( submit->isEnabled() );

  manager.showPluginDetails( &item );
  QVERIFY( !submit->isEnabled() );
}

QGSTEST_MAIN( TestQgsPluginManagerVote )
#include "testqgspluginmanagervote.moc"
