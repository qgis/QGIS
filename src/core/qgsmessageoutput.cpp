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

#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QRegularExpression>
#include <QString>

#include "moc_qgsmessageoutput.cpp"

using namespace Qt::StringLiterals;

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

void QgsMessageOutput::showMessage( const QString &title, const QString &message, Qgis::StringFormat format )
{
  QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
  output->setTitle( title );
  output->setMessage( message, format );
  output->showMessage();
}

////////////////////////////////
// QgsMessageOutputConsole

void QgsMessageOutputConsole::setMessage( const QString &message, Qgis::StringFormat format )
{
  mMessage = message;
  mFormat = format;
}

void QgsMessageOutputConsole::appendMessage( const QString &message )
{
  mMessage += message;
}

void QgsMessageOutputConsole::showMessage( bool )
{
  if ( mFormat == Qgis::StringFormat::Html )
  {
    mMessage.replace( "<br>"_L1, "\n"_L1 );
    mMessage.replace( "&nbsp;"_L1, " "_L1 );
    const thread_local QRegularExpression tagRX( u"</?[^>]+>"_s );
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

