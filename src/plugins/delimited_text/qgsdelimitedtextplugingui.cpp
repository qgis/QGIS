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
#include <QUrl>
#include "qgslogger.h"

QgsDelimitedTextPluginGui::QgsDelimitedTextPluginGui( QgisInterface * _qI, QWidget * parent, Qt::WFlags fl )
    : QDialog( parent, fl ), qI( _qI )
{
  setupUi( this );
  pbnOK = buttonBox->button( QDialogButtonBox::Ok );

  updateFieldsAndEnable();

  // at startup, fetch the last used delimiter and directory from
  // settings
  QSettings settings;
  QString key = "/Plugin-DelimitedText";
  txtDelimiter->setText( settings.value( key + "/delimiter" ).toString() );

  // and how to use the delimiter
  QString delimiterType = settings.value( key + "/delimiterType", "plain" ).toString();
  if ( delimiterType == "selection" )
  {
    delimiterSelection->setChecked( true );
  }
  else if ( delimiterType == "plain" )
  {
    delimiterPlain->setChecked( true );
  }
  else
  {
    delimiterRegexp->setChecked( true );
  }

  QString delimiterChars = settings.value( key + "/delimiterChars", " " ).toString();
  cbxDelimSpace->setChecked( delimiterChars.contains( " " ) );
  cbxDelimTab->setChecked( delimiterChars.contains( "\\t" ) );
  cbxDelimColon->setChecked( delimiterChars.contains( ":" ) );
  cbxDelimSemicolon->setChecked( delimiterChars.contains( ":" ) );
  cbxDelimComma->setChecked( delimiterChars.contains( "," ) );

  cmbXField->setDisabled( true );
  cmbYField->setDisabled( true );
  cmbWktField->setDisabled( true );

  connect( txtFilePath, SIGNAL( textChanged( QString ) ), this, SLOT( updateFieldsAndEnable() ) );

  connect( delimiterSelection, SIGNAL( toggled( bool ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( delimiterPlain, SIGNAL( toggled( bool ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( delimiterRegexp, SIGNAL( toggled( bool ) ), this, SLOT( updateFieldsAndEnable() ) );

  connect( cbxDelimSpace, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimTab, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimSemicolon, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimComma, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimColon, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );

  connect( txtDelimiter, SIGNAL( editingFinished() ), this, SLOT( updateFieldsAndEnable() ) );

  connect( rowCounter, SIGNAL( valueChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
}

QgsDelimitedTextPluginGui::~QgsDelimitedTextPluginGui()
{
}
/** Autoconnected slots **/
void QgsDelimitedTextPluginGui::on_btnBrowseForFile_clicked()
{
  getOpenFileName();
}

void QgsDelimitedTextPluginGui::on_buttonBox_accepted()
{
  if ( !txtLayerName->text().isEmpty() )
  {
    //Build the delimited text URI from the user provided information
    QString delimiterType;
    if ( delimiterSelection->isChecked() )
      delimiterType = "regexp";
    else if ( delimiterPlain->isChecked() )
      delimiterType = "plain";
    else if ( delimiterRegexp->isChecked() )
      delimiterType = "regexp";

    QUrl url = QUrl::fromLocalFile( txtFilePath->text() );
    url.addQueryItem( "delimiter", txtDelimiter->text() );
    url.addQueryItem( "delimiterType", delimiterType );

    if ( geomTypeXY->isChecked() )
    {
      if ( !cmbXField->currentText().isEmpty() && !cmbYField->currentText().isEmpty() )
      {
        url.addQueryItem( "xField", cmbXField->currentText() );
        url.addQueryItem( "yField", cmbYField->currentText() );
      }
    }
    else
    {
      if ( ! cmbWktField->currentText().isEmpty() )
      {
        url.addQueryItem( "wktField", cmbWktField->currentText() );
      }
    }

    int skipLines = rowCounter->value();
    if ( skipLines > 0 )
      url.addQueryItem( "skipLines", QString( "%1" ).arg( skipLines ) );

    // add the layer to the map
    emit drawVectorLayer( QString::fromAscii( url.toEncoded() ), txtLayerName->text(), "delimitedtext" );

    // store the settings
    QSettings settings;
    QString key = "/Plugin-DelimitedText";
    settings.setValue( key + "/delimiter", txtDelimiter->text() );
    QFileInfo fi( txtFilePath->text() );
    settings.setValue( key + "/text_path", fi.path() );

    if ( delimiterSelection->isChecked() )
      settings.setValue( key + "/delimiterType", "selection" );
    else if ( delimiterPlain->isChecked() )
      settings.setValue( key + "/delimiterType", "plain" );
    else
      settings.setValue( key + "/delimiterType", "regexp" );
    settings.setValue( key + "/delimiterChars", selectedChars() );

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

QString QgsDelimitedTextPluginGui::selectedChars()
{
  QString chars = "";
  if ( cbxDelimSpace->isChecked() )
    chars += " ";
  if ( cbxDelimTab->isChecked() )
    chars += "\\t";
  if ( cbxDelimSemicolon->isChecked() )
    chars += ";";
  if ( cbxDelimComma->isChecked() )
    chars += ",";
  if ( cbxDelimColon->isChecked() )
    chars += ":";
  return chars;
}

QStringList QgsDelimitedTextPluginGui::splitLine( QString line )
{
  QStringList fieldList;
  QString delimiter = txtDelimiter->text();

  if ( delimiterPlain->isChecked() )
  {
    // convert \t to tabulator
    delimiter = txtDelimiter->text();
    delimiter.replace( "\\t", "\t" );
    fieldList = line.split( delimiter );
  }
  else if ( delimiterSelection->isChecked() )
  {
    delimiter = "[";
    delimiter += selectedChars();
    delimiter += "]";
    txtDelimiter->setText( delimiter );
    fieldList = line.split( QRegExp( delimiter ) );
  }
  else
  {
    QRegExp del( delimiter );
    fieldList = line.split( QRegExp( delimiter ) );
  }

  return fieldList;
}

bool QgsDelimitedTextPluginGui::haveValidFileAndDelimiters()
{

  bool valid = true;
  // If there is no valid file or field delimiters than cannot determine fields
  if ( txtFilePath->text().isEmpty() || !QFile( txtFilePath->text() ).exists() )
  {
    valid = false;
  }
  else if ( delimiterSelection->isChecked() )
  {
    valid =
      cbxDelimSpace->isChecked() ||
      cbxDelimTab->isChecked() ||
      cbxDelimSemicolon->isChecked() ||
      cbxDelimComma->isChecked() ||
      cbxDelimColon->isChecked();
  }
  else
  {
    valid = !txtDelimiter->text().isEmpty();
  }
  return valid;
}

void QgsDelimitedTextPluginGui::updateFieldLists()
{
  // Update the x and y field dropdown boxes
  QgsDebugMsg( "Updating field lists" );

  disconnect( cmbXField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
  disconnect( cmbYField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
  disconnect( cmbWktField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
  disconnect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbXField, SLOT( setEnabled( bool ) ) );
  disconnect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbYField, SLOT( setEnabled( bool ) ) );
  disconnect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbWktField, SLOT( setDisabled( bool ) ) );

  QString columnX = cmbXField->currentText();
  QString columnY = cmbYField->currentText();
  QString columnWkt = cmbWktField->currentText();

  // clear the field lists
  cmbXField->clear();
  cmbYField->clear();
  cmbWktField->clear();

  geomTypeXY->setEnabled( false );
  geomTypeWKT->setEnabled( false );
  cmbXField->setEnabled( false );
  cmbYField->setEnabled( false );
  cmbWktField->setEnabled( false );

  if ( ! haveValidFileAndDelimiters() )
    return;

  QFile file( txtFilePath->text() );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;

  int skipLines = rowCounter->value();

  QTextStream stream( &file );
  QString line;
  do
  {
    line = readLine( stream ); // line of text excluding '\n'
  }
  while ( !line.isEmpty() && skipLines-- > 0 );

  QgsDebugMsg( QString( "Attempting to split the input line: %1 using delimiter %2" ).arg( line ).arg( txtDelimiter->text() ) );

  QString delimiter = txtDelimiter->text();

  QStringList fieldList = splitLine( line );

  QgsDebugMsg( QString( "Split line into %1 parts" ).arg( fieldList.size() ) );

  //
  // We don't know anything about a text based field other
  // than its name. All fields are assumed to be text
  bool haveFields = false;

  foreach( QString field, fieldList )
  {
    if (( field.left( 1 ) == "'" || field.left( 1 ) == "\"" ) &&
        field.left( 1 ) == field.right( 1 ) )
      // eat quotes
      field = field.mid( 1, field.length() - 2 );

    if ( field.length() == 0 )
      // skip empty field names
      continue;

    cmbXField->addItem( field );
    cmbYField->addItem( field );
    cmbWktField->addItem( field );
    haveFields = true;
  }

  int indexWkt = -1;
  if ( ! columnWkt.isEmpty() )
  {
    indexWkt = cmbWktField->findText( columnWkt );
  }
  if ( indexWkt < 0 )
  {
    indexWkt = cmbWktField->findText( "wkt", Qt::MatchContains );
  }
  if ( indexWkt < 0 )
  {
    indexWkt = cmbWktField->findText( "geometry", Qt::MatchContains );
  }
  if ( indexWkt < 0 )
  {
    indexWkt = cmbWktField->findText( "shape", Qt::MatchContains );
  }
  cmbWktField->setCurrentIndex( indexWkt );

  int indexX = -1;
  if ( !columnX.isEmpty() )
  {
    indexX = cmbXField->findText( columnX );
  }

  if ( indexX < 0 )
  {
    indexX = cmbXField->findText( "lon", Qt::MatchContains );
  }

  if ( indexX < 0 )
  {
    indexX = cmbXField->findText( "x", Qt::MatchContains );
  }

  cmbXField->setCurrentIndex( indexX );

  int indexY = -1;
  if ( !columnY.isEmpty() )
  {
    indexY = cmbYField->findText( columnY );
  }

  if ( indexY < 0 )
  {
    indexY = cmbYField->findText( "lat", Qt::MatchContains );
  }

  if ( indexY < 0 )
  {
    indexY = cmbYField->findText( "y", Qt::MatchContains );
  }

  cmbYField->setCurrentIndex( indexY );


  bool isXY = ( geomTypeXY->isChecked() && indexX >= 0 && indexY >= 0 ) || indexWkt < 0;

  geomTypeXY->setChecked( isXY );
  geomTypeWKT->setChecked( ! isXY );

  if ( haveFields )
  {
    geomTypeXY->setEnabled( true );
    geomTypeWKT->setEnabled( true );
    cmbXField->setEnabled( isXY );
    cmbYField->setEnabled( isXY );
    cmbWktField->setEnabled( !  isXY );


    connect( cmbXField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
    connect( cmbYField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
    connect( cmbWktField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
    connect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbXField, SLOT( setEnabled( bool ) ) );
    connect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbYField, SLOT( setEnabled( bool ) ) );
    connect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbWktField, SLOT( setDisabled( bool ) ) );
  }

  // clear the sample text box
  tblSample->clear();

  tblSample->setColumnCount( fieldList.size() );
  tblSample->setHorizontalHeaderLabels( fieldList );

  // put a few more lines into the sample box
  int counter = 0;
  line = readLine( stream );
  while ( !line.isEmpty() && counter < 20 )
  {
    QStringList values = splitLine( line );

    tblSample->setRowCount( counter + 1 );

    for ( int i = 0; i < tblSample->columnCount(); i++ )
    {
      tblSample->setItem( counter, i, new QTableWidgetItem( i < values.size() ? values[i] : "" ) );
    }

    counter++;
    line = readLine( stream );
  }
  // close the file
  file.close();

  // put a default layer name in the text entry
  QFileInfo finfo( txtFilePath->text() );
  txtLayerName->setText( finfo.completeBaseName() );
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
                "Text files (*.txt *.csv);; Well Known Text files (*.wkt);; All files (* *.*)" );

  // set path
  txtFilePath->setText( s );
}

void QgsDelimitedTextPluginGui::updateFieldsAndEnable()
{
  updateFieldLists();
  enableAccept();
}

void QgsDelimitedTextPluginGui::enableAccept()
{

  // If the geometry type field is enabled then there must be
  // a valid file, and it must be
  bool enabled = haveValidFileAndDelimiters();

  if ( enabled )
  {

    if ( geomTypeXY->isChecked() )
    {
      enabled = !( cmbXField->currentText().isEmpty()  || cmbYField->currentText().isEmpty() || cmbXField->currentText() == cmbYField->currentText() );
    }
    else
    {
      enabled = !cmbWktField->currentText().isEmpty();
    }
  }

  pbnOK->setEnabled( enabled );
}

QString QgsDelimitedTextPluginGui::readLine( QTextStream &stream )
{
  QString buffer;

  while ( !stream.atEnd() )
  {
    QChar c = stream.read( 1 ).at( 0 );

    if ( c == '\r' || c == '\n' )
    {
      if ( buffer.isEmpty() )
      {
        // skip leading CR / LF
        continue;
      }

      break;
    }

    buffer.append( c );
  }

  return buffer;
}
