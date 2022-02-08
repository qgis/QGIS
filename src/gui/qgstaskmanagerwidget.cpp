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
#include <QHeaderView>

//
// QgsTaskManagerWidget
//

QgsTaskManagerWidget::QgsTaskManagerWidget( QgsTaskManager *manager, QWidget *parent )
  : QWidget( parent )
  , mManager( manager )
{
  Q_ASSERT( manager );

  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  mTreeView = new QTreeView();
  mModel = new QgsTaskManagerModel( manager, this );
  mTreeView->setModel( mModel );
  connect( mModel, &QgsTaskManagerModel::rowsInserted, this, &QgsTaskManagerWidget::modelRowsInserted );
  mTreeView->setHeaderHidden( true );
  mTreeView->setRootIsDecorated( false );
  mTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );

  const int progressColWidth = static_cast< int >( fontMetrics().horizontalAdvance( 'X' ) * 10 * Qgis::UI_SCALE_FACTOR );
  mTreeView->setColumnWidth( QgsTaskManagerModel::Progress, progressColWidth );

  const int statusColWidth = static_cast< int >( fontMetrics().horizontalAdvance( 'X' ) * 2 * Qgis::UI_SCALE_FACTOR );
  mTreeView->setColumnWidth( QgsTaskManagerModel::Status, statusColWidth );
  mTreeView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mTreeView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  mTreeView->header()->setStretchLastSection( false );
  mTreeView->header()->setSectionResizeMode( QgsTaskManagerModel::Description, QHeaderView::Stretch );

  connect( mTreeView, &QTreeView::clicked, this, &QgsTaskManagerWidget::clicked );

  vLayout->addWidget( mTreeView );

  setLayout( vLayout );
}

QgsTaskManagerWidget::~QgsTaskManagerWidget()
{
  delete mModel;
}


void QgsTaskManagerWidget::modelRowsInserted( const QModelIndex &, int start, int end )
{
  for ( int row = start; row <= end; ++row )
  {
    QgsTask *task = mModel->indexToTask( mModel->index( row, 1 ) );
    if ( !task )
      continue;

    QProgressBar *progressBar = new QProgressBar();
    progressBar->setAutoFillBackground( true );
    progressBar->setRange( 0, 0 );
    connect( task, &QgsTask::progressChanged, progressBar, [progressBar]( double progress )
    {
      //until first progress report, we show a progress bar of interderminant length
      if ( progress > 0 )
      {
        progressBar->setMaximum( 100 );
        progressBar->setValue( static_cast< int >( std::round( progress ) ) );
      }
      else
        progressBar->setMaximum( 0 );
    }
           );
    mTreeView->setIndexWidget( mModel->index( row, QgsTaskManagerModel::Progress ), progressBar );

    QgsTaskStatusWidget *statusWidget = new QgsTaskStatusWidget( nullptr, task->status(), task->canCancel() );
    statusWidget->setAutoFillBackground( true );
    connect( task, &QgsTask::statusChanged, statusWidget, &QgsTaskStatusWidget::setStatus );
    connect( statusWidget, &QgsTaskStatusWidget::cancelClicked, task, &QgsTask::cancel );
    mTreeView->setIndexWidget( mModel->index( row, QgsTaskManagerModel::Status ), statusWidget );
  }
}

void QgsTaskManagerWidget::clicked( const QModelIndex &index )
{
  QgsTask *task = mModel->indexToTask( index );
  if ( !task )
    return;

  mManager->triggerTask( task );
}

///@cond PRIVATE
//
// QgsTaskManagerModel
//

QgsTaskManagerModel::QgsTaskManagerModel( QgsTaskManager *manager, QObject *parent )
  : QAbstractItemModel( parent )
  , mManager( manager )
{
  Q_ASSERT( mManager );

  //populate row to id map
  const auto constTasks = mManager->tasks();
  for ( QgsTask *task : constTasks )
  {
    mRowToTaskIdList << mManager->taskId( task );
  }

  connect( mManager, &QgsTaskManager::taskAdded, this, &QgsTaskManagerModel::taskAdded );
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

  if ( !parent.isValid() && row >= 0 && row < mRowToTaskIdList.count() )
  {
    //return an index for the task at this position
    return createIndex( row, column );
  }

  //only top level supported
  return QModelIndex();

}

