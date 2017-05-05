/***************************************************************************
                         qgslocatorwidget.cpp
                         --------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
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


#include "qgslocatorwidget.h"
#include "qgslocator.h"
#include "qgsfilterlineedit.h"
#include <QLayout>
#include <QCompleter>
#include "qgsapplication.h"
#include "qgslogger.h"

QgsLocatorWidget::QgsLocatorWidget( QWidget *parent )
  : QWidget( parent )
  , mLocator( new QgsLocator( this ) )
  , mLineEdit( new QgsFilterLineEdit() )
  , mLocatorModel( new QgsLocatorModel( this ) )
  , mResultsView( new QgsLocatorResultsView( this ) )
{
  mLineEdit->setShowClearButton( true );
  mLineEdit->setShowSearchIcon( true );

  resize( 200, 30 );
  QSizePolicy sizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  sizePolicy.setHorizontalStretch( 0 );
  sizePolicy.setVerticalStretch( 0 );
  setSizePolicy( sizePolicy );
  setMinimumSize( QSize( 200, 0 ) );

  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mLineEdit );
  setLayout( layout );

  setFocusProxy( mLineEdit );

  // setup floating container widget
  mResultsContainer = new QgsFloatingWidget( parent ? parent->window() : nullptr );
  mResultsContainer->setAnchorWidget( mLineEdit );
  mResultsContainer->setAnchorPoint( QgsFloatingWidget::BottomRight );
  mResultsContainer->setAnchorWidgetPoint( QgsFloatingWidget::TopRight );

  QHBoxLayout *containerLayout = new QHBoxLayout( this );
  containerLayout->setMargin( 0 );
  containerLayout->setContentsMargins( 0, 0, 0, 0 );
  containerLayout->addWidget( mResultsView );
  mResultsContainer->setLayout( containerLayout );
  mResultsContainer->hide();

  mResultsView->setModel( mLocatorModel );
  mResultsView->recalculateSize();

  connect( mLocator, &QgsLocator::foundResult, this, &QgsLocatorWidget::addResult );
  connect( mLocator, &QgsLocator::finished, this, &QgsLocatorWidget::searchFinished );
  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsLocatorWidget::scheduleDelayedPopup );
  connect( mResultsView, &QAbstractItemView::activated, this, &QgsLocatorWidget::acceptCurrentEntry );

  // have a tiny delay between typing text in line edit and showing the window
  mPopupTimer.setInterval( 100 );
  mPopupTimer.setSingleShot( true );
  connect( &mPopupTimer, &QTimer::timeout, this, &QgsLocatorWidget::performSearch );

  mLineEdit->installEventFilter( this );
  mResultsContainer->installEventFilter( this );
  mResultsView->installEventFilter( this );
  installEventFilter( this );
  window()->installEventFilter( this );
}

QgsLocator *QgsLocatorWidget::locator()
{
  return mLocator;
}

void QgsLocatorWidget::search( const QString &string )
{
  mLineEdit->setText( string );
  window()->activateWindow(); // window must also be active - otherwise floating docks can steal keystrokes
  mLineEdit->setFocus();
  performSearch();
}

void QgsLocatorWidget::scheduleDelayedPopup()
{
  mPopupTimer.start();
}

void QgsLocatorWidget::performSearch()
{
  mPopupTimer.stop();
  updateResults( mLineEdit->text() );
  showList();
}

void QgsLocatorWidget::showList()
{
  mResultsContainer->show();
  mResultsContainer->raise();
}

void QgsLocatorWidget::searchFinished()
{
  if ( mHasQueuedRequest )
  {
    // a queued request was waiting for this - run the queued search now
    QString nextSearch = mNextRequestedString;
    mNextRequestedString.clear();
    mHasQueuedRequest = false;
    updateResults( nextSearch );
  }
}

bool QgsLocatorWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == mLineEdit && event->type() == QEvent::KeyPress )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    switch ( keyEvent->key() )
    {
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_PageUp:
      case Qt::Key_PageDown:
        showList();
        QgsApplication::sendEvent( mResultsView, event );
        return true;
      case Qt::Key_Home:
      case Qt::Key_End:
        if ( keyEvent->modifiers() & Qt::ControlModifier )
        {
          showList();
          QgsApplication::sendEvent( mResultsView, event );
          return true;
        }
        break;
      case Qt::Key_Enter:
      case Qt::Key_Return:
        acceptCurrentEntry();
        return true;
      case Qt::Key_Escape:
        mResultsContainer->hide();
        return true;
      case Qt::Key_Tab:
        mResultsView->selectNextResult();
        return true;
      case Qt::Key_Backtab:
        mResultsView->selectPreviousResult();
        return true;
      default:
        break;
    }
  }
  else if ( event->type() == QEvent::FocusOut && ( obj == mLineEdit || obj == mResultsContainer || obj == mResultsView ) )
  {
    if ( !mLineEdit->hasFocus() && !mResultsContainer->hasFocus() && !mResultsView->hasFocus() )
    {
      mResultsContainer->hide();
    }
  }
  else if ( event->type() == QEvent::FocusIn && obj == mLineEdit )
  {
    showList();
  }
  else if ( obj == window() && event->type() == QEvent::Resize )
  {
    mResultsView->recalculateSize();
  }
  return QWidget::eventFilter( obj, event );
}

void QgsLocatorWidget::addResult( const QgsLocatorResult &result )
{
  bool selectFirst = mLocatorModel->rowCount() == 0;
  mLocatorModel->addResult( result );
  if ( selectFirst )
    mResultsView->setCurrentIndex( mLocatorModel->index( 0, 0 ) );
}

void QgsLocatorWidget::updateResults( const QString &text )
{
  if ( mLocator->isRunning() )
  {
    // can't do anything while a query is running, and can't block
    // here waiting for the current query to cancel
    // so we queue up this string until cancel has happened
    mLocator->cancelWithoutBlocking();
    mNextRequestedString = text;
    mHasQueuedRequest = true;
    return;
  }
  else
  {
    mLocatorModel->clear();
    if ( !text.isEmpty() )
      mLocator->fetchResults( text );
  }
}

void QgsLocatorWidget::acceptCurrentEntry()
{
  if ( mHasQueuedRequest )
  {
    return;
  }
  else
  {
    if ( !mResultsView->isVisible() )
      return;

    QModelIndex index = mResultsView->currentIndex();
    if ( !index.isValid() )
      return;

    QgsLocatorResult result = mLocatorModel->data( index, QgsLocatorModel::ResultDataRole ).value< QgsLocatorResult >();
    mResultsContainer->hide();
    mLineEdit->clearFocus();
    result.filter->triggerResult( result );
  }
}

///@cond PRIVATE

//
// QgsLocatorModel
//

QgsLocatorModel::QgsLocatorModel( QObject *parent )
  : QAbstractListModel( parent )
{}

void QgsLocatorModel::clear()
{
  beginResetModel();
  mResults.clear();
  endResetModel();
}

int QgsLocatorModel::rowCount( const QModelIndex & ) const
{
  return mResults.size();
}

int QgsLocatorModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QVariant QgsLocatorModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return mResults.at( index.row() ).displayString;

    case ResultDataRole:
      return QVariant::fromValue( mResults.at( index.row() ) );
  }

  return QVariant();
}

void QgsLocatorModel::addResult( const QgsLocatorResult &result )
{
  beginInsertRows( QModelIndex(), mResults.size(), mResults.size() );
  mResults << result;
  endInsertRows();
}


//
// QgsLocatorResultsView
//

QgsLocatorResultsView::QgsLocatorResultsView( QWidget *parent )
  : QTreeView( parent )
{
  setRootIsDecorated( false );
  setUniformRowHeights( true );
  header()->hide();
  header()->setStretchLastSection( true );
}

void QgsLocatorResultsView::recalculateSize()
{
  // try to show about 20 rows
  int rowSize = 20 * itemDelegate()->sizeHint( viewOptions(), model()->index( 0, 0 ) ).height();

  // try to take up a sensible portion of window width (about half)
  int width = qMax( 300, window()->size().width() / 2 );
  QSize newSize( width, rowSize + frameWidth() * 2 );
  // resize the floating widget this is contained within
  parentWidget()->resize( newSize );
  QTreeView::resize( newSize );
}

void QgsLocatorResultsView::selectNextResult()
{
  int nextRow = currentIndex().row() + 1;
  nextRow = nextRow % model()->rowCount( QModelIndex() );
  setCurrentIndex( model()->index( nextRow, 0 ) );
}

void QgsLocatorResultsView::selectPreviousResult()
{
  int previousRow = currentIndex().row() - 1;
  if ( previousRow < 0 )
    previousRow = model()->rowCount( QModelIndex() ) - 1;
  setCurrentIndex( model()->index( previousRow, 0 ) );
}

///@endcond

