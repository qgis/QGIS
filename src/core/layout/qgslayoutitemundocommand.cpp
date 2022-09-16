/***************************************************************************
                          qgslayoutitemundocommand.cpp
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

#include "qgslayoutitemundocommand.h"
#include "qgslayoutitem.h"
#include "qgsreadwritecontext.h"
#include "qgslayout.h"
#include "qgsproject.h"
#include "qgslayoutundostack.h"

///@cond PRIVATE
QgsLayoutItemUndoCommand::QgsLayoutItemUndoCommand( QgsLayoutItem *item, const QString &text, int id, QUndoCommand *parent )
  : QgsAbstractLayoutUndoCommand( text, id, parent )
  , mItemUuid( item->uuid() )
  , mLayout( item->layout() )
  , mItemType( item->type() )
{

}

bool QgsLayoutItemUndoCommand::mergeWith( const QUndoCommand *command )
{
  if ( command->id() == 0 )
    return false;

  const QgsLayoutItemUndoCommand *c = dynamic_cast<const QgsLayoutItemUndoCommand *>( command );
  if ( !c )
  {
    return false;
  }
  if ( c->itemUuid() != itemUuid() )
    return false;

  setAfterState( c->afterState() );
  return true;
}

void QgsLayoutItemUndoCommand::saveState( QDomDocument &stateDoc ) const
{
  stateDoc.clear();
  QDomElement documentElement = stateDoc.createElement( QStringLiteral( "ItemState" ) );

  QgsLayoutItem *item = mLayout->itemByUuid( mItemUuid );
  if ( item )
  {
    item->writeXml( documentElement, stateDoc, QgsReadWriteContext() );
    stateDoc.appendChild( documentElement );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "QgsLayoutItemUndoCommand::saveState: could not retrieve item %1 for saving state" ).arg( mItemUuid ) );
  }
}

void QgsLayoutItemUndoCommand::restoreState( QDomDocument &stateDoc )
{
  // find item by uuid...
  QgsLayoutItem *item = mLayout->itemByUuid( mItemUuid );
  if ( !item )
  {
    // uh oh - it's been deleted! we need to create a new instance
    item = recreateItem( mItemType, mLayout );
  }

  item->readXml( stateDoc.documentElement().firstChild().toElement(), stateDoc, QgsReadWriteContext() );
  item->finalizeRestoreFromXml();
  mLayout->project()->setDirty( true );
  mLayout->undoStack()->notifyUndoRedoOccurred( item );
}

QgsLayoutItem *QgsLayoutItemUndoCommand::recreateItem( int itemType, QgsLayout *layout )
{
  QgsLayoutItem *item = QgsApplication::layoutItemRegistry()->createItem( itemType, layout );
  mLayout->addLayoutItemPrivate( item );
  return item;
}

QString QgsLayoutItemUndoCommand::itemUuid() const
{
  return mItemUuid;
}

QgsLayout *QgsLayoutItemUndoCommand::layout() const
{
  return mLayout;
}


//
// QgsLayoutItemDeleteUndoCommand
//

QgsLayoutItemDeleteUndoCommand::QgsLayoutItemDeleteUndoCommand( QgsLayoutItem *item, const QString &text, int id, QUndoCommand *parent )
  : QgsLayoutItemUndoCommand( item, text, id, parent )
{
  saveBeforeState();
}

bool QgsLayoutItemDeleteUndoCommand::mergeWith( const QUndoCommand * )
{
  return false;
}

void QgsLayoutItemDeleteUndoCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }

  QgsLayoutItem *item = layout()->itemByUuid( itemUuid() );
  //Q_ASSERT_X( item, "QgsLayoutItemDeleteUndoCommand::redo", "could not find item to re-delete!" );

  layout()->undoStack()->blockCommands( true );
  if ( item )
    layout()->removeLayoutItemPrivate( item );
  layout()->undoStack()->blockCommands( false );
}

QgsLayoutItemAddItemCommand::QgsLayoutItemAddItemCommand( QgsLayoutItem *item, const QString &text, int id, QUndoCommand *parent )
  : QgsLayoutItemUndoCommand( item, text, id, parent )
{
  saveAfterState();
}

bool QgsLayoutItemAddItemCommand::containsChange() const
{
  return true;
}

bool QgsLayoutItemAddItemCommand::mergeWith( const QUndoCommand * )
{
  return false;
}

void QgsLayoutItemAddItemCommand::undo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }

  QgsLayoutItem *item = layout()->itemByUuid( itemUuid() );
  if ( item )
  {
    layout()->removeLayoutItemPrivate( item );
  }
}


///@endcond
