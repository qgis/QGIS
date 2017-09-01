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

#include <QClipboard>


QgsCrashDialog::QgsCrashDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( tr( "Oh Uh!" ) );

  mCrashHeaderMessage->setText( tr( "Oh Uh!" ) );
  mCrashMessage->setText( tr( "Sorry. It looks something unexpected happened that we didn't handle and QGIS crashed." ) );
  connect( mCopyReportButton, &QPushButton::clicked, this, &QgsCrashDialog::createBugReport );

  mHelpLabel->setText( tr( "Keen to help us fix bugs? "
                           "<a href=\"http://qgis.org/en/site/getinvolved/development/bugreporting.html#bugs-features-and-issues\">Follow the steps to help our developers.</a>"
                           "<br><br>"
                           "You can also send us a helpful bug report using the Copy Report button <br>and opening a ticket at "
                           "<a href=\"https://issues.qgis.org/\">issues.qgis.org</a>" ) );
  mHelpLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
  mHelpLabel->setOpenExternalLinks( true );

}

void QgsCrashDialog::setBugReport( const QString &reportData )
{
  mReportData = reportData;
  mReportDetailsText->setHtml( reportData );
}

void QgsCrashDialog::showReportWidget()
{
}

void QgsCrashDialog::createBugReport()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString userText = "h2. User Feedback\n\n" + mUserFeedbackText->toPlainText();
  QString details = "h2. Report Details\n\n" + mReportData;
  QString finalText = userText + "\n\n" + details;
  QString markdown = htmlToMarkdown( finalText );
  clipboard->setText( markdown );
}

QString QgsCrashDialog::htmlToMarkdown( const QString &html )
{
  QString markdown = html;
  markdown.replace( "<br>", "\n" );
  markdown.replace( "<b>", "*" );
  markdown.replace( "</b>", "*" );
  markdown.replace( "QGIS code revision: ", "QGIS code revision: commit:" );
  return markdown;
}

