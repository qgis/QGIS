/***************************************************************************
                         testqgsprocessingmodel.cpp
                         --------------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
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
#include "qgsprocessingrecentalgorithmlog.h"
#include "qgsprocessingtoolboxtreeview.h"
#include "qgssettings.h"

#include <QtTest/QSignalSpy>
#include "qgstest.h"

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif


class DummyAlgorithm : public QgsProcessingAlgorithm
{
  public:

    DummyAlgorithm( const QString &name, const QString &group,
                    QgsProcessingAlgorithm::Flags flags = QgsProcessingAlgorithm::Flags(),
                    const QString &tags = QString(),
                    const QString &shortDescription = QString(),
                    const QString &displayName = QString() )
      : mName( name )
      , mDisplayName( displayName )
      , mGroup( group )
      , mFlags( flags )
      , mTags( tags.split( ',' ) )
      , mShortDescription( shortDescription )
    {}

    void initAlgorithm( const QVariantMap & = QVariantMap() ) override {}
    QgsProcessingAlgorithm::Flags flags() const override { return mFlags; }
    QString name() const override { return mName; }
    QString displayName() const override { return mDisplayName.isEmpty() ? mName : mDisplayName; }
    QString group() const override { return mGroup; }
    QString groupId() const override { return mGroup; }
    QString shortDescription() const override { return mShortDescription; }
    QStringList tags() const override { return mTags; }
    QVariantMap processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) override { return QVariantMap(); }

    DummyAlgorithm *createInstance() const override { return new DummyAlgorithm( mName, mGroup, mFlags, mTags.join( ',' ), mShortDescription, mDisplayName ); }

    QString mName;
    QString mDisplayName;
    QString mGroup;
    QgsProcessingAlgorithm::Flags mFlags = QgsProcessingAlgorithm::Flags();
    QStringList mTags;
    QString mShortDescription;

};
//dummy provider for testing
class DummyProvider : public QgsProcessingProvider // clazy:exclude=missing-qobject-macro
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
    void testKnownIssues();
    void testProxyModel();
    void testView();
};


void TestQgsProcessingModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsSettings().clear();
}

void TestQgsProcessingModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingModel::testModel()
{
  QgsProcessingRegistry registry;
  QgsProcessingRecentAlgorithmLog recentLog;
  QgsProcessingToolboxModel model( nullptr, &registry, &recentLog );

#ifdef ENABLE_MODELTEST
  new ModelTest( &model, this ); // for model validity checking
#endif

  QCOMPARE( model.columnCount(), 1 );
  QCOMPARE( model.rowCount(), 1 );
  QVERIFY( model.hasChildren() );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "Recently used" ) );
  QCOMPARE( model.rowCount( model.index( 0, 0, QModelIndex() ) ), 0 );
  QVERIFY( !model.providerForIndex( model.index( 0, 0, QModelIndex() ) ) );
  QVERIFY( !model.providerForIndex( model.index( 1, 0, QModelIndex() ) ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );
  QVERIFY( model.index2node( QModelIndex() ) ); // root node
  QCOMPARE( model.index2node( QModelIndex() )->nodeType(), QgsProcessingToolboxModelNode::NodeGroup );
  QVERIFY( model.index2node( model.index( -1, 0, QModelIndex() ) ) ); // root node
  QCOMPARE( model.index2node( QModelIndex() ), model.index2node( model.index( -1, 0, QModelIndex() ) ) );

  // add a provider
  DummyProvider *p1 = new DummyProvider( "p1", "provider1" );
  registry.addProvider( p1 );
  QCOMPARE( model.rowCount(), 2 );
  QVERIFY( model.hasChildren() );

  QVERIFY( model.index( 0, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "Recently used" ) );
  QCOMPARE( model.providerForIndex( model.index( 1, 0, QModelIndex() ) ), p1 );
  QVERIFY( !model.providerForIndex( model.index( 2, 0, QModelIndex() ) ) );
  QCOMPARE( model.indexForProvider( p1->id() ), model.index( 1, 0, QModelIndex() ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );
  QCOMPARE( model.rowCount( model.index( 1, 0, QModelIndex() ) ), 0 );
  QVERIFY( !model.hasChildren( model.index( 1, 0, QModelIndex() ) ) );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), QStringLiteral( "long name provider1" ) );
  QVERIFY( !model.data( model.index( 2, 0, QModelIndex() ), Qt::DisplayRole ).isValid() );
  QVERIFY( !model.data( model.index( 2, 0, QModelIndex() ), Qt::ToolTipRole ).isValid() );

  // second provider
  DummyProvider *p2 = new DummyProvider( "p2", "provider2" );
  registry.addProvider( p2 );
  QCOMPARE( model.rowCount(), 3 );
  QVERIFY( model.hasChildren() );

  QVERIFY( model.index( 2, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.providerForIndex( model.index( 1, 0, QModelIndex() ) ), p1 );
  QCOMPARE( model.providerForIndex( model.index( 2, 0, QModelIndex() ) ), p2 );
  QVERIFY( !model.providerForIndex( model.index( 3, 0, QModelIndex() ) ) );
  QCOMPARE( model.indexForProvider( p1->id() ), model.index( 1, 0, QModelIndex() ) );
  QCOMPARE( model.indexForProvider( p2->id() ), model.index( 2, 0, QModelIndex() ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );
  QVERIFY( !model.hasChildren( model.index( 2, 0, QModelIndex() ) ) );

  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), QStringLiteral( "long name provider1" ) );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), QStringLiteral( "long name provider2" ) );
  QVERIFY( !model.data( model.index( 3, 0, QModelIndex() ), Qt::DisplayRole ).isValid() );
  QVERIFY( !model.data( model.index( 3, 0, QModelIndex() ), Qt::ToolTipRole ).isValid() );

  // provider with algs and groups
  DummyAlgorithm *a1 = new DummyAlgorithm( "a1", "group1", QgsProcessingAlgorithm::FlagHideFromModeler, QStringLiteral( "tag1,tag2" ), QStringLiteral( "short desc a" ) );
  DummyAlgorithm *a2 = new DummyAlgorithm( "a2", "group2", QgsProcessingAlgorithm::FlagHideFromToolbox );
  DummyProvider *p3 = new DummyProvider( "p3", "provider3", QList< QgsProcessingAlgorithm * >() << a1 << a2 );
  registry.addProvider( p3 );

  QCOMPARE( model.rowCount(), 4 );
  QVERIFY( model.hasChildren() );
  QVERIFY( model.index( 3, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.providerForIndex( model.index( 3, 0, QModelIndex() ) ), p3 );
  QCOMPARE( model.indexForProvider( p1->id() ), model.index( 1, 0, QModelIndex() ) );
  QCOMPARE( model.indexForProvider( p2->id() ), model.index( 2, 0, QModelIndex() ) );
  QCOMPARE( model.indexForProvider( p3->id() ), model.index( 3, 0, QModelIndex() ) );
  QCOMPARE( model.rowCount( model.index( 2, 0, QModelIndex() ) ), 0 );
  QCOMPARE( model.rowCount( model.index( 3, 0, QModelIndex() ) ), 2 );
  QVERIFY( !model.hasChildren( model.index( 2, 0, QModelIndex() ) ) );
  QVERIFY( model.hasChildren( model.index( 3, 0, QModelIndex() ) ) );
  QModelIndex providerIndex = model.index( 3, 0, QModelIndex() );
  QVERIFY( !model.providerForIndex( model.index( 1, 0, providerIndex ) ) );
  QVERIFY( !model.providerForIndex( model.index( 2, 0, providerIndex ) ) );

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
  QCOMPARE( model.data( alg1Index, Qt::ToolTipRole ).toString(), QStringLiteral( u"<p><b>a1</b></p><p>short desc a</p><p>Algorithm ID: \u2018<i>p3:a1</i>\u2019</p>" ) );
  QCOMPARE( model.data( alg1Index, QgsProcessingToolboxModel::RoleAlgorithmFlags ).toInt(), static_cast< int >( QgsProcessingAlgorithm::FlagHideFromModeler ) );
  QCOMPARE( model.data( alg1Index, QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p3:a1" ) );
  QCOMPARE( model.data( alg1Index, QgsProcessingToolboxModel::RoleAlgorithmName ).toString(), QStringLiteral( "a1" ) );
  QCOMPARE( model.data( alg1Index, QgsProcessingToolboxModel::RoleAlgorithmTags ).toStringList().join( ',' ), QStringLiteral( "tag1,tag2" ) );
  QCOMPARE( model.data( alg1Index, QgsProcessingToolboxModel::RoleAlgorithmShortDescription ).toString(), QStringLiteral( "short desc a" ) );

  QCOMPARE( model.algorithmForIndex( alg1Index )->id(), QStringLiteral( "p3:a1" ) );

  QModelIndex group2Index = model.index( 1, 0, providerIndex );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QVERIFY( model.hasChildren( group2Index ) );
  QModelIndex alg2Index = model.index( 0, 0, group2Index );
  QCOMPARE( model.data( alg2Index, Qt::DisplayRole ).toString(), QStringLiteral( "a2" ) );
  QCOMPARE( model.data( alg2Index, Qt::ToolTipRole ).toString(), QStringLiteral( u"<p><b>a2</b></p><p>Algorithm ID: \u2018<i>p3:a2</i>\u2019</p>" ) );
  QCOMPARE( model.data( alg2Index, QgsProcessingToolboxModel::RoleAlgorithmFlags ).toInt(), static_cast< int >( QgsProcessingAlgorithm::FlagHideFromToolbox ) );
  QCOMPARE( model.data( alg2Index, QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p3:a2" ) );
  QCOMPARE( model.data( alg2Index, QgsProcessingToolboxModel::RoleAlgorithmName ).toString(), QStringLiteral( "a2" ) );
  QCOMPARE( model.data( alg2Index, QgsProcessingToolboxModel::RoleAlgorithmTags ).toStringList().join( ',' ), QString() );
  QCOMPARE( model.data( alg2Index, QgsProcessingToolboxModel::RoleAlgorithmShortDescription ).toString(), QString() );
  QCOMPARE( model.algorithmForIndex( alg2Index )->id(), QStringLiteral( "p3:a2" ) );

  // combined groups
  DummyAlgorithm *a3 = new DummyAlgorithm( "a3", "group1" );
  DummyAlgorithm *a4 = new DummyAlgorithm( "a4", "group1" );
  DummyProvider *p4 = new DummyProvider( "p4", "provider4", QList< QgsProcessingAlgorithm * >() << a3 << a4 );
  registry.addProvider( p4 );
  const QModelIndex p4ProviderIndex = model.indexForProvider( p4->id() );
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
  QCOMPARE( model.rowCount(), 6 );
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
  QCOMPARE( model.rowCount(), 6 );
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

  // recent algorithms
  QModelIndex recentIndex = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.data( recentIndex, Qt::DisplayRole ).toString(), QStringLiteral( "Recently used" ) );
  QCOMPARE( model.rowCount( recentIndex ), 0 );
  recentLog.push( QStringLiteral( "p5:a5" ) );
  QCOMPARE( model.rowCount( recentIndex ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p5:a5" ) );
  recentLog.push( QStringLiteral( "not valid" ) );
  QCOMPARE( model.rowCount( recentIndex ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p5:a5" ) );
  recentLog.push( QStringLiteral( "p4:a3" ) );
  QCOMPARE( model.rowCount( recentIndex ), 2 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p4:a3" ) );
  QCOMPARE( model.data( model.index( 1, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p5:a5" ) );

  // remove a provider
  registry.removeProvider( p1 );
  QCOMPARE( model.rowCount(), 5 );
  QVERIFY( model.index( 0, 0, QModelIndex() ).isValid() );
  QCOMPARE( model.providerForIndex( model.index( 1, 0, QModelIndex() ) ), p2 );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  registry.removeProvider( p5 );
  QCOMPARE( model.rowCount(), 4 );
  recentIndex = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( recentIndex ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p4:a3" ) );
  registry.removeProvider( p2 );
  QCOMPARE( model.rowCount(), 3 );
  registry.removeProvider( p3 );
  QCOMPARE( model.rowCount(), 2 );
  recentIndex = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( recentIndex ), 1 );
  registry.removeProvider( p4 );
  recentIndex = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( recentIndex ), 0 );
  QCOMPARE( model.rowCount(), 1 );
  QCOMPARE( model.columnCount(), 1 );
  QVERIFY( model.hasChildren() );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), QStringLiteral( "Recently used" ) );
  QVERIFY( !model.providerForIndex( model.index( 0, 0, QModelIndex() ) ) );
  QVERIFY( !model.providerForIndex( model.index( 1, 0, QModelIndex() ) ) );
  QVERIFY( !model.indexForProvider( nullptr ).isValid() );

  // qgis native algorithms put groups at top level
  QgsProcessingRegistry registry2;
  const QgsProcessingToolboxModel model2( nullptr, &registry2 );
  DummyAlgorithm *qgisA1 = new DummyAlgorithm( "a1", "group1" );
  DummyAlgorithm *qgisA2 = new DummyAlgorithm( "a2", "group2" );
  DummyAlgorithm *qgisA3 = new DummyAlgorithm( "a3", "group1" );
  DummyAlgorithm *qgisA4 = new DummyAlgorithm( "a4", "group3" );
  DummyProvider *qgisP = new DummyProvider( "qgis", "qgis_provider", QList< QgsProcessingAlgorithm * >() << qgisA1 << qgisA2 << qgisA3 << qgisA4 );
  registry2.addProvider( qgisP );

  QCOMPARE( model2.rowCount(), 3 );
  group1Index = model2.index( 0, 0 );
  group2Index = model2.index( 1, 0 );
  const QModelIndex group3Index = model2.index( 2, 0 );
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
  QgsSettings().clear();
  QgsProcessingRegistry registry;
  QgsProcessingRecentAlgorithmLog recentLog;
  QgsProcessingToolboxProxyModel model( nullptr, &registry, &recentLog );

#ifdef ENABLE_MODELTEST
  new ModelTest( &model, this ); // for model validity checking
#endif

  // add a provider
  DummyAlgorithm *a1 = new DummyAlgorithm( "a1", "group2", QgsProcessingAlgorithm::FlagHideFromModeler );
  DummyProvider *p1 = new DummyProvider( "p2", "provider2", QList< QgsProcessingAlgorithm * >() << a1 );
  registry.addProvider( p1 );
  // second provider
  DummyAlgorithm *a2 = new DummyAlgorithm( "a2", "group2", QgsProcessingAlgorithm::FlagHideFromModeler,
      QStringLiteral( "buffer,vector" ), QStringLiteral( "short desc" ), QStringLiteral( "algorithm2" ) );
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
  const QModelIndex alg2Index = model.index( 0, 0, group1Index );
  QCOMPARE( model.data( alg2Index, Qt::DisplayRole ).toString(), QStringLiteral( "a2" ) );

  // empty providers/groups should not be shown
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

  // test filter strings
  model.setFilters( QgsProcessingToolboxProxyModel::Filters() );
  // filter by algorithm id
  model.setFilterString( "a1" );
  QCOMPARE( model.rowCount(), 2 );
  group1Index = model.index( 0, 0, QModelIndex() );
  QModelIndex provider2Index = model.index( 1, 0, QModelIndex() );
  QCOMPARE( model.rowCount( group1Index ), 1 );
  QCOMPARE( model.data( group1Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.data( model.index( 0, 0, group1Index ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "qgis:a1" ) );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( provider2Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  QCOMPARE( model.rowCount( provider2Index ), 1 );
  group2Index = model.index( 0, 0, provider2Index );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.data( model.index( 0, 0, group2Index ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p2:a1" ) );

  // filter by algorithm display name
  model.setFilterString( QStringLiteral( "ALGOR" ) );
  QCOMPARE( model.rowCount(), 1 );
  QModelIndex provider1Index = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  QCOMPARE( model.data( provider1Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  group2Index = model.index( 0, 0, provider1Index );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, group2Index ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p1:a2" ) );

  // filter by algorithm tags
  model.setFilterString( QStringLiteral( "buff CTOR" ) );
  QCOMPARE( model.rowCount(), 1 );
  provider1Index = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  QCOMPARE( model.data( provider1Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  group2Index = model.index( 0, 0, provider1Index );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, group2Index ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p1:a2" ) );

  // filter by algorithm short desc
  model.setFilterString( QStringLiteral( "buff CTOR desc" ) );
  QCOMPARE( model.rowCount(), 1 );
  provider1Index = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  QCOMPARE( model.data( provider1Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  group2Index = model.index( 0, 0, provider1Index );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, group2Index ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p1:a2" ) );

  // filter by group
  model.setFilterString( QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount(), 3 );
  group2Index = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  alg1Index = model.index( 0, 0, group2Index );
  QCOMPARE( model.data( alg1Index, QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "qgis:a1" ) );
  provider1Index = model.index( 1, 0, QModelIndex() );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  QCOMPARE( model.data( provider1Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( model.rowCount( provider1Index ), 1 );
  group2Index = model.index( 0, 0, provider1Index );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, group2Index ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p1:a2" ) );
  provider2Index = model.index( 2, 0, QModelIndex() );
  QCOMPARE( model.rowCount( provider2Index ), 1 );
  QCOMPARE( model.data( provider2Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  group2Index = model.index( 0, 0, provider2Index );
  QCOMPARE( model.data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QCOMPARE( model.rowCount( group2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, group2Index ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p2:a1" ) );

  model.setFilterString( QString() );
  QCOMPARE( model.rowCount(), 4 );

  // check sort order of recent algorithms
  recentLog.push( QStringLiteral( "p2:a1" ) );
  QCOMPARE( model.rowCount(), 5 );
  const QModelIndex recentIndex = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.data( recentIndex, Qt::DisplayRole ).toString(), QStringLiteral( "Recently used" ) );
  QCOMPARE( model.rowCount( recentIndex ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p2:a1" ) );
  recentLog.push( QStringLiteral( "p1:a2" ) );
  QCOMPARE( model.rowCount( recentIndex ), 2 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p1:a2" ) );
  QCOMPARE( model.data( model.index( 1, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p2:a1" ) );
  recentLog.push( QStringLiteral( "qgis:a1" ) );
  QCOMPARE( model.rowCount( recentIndex ), 3 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "qgis:a1" ) );
  QCOMPARE( model.data( model.index( 1, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p1:a2" ) );
  QCOMPARE( model.data( model.index( 2, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p2:a1" ) );
  recentLog.push( QStringLiteral( "p2:a1" ) );
  QCOMPARE( model.rowCount( recentIndex ), 3 );
  QCOMPARE( model.data( model.index( 0, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p2:a1" ) );
  QCOMPARE( model.data( model.index( 1, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "qgis:a1" ) );
  QCOMPARE( model.data( model.index( 2, 0, recentIndex ), QgsProcessingToolboxModel::RoleAlgorithmId ).toString(), QStringLiteral( "p1:a2" ) );

  // inactive provider - should not be visible
  QCOMPARE( model.rowCount(), 5 );
  DummyAlgorithm *qgisA31 = new DummyAlgorithm( "a3", "group1" );
  DummyProvider *p3 = new DummyProvider( "p3", "provider3", QList< QgsProcessingAlgorithm * >() << qgisA31 );
  p3->mActive = false;
  registry.addProvider( p3 );
  QCOMPARE( model.rowCount(), 5 );
}

void TestQgsProcessingModel::testView()
{
  QgsSettings().clear();
  QgsProcessingRegistry registry;
  QgsProcessingRecentAlgorithmLog recentLog;
  QgsProcessingToolboxTreeView view( nullptr, &registry, &recentLog );

  // Check view model consistency
  QVERIFY( view.mModel );
  QVERIFY( view.mToolboxModel );
  QCOMPARE( view.mModel->toolboxModel(), view.mToolboxModel );

  QVERIFY( !view.algorithmForIndex( QModelIndex() ) );

  // add a provider
  DummyAlgorithm *a1 = new DummyAlgorithm( "a1", "group2", QgsProcessingAlgorithm::FlagHideFromToolbox );
  DummyProvider *p1 = new DummyProvider( "p2", "provider2", QList< QgsProcessingAlgorithm * >() << a1 );
  registry.addProvider( p1 );
  // second provider
  DummyAlgorithm *a2 = new DummyAlgorithm( "a2", "group2", QgsProcessingAlgorithm::FlagHideFromModeler,
      QStringLiteral( "buffer,vector" ), QStringLiteral( "short desc" ), QStringLiteral( "algorithm2" ) );
  DummyProvider *p2 = new DummyProvider( "p1", "provider1", QList< QgsProcessingAlgorithm * >() << a2 );
  registry.addProvider( p2 );

  QModelIndex provider1Index = view.model()->index( 0, 0, QModelIndex() );
  QVERIFY( !view.algorithmForIndex( provider1Index ) );
  QModelIndex group2Index = view.model()->index( 0, 0, provider1Index );
  QCOMPARE( view.model()->data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  QVERIFY( !view.algorithmForIndex( group2Index ) );
  QModelIndex alg2Index = view.model()->index( 0, 0, group2Index );
  QCOMPARE( view.algorithmForIndex( alg2Index )->id(), QStringLiteral( "p1:a2" ) );
  QModelIndex provider2Index = view.model()->index( 1, 0, QModelIndex() );
  QVERIFY( !view.algorithmForIndex( provider2Index ) );
  QCOMPARE( view.model()->data( provider2Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  group2Index = view.model()->index( 0, 0, provider2Index );
  QCOMPARE( view.model()->data( group2Index, Qt::DisplayRole ).toString(), QStringLiteral( "group2" ) );
  const QModelIndex alg1Index = view.model()->index( 0, 0, group2Index );
  QCOMPARE( view.algorithmForIndex( alg1Index )->id(), QStringLiteral( "p2:a1" ) );

  // empty providers/groups should not be shown
  view.setFilters( QgsProcessingToolboxProxyModel::FilterModeler );
  QCOMPARE( view.filters(), QgsProcessingToolboxProxyModel::FilterModeler );
  QCOMPARE( view.model()->rowCount(), 1 );
  provider2Index = view.model()->index( 0, 0, QModelIndex() );
  QCOMPARE( view.model()->data( provider2Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  QCOMPARE( view.model()->rowCount( provider2Index ), 1 );
  group2Index = view.model()->index( 0, 0, provider2Index );
  QCOMPARE( view.model()->rowCount( group2Index ), 1 );
  QCOMPARE( view.algorithmForIndex( view.model()->index( 0, 0, group2Index ) )->id(), QStringLiteral( "p2:a1" ) );
  view.setFilters( QgsProcessingToolboxProxyModel::FilterToolbox );
  QCOMPARE( view.model()->rowCount(), 1 );
  provider1Index = view.model()->index( 0, 0, QModelIndex() );
  QCOMPARE( view.model()->data( provider1Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider1" ) );
  QCOMPARE( view.model()->rowCount( provider1Index ), 1 );
  group2Index = view.model()->index( 0, 0, provider1Index );
  QCOMPARE( view.model()->rowCount( group2Index ), 1 );
  QCOMPARE( view.algorithmForIndex( view.model()->index( 0, 0, group2Index ) )->id(), QStringLiteral( "p1:a2" ) );

  view.setFilters( QgsProcessingToolboxProxyModel::Filters() );
  QCOMPARE( view.filters(), QgsProcessingToolboxProxyModel::Filters() );
  // test filter strings
  view.setFilterString( "a1" );
  provider2Index = view.model()->index( 0, 0, QModelIndex() );
  QCOMPARE( view.model()->data( provider2Index, Qt::DisplayRole ).toString(), QStringLiteral( "provider2" ) );
  QCOMPARE( view.model()->rowCount(), 1 );
  QCOMPARE( view.model()->rowCount( provider2Index ), 1 );
  group2Index = view.model()->index( 0, 0, provider2Index );
  QCOMPARE( view.model()->rowCount( group2Index ), 1 );
  QCOMPARE( view.algorithmForIndex( view.model()->index( 0, 0, group2Index ) )->id(), QStringLiteral( "p2:a1" ) );

  view.setFilterString( QString() );

  // selected algorithm
  provider1Index = view.model()->index( 0, 0, QModelIndex() );
  view.selectionModel()->clear();
  QVERIFY( !view.selectedAlgorithm() );
  view.selectionModel()->select( provider1Index, QItemSelectionModel::ClearAndSelect );
  QVERIFY( !view.selectedAlgorithm() );
  group2Index = view.model()->index( 0, 0, provider1Index );
  view.selectionModel()->select( group2Index, QItemSelectionModel::ClearAndSelect );
  QVERIFY( !view.selectedAlgorithm() );
  alg2Index = view.model()->index( 0, 0, group2Index );
  view.selectionModel()->select( alg2Index, QItemSelectionModel::ClearAndSelect );
  QCOMPARE( view.selectedAlgorithm()->id(), QStringLiteral( "p1:a2" ) );

  // when a filter string removes the selected algorithm, the next matching algorithm should be auto-selected
  view.setFilterString( QStringLiteral( "p2:a1" ) );
  QCOMPARE( view.selectedAlgorithm()->id(), QStringLiteral( "p2:a1" ) );
  // but if it doesn't remove the selected one, that algorithm should not be deselected
  view.setFilterString( QStringLiteral( "a" ) );
  QCOMPARE( view.selectedAlgorithm()->id(), QStringLiteral( "p2:a1" ) );

  // Check view model consistency after resetting registry
  view.setRegistry( &registry );
  QVERIFY( view.mModel );
  QVERIFY( view.mToolboxModel );
  QCOMPARE( view.mModel->toolboxModel(), view.mToolboxModel );
}

void TestQgsProcessingModel::testKnownIssues()
{
  QgsProcessingRegistry registry;
  QgsProcessingRecentAlgorithmLog recentLog;
  const QgsProcessingToolboxModel model( nullptr, &registry, &recentLog );
  DummyAlgorithm *a1 = new DummyAlgorithm( "a1", "group1", QgsProcessingAlgorithm::FlagKnownIssues, QStringLiteral( "tag1,tag2" ), QStringLiteral( "short desc a" ) );
  DummyAlgorithm *a2 = new DummyAlgorithm( "b1", "group1", QgsProcessingAlgorithm::Flags(), QStringLiteral( "tag1,tag2" ), QStringLiteral( "short desc b" ) );
  DummyProvider *p = new DummyProvider( "p3", "provider3", QList< QgsProcessingAlgorithm * >() << a1 << a2 );
  registry.addProvider( p );

  QModelIndex providerIndex = model.index( 1, 0, QModelIndex() );
  QModelIndex group1Index = model.index( 0, 0, providerIndex );
  QCOMPARE( model.data( model.index( 0, 0, group1Index ), Qt::DisplayRole ).toString(), QStringLiteral( "a1" ) );
  QVERIFY( model.data( model.index( 0, 0, group1Index ), Qt::ToolTipRole ).toString().contains( QStringLiteral( "known issues" ) ) );
  QCOMPARE( model.data( model.index( 0, 0, group1Index ), Qt::ForegroundRole ).value< QBrush >().color().name(), QStringLiteral( "#ff0000" ) );
  QCOMPARE( model.data( model.index( 1, 0, group1Index ), Qt::DisplayRole ).toString(), QStringLiteral( "b1" ) );
  QVERIFY( !model.data( model.index( 1, 0, group1Index ), Qt::ToolTipRole ).toString().contains( QStringLiteral( "known issues" ) ) );
  QCOMPARE( model.data( model.index( 1, 0, group1Index ), Qt::ForegroundRole ).value< QBrush >().color().name(), QStringLiteral( "#000000" ) );

  QgsProcessingToolboxProxyModel proxyModel( nullptr, &registry, &recentLog );
  providerIndex = proxyModel.index( 0, 0, QModelIndex() );
  group1Index = proxyModel.index( 0, 0, providerIndex );
  // by default known issues are filtered out
  QCOMPARE( proxyModel.rowCount( group1Index ), 1 );
  QCOMPARE( proxyModel.data( proxyModel.index( 0, 0, group1Index ), Qt::DisplayRole ).toString(), QStringLiteral( "b1" ) );
  QVERIFY( !proxyModel.data( proxyModel.index( 0, 0, group1Index ), Qt::ToolTipRole ).toString().contains( QStringLiteral( "known issues" ) ) );
  QCOMPARE( proxyModel.data( proxyModel.index( 0, 0, group1Index ), Qt::ForegroundRole ).value< QBrush >().color().name(), QStringLiteral( "#000000" ) );
  proxyModel.setFilters( QgsProcessingToolboxProxyModel::Filters( QgsProcessingToolboxProxyModel::FilterToolbox | QgsProcessingToolboxProxyModel::FilterShowKnownIssues ) );
  QCOMPARE( proxyModel.rowCount( group1Index ), 2 );
  QCOMPARE( proxyModel.data( proxyModel.index( 0, 0, group1Index ), Qt::DisplayRole ).toString(), QStringLiteral( "a1" ) );
  QVERIFY( proxyModel.data( proxyModel.index( 0, 0, group1Index ), Qt::ToolTipRole ).toString().contains( QStringLiteral( "known issues" ) ) );
  QCOMPARE( proxyModel.data( proxyModel.index( 0, 0, group1Index ), Qt::ForegroundRole ).value< QBrush >().color().name(), QStringLiteral( "#ff0000" ) );
  QCOMPARE( proxyModel.data( proxyModel.index( 1, 0, group1Index ), Qt::DisplayRole ).toString(), QStringLiteral( "b1" ) );
  QVERIFY( !proxyModel.data( proxyModel.index( 1, 0, group1Index ), Qt::ToolTipRole ).toString().contains( QStringLiteral( "known issues" ) ) );
  QCOMPARE( proxyModel.data( proxyModel.index( 1, 0, group1Index ), Qt::ForegroundRole ).value< QBrush >().color().name(), QStringLiteral( "#000000" ) );


}

QGSTEST_MAIN( TestQgsProcessingModel )
#include "testqgsprocessingmodel.moc"
