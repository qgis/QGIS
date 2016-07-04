/***************************************************************************
                          qgsgroupungroupitemscommand.h
                          ------------------------
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

#ifndef QGSGROUPUNGROUPITEMSCOMMAND_H
#define QGSGROUPUNGROUPITEMSCOMMAND_H

#include <QUndoCommand>
#include "qgscomposeritemgroup.h"
class QgsComposerItem;
class QgsComposition;

/** \ingroup core
 * A composer command class for grouping / ungrouping composer items.
 *
 * If mState == Ungrouped, the command owns the group item
 */
class CORE_EXPORT QgsGroupUngroupItemsCommand: public QObject, public QUndoCommand
{
    Q_OBJECT

  public:

    /** Command kind, and state */
    enum State
    {
      Grouped = 0,
      Ungrouped
    };

    /** Create a group or ungroup command
     *
     * @param s command kind (@see State)
     * @param item the group item being created or ungrouped
     * @param c the composition including this group
     * @param text command label
     * @param parent parent command, if any
     *
     */
    QgsGroupUngroupItemsCommand( State s, QgsComposerItemGroup* item, QgsComposition* c, const QString& text, QUndoCommand* parent = nullptr );
    ~QgsGroupUngroupItemsCommand();

    void redo() override;
    void undo() override;

  signals:
    /** Signals addition of an item (the group) */
    void itemAdded( QgsComposerItem* item );
    /** Signals removal of an item (the group) */
    void itemRemoved( QgsComposerItem* item );

  private:
    QgsComposerItemGroup* mGroup;
    QSet<QgsComposerItem*> mItems;
    QgsComposition* mComposition;
    State mState;
    bool mFirstRun; //flag to prevent execution when the command is pushed to the QUndoStack

    //changes between added / removed state
    void switchState();
};

#endif // QGSGROUPUNGROUPITEMSCOMMAND_H
