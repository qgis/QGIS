/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** This file is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>
#include <QtGui/QtGui>

#include "modeltest.h"
#include "dynamictreemodel.h"


class tst_ModelTest : public QObject
{
    Q_OBJECT

  public:
    tst_ModelTest() {}
    virtual ~tst_ModelTest() {}

  public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

  private slots:
    void stringListModel();
    void treeWidgetModel();
    void standardItemModel();
    void testInsertThroughProxy();
    void moveSourceItems();
    void testResetThroughProxy();
};



void tst_ModelTest::initTestCase()
{
}

void tst_ModelTest::cleanupTestCase()
{
}

void tst_ModelTest::init()
{

}

void tst_ModelTest::cleanup()
{
}
/*
  tests
*/

void tst_ModelTest::stringListModel()
{
  QStringListModel model;
  QSortFilterProxyModel proxy;

  ModelTest t1( &model );
  ModelTest t2( &proxy );

  proxy.setSourceModel( &model );

  model.setStringList( QStringList() << "2" << "3" << "1" );
  model.setStringList( QStringList() << "a" << "e" << "plop" << "b" << "c" );

  proxy.setDynamicSortFilter( true );
  proxy.setFilterRegExp( QRegExp( "[^b]" ) );
}

void tst_ModelTest::treeWidgetModel()
{
  QTreeWidget widget;

  ModelTest t1( widget.model() );

  QTreeWidgetItem *root = new QTreeWidgetItem( &widget, QStringList( "root" ) );
  for ( int i = 0; i < 20; ++i )
  {
    new QTreeWidgetItem( root, QStringList( QString::number( i ) ) );
  }
  QTreeWidgetItem *remove = root->child( 2 );
  root->removeChild( remove );
  QTreeWidgetItem *parent = new QTreeWidgetItem( &widget, QStringList( "parent" ) );
  new QTreeWidgetItem( parent, QStringList( "child" ) );
  widget.setItemHidden( parent, true );

  widget.sortByColumn( 0 );
}

void tst_ModelTest::standardItemModel()
{
  QStandardItemModel model( 10, 10 );
  QSortFilterProxyModel proxy;


  ModelTest t1( &model );
  ModelTest t2( &proxy );

  proxy.setSourceModel( &model );

  model.insertRows( 2, 5 );
  model.removeRows( 4, 5 );

  model.insertColumns( 2, 5 );
  model.removeColumns( 4, 5 );

  model.insertRows( 0, 5, model.index( 1, 1 ) );
  model.insertColumns( 0, 5, model.index( 1, 3 ) );
}

void tst_ModelTest::testInsertThroughProxy()
{
  DynamicTreeModel *model = new DynamicTreeModel( this );

  QSortFilterProxyModel *proxy = new QSortFilterProxyModel( this );
  proxy->setSourceModel( model );

  new ModelTest( proxy, this );

  ModelInsertCommand *insertCommand = new ModelInsertCommand( model, this );
  insertCommand->setNumCols( 4 );
  insertCommand->setStartRow( 0 );
  insertCommand->setEndRow( 9 );
  // Parent is QModelIndex()
  insertCommand->doCommand();

  insertCommand = new ModelInsertCommand( model, this );
  insertCommand->setNumCols( 4 );
  insertCommand->setAncestorRowNumbers( QList<int>() << 5 );
  insertCommand->setStartRow( 0 );
  insertCommand->setEndRow( 9 );
  insertCommand->doCommand();

  ModelMoveCommand *moveCommand = new ModelMoveCommand( model, this );
  moveCommand->setNumCols( 4 );
  moveCommand->setStartRow( 0 );
  moveCommand->setEndRow( 0 );
  moveCommand->setDestRow( 9 );
  moveCommand->setDestAncestors( QList<int>() << 5 );
  moveCommand->doCommand();
}

/**
  Makes the persistent index list publicly accessible
*/
class AccessibleProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    AccessibleProxyModel( QObject *parent = 0 ) : QSortFilterProxyModel( parent ) {}

    QModelIndexList persistent()
    {
      return persistentIndexList();
    }
};

