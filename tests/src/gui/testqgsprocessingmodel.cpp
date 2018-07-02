/***************************************************************************
                         testqgsprocessing.cpp
                         ---------------------
    begin                : January 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingregistry.h"
#include "qgsprocessingtoolboxmodel.h"
#include <QtTest/QSignalSpy>
#include "qgstest.h"
class DummyAlgorithm : public QgsProcessingAlgorithm
{
  public:

    DummyAlgorithm( const QString &name, const QString &group,
                    QgsProcessingAlgorithm::Flags flags = 0 )
      : mName( name )
      , mGroup( group )
      , mFlags( flags )
    {}

    void initAlgorithm( const QVariantMap & = QVariantMap() ) override {}
    QgsProcessingAlgorithm::Flags flags() const override { return mFlags; }
    QString name() const override { return mName; }
    QString displayName() const override { return mName; }
    QString group() const override { return mGroup; }
    QString groupId() const override { return mGroup; }
    QVariantMap processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) override { return QVariantMap(); }

    DummyAlgorithm *createInstance() const override { return new DummyAlgorithm( mName, mGroup, mFlags ); }

    QString mName;
    QString mGroup;
    QgsProcessingAlgorithm::Flags mFlags = 0;

};
//dummy provider for testing
class DummyProvider : public QgsProcessingProvider
{
  public:

    DummyProvider( const QString &id, const QString &name, const QList< QgsProcessingAlgorithm *> algs = QList< QgsProcessingAlgorithm *>() )
      : mId( id )
      , mName( name )
      , mAlgs( algs )
    {

    }
    ~DummyProvider() override
    {
      qDeleteAll( mAlgs );
    }

    QString id() const override { return mId; }
    bool isActive() const override { return mActive; }

    QString name() const override { return mName; }
    QString longName() const override { return QStringLiteral( "long name %1" ).arg( mName );}
    bool mActive = true;
  protected:
    void loadAlgorithms() override
    {
      for ( QgsProcessingAlgorithm *alg : mAlgs )
        addAlgorithm( alg->create() );
    }
    QString mId;
    QString mName;

    QList< QgsProcessingAlgorithm *> mAlgs;

};


class TestQgsProcessingModel: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testModel();
    void testProxyModel();
};


void TestQgsProcessingModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
}

void TestQgsProcessingModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingModel::testModel()
{
  QgsProcessingRegistry registry;
  QgsProcessingToolboxModel model( nullptr, &registry );

  QCOMPARE( model.columnCount(), 1 );
  QCOMPARE( model.rowCount(), 0 );
  QVERIFY( !model.hasChildren() );
  QVERIFY( !model.providerForIndex( model.index( 0, 0, QModelIndex() ) ) );
  QVERIFY( !model.providerForIndex( model.index( 1, 0, QModelIndex() ) ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );

  // add a provider
  DummyProvider *p1 = new DummyProvider( "p1", "provider1" );
  registry.addProvider( p1 );
  QCOMPARE( model.rowCount(), 1 );
  QVERIFY( model.hasChildren() );

  QVERIFY( model.index( 0, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.providerForIndex( model.index( 0, 0, QModelIndex() ) ), p1 );
  QVERIFY( !model.providerForIndex( model.index( 1, 0, QModelIndex() ) ) );
  QCOMPARE( model.indexForProvider( p1->id() ), model.index( 0, 0, QModelIndex() ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );
  QCOMPARE( model.rowCount( model.index( 0, 0, QModelIndex() ) ), 0 );
  QVERIFY( !model.hasChildren( model.index( 0, 0, QModelIndex() ) ) );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), QStringLiteral( "long name provider1" ) );
  QVERIFY( !model.data( model.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).isValid() );
  QVERIFY( !model.data( model.index( 1, 0, QModelIndex() ), Qt::ToolTipRole ).isValid() );

  // second provider
  DummyProvider *p2 = new DummyProvider( "p2", "provider2" );
  registry.addProvider( p2 );
  QCOMPARE( model.rowCount(), 2 );
  QVERIFY( model.hasChildren() );

  QVERIFY( model.index( 1, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.providerForIndex( model.index( 0, 0, QModelIndex() ) ), p1 );
  QCOMPARE( model.providerForIndex( model.index( 1, 0, QModelIndex() ) ), p2 );
  QVERIFY( !model.providerForIndex( model.index( 2, 0, QModelIndex() ) ) );
  QCOMPARE( model.indexForProvider( p1->id() ), model.index( 0, 0, QModelIndex() ) );
  QCOMPARE( model.indexForProvider( p2->id() ), model.index( 1, 0, QModelIndex() ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );
  QVERIFY( !model.hasChildren( model.index( 1, 0, QModelIndex() ) ) );

  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), QStringLiteral( "long name provider1" ) );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), QStringLiteral( "long name provider2" ) );
  QVERIFY( !model.data( model.index( 2, 0, QModelIndex() ), Qt::DisplayRole ).isValid() );
  QVERIFY( !model.data( model.index( 2, 0, QModelIndex() ), Qt::ToolTipRole ).isValid() );

  // provider with algs and groups
  DummyAlgorithm *a1 = new DummyAlgorithm( "a1", "group1", QgsProcessingAlgorithm::FlagHideFromModeler );
  DummyAlgorithm *a2 = new DummyAlgorithm( "a2", "group2", QgsProcessingAlgorithm::FlagHideFromToolbox );
  DummyProvider *p3 = new DummyProvider( "p3", "provider3", QList< QgsProcessingAlgorithm * >() << a1 << a2 );
  registry.addProvider( p3 );

  QCOMPARE( model.rowCount(), 3 );
  QVERIFY( model.hasChildren() );
  QVERIFY( model.index( 2, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.providerForIndex( model.index( 2, 0, QModelIndex() ) ), p3 );
  QCOMPARE( model.indexForProvider( p1->id() ), model.index( 0, 0, QModelIndex() ) );
  QCOMPARE( model.indexForProvider( p2->id() ), model.index( 1, 0, QModelIndex() ) );
  QCOMPARE( model.indexForProvider( p3->id() ), model.index( 2, 0, QModelIndex() ) );
  QCOMPARE( model.rowCount( model.index( 1, 0, QModelIndex() ) ), 0 );
  QCOMPARE( model.rowCount( model.index( 2, 0, QModelIndex() ) ), 2 );
  QVERIFY( !model.hasChildren( model.index( 1, 0, QModelIndex() ) ) );
  QVERIFY( model.hasChildren( model.index( 2, 0, QModelIndex() ) ) );
  QModelIndex providerIndex = model.index( 2, 0, QModelIndex() );
  QVERIFY( !model.providerForIndex( model.index( 0, 0, providerIndex ) ) );
  QVERIFY( !model.providerForIndex( model.index( 1, 0, providerIndex ) ) );

  QCOMPARE( model.data( model.index( 0, 0, providerIndex ), Qt::DisplayRole ).toString(), QStringLiteral( "group1" ) );
  QCOMPARE( model.data( model.index( 0, 0, providerIndex ), Qt::ToolTipRole ).toString(), QStringLiteral( "group1" ) );
  QCOMPARE( model.data( model.index( 1, 0, providerIndex ), Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.data( model.index( 1, 0, providerIndex ), Qt::ToolTipRole ).toString(), QStringLiteral( "group2" ) );

  QModelIndex group1Index = model.index( 0, 0, providerIndex );
  QVERIFY( !model.providerForIndex( group1Index ) );
  QVERIFY( !model.algorithmForIndex( group1Index ) );
  QCOMPARE( model.rowCount( group1Index ), 1 );
  QVERIFY( model.hasChildren( group1Index ) );
  QModelIndex alg1Index = model.index( 0, 0, group1Index );
  QVERIFY( !model.providerForIndex( alg1Index ) );
  QCOMPARE( model.data( alg1Index, Qt::DisplayRole ).toString(), QStringLiteral( "a1" ) );
  QCOMPARE( model.data( alg1Index, Qt::ToolTipRole ).toString(), QStringLiteral( "<p><b>a1</b></p><p>Algorithm ID: \u2018<i>p3:a1</i>\u2019</p>" ) );
  QCOMPARE( model.data( alg1Index, QgsProcessingToolboxModel::RoleAlgorithmFlags ).toInt(), static_cast< int >( QgsProcessingAlgorithm::FlagHideFromModeler ) );
  QCOMPARE( model.algorithmForIndex( alg1Index )->id(), QStringLiteral( "p3:a1" ) );

  QModelIndex group2Index = model.index( 1, 0, providerIndex );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QVERIFY( model.hasChildren( group2Index ) );
  QModelIndex alg2Index = model.index( 0, 0, group2Index );
  QCOMPARE( model.data( alg2Index, Qt::DisplayRole ).toString(), QStringLiteral( "a2" ) );
  QCOMPARE( model.data( alg2Index, Qt::ToolTipRole ).toString(), QStringLiteral( "<p><b>a2</b></p><p>Algorithm ID: \u2018<i>p3:a2</i>\u2019</p>" ) );
  QCOMPARE( model.data( alg2Index, QgsProcessingToolboxModel::RoleAlgorithmFlags ).toInt(), static_cast< int >( QgsProcessingAlgorithm::FlagHideFromToolbox ) );
  QCOMPARE( model.algorithmForIndex( alg2Index )->id(), QStringLiteral( "p3:a2" ) );

  // combined groups
  DummyAlgorithm *a3 = new DummyAlgorithm( "a3", "group1" );
  DummyAlgorithm *a4 = new DummyAlgorithm( "a4", "group1" );
  DummyProvider *p4 = new DummyProvider( "p4", "provider4", QList< QgsProcessingAlgorithm * >() << a3 << a4 );
  registry.addProvider( p4 );
  QModelIndex p4ProviderIndex = model.indexForProvider( p4->id() );
  QModelIndex groupIndex = model.index( 0, 0, p4ProviderIndex );
  QCOMPARE( model.rowCount( groupIndex ), 2 );
  QCOMPARE( model.algorithmForIndex( model.index( 0, 0, groupIndex ) )->id(), QStringLiteral( "p4:a3" ) );
  QCOMPARE( model.algorithmForIndex( model.index( 1, 0, groupIndex ) )->id(), QStringLiteral( "p4:a4" ) );

  // provider with algs with no groups
  DummyAlgorithm *a5 = new DummyAlgorithm( "a5", "group1" );
  DummyAlgorithm *a6 = new DummyAlgorithm( "a6", QString() );
  DummyAlgorithm *a7 = new DummyAlgorithm( "a7", "group2" );
  DummyProvider *p5 = new DummyProvider( "p5", "provider5", QList< QgsProcessingAlgorithm * >() << a5 << a6 << a7 );
  registry.addProvider( p5 );
  QCOMPARE( model.rowCount(), 5 );
  QModelIndex p5ProviderIndex = model.indexForProvider( p5->id() );
  QCOMPARE( model.rowCount( p5ProviderIndex ), 3 );

  groupIndex = model.index( 0, 0, p5ProviderIndex );
  QCOMPARE( model.data( groupIndex, Qt::DisplayRole ).toString(), QStringLiteral( "group1" ) );
  QCOMPARE( model.rowCount( groupIndex ), 1 );
  QCOMPARE( model.algorithmForIndex( model.index( 0, 0, groupIndex ) )->id(), QStringLiteral( "p5:a5" ) );

  QCOMPARE( model.algorithmForIndex( model.index( 1, 0, p5ProviderIndex ) )->id(), QStringLiteral( "p5:a6" ) );

  groupIndex = model.index( 2, 0, p5ProviderIndex );
  QCOMPARE( model.data( groupIndex, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( groupIndex ), 1 );
  QCOMPARE( model.algorithmForIndex( model.index( 0, 0, groupIndex ) )->id(), QStringLiteral( "p5:a7" ) );

  // reload provider
  p5->refreshAlgorithms();
  QCOMPARE( model.rowCount(), 5 );
  p5ProviderIndex = model.indexForProvider( p5->id() );
  QCOMPARE( model.rowCount( p5ProviderIndex ), 3 );

  groupIndex = model.index( 0, 0, p5ProviderIndex );
  QCOMPARE( model.data( groupIndex, Qt::DisplayRole ).toString(), QStringLiteral( "group1" ) );
  QCOMPARE( model.rowCount( groupIndex ), 1 );
  QCOMPARE( model.algorithmForIndex( model.index( 0, 0, groupIndex ) )->id(), QStringLiteral( "p5:a5" ) );
  QCOMPARE( model.algorithmForIndex( model.index( 1, 0, p5ProviderIndex ) )->id(), QStringLiteral( "p5:a6" ) );
  groupIndex = model.index( 2, 0, p5ProviderIndex );
  QCOMPARE( model.data( groupIndex, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( groupIndex ), 1 );
  QCOMPARE( model.algorithmForIndex( model.index( 0, 0, groupIndex ) )->id(), QStringLiteral( "p5:a7" ) );

  p3->refreshAlgorithms();
  providerIndex = model.indexForProvider( p3->id() );
  group1Index = model.index( 0, 0, providerIndex );
  QCOMPARE( model.rowCount( group1Index ), 1 );
  alg1Index = model.index( 0, 0, group1Index );
  QCOMPARE( model.algorithmForIndex( alg1Index )->id(), QStringLiteral( "p3:a1" ) );
  group2Index = model.index( 1, 0, providerIndex );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  alg2Index = model.index( 0, 0, group2Index );
  QCOMPARE( model.algorithmForIndex( alg2Index )->id(), QStringLiteral( "p3:a2" ) );

  // remove a provider
  registry.removeProvider( p1 );
  QCOMPARE( model.rowCount(), 4 );
  QVERIFY( model.index( 0, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.providerForIndex( model.index( 0, 0, QModelIndex() ) ), p2 );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  registry.removeProvider( p5 );
  QCOMPARE( model.rowCount(), 3 );
  registry.removeProvider( p2 );
  QCOMPARE( model.rowCount(), 2 );
  registry.removeProvider( p3 );
  QCOMPARE( model.rowCount(), 1 );
  registry.removeProvider( p4 );
  QCOMPARE( model.rowCount(), 0 );
  QCOMPARE( model.columnCount(), 1 );
  QVERIFY( !model.hasChildren() );
  QVERIFY( !model.providerForIndex( model.index( 0, 0, QModelIndex() ) ) );
  QVERIFY( !model.providerForIndex( model.index( 1, 0, QModelIndex() ) ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );

  // qgis native algorithms put groups at top level
  QgsProcessingRegistry registry2;
  QgsProcessingToolboxModel model2( nullptr, &registry2 );
  DummyAlgorithm *qgisA1 = new DummyAlgorithm( "a1", "group1" );
  DummyAlgorithm *qgisA2 = new DummyAlgorithm( "a2", "group2" );
  DummyAlgorithm *qgisA3 = new DummyAlgorithm( "a3", "group1" );
  DummyAlgorithm *qgisA4 = new DummyAlgorithm( "a4", "group3" );
  DummyProvider *qgisP = new DummyProvider( "qgis", "qgis_provider", QList< QgsProcessingAlgorithm * >() << qgisA1 << qgisA2 << qgisA3 << qgisA4 );
  registry2.addProvider( qgisP );

  QCOMPARE( model2.rowCount(), 3 );
  group1Index = model2.index( 0, 0 );
  group2Index = model2.index( 1, 0 );
  QModelIndex group3Index = model2.index( 2, 0 );
  QCOMPARE( model2.data( group1Index, Qt::DisplayRole ).toString(), QStringLiteral( "group1" ) );
  QCOMPARE( model2.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model2.data( group3Index, Qt::DisplayRole ).toString(), QStringLiteral( "group3" ) );

  QCOMPARE( model2.rowCount( group1Index ), 2 );
  QCOMPARE( model2.algorithmForIndex( model2.index( 0, 0, group1Index ) )->id(), QStringLiteral( "qgis:a1" ) );
  QCOMPARE( model2.algorithmForIndex( model2.index( 1, 0, group1Index ) )->id(), QStringLiteral( "qgis:a3" ) );
  QCOMPARE( model2.rowCount( group2Index ), 1 );
  QCOMPARE( model2.algorithmForIndex( model2.index( 0, 0, group2Index ) )->id(), QStringLiteral( "qgis:a2" ) );
  QCOMPARE( model2.rowCount( group3Index ), 1 );
  QCOMPARE( model2.algorithmForIndex( model2.index( 0, 0, group3Index ) )->id(), QStringLiteral( "qgis:a4" ) );
}

void TestQgsProcessingModel::testProxyModel()
{
  QgsProcessingRegistry registry;
  QgsProcessingToolboxProxyModel model( nullptr, &registry );

  // add a provider
  DummyAlgorithm *a1 = new DummyAlgorithm( "a1", "group2", QgsProcessingAlgorithm::FlagHideFromModeler );
  DummyProvider *p1 = new DummyProvider( "p2", "provider2", QList< QgsProcessingAlgorithm * >() << a1 );
  registry.addProvider( p1 );
  // second provider
  DummyAlgorithm *a2 = new DummyAlgorithm( "a2", "group2", QgsProcessingAlgorithm::FlagHideFromModeler );
  DummyProvider *p2 = new DummyProvider( "p1", "provider1", QList< QgsProcessingAlgorithm * >() << a2 );
  registry.addProvider( p2 );

  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );

  // top level groups come first
  DummyAlgorithm *qgisA1 = new DummyAlgorithm( "a1", "group2", QgsProcessingAlgorithm::FlagHideFromModeler );
  DummyAlgorithm *qgisA2 = new DummyAlgorithm( "a2", "group1", QgsProcessingAlgorithm::FlagHideFromToolbox );
  DummyProvider *qgisP = new DummyProvider( "qgis", "qgis_provider", QList< QgsProcessingAlgorithm * >() << qgisA1 << qgisA2 );
  registry.addProvider( qgisP );

  QModelIndex group1Index = model.index( 0, 0, QModelIndex() );
  QModelIndex group2Index = model.index( 1, 0, QModelIndex() );
  QCOMPARE( model.data( group1Index, Qt::DisplayRole ).toString(), QStringLiteral( "group1" ) );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  QCOMPARE( model.rowCount(), 4 );

  QModelIndex alg1Index = model.index( 0, 0, group2Index );
  QCOMPARE( model.data( alg1Index, Qt::DisplayRole ).toString(), QStringLiteral( "a1" ) );
  QModelIndex alg2Index = model.index( 0, 0, group1Index );
  QCOMPARE( model.data( alg2Index, Qt::DisplayRole ).toString(), QStringLiteral( "a2" ) );

  model.setFilters( QgsProcessingToolboxProxyModel::FilterModeler );
  group1Index = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount(), 1 );
  QCOMPARE( model.rowCount( group1Index ), 1 );
  QCOMPARE( model.data( group1Index, Qt::DisplayRole ).toString(), QStringLiteral( "group1" ) );
  QCOMPARE( model.data( model.index( 0, 0, group1Index ), Qt::DisplayRole ).toString(), QStringLiteral( "a2" ) );
  model.setFilters( QgsProcessingToolboxProxyModel::FilterToolbox );
  group2Index = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount(), 3 );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, group1Index ), Qt::DisplayRole ).toString(), QStringLiteral( "a1" ) );

  // inactive provider - should not be visible
  DummyAlgorithm *qgisA31 = new DummyAlgorithm( "a3", "group1" );
  DummyProvider *p3 = new DummyProvider( "p3", "provider3", QList< QgsProcessingAlgorithm * >() << qgisA31 );
  p3->mActive = false;
  registry.addProvider( p3 );
  QCOMPARE( model.rowCount(), 3 );
}

QGSTEST_MAIN( TestQgsProcessingModel )
#include "testqgsprocessingmodel.moc"
