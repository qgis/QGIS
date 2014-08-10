/** \ingroup MapComposer
A composer command class for adding / removing composer items. If mState == Removed, the command owns the item*/
class QgsAddRemoveItemCommand: QObject, QUndoCommand
{
%TypeHeaderCode
  #include "qgsaddremoveitemcommand.h"
%End
  public:

    enum State
    {
      Added,
      Removed
    };

    QgsAddRemoveItemCommand( State s, QgsComposerItem* item, QgsComposition* c, const QString& text, QUndoCommand* parent = 0 );
    ~QgsAddRemoveItemCommand();

    void redo();
    void undo();

  signals:
    void itemAdded( QgsComposerItem* item );
    void itemRemoved( QgsComposerItem* item );
};