class ObservingObject : public QObject
{
    Q_OBJECT
  public:
    ObservingObject( AccessibleProxyModel  *proxy, QObject *parent = 0 )
        : QObject( parent ),
        m_proxy( proxy )
    {
      connect( m_proxy, SIGNAL( layoutAboutToBeChanged() ), SLOT( storePersistent() ) );
      connect( m_proxy, SIGNAL( layoutChanged() ), SLOT( checkPersistent() ) );
    }

  public slots:

    void storePersistent( const QModelIndex &parent )
    {
      for ( int row = 0; row < m_proxy->rowCount( parent ); ++row )
      {
        QModelIndex proxyIndex = m_proxy->index( row, 0, parent );
        QModelIndex sourceIndex = m_proxy->mapToSource( proxyIndex );
        Q_ASSERT( proxyIndex.isValid() );
        Q_ASSERT( sourceIndex.isValid() );
        m_persistentSourceIndexes.append( sourceIndex );
        m_persistentProxyIndexes.append( proxyIndex );
        if ( m_proxy->hasChildren( proxyIndex ) )
          storePersistent( proxyIndex );
      }
    }

    void storePersistent()
    {
      foreach ( const QModelIndex &idx, m_persistentProxyIndexes )
        Q_ASSERT( idx.isValid() ); // This is called from layoutAboutToBeChanged. Persistent indexes should be valid

      Q_ASSERT( m_proxy->persistent().isEmpty() );
      storePersistent( QModelIndex() );
      Q_ASSERT( !m_proxy->persistent().isEmpty() );
    }

    void checkPersistent()
    {
      for ( int row = 0; row < m_persistentProxyIndexes.size(); ++row )
      {
        QModelIndex updatedProxy = m_persistentProxyIndexes.at( row );
        QModelIndex updatedSource = m_persistentSourceIndexes.at( row );
      }
      for ( int row = 0; row < m_persistentProxyIndexes.size(); ++row )
      {
        QModelIndex updatedProxy = m_persistentProxyIndexes.at( row );
        QModelIndex updatedSource = m_persistentSourceIndexes.at( row );
        QCOMPARE( m_proxy->mapToSource( updatedProxy ), updatedSource );
      }
      m_persistentSourceIndexes.clear();
      m_persistentProxyIndexes.clear();
    }

  private:
    AccessibleProxyModel  *m_proxy;
    QList<QPersistentModelIndex> m_persistentSourceIndexes;
    QList<QPersistentModelIndex> m_persistentProxyIndexes;
};

void tst_ModelTest::moveSourceItems()
{
  DynamicTreeModel *model = new DynamicTreeModel( this );
  AccessibleProxyModel *proxy = new AccessibleProxyModel( this );
  proxy->setSourceModel( model );

  ModelInsertCommand *insertCommand = new ModelInsertCommand( model, this );
  insertCommand->setStartRow( 0 );
  insertCommand->setEndRow( 2 );
  insertCommand->doCommand();

  insertCommand = new ModelInsertCommand( model, this );
  insertCommand->setAncestorRowNumbers( QList<int>() << 1 );
  insertCommand->setStartRow( 0 );
  insertCommand->setEndRow( 2 );
  insertCommand->doCommand();

  ObservingObject observer( proxy );

  ModelMoveCommand *moveCommand = new ModelMoveCommand( model, this );
  moveCommand->setStartRow( 0 );
  moveCommand->setEndRow( 0 );
  moveCommand->setDestAncestors( QList<int>() << 1 );
  moveCommand->setDestRow( 0 );
  moveCommand->doCommand();
}

void tst_ModelTest::testResetThroughProxy()
{
  DynamicTreeModel *model = new DynamicTreeModel( this );

  ModelInsertCommand *insertCommand = new ModelInsertCommand( model, this );
  insertCommand->setStartRow( 0 );
  insertCommand->setEndRow( 2 );
  insertCommand->doCommand();

  QPersistentModelIndex persistent = model->index( 0, 0 );

  AccessibleProxyModel *proxy = new AccessibleProxyModel( this );
  proxy->setSourceModel( model );

  ObservingObject observer( proxy );
  observer.storePersistent();

  ModelResetCommand *resetCommand = new ModelResetCommand( model, this );
  resetCommand->setNumCols( 0 );
  resetCommand->doCommand();
}


QTEST_MAIN( tst_ModelTest )
#include "tst_modeltest.moc"
