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
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  switchState();
}

void QgsAddRemoveItemCommand::undo()
{
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
    if ( mComposition )
    {
      mComposition->removeItem( mItem );
    }
    emit itemRemoved( mItem );
    mState = Removed;
  }
  else //Removed
  {
    if ( mComposition )
    {
      mComposition->addItem( mItem );
    }
    emit itemAdded( mItem );
    mState = Added;
  }
}
