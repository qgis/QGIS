class QgsComposerMultiFrameCommand : QUndoCommand
{
%TypeHeaderCode
#include <qgscomposermultiframecommand.h>
%End
  public:
    QgsComposerMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, QUndoCommand* parent = 0 );
    ~QgsComposerMultiFrameCommand();

    void undo();
    void redo();

    void savePreviousState();
    void saveAfterState();

    /**Returns true if previous state and after state are valid and different*/
    bool containsChange() const;
};
