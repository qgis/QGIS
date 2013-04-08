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
#include "qgsdelimitedtextsourceselect.h"

#include "qgisinterface.h"
#include "qgscontexthelp.h"
#include "qgslogger.h"

#include "qgsdelimitedtextprovider.h"
#include "qgsdelimitedtextfile.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegExp>
#include <QSettings>
#include <QTextStream>
#include <QUrl>

QgsDelimitedTextSourceSelect::QgsDelimitedTextSourceSelect( QWidget * parent, Qt::WFlags fl, bool embedded ):
    QDialog( parent, fl ),
    mFile( new QgsDelimitedTextFile() ),
    mExampleRowCount(20),
    mColumnNamePrefix("Column_"),
    mPluginKey("/Plugin-DelimitedText")
{

  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( mPluginKey+"/geometry" ).toByteArray() );

  if ( embedded )
  {
    buttonBox->button( QDialogButtonBox::Cancel )->hide();
    buttonBox->button( QDialogButtonBox::Ok )->hide();
  }

  updateFieldsAndEnable();

  // at startup, fetch the last used delimiter and directory from
  // settings
  QString key = mPluginKey;

  // and how to use the delimiter
  QString delimiterType = settings.value( key + "/delimiterType", "chars" ).toString();
  if ( delimiterType == "whitespace" )
  {
    delimiterWhitespace->setChecked( true );
  }
  else if ( delimiterType == "chars" )
  {
    delimiterChars->setChecked( true );
  }
  else if ( delimiterType == "regexp" )
  {
    delimiterRegexp->setChecked( true );
  }
  else
  {
    delimiterCSV->setChecked( true );
  }

  QString delimiters = settings.value( key + "/delimiters", " " ).toString();
  cbxDelimComma->setChecked( delimiters.contains( "," ) );
  cbxDelimSpace->setChecked( delimiters.contains( " " ) );
  cbxDelimTab->setChecked( delimiters.contains( "\\t" ) );
  cbxDelimColon->setChecked( delimiters.contains( ":" ) );
  cbxDelimSemicolon->setChecked( delimiters.contains( ";" ) );
  txtDelimiterOther->setText(delimiters.remove(QRegExp("([ ,:;]|\\t")));
  txtQuoteChars->setText(settings.value(key+"/quoteChars","\"").toString());
  txtEscapeChars->setText(settings.value(key+"/escapeChars","\"").toString());

  txtDelimiterRegexp->setText( settings.value( key + "/delimiterRegexp" ).toString() );

  rowCounter->setValue( settings.value( key + "/startFrom", 0 ).toInt() );

  cbxUseHeader->setChecked( settings.value(key + "/useHeader","true")=="false" );
  decimalPoint->setText( settings.value(key + "/decimalPoint",".").toString() );

  cmbXField->setDisabled( true );
  cmbYField->setDisabled( true );
  cmbWktField->setDisabled( true );

  connect( txtFilePath, SIGNAL( textChanged( QString ) ), this, SLOT( updateFileName() ) );
  connect( decimalPoint, SIGNAL( textChanged( QString ) ), this, SLOT( updateFieldsAndEnable() ) );

  connect( delimiterCSV, SIGNAL( toggled( bool ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( delimiterWhitespace, SIGNAL( toggled( bool ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( delimiterChars, SIGNAL( toggled( bool ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( delimiterRegexp, SIGNAL( toggled( bool ) ), this, SLOT( updateFieldsAndEnable() ) );

  connect( cbxDelimComma, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimSpace, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimTab, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimSemicolon, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxDelimColon, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );

  connect( txtDelimiterOther, SIGNAL( editingFinished() ), this, SLOT( updateFieldsAndEnable() ) );
  connect( txtQuoteChars, SIGNAL( editingFinished() ), this, SLOT( updateFieldsAndEnable() ) );
  connect( txtEscapeChars, SIGNAL( editingFinished() ), this, SLOT( updateFieldsAndEnable() ) );

  connect( rowCounter, SIGNAL( valueChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
  connect( cbxUseHeader, SIGNAL( stateChanged( int ) ), this, SLOT( updateFieldsAndEnable() ) );
}

QgsDelimitedTextSourceSelect::~QgsDelimitedTextSourceSelect()
{
  QSettings settings;
  settings.setValue( mPluginKey+"/geometry", saveGeometry() );
  delete mFile;
}

void QgsDelimitedTextSourceSelect::on_btnBrowseForFile_clicked()
{
  getOpenFileName();
}

void QgsDelimitedTextSourceSelect::on_buttonBox_accepted()
{
  if ( txtLayerName->text().isEmpty() )
  {
    QMessageBox::warning( this, tr( "No layer name" ), tr( "Please enter a layer name before adding the layer to the map" ) );
    return;
  }
  if( delimiterChars->isChecked() )
  {
      if( selectedChars().size() == 0 )
      {
            QMessageBox::warning( this, tr( "No delimiters set" ), tr( "Please one or more characters to use as the delimiter, or choose a different delimiter type" ) );
            return;
      }
  }
  if( delimiterRegexp->isChecked() )
  {
      QRegExp re(txtDelimiterRegexp->text());
      if( ! re.isValid() )
      {
            QMessageBox::warning( this, tr( "Invalid regular expression" ), tr( "Please enter a valid regular expression as the delimiter, or choose a different delimiter type" ) );
            return;
      }
  }
  if( ! mFile->isValid() )
  {
       QMessageBox::warning( this, tr( "Invalid delimited text file" ), tr( "Please enter a valid file and delimiter" ) );
       return;
  }

    //Build the delimited text URI from the user provided information

    QUrl url = mFile->url();

    if ( !decimalPoint->text().isEmpty() )
    {
      url.addQueryItem( "decimalPoint", decimalPoint->text() );
    }

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

    // add the layer to the map
    emit addVectorLayer( QString::fromAscii( url.toEncoded() ), txtLayerName->text(), "delimitedtext" );

    // store the settings
    QSettings settings;
    QString key = mPluginKey;
    settings.setValue( key + "/geometry", saveGeometry() );
    QFileInfo fi( txtFilePath->text() );
    settings.setValue( key + "/text_path", fi.path() );

    if ( delimiterCSV->isChecked() )
      settings.setValue( key + "/delimiterType", "csv" );
    else if ( delimiterWhitespace->isChecked() )
      settings.setValue( key + "/delimiterType", "whitespace" );
    else if ( delimiterChars->isChecked() )
      settings.setValue( key + "/delimiterChars", "chars" );
    else
      settings.setValue( key + "/delimiterType", "regexp" );
    settings.setValue( key + "/delimiterChars", selectedChars() );
    settings.setValue( key + "/quoteChars", txtQuoteChars->text() );
    settings.setValue( key + "/escapeChars", txtEscapeChars->text() );
    settings.setValue( key + "/delimiterRegexp", txtDelimiterRegexp->text() );
    settings.setValue( key + "/decimalPoint", decimalPoint->text() );
    settings.setValue( key + "/startFrom", rowCounter->value() );
    settings.setValue( key + "/useHeader", cbxUseHeader->isChecked() ? "true" : "false" );

    accept();
}

void QgsDelimitedTextSourceSelect::on_buttonBox_rejected()
{
  reject();
}

QString QgsDelimitedTextSourceSelect::selectedChars()
{
  QString chars = txtDelimiterOther->text();
  if ( cbxDelimComma->isChecked() )
    chars.append(",");
  if ( cbxDelimSpace->isChecked() )
    chars.append(" ");
  if ( cbxDelimTab->isChecked() )
    chars.append("\\t");
  if ( cbxDelimSemicolon->isChecked() )
    chars.append(";");
  if ( cbxDelimColon->isChecked() )
    chars.append(":");
  return chars;
}

bool QgsDelimitedTextSourceSelect::loadDelimitedFileDefinition()
{
    mFile->setFileName(txtFilePath->text());
    if( delimiterWhitespace->isChecked())
    {
        mFile->setTypeWhitespace();
    }
    else if( delimiterChars->isChecked())
    {
        mFile->setTypeCSV(selectedChars(),txtQuoteChars->text(),txtEscapeChars->text());
    }
    else if( delimiterRegexp->isChecked())
    {
        mFile->setTypeRegexp( delimiterRegexp->text());
    }
    else
    {
        mFile->setTypeCSV();
    }
    mFile->setSkipLines( rowCounter->value() );
    mFile->setUseHeader( cbxUseHeader->isChecked());
    return mFile->isValid();
}


void QgsDelimitedTextSourceSelect::updateFieldLists()
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

  // clear the sample text box
  tblSample->clear();

  if ( ! loadDelimitedFileDefinition() )
    return;

  bool useHeader = mFile->useHeader();
  QStringList fieldList;

  int indexWkt = -1;
  int indexX = -1;
  int indexY = -1;

  if( useHeader )
  {
      fieldList = mFile->columnNames();

      if( ! columnWkt.isEmpty()) indexWkt=fieldList.indexOf(columnWkt);
      if( indexWkt < 0 ) indexWkt=fieldList.indexOf(QRegExp("wkt.*",Qt::CaseInsensitive));
      if( indexWkt < 0 ) indexWkt=fieldList.indexOf(QRegExp("geom.*",Qt::CaseInsensitive));
      if( indexWkt < 0 ) indexWkt=fieldList.indexOf(QRegExp("shape.*",Qt::CaseInsensitive));
      if( indexWkt < 0 ) indexWkt=fieldList.indexOf(QRegExp(".*wkt.*",Qt::CaseInsensitive));
      if( indexWkt < 0 ) indexWkt=fieldList.indexOf(QRegExp(".*geom.*",Qt::CaseInsensitive));
      if( indexWkt < 0 ) indexWkt=fieldList.indexOf(QRegExp(".*shape.*",Qt::CaseInsensitive));

      if( ! columnX.isEmpty()) indexX=fieldList.indexOf(columnX);
      if( indexX < 0 ) indexX=fieldList.indexOf(QRegExp("lon.*",Qt::CaseInsensitive));
      if( indexX < 0 ) indexX=fieldList.indexOf(QRegExp("east.*",Qt::CaseInsensitive));
      if( indexX < 0 ) indexX=fieldList.indexOf(QRegExp("x.*",Qt::CaseInsensitive));
      if( indexX < 0 ) indexX=fieldList.indexOf(QRegExp(".*lon.*",Qt::CaseInsensitive));
      if( indexX < 0 ) indexX=fieldList.indexOf(QRegExp(".*east.*",Qt::CaseInsensitive));
      if( indexX < 0 ) indexX=fieldList.indexOf(QRegExp(".*x.*",Qt::CaseInsensitive));

      if( ! columnY.isEmpty()) indexY=fieldList.indexOf(columnY);
      if( indexY < 0 ) indexY=fieldList.indexOf(QRegExp("lat.*",Qt::CaseInsensitive));
      if( indexY < 0 ) indexY=fieldList.indexOf(QRegExp("north.*",Qt::CaseInsensitive));
      if( indexY < 0 ) indexY=fieldList.indexOf(QRegExp("y.*",Qt::CaseInsensitive));
      if( indexY < 0 ) indexY=fieldList.indexOf(QRegExp(".*lat.*",Qt::CaseInsensitive));
      if( indexY < 0 ) indexY=fieldList.indexOf(QRegExp(".*north.*",Qt::CaseInsensitive));
      if( indexY < 0 ) indexY=fieldList.indexOf(QRegExp(".*y.*",Qt::CaseInsensitive));

      tblSample->setColumnCount( fieldList.size() );
  }

  // put a lines into the sample box

  int counter = 0;
  QStringList values;
  while ( counter < mExampleRowCount )
  {
      QgsDelimitedTextFile::Status status = mFile->nextRecord( values );
      if( status == QgsDelimitedTextFile::RecordEOF ) break;
      if( status != QgsDelimitedTextFile::RecordOk ) continue;
      counter++;

      // If don't have headers, then check column count and expand if necessary
      // Don't count blank columns

      int nv = values.size();
      while( nv > 0 && values[nv-1].isEmpty()) nv--;

      if( nv > fieldList.size())
      {
          while( fieldList.size() < nv )
          {
              int nc = fieldList.size();
              QString column = mColumnNamePrefix+QString::number(nc+1);
              if( column == columnWkt ) indexWkt=nc;
              if( column == columnX ) indexX=nc;
              if( column == columnY ) indexY=nc;
              fieldList.append(column);
          }
          tblSample->setColumnCount(fieldList.size());
      }

    tblSample->setRowCount( counter );

    for ( int i = 0; i < tblSample->columnCount(); i++ )
    {
      QString value = i < values.size() ? values[i] : "";
      bool ok = true;
      if ( i == indexX || i == indexY )
      {
        if ( !decimalPoint->text().isEmpty() )
        {
          value.replace( decimalPoint->text(), "." );
        }

        value.toDouble( &ok );
      }
      QTableWidgetItem *item = new QTableWidgetItem( value );
      if ( !ok )
        item->setTextColor( Qt::red );
      tblSample->setItem( counter, i, item );
    }
  }

  tblSample->setHorizontalHeaderLabels( fieldList );

  // We don't know anything about a text based field other
  // than its name. All fields are assumed to be text
  // As we ignore blank fields we need to map original index
  // of selected fields to index in combo box.

  int fieldNo = 0; 
  int cmbIndexWkt=-1;
  int cmbIndexX=-1;
  int cmbIndexY=-1;

  for( int i = 0; i < fieldList.size(); i++ )
  {
    QString field = fieldList[i];
    // skip empty field names
    if ( field.isEmpty() ) continue;
    cmbXField->addItem( field );
    cmbYField->addItem( field );
    cmbWktField->addItem( field );
    if( i == indexWkt ) cmbIndexWkt = fieldNo;
    if( i == indexX ) cmbIndexX = fieldNo;
    if( i == indexY ) cmbIndexY = fieldNo;
    fieldNo++;
  }

  bool haveFields = fieldNo > 0;

  cmbWktField->setCurrentIndex( cmbIndexWkt );
  cmbXField->setCurrentIndex( cmbIndexX );
  cmbYField->setCurrentIndex( cmbIndexY );

  bool isXY = ( geomTypeXY->isChecked() && indexX >= 0 && indexY >= 0 ) || indexWkt < 0;
  geomTypeXY->setChecked( isXY );
  geomTypeWKT->setChecked( ! isXY );

  if ( haveFields )
  {
    geomTypeXY->setEnabled( true );
    geomTypeWKT->setEnabled( true );
    cmbXField->setEnabled( isXY );
    cmbYField->setEnabled( isXY );
    cmbWktField->setEnabled( ! isXY );

    connect( cmbXField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
    connect( cmbYField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
    connect( cmbWktField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableAccept() ) );
    connect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbXField, SLOT( setEnabled( bool ) ) );
    connect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbYField, SLOT( setEnabled( bool ) ) );
    connect( geomTypeXY, SIGNAL( toggled( bool ) ), cmbWktField, SLOT( setDisabled( bool ) ) );
  }

}

void QgsDelimitedTextSourceSelect::getOpenFileName()
{
  // Get a file to process, starting at the current directory
  // Set inital dir to last used
  QSettings settings;

  QString s = QFileDialog::getOpenFileName(
                this,
                tr( "Choose a delimited text file to open" ),
                settings.value( mPluginKey+"/text_path", "./" ).toString(),
                tr( "Text files" ) + " (*.txt *.csv);;"
                + tr( "Well Known Text files" ) + " (*.wkt);;"
                + tr( "All files" ) + " (* *.*)" );
  // set path
  txtFilePath->setText( s );
}

void QgsDelimitedTextSourceSelect::updateFileName()
{
  // put a default layer name in the text entry
  QFileInfo finfo( txtFilePath->text() );
  txtLayerName->setText( finfo.completeBaseName() );
  updateFieldsAndEnable();
}

void QgsDelimitedTextSourceSelect::updateFieldsAndEnable()
{
  // Check the regular expression is valid
  lblRegexpError->setText("");
  if( delimiterRegexp->isChecked())
  {
      QRegExp re(txtDelimiterRegexp->text());
      if( ! re.isValid()) lblRegexpError->setText(tr("Expression is not valid"));
  }
    
  updateFieldLists();
  enableAccept();
}

void QgsDelimitedTextSourceSelect::enableAccept()
{
  // If the geometry type field is enabled then there must be
  // a valid file, and it must be


  bool enabled = mFile->isValid();

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

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}
