/***************************************************************************
                          qgslayoutmultiframeundocommand.h
                          ----------------------
    begin                : October 2017
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

#ifndef QGSLAYOUTMULTIFRAMEUNDOCOMMAND_H
#define QGSLAYOUTMULTIFRAMEUNDOCOMMAND_H

#include "qgslayoutundocommand.h"
#include "qgis_core.h"

class QgsLayout;
class QgsLayoutMultiFrame;

SIP_NO_FILE

///@cond PRIVATE

/**
 * \ingroup core
 * An undo command subclass for layout multiframe undo commands.
 *
 * QgsLayoutMultiFrameUndoCommand is a specific layout undo command which is
 * designed for use with QgsLayoutMultiFrames. It automatically handles
 * recreating a deleted multiframes when the undo stack rolls back past
 * the item deletion command.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutMultiFrameUndoCommand: public QgsAbstractLayoutUndoCommand
{
  public:

    /**
     * Constructor for QgsLayoutMultiFrameUndoCommand.
     * \param item associated layout item
     * \param text undo command descriptive text
     * \param id optional undo command id, used for automatic command merging
     * \param parent command
     */
    QgsLayoutMultiFrameUndoCommand( QgsLayoutMultiFrame *frame, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );

    bool mergeWith( const QUndoCommand *command ) override;

    /**
     * Returns the layout associated with this command.
     */
    QgsLayout *layout() const;

    /**
     * Returns the associated multiframes's UUID, which uniquely identifies the frame
     * within the layout.
     */
    QString multiFrameUuid() const;

  protected:

    void saveState( QDomDocument &stateDoc ) const override;
    void restoreState( QDomDocument &stateDoc ) override;

    virtual QgsLayoutMultiFrame *recreateItem( int itemType, QgsLayout *layout ) SIP_FACTORY;

  private:

    QString mFrameUuid;
    QgsLayout *mLayout = nullptr;
    int mItemType = 0;

};

/**
 * \ingroup core
 * An undo command subclass for layout multiframe deletion undo commands.
 *
 * QgsLayoutMultiFrameDeleteUndoCommand is a specific layout undo command which handles
 * layout multiframe deletion. When applied (e.g. as a result of a 'redo' action),
 * the associated layout multiframe is deleted and removed from the layout.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutMultiFrameDeleteUndoCommand: public QgsLayoutMultiFrameUndoCommand
{
  public:

    /**
     * Constructor for QgsLayoutMultiFrameDeleteUndoCommand.
     * \param item associated layout item
     * \param text undo command descriptive text
     * \param id optional undo command id, used for automatic command merging
     * \param parent command
     */
    QgsLayoutMultiFrameDeleteUndoCommand( QgsLayoutMultiFrame *frame, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );
    bool mergeWith( const QUndoCommand *command ) override;
    void redo() override;

};


/**
 * \ingroup core
 * An undo command subclass for layout item addition undo commands.
 *
 * QgsLayoutMultiFrameAddItemCommand is a specific layout undo command which handles
 * layout multiframe creation. When applied (e.g. as a result of a 'redo' action),
 * the associated layout multiframe is recreated and added to the layout.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutMultiFrameAddItemCommand: public QgsLayoutMultiFrameUndoCommand
{
  public:

    /**
     * Constructor for QgsLayoutMultiFrameAddItemCommand.
     * \param item associated layout item
     * \param text undo command descriptive text
     * \param id optional undo command id, used for automatic command merging
     * \param parent command
     */
    QgsLayoutMultiFrameAddItemCommand( QgsLayoutMultiFrame *frame, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );
    bool containsChange() const override;
    bool mergeWith( const QUndoCommand *command ) override;
    void undo() override;

};

///@endcond

#endif //QGSLAYOUTMULTIFRAMEUNDOCOMMAND_H
