/***************************************************************************
                          qgslayoutundostack.h
                          ----------------------
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

#ifndef QGSLAYOUTUNDOSTACK_H
#define QGSLAYOUTUNDOSTACK_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgslayoutundocommand.h"
#include "qgslayoutitem.h"

#include <memory>

class QgsLayout;
class QUndoStack;

/**
 * \ingroup core
 * An undo stack for QgsLayouts.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutUndoStack : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutUndoStack, for the specified parent \a layout.
     */
    QgsLayoutUndoStack( QgsLayout *layout );

    /**
     * Starts a macro command, with the given descriptive \a commandText.
     *
     * Any commands added to the stack (either via direct manipulation of
     * stack() or via beginCommand()/endCommand() calls) between a
     * beginMacro() and endMacro() block are collapsed into a single
     * undo command, which will be applied or rolled back in a single step.
     *
     * \see endMacro()
     */
    void beginMacro( const QString &commandText );

    /**
     * Ends a macro command. This must be called after beginMacro(), when
     * all child undo commands which form part of the macro have been completed.
     *
     * Any commands added to the stack (either via direct manipulation of
     * stack() or via beginCommand()/endCommand() calls) between a
     * beginMacro() and endMacro() block are collapsed into a single
     * undo command, which will be applied or rolled back in a single step.
     *
     * \see beginMacro()
     */
    void endMacro();

    /**
     * Begins a new undo command for the specified \a object.
     *
     * This must be followed by a call to endCommand() or cancelCommand() after the desired changes
     * have been made to \a object.
     *
     * The \a id argument can be used to specify an id number for the source event - this is used to determine whether QUndoCommand
     * command compression can apply to the command.
     *
     * \see endCommand()
     * \see cancelCommand()
     */
    void beginCommand( QgsLayoutUndoObjectInterface *object, const QString &commandText, int id = 0 );

    /**
     * Saves final state of an object and pushes the active command to the undo history.
     * \see beginCommand()
     * \see cancelCommand()
     */
    void endCommand();

    /**
     * Cancels the active command, discarding it without pushing to the undo history.
     * \see endCommand()
     * \see cancelCommand()
     */
    void cancelCommand();

    /**
     * Returns a pointer to the internal QUndoStack.
     */
    QUndoStack *stack();

    /**
     * Notifies the stack that an undo or redo action occurred for a specified \a item.
     */
    void notifyUndoRedoOccurred( QgsLayoutItem *item );

    /**
     * Sets whether undo commands for the layout should be temporarily blocked.
     *
     * If \a blocked is TRUE, subsequent undo commands will be blocked until a follow-up
     * call to blockCommands( FALSE ) is made.
     *
     * Note that calls to blockCommands are stacked, so two calls blocking the commands
     * will take two calls unblocking commands in order to release the block.
     *
     * \see isBlocked()
     */
    void blockCommands( bool blocked );

    /**
     * Returns TRUE if undo commands are currently blocked.
     * \see blockCommands()
     */
    bool isBlocked() const;

    /**
     * Manually pushes a \a command to the stack, and takes ownership of the command.
     */
    void push( QUndoCommand *command SIP_TRANSFER );

  signals:

    /**
     * Emitted when an undo or redo action has occurred, which affected a
     * set of layout \a itemUuids.
     */
    void undoRedoOccurredForItems( QSet< QString > itemUuids );

  private slots:

    void indexChanged();

  private:

    QgsLayout *mLayout = nullptr;

    std::unique_ptr< QUndoStack > mUndoStack;

    std::vector< std::unique_ptr< QgsAbstractLayoutUndoCommand > > mActiveCommands;

    QSet< QString > mUndoRedoOccurredItemUuids;

    int mBlockedCommands = 0;

#ifdef SIP_RUN
    QgsLayoutUndoStack( const QgsLayoutUndoStack &other );
#endif
};

#endif // QGSLAYOUTUNDOCOMMAND_H
