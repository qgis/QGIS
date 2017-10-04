/***************************************************************************
                          qgslayoutitemundocommand.h
                          ----------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMUNDOCOMMAND_H
#define QGSLAYOUTITEMUNDOCOMMAND_H

#include "qgslayoutundocommand.h"
#include "qgis_core.h"

class QgsLayout;
class QgsLayoutItem;

SIP_NO_FILE

///@cond PRIVATE

/**
 * \ingroup core
 * An undo command subclass for layout item undo commands.
 *
 * QgsLayoutItemUndoCommand is a specific layout undo command which is
 * designed for use with QgsLayoutItems. It automatically handles
 * recreating a deleted item when the undo stack rolls back past
 * the item deletion command.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutItemUndoCommand: public QgsAbstractLayoutUndoCommand
{
  public:

    /**
     * Constructor for QgsLayoutItemUndoCommand.
     * \param item associated layout item
     * \param text undo command descriptive text
     * \param id optional undo command id, used for automatic command merging
     * \param parent command
     */
    QgsLayoutItemUndoCommand( QgsLayoutItem *item, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );

    bool mergeWith( const QUndoCommand *command ) override;

    /**
     * Returns the layout associated with this command.
     */
    QgsLayout *layout() const;

    /**
     * Returns the associated item's UUID, which uniquely identifies the item
     * within the layout.
     */
    QString itemUuid() const;

  protected:

    void saveState( QDomDocument &stateDoc ) const override;
    void restoreState( QDomDocument &stateDoc ) override;

    virtual QgsLayoutItem *recreateItem( int itemType, QgsLayout *layout ) SIP_FACTORY;

  private:

    QString mItemUuid;
    QgsLayout *mLayout = nullptr;
    int mItemType = 0;

};

/**
 * \ingroup core
 * An undo command subclass for layout item deletion undo commands.
 *
 * QgsLayoutItemDeleteUndoCommand is a specific layout undo command which handles
 * layout item deletion. When applied (e.g. as a result of a 'redo' action),
 * the associated layout item is deleted and removed from the layout.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutItemDeleteUndoCommand: public QgsLayoutItemUndoCommand
{
  public:

    /**
     * Constructor for QgsLayoutItemDeleteUndoCommand.
     * \param item associated layout item
     * \param text undo command descriptive text
     * \param id optional undo command id, used for automatic command merging
     * \param parent command
     */
    QgsLayoutItemDeleteUndoCommand( QgsLayoutItem *item, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );
    bool mergeWith( const QUndoCommand *command ) override;
    void redo() override;

};


/**
 * \ingroup core
 * An undo command subclass for layout item addition undo commands.
 *
 * QgsLayoutItemAddItemCommand is a specific layout undo command which handles
 * layout item creation. When applied (e.g. as a result of a 'redo' action),
 * the associated layout item is recreated and added to the layout.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutItemAddItemCommand: public QgsLayoutItemUndoCommand
{
  public:

    /**
     * Constructor for QgsLayoutItemAddItemCommand.
     * \param item associated layout item
     * \param text undo command descriptive text
     * \param id optional undo command id, used for automatic command merging
     * \param parent command
     */
    QgsLayoutItemAddItemCommand( QgsLayoutItem *item, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );
    bool containsChange() const override;
    bool mergeWith( const QUndoCommand *command ) override;
    void undo() override;

};

///@endcond

#endif
