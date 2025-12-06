/***************************************************************************
                          qgswebengineview.cpp
                             -------------------
    begin                : December 2025
    copyright            : (C) 2025 by Jean-Baptiste Peter
    email                : jbpeter at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswebengineview.h"
#include "moc_qgswebengineview.cpp"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>
#include <QUrl>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QDialog>
#include <QDialogButtonBox>

/**
 * Constructor for QgsWebEngineView.
 * Creates a new web engine view widget with drag and drop support.
 * \param parent The parent widget
 */
QgsWebEngineView::QgsWebEngineView( QWidget *parent )
  : QWidget( parent )
  , mView { std::make_unique< QWebEngineView >() }
  , mDebugView { nullptr }
{
  // Set up the layout to contain the web engine view
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mView.get() );
  setLayout( layout );

  // Install event filter to intercept events from the web engine view
  mView->installEventFilter( this );
}

/**
 * Destructor for QgsWebEngineView.
 */
QgsWebEngineView::~QgsWebEngineView() = default;

/**
 * Sets the URL to be loaded in the web engine view.
 * \param url The URL to load
 */
void QgsWebEngineView::setUrl( const QUrl &url )
{
  if ( mView )
  {
    mView->setUrl( url );
  }
}

/**
 * Sets whether the web engine view accepts drop events.
 * \param accept True to accept drops, false otherwise
 */
void QgsWebEngineView::setAcceptDrops( bool accept )
{
  QWidget::setAcceptDrops( accept );
  if ( mView )
  {
    mView->setAcceptDrops( accept );
  }
}

/**
 * Sets the context menu policy for the web engine view.
 * \param policy The context menu policy to apply
 */
void QgsWebEngineView::setContextMenuPolicy( Qt::ContextMenuPolicy policy )
{
  if ( mView )
  {
    mView->setContextMenuPolicy( policy );
  }
}

/**
 * Opens a web inspector debug view in a separate dialog.
 */
void QgsWebEngineView::openDebugView()
{
  if ( !mView || !mView->page() )
    return;

  // Create debug view if it doesn't exist
  if ( !mDebugView )
  {
    // Create a new dialog to hold the debug view
    QDialog *debugDialog = new QDialog( this );
    debugDialog->setWindowTitle( tr( "Web Inspector" ) );
    debugDialog->resize( 800, 600 );
    debugDialog->setAttribute( Qt::WA_DeleteOnClose );

    // Create debug web view
    mDebugView = std::make_unique< QWebEngineView >( debugDialog );

    // Set up layout
    QVBoxLayout *layout = new QVBoxLayout( debugDialog );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( mDebugView.get() );

    debugDialog->setLayout( layout );

    // Clean up debug view when dialog is closed
    connect( debugDialog, &QDialog::destroyed, [this]() {
      mDebugView.reset();
    } );
  }

  // Connect the debug view to the main view's page
  if ( mDebugView )
  {
    mView->page()->setDevToolsPage( mDebugView->page() );

    // Show the debug dialog
    if ( QDialog *debugDialog = qobject_cast<QDialog *>( mDebugView->parent() ) )
    {
      debugDialog->show();
      debugDialog->raise();
      debugDialog->activateWindow();
    }
  }
}

/**
 * Handles drag enter events for the web engine view.
 * \param event The drag enter event
 */
void QgsWebEngineView::dragEnterEvent( QDragEnterEvent *event )
{
  event->acceptProposedAction();
}

/**
 * Handles drop events for the web engine view.
 * \param event The drop event
 */
void QgsWebEngineView::dropEvent( QDropEvent *event )
{
  QWidget::dropEvent( event );
}

/**
 * Event filter to intercept events from the web engine view.
 * Specifically handles drag and drop events by forwarding them to
 * the appropriate virtual methods for customization.
 * \param obj The object that received the event
 * \param event The event to process
 * \return True if the event was handled, false otherwise
 */
bool QgsWebEngineView::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == mView.get() )
  {
    switch ( event->type() )
    {
      case QEvent::DragEnter:
        dragEnterEvent( static_cast<QDragEnterEvent *>( event ) );
        return true;
      case QEvent::Drop:
        dropEvent( static_cast<QDropEvent *>( event ) );
        return true;
      default:
        break;
    }
  }
  return QWidget::eventFilter( obj, event );
}