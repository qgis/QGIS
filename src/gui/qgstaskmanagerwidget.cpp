/***************************************************************************
                             qgstaskmanagerwidget.cpp
                             ------------------------
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

#include "qgstaskmanagerwidget.h"
#include "qgstaskmanager.h"
#include "qgsapplication.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTreeView>
#include <QLayout>
#include <QToolBar>
#include <QProgressBar>
#include <QAction>

//
// QgsTaskManagerWidget
//

QgsTaskManagerWidget::QgsTaskManagerWidget( QgsTaskManager *manager, QWidget *parent )
    : QWidget( parent )
{
  Q_ASSERT( manager );

  QVBoxLayout* vLayout = new QVBoxLayout();
  vLayout->setMargin( 0 );
#if 0
  QToolBar* toolbar = new QToolBar();
  toolbar->setIconSize( QSize( 16, 16 ) );
  toolbar->addAction( new QAction( "test", this ) );
  vLayout->addWidget( toolbar );
#endif
  mTreeView = new QTreeView();
  mTreeView->setModel( new QgsTaskManagerModel( manager, this ) );
  mTreeView->setItemDelegateForColumn( 1, new QgsProgressBarDelegate( this ) );
  mTreeView->setItemDelegateForColumn( 2, new QgsTaskStatusDelegate( this ) );
  mTreeView->setHeaderHidden( true );
  mTreeView->setRootIsDecorated( false );
  mTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );

  vLayout->addWidget( mTreeView );


  setLayout( vLayout );
}


//
// QgsTaskManagerModel
//

QgsTaskManagerModel::QgsTaskManagerModel( QgsTaskManager *manager, QObject *parent )
    : QAbstractItemModel( parent )
    , mManager( manager )
{
  Q_ASSERT( mManager );

  //populate row to id map
  int i = 0;
  Q_FOREACH ( QgsTask* task, mManager->tasks() )
  {
    mRowToTaskIdMap.insert( i, mManager->taskId( task ) );
  }

  connect( mManager, &QgsTaskManager::taskAdded, this, &QgsTaskManagerModel::taskAdded );
  // not right - should be completion
  connect( mManager, &QgsTaskManager::taskAboutToBeDeleted, this, &QgsTaskManagerModel::taskDeleted );
  connect( mManager, &QgsTaskManager::progressChanged, this, &QgsTaskManagerModel::progressChanged );
  connect( mManager, &QgsTaskManager::statusChanged, this, &QgsTaskManagerModel::statusChanged );
}

QModelIndex QgsTaskManagerModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() )
  {
    //column out of bounds
    return QModelIndex();
  }

  if ( !parent.isValid() && row >= 0 && row < mManager->count() )
  {
    //return an index for the task at this position
    return createIndex( row, column );
  }

  //only top level supported
  return QModelIndex();

}

QModelIndex QgsTaskManagerModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index );

  //all items are top level
  return QModelIndex();
}

int QgsTaskManagerModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mManager->count();
  }
  else
  {
    //no children
    return 0;
  }
}

int QgsTaskManagerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 3;
}

QVariant QgsTaskManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsTask* task = indexToTask( index );
  if ( task )
  {
    switch ( role )
    {
      case Qt::DisplayRole:
      case Qt::EditRole:
        switch ( index.column() )
        {
          case Description:
            return task->description();
          case Progress:
            return task->progress();
          case Status:
            return static_cast<int>( task->status() );
          default:
            return QVariant();
        }

      case StatusRole:
        return static_cast<int>( task->status() );

      default:
        return QVariant();
    }
  }

  return QVariant();
}

Qt::ItemFlags QgsTaskManagerModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( ! index.isValid() )
  {
    return flags;
  }

  if ( index.column() == Status )
  {
    //if ( static_cast< QgsTask::TaskStatus >( data( index, StatusRole ).toInt() ) == QgsTask::Running )
    flags = flags | Qt::ItemIsEditable;
  }
  return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool QgsTaskManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( role );

  if ( !index.isValid() )
    return false;

  QgsTask* task = indexToTask( index );
  if ( !task )
    return false;

  switch ( index.column() )
  {
    case Status:
    {
      if ( value.toBool() )
        task->cancel();
      return true;
    }

    default:
      return false;
  }
}

void QgsTaskManagerModel::taskAdded( long id )
{
  beginInsertRows( QModelIndex(), mRowToTaskIdMap.count(),
                   mRowToTaskIdMap.count() );
  mRowToTaskIdMap.insert( mRowToTaskIdMap.count(), id );
  endInsertRows();
}

void QgsTaskManagerModel::taskDeleted( long id )
{
  for ( QMap< int, long >::iterator it = mRowToTaskIdMap.begin(); it != mRowToTaskIdMap.end(); )
  {
    if ( it.value() == id )
    {
      beginRemoveRows( QModelIndex(), it.key(), it.key() );
      it = mRowToTaskIdMap.erase( it );
      endRemoveRows();
      return;
    }
    else
      ++it;
  }
}

void QgsTaskManagerModel::progressChanged( long id, double progress )
{
  Q_UNUSED( progress );

  QModelIndex index = idToIndex( id, Progress );
  if ( !index.isValid() )
  {
    return;
  }

  emit dataChanged( index, index );
}

void QgsTaskManagerModel::statusChanged( long id, int status )
{
  Q_UNUSED( status );

  QModelIndex index = idToIndex( id, Status );
  if ( !index.isValid() )
  {
    return;
  }

  emit dataChanged( index, index );
}

QgsTask *QgsTaskManagerModel::indexToTask( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.parent().isValid() )
    return nullptr;

  long id = mRowToTaskIdMap.value( index.row(), -1 );
  if ( id >= 0 )
    return mManager->task( id );
  else
    return nullptr;
}

int QgsTaskManagerModel::idToRow( long id ) const
{
  for ( QMap< int, long >::const_iterator it = mRowToTaskIdMap.constBegin(); it != mRowToTaskIdMap.constEnd(); ++it )
  {
    if ( it.value() == id )
    {
      return it.key();
    }
  }
  return -1;
}

QModelIndex QgsTaskManagerModel::idToIndex( long id, int column ) const
{
  int row = idToRow( id );
  if ( row < 0 )
    return QModelIndex();

  return index( row, column );
}



//
// QgsProgressBarDelegate
//

QgsProgressBarDelegate::QgsProgressBarDelegate( QObject *parent )
    : QStyledItemDelegate( parent )
{}

void QgsProgressBarDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyledItemDelegate::paint( painter, option, index );

  int progress = index.data().toInt();

  QStyleOptionProgressBar progressBarOption;
  progressBarOption.state = option.state;
  progressBarOption.rect = option.rect;
  progressBarOption.rect.setTop( option.rect.top() + 1 );
  progressBarOption.rect.setHeight( option.rect.height() - 2 );
  progressBarOption.minimum = 0;
  progressBarOption.maximum = 100;
  progressBarOption.progress = progress;
  progressBarOption.text = QString::number( progress ) + "%";
  progressBarOption.textVisible = true;
  progressBarOption.textAlignment = Qt::AlignCenter;

  QgsApplication::style()->drawControl( QStyle::CE_ProgressBar, &progressBarOption, painter );
}

QSize QgsProgressBarDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  return QSize( option.rect.width(), option.fontMetrics.height() + 10 );
}


//
// QgsTaskStatusDelegate
//

QgsTaskStatusDelegate::QgsTaskStatusDelegate( QObject *parent )
    : QStyledItemDelegate( parent )
{

}

void QgsTaskStatusDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyledItemDelegate::paint( painter, option, index );

  QIcon icon;
  switch ( static_cast< QgsTask::TaskStatus >( index.data().toInt() ) )
  {
    case QgsTask::Queued:
    case QgsTask::OnHold:
      icon = QgsApplication::getThemeIcon( "/mIconFieldTime.svg" );
      break;
    case QgsTask::Running:
      icon = QgsApplication::getThemeIcon( "/mActionRefresh.png" );
      break;
    case QgsTask::Complete:
      icon = QgsApplication::getThemeIcon( "/mActionCheckQgisVersion.png" );
      break;
    case QgsTask::Terminated:
      icon = QgsApplication::getThemeIcon( "/mActionRemove.svg" );
      break;
  }
  icon.paint( painter, option.rect.left() + 1, ( option.rect.top() + option.rect.bottom() ) / 2 - 12, 24, 24 );
}

QSize QgsTaskStatusDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );
  return QSize( 32, 32 );
}

bool QgsTaskStatusDelegate::editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index )
{
  Q_UNUSED( option );
  if ( event->type() == QEvent::MouseButtonPress )
  {
    QMouseEvent *e = static_cast<QMouseEvent*>( event );
    if ( e->button() == Qt::LeftButton )
    {
      if ( !index.model()->flags( index ).testFlag( Qt::ItemIsEditable ) )
      {
        //item not editable
        return false;
      }

      return model->setData( index, true, Qt::EditRole );
    }
  }

  return false;
}

QgsTaskManagerFloatingWidget::QgsTaskManagerFloatingWidget( QgsTaskManager *manager, QWidget *parent )
    : QgsFloatingWidget( parent )
{
  setLayout( new QVBoxLayout() );
  setMinimumSize( 250, 150 );
  QgsTaskManagerWidget* w = new QgsTaskManagerWidget( manager );
  layout()->addWidget( w );
  setStyleSheet( ".QgsTaskManagerFloatingWidget { border-top-left-radius: 8px;"
                 "border-top-right-radius: 8px; background-color: rgb(0, 0, 0, 70%); }" );
}

QgsTaskManagerStatusBarWidget::QgsTaskManagerStatusBarWidget( QgsTaskManager *manager, QWidget *parent )
    : QToolButton( parent )
    , mProgressBar( nullptr )
    , mManager( manager )
{
  setAutoRaise( true );
  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
  setLayout( new QVBoxLayout() );

  mProgressBar = new QProgressBar();
  mProgressBar->setMinimum( 0 );
  mProgressBar->setMaximum( 0 );
  layout()->setContentsMargins( 5, 5, 5, 5 );
  layout()->addWidget( mProgressBar );

  mFloatingWidget = new QgsTaskManagerFloatingWidget( manager, parent ? parent->window() : nullptr );
  mFloatingWidget->setAnchorWidget( this );
  mFloatingWidget->setAnchorPoint( QgsFloatingWidget::BottomMiddle );
  mFloatingWidget->setAnchorWidgetPoint( QgsFloatingWidget::TopMiddle );
  mFloatingWidget->hide();
  connect( this, &QgsTaskManagerStatusBarWidget::clicked, this, &QgsTaskManagerStatusBarWidget::toggleDisplay );
  hide();

  connect( manager, &QgsTaskManager::taskAdded, this, &QgsTaskManagerStatusBarWidget::showButton );
  connect( manager, &QgsTaskManager::allTasksFinished, this, &QgsTaskManagerStatusBarWidget::allFinished );
  connect( manager, &QgsTaskManager::finalTaskProgressChanged, this, &QgsTaskManagerStatusBarWidget::overallProgressChanged );
  connect( manager, &QgsTaskManager::countActiveTasksChanged, this, &QgsTaskManagerStatusBarWidget::countActiveTasksChanged );
}

QSize QgsTaskManagerStatusBarWidget::sizeHint() const
{
  int width = 100;
  int height = QToolButton::sizeHint().height();
  return QSize( width, height );
}

void QgsTaskManagerStatusBarWidget::toggleDisplay()
{
  if ( mFloatingWidget->isVisible() )
    mFloatingWidget->hide();
  else
  {
    mFloatingWidget->show();
    mFloatingWidget->raise();
  }
}

void QgsTaskManagerStatusBarWidget::overallProgressChanged( double progress )
{
  mProgressBar->setValue( progress );
  if ( mProgressBar->maximum( ) == 0 )
    mProgressBar->setMaximum( 100 );
  setToolTip( mManager->activeTasks().at( 0 )->description() );
}

void QgsTaskManagerStatusBarWidget::countActiveTasksChanged( int count )
{
  if ( count > 1 )
  {
    mProgressBar->setMaximum( 0 );
    setToolTip( tr( "%1 active tasks running" ).arg( count ) );
  }
}

void QgsTaskManagerStatusBarWidget::allFinished()
{
  mFloatingWidget->hide();
  hide();

  mProgressBar->setMaximum( 0 );
  mProgressBar->setValue( 0 );
}

void QgsTaskManagerStatusBarWidget::showButton()
{
  if ( !isVisible() )
  {
    mProgressBar->setMaximum( 100 );
    mProgressBar->setValue( 0 );
    show();
  }
}
