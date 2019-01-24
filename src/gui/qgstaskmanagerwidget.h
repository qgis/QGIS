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
#include "qgis_sip.h"
#include "qgstaskmanager.h"
#include <QStyledItemDelegate>
#include <QToolButton>
#include "qgis_gui.h"

class QgsTaskManager;
class QgsTask;
class QTreeView;
class QProgressBar;
class QgsTaskManagerModel;

/**
 * \ingroup gui
 * \class QgsTaskManagerWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager.
 * \see QgsTaskManager
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTaskManagerWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTaskManagerWidget
     * \param manager task manager associated with widget
     * \param parent parent widget
     */
    QgsTaskManagerWidget( QgsTaskManager *manager, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsTaskManagerWidget() override;

  private slots:

    void modelRowsInserted( const QModelIndex &index, int start, int end );
    void clicked( const QModelIndex &index );

  private:

    QgsTaskManager *mManager = nullptr;
    QTreeView *mTreeView = nullptr;
    QgsTaskManagerModel *mModel = nullptr;
};

#ifndef SIP_RUN

///@cond PRIVATE

/**
 * \ingroup gui
 * \class QgsTaskManagerFloatingWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager.
 * \see QgsTaskManager
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTaskManagerFloatingWidget : public QgsFloatingWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTaskManagerWidget
     * \param manager task manager associated with widget
     * \param parent parent widget
     */
    QgsTaskManagerFloatingWidget( QgsTaskManager *manager, QWidget *parent = nullptr );

};

/**
 * \class QgsTaskManagerStatusBarWidget
 * A compact widget designed for embedding in a status bar, which displays tasks from a
 * QgsTaskManager and allows for interaction with the manager.
 * \see QgsTaskManager
 * \ingroup gui
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTaskManagerStatusBarWidget : public QToolButton
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTaskManagerWidget.
     * \param manager task manager associated with widget
     * \param parent parent widget
     */
    QgsTaskManagerStatusBarWidget( QgsTaskManager *manager, QWidget *parent = nullptr );

    QSize sizeHint() const override;

  protected:

    void changeEvent( QEvent *event ) override;

  private slots:

    void toggleDisplay();
    void overallProgressChanged( double progress );
    void countActiveTasksChanged( int count );
    void allFinished();
    void showButton();

  private:

    QgsTaskManagerFloatingWidget *mFloatingWidget = nullptr;
    QProgressBar *mProgressBar = nullptr;
    QgsTaskManager *mManager = nullptr;
};

/**
 * \ingroup gui
 * \class QgsTaskManagerModel
 * A model representing a QgsTaskManager.
 * \see QgsTaskManager
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTaskManagerModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    enum Columns
    {
      Description = 0,
      Progress = 1,
      Status = 2,
    };

    /**
     * Constructor for QgsTaskManagerModel
     * \param manager task manager for model
     * \param parent parent object
     */
    explicit QgsTaskManagerModel( QgsTaskManager *manager, QObject *parent = nullptr );

    //reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Returns the task associated with a specified model index, or nullptr if no
     * task was found.
     */
    QgsTask *indexToTask( const QModelIndex &index ) const;

    //! Model roles
    enum Roles
    {
      StatusRole = Qt::UserRole, //!< Status role
    };

  private slots:

    void taskAdded( long id );
    void taskDeleted( long id );
    void progressChanged( long id, double progress );
    void statusChanged( long id, int status );

  private:

    enum ToolTipType
    {
      ToolTipDescription,
      ToolTipStatus,
      ToolTipProgress,
    };

    QgsTaskManager *mManager = nullptr;

    QList< long > mRowToTaskIdList;


    int idToRow( long id ) const;
    QModelIndex idToIndex( long id, int column ) const;
    static QString createTooltip( QgsTask *task, ToolTipType type );

    friend class QgsTaskManagerStatusBarWidget;
};

/**
 * \ingroup gui
 * \class QgsTaskStatusWidget
 * A widget for showing task status within a view. Clicks on the widget will cause the task to be canceled (via the model).
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTaskStatusWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTaskStatusWidget
     * \param parent parent object
     */
    QgsTaskStatusWidget( QWidget *parent = nullptr, QgsTask::TaskStatus status = QgsTask::Queued, bool canCancel = true );


    QSize sizeHint() const override;

    //bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index ) override;

  public slots:

    /**
     * Sets the status to show for the task.
     */
    void setStatus( int status );

  signals:

    /**
     * Emitted when the user clicks a cancelable task.
     */
    void cancelClicked();

  protected:

    void paintEvent( QPaintEvent *e ) override;
    void mousePressEvent( QMouseEvent *e ) override;
    void mouseMoveEvent( QMouseEvent *e ) override;
    void leaveEvent( QEvent *e ) override;

  private:

    bool mCanCancel;
    QgsTask::TaskStatus mStatus;
    bool mInside = false;
};

///@endcond
///

#endif

#endif //QGSTASKMANAGERWIDGET_H
