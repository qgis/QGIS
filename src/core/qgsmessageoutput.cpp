/***************************************************************************
    qgsmessageoutput.h  -  interface for showing messages
    ----------------------
    begin                : April 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessageoutput.h"
#include "moc_qgsmessageoutput.cpp"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QRegularExpression>

static QgsMessageOutput *messageOutputConsole_()
{
  return new QgsMessageOutputConsole;
}

// default output creator - console
MESSAGE_OUTPUT_CREATOR QgsMessageOutput::mMessageOutputCreator = messageOutputConsole_;


void QgsMessageOutput::setMessageOutputCreator( MESSAGE_OUTPUT_CREATOR f )
{
  mMessageOutputCreator = f;
}

QgsMessageOutput *QgsMessageOutput::createMessageOutput()
{
  return mMessageOutputCreator();
}

void QgsMessageOutput::showMessage( const QString &title, const QString &message, MessageType msgType )
{
  QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
  output->setTitle( title );
  output->setMessage( message, msgType );
  output->showMessage();
}

////////////////////////////////
// QgsMessageOutputConsole

void QgsMessageOutputConsole::setMessage( const QString &message, MessageType msgType )
{
  mMessage = message;
  mMsgType = msgType;
}

void QgsMessageOutputConsole::appendMessage( const QString &message )
{
  mMessage += message;
}

void QgsMessageOutputConsole::showMessage( bool )
{
  if ( mMsgType == MessageHtml )
  {
    mMessage.replace( QLatin1String( "<br>" ), QLatin1String( "\n" ) );
    mMessage.replace( QLatin1String( "&nbsp;" ), QLatin1String( " " ) );
    const thread_local QRegularExpression tagRX( QStringLiteral( "</?[^>]+>" ) );
    mMessage.replace( tagRX, QString() );
  }
  QgsMessageLog::logMessage( mMessage, mTitle.isNull() ? QObject::tr( "Console" ) : mTitle );
  emit destroyed();
  delete this;
}

void QgsMessageOutputConsole::setTitle( const QString &title )
{
  mTitle = title;
}

