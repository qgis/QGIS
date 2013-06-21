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
#include "qgsproject.h"

QgsComposerItemCommand::QgsComposerItemCommand( QgsComposerItem* item, const QString& text, QUndoCommand* parent ):
    QUndoCommand( text, parent ), mItem( item ), mFirstRun( true )
{
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
  if ( mItem )
  {
    stateDoc.clear();
    QDomElement documentElement = stateDoc.createElement( "ComposerItemState" );
    mItem->writeXML( documentElement, stateDoc );
    stateDoc.appendChild( documentElement );
  }
}

void QgsComposerItemCommand::restoreState( QDomDocument& stateDoc ) const
{
  if ( mItem )
  {
    mItem->readXML( stateDoc.documentElement().firstChild().toElement(), stateDoc );
    mItem->repaint();
    QgsProject::instance()->dirty( true );
  }
}

QgsComposerMergeCommand::QgsComposerMergeCommand( Context c, QgsComposerItem* item, const QString& text ): QgsComposerItemCommand( item, text ), mContext( c )
{
}

QgsComposerMergeCommand::~QgsComposerMergeCommand()
{
}

bool QgsComposerMergeCommand::mergeWith( const QUndoCommand * command )
{
  const QgsComposerItemCommand* c = dynamic_cast<const QgsComposerItemCommand*>( command );
  if ( !c || mItem != c->item() )
  {
    return false;
  }
  mAfterState = c->afterState();
  return true;
}

