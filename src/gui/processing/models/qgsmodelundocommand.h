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
    QgsModelUndoCommand( QgsProcessingModelAlgorithm *model, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Saves the "after" state of the model. Should be called after making the actual associated change within the model.
     */
    void saveAfterState();

    int id() const override;
    void undo() override;
    void redo() override;
    bool mergeWith( const QUndoCommand *other ) override;

  private:
    //! Flag to prevent the first redo() if the command is pushed to the undo stack
    bool mFirstRun = true;

    QgsProcessingModelAlgorithm *mModel = nullptr;
    QVariant mBeforeState;
    QVariant mAfterState;
    int mId = 0;
};

///@endcond

#endif // QGSMODELUNDOCOMMAND_H
