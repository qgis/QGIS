/***************************************************************************
     testqgsapplayoutvaliditychecks.cpp
     --------------------------------------
    Date                 : January 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QStringList>

//qgis includes...
#include "qgsdataitem.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgslayout.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitempicture.h"
#include "qgsabstractvaliditycheck.h"
#include "qgsvaliditycheckcontext.h"
#include "layout/qgslayoutvaliditychecks.h"
#include "qgsfeedback.h"

class TestQgsLayoutValidityChecks : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutValidityChecks();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testScaleBarValidity();
    void testNorthArrowValidity();
    void testOverviewValidity();
    void testPictureValidity();

  private:
    QString mTestDataDir;
};

TestQgsLayoutValidityChecks::TestQgsLayoutValidityChecks() = default;

void TestQgsLayoutValidityChecks::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = dataDir + '/';
}

void TestQgsLayoutValidityChecks::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutValidityChecks::testScaleBarValidity()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutItemScaleBar *scale = new QgsLayoutItemScaleBar( &l );
  l.addItem( scale );

  QgsLayoutValidityCheckContext context( &l );
  QgsFeedback f;

  // scalebar not linked to map
  QgsLayoutScaleBarValidityCheck check;
  QVERIFY( check.prepareCheck( &context, &f ) );
  QList< QgsValidityCheckResult > res = check.runCheck( &context, &f );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );

  // now link a map
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  l.addItem( map );
  scale->setLinkedMap( map );

  QgsLayoutScaleBarValidityCheck check2;
  QVERIFY( check2.prepareCheck( &context, &f ) );
  res = check2.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );
}

void TestQgsLayoutValidityChecks::testNorthArrowValidity()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutItemPicture *picture = new QgsLayoutItemPicture( &l );
  // we identify this as a north arrow based on the default picture path pointing to the default north arrow
  picture->setPicturePath( QStringLiteral( ":/images/north_arrows/layout_default_north_arrow.svg" ) );

  l.addItem( picture );

  QgsLayoutValidityCheckContext context( &l );
  QgsFeedback f;

  // scalebar not linked to map
  QgsLayoutNorthArrowValidityCheck check;
  QVERIFY( check.prepareCheck( &context, &f ) );
  QList< QgsValidityCheckResult > res = check.runCheck( &context, &f );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );

  // now link a map
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  l.addItem( map );
  picture->setLinkedMap( map );

  QgsLayoutNorthArrowValidityCheck check2;
  QVERIFY( check2.prepareCheck( &context, &f ) );
  res = check2.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );

  // test with ID check
  picture->setPicturePath( QStringLiteral( "a" ) );
  picture->setId( QStringLiteral( "north arrow 2" ) );
  picture->setLinkedMap( nullptr );

  QgsLayoutNorthArrowValidityCheck check3;
  QVERIFY( check3.prepareCheck( &context, &f ) );
  res = check3.runCheck( &context, &f );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );

  // no longer looks like a north arrow
  picture->setId( QStringLiteral( "a" ) );
  QgsLayoutNorthArrowValidityCheck check4;
  QVERIFY( check4.prepareCheck( &context, &f ) );
  res = check4.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );
}

void TestQgsLayoutValidityChecks::testOverviewValidity()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutItemMap *map1 = new QgsLayoutItemMap( &l );
  l.addItem( map1 );

  QgsLayoutValidityCheckContext context( &l );
  QgsFeedback f;

  // no overviews
  QgsLayoutOverviewValidityCheck check;
  QVERIFY( check.prepareCheck( &context, &f ) );
  QList< QgsValidityCheckResult > res = check.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );

  // overview not linked to map
  map1->overviews()->addOverview( new QgsLayoutItemMapOverview( QStringLiteral( "blah" ), map1 ) );
  QgsLayoutOverviewValidityCheck check2;
  QVERIFY( check2.prepareCheck( &context, &f ) );
  res = check2.runCheck( &context, &f );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );
  map1->overview()->setEnabled( false );
  QgsLayoutOverviewValidityCheck check3;
  QVERIFY( check3.prepareCheck( &context, &f ) );
  res = check3.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );

  // now link a map
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  l.addItem( map2 );
  map1->overview()->setLinkedMap( map2 );
  map1->overview()->setEnabled( true );

  QgsLayoutOverviewValidityCheck check4;
  QVERIFY( check4.prepareCheck( &context, &f ) );
  res = check4.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );

  map1->overviews()->addOverview( new QgsLayoutItemMapOverview( QStringLiteral( "blah2" ), map1 ) );
  QgsLayoutOverviewValidityCheck check5;
  QVERIFY( check5.prepareCheck( &context, &f ) );
  res = check5.runCheck( &context, &f );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );

  map1->overviews()->addOverview( new QgsLayoutItemMapOverview( QStringLiteral( "blah3" ), map1 ) );
  QgsLayoutOverviewValidityCheck check6;
  QVERIFY( check6.prepareCheck( &context, &f ) );
  res = check6.runCheck( &context, &f );
  QCOMPARE( res.size(), 2 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );
  QCOMPARE( res.at( 1 ).type, QgsValidityCheckResult::Warning );
}

void TestQgsLayoutValidityChecks::testPictureValidity()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutItemPicture *picture = new QgsLayoutItemPicture( &l );
  l.addItem( picture );

  QgsLayoutValidityCheckContext context( &l );
  QgsFeedback f;

  // invalid picture source
  picture->setPicturePath( QStringLiteral( "blaaaaaaaaaaaaaaaaah" ) );
  QgsLayoutPictureSourceValidityCheck check;
  QVERIFY( check.prepareCheck( &context, &f ) );
  QList< QgsValidityCheckResult > res = check.runCheck( &context, &f );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );

  QgsLayoutPictureSourceValidityCheck check2;
  picture->setPicturePath( QString() );
  QVERIFY( check2.prepareCheck( &context, &f ) );
  res = check2.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );

  QgsLayoutPictureSourceValidityCheck check3;
  picture->setPicturePath( QStringLiteral( TEST_DATA_DIR ) + "/sample_svg.svg" );
  QVERIFY( check3.prepareCheck( &context, &f ) );
  res = check3.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );

  QgsLayoutItemPicture *picture2 = new QgsLayoutItemPicture( &l );
  l.addItem( picture2 );
  picture2->dataDefinedProperties().setProperty( QgsLayoutObject::PictureSource, QgsProperty::fromExpression( QStringLiteral( "'d:/bad' || 'robot'" ) ) );
  l.refresh();

  QgsLayoutPictureSourceValidityCheck check4;
  QVERIFY( check4.prepareCheck( &context, &f ) );
  res = check4.runCheck( &context, &f );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).type, QgsValidityCheckResult::Warning );

  picture2->dataDefinedProperties().setProperty( QgsLayoutObject::PictureSource, QgsProperty::fromExpression( QStringLiteral( "''" ) ) );
  l.refresh();
  QgsLayoutPictureSourceValidityCheck check5;
  QVERIFY( check5.prepareCheck( &context, &f ) );
  res = check5.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );

  picture2->dataDefinedProperties().setProperty( QgsLayoutObject::PictureSource, QgsProperty::fromExpression( QStringLiteral( "'%1'" ).arg( QStringLiteral( TEST_DATA_DIR ) + "/sam' || 'ple_svg.svg" ) ) );
  l.refresh();
  QgsLayoutPictureSourceValidityCheck check6;
  QVERIFY( check6.prepareCheck( &context, &f ) );
  res = check6.runCheck( &context, &f );
  QCOMPARE( res.size(), 0 );
}



QGSTEST_MAIN( TestQgsLayoutValidityChecks )
#include "testqgsapplayoutvaliditychecks.moc"
