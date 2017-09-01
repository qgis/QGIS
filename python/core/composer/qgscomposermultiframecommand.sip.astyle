/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermultiframecommand.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsComposerMultiFrameCommand: QUndoCommand
{

%TypeHeaderCode
#include "qgscomposermultiframecommand.h"
%End
  public:
    QgsComposerMultiFrameCommand( QgsComposerMultiFrame *multiFrame, const QString &text, QUndoCommand *parent /TransferThis/ = 0 );

    virtual void undo();

    virtual void redo();


    void savePreviousState();
    void saveAfterState();

    QDomDocument previousState() const;
%Docstring
 :rtype: QDomDocument
%End
    QDomDocument afterState() const;
%Docstring
 :rtype: QDomDocument
%End

    bool containsChange() const;
%Docstring
Returns true if previous state and after state are valid and different
 :rtype: bool
%End

    const QgsComposerMultiFrame *multiFrame() const;
%Docstring
 :rtype: QgsComposerMultiFrame
%End

  protected:



    QgsComposerMultiFrameCommand(); //forbidden
    void saveState( QDomDocument &stateDoc );
    void restoreState( QDomDocument &stateDoc );
    bool checkFirstRun();
%Docstring
 :rtype: bool
%End
};

class QgsComposerMultiFrameMergeCommand: QgsComposerMultiFrameCommand
{
%Docstring
 A composer command that merges together with other commands having the same context (=id)
 for multi frame items. Keeps the oldest previous state and uses the newest after state.
 The purpose is to avoid too many micro changes in the history*
%End

%TypeHeaderCode
#include "qgscomposermultiframecommand.h"
%End
  public:
    enum Context
    {
      Unknown,
      //composer
      HtmlSource,
      HtmlStylesheet,
      HtmlBreakDistance,
      //attribute
      TableMaximumFeatures,
      TableMargin,
      TableGridStrokeWidth,
      TableCellStyle,
      TableHeaderFontColor,
      TableContentFontColor,
      TableGridColor,
      TableBackgroundColor,
    };

    QgsComposerMultiFrameMergeCommand( Context c, QgsComposerMultiFrame *multiFrame, const QString &text );

    virtual bool mergeWith( const QUndoCommand *command );

    virtual int id() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermultiframecommand.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
