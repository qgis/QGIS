/***************************************************************************
                          qgslayoutundostack.cpp
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

#include "qgslayoutundostack.h"
#include "qgslayout.h"
#include "qgsproject.h"
#include <QUndoStack>

QgsLayoutUndoStack::QgsLayoutUndoStack( QgsLayout *layout )
  : mLayout( layout )
  , mUndoStack( new QUndoStack( layout ) )
{

}

void QgsLayoutUndoStack::beginMacro( const QString &commandText )
{
  mUndoStack->beginMacro( commandText );
}

void QgsLayoutUndoStack::endMacro()
{
  mUndoStack->endMacro();
}

void QgsLayoutUndoStack::beginCommand( QgsLayoutUndoObjectInterface *object, const QString &commandText, int id )
{
  if ( !object )
  {
    return;
  }

  mActiveCommand.reset( object->createCommand( commandText, id, nullptr ) );
  mActiveCommand->saveBeforeState();
}

void QgsLayoutUndoStack::endCommand()
{
  if ( !mActiveCommand )
    return;

  mActiveCommand->saveAfterState();
  if ( mActiveCommand->containsChange() ) //protect against empty commands
  {
    mUndoStack->push( mActiveCommand.release() );

    mLayout->project()->setDirty( true );
  }
}

void QgsLayoutUndoStack::cancelCommand()
{
  mActiveCommand.reset();
}

QUndoStack *QgsLayoutUndoStack::stack()
{
  return mUndoStack.get();

}
