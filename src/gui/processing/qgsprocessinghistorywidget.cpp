/***************************************************************************
                             qgsprocessinghistorywidget.cpp
                             ------------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessinghistorywidget.h"
#include "moc_qgsprocessinghistorywidget.cpp"
#include "qgshistorywidget.h"
#include "qgsgui.h"
#include "qgshistoryproviderregistry.h"
#include "qgshelp.h"
#include "qgsfileutils.h"
#include "qgshistoryentry.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDialogButtonBox>
#include <QPushButton>

QgsProcessingHistoryWidget::QgsProcessingHistoryWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  mHistoryWidget = new QgsHistoryWidget( QStringLiteral( "processing" ) );
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->addWidget( mHistoryWidget );
  setLayout( vl );
}

void QgsProcessingHistoryWidget::clearHistory()
{
  if ( QMessageBox::question( this, tr( "Clear History" ), tr( "Are you sure you want to clear the Processing history?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
  {
    QgsGui::historyProviderRegistry()->clearHistory( Qgis::HistoryProviderBackend::LocalProfile, QStringLiteral( "processing" ) );
  }
}

void QgsProcessingHistoryWidget::openHelp()
{
  QgsHelp::openHelp( QStringLiteral( "processing/history.html" ) );
}

void QgsProcessingHistoryWidget::saveLog()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save File" ), QDir::homePath(), tr( "Log files (*.log *.LOG)" ) );
  // return dialog focus on Mac
  activateWindow();
  raise();

  if ( fileName.isEmpty() )
    return;

  fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, { QStringLiteral( "log" ) } );

  const QList<QgsHistoryEntry> entries = QgsGui::historyProviderRegistry()->queryEntries( QDateTime(), QDateTime(), QStringLiteral( "processing" ) );

  const QString logSeparator = QStringLiteral( "|~|" );
  QFile logFile( fileName );
  if ( logFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QTextStream logOut( &logFile );
    for ( const QgsHistoryEntry &entry : entries )
    {
      logOut << QStringLiteral( "ALGORITHM%1%2%3%4\n" ).arg( logSeparator, entry.timestamp.toString( "yyyy-MM-dd HH:mm:ss" ), logSeparator, entry.entry.value( QStringLiteral( "python_command" ) ).toString() );
    }
  }
}


//
// QgsProcessingHistoryDialog
//

QgsProcessingHistoryDialog::QgsProcessingHistoryDialog( QWidget *parent )
  : QDialog( parent )
{
  setObjectName( QStringLiteral( "QgsProcessingHistoryDialog" ) );
  QgsGui::instance()->enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Processing History" ) );

  QVBoxLayout *vl = new QVBoxLayout();
  mWidget = new QgsProcessingHistoryWidget();
  vl->addWidget( mWidget, 1 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Close | QDialogButtonBox::Help );

  QPushButton *clearButton = new QPushButton( tr( "Clear" ) );
  clearButton->setToolTip( tr( "Clear history" ) );
  mButtonBox->addButton( clearButton, QDialogButtonBox::ActionRole );

  QPushButton *saveButton = new QPushButton( tr( "Save As…" ) );
  saveButton->setToolTip( tr( "Save history" ) );
  mButtonBox->addButton( saveButton, QDialogButtonBox::ActionRole );

  connect( clearButton, &QPushButton::clicked, mWidget, &QgsProcessingHistoryWidget::clearHistory );
  connect( saveButton, &QPushButton::clicked, mWidget, &QgsProcessingHistoryWidget::saveLog );
  connect( mButtonBox->button( QDialogButtonBox::Help ), &QPushButton::clicked, mWidget, &QgsProcessingHistoryWidget::openHelp );
  connect( mButtonBox->button( QDialogButtonBox::Close ), &QPushButton::clicked, mWidget, [=]() { close(); } );

  vl->addWidget( mButtonBox );

  setLayout( vl );
}
