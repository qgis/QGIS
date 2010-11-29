/***************************************************************************
                          qgsaddremoveitemcommand.h
                          ------------------------
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

#ifndef QGSADDREMOVEITEMCOMMAND_H
#define QGSADDREMOVEITEMCOMMAND_H

#include <QUndoCommand>
class QgsComposerItem;
class QgsComposition;

/**A composer command class for adding / removing composer items. If mState == Removed, the command owns the item*/
class CORE_EXPORT QgsAddRemoveItemCommand: public QObject, public QUndoCommand
{
    Q_OBJECT

  public:

    enum State
    {
      Added = 0,
      Removed
    };

    QgsAddRemoveItemCommand( State s, QgsComposerItem* item, QgsComposition* c, const QString& text, QUndoCommand* parent = 0 );
    ~QgsAddRemoveItemCommand();

    void redo();
    void undo();

  signals:
    void itemAdded( QgsComposerItem* item );
    void itemRemoved( QgsComposerItem* item );

  private:
    QgsComposerItem* mItem;
    QgsComposition* mComposition;
    State mState;
    bool mFirstRun; //flag to prevent execution when the command is pushed to the QUndoStack

    //changes between added / removed state
    void switchState();
};

#endif // QGSADDREMOVEITEMCOMMAND_H
