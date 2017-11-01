/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsgroupungroupitemscommand.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGroupUngroupItemsCommand: QObject, QUndoCommand
{
%Docstring
 A composer command class for grouping / ungrouping composer items.

 If mState == Ungrouped, the command owns the group item
%End

%TypeHeaderCode
#include "qgsgroupungroupitemscommand.h"
%End
  public:

    enum State
    {
      Grouped,
      Ungrouped
    };

    QgsGroupUngroupItemsCommand( State s, QgsComposerItemGroup *item, QgsComposition *c, const QString &text, QUndoCommand *parent = 0 );
%Docstring
 Create a group or ungroup command

.. seealso:: State)
 \param item the group item being created or ungrouped
 \param c the composition including this group
 \param text command label
 \param parent parent command, if any
%End
    ~QgsGroupUngroupItemsCommand();

    virtual void redo();

    virtual void undo();


  signals:
    void itemAdded( QgsComposerItem *item );
%Docstring
Signals addition of an item (the group)
%End
    void itemRemoved( QgsComposerItem *item );
%Docstring
Signals removal of an item (the group)
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsgroupungroupitemscommand.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
