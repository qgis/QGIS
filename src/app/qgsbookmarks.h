/***************************************************************************
               QgsBookmarks.h  - Spatial Bookmarks
                             -------------------
    begin                : 2005-04-23
    copyright            : (C) 2005 Gary Sherman
    email                : sherman at mrcc dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBOOKMARKS_H
#define QGSBOOKMARKS_H

#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

#include "ui_qgsbookmarksbase.h"
#include "qgsdockwidget.h"
#include "qgis_app.h"

class QgsBookmark;
class QgsBookmarkManager;
class QgsBookmarkManagerProxyModel;

/**
 * \brief QgsDoubleSpinBoxBookmarksDelegate class shows 6 digits when value is a double
 */
class QgsDoubleSpinBoxBookmarksDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsDoubleSpinBoxBookmarksDelegate( QObject *parent = nullptr, int decimals = -1 );

    QString displayText( const QVariant &value, const QLocale &locale ) const override;

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  private:
    static const int DEFAULT_DECIMAL_PLACES;
    int mDecimals;
};


class APP_EXPORT QgsBookmarks : public QgsDockWidget, private Ui::QgsBookmarksBase
{
    Q_OBJECT

  public:
    QgsBookmarks( QWidget *parent = nullptr );
    ~QgsBookmarks() override;
    QMap<QString, QModelIndex> getIndexMap();
    void zoomToBookmarkIndex( const QModelIndex & );

  public slots:
    void addClicked();

  private slots:
    void deleteClicked();
    void zoomToBookmark();
    void exportToXml();
    void importFromXml();

    void lstBookmarks_doubleClicked( const QModelIndex & );
    void lstBookmarks_customContextMenuRequested( QPoint pos );

  private:
    QgsBookmarkManagerProxyModel *mBookmarkModel = nullptr;

    void saveWindowLocation();
};


#endif // QGSBOOKMARKS_H
