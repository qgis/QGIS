/** A composer command class for grouping / ungrouping composer items.
 *
 * If mState == Ungrouped, the command owns the group item
 */
class QgsGroupUngroupItemsCommand: QObject, QUndoCommand
{
%TypeHeaderCode
#include "qgsgroupungroupitemscommand.h"
%End

  public:

    /** Command kind, and state */
    enum State
    {
      Grouped = 0,
      Ungrouped
    };

    /** Create a group or ungroup command
     *
     * @param s command kind (@see State)
     * @param item the group item being created or ungrouped
     * @param c the composition including this group
     * @param text command label
     * @param parent parent command, if any
     *
     */
    QgsGroupUngroupItemsCommand( State s, QgsComposerItemGroup* item, QgsComposition* c, const QString& text, QUndoCommand* parent = nullptr );
    ~QgsGroupUngroupItemsCommand();

    void redo();
    void undo();

  signals:
    /** Signals addition of an item (the group) */
    void itemAdded( QgsComposerItem* item );
    /** Signals removal of an item (the group) */
    void itemRemoved( QgsComposerItem* item );

};


