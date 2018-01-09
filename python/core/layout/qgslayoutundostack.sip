/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutundostack.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLayoutUndoStack : QObject
{
%Docstring
An undo stack for QgsLayouts.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutundostack.h"
%End
  public:

    QgsLayoutUndoStack( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutUndoStack, for the specified parent ``layout``.
%End

    void beginMacro( const QString &commandText );
%Docstring
Starts a macro command, with the given descriptive ``commandText``.

Any commands added to the stack (either via direct manipulation of
stack() or via beginCommand()/endCommand() calls) between a
beginMacro() and endMacro() block are collapsed into a single
undo command, which will be applied or rolled back in a single step.

.. seealso:: :py:func:`endMacro`
%End

    void endMacro();
%Docstring
Ends a macro command. This must be called after beginMacro(), when
all child undo commands which form part of the macro have been completed.

Any commands added to the stack (either via direct manipulation of
stack() or via beginCommand()/endCommand() calls) between a
beginMacro() and endMacro() block are collapsed into a single
undo command, which will be applied or rolled back in a single step.

.. seealso:: :py:func:`beginMacro`
%End

    void beginCommand( QgsLayoutUndoObjectInterface *object, const QString &commandText, int id = 0 );
%Docstring
Begins a new undo command for the specified ``object``.

This must be followed by a call to endCommand() or cancelCommand() after the desired changes
have been made to ``object``.

The ``id`` argument can be used to specify an id number for the source event - this is used to determine whether QUndoCommand
command compression can apply to the command.

.. seealso:: :py:func:`endCommand`

.. seealso:: :py:func:`cancelCommand`
%End

    void endCommand();
%Docstring
Saves final state of an object and pushes the active command to the undo history.

.. seealso:: :py:func:`beginCommand`

.. seealso:: :py:func:`cancelCommand`
%End

    void cancelCommand();
%Docstring
Cancels the active command, discarding it without pushing to the undo history.

.. seealso:: :py:func:`endCommand`

.. seealso:: :py:func:`cancelCommand`
%End

    QUndoStack *stack();
%Docstring
Returns a pointer to the internal QUndoStack.
%End

    void notifyUndoRedoOccurred( QgsLayoutItem *item );
%Docstring
Notifies the stack that an undo or redo action occurred for a specified ``item``.
%End

    void blockCommands( bool blocked );
%Docstring
Sets whether undo commands for the layout should be temporarily blocked.

If ``blocked`` is true, subsequent undo commands will be blocked until a follow-up
call to blockCommands( false ) is made.

Note that calls to blockCommands are stacked, so two calls blocking the commands
will take two calls unblocking commands in order to release the block.

.. seealso:: :py:func:`isBlocked`
%End

    bool isBlocked() const;
%Docstring
Returns true if undo commands are currently blocked.

.. seealso:: :py:func:`blockCommands`
%End

    void push( QUndoCommand *command /Transfer/ );
%Docstring
Manually pushes a ``command`` to the stack, and takes ownership of the command.
%End

  signals:

    void undoRedoOccurredForItems( const QSet< QString > itemUuids );
%Docstring
Emitted when an undo or redo action has occurred, which affected a
set of layout ``itemUuids``.
%End

  private:
    QgsLayoutUndoStack( const QgsLayoutUndoStack &other );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutundostack.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
