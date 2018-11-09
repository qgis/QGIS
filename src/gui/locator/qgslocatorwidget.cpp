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
#include "qgslocatormodel.h"
#include "qgsfilterlineedit.h"
#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include <QLayout>
#include <QCompleter>
#include <QMenu>

QgsLocatorWidget::QgsLocatorWidget( QWidget *parent )
  : QWidget( parent )
  , mLocator( new QgsLocator( this ) )
  , mLineEdit( new QgsFilterLineEdit() )
  , mLocatorModel( new QgsLocatorModel( this ) )
  , mResultsView( new QgsLocatorResultsView() )
{
  mLineEdit->setShowClearButton( true );
#ifdef Q_OS_MACX
  mLineEdit->setPlaceholderText( tr( "Type to locate (⌘K)" ) );
#else
  mLineEdit->setPlaceholderText( tr( "Type to locate (Ctrl+K)" ) );
#endif

  int placeholderMinWidth = mLineEdit->fontMetrics().width( mLineEdit->placeholderText() );
  int minWidth = std::max( 200, static_cast< int >( placeholderMinWidth * 1.8 ) );
  resize( minWidth, 30 );
  QSizePolicy sizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  sizePolicy.setHorizontalStretch( 0 );
  sizePolicy.setVerticalStretch( 0 );
  setSizePolicy( sizePolicy );
  setMinimumSize( QSize( minWidth, 0 ) );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setMargin( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mLineEdit );
  setLayout( layout );

  setFocusProxy( mLineEdit );

  // setup floating container widget
  mResultsContainer = new QgsFloatingWidget( parent ? parent->window() : nullptr );
  mResultsContainer->setAnchorWidget( mLineEdit );
  mResultsContainer->setAnchorPoint( QgsFloatingWidget::BottomLeft );
  mResultsContainer->setAnchorWidgetPoint( QgsFloatingWidget::TopLeft );

  QHBoxLayout *containerLayout = new QHBoxLayout();
  containerLayout->setMargin( 0 );
  containerLayout->setContentsMargins( 0, 0, 0, 0 );
  containerLayout->addWidget( mResultsView );
  mResultsContainer->setLayout( containerLayout );
  mResultsContainer->hide();

  mProxyModel = new QgsLocatorProxyModel( mLocatorModel );
  mProxyModel->setSourceModel( mLocatorModel );
  mResultsView->setModel( mProxyModel );
  mResultsView->setUniformRowHeights( true );
  mResultsView->setIconSize( QSize( 16, 16 ) );
  mResultsView->recalculateSize();

  connect( mLocator, &QgsLocator::foundResult, this, &QgsLocatorWidget::addResult );
  connect( mLocator, &QgsLocator::finished, this, &QgsLocatorWidget::searchFinished );
  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsLocatorWidget::scheduleDelayedPopup );
  connect( mResultsView, &QAbstractItemView::activated, this, &QgsLocatorWidget::acceptCurrentEntry );

  // have a tiny delay between typing text in line edit and showing the window
  mPopupTimer.setInterval( 100 );
  mPopupTimer.setSingleShot( true );
  connect( &mPopupTimer, &QTimer::timeout, this, &QgsLocatorWidget::performSearch );
  mFocusTimer.setInterval( 110 );
  mFocusTimer.setSingleShot( true );
  connect( &mFocusTimer, &QTimer::timeout, this, &QgsLocatorWidget::triggerSearchAndShowList );

  mLineEdit->installEventFilter( this );
  mResultsContainer->installEventFilter( this );
  mResultsView->installEventFilter( this );
  installEventFilter( this );
  window()->installEventFilter( this );

  mLocator->registerFilter( new QgsLocatorFilterFilter( this, this ) );

  mMenu = new QMenu( this );
  QAction *menuAction = mLineEdit->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/search.svg" ) ), QLineEdit::LeadingPosition );
  connect( menuAction, &QAction::triggered, this, [ = ]
  {
    mFocusTimer.stop();
    mResultsContainer->hide();
    mMenu->exec( QCursor::pos() );
  } );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsLocatorWidget::configMenuAboutToShow );

}

QgsLocator *QgsLocatorWidget::locator()
{
  return mLocator;
}

void QgsLocatorWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

void QgsLocatorWidget::search( const QString &string )
{
  mLineEdit->setText( string );
  window()->activateWindow(); // window must also be active - otherwise floating docks can steal keystrokes
  mLineEdit->setFocus();
  performSearch();
}

