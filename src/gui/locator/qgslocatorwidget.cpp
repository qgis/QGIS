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

#include "qgslocator.h"
#include "qgslocatormodel.h"
#include "qgslocatorwidget.h"
#include "qgslocatormodelbridge.h"
#include "qgsfilterlineedit.h"
#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsguiutils.h"

#include <QLayout>
#include <QCompleter>
#include <QMenu>
#include <QTextLayout>
#include <QTextLine>

QgsLocatorWidget::QgsLocatorWidget( QWidget *parent )
  : QWidget( parent )
  , mModelBridge( new QgsLocatorModelBridge( this ) )
  , mLineEdit( new QgsLocatorLineEdit( this ) )
  , mResultsView( new QgsLocatorResultsView() )
{
  mLineEdit->setShowClearButton( true );
#ifdef Q_OS_MACX
  mLineEdit->setPlaceholderText( tr( "Type to locate (⌘K)" ) );
#else
  mLineEdit->setPlaceholderText( tr( "Type to locate (Ctrl+K)" ) );
#endif

  int placeholderMinWidth = mLineEdit->fontMetrics().boundingRect( mLineEdit->placeholderText() ).width();
  int minWidth = std::max( 200, static_cast< int >( placeholderMinWidth * 1.8 ) );
  resize( minWidth, 30 );
  QSizePolicy sizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  sizePolicy.setHorizontalStretch( 0 );
  sizePolicy.setVerticalStretch( 0 );
  setSizePolicy( sizePolicy );
  setMinimumSize( QSize( minWidth, 0 ) );

  QHBoxLayout *layout = new QHBoxLayout();
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
  containerLayout->setContentsMargins( 0, 0, 0, 0 );
  containerLayout->addWidget( mResultsView );
  mResultsContainer->setLayout( containerLayout );
  mResultsContainer->hide();

  mResultsView->setModel( mModelBridge->proxyModel() );
  mResultsView->setUniformRowHeights( true );

  int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  mResultsView->setIconSize( QSize( iconSize, iconSize ) );
  mResultsView->recalculateSize();
  mResultsView->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsLocatorWidget::scheduleDelayedPopup );
  connect( mResultsView, &QAbstractItemView::activated, this, &QgsLocatorWidget::acceptCurrentEntry );
  connect( mResultsView, &QAbstractItemView::customContextMenuRequested, this, &QgsLocatorWidget::showContextMenu );

  connect( mModelBridge, &QgsLocatorModelBridge::resultAdded, this, &QgsLocatorWidget::resultAdded );
  connect( mModelBridge, &QgsLocatorModelBridge::isRunningChanged, this, [ = ]() {mLineEdit->setShowSpinner( mModelBridge->isRunning() );} );
  connect( mModelBridge, &QgsLocatorModelBridge::resultsCleared, this, [ = ]() {mHasSelectedResult = false;} );

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

  mModelBridge->locator()->registerFilter( new QgsLocatorFilterFilter( this, this ) );

  mMenu = new QMenu( this );
  QAction *menuAction = mLineEdit->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/search.svg" ) ), QLineEdit::LeadingPosition );
  connect( menuAction, &QAction::triggered, this, [ = ]
  {
    mFocusTimer.stop();
    mResultsContainer->hide();
    mMenu->exec( QCursor::pos() );
  } );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsLocatorWidget::configMenuAboutToShow );

  mModelBridge->setTransformContext( QgsProject::instance()->transformContext() );
  connect( QgsProject::instance(), &QgsProject::transformContextChanged,
           this, [ = ]
  {
    mModelBridge->setTransformContext( QgsProject::instance()->transformContext() );
  } );
}

QgsLocator *QgsLocatorWidget::locator()
{
  return mModelBridge->locator();
}

void QgsLocatorWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  if ( mMapCanvas == canvas )
    return;

  for ( const QMetaObject::Connection &conn : std::as_const( mCanvasConnections ) )
  {
    disconnect( conn );
  }
  mCanvasConnections.clear();

  mMapCanvas = canvas;
  if ( mMapCanvas )
  {
    mModelBridge->updateCanvasExtent( mMapCanvas->mapSettings().visibleExtent() );
    mModelBridge->updateCanvasCrs( mMapCanvas->mapSettings().destinationCrs() );
    mCanvasConnections
    << connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, [ = ]() {mModelBridge->updateCanvasExtent( mMapCanvas->mapSettings().visibleExtent() );} )
    << connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, [ = ]() {mModelBridge->updateCanvasCrs( mMapCanvas->mapSettings().destinationCrs() );} ) ;
  }
}

void QgsLocatorWidget::search( const QString &string )
{
  window()->activateWindow(); // window must also be active - otherwise floating docks can steal keystrokes
  if ( string.isEmpty() )
  {
    mLineEdit->setFocus();
    mLineEdit->selectAll();
  }
  else
  {
    scheduleDelayedPopup();
    mLineEdit->setFocus();
    mLineEdit->setText( string );
    performSearch();
  }
}

void QgsLocatorWidget::invalidateResults()
{
  mModelBridge->invalidateResults();
  mResultsContainer->hide();
}

void QgsLocatorWidget::scheduleDelayedPopup()
{
  mPopupTimer.start();
}

void QgsLocatorWidget::resultAdded()
{
  bool selectFirst = !mHasSelectedResult || mModelBridge->proxyModel()->rowCount() == 0;
  if ( selectFirst )
  {
    int row = -1;
    bool selectable = false;
    while ( !selectable && row < mModelBridge->proxyModel()->rowCount() )
    {
      row++;
      selectable = mModelBridge->proxyModel()->flags( mModelBridge->proxyModel()->index( row, 0 ) ).testFlag( Qt::ItemIsSelectable );
    }
    if ( selectable )
      mResultsView->setCurrentIndex( mModelBridge->proxyModel()->index( row, 0 ) );
  }
}

