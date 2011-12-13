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

#include <QSettings>
#include <QTableWidget>
#include <QDateTime>

static QgsMessageLogViewer *gmInstance = 0;

QgsMessageLogViewer::QgsMessageLogViewer( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  gmInstance = this;
  QgsMessageLog::setLogger( logger );

  connect( tabWidget, SIGNAL( tabCloseRequested( int ) ), this, SLOT( closeTab( int ) ) );
}

QgsMessageLogViewer::~QgsMessageLogViewer()
{
  QgsMessageLog::setLogger( 0 );
}

void QgsMessageLogViewer::logger( QString message, QString tag, int level )
{
  if ( !gmInstance )
    return;

  gmInstance->logMessage( message, tag, level );
}

void QgsMessageLogViewer::logMessage( QString message, QString tag, int level )
{
  if ( tag.isNull() )
    tag = tr( "General" );

  int i;
  for ( i = 0; i < tabWidget->count() && tabWidget->tabText( i ) != tag; i++ )
    ;

  QTableWidget *w;
  if ( i < tabWidget->count() )
  {
    w = qobject_cast<QTableWidget *>( tabWidget->widget( i ) );
  }
  else
  {
    w = new QTableWidget( 0, 3, this );
    w->verticalHeader()->setDefaultSectionSize( 16 );
    w->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );
    w->verticalHeader()->setVisible( false );
    w->setGridStyle( Qt::DotLine );
    w->setEditTriggers( QAbstractItemView::NoEditTriggers );
    w->setHorizontalHeaderLabels( QStringList() << tr( "Timestamp" ) << tr( "Message" ) << tr( "Level" ) );
    w->horizontalHeader()->setResizeMode( QHeaderView::ResizeToContents );
    tabWidget->addTab( w, tag );
  }

  int n = w->rowCount();

  w->setRowCount( n + 1 );
  QTableWidgetItem *item = new QTableWidgetItem( QDateTime::currentDateTime().toString( Qt::ISODate ) );
  w->setItem( n, 0, item );
  w->setItem( n, 1, new QTableWidgetItem( message ) );
  w->setItem( n, 2, new QTableWidgetItem( QString::number( level ) ) );
  w->scrollToBottom();
}

void QgsMessageLogViewer::closeTab( int index )
{
  tabWidget->removeTab( index );
}
