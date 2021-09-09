/***************************************************************************
                          qgsmessagelogviewer.cpp  -  description
                             -------------------
    begin                : October 2011
    copyright            : (C) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessagelogviewer.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsdockwidget.h"

#include <QFile>
#include <QDateTime>
#include <QTableWidget>
#include <QToolButton>
#include <QStatusBar>
#include <QToolTip>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QDebug>
#include <QDesktopServices>

QgsMessageLogViewer::QgsMessageLogViewer( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );

  connect( QgsApplication::messageLog(), static_cast<void ( QgsMessageLog::* )( const QString &, const QString &, Qgis::MessageLevel )>( &QgsMessageLog::messageReceived ),
           this, static_cast<void ( QgsMessageLogViewer::* )( const QString &, const QString &, Qgis::MessageLevel )>( &QgsMessageLogViewer::logMessage ) );

  connect( tabWidget, &QTabWidget::tabCloseRequested, this, &QgsMessageLogViewer::closeTab );

  mTabBarContextMenu = new QMenu( this );
  tabWidget->tabBar()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( tabWidget->tabBar(), &QWidget::customContextMenuRequested, this, &QgsMessageLogViewer::showContextMenuForTabBar );
}

void QgsMessageLogViewer::showContextMenuForTabBar( QPoint point )
{
  if ( point.isNull() )
  {
    return;
  }

  mTabBarContextMenu->clear();

  const int tabIndex = tabWidget->tabBar()->tabAt( point );

  QAction *actionCloseTab = new QAction( tr( "Close Tab" ), mTabBarContextMenu );
  connect( actionCloseTab, &QAction::triggered, this, [this, tabIndex]
  {
    closeTab( tabIndex );
  }
         );
  mTabBarContextMenu->addAction( actionCloseTab );

  QAction *actionCloseOtherTabs = new QAction( tr( "Close Other Tabs" ), mTabBarContextMenu );
  actionCloseOtherTabs->setEnabled( tabWidget->tabBar()->count() > 1 );
  connect( actionCloseOtherTabs, &QAction::triggered, this, [this, tabIndex]
  {
    int i;
    for ( i = tabWidget->tabBar()->count() - 1; i >= 0; i-- )
    {
      if ( i != tabIndex )
      {
        closeTab( i );
      }
    }
  }
         );
  mTabBarContextMenu->addAction( actionCloseOtherTabs );

  QAction *actionCloseAllTabs = new QAction( tr( "Close All Tabs" ), mTabBarContextMenu );
  actionCloseAllTabs->setEnabled( tabWidget->tabBar()->count() > 0 );
  connect( actionCloseAllTabs, &QAction::triggered, this, [this]
  {
    int i;
    for ( i = tabWidget->tabBar()->count() - 1; i >= 0; i-- )
    {
      closeTab( i );
    }
  }
         );
  mTabBarContextMenu->addAction( actionCloseAllTabs );

  mTabBarContextMenu->exec( tabWidget->tabBar()->mapToGlobal( point ) );
}

void QgsMessageLogViewer::closeEvent( QCloseEvent *e )
{
  e->ignore();
}

void QgsMessageLogViewer::reject()
{
}

void QgsMessageLogViewer::logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level )
{
  constexpr int MESSAGE_COUNT_LIMIT = 10000;
  // Avoid logging too many messages, which might blow memory.
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
    return;
  ++mMessageLoggedCount;

  QString cleanedTag = tag;
  if ( cleanedTag.isNull() )
    cleanedTag = tr( "General" );

  int i;
  for ( i = 0; i < tabWidget->count() && tabWidget->tabText( i ).remove( QChar( '&' ) ) != cleanedTag; i++ );

  QPlainTextEdit *w = nullptr;
  if ( i < tabWidget->count() )
  {
    w = qobject_cast<QPlainTextEdit *>( tabWidget->widget( i ) );
    tabWidget->setCurrentIndex( i );
  }
  else
  {
    w = new QPlainTextEdit( this );
    w->setReadOnly( true );
    w->viewport()->installEventFilter( this );
    tabWidget->addTab( w, cleanedTag );
    tabWidget->setCurrentIndex( tabWidget->count() - 1 );
  }

  QString levelString;
  const QgsSettings settings;
  const QPalette pal = qApp->palette();
  const QString defaultColorName = pal.color( QPalette::WindowText ).name();
  QString colorName;
  switch ( level )
  {
    case Qgis::MessageLevel::Info:
      levelString = QStringLiteral( "INFO" );
      colorName = settings.value( QStringLiteral( "colors/info" ), QString() ).toString();
      break;
    case Qgis::MessageLevel::Warning:
      levelString = QStringLiteral( "WARNING" );
      colorName = settings.value( QStringLiteral( "colors/warning" ), QString() ).toString();
      break;
    case Qgis::MessageLevel::Critical:
      levelString = QStringLiteral( "CRITICAL" );
      colorName = settings.value( QStringLiteral( "colors/critical" ), QString() ).toString();
      break;
    case Qgis::MessageLevel::Success:
      levelString = QStringLiteral( "SUCCESS" );
      colorName = settings.value( QStringLiteral( "colors/success" ), QString() ).toString();
      break;
    case Qgis::MessageLevel::NoLevel:
      levelString = QStringLiteral( "NONE" );
      colorName = settings.value( QStringLiteral( "colors/default" ), QString() ).toString();
      break;
  }
  const QColor color = QColor( !colorName.isEmpty() ? colorName : defaultColorName );

  const QString prefix = QStringLiteral( "<font color=\"%1\">%2 &nbsp;&nbsp;&nbsp; %3 &nbsp;&nbsp;&nbsp;</font>" )
                         .arg( color.name(), QDateTime::currentDateTime().toString( Qt::ISODate ), levelString );
  QString cleanedMessage = message;
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
    cleanedMessage = tr( "Message log truncated" );

  cleanedMessage = cleanedMessage.prepend( prefix ).replace( '\n', QLatin1String( "<br>&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;" ) );
  w->appendHtml( cleanedMessage );
  w->verticalScrollBar()->setValue( w->verticalScrollBar()->maximum() );
  tabWidget->show();
  emptyLabel->hide();
}

void QgsMessageLogViewer::closeTab( int index )
{
  tabWidget->removeTab( index );
  if ( tabWidget->count() == 0 )
  {
    tabWidget->hide();
    emptyLabel->show();
  }
}

bool QgsMessageLogViewer::eventFilter( QObject *object, QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::MouseButtonPress:
    {
      if ( QPlainTextEdit *te = qobject_cast<QPlainTextEdit *>( object->parent() ) )
      {
        QMouseEvent *me = static_cast< QMouseEvent *>( event );
        mClickedAnchor = ( me->button() & Qt::LeftButton ) ? te->anchorAt( me->pos() ) :
                         QString();
        if ( !mClickedAnchor.isEmpty() )
          return true;
      }
      break;
    }

    case QEvent::MouseButtonRelease:
    {
      if ( QPlainTextEdit *te = qobject_cast<QPlainTextEdit *>( object->parent() ) )
      {
        QMouseEvent *me = static_cast< QMouseEvent *>( event );
        const QString clickedAnchor = ( me->button() & Qt::LeftButton ) ? te->anchorAt( me->pos() ) :
                                      QString();
        if ( !clickedAnchor.isEmpty() && clickedAnchor == mClickedAnchor )
        {
          QDesktopServices::openUrl( mClickedAnchor );
          return true;
        }
      }
      break;
    }

    default:
      break;
  }

  return QDialog::eventFilter( object, event );
}
