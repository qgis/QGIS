/***************************************************************************
  testqgslayertree.cpp
  --------------------------------------
  Date                 : September 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>

#include <qgsapplication.h>
#include <qgslayertree.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerdiagramprovider.h>
#include <qgsvectorlayerlabelprovider.h>

class TestQgsLayerTree : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testCheckStateParentToChild();
    void testCheckStateChildToParent();
    void testCheckStateMutuallyExclusive();
    void testCheckStateMutuallyExclusiveEdgeCases();

  private:

    QgsLayerTreeGroup* mRoot;
    //QgsVectorLayer* vl;

    Qt::CheckState childState( int childIndex )
    {
      return QgsLayerTree::toGroup( mRoot->children()[childIndex] )->isVisible();
    }
    void setChildState( int childIndex, Qt::CheckState state )
    {
      QgsLayerTree::toGroup( mRoot->children()[childIndex] )->setVisible( state );
    }
};

void TestQgsLayerTree::initTestCase()
{
  mRoot = new QgsLayerTreeGroup();
  mRoot->addGroup( "grp1" );
  mRoot->addGroup( "grp2" );
  mRoot->addGroup( "grp3" );

  // all cases start with all items checked
}

void TestQgsLayerTree::cleanupTestCase()
{
  delete mRoot;
}

void TestQgsLayerTree::testCheckStateParentToChild()
{
  mRoot->setVisible( Qt::Unchecked );

  // all children unchecked
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );

  mRoot->setVisible( Qt::Checked );

  // all children checked
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Checked );
  QCOMPARE( childState( 2 ), Qt::Checked );
}

void TestQgsLayerTree::testCheckStateChildToParent()
{
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // uncheck a child - parent should be partial
  setChildState( 0, Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::PartiallyChecked );
  setChildState( 1, Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::PartiallyChecked );

  // uncheck last child - parent should be unchecked
  setChildState( 2, Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // go back to original state
  mRoot->setVisible( Qt::Checked );
}

void TestQgsLayerTree::testCheckStateMutuallyExclusive()
{
  mRoot->setIsMutuallyExclusive( true );

  // only first should be enabled
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked ); // fully checked, not just partial

  // switch to some other child
  setChildState( 2, Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // now uncheck the root
  mRoot->setVisible( Qt::Unchecked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // check one of the children - should also check the root
  setChildState( 2, Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // uncheck the child - should also uncheck the root
  setChildState( 2, Qt::Unchecked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // check the root back - should have the same node
  mRoot->setVisible( Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // remove a child
  mRoot->removeChildNode( mRoot->children()[0] );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // add the group back - will not be checked
  mRoot->insertGroup( 0, "grp1" );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // remove a child that is checked
  mRoot->removeChildNode( mRoot->children()[2] );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // check the root again - first item should be checked
  mRoot->setVisible( Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // add the item back
  mRoot->addGroup( "grp3" );
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  mRoot->setIsMutuallyExclusive( false );

  QCOMPARE( mRoot->isVisible(), Qt::PartiallyChecked );

  // go back to original state
  mRoot->setVisible( Qt::Checked );
}

void TestQgsLayerTree::testCheckStateMutuallyExclusiveEdgeCases()
{
  // starting with empty mutually exclusive group
  QgsLayerTreeGroup* root2 = new QgsLayerTreeGroup();
  root2->setIsMutuallyExclusive( true );
  root2->addGroup( "1" );
  QCOMPARE( QgsLayerTree::toGroup( root2->children()[0] )->isVisible(), Qt::Checked );
  root2->addGroup( "2" );
  QCOMPARE( QgsLayerTree::toGroup( root2->children()[0] )->isVisible(), Qt::Checked );
  QCOMPARE( QgsLayerTree::toGroup( root2->children()[1] )->isVisible(), Qt::Unchecked );
  delete root2;

  // check-uncheck the only child
  QgsLayerTreeGroup* root3 = new QgsLayerTreeGroup();
  root3->setIsMutuallyExclusive( true );
  root3->addGroup( "1" );
  QCOMPARE( QgsLayerTree::toGroup( root3->children()[0] )->isVisible(), Qt::Checked );
  QgsLayerTree::toGroup( root3->children()[0] )->setVisible( Qt::Unchecked );
  QCOMPARE( QgsLayerTree::toGroup( root3->children()[0] )->isVisible(), Qt::Unchecked );
  QCOMPARE( root3->isVisible(), Qt::Unchecked );
  QgsLayerTree::toGroup( root3->children()[0] )->setVisible( Qt::Checked );
  QCOMPARE( QgsLayerTree::toGroup( root3->children()[0] )->isVisible(), Qt::Checked );
  QCOMPARE( root3->isVisible(), Qt::Checked );
  delete root3;
}


QTEST_MAIN( TestQgsLayerTree )
#include "testqgslayertree.moc"
