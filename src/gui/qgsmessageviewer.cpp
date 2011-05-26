/***************************************************************************
                          qgsmessageviewer.cpp  -  description
                             -------------------
    begin                : Wed Jun 4 2003
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessageviewer.h"
#include <QSettings>

QgsMessageViewer::QgsMessageViewer( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  setAttribute( Qt::WA_DeleteOnClose );

  // Default state for the checkbox
  setCheckBoxVisible( false );
  setCheckBoxState( Qt::Unchecked );

  mCheckBoxQSettingsLabel = "";
}

QgsMessageViewer::~QgsMessageViewer()
{
}

void QgsMessageViewer::setMessageAsHtml( const QString& msg )
{
  txtMessage->setHtml( msg );
}

void QgsMessageViewer::setMessageAsPlainText( const QString& msg )
{
  txtMessage->setPlainText( msg );
}

void QgsMessageViewer::appendMessage( const QString& msg )
{
  txtMessage->append( msg );
}


void QgsMessageViewer::setMessage( const QString& message, MessageType msgType )
{
  if ( msgType == MessageHtml )
    setMessageAsHtml( message );
  else
    setMessageAsPlainText( message );
}

void QgsMessageViewer::showMessage( bool blocking )
{
  if ( blocking )
  {
    QApplication::setOverrideCursor( Qt::ArrowCursor );
    exec();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    show();
  }
}

void QgsMessageViewer::setTitle( const QString& title )
{
  setWindowTitle( title );
}

void QgsMessageViewer::setCheckBoxText( const QString& text )
{
  checkBox->setText( text );
}

void QgsMessageViewer::setCheckBoxVisible( bool visible )
{
  checkBox->setVisible( visible );
}

void QgsMessageViewer::setCheckBoxState( Qt::CheckState state )
{
  checkBox->setCheckState( state );
}

Qt::CheckState QgsMessageViewer::checkBoxState()
{
  return checkBox->checkState();
}

void QgsMessageViewer::setCheckBoxQSettingsLabel( QString label )
{
  mCheckBoxQSettingsLabel = label;
}


void QgsMessageViewer::on_checkBox_toggled( bool toggled )
{
  if ( !mCheckBoxQSettingsLabel.isEmpty() )
  {
    QSettings settings;
    if ( checkBox->checkState() == Qt::Checked )
      settings.setValue( mCheckBoxQSettingsLabel, false );
    else
      settings.setValue( mCheckBoxQSettingsLabel, true );
  }
}
