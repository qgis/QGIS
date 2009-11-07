/***************************************************************************
 *   Copyright (C) 2004 by Gary Sherman                                    *
 *   sherman at mrcc.com                                                   *
 *                                                                         *
 *   GUI for loading a delimited text file as a layer in QGIS              *
 *   This plugin works in conjuction with the delimited text data          *
 *   provider plugin                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
/* $Id$ */
#include "qgsdelimitedtextplugingui.h"
#include "qgscontexthelp.h"

#include "qgisinterface.h"

#include <QFileDialog>
#include <QFile>
#include <QComboBox>
#include <QSettings>
#include <QFileInfo>
#include <QRegExp>
#include <QMessageBox>
#include <QTextStream>
#include "qgslogger.h"

QgsDelimitedTextPluginGui::QgsDelimitedTextPluginGui( QgisInterface * _qI, QWidget * parent, Qt::WFlags fl )
    : QDialog( parent, fl ), qI( _qI )
{
  setupUi( this );
  pbnOK = buttonBox->button( QDialogButtonBox::Ok );
  pbnParse = buttonBox->addButton( tr( "Parse" ), QDialogButtonBox::ActionRole );
  connect( pbnParse, SIGNAL( clicked() ), this, SLOT( pbnParse_clicked() ) );
  connect( txtFilePath, SIGNAL( textChanged( const QString& ) ), this, SLOT( pbnParse_clicked() ) );
  enableButtons();
  // at startup, fetch the last used delimiter and directory from
  // settings
  QSettings settings;
  QString key = "/Plugin-DelimitedText";
  txtDelimiter->setText( settings.value( key + "/delimiter" ).toString() );

  // and how to use the delimiter
  QString delimiterType = settings.value( key + "/delimiterType",
                                          "plain" ).toString();
  if ( delimiterType == "plain" )
  {
    delimiterPlain->setChecked( true );
    delimiterRegexp->setChecked( false );
  }
  else
  {
    delimiterPlain->setChecked( false );
    delimiterRegexp->setChecked( true );
  }

  txtSample->setFixedHeight( 120 );
}
QgsDelimitedTextPluginGui::~QgsDelimitedTextPluginGui()
{
}
/** Autoconnected slots **/
void QgsDelimitedTextPluginGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}
void QgsDelimitedTextPluginGui::on_btnBrowseForFile_clicked()
{
  getOpenFileName();
}
void QgsDelimitedTextPluginGui::pbnParse_clicked()
{
  updateFieldLists();
}
void QgsDelimitedTextPluginGui::on_buttonBox_accepted()
{
  if ( txtLayerName->text().length() > 0 )
  {
    //Build the delimited text URI from the user provided information
    QString delimiterType = "plain";
    if ( delimiterRegexp->isChecked() )
      delimiterType = "regexp";

    QString uri = QString( "%1?delimiter=%2&delimiterType=%3&xField=%4&yField=%5" )
                  .arg( txtFilePath->text() )
                  .arg( txtDelimiter->text() )
                  .arg( delimiterType )
                  .arg( cmbXField->currentText() )
                  .arg( cmbYField->currentText() );

    // add the layer to the map
    emit drawVectorLayer( uri, txtLayerName->text(), "delimitedtext" );
    // store the settings

    QSettings settings;
    QString key = "/Plugin-DelimitedText";
    settings.setValue( key + "/delimiter", txtDelimiter->text() );
    QFileInfo fi( txtFilePath->text() );
    settings.setValue( key + "/text_path", fi.path() );

    if ( delimiterPlain->isChecked() )
      settings.setValue( key + "/delimiterType", "plain" );
    else
      settings.setValue( key + "/delimiterType", "regexp" );

    accept();
  }
  else
  {
    QMessageBox::warning( this, tr( "No layer name" ), tr( "Please enter a layer name before adding the layer to the map" ) );
  }
}

void QgsDelimitedTextPluginGui::on_buttonBox_rejected()
{
  reject();
}

