/***************************************************************************
                          qgslayoutitemgroupundocommand.h
                          -------------------------------
    begin                : 2016-06-09
    copyright            : (C) 2016 by Sandro Santilli
    email                : strk at kbt dot io
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMGROUPUNDOCOMMAND_H
#define QGSLAYOUTITEMGROUPUNDOCOMMAND_H

#include "qgis_core.h"
#include <QUndoCommand>
#include "qgslayoutitem.h"

#define SIP_NO_FILE
///@cond PRIVATE

/**
 * \ingroup core
 * A layout undo command class for grouping / ungrouping layout items.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemGroupUndoCommand: public QObject, public QUndoCommand
{
    Q_OBJECT

  public:

    //! Command kind, and state
    enum State
    {
      Grouped = 0,
      Ungrouped
    };

    /**
     * Create a group or ungroup command
     *
     * \param s command kind (\see State)
     * \param item the group item being created or ungrouped
     * \param c the composition including this group
     * \param text command label
     * \param parent parent command, if any
     *
     */
    QgsLayoutItemGroupUndoCommand( State s, QgsLayoutItemGroup *group, QgsLayout *layout,
                                   const QString &text, QUndoCommand *parent = nullptr );

    void redo() override;
    void undo() override;

  private:
    QString mGroupUuid;
    QSet<QString> mItemUuids;
    QgsLayout *mLayout = nullptr;
    State mState;
    //! Flag to prevent execution when the command is pushed to the QUndoStack
    bool mFirstRun = true;

    //changes between added / removed state
    void switchState();
};
///@endcond

#endif // QGSLAYOUTITEMGROUPUNDOCOMMAND_H
