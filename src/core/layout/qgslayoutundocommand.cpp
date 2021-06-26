/***************************************************************************
                          qgslayoutundocommand.cpp
                          ------------------------
    begin                : July 2017
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

#include "qgslayoutundocommand.h"


QgsAbstractLayoutUndoCommand::QgsAbstractLayoutUndoCommand( const QString &text, int id, QUndoCommand *parent )
  : QUndoCommand( text, parent )
  , mId( id )
{}

void QgsAbstractLayoutUndoCommand::undo()
{
  QUndoCommand::undo();
  restoreState( mBeforeState );
}

void QgsAbstractLayoutUndoCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  QUndoCommand::redo();
  restoreState( mAfterState );
}

void QgsAbstractLayoutUndoCommand::saveBeforeState()
{
  saveState( mBeforeState );
}

void QgsAbstractLayoutUndoCommand::saveAfterState()
{
  saveState( mAfterState );
}

bool QgsAbstractLayoutUndoCommand::containsChange() const
{
  return !( mBeforeState.isNull() || mAfterState.isNull() || mBeforeState.toString() == mAfterState.toString() );
}

void QgsAbstractLayoutUndoCommand::setAfterState( const QDomDocument &stateDoc )
{
  mAfterState = stateDoc;
}
