/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutundostack.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutUndoStack
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

    void endMacro();

    void beginCommand( QgsLayoutUndoObjectInterface *object, const QString &commandText, int id = 0 );
%Docstring
 Begins a new undo command for the specified ``object``.

 This must be followed by a call to endCommand() or cancelCommand() after the desired changes
 have been made to ``object``.

 The ``id`` argument can be used to specify an id number for the source event - this is used to determine whether QUndoCommand
 command compression can apply to the command.

.. seealso:: endCommand()
.. seealso:: cancelCommand()
%End

    void endCommand();
%Docstring
 Saves final state of an object and pushes the active command to the undo history.
.. seealso:: beginCommand()
.. seealso:: cancelCommand()
%End

    void cancelCommand();
%Docstring
 Cancels the active command, discarding it without pushing to the undo history.
.. seealso:: endCommand()
.. seealso:: cancelCommand()
%End

    QUndoStack *stack();
%Docstring
 Returns a pointer to the internal QUndoStack.
 :rtype: QUndoStack
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
