class QgsComposerMultiFrameCommand : QUndoCommand
{
%TypeHeaderCode
#include <qgscomposermultiframecommand.h>
%End
  public:
    QgsComposerMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, QUndoCommand* parent /TransferThis/ = 0 );
    ~QgsComposerMultiFrameCommand();

    void undo();
    void redo();

    void savePreviousState();
    void saveAfterState();

    QDomDocument previousState() const;
    QDomDocument afterState() const;

    /**Returns true if previous state and after state are valid and different*/
    bool containsChange() const;

    const QgsComposerMultiFrame* multiFrame() const;

  protected:

    QgsComposerMultiFrameCommand(); //forbidden
    void saveState( QDomDocument& stateDoc );
    void restoreState( QDomDocument& stateDoc );
    bool checkFirstRun();
};


/**A composer command that merges together with other commands having the same context (=id)
 * for multi frame items. Keeps the oldest previous state and uses the newest after state.
 * The purpose is to avoid too many micro changes in the history*/
class QgsComposerMultiFrameMergeCommand: QgsComposerMultiFrameCommand
{
%TypeHeaderCode
#include <qgscomposermultiframecommand.h>
%End
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

    bool mergeWith( const QUndoCommand * command );
    int id() const;

};

