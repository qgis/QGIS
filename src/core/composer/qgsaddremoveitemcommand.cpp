/***************************************************************************
                          qgsaddremoveitemcommand.cpp
                          ---------------------------
    begin                : 2010-11-27
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaddremoveitemcommand.h"
#include "qgscomposeritem.h"
#include "qgscomposition.h"
#include "qgsproject.h"
#include "qgscomposermodel.h"

QgsAddRemoveItemCommand::QgsAddRemoveItemCommand( State s, QgsComposerItem* item, QgsComposition* c, const QString& text, QUndoCommand* parent ):
    QUndoCommand( text, parent ), mItem( item ), mComposition( c ), mState( s ), mFirstRun( true )
{
}

QgsAddRemoveItemCommand::~QgsAddRemoveItemCommand()
{
  if ( mState == Removed ) //command class stores the item if removed from the composition
  {
    delete mItem;
  }
}

void QgsAddRemoveItemCommand::redo()
{
  QUndoCommand::redo(); // call redo() on all childs
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  switchState();
}

void QgsAddRemoveItemCommand::undo()
{
  QUndoCommand::undo(); // call undo() on all childs, in reverse order
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  switchState();
}

void QgsAddRemoveItemCommand::switchState()
{
  if ( mState == Added )
  {
    // Remove
    if ( mComposition )
    {
      mComposition->itemsModel()->setItemRemoved( mItem );
      mComposition->removeItem( mItem );
    }
    emit itemRemoved( mItem );
    mState = Removed;
  }
  else //Removed
  {
    // Add
    if ( mComposition )
    {
      mComposition->itemsModel()->setItemRestored( mItem );
      mComposition->addItem( mItem );
    }
    emit itemAdded( mItem );
    mState = Added;
  }
  QgsProject::instance()->setDirty( true );
}
