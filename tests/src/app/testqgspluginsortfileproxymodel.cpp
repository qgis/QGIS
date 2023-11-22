/***************************************************************************
                         testqgspluginsortfileproxymodel.cpp
                         --------------------------
    begin                : November 2023
    copyright            : (C) 2023 by Germán Carrillo
    email                : gcarrillo at linuxmail dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QStandardItemModel>

#include "qgspluginsortfilterproxymodel.h"
#include "qgssettings.h"

#include <QtTest/QSignalSpy>
#include "qgstest.h"

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif


class TestQgsPluginSortFileProxyModel: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testProxyModelFilters();
};


void TestQgsPluginSortFileProxyModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsSettings().clear();
}

void TestQgsPluginSortFileProxyModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPluginSortFileProxyModel::testProxyModelFilters()
{
  QStandardItemModel *model = new QStandardItemModel( 0, 1 );
  QgsPluginSortFilterProxyModel *mModelProxy = new QgsPluginSortFilterProxyModel( this );
  mModelProxy->setSourceModel( model );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  mModelProxy->setSortRole( Qt::DisplayRole );
  mModelProxy->setDynamicSortFilter( true );
  mModelProxy->sort( 0, Qt::AscendingOrder );
  mModelProxy->setFilterRole( 0 );  // Full text search

#ifdef ENABLE_MODELTEST
  new ModelTest( mModelProxy, this ); // for model validity checking
#endif

  // Add plugin items
  QStandardItem *pluginItem = new QStandardItem( "My Fantastic Plugin 1" );
  pluginItem->setData( "myplugin1", PLUGIN_BASE_NAME_ROLE );
  pluginItem->setData( "great plugin description", PLUGIN_DESCRIPTION_ROLE );
  pluginItem->setData( "gcarrillo", PLUGIN_AUTHOR_ROLE );
  pluginItem->setData( QStringLiteral( "abc,def,ghi" ), PLUGIN_TAGS_ROLE );
  model->appendRow( pluginItem );

  pluginItem = new QStandardItem( "My New Fantastic Plugin" );
  pluginItem->setData( "myplugin2", PLUGIN_BASE_NAME_ROLE );
  pluginItem->setData( "This is a new plugin to do new stuff", PLUGIN_DESCRIPTION_ROLE );
  pluginItem->setData( "germap", PLUGIN_AUTHOR_ROLE );
  pluginItem->setData( QStringLiteral( "abc,def,ghi" ), PLUGIN_TAGS_ROLE );
  model->appendRow( pluginItem );

  pluginItem = new QStandardItem( "Map import tools" );
  pluginItem->setData( "myplugin3", PLUGIN_BASE_NAME_ROLE );
  pluginItem->setData( "Tools to import things into maps", PLUGIN_DESCRIPTION_ROLE );
  pluginItem->setData( "germap", PLUGIN_AUTHOR_ROLE );
  pluginItem->setData( QStringLiteral( "jkl,mno,pqr" ), PLUGIN_TAGS_ROLE );
  model->appendRow( pluginItem );

  QCOMPARE( mModelProxy->columnCount(), 1 );
  QCOMPARE( mModelProxy->rowCount(), 3 );

  // Filter by author
  QRegularExpression filterRegExp( "gcarrillo", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );

  filterRegExp = QRegularExpression( QString( "   gcarrillo   " ).simplified(), QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );

  filterRegExp = QRegularExpression( "caRril", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );

  // Filter by plugin title (display role)
  filterRegExp = QRegularExpression( "my fan", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );

  filterRegExp = QRegularExpression( QString( "   my   fan   " ).simplified(), QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );

  filterRegExp = QRegularExpression( "FanTastI", QRegularExpression::CaseInsensitiveOption );  //#spellok
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  // Filter by plugin description
  filterRegExp = QRegularExpression( "new plUgi", QRegularExpression::CaseInsensitiveOption );  //#spellok
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  filterRegExp = QRegularExpression( QString( "   new   plUgi   " ).simplified(), QRegularExpression::CaseInsensitiveOption );  //#spellok
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  // Filter by tags (full text search)
  filterRegExp = QRegularExpression( "abc", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  filterRegExp = QRegularExpression( QString( "   abc   " ).simplified(), QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  filterRegExp = QRegularExpression( "mNo", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 1 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "Map import tools" );

  filterRegExp = QRegularExpression( "abc def", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  filterRegExp = QRegularExpression( "def ghi abc", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  filterRegExp = QRegularExpression( QString( "   def   ghi   abc   " ).simplified(), QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  filterRegExp = QRegularExpression( "ghi ab", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  // Not found
  filterRegExp = QRegularExpression( "pluggin", QRegularExpression::CaseInsensitiveOption );  //#spellok
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 0 );

  // Filter by tag role ('tag:' shortcut)
  mModelProxy->setFilterRole( PLUGIN_TAGS_ROLE );

  filterRegExp = QRegularExpression( "abc", QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 2 );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My Fantastic Plugin 1" );
  QCOMPARE( mModelProxy->data( mModelProxy->index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), "My New Fantastic Plugin" );

  // Tag not found
  filterRegExp = QRegularExpression( "tagg", QRegularExpression::CaseInsensitiveOption );  //#spellok
  mModelProxy->setFilterRegularExpression( filterRegExp );
  QCOMPARE( mModelProxy->rowCount(), 0 );

}


QGSTEST_MAIN( TestQgsPluginSortFileProxyModel )
#include "testqgspluginsortfileproxymodel.moc"
