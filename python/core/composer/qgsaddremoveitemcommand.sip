/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsaddremoveitemcommand.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAddRemoveItemCommand: QObject, QUndoCommand
{
%Docstring
A composer command class for adding / removing composer items. If mState == Removed, the command owns the item
%End

%TypeHeaderCode
#include "qgsaddremoveitemcommand.h"
%End
  public:

    enum State
    {
      Added,
      Removed
    };

    QgsAddRemoveItemCommand( State s, QgsComposerItem *item, QgsComposition *c, const QString &text, QUndoCommand *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsAddRemoveItemCommand
%End
    ~QgsAddRemoveItemCommand();

    virtual void redo();

    virtual void undo();


  signals:
    void itemAdded( QgsComposerItem *item );
    void itemRemoved( QgsComposerItem *item );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsaddremoveitemcommand.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
