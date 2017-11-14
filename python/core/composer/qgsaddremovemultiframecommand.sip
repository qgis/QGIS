/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsaddremovemultiframecommand.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAddRemoveMultiFrameCommand: QUndoCommand
{

%TypeHeaderCode
#include "qgsaddremovemultiframecommand.h"
%End
  public:

    enum State
    {
      Added,
      Removed
    };

    QgsAddRemoveMultiFrameCommand( State s, QgsComposerMultiFrame *multiFrame, QgsComposition *c, const QString &text, QUndoCommand *parent /TransferThis/ = 0 );
    ~QgsAddRemoveMultiFrameCommand();
    virtual void redo();

    virtual void undo();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsaddremovemultiframecommand.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
