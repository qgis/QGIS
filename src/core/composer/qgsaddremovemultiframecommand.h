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

class QgsComposerMultiFrame;
class QgsComposition;

/** \ingroup core
 * \class QgsAddRemoveMultiFrameCommand
 */
class CORE_EXPORT QgsAddRemoveMultiFrameCommand: public QUndoCommand
{
  public:

    enum State
    {
      Added = 0,
      Removed
    };

    QgsAddRemoveMultiFrameCommand( State s, QgsComposerMultiFrame* multiFrame, QgsComposition* c, const QString& text, QUndoCommand* parent = nullptr );
    ~QgsAddRemoveMultiFrameCommand();
    void redo() override;
    void undo() override;

  private:
    QgsAddRemoveMultiFrameCommand();

    //changes between added / removed state
    void switchState();
    bool checkFirstRun();

    QgsComposerMultiFrame* mMultiFrame;
    QgsComposition* mComposition;
    State mState;
    bool mFirstRun;
};

#endif // QGSADDREMOVEMULTIFRAMECOMMAND_H
