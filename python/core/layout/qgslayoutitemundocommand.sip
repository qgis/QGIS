/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemundocommand.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutItemUndoCommand: QgsAbstractLayoutUndoCommand
{
%Docstring
*************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgslayoutitemundocommand.h"
%End
  public:

    QgsLayoutItemUndoCommand( QgsLayoutItem *item, const QString &text, int id = 0, QUndoCommand *parent /TransferThis/ = 0 );

    virtual bool mergeWith( const QUndoCommand *command );


    QgsLayout *layout() const;
%Docstring
 :rtype: QgsLayout
%End

    QString itemUuid() const;
%Docstring
 :rtype: str
%End

  protected:

    virtual void saveState( QDomDocument &stateDoc ) const;

    virtual void restoreState( QDomDocument &stateDoc );


    virtual QgsLayoutItem *recreateItem( int itemType, QgsLayout *layout ) /Factory/;
%Docstring
 :rtype: QgsLayoutItem
%End

};

class QgsLayoutItemDeleteUndoCommand: QgsLayoutItemUndoCommand
{

%TypeHeaderCode
#include "qgslayoutitemundocommand.h"
%End
  public:

    QgsLayoutItemDeleteUndoCommand( QgsLayoutItem *item, const QString &text, int id = 0, QUndoCommand *parent /TransferThis/ = 0 );
    virtual bool mergeWith( const QUndoCommand *command );

    virtual void redo();


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemundocommand.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
