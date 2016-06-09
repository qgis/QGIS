/***************************************************************************
                          qgsgroupungroupitemscommand.cpp
                          ---------------------------
    begin                : 2016-06-09
    copyright            : (C) 2016 by Sandro Santilli
    email                : strk at kbt dot io
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgroupungroupitemscommand.h"
#include "qgscomposeritem.h"
#include "qgscomposeritemgroup.h"
#include "qgscomposition.h"
#include "qgsproject.h"
#include "qgscomposermodel.h"
#include "qgslogger.h"

QgsGroupUngroupItemsCommand::QgsGroupUngroupItemsCommand( State s, QgsComposerItemGroup* item, QgsComposition* c, const QString& text, QUndoCommand* parent ):
    QUndoCommand( text, parent ), mGroup( item ), mComposition( c ), mState( s ), mFirstRun( true )
{
  mItems = mGroup->items();
}

QgsGroupUngroupItemsCommand::~QgsGroupUngroupItemsCommand()
{
  if ( mState == Ungrouped )
  {
    //command class stores the item if ungrouped from the composition
    delete mGroup;
  }
}

void QgsGroupUngroupItemsCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  switchState();
}

void QgsGroupUngroupItemsCommand::undo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  switchState();
}

void QgsGroupUngroupItemsCommand::switchState()
{
  if ( mState == Grouped )
  {
    // ungroup
    if ( mComposition )
    {
      // This is probably redundant
      mComposition->itemsModel()->setItemRemoved( mGroup );
      mComposition->removeItem( mGroup );
    }
    mGroup->removeItems();
    emit itemRemoved( mGroup );
    mState = Ungrouped;
  }
  else //Ungrouped
  {
    // group
    if ( mComposition )
    {
      //delete mGroup; mGroup = new QgsComposerItemGroup( mCompoiser );
      QSet<QgsComposerItem*>::iterator itemIter = mItems.begin();
      for ( ; itemIter != mItems.end(); ++itemIter )
      {
        mGroup->addItem( *itemIter );
        QgsDebugMsg( QString( "itemgroup now has %1" ) .arg( mGroup->items().size() ) );
      }
      // Add the group
      mComposition->itemsModel()->setItemRestored( mGroup );
      mComposition->addItem( mGroup );
    }
    mState = Grouped;
    emit itemAdded( mGroup );
  }
  QgsProject::instance()->setDirty( true );
}
