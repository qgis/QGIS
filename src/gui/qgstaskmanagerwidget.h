/***************************************************************************
                             qgstaskmanagerwidget.h
                             ----------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
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
#ifndef QGSTASKMANAGERWIDGET_H
#define QGSTASKMANAGERWIDGET_H

#include "qgsfloatingwidget.h"
#include <QStyledItemDelegate>
#include <QToolButton>

class QgsTaskManager;
class QgsTask;
class QTreeView;
class QProgressBar;

/**
 * \ingroup gui
 * \class QgsTaskManagerWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager.
 * @see QgsTaskManager
 * \note added in QGIS 3.0
 */
class GUI_EXPORT QgsTaskManagerWidget : public QWidget
{
    Q_OBJECT

  public:

    /** Constructor for QgsTaskManagerWidget
     * @param manager task manager associated with widget
     * @param parent parent widget
     */
    QgsTaskManagerWidget( QgsTaskManager* manager, QWidget* parent = nullptr );

  private:

    QTreeView* mTreeView;
};

/**
 * \ingroup gui
 * \class QgsTaskManagerFloatingWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager.
 * @see QgsTaskManager
 * \note added in QGIS 2.16
 */
class GUI_EXPORT QgsTaskManagerFloatingWidget : public QgsFloatingWidget
{
    Q_OBJECT

  public:

    /** Constructor for QgsTaskManagerWidget
     * @param manager task manager associated with widget
     * @param parent parent widget
     */
    QgsTaskManagerFloatingWidget( QgsTaskManager* manager, QWidget* parent = nullptr );

};

/**
 * \class QgsTaskManagerStatusBarWidget
 * A compact widget designed for embedding in a status bar, which displays tasks from a
 * QgsTaskManager and allows for interaction with the manager.
 * @see QgsTaskManager
 * \ingroup gui
 * \note added in QGIS 3.0
 */
class GUI_EXPORT QgsTaskManagerStatusBarWidget : public QToolButton
{
    Q_OBJECT

  public:

    /** Constructor for QgsTaskManagerWidget.
     * @param manager task manager associated with widget
     * @param parent parent widget
     */
    QgsTaskManagerStatusBarWidget( QgsTaskManager* manager, QWidget* parent = nullptr );

    QSize sizeHint() const override;

  private slots:

    void toggleDisplay();
    void overallProgressChanged( double progess );
    void countActiveTasksChanged( int count );
    void allFinished();
    void showButton();

  private:

    QgsTaskManagerFloatingWidget* mFloatingWidget;
    QProgressBar* mProgressBar;
    QgsTaskManager* mManager;
};



/**
 * \ingroup gui
 * \class QgsTaskManagerModel
 * A model representing a QgsTaskManager.
 * @see QgsTaskManager
 * \note added in QGIS 2.16
 */
class GUI_EXPORT QgsTaskManagerModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    /** Constructor for QgsTaskManagerModel
     * @param manager task manager for model
     * @param parent parent object
     */
    explicit QgsTaskManagerModel( QgsTaskManager* manager, QObject* parent = nullptr );

    //reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex & index ) const override;
    bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) override;

  private slots:

    void taskAdded( long id );
    void taskDeleted( long id );
    void progressChanged( long id, double progress );
    void statusChanged( long id, int status );

  private:

    enum Columns
    {
      Description = 0,
      Progress = 1,
      Status = 2,
    };

    enum Roles
    {
      StatusRole = Qt::UserRole,
    };

    QgsTaskManager* mManager;

    QMap< int, long > mRowToTaskIdMap;

    QgsTask* indexToTask( const QModelIndex& index ) const;
    int idToRow( long id ) const;
    QModelIndex idToIndex( long id, int column ) const;
};


/**
 * \ingroup gui
 * \class QgsProgressBarDelegate
 * A delegate for showing a progress bar within a view.
 * \note added in QGIS 3.0
 */
class GUI_EXPORT QgsProgressBarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    /** Constructor for QgsProgressBarDelegate
     * @param parent parent object
     */
    QgsProgressBarDelegate( QObject* parent = nullptr );

    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

/**
 * \ingroup gui
 * \class QgsTaskStatusDelegate
 * A delegate for showing task status within a view. Clicks on the delegate will cause the task to be cancelled (via the model).
 * \note added in QGIS 3.0
 */
class GUI_EXPORT QgsTaskStatusDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    /** Constructor for QgsTaskStatusDelegate
     * @param parent parent object
     */
    QgsTaskStatusDelegate( QObject* parent = nullptr );

    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index ) override;
};

#endif //QGSTASKMANAGERWIDGET_H
