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

QgsMessageLogViewer::QgsMessageLogViewer( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );

  connect( QgsApplication::messageLog(), static_cast<void ( QgsMessageLog::* )( const QString &, const QString &, Qgis::MessageLevel )>( &QgsMessageLog::messageReceived ),
           this, static_cast<void ( QgsMessageLogViewer::* )( const QString &, const QString &, Qgis::MessageLevel )>( &QgsMessageLogViewer::logMessage ) );

  connect( tabWidget, &QTabWidget::tabCloseRequested, this, &QgsMessageLogViewer::closeTab );
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
  }

  QString levelString;
  QgsSettings settings;
  QColor color;
  switch ( level )
  {
    case Qgis::Info:
      levelString = QStringLiteral( "INFO" );
      color = QColor( settings.value( QStringLiteral( "colors/info" ), QStringLiteral( "#000000" ) ).toString() );
      break;
    case Qgis::Warning:
      levelString = QStringLiteral( "WARNING" );
      color = QColor( settings.value( QStringLiteral( "colors/warning" ), QStringLiteral( "#000000" ) ).toString() );
      break;
    case Qgis::Critical:
      levelString = QStringLiteral( "CRITICAL" );
      color = QColor( settings.value( QStringLiteral( "colors/critical" ), QStringLiteral( "#000000" ) ).toString() );
      break;
    case Qgis::Success:
      levelString = QStringLiteral( "SUCCESS" );
      color = QColor( settings.value( QStringLiteral( "colors/success" ), QStringLiteral( "#000000" ) ).toString() );
      break;
    case Qgis::None:
      levelString = QStringLiteral( "NONE" );
      color = QColor( settings.value( QStringLiteral( "colors/default" ), QStringLiteral( "#000000" ) ).toString() );
      break;
  }

  QString prefix = QStringLiteral( "<font color=\"%1\">%2 &nbsp;&nbsp;&nbsp; %3 &nbsp;&nbsp;&nbsp;</font>" )
                   .arg( color.name(), QDateTime::currentDateTime().toString( Qt::ISODate ), levelString );
  QString cleanedMessage = message;
  cleanedMessage = cleanedMessage.prepend( prefix ).replace( '\n', QLatin1String( "<br>&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;" ) );
  w->appendHtml( cleanedMessage );
  w->verticalScrollBar()->setValue( w->verticalScrollBar()->maximum() );
}

void QgsMessageLogViewer::closeTab( int index )
{
  if ( tabWidget->count() == 1 )
    qobject_cast<QPlainTextEdit *>( tabWidget->widget( 0 ) )->clear();
  else
    tabWidget->removeTab( index );
}
