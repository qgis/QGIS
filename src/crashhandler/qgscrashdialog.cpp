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
  , mPythonFault( QgsCrashReport::PythonFault() )
{
  setupUi( this );
  setWindowTitle( tr( "Uh-oh!" ) );

  mCrashHeaderMessage->setText( tr( "QGIS ended unexpectedly" ) );
  connect( mReloadQGISButton, &QPushButton::clicked, this, &QgsCrashDialog::reloadQGIS );
  connect( mCopyReportButton, &QPushButton::clicked, this, &QgsCrashDialog::createBugReport );
  mCopyReportButton->setEnabled( false );

  mCrashMessage->setText( tr( "Oh dear! Something unexpected happened and QGIS ended without being able to handle the error gracefully."
                              "<br><br>" )
                          +  tr( "Are you keen to help us fix bugs? QGIS relies on donations to pay developers to do funded bug fixing to improve the stability of the software. "
                                 "We also have a team of enthusiastic volunteers who are all working hard to improve the quality of QGIS. To do that, we need your help. "
                                 "<a href=\"http://qgis.org/en/site/getinvolved/development/bugreporting.html#bugs-features-and-issues\">Find out how to help our developers</a>."
                                 "<br><br>"
                                 "Send us a helpful bug report by using the 'Copy Report' button below, <br>then open a ticket on the "
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
  mPythonFault = fault;
  switch ( fault.cause )
  {
    case QgsCrashReport::LikelyPythonFaultCause::Unknown:
    case QgsCrashReport::LikelyPythonFaultCause::NotPython:
      break;

    case QgsCrashReport::LikelyPythonFaultCause::ProcessingScript:
      mCrashHeaderMessage->setText( tr( "A user script crashed QGIS" ).arg( fault.title ) );
      mCrashMessage->setText( tr( "This user script <b>%1</b> caused QGIS to crash." ).arg( fault.filePath )
                              + "<br><br>"
                              +  tr( "This is a third party custom script, and this issue should be reported to the author of that script." ) );
      splitter->setSizes( { 0, splitter->width() } );
      mCopyReportButton->setEnabled( true );
      break;

    case QgsCrashReport::LikelyPythonFaultCause::Plugin:
      mCrashHeaderMessage->setText( tr( "Plugin %1 crashed QGIS" ).arg( fault.title ) );
      mCrashMessage->setText( tr( "The plugin <b>%1</b> caused QGIS to crash." ).arg( fault.title )
                              + "<br><br>"
                              +  tr( "Please report this issue to the author of this plugin." ) );
      splitter->setSizes( { 0, splitter->width() } );
      mCopyReportButton->setEnabled( true );
      break;

    case QgsCrashReport::LikelyPythonFaultCause::ConsoleCommand:
      mCrashHeaderMessage->setText( tr( "Command crashed QGIS" ).arg( fault.title ) );
      mCrashMessage->setText( tr( "A command entered in the Python console caused QGIS to crash." ) );
      splitter->setSizes( { 0, splitter->width() } );
      mCopyReportButton->setEnabled( true );
      break;
  }
}

void QgsCrashDialog::showReportWidget()
{
}

void QgsCrashDialog::on_mUserFeedbackText_textChanged()
{
  mCopyReportButton->setEnabled( !mUserFeedbackText->toPlainText().isEmpty()
                                 || ( mPythonFault.cause != QgsCrashReport::LikelyPythonFaultCause::NotPython
                                      && mPythonFault.cause != QgsCrashReport::LikelyPythonFaultCause::Unknown ) );
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

  const QString userText = !mUserFeedbackText->toPlainText().isEmpty()
                           ? ( "## User Feedback\n\n" + mUserFeedbackText->toPlainText() )
                           : QString();
  const QString details = "## Report Details\n\n" + mReportData;
  const QString finalText = ( !userText.isEmpty() ? ( userText + "\n\n" ) : QString() )
                            + details;
  const QString markdown = QgsCrashReport::htmlToMarkdown( finalText );
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
