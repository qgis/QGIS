/***************************************************************************
                          qgscomposermultiframecommand.h
                          ------------------------------
    begin                : 2012-08-02
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

#ifndef QGSCOMPOSERMULTIFRAMECOMMAND_H
#define QGSCOMPOSERMULTIFRAMECOMMAND_H

#include <QUndoCommand>
#include <QDomDocument>

class QgsComposerMultiFrame;

class QgsComposerMultiFrameCommand: public QUndoCommand
{
  public:
    QgsComposerMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, QUndoCommand* parent = 0 );
    ~QgsComposerMultiFrameCommand();

    void undo();
    void redo();

    void savePreviousState();
    void saveAfterState();

    /**Returns true if previous state and after state are valid and different*/
    bool containsChange() const;

  private:
    QgsComposerMultiFrame* mMultiFrame;

    QDomDocument mPreviousState;
    QDomDocument mAfterState;

    bool mFirstRun;

    QgsComposerMultiFrameCommand(); //forbidden
    void saveState( QDomDocument& stateDoc );
    void restoreState( QDomDocument& stateDoc );
    bool checkFirstRun();
};

#endif // QGSCOMPOSERMULTIFRAMECOMMAND_H
