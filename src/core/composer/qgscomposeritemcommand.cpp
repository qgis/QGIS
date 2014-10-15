/***************************************************************************
                          qgscomposeritemcommand.cpp
                          --------------------------
    begin                : 2010-11-18
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

#include "qgscomposeritemcommand.h"
#include "qgscomposeritem.h"
#include "qgscomposerframe.h"
#include "qgscomposermultiframe.h"
#include "qgsproject.h"
#include "qgslogger.h"

QgsComposerItemCommand::QgsComposerItemCommand( QgsComposerItem* item, const QString& text, QUndoCommand* parent )
    : QUndoCommand( text, parent )
    , mItem( item )
    , mMultiFrame( 0 )
    , mFrameNumber( 0 )
    , mFirstRun( true )
{
  //is item a frame?
  QgsComposerFrame* frame = dynamic_cast<QgsComposerFrame*>( mItem );
  if ( frame )
  {
    //store parent multiframe and frame index
    mMultiFrame = frame->multiFrame();
    mFrameNumber = mMultiFrame->frameIndex( frame );
  }
}

QgsComposerItemCommand::~QgsComposerItemCommand()
{
}

void QgsComposerItemCommand::undo()
{
  restoreState( mPreviousState );
}

void QgsComposerItemCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  restoreState( mAfterState );
}

bool QgsComposerItemCommand::containsChange() const
{
  return !( mPreviousState.isNull() || mAfterState.isNull() || mPreviousState.toString() == mAfterState.toString() );
}

QgsComposerItem* QgsComposerItemCommand::item() const
{
  QgsComposerItem* item = 0;
  if ( mMultiFrame )
  {
    //item is a frame, so it needs to be handled differently
    //in this case the target item is the matching frame number, as subsequent
    //changes to the multiframe may have deleted mItem
    if ( mMultiFrame->frameCount() > mFrameNumber )
    {
      item = mMultiFrame->frame( mFrameNumber );
    }
  }
  else if ( mItem )
  {
    item = mItem;
  }

  return item;
}

void QgsComposerItemCommand::savePreviousState()
{
  saveState( mPreviousState );
}

void QgsComposerItemCommand::saveAfterState()
{
  saveState( mAfterState );
}

void QgsComposerItemCommand::saveState( QDomDocument& stateDoc ) const
{
  const QgsComposerItem* source = item();
  if ( !source )
  {
    return;
  }

  stateDoc.clear();
  QDomElement documentElement = stateDoc.createElement( "ComposerItemState" );
  source->writeXML( documentElement, stateDoc );
  stateDoc.appendChild( documentElement );
}

void QgsComposerItemCommand::restoreState( QDomDocument& stateDoc ) const
{
  QgsComposerItem* destItem = item();
  if ( !destItem )
  {
    return;
  }

  destItem->readXML( stateDoc.documentElement().firstChild().toElement(), stateDoc );
  destItem->repaint();
  QgsProject::instance()->dirty( true );
}

//
//QgsComposerMergeCommand
//

QgsComposerMergeCommand::QgsComposerMergeCommand( Context c, QgsComposerItem* item, const QString& text )
    : QgsComposerItemCommand( item, text )
    , mContext( c )
{
}

QgsComposerMergeCommand::~QgsComposerMergeCommand()
{
}

bool QgsComposerMergeCommand::mergeWith( const QUndoCommand * command )
{
  QgsComposerItem* thisItem = item();
  if ( !thisItem )
  {
    return false;
  }

  const QgsComposerItemCommand* c = dynamic_cast<const QgsComposerItemCommand*>( command );
  if ( !c || thisItem != c->item() )
  {
    return false;
  }

  mAfterState = c->afterState();
  return true;
}