QModelIndex QgsTaskManagerModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )

  //all items are top level
  return QModelIndex();
}

int QgsTaskManagerModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mRowToTaskIdList.count();
  }
  else
  {
    //no children
    return 0;
  }
}

int QgsTaskManagerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 3;
}

QVariant QgsTaskManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsTask *task = indexToTask( index );
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
            // delegate shows status
            return QVariant();
          default:
            return QVariant();
        }

      case StatusRole:
        return static_cast<int>( task->status() );

      case Qt::ToolTipRole:
        switch ( index.column() )
        {
          case Description:
            return createTooltip( task, ToolTipDescription );
          case Progress:
            return createTooltip( task, ToolTipProgress );
          case Status:
            return createTooltip( task, ToolTipStatus );
          default:
            return QVariant();
        }


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

  QgsTask *task = indexToTask( index );
  if ( index.column() == Status )
  {
    if ( task && task->canCancel() )
      flags = flags | Qt::ItemIsEditable;
  }
  return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool QgsTaskManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( role )

  if ( !index.isValid() )
    return false;

  QgsTask *task = indexToTask( index );
  if ( !task )
    return false;

  switch ( index.column() )
  {
    case Status:
    {
      if ( value.toBool() && task->canCancel() )
        task->cancel();
      return true;
    }

    default:
      return false;
  }
}

void QgsTaskManagerModel::taskAdded( long id )
{
  beginInsertRows( QModelIndex(), mRowToTaskIdList.count(),
                   mRowToTaskIdList.count() );
  mRowToTaskIdList << id;
  endInsertRows();
}

void QgsTaskManagerModel::taskDeleted( long id )
{
  for ( int row = 0; row < mRowToTaskIdList.count(); ++row )
  {
    if ( mRowToTaskIdList.at( row ) == id )
    {
      beginRemoveRows( QModelIndex(), row, row );
      mRowToTaskIdList.removeAt( row );
      endRemoveRows();
      return;
    }
  }
}

void QgsTaskManagerModel::progressChanged( long id, double progress )
{
  Q_UNUSED( progress )

  const QModelIndex index = idToIndex( id, Progress );
  if ( !index.isValid() )
  {
    return;
  }

  emit dataChanged( index, index );
}

void QgsTaskManagerModel::statusChanged( long id, int status )
{
  if ( status == QgsTask::Complete || status == QgsTask::Terminated )
  {
    taskDeleted( id );
  }
  else
  {
    const QModelIndex index = idToIndex( id, Status );
    if ( !index.isValid() )
    {
      return;
    }

    emit dataChanged( index, index );
  }
}

QgsTask *QgsTaskManagerModel::indexToTask( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.parent().isValid() )
    return nullptr;

  const long id = index.row() >= 0 && index.row() < mRowToTaskIdList.count() ? mRowToTaskIdList.at( index.row() ) : -1;
  if ( id >= 0 )
    return mManager->task( id );
  else
    return nullptr;
}

int QgsTaskManagerModel::idToRow( long id ) const
{
  for ( int row = 0; row < mRowToTaskIdList.count(); ++row )
  {
    if ( mRowToTaskIdList.at( row ) == id )
    {
      return row;
    }
  }
  return -1;
}

QModelIndex QgsTaskManagerModel::idToIndex( long id, int column ) const
{
  const int row = idToRow( id );
  if ( row < 0 )
    return QModelIndex();

  return index( row, column );
}

