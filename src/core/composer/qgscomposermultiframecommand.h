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

class CORE_EXPORT QgsComposerMultiFrameCommand: public QUndoCommand
{
  public:
    QgsComposerMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, QUndoCommand* parent = 0 );
    ~QgsComposerMultiFrameCommand();

    void undo() override;
    void redo() override;

    void savePreviousState();
    void saveAfterState();

    QDomDocument previousState() const { return mPreviousState.cloneNode().toDocument(); }
    QDomDocument afterState() const { return mAfterState.cloneNode().toDocument(); }

    /** Returns true if previous state and after state are valid and different*/
    bool containsChange() const;

    const QgsComposerMultiFrame* multiFrame() const { return mMultiFrame; }

  protected:
    QgsComposerMultiFrame* mMultiFrame;

    QDomDocument mPreviousState;
    QDomDocument mAfterState;

    bool mFirstRun;

    QgsComposerMultiFrameCommand(); //forbidden
    void saveState( QDomDocument& stateDoc );
    void restoreState( QDomDocument& stateDoc );
    bool checkFirstRun();
};

/** A composer command that merges together with other commands having the same context (=id)
 * for multi frame items. Keeps the oldest previous state and uses the newest after state.
 * The purpose is to avoid too many micro changes in the history*/
class CORE_EXPORT QgsComposerMultiFrameMergeCommand: public QgsComposerMultiFrameCommand
{
  public:
    enum Context
    {
      Unknown = 0,
      //composer html
      HtmlSource,
      HtmlStylesheet,
      HtmlBreakDistance,
      //attribute table
      TableMaximumFeatures,
      TableMargin,
      TableGridStrokeWidth
    };

    QgsComposerMultiFrameMergeCommand( Context c, QgsComposerMultiFrame* multiFrame, const QString& text );
    ~QgsComposerMultiFrameMergeCommand();

    bool mergeWith( const QUndoCommand * command ) override;
    int id() const override { return ( int )mContext; }

  private:
    Context mContext;
};

#endif // QGSCOMPOSERMULTIFRAMECOMMAND_H
