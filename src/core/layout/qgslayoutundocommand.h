/***************************************************************************
                          qgslayoutundocommand.h
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

#ifndef QGSLAYOUTUNDOCOMMAND_H
#define QGSLAYOUTUNDOCOMMAND_H

#include <QUndoCommand>
#include "qgis_sip.h"
#include <QDomDocument>

#include "qgis_core.h"

class QgsLayout;

/**
 * \ingroup core
 * Base class for commands to undo/redo layout and layout object changes.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsAbstractLayoutUndoCommand: public QUndoCommand
{
  public:

    /**
     * Constructor for QgsLayoutUndoCommand.
     * The \a id argument can be used to specify an id number for the source event - this is used to determine whether QUndoCommand
     * command compression can apply to the command.
     */
    QgsAbstractLayoutUndoCommand( const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );

    void undo() override;
    void redo() override;
    int id() const override { return mId; }

    /**
     * Saves current layout state as before state.
     * \see beforeState()
     * \see saveAfterState()
     */
    void saveBeforeState();

    /**
     * Saves current layout state as after state.
     * \see afterState()
     * \see saveBeforeState()
     */
    void saveAfterState();

    /**
     * Returns the before state for the layout.
     * \see saveBeforeState()
     * \see afterState()
     */
    QDomDocument beforeState() const { return mBeforeState.cloneNode().toDocument(); }

    /**
     * Returns the after state for the layout.
     * \see saveAfterState()
     * \see beforeState()
     */
    QDomDocument afterState() const { return mAfterState.cloneNode().toDocument(); }

    /**
     * Returns true if both the before and after states are valid and different.
     */
    virtual bool containsChange() const;

  protected:

    /**
     * Saves the state of the object to the specified \a stateDoc.
     *
     * Subclasses must implement this to handle encapsulating their current state into a DOM document.
     *
     * \see restoreState()
     */
    virtual void saveState( QDomDocument &stateDoc ) const = 0;

    /**
     * Restores the state of the object from the specified \a stateDoc.
     *
     * Subclasses must implement this to handle restoring their current state from the encapsulated state.
     *
     * \see saveState()
     */
    virtual void restoreState( QDomDocument &stateDoc ) = 0;

    /**
     * Manually sets the after state for the command. Generally this should not be called directly.
     */
    void setAfterState( const QDomDocument &stateDoc );

    //! Flag to prevent the first redo() if the command is pushed to the undo stack
    bool mFirstRun = true;

  private:

    //! XML that saves the state before executing the command
    QDomDocument mBeforeState;

    //! XML containing the state after executing the command
    QDomDocument mAfterState;

    //! Command id
    int mId = 0;

};

/**
 * \ingroup core
 * Interface for layout objects which support undo/redo commands.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutUndoObjectInterface
{
  public:

    /**
     * Destructor for QgsLayoutUndoObjectInterface.
     */
    virtual ~QgsLayoutUndoObjectInterface() = default;

    /**
     * Creates a new layout undo command with the specified \a text and \a parent.
     *
     * The \a id argument can be used to specify an id number for the source event - this is used to determine whether QUndoCommand
     * command compression can apply to the command.
     */
    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id = 0, QUndoCommand *parent = nullptr ) = 0 SIP_FACTORY;
};


#endif // QGSLAYOUTUNDOCOMMAND_H