QString QgsTaskManagerModel::createTooltip( QgsTask *task, ToolTipType type )
{
  if ( task->status() != QgsTask::Running )
  {
    switch ( type )
    {
      case ToolTipDescription:
        return task->description();

      case ToolTipStatus:
      case ToolTipProgress:
      {
        switch ( task->status() )
        {
          case QgsTask::Queued:
            return tr( "Queued" );
          case QgsTask::OnHold:
            return tr( "On hold" );
          case QgsTask::Running:
          {
            if ( type == ToolTipStatus && !task->canCancel() )
              return tr( "Running (cannot cancel)" );
            else
              return tr( "Running" );
          }
          case QgsTask::Complete:
            return tr( "Complete" );
          case QgsTask::Terminated:
            return tr( "Terminated" );
        }
      }
    }
  }

  QString formattedTime;

  const qint64 elapsed = task->elapsedTime();

  if ( task->progress() > 0 )
  {
    // estimate time remaining
    const qint64 msRemain = static_cast< qint64 >( elapsed * 100.0 / task->progress() - elapsed );
    if ( msRemain > 120 * 1000 )
    {
      const long long minutes = msRemain / 1000 / 60;
      const int seconds = ( msRemain / 1000 ) % 60;
      formattedTime = tr( "%1:%2 minutes" ).arg( minutes ).arg( seconds, 2, 10, QChar( '0' ) );
    }
    else
      formattedTime = tr( "%1 seconds" ).arg( msRemain / 1000 );

    formattedTime = tr( "Estimated time remaining: %1" ).arg( formattedTime );

    const QTime estimatedEnd = QTime::currentTime().addMSecs( msRemain );
    formattedTime += tr( " (%1)" ).arg( QLocale::system().toString( estimatedEnd, QLocale::ShortFormat ) );
  }
  else
  {
    if ( elapsed > 120 * 1000 )
    {
      const long long minutes = elapsed / 1000 / 60;
      const int seconds = ( elapsed / 1000 ) % 60;
      formattedTime = tr( "%1:%2 minutes" ).arg( minutes ).arg( seconds, 2, 10, QChar( '0' ) );
    }
    else
      formattedTime = tr( "%1 seconds" ).arg( elapsed / 1000 );

    formattedTime = tr( "Time elapsed: %1" ).arg( formattedTime );
  }

  switch ( type )
  {
    case ToolTipDescription:
      return tr( "%1<br>%2" ).arg( task->description(), formattedTime );

    case ToolTipStatus:
    case ToolTipProgress:
    {
      switch ( task->status() )
      {
        case QgsTask::Queued:
          return tr( "Queued" );
        case QgsTask::OnHold:
          return tr( "On hold" );
        case QgsTask::Running:
        {
          QString statusDesc;
          if ( type == ToolTipStatus && !task->canCancel() )
            statusDesc = tr( "Running (cannot cancel)" );
          else
            statusDesc = tr( "Running" );
          return tr( "%1<br>%2" ).arg( statusDesc, formattedTime );
        }
        case QgsTask::Complete:
          return tr( "Complete" );
        case QgsTask::Terminated:
          return tr( "Terminated" );
      }
    }
  }
  // no warnings
  return QString();
}


//
// QgsTaskStatusDelegate
//

QgsTaskStatusWidget::QgsTaskStatusWidget( QWidget *parent, QgsTask::TaskStatus status, bool canCancel )
  : QWidget( parent )
  , mCanCancel( canCancel )
  , mStatus( status )
{
  setMouseTracking( true );
}

QSize QgsTaskStatusWidget::sizeHint() const
{
  return QSize( 32, 32 );
}

void QgsTaskStatusWidget::setStatus( int status )
{
  mStatus = static_cast< QgsTask::TaskStatus >( status );
  update();
}

