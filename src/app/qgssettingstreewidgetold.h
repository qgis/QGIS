/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QGSSETTINGSTREEWIDGETOLD_H
#define QGSSETTINGSTREEWIDGETOLD_H

#include "qgssettings.h"
#include <QIcon>
#include <QTimer>
#include <QTreeWidget>


class QgsSettingsTreeWidgetOld : public QTreeWidget
{
    Q_OBJECT

  public:
    //! Model roles
    enum Roles
    {
      TypeRole = Qt::UserRole + 1, //!< Item type role, see Type enum
      PathRole,                    //!< Complete setting path, including parent groups
    };

    //! Item types
    enum Type
    {
      Group = 0, //!< Group item
      Setting,   //!< Setting item
    };
    Q_ENUM( Type )

    explicit QgsSettingsTreeWidgetOld( QWidget *parent = nullptr );

    QSize sizeHint() const override;

    void setSettingsMap( QMap<QString, QStringList> &map ) { mSettingsMap = map; }
    QString itemKey( QTreeWidgetItem *item );

  public slots:
    void setAutoRefresh( bool mAutoRefresh );
    void maybeRefresh();
    void refresh();

  protected:
    bool event( QEvent *event ) override;
    void showEvent( QShowEvent * ) override;

  private slots:
    void updateSetting( QTreeWidgetItem *item );
    void showContextMenu( QPoint pos );

  private:
    enum Columns
    {
      ColumnSettings = 0,
      ColumnType,
      ColumnValue,
      ColumnDescription
    };

    void updateChildItems( QTreeWidgetItem *parent );
    QTreeWidgetItem *createItem( const QString &text, QTreeWidgetItem *parent, int index, bool isGroup );
    QTreeWidgetItem *childAt( QTreeWidgetItem *parent, int index );
    int childCount( QTreeWidgetItem *parent );
    int findChild( QTreeWidgetItem *parent, const QString &text, int startIndex );
    void moveItemForward( QTreeWidgetItem *parent, int oldIndex, int newIndex );

    QgsSettings mSettings;
    QTimer mRefreshTimer;
    bool mAutoRefresh = false;
    QIcon mGroupIcon;
    QIcon mKeyIcon;

    QMap<QString, QStringList> mSettingsMap;
    QMenu *mContextMenu = nullptr;
};

#endif
