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

QgsMessageLogViewer::QgsMessageLogViewer( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );

  connect( QgsApplication::messageLog(), static_cast<void ( QgsMessageLog::* )( const QString &, const QString &, QgsMessageLog::MessageLevel )>( &QgsMessageLog::messageReceived ),
           this, static_cast<void ( QgsMessageLogViewer::* )( const QString &, const QString &, QgsMessageLog::MessageLevel )>( &QgsMessageLogViewer::logMessage ) );

  connect( tabWidget, &QTabWidget::tabCloseRequested, this, &QgsMessageLogViewer::closeTab );
}

void QgsMessageLogViewer::closeEvent( QCloseEvent *e )
{
  e->ignore();
}

void QgsMessageLogViewer::reject()
{
}

void QgsMessageLogViewer::logMessage( const QString &message, const QString &tag, QgsMessageLog::MessageLevel level )
{
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
    tabWidget->addTab( w, cleanedTag );
    tabWidget->setCurrentIndex( tabWidget->count() - 1 );
    tabWidget->setTabsClosable( true );
  }

  QString levelString;
  switch ( level )
  {
    case QgsMessageLog::INFO:
      levelString = QStringLiteral( "INFO" );
      break;
    case QgsMessageLog::WARNING:
      levelString = QStringLiteral( "WARNING" );
      break;
    case QgsMessageLog::CRITICAL:
      levelString = QStringLiteral( "CRITICAL" );
      break;
    case QgsMessageLog::NONE:
      levelString = QStringLiteral( "NONE" );
      break;
  }

  QString prefix = QStringLiteral( "%1\t%2\t" )
                   .arg( QDateTime::currentDateTime().toString( Qt::ISODate ), levelString );
  QString cleanedMessage = message;
  cleanedMessage = cleanedMessage.prepend( prefix ).replace( '\n', QLatin1String( "\n\t\t\t" ) );
  w->appendPlainText( cleanedMessage );
  w->verticalScrollBar()->setValue( w->verticalScrollBar()->maximum() );
}

void QgsMessageLogViewer::closeTab( int index )
{
  tabWidget->removeTab( index );
  tabWidget->setTabsClosable( tabWidget->count() > 1 );
}
