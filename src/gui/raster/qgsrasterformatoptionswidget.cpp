/***************************************************************************
                          qgsrasterformatoptionswidget.cpp
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterformatoptionswidget.h"
#include "qgslogger.h"
#include "qgsdialog.h"

#include "gdal.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "cpl_minixml.h"

#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>

// todo put this somewhere else - how can we access gdal provider?
char** papszFromStringList( const QStringList& list )
{
  char **papszRetList = NULL;
  foreach( QString elem, list )
  {
    papszRetList = CSLAddString( papszRetList, elem.toLocal8Bit().constData() );
  }
  return papszRetList;
}


QMap< QString, QStringList > QgsRasterFormatOptionsWidget::mBuiltinProfiles;

QgsRasterFormatOptionsWidget::QgsRasterFormatOptionsWidget( QWidget* parent, QString format, QString provider )
    : QWidget( parent ), mFormat( format ), mProvider( provider )

{
  setupUi( this );

  if ( mBuiltinProfiles.isEmpty() )
  {
    // key=profileKey values=format,profileName,options
    mBuiltinProfiles[ "z_adefault" ] = ( QStringList() << "" << tr( "Default" ) << "" );
    // these GTiff profiles are based on Tim's benchmarks at
    // http://linfiniti.com/2011/05/gdal-efficiency-of-various-compression-algorithms/
    // big: no compression | medium: reasonable size/speed tradeoff | small: smallest size
    mBuiltinProfiles[ "z_gtiff_1big" ] = ( QStringList() << "GTiff" << tr( "No compression" )
                                           << "COMPRESS=NONE BIGTIFF=IF_NEEDED" );
    mBuiltinProfiles[ "z_gtiff_2medium" ] = ( QStringList() << "GTiff" << tr( "Low compression" )
                                            << "COMPRESS=PACKBITS" );
    mBuiltinProfiles[ "z_gtiff_3small" ] = ( QStringList() << "GTiff" << tr( "High compression" )
                                           << "COMPRESS=DEFLATE PREDICTOR=2 ZLEVEL=9" );
    mBuiltinProfiles[ "z_gtiff_4jpeg" ] = ( QStringList() << "GTiff" << tr( "Lossy compression" )
                                            << "COMPRESS=JPEG" );
  }

  showProfileButtons( false );

  connect( mProfileComboBox, SIGNAL( currentIndexChanged( const QString & ) ),
           this, SLOT( updateOptions() ) );
  connect( mOptionsTable, SIGNAL( cellChanged( int, int ) ), this, SLOT( optionsTableChanged() ) );
  // connect( mOptionsAddButton, SIGNAL( clicked() ), this, SLOT( optionsTableAddNewRow() ) );
  // connect( mOptionsDeleteButton, SIGNAL( clicked() ), this, SLOT( optionsTableDeleteRow() ) );
  connect( mOptionsHelpButton, SIGNAL( clicked() ), this, SLOT( optionsHelp() ) );
  connect( mOptionsValidateButton, SIGNAL( clicked() ), this, SLOT( optionsValidate() ) );

  updateProfiles();
}

QgsRasterFormatOptionsWidget::~QgsRasterFormatOptionsWidget()
{
}

void QgsRasterFormatOptionsWidget::setFormat( QString format )
{
  mFormat = format;
  updateProfiles();
}

void QgsRasterFormatOptionsWidget::setProvider( QString provider )
{
  mProvider = provider;
}


void QgsRasterFormatOptionsWidget::showProfileButtons( bool show )
{
  mProfileButtons->setVisible( show );
}

void QgsRasterFormatOptionsWidget::updateProfiles()
{
  // build profiles list = user + builtin(last)
  QStringList profileKeys = profiles();
  QMapIterator<QString, QStringList> it( mBuiltinProfiles );
  while ( it.hasNext() )
  {
    it.next();
    QString profileKey = it.key();
    if ( ! profileKeys.contains( profileKey ) )
    {
      // insert key if is for all formats or this format (GTiff)
      if ( it.value()[0] == "" ||  it.value()[0] == mFormat )
        profileKeys.insert( 0, profileKey );
    }
  }
  qSort( profileKeys );

  // populate mOptionsMap and mProfileComboBox
  mOptionsMap.clear();
  mProfileComboBox->blockSignals( true );
  mProfileComboBox->clear();
  foreach( QString profileKey, profileKeys )
  {
    QString profileName, profileOptions;
    profileOptions = createOptions( profileKey );
    if ( mBuiltinProfiles.contains( profileKey ) )
    {
      profileName = mBuiltinProfiles[ profileKey ][ 1 ];
      if ( profileOptions.isEmpty() )
        profileOptions = mBuiltinProfiles[ profileKey ][ 2 ];
    }
    else
    {
      profileName = profileKey;
    }
    mOptionsMap[ profileKey ] = profileOptions;
    mProfileComboBox->addItem( profileName, profileKey );
  }

  // update UI
  mProfileComboBox->blockSignals( false );
  mProfileComboBox->setCurrentIndex( 0 );
  updateOptions();
}

// void QgsRasterFormatOptionsWidget::on_mProfileComboBox_currentIndexChanged( const QString & text )
void QgsRasterFormatOptionsWidget::updateOptions()
{
  QString myOptions = mOptionsMap.value( currentProfileKey() );
  QStringList myOptionsList = myOptions.trimmed().split( " ", QString::SkipEmptyParts );

  if ( mOptionsStackedWIdget->currentIndex() == 0 )
  {
    mOptionsTable->setRowCount( 0 );
    for ( int i = 0; i < myOptionsList.count(); i++ )
    {
      QStringList key_value = myOptionsList[i].split( "=" );
      if ( key_value.count() == 2 )
      {
        mOptionsTable->insertRow( i );
        mOptionsTable->setItem( i, 0, new QTableWidgetItem( key_value[0] ) );
        mOptionsTable->setItem( i, 1, new QTableWidgetItem( key_value[1] ) );
      }
    }
  }
  else
  {
    mOptionsLineEdit->setText( myOptions );
  }
}

void QgsRasterFormatOptionsWidget::apply()
{
  setCreateOptions();
}


void QgsRasterFormatOptionsWidget::optionsHelp()
{
  QString message;

  if ( mProvider == "gdal" && mFormat != "" && mFormat != "_pyramids" )
  {
    GDALDriverH myGdalDriver = GDALGetDriverByName( mFormat.toLocal8Bit().constData() );
    if ( myGdalDriver )
    {
      // need to serialize xml to get newlines
      CPLXMLNode *psCOL = CPLParseXMLString( GDALGetMetadataItem( myGdalDriver,
                                             GDAL_DMD_CREATIONOPTIONLIST, "" ) );
      char *pszFormattedXML = CPLSerializeXMLTree( psCOL );
      if ( pszFormattedXML )
        message = tr( "Create Options:\n\n%1" ).arg( pszFormattedXML );
      if ( psCOL )
        CPLDestroyXMLNode( psCOL );
      if ( pszFormattedXML )
        CPLFree( pszFormattedXML );
    }
    if ( message.isEmpty() )
      message = tr( "Cannot get create options for driver %1" ).arg( mFormat );
  }
  else
    message = tr( "No help available" );

  // show simple non-modal dialog - should we make the basic xml prettier?
  QgsDialog *dlg = new QgsDialog( this );
  QTextEdit *textEdit = new QTextEdit( dlg );
  textEdit->setReadOnly( true );
  textEdit->setText( message );
  dlg->layout()->addWidget( textEdit );
  dlg->resize( 600, 600 );
  dlg->show(); //non modal
}

bool QgsRasterFormatOptionsWidget::optionsValidate( bool message )
{
  QStringList options = createOptions();
  bool ok = false;

  if ( !options.isEmpty() && mProvider == "gdal" && mFormat != "" && mFormat != "_pyramids" )
  {
    GDALDriverH myGdalDriver = GDALGetDriverByName( mFormat.toLocal8Bit().constData() );
    if ( myGdalDriver )
    {
      // print error string?
      char** papszOptions = papszFromStringList( options );
      ok = ( GDALValidateCreationOptions( myGdalDriver, papszOptions ) == TRUE );
      CSLDestroy( papszOptions );
      if ( message )
      {
        if ( ok )
          QMessageBox::information( this, "", tr( "Valid" ), QMessageBox::Close );
        else
          QMessageBox::warning( this, "", tr( "Invalid" ), QMessageBox::Close );
      }
    }
  }
  return ok;
}

void QgsRasterFormatOptionsWidget::optionsTableChanged()
{
  QTableWidgetItem *key, *value;
  QString options;
  for ( int i = 0 ; i < mOptionsTable->rowCount(); i++ )
  {
    key = mOptionsTable->item( i, 0 );
    if ( ! key  || key->text().isEmpty() )
      continue;
    value = mOptionsTable->item( i, 1 );
    if ( ! value  || value->text().isEmpty() )
      continue;
    options += key->text() + "=" + value->text() + " ";
  }
  options = options.trimmed();
  mOptionsMap[ currentProfileKey()] = options;
  mOptionsLineEdit->setText( options );
}

void QgsRasterFormatOptionsWidget::on_mOptionsLineEdit_editingFinished()
{
  mOptionsMap[ currentProfileKey()] = mOptionsLineEdit->text().trimmed();
}

void QgsRasterFormatOptionsWidget::on_mProfileNewButton_clicked()
{
  QString profileName = QInputDialog::getText( this, "", tr( "Profile name:" ) );
  if ( ! profileName.isEmpty() )
  {
    profileName = profileName.trimmed();
    mOptionsMap[ profileName ] = "";
    mProfileComboBox->addItem( profileName, profileName );
    mProfileComboBox->setCurrentIndex( mProfileComboBox->count() - 1 );
  }
}

void QgsRasterFormatOptionsWidget::on_mProfileDeleteButton_clicked()
{
  int index = mProfileComboBox->currentIndex();
  QString profileKey = currentProfileKey();
  if ( index != -1 && ! mBuiltinProfiles.contains( profileKey ) )
  {
    mOptionsMap.remove( profileKey );
    mProfileComboBox->removeItem( index );
  }
}

void QgsRasterFormatOptionsWidget::on_mProfileResetButton_clicked()
{
  QString profileKey = currentProfileKey();
  if ( mBuiltinProfiles.contains( profileKey ) )
  {
    mOptionsMap[ profileKey ] = mBuiltinProfiles[ profileKey ][ 2 ];
  }
  else
  {
    mOptionsMap[ profileKey ] = "";
  }
  mOptionsLineEdit->setText( mOptionsMap.value( currentProfileKey() ) );
  updateOptions();
}

void QgsRasterFormatOptionsWidget::on_mOptionsLabel_clicked()
{
  mOptionsStackedWIdget->setCurrentIndex(( mOptionsStackedWIdget->currentIndex() + 1 ) % 2 );
  updateOptions();
}

void QgsRasterFormatOptionsWidget::optionsTableEnableDeleteButton()
{
  mOptionsDeleteButton->setEnabled( mOptionsTable->currentRow() >= 0 );
}

void QgsRasterFormatOptionsWidget::on_mOptionsAddButton_clicked()
{
  mOptionsTable->insertRow( mOptionsTable->rowCount() );
  // select the added row
  int newRow = mOptionsTable->rowCount() - 1;
  QTableWidgetItem* item = new QTableWidgetItem();
  mOptionsTable->setItem( newRow, 0, item );
  mOptionsTable->setCurrentItem( item );
}

void QgsRasterFormatOptionsWidget::on_mOptionsDeleteButton_clicked()
{
  if ( mOptionsTable->currentRow() >= 0 )
  {
    mOptionsTable->removeRow( mOptionsTable->currentRow() );
    // select the previous row or the next one if there is no previous row
    QTableWidgetItem* item = mOptionsTable->item( mOptionsTable->currentRow(), 0 );
    mOptionsTable->setCurrentItem( item );
    optionsTableChanged();
  }
}


QString QgsRasterFormatOptionsWidget::settingsKey( QString profileName ) const
{
  if ( profileName != "" )
    profileName = "/profile_" + profileName;
  else
    profileName = "/profile_default" + profileName;
  return mProvider + "/driverOptions/" + mFormat.toLower() + profileName + "/create";
}

QString QgsRasterFormatOptionsWidget::currentProfileKey() const
{
  return mProfileComboBox->itemData( mProfileComboBox->currentIndex() ).toString();
}

QStringList QgsRasterFormatOptionsWidget::createOptions() const
{
  return mOptionsMap.value( currentProfileKey() ).trimmed().split( " ", QString::SkipEmptyParts );
}

QString QgsRasterFormatOptionsWidget::createOptions( QString profileName ) const
{
  QSettings mySettings;
  return mySettings.value( settingsKey( profileName ), "" ).toString();
}

void QgsRasterFormatOptionsWidget::deleteCreateOptions( QString profileName )
{
  QSettings mySettings;
  mySettings.remove( settingsKey( profileName ) );
}

void QgsRasterFormatOptionsWidget::setCreateOptions()
{
  QSettings mySettings;
  QString myProfiles;
  QMap< QString, QString >::iterator i = mOptionsMap.begin();
  while ( i != mOptionsMap.end() )
  {
    setCreateOptions( i.key(), i.value() );
    myProfiles += i.key() + QString( " " );
    ++i;
  }
  mySettings.setValue( mProvider + "/driverOptions/" + mFormat.toLower() + "/profiles", myProfiles.trimmed() );
}

void QgsRasterFormatOptionsWidget::setCreateOptions( QString profileName, QString options )
{
  QSettings mySettings;
  mySettings.setValue( settingsKey( profileName ), options.trimmed() );
}

void QgsRasterFormatOptionsWidget::setCreateOptions( QString profileName, QStringList list )
{
  setCreateOptions( profileName, list.join( " " ) );
}

QStringList QgsRasterFormatOptionsWidget::profiles() const
{
  QSettings mySettings;
  return mySettings.value( mProvider + "/driverOptions/" + mFormat.toLower() + "/profiles", "" ).toString().trimmed().split( " ", QString::SkipEmptyParts );
}
