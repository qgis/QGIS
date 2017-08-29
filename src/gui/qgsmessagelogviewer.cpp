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
#include <QComboBox>


QgsMessageLogViewer::QgsMessageLogViewer( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );

  connect( QgsApplication::messageLog(), static_cast<void ( QgsMessageLog::* )( const QString &, const QString &, QgsMessageLog::MessageLevel )>( &QgsMessageLog::messageReceived ),
           this, static_cast<void ( QgsMessageLogViewer::* )( const QString &, const QString &, QgsMessageLog::MessageLevel )>( &QgsMessageLogViewer::logMessage ) );

  connect( tabWidget, &QTabWidget::tabCloseRequested, this, &QgsMessageLogViewer::closeTab );

  connect( tabWidget, &QTabWidget::currentChanged, this, &QgsMessageLogViewer::filter );

  connect( mLevelBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMessageLogViewer::filter );

  connect( mFilterEdit, &QLineEdit::textChanged, this, &QgsMessageLogViewer::filter );
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
  for ( i = 0; i < tabWidget->count() && tabWidget->tabText( i ) != cleanedTag; i++ )
    ;

  QTableWidget *w = nullptr;
  if ( i < tabWidget->count() )
  {
    w = qobject_cast<QTableWidget *>( tabWidget->widget( i ) );
    tabWidget->setCurrentIndex( i );
  }
  else
  {
    w = new QTableWidget( 0, 3, this );
    QStringList labels = ( QStringList() << "Time" << "Level" << "Message" );
    w->setHorizontalHeaderLabels( labels );
    w->horizontalHeader()->setStretchLastSection( true );
    w->verticalHeader()->hide();
    w->setSelectionBehavior( QAbstractItemView::SelectRows );

    tabWidget->addTab( w, cleanedTag );
    tabWidget->setCurrentIndex( tabWidget->count() - 1 );
    tabWidget->setTabsClosable( true );
  }

  QString levelString;
  switch ( level )
  {
    case QgsMessageLog::INFO:
      levelString = "INFO";
      break;
    case QgsMessageLog::WARNING:
      levelString = "WARNING";
      break;
    case QgsMessageLog::CRITICAL:
      levelString = "CRITICAL";
      break;
    case QgsMessageLog::NONE:
      levelString = "NONE";
      break;
  }
  QString date = QString( QDateTime::currentDateTime().toString( Qt::ISODate ) );
  int row = w->rowCount();
  w->insertRow( row );

  QTableWidgetItem *dateItem = new QTableWidgetItem( date );
  dateItem->setFlags( dateItem->flags() ^ Qt::ItemIsEditable );
  w->setItem( row, 0, dateItem );

  QTableWidgetItem *levelItem = new QTableWidgetItem( levelString );
  levelItem->setFlags( levelItem->flags() ^ Qt::ItemIsEditable );
  w->setItem( row, 1, levelItem );

  QTableWidgetItem *messageItem = new QTableWidgetItem( message );
  messageItem->setFlags( messageItem->flags() ^ Qt::ItemIsEditable );
  w->setItem( row, 2, messageItem );

  //Size the table appropriately
  w->resizeRowsToContents();
  w->resizeColumnToContents( 0 );
  w->resizeColumnToContents( 1 );
  w->verticalScrollBar()->setValue( w->verticalScrollBar()->maximum() );
}

void QgsMessageLogViewer::closeTab( int index )
{
  tabWidget->removeTab( index );
  tabWidget->setTabsClosable( tabWidget->count() > 1 );
}

void QgsMessageLogViewer::filter()
{
  QTableWidget *w = nullptr;
  w = qobject_cast<QTableWidget *>( tabWidget->currentWidget() );

  QString messageFilter = mFilterEdit->text();
  QString levelFilter = mLevelBox->currentText();

  //iterate rows and hide unmatching
  bool hide;
  QString itemLevel, itemMessage;
  bool levelMatch, messageMatch;


  for ( int i = 0; i < w->rowCount(); i++ )
  {
    hide = true;
    w->setRowHidden( i, hide );
    itemLevel = w->item( i, 1 )->text();
    itemMessage = w->item( i, 2 )->text();

    levelMatch = -1 != itemLevel.indexOf( levelFilter, 0,  Qt::CaseInsensitive );
    messageMatch = -1 != itemMessage.indexOf( messageFilter, 0,  Qt::CaseInsensitive );

    if ( levelFilter == "ALL" or levelMatch )
    {
      if ( messageFilter.isEmpty() or messageMatch )
      {
        hide = false;
      }
    }

    w->setRowHidden( i, hide );
  }
}
