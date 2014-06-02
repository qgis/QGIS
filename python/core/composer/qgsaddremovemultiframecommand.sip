class QgsAddRemoveMultiFrameCommand : QUndoCommand
{
%TypeHeaderCode
#include <qgsaddremovemultiframecommand.h>
%End

  public:

    enum State
    {
      Added,
      Removed
    };

    QgsAddRemoveMultiFrameCommand( State s, QgsComposerMultiFrame* multiFrame, QgsComposition* c, const QString& text, QUndoCommand* parent = 0 );
    ~QgsAddRemoveMultiFrameCommand();
    void redo();
    void undo();
};
