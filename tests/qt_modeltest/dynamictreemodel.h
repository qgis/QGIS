/****************************************************************************
**
** Copyright (C) 2009 Stephen Kelly <steveire@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
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
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &index = QModelIndex() ) const override;
    int columnCount( const QModelIndex &index = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    void clear();
  protected slots:

    /**
     * Finds the parent id of the string with id @p searchId.
     * Returns -1 if not found.
    */
    qint64 findParentId( qint64 searchId ) const;

  private:
    QHash<qint64, QString> m_items;
    QHash<qint64, QList<QList<qint64>>> m_childItems;
    qint64 nextId;
    qint64 newId()
    {
      return nextId++;
    }
    QModelIndex m_nextParentIndex;
    int m_nextRow;
    int m_depth;
    int maxDepth;
    friend class ModelInsertCommand;
    friend class ModelMoveCommand;
    friend class ModelResetCommand;
    friend class ModelResetCommandFixed;
    friend class ModelChangeChildrenLayoutsCommand;
};
class ModelChangeCommand : public QObject
{
    Q_OBJECT
  public:
    ModelChangeCommand( DynamicTreeModel *model, QObject *parent = 0 );
    void setAncestorRowNumbers( QList<int> rowNumbers )
    {
      m_rowNumbers = rowNumbers;
    }
    QModelIndex findIndex( const QList<int> &rows ) const;
    void setStartRow( int row )
    {
      m_startRow = row;
    }
    void setEndRow( int row )
    {
      m_endRow = row;
    }
    void setNumCols( int cols )
    {
      m_numCols = cols;
    }
    virtual void doCommand() = 0;

  protected:
    DynamicTreeModel *m_model;
    QList<int> m_rowNumbers;
    int m_numCols;
    int m_startRow;
    int m_endRow;
};
typedef QList<ModelChangeCommand *> ModelChangeCommandList;
class ModelInsertCommand : public ModelChangeCommand
{
    Q_OBJECT
  public:
    ModelInsertCommand( DynamicTreeModel *model, QObject *parent = 0 );
    void doCommand() override;
};
class ModelMoveCommand : public ModelChangeCommand
{
    Q_OBJECT
  public:
    ModelMoveCommand( DynamicTreeModel *model, QObject *parent );
    virtual bool emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow );
    void doCommand() override;
    virtual void emitPostSignal();
    void setDestAncestors( QList<int> rows )
    {
      m_destRowNumbers = rows;
    }
    void setDestRow( int row )
    {
      m_destRow = row;
    }

  protected:
    QList<int> m_destRowNumbers;
    int m_destRow;
};

/**
 * A command which does a move and emits a reset signal.
*/
class ModelResetCommand : public ModelMoveCommand
{
    Q_OBJECT
  public:
    ModelResetCommand( DynamicTreeModel *model, QObject *parent = 0 );
    bool emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow ) override;
    void emitPostSignal() override;
};

/**
 * A command which does a move and emits a beginResetModel and endResetModel signals.
*/
class ModelResetCommandFixed : public ModelMoveCommand
{
    Q_OBJECT
  public:
    ModelResetCommandFixed( DynamicTreeModel *model, QObject *parent = 0 );
    bool emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow ) override;
    void emitPostSignal() override;
};
class ModelChangeChildrenLayoutsCommand : public ModelChangeCommand
{
    Q_OBJECT
  public:
    ModelChangeChildrenLayoutsCommand( DynamicTreeModel *model, QObject *parent );
    void doCommand() override;
    void setSecondAncestorRowNumbers( QList<int> rows )
    {
      m_secondRowNumbers = rows;
    }

  protected:
    QList<int> m_secondRowNumbers;
    int m_destRow;
};
#endif
