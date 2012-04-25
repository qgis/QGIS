/****************************************************************************
**
** Copyright (C) 2009 Stephen Kelly <steveire@gmail.com>
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DYNAMICTREEMODEL_H
#define DYNAMICTREEMODEL_H

#include <QtCore/QAbstractItemModel>

#include <QtCore/QHash>
#include <QtCore/QList>


class DynamicTreeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    DynamicTreeModel( QObject *parent = 0 );

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    int rowCount( const QModelIndex &index = QModelIndex() ) const;
    int columnCount( const QModelIndex &index = QModelIndex() ) const;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    void clear();

  protected slots:

    /**
    Finds the parent id of the string with id @p searchId.

    Returns -1 if not found.
    */
    qint64 findParentId( qint64 searchId ) const;

  private:
    QHash<qint64, QString> m_items;
    QHash<qint64, QList<QList<qint64> > > m_childItems;
    qint64 nextId;
    qint64 newId() { return nextId++; };

    QModelIndex m_nextParentIndex;
    int m_nextRow;

    int m_depth;
    int maxDepth;

    friend class ModelInsertCommand;
    friend class ModelMoveCommand;
    friend class ModelResetCommand;
    friend class ModelResetCommandFixed;

};


class ModelChangeCommand : public QObject
{
    Q_OBJECT
  public:

    ModelChangeCommand( DynamicTreeModel *model, QObject *parent = 0 );

    virtual ~ModelChangeCommand() {}

    void setAncestorRowNumbers( QList<int> rowNumbers ) { m_rowNumbers = rowNumbers; }

    QModelIndex findIndex( QList<int> rows );

    void setStartRow( int row ) { m_startRow = row; }

    void setEndRow( int row ) { m_endRow = row; }

    void setNumCols( int cols ) { m_numCols = cols; }

    virtual void doCommand() = 0;

  protected:
    DynamicTreeModel* m_model;
    QList<int> m_rowNumbers;
    int m_numCols;
    int m_startRow;
    int m_endRow;

};

typedef QList<ModelChangeCommand*> ModelChangeCommandList;

class ModelInsertCommand : public ModelChangeCommand
{
    Q_OBJECT

  public:

    ModelInsertCommand( DynamicTreeModel *model, QObject *parent = 0 );
    virtual ~ModelInsertCommand() {}

    virtual void doCommand();
};


class ModelMoveCommand : public ModelChangeCommand
{
    Q_OBJECT
  public:
    ModelMoveCommand( DynamicTreeModel *model, QObject *parent );

    virtual ~ModelMoveCommand() {}

    virtual bool emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow );

    virtual void doCommand();

    virtual void emitPostSignal();

    void setDestAncestors( QList<int> rows ) { m_destRowNumbers = rows; }

    void setDestRow( int row ) { m_destRow = row; }

  protected:
    QList<int> m_destRowNumbers;
    int m_destRow;
};

/**
  A command which does a move and emits a reset signal.
*/
class ModelResetCommand : public ModelMoveCommand
{
    Q_OBJECT
  public:
    ModelResetCommand( DynamicTreeModel* model, QObject* parent = 0 );

    virtual ~ModelResetCommand();

    virtual bool emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow );
    virtual void emitPostSignal();

};

/**
  A command which does a move and emits a beginResetModel and endResetModel signals.
*/
class ModelResetCommandFixed : public ModelMoveCommand
{
    Q_OBJECT
  public:
    ModelResetCommandFixed( DynamicTreeModel* model, QObject* parent = 0 );

    virtual ~ModelResetCommandFixed();

    virtual bool emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow );
    virtual void emitPostSignal();

};


#endif
