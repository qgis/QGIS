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

class CORE_EXPORT QgsLayoutItemUndoCommand: public QgsAbstractLayoutUndoCommand
{
  public:

    QgsLayoutItemUndoCommand( QgsLayoutItem *item, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );

    bool mergeWith( const QUndoCommand *command ) override;

    QgsLayout *layout() const;

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

class CORE_EXPORT QgsLayoutItemDeleteUndoCommand: public QgsLayoutItemUndoCommand
{
  public:

    QgsLayoutItemDeleteUndoCommand( QgsLayoutItem *item, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr );
    bool mergeWith( const QUndoCommand *command ) override;
    void redo() override;

};


#endif
