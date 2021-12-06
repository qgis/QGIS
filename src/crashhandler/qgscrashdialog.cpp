/***************************************************************************
                qgscrashdialog.cpp - QgsCrashDialog

 ---------------------
 begin                : 11.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgscrashdialog.h"

#include "qgscrashreport.h"

#include <QClipboard>
#include <QProcess>

QgsCrashDialog::QgsCrashDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( tr( "Uh-oh!" ) );

  mCrashHeaderMessage->setText( tr( "QGIS unexpectedly ended" ) );
  connect( mReloadQGISButton, &QPushButton::clicked, this, &QgsCrashDialog::reloadQGIS );
  connect( mCopyReportButton, &QPushButton::clicked, this, &QgsCrashDialog::createBugReport );
  mCopyReportButton->setEnabled( false );

  mCrashMessage->setText( tr( "Sorry :( It looks something unexpected happened that we didn't handle and QGIS ended unexpectedly."
                              "<br><br>" )
                          +  tr( "Keen to help us fix bugs? "
                                 "<a href=\"http://qgis.org/en/site/getinvolved/development/bugreporting.html#bugs-features-and-issues\">Follow the steps to help our developers</a>."
                                 "<br><br>"
                                 "You can also send us a helpful bug report using the Copy Report button <br>and opening a ticket at "
                                 "<a href=\"https://github.com/qgis/QGIS/issues\">QGIS Issue Tracker</a>." ) );
  mCrashMessage->setTextInteractionFlags( Qt::TextBrowserInteraction );
  mCrashMessage->setOpenExternalLinks( true );

}

void QgsCrashDialog::setBugReport( const QString &reportData )
{
  mReportData = reportData;
  mReportDetailsText->setHtml( reportData );
}

void QgsCrashDialog::setReloadArgs( const QString &reloadArgs )
{
  mReloadArgs = reloadArgs;
}

void QgsCrashDialog::setPythonFault( const QgsCrashReport::PythonFault &fault )
{
  switch ( fault.cause )
  {
    case QgsCrashReport::LikelyPythonFaultCause::Unknown:
      break;

    case QgsCrashReport::LikelyPythonFaultCause::ProcessingScript:
      mCrashHeaderMessage->setText( tr( "User script crashed QGIS" ).arg( fault.title ) );
      mCrashMessage->setText( tr( "The user script <b>%1</b> caused QGIS to crash." ).arg( fault.filePath )
                              + "<br><br>"
                              +  tr( "This is a third party custom script, and this issue should be reported to the author of that script." ) );
      splitter->setSizes( { 0, splitter->width() } );
      break;

    case QgsCrashReport::LikelyPythonFaultCause::Plugin:
      mCrashHeaderMessage->setText( tr( "Plugin %1 crashed QGIS" ).arg( fault.title ) );
      mCrashMessage->setText( tr( "The plugin <b>%1</b> caused QGIS to crash." ).arg( fault.title )
                              + "<br><br>"
                              +  tr( "Please report this issue to the author of that plugin." ) );
      splitter->setSizes( { 0, splitter->width() } );
      break;

    case QgsCrashReport::LikelyPythonFaultCause::ConsoleCommand:
      mCrashHeaderMessage->setText( tr( "Command crashed QGIS" ).arg( fault.title ) );
      mCrashMessage->setText( tr( "A command entered in the Python console caused QGIS to crash." ) );
      splitter->setSizes( { 0, splitter->width() } );
      break;
  }
}

void QgsCrashDialog::showReportWidget()
{
}

void QgsCrashDialog::on_mUserFeedbackText_textChanged()
{
  mCopyReportButton->setEnabled( !mUserFeedbackText->toPlainText().isEmpty() );
}

QStringList QgsCrashDialog::splitCommand( const QString &command )
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  return QProcess::splitCommand( command );
#else
  // taken from Qt 5.15's implementation
  QStringList args;
  QString tmp;
  int quoteCount = 0;
  bool inQuote = false;

  // handle quoting. tokens can be surrounded by double quotes
  // "hello world". three consecutive double quotes represent
  // the quote character itself.
  for ( int i = 0; i < command.size(); ++i )
  {
    if ( command.at( i ) == QLatin1Char( '"' ) )
    {
      ++quoteCount;
      if ( quoteCount == 3 )
      {
        // third consecutive quote
        quoteCount = 0;
        tmp += command.at( i );
      }
      continue;
    }
    if ( quoteCount )
    {
      if ( quoteCount == 1 )
        inQuote = !inQuote;
      quoteCount = 0;
    }
    if ( !inQuote && command.at( i ).isSpace() )
    {
      if ( !tmp.isEmpty() )
      {
        args += tmp;
        tmp.clear();
      }
    }
    else
    {
      tmp += command.at( i );
    }
  }
  if ( !tmp.isEmpty() )
    args += tmp;

  return args;
#endif
}

void QgsCrashDialog::createBugReport()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString userText = "## User Feedback\n\n" + mUserFeedbackText->toPlainText();
  QString details = "## Report Details\n\n" + mReportData;
  QString finalText = userText + "\n\n" + details;
  QString markdown = QgsCrashReport::htmlToMarkdown( finalText );
  clipboard->setText( markdown );
}

void QgsCrashDialog::reloadQGIS()
{
  const QStringList command = splitCommand( mReloadArgs );
  if ( command.isEmpty() )
    return;

  if ( QProcess::startDetached( command.at( 0 ), command.mid( 1 ) ) )
  {
    accept();
  }
}
