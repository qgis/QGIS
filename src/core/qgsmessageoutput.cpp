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
/* $Id$ */

#include "qgsmessageoutput.h"
#include "qgslogger.h"

static QgsMessageOutput* messageOutputConsole_()
{
  return new QgsMessageOutputConsole;
}

// default output creator - console
MESSAGE_OUTPUT_CREATOR QgsMessageOutput::mMessageOutputCreator = messageOutputConsole_;


void QgsMessageOutput::setMessageOutputCreator( MESSAGE_OUTPUT_CREATOR f )
{
  mMessageOutputCreator = f;
}

QgsMessageOutput* QgsMessageOutput::createMessageOutput()
{
  return mMessageOutputCreator();
}

QgsMessageOutput::~QgsMessageOutput()
{
}

////////////////////////////////
// QgsMessageOutputConsole

QgsMessageOutputConsole::QgsMessageOutputConsole()
    : mMessage( "" )
{
}

void QgsMessageOutputConsole::setMessage( const QString& message, MessageType )
{
  mMessage = message;
}

void QgsMessageOutputConsole::appendMessage( const QString& message )
{
  mMessage += message;
}

void QgsMessageOutputConsole::showMessage( bool )
{
  // show title if provided
  if ( !mTitle.isNull() )
  {
    QgsDebugMsg( QString( "%1:" ).arg( mTitle ) );
  }

  // show the message
  QgsDebugMsg( mMessage );
  emit destroyed();
  delete this;
}

void QgsMessageOutputConsole::setTitle( const QString& title )
{
  mTitle = title;
}