void QgsDelimitedTextPluginGui::updateFieldLists()
{
  // Update the x and y field dropdown boxes
  QgsDebugMsg( "Updating field lists" );
  // open the file

  if ( QFile::exists( txtFilePath->text() ) )
  {
    QFile *file = new QFile( txtFilePath->text() );
    if ( file->open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      // clear the field lists
      cmbXField->clear();
      cmbYField->clear();
      QTextStream stream( file );
      QString line;
      line = readLine( stream ); // line of text excluding '\n'
      if ( txtDelimiter->text().length() > 0 )
      {
        QgsDebugMsg( QString( "Attempting to split the input line: %1 using delimiter %2" ).arg( line ).arg( txtDelimiter->text() ) );
        QString delimiter = txtDelimiter->text();

        QStringList fieldList;

        if ( delimiterPlain->isChecked() )
        {
          // convert \t to tabulator
          delimiter.replace( "\\t", "\t" );
          fieldList = line.split( delimiter );
        }
        else
        {
          QRegExp del( delimiter );
          fieldList = line.split( QRegExp( delimiter ) );
        }

        QgsDebugMsg( QString( "Split line into %1 parts" ).arg( fieldList.size() ) );
        //
        // We don't know anything about a text based field other
        // than its name. All fields are assumed to be text
        for ( QStringList::Iterator it = fieldList.begin(); it != fieldList.end(); ++it )
        {
          // add item to both drop-downs (X field and Y field)
          if (( *it ).length() > 0 )
          {
            cmbXField->addItem( *it );
            cmbYField->addItem( *it );
          }
        }
        // Have a go at setting the selected items in the X and Y
        // combo boxes to something sensible.
        int indexX = cmbXField->findText( "lon", Qt::MatchContains );
        int indexY = cmbXField->findText( "lat", Qt::MatchContains );
        if ( indexX != -1 && indexY != -1 )
        {
          cmbXField->setCurrentIndex( indexX );
          cmbYField->setCurrentIndex( indexY );
        }
        else
        {
          indexX = cmbXField->findText( "x", Qt::MatchContains );
          indexY = cmbXField->findText( "y", Qt::MatchContains );
          if ( indexX != -1 && indexY != -1 )
          {
            cmbXField->setCurrentIndex( indexX );
            cmbYField->setCurrentIndex( indexY );
          }
        }
        // enable the buttons
        enableButtons();
      }
      else
      {
        QMessageBox::warning( this, tr( "No delimiter" ), tr( "Please specify a delimiter prior to parsing the file" ) );
      }
      // clear the sample text box
      txtSample->clear();
      // put the header row in the sample box
      txtSample->insertPlainText( line + "\n" );
      // put a few more lines into the sample box
      int counter = 0;
      line = QgsDelimitedTextPluginGui::readLine( stream );
      while ( !line.isEmpty() && ( counter < 20 ) )
      {
        txtSample->insertPlainText( line + "\n" );
        counter++;
        line = QgsDelimitedTextPluginGui::readLine( stream );
      }
      // close the file
      file->close();
      // put a default layer name in the text entry
      QFileInfo finfo( txtFilePath->text() );
      txtLayerName->setText( finfo.completeBaseName() );
    }

  }

}
void QgsDelimitedTextPluginGui::getOpenFileName()
{
  // Get a file to process, starting at the current directory
  // Set inital dir to last used
  QSettings settings;

  QString s = QFileDialog::getOpenFileName(
                this,
                tr( "Choose a delimited text file to open" ),
                settings.value( "/Plugin-DelimitedText/text_path", "./" ).toString(),
                "Text files (*.txt *.csv);; All files (* *.*)" );

  // set path
  txtFilePath->setText( s );
  // update the field drop-down boxes by parsing and splitting
  // the header row
  updateFieldLists();
}
void QgsDelimitedTextPluginGui::enableButtons()
{
  pbnParse->setEnabled( txtDelimiter->text().length() > 0 && txtFilePath->text().length() > 0 );
  pbnOK->setEnabled( txtDelimiter->text().length() > 0 && txtFilePath->text().length() > 0 );
}
void QgsDelimitedTextPluginGui::help()
{
  qI->openURL( "plugins/delimited_text/index.html", true );
}

void QgsDelimitedTextPluginGui::on_txtDelimiter_textChanged( const QString & text )
{
  if ( !text.isEmpty() )
  {
    pbnParse->setEnabled( true );
  }
}

QString QgsDelimitedTextPluginGui::readLine( QTextStream & stream )
{
  QString buffer( "" );
  QString c;

  // Strip leading newlines

  c = stream.read( 1 );
  if ( c == NULL || c.size() == 0 )
  {
    // Reach end of file
    return buffer;
  }
  while ( c == ( char * )"\r" || c == ( char * )"\n" )
  {
    c = stream.read( 1 );
    if ( c == NULL || c.size() == 0 )
    {
      // Reach end of file
      return buffer;
    }
  }

  // First non-newline character
  buffer.append( c );

  c = stream.read( 1 );
  if ( c == NULL || c.size() == 0 )
  {
    // Reach end of file
    return buffer;
  }

  while ( !( c == ( char * )"\r" || c == ( char * )"\n" ) )
  {

    buffer.append( c );
    c = stream.read( 1 );
    if ( c == NULL || c.size() == 0 )
    {
      // Reach end of file
      return buffer;
    }
  }

  return buffer;

}