void QgsLocatorWidget::showContextMenu( const QPoint &point )
{
  QModelIndex index = mResultsView->indexAt( point );
  if ( !index.isValid() )
    return;

  const QList<QgsLocatorResult::ResultAction> actions = mResultsView->model()->data( index, QgsLocatorModel::ResultActionsRole ).value<QList<QgsLocatorResult::ResultAction>>();
  QMenu *contextMenu = new QMenu( mResultsView );
  for ( auto resultAction : actions )
  {
    QAction *menuAction = new QAction( resultAction.text, contextMenu );
    if ( !resultAction.iconPath.isEmpty() )
      menuAction->setIcon( QIcon( resultAction.iconPath ) );
    connect( menuAction, &QAction::triggered, this, [ = ]() {mModelBridge->triggerResult( index, resultAction.id );} );
    contextMenu->addAction( menuAction );
  }
  contextMenu->exec( mResultsView->viewport()->mapToGlobal( point ) );
}

void QgsLocatorWidget::performSearch()
{
  mPopupTimer.stop();
  mModelBridge->performSearch( mLineEdit->text() );
  showList();
}

void QgsLocatorWidget::showList()
{
  mResultsContainer->show();
  mResultsContainer->raise();
}

void QgsLocatorWidget::triggerSearchAndShowList()
{
  if ( mModelBridge->proxyModel()->rowCount() == 0 )
    performSearch();
  else
    showList();
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
        if ( !mLineEdit->performCompletion() )
        {
          mHasSelectedResult = true;
          mResultsView->selectNextResult();
        }
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

void QgsLocatorWidget::configMenuAboutToShow()
{
  mMenu->clear();
  for ( QgsLocatorFilter *filter : mModelBridge->locator()->filters() )
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
        if ( parts.count() > 1 && mModelBridge->locator()->filters( parts.at( 0 ) ).count() > 0 )
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


void QgsLocatorWidget::acceptCurrentEntry()
{
  if ( mModelBridge->hasQueueRequested() )
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

    mResultsContainer->hide();
    mLineEdit->clearFocus();
    mModelBridge->triggerResult( index );
  }
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
  QStyleOptionViewItem optView;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  optView.init( this );
#else
  optView.initFrom( this );
#endif

  // try to show about 20 rows
  int rowSize = 20 * itemDelegate()->sizeHint( optView, model()->index( 0, 0 ) ).height();

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
  const int rowCount = model()->rowCount( QModelIndex() );
  if ( rowCount == 0 )
    return;

  int nextRow = currentIndex().row() + 1;
  nextRow = nextRow % rowCount;
  setCurrentIndex( model()->index( nextRow, 0 ) );
}

void QgsLocatorResultsView::selectPreviousResult()
{
  const int rowCount = model()->rowCount( QModelIndex() );
  if ( rowCount == 0 )
    return;

  int previousRow = currentIndex().row() - 1;
  if ( previousRow < 0 )
    previousRow = rowCount - 1;
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
    result.userData = QString( filter->activePrefix() + ' ' );
    result.icon = QgsApplication::getThemeIcon( QStringLiteral( "/search.svg" ) );
    emit resultFetched( result );
  }
}

void QgsLocatorFilterFilter::triggerResult( const QgsLocatorResult &result )
{
  mLocator->search( result.userData.toString() );
}

QgsLocatorLineEdit::QgsLocatorLineEdit( QgsLocatorWidget *locator, QWidget *parent )
  : QgsFilterLineEdit( parent )
  , mLocatorWidget( locator )
{
  connect( mLocatorWidget->locator(), &QgsLocator::searchPrepared, this, [&] { update(); } );
}

void QgsLocatorLineEdit::paintEvent( QPaintEvent *event )
{
  // this adds the completion as grey text at the right of the cursor
  // see https://stackoverflow.com/a/50425331/1548052
  // this is possible that the completion might be badly rendered if the cursor is larger than the line edit
  // this sounds acceptable as it is not very likely to use completion for super long texts
  // for more details see https://stackoverflow.com/a/54218192/1548052

  QLineEdit::paintEvent( event );

  if ( !hasFocus() )
    return;

  QString currentText = text();

  if ( currentText.length() == 0 || cursorPosition() < currentText.length() )
    return;

  const QStringList completionList = mLocatorWidget->locator()->completionList();

  mCompletionText.clear();
  QString completion;
  for ( const QString &candidate : completionList )
  {
    if ( candidate.startsWith( currentText ) )
    {
      completion = candidate.right( candidate.length() - currentText.length() );
      mCompletionText = candidate;
      break;
    }
  }

  if ( completion.isEmpty() )
    return;

  ensurePolished(); // ensure font() is up to date

  QRect cr = cursorRect();
  QPoint pos = cr.topRight() - QPoint( cr.width() / 2, 0 );

  QTextLayout l( completion, font() );
  l.beginLayout();
  QTextLine line = l.createLine();
  line.setLineWidth( width() - pos.x() );
  line.setPosition( pos );
  l.endLayout();

  QPainter p( this );
  p.setPen( QPen( Qt::gray, 1 ) );
  l.draw( &p, QPoint( 0, 0 ) );
}

bool QgsLocatorLineEdit::performCompletion()
{
  if ( !mCompletionText.isEmpty() )
  {
    setText( mCompletionText );
    mCompletionText.clear();
    return true;
  }
  else
    return false;
}


///@endcond