void QgsTaskStatusWidget::paintEvent( QPaintEvent *e )
{
  QWidget::paintEvent( e );

  QIcon icon;
  if ( mInside && ( mCanCancel || ( mStatus == QgsTask::Queued || mStatus == QgsTask::OnHold ) ) )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mTaskCancel.svg" ) );
  }
  else
  {
    switch ( mStatus )
    {
      case QgsTask::Queued:
        icon = QgsApplication::getThemeIcon( QStringLiteral( "/mTaskQueued.svg" ) );
        break;
      case QgsTask::OnHold:
        icon = QgsApplication::getThemeIcon( QStringLiteral( "/mTaskOnHold.svg" ) );
        break;
      case QgsTask::Running:
        icon = QgsApplication::getThemeIcon( QStringLiteral( "/mTaskRunning.svg" ) );
        break;
      case QgsTask::Complete:
        icon = QgsApplication::getThemeIcon( QStringLiteral( "/mTaskComplete.svg" ) );
        break;
      case QgsTask::Terminated:
        icon = QgsApplication::getThemeIcon( QStringLiteral( "/mTaskTerminated.svg" ) );
        break;
    }
  }

  QPainter p( this );
  icon.paint( &p, 1, height() / 2 - 12, 24, 24 );
  p.end();
}

void QgsTaskStatusWidget::mousePressEvent( QMouseEvent * )
{
  if ( mCanCancel || ( mStatus == QgsTask::Queued || mStatus == QgsTask::OnHold ) )
    emit cancelClicked();
}

void QgsTaskStatusWidget::mouseMoveEvent( QMouseEvent * )
{
  if ( !mInside )
  {
    mInside = true;
    update();
  }
}

void QgsTaskStatusWidget::leaveEvent( QEvent * )
{
  mInside = false;
  update();
}


/*
bool QgsTaskStatusWidget::editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index )
{
  Q_UNUSED( option )
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
*/

QgsTaskManagerFloatingWidget::QgsTaskManagerFloatingWidget( QgsTaskManager *manager, QWidget *parent )
  : QgsFloatingWidget( parent )
{
  setLayout( new QVBoxLayout() );
  QgsTaskManagerWidget *w = new QgsTaskManagerWidget( manager );

  const int minWidth = static_cast< int >( fontMetrics().horizontalAdvance( 'X' ) * 60 * Qgis::UI_SCALE_FACTOR );
  const int minHeight = static_cast< int >( fontMetrics().height() * 15 * Qgis::UI_SCALE_FACTOR );
  setMinimumSize( minWidth, minHeight );
  layout()->addWidget( w );
  setStyleSheet( ".QgsTaskManagerFloatingWidget { border-top-left-radius: 8px;"
                 "border-top-right-radius: 8px; background-color: rgba(0, 0, 0, 70%); }" );
}


QgsTaskManagerStatusBarWidget::QgsTaskManagerStatusBarWidget( QgsTaskManager *manager, QWidget *parent )
  : QToolButton( parent )
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

  if ( manager->countActiveTasks() )
    showButton();
}

QSize QgsTaskManagerStatusBarWidget::sizeHint() const
{
  const int width = static_cast< int >( fontMetrics().horizontalAdvance( 'X' ) * 20 * Qgis::UI_SCALE_FACTOR );
  const int height = QToolButton::sizeHint().height();
  return QSize( width, height );
}

void QgsTaskManagerStatusBarWidget::changeEvent( QEvent *event )
{
  QToolButton::changeEvent( event );

  if ( event->type() == QEvent::FontChange )
  {
    mProgressBar->setFont( font() );
  }
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
  mProgressBar->setValue( static_cast< int >( std::round( progress ) ) );
  if ( qgsDoubleNear( progress, 0.0 ) )
    mProgressBar->setMaximum( 0 );
  else if ( mProgressBar->maximum() == 0 )
    mProgressBar->setMaximum( 100 );
  setToolTip( QgsTaskManagerModel::createTooltip( mManager->activeTasks().at( 0 ), QgsTaskManagerModel::ToolTipDescription ) );
}

void QgsTaskManagerStatusBarWidget::countActiveTasksChanged( int count )
{
  if ( count > 1 )
  {
    mProgressBar->setMaximum( 0 );
    setToolTip( tr( "%n active task(s) running", nullptr, count ) );
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
    mProgressBar->setMaximum( 0 );
    mProgressBar->setValue( 0 );
    show();
  }
}
///@endcond