void QgsLocatorWidget::invalidateResults()
{
  mLocator->cancelWithoutBlocking();
  mLocatorModel->clear();
  mResultsContainer->hide();
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

void QgsLocatorWidget::triggerSearchAndShowList()
{
  if ( mProxyModel->rowCount() == 0 )
    performSearch();
  else
    showList();
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
  else
  {
    if ( !mLocator->isRunning() )
      mLineEdit->setShowSpinner( false );
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
        triggerSearchAndShowList();
        mHasSelectedResult = true;
        QgsApplication::sendEvent( mResultsView, event );
        return true;
      case Qt::Key_Home:
      case Qt::Key_End:
        if ( keyEvent->modifiers() & Qt::ControlModifier )
        {
          triggerSearchAndShowList();
          mHasSelectedResult = true;
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
        mHasSelectedResult = true;
        mResultsView->selectNextResult();
        return true;
      case Qt::Key_Backtab:
        mHasSelectedResult = true;
        mResultsView->selectPreviousResult();
        return true;
      default:
        break;
    }
  }
  else if ( obj == mResultsView && event->type() == QEvent::MouseButtonPress )
  {
    mHasSelectedResult = true;
  }
  else if ( event->type() == QEvent::FocusOut && ( obj == mLineEdit || obj == mResultsContainer || obj == mResultsView ) )
  {
    if ( !mLineEdit->hasFocus() && !mResultsContainer->hasFocus() && !mResultsView->hasFocus() )
    {
      mFocusTimer.stop();
      mResultsContainer->hide();
    }
  }
  else if ( event->type() == QEvent::FocusIn && obj == mLineEdit )
  {
    mFocusTimer.start();
  }
  else if ( obj == window() && event->type() == QEvent::Resize )
  {
    mResultsView->recalculateSize();
  }
  return QWidget::eventFilter( obj, event );
}

void QgsLocatorWidget::addResult( const QgsLocatorResult &result )
{
  bool selectFirst = !mHasSelectedResult || mProxyModel->rowCount() == 0;
  mLocatorModel->addResult( result );
  if ( selectFirst )
  {
    int row = -1;
    bool selectable = false;
    while ( !selectable && row < mProxyModel->rowCount() )
    {
      row++;
      selectable = mProxyModel->flags( mProxyModel->index( row, 0 ) ).testFlag( Qt::ItemIsSelectable );
    }
    if ( selectable )
      mResultsView->setCurrentIndex( mProxyModel->index( row, 0 ) );
  }
}

void QgsLocatorWidget::configMenuAboutToShow()
{
  mMenu->clear();
  for ( QgsLocatorFilter *filter : mLocator->filters() )
  {
    if ( !filter->enabled() )
      continue;

    QAction *action = new QAction( filter->displayName(), mMenu );
    connect( action, &QAction::triggered, this, [ = ]
    {
      QString currentText = mLineEdit->text();
      if ( currentText.isEmpty() )
        currentText = tr( "<type here>" );
      else
      {
        QStringList parts = currentText.split( ' ' );
        if ( parts.count() > 1 && mLocator->filters( parts.at( 0 ) ).count() > 0 )
        {
          parts.pop_front();
          currentText = parts.join( ' ' );
        }
      }

      mLineEdit->setText( filter->activePrefix() + ' ' + currentText );
      mLineEdit->setSelection( filter->activePrefix().length() + 1, currentText.length() );
    } );
    mMenu->addAction( action );
  }
  mMenu->addSeparator();
  QAction *configAction = new QAction( tr( "Configure…" ), mMenu );
  connect( configAction, &QAction::triggered, this, &QgsLocatorWidget::configTriggered );
  mMenu->addAction( configAction );

}

void QgsLocatorWidget::updateResults( const QString &text )
{
  mLineEdit->setShowSpinner( true );
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
    mHasSelectedResult = false;
    mLocatorModel->deferredClear();
    mLocator->fetchResults( text, createContext() );
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

    QgsLocatorResult result = mProxyModel->data( index, QgsLocatorModel::ResultDataRole ).value< QgsLocatorResult >();
    mResultsContainer->hide();
    mLineEdit->clearFocus();
    mLocator->clearPreviousResults();
    result.filter->triggerResult( result );
  }
}

QgsLocatorContext QgsLocatorWidget::createContext()
{
  QgsLocatorContext context;
  if ( mMapCanvas )
  {
    context.targetExtent = mMapCanvas->mapSettings().visibleExtent();
    context.targetExtentCrs = mMapCanvas->mapSettings().destinationCrs();
  }
  return context;
}

///@cond PRIVATE

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
  int width = std::max( 300, window()->size().width() / 2 );
  QSize newSize( width, rowSize + frameWidth() * 2 );
  // resize the floating widget this is contained within
  parentWidget()->resize( newSize );
  QTreeView::resize( newSize );

  header()->resizeSection( 0, width / 2 );
  header()->resizeSection( 1, 0 );
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

//
// QgsLocatorFilterFilter
//

QgsLocatorFilterFilter::QgsLocatorFilterFilter( QgsLocatorWidget *locator, QObject *parent )
  : QgsLocatorFilter( parent )
  , mLocator( locator )
{}

QgsLocatorFilterFilter *QgsLocatorFilterFilter::clone() const
{
  return new QgsLocatorFilterFilter( mLocator );
}

QgsLocatorFilter::Flags QgsLocatorFilterFilter::flags() const
{
  return QgsLocatorFilter::FlagFast;
}

void QgsLocatorFilterFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  if ( !string.isEmpty() )
  {
    //only shows results when nothing typed
    return;
  }

  for ( QgsLocatorFilter *filter : mLocator->locator()->filters() )
  {
    if ( feedback->isCanceled() )
      return;

    if ( filter == this || !filter || !filter->enabled() )
      continue;

    QgsLocatorResult result;
    result.displayString = filter->activePrefix();
    result.description = filter->displayName();
    result.userData = filter->activePrefix() + ' ';
    result.icon = QgsApplication::getThemeIcon( QStringLiteral( "/search.svg" ) );
    emit resultFetched( result );
  }
}

void QgsLocatorFilterFilter::triggerResult( const QgsLocatorResult &result )
{
  mLocator->search( result.userData.toString() );
}


///@endcond
