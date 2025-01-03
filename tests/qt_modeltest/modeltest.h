/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
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
****************************************************************************/

#ifndef MODELTEST_H
#define MODELTEST_H

#include <QAbstractItemModel>
#include <QObject>
#include <QStack>

#include "qgis_core.h"

class CORE_EXPORT ModelTest : public QObject
{
    Q_OBJECT

  public:
    ModelTest( QAbstractItemModel *model, QObject *parent = nullptr );

  private Q_SLOTS:
    void nonDestructiveBasicTest();
    void rowCount();
    void columnCount();
    void hasIndex();
    void index();
    void parent();
    void data();

  protected Q_SLOTS:
    void runAllTests();
    void layoutAboutToBeChanged();
    void layoutChanged();
    void rowsAboutToBeInserted( const QModelIndex &parent, int start, int end );
    void rowsInserted( const QModelIndex &parent, int start, int end );
    void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );
    void rowsRemoved( const QModelIndex &parent, int start, int end );

  private:
    void checkChildren( const QModelIndex &parent, int currentDepth = 0 );

    QAbstractItemModel *model = nullptr;

    struct Changing
    {
        QModelIndex parent;
        int oldSize;
        QVariant last;
        QVariant next;
    };
    QStack<Changing> insert;
    QStack<Changing> remove;

    bool fetchingMore = false;

    QList<QPersistentModelIndex> changing;
};

#endif
