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
  connect( mUndoStack.get(), &QUndoStack::indexChanged, this, &QgsLayoutUndoStack::indexChanged );
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

  mActiveCommands.emplace_back( std::unique_ptr< QgsAbstractLayoutUndoCommand >( object->createCommand( commandText, id, nullptr ) ) );
  mActiveCommands.back()->saveBeforeState();
}

void QgsLayoutUndoStack::endCommand()
{
  if ( mActiveCommands.empty() )
    return;

  mActiveCommands.back()->saveAfterState();
  if ( mActiveCommands.back()->containsChange() ) //protect against empty commands
  {
    mUndoStack->push( mActiveCommands.back().release() );
    mActiveCommands.pop_back();

    mLayout->project()->setDirty( true );
  }
}

void QgsLayoutUndoStack::cancelCommand()
{
  if ( mActiveCommands.empty() )
    return;

  mActiveCommands.pop_back();
}

QUndoStack *QgsLayoutUndoStack::stack()
{
  return mUndoStack.get();
}

void QgsLayoutUndoStack::notifyUndoRedoOccurred( QgsLayoutItem *item )
{
  mUndoRedoOccurredItemUuids.insert( item->uuid() );
}

void QgsLayoutUndoStack::indexChanged()
{
  if ( mUndoRedoOccurredItemUuids.empty() )
    return;

  emit undoRedoOccurredForItems( mUndoRedoOccurredItemUuids );
  mUndoRedoOccurredItemUuids.clear();
}
