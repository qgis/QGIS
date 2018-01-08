/***************************************************************************
                          qgsaddremovemultiframecommand.h
                          -------------------------------
    begin                : 2012-07-31
    copyright            : (C) 2012 by Marco Hugentobler
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

#ifndef QGSADDREMOVEMULTIFRAMECOMMAND_H
#define QGSADDREMOVEMULTIFRAMECOMMAND_H

#include <QUndoCommand>
#include "qgis.h"

#include "qgis_core.h"

#define SIP_NO_FILE

class QgsComposerMultiFrame;
class QgsComposition;

/**
 * \ingroup core
 * \class QgsAddRemoveMultiFrameCommand
 * \note Not available in Python bindings
 * \deprecated Will be removed in QGIS 3.2
 */
class CORE_EXPORT QgsAddRemoveMultiFrameCommand: public QUndoCommand
{
  public:

    enum State
    {
      Added = 0,
      Removed
    };

    //! Constructor for QgsAddRemoveMultiFrameCommand
    QgsAddRemoveMultiFrameCommand( State s, QgsComposerMultiFrame *multiFrame, QgsComposition *c, const QString &text, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsAddRemoveMultiFrameCommand() override;
    void redo() override;
    void undo() override;

  private:
    QgsAddRemoveMultiFrameCommand() = delete;

    //changes between added / removed state
    void switchState();
    bool checkFirstRun();

    QgsComposerMultiFrame *mMultiFrame = nullptr;
    QgsComposition *mComposition = nullptr;
    State mState = Added;
    bool mFirstRun = true;
};

#endif // QGSADDREMOVEMULTIFRAMECOMMAND_H
