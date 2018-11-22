/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutundocommand.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAbstractLayoutUndoCommand: QUndoCommand
{
%Docstring
Base class for commands to undo/redo layout and layout object changes.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutundocommand.h"
%End
  public:

    QgsAbstractLayoutUndoCommand( const QString &text, int id = 0, QUndoCommand *parent /TransferThis/ = 0 );
%Docstring
Constructor for :py:class:`QgsLayoutUndoCommand`.
The ``id`` argument can be used to specify an id number for the source event - this is used to determine whether QUndoCommand
command compression can apply to the command.
%End

    virtual void undo();

    virtual void redo();

    virtual int id() const;

    void saveBeforeState();
%Docstring
Saves current layout state as before state.

.. seealso:: :py:func:`beforeState`

.. seealso:: :py:func:`saveAfterState`
%End

    void saveAfterState();
%Docstring
Saves current layout state as after state.

.. seealso:: :py:func:`afterState`

.. seealso:: :py:func:`saveBeforeState`
%End

    QDomDocument beforeState() const;
%Docstring
Returns the before state for the layout.

.. seealso:: :py:func:`saveBeforeState`

.. seealso:: :py:func:`afterState`
%End

    QDomDocument afterState() const;
%Docstring
Returns the after state for the layout.

.. seealso:: :py:func:`saveAfterState`

.. seealso:: :py:func:`beforeState`
%End

    virtual bool containsChange() const;
%Docstring
Returns true if both the before and after states are valid and different.
%End

  protected:

    virtual void saveState( QDomDocument &stateDoc ) const = 0;
%Docstring
Saves the state of the object to the specified ``stateDoc``.

Subclasses must implement this to handle encapsulating their current state into a DOM document.

.. seealso:: :py:func:`restoreState`
%End

    virtual void restoreState( QDomDocument &stateDoc ) = 0;
%Docstring
Restores the state of the object from the specified ``stateDoc``.

Subclasses must implement this to handle restoring their current state from the encapsulated state.

.. seealso:: :py:func:`saveState`
%End

    void setAfterState( const QDomDocument &stateDoc );
%Docstring
Manually sets the after state for the command. Generally this should not be called directly.
%End


};

class QgsLayoutUndoObjectInterface
{
%Docstring
Interface for layout objects which support undo/redo commands.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutundocommand.h"
%End
  public:

    virtual ~QgsLayoutUndoObjectInterface();

    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id = 0, QUndoCommand *parent = 0 ) = 0 /Factory/;
%Docstring
Creates a new layout undo command with the specified ``text`` and ``parent``.

The ``id`` argument can be used to specify an id number for the source event - this is used to determine whether QUndoCommand
command compression can apply to the command.
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutundocommand.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
