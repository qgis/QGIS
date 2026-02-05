/***************************************************************************
                             qgsmodelundocommand.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELUNDOCOMMAND_H
#define QGSMODELUNDOCOMMAND_H

#include "qgis.h"
#include "qgis_gui.h"

#include <QUndoCommand>

class QgsProcessingModelAlgorithm;

#define SIP_NO_FILE

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief A undo command for the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelUndoCommand : public QUndoCommand
{
  public:
    enum class CommandOperation
    {
      Unknown = 0,      //!< Unknown operation
      NameChanged = 1,  //!< Model name changed
      GroupChanged = 2, //!< Model group changed
    };

    /**
     * Constructor for QgsModelUndoCommand.
     *
     * This variant accepts a CommandOperation enum for collapsing commands.
    */
    QgsModelUndoCommand( QgsProcessingModelAlgorithm *model, const QString &text, CommandOperation operation = CommandOperation::Unknown, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsModelUndoCommand.
     *
     * This variant accepts an arbitrary string value for collapsing commands.
     */
    QgsModelUndoCommand( QgsProcessingModelAlgorithm *model, const QString &text, const QString &idString, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Saves the "after" state of the model. Should be called after making the actual associated change within the model.
     */
    void saveAfterState();

    int id() const override;
    void undo() override;
    void redo() override;
    bool mergeWith( const QUndoCommand *other ) override;

    /**
     * Returns the command operation.
     */
    CommandOperation operation() const { return mOperation; }

    /**
     * Returns the (optional) string value ID for collapsing commands.
     */
    QString idString() const { return mIdString; }

  private:
    //! Flag to prevent the first redo() if the command is pushed to the undo stack
    bool mFirstRun = true;

    QgsProcessingModelAlgorithm *mModel = nullptr;
    QVariant mBeforeState;
    QVariant mAfterState;
    CommandOperation mOperation = CommandOperation::Unknown;
    QString mIdString;
};

///@endcond

#endif // QGSMODELUNDOCOMMAND_H
