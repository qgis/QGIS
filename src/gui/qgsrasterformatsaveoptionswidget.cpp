/***************************************************************************
                          qgsrasterformatsaveoptionswidget.cpp
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

#include "qgsrasterformatsaveoptionswidget.h"
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
#include <QMouseEvent>
#include <QMenu>

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

QMap< QString, QStringList > QgsRasterFormatSaveOptionsWidget::mBuiltinProfiles;

QgsRasterFormatSaveOptionsWidget::QgsRasterFormatSaveOptionsWidget( QWidget* parent, QString format,
    QgsRasterFormatSaveOptionsWidget::Type type,
    QString provider )
    : QWidget( parent ), mFormat( format ), mProvider( provider )

{
  setupUi( this );


  setType( type );

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

  connect( mProfileComboBox, SIGNAL( currentIndexChanged( const QString & ) ),
           this, SLOT( updateOptions() ) );
  connect( mOptionsTable, SIGNAL( cellChanged( int, int ) ), this, SLOT( optionsTableChanged() ) );
  connect( mOptionsHelpButton, SIGNAL( clicked() ), this, SLOT( helpOptions() ) );
  connect( mOptionsValidateButton, SIGNAL( clicked() ), this, SLOT( validateOptions() ) );

  // create eventFilter to map right click to swapOptionsUI()
  // mOptionsLabel->installEventFilter( this );
  mOptionsLineEdit->installEventFilter( this );
  mOptionsStackedWidget->installEventFilter( this );

  updateProfiles();
}

QgsRasterFormatSaveOptionsWidget::~QgsRasterFormatSaveOptionsWidget()
{
}

void QgsRasterFormatSaveOptionsWidget::setFormat( QString format )
{
  mFormat = format;
  updateProfiles();
}

void QgsRasterFormatSaveOptionsWidget::setProvider( QString provider )
{
  mProvider = provider;
}

// show/hide widgets - we need this function if widget is used in creator
void QgsRasterFormatSaveOptionsWidget::setType( QgsRasterFormatSaveOptionsWidget::Type type )
{
  QList< QWidget* > widgets = this->findChildren<QWidget *>();
  if (( type == Table ) || ( type == LineEdit ) )
  {
    // hide all controls, except stacked widget
    foreach( QWidget* widget, widgets )
    {
      widget->setVisible( false );
    }
    mOptionsStackedWidget->setVisible( true );
    foreach( QWidget* widget, mOptionsStackedWidget->findChildren<QWidget *>() )
    {
      widget->setVisible( true );
    }
    // show page relevant page
    if ( type == Table )
      swapOptionsUI( 0 );
    else if ( type == LineEdit )
      swapOptionsUI( 1 );
  }
  else
  {
    // show all widgets, except profile buttons (unless Full)
    foreach( QWidget* widget, widgets )
    {
      widget->setVisible( true );
    }
    if ( type != Full )
    {
      mProfileButtons->setVisible( false );
    }
  }
}

void QgsRasterFormatSaveOptionsWidget::updateProfiles()
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

void QgsRasterFormatSaveOptionsWidget::updateOptions()
{
  QString myOptions = mOptionsMap.value( currentProfileKey() );
  QStringList myOptionsList = myOptions.trimmed().split( " ", QString::SkipEmptyParts );

  if ( mOptionsStackedWidget->currentIndex() == 0 )
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

void QgsRasterFormatSaveOptionsWidget::apply()
{
  setCreateOptions();
}


void QgsRasterFormatSaveOptionsWidget::helpOptions()
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

bool QgsRasterFormatSaveOptionsWidget::validateOptions( bool gui )
{
  QStringList createOptions = options();
  bool ok = false;

  if ( !createOptions.isEmpty() && mProvider == "gdal" && mFormat != "" && mFormat != "_pyramids" )
  {
    GDALDriverH myGdalDriver = GDALGetDriverByName( mFormat.toLocal8Bit().constData() );
    if ( myGdalDriver )
    {
      // print error string?
      char** papszOptions = papszFromStringList( createOptions );
      ok = ( GDALValidateCreationOptions( myGdalDriver, papszOptions ) == TRUE );
      CSLDestroy( papszOptions );
      if ( gui )
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

void QgsRasterFormatSaveOptionsWidget::optionsTableChanged()
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

void QgsRasterFormatSaveOptionsWidget::on_mOptionsLineEdit_editingFinished()
{
  mOptionsMap[ currentProfileKey()] = mOptionsLineEdit->text().trimmed();
}

void QgsRasterFormatSaveOptionsWidget::on_mProfileNewButton_clicked()
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

void QgsRasterFormatSaveOptionsWidget::on_mProfileDeleteButton_clicked()
{
  int index = mProfileComboBox->currentIndex();
  QString profileKey = currentProfileKey();
  if ( index != -1 && ! mBuiltinProfiles.contains( profileKey ) )
  {
    mOptionsMap.remove( profileKey );
    mProfileComboBox->removeItem( index );
  }
}

void QgsRasterFormatSaveOptionsWidget::on_mProfileResetButton_clicked()
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

void QgsRasterFormatSaveOptionsWidget::optionsTableEnableDeleteButton()
{
  mOptionsDeleteButton->setEnabled( mOptionsTable->currentRow() >= 0 );
}

void QgsRasterFormatSaveOptionsWidget::on_mOptionsAddButton_clicked()
{
  mOptionsTable->insertRow( mOptionsTable->rowCount() );
  // select the added row
  int newRow = mOptionsTable->rowCount() - 1;
  QTableWidgetItem* item = new QTableWidgetItem();
  mOptionsTable->setItem( newRow, 0, item );
  mOptionsTable->setCurrentItem( item );
}

void QgsRasterFormatSaveOptionsWidget::on_mOptionsDeleteButton_clicked()
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


QString QgsRasterFormatSaveOptionsWidget::settingsKey( QString profileName ) const
{
  if ( profileName != "" )
    profileName = "/profile_" + profileName;
  else
    profileName = "/profile_default" + profileName;
  return mProvider + "/driverOptions/" + mFormat.toLower() + profileName + "/create";
}

QString QgsRasterFormatSaveOptionsWidget::currentProfileKey() const
{
  return mProfileComboBox->itemData( mProfileComboBox->currentIndex() ).toString();
}

QStringList QgsRasterFormatSaveOptionsWidget::options() const
{
  return mOptionsMap.value( currentProfileKey() ).trimmed().split( " ", QString::SkipEmptyParts );
}

QString QgsRasterFormatSaveOptionsWidget::createOptions( QString profileName ) const
{
  QSettings mySettings;
  return mySettings.value( settingsKey( profileName ), "" ).toString();
}

void QgsRasterFormatSaveOptionsWidget::deleteCreateOptions( QString profileName )
{
  QSettings mySettings;
  mySettings.remove( settingsKey( profileName ) );
}

void QgsRasterFormatSaveOptionsWidget::setCreateOptions()
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

void QgsRasterFormatSaveOptionsWidget::setCreateOptions( QString profileName, QString options )
{
  QSettings mySettings;
  mySettings.setValue( settingsKey( profileName ), options.trimmed() );
}

void QgsRasterFormatSaveOptionsWidget::setCreateOptions( QString profileName, QStringList list )
{
  setCreateOptions( profileName, list.join( " " ) );
}

QStringList QgsRasterFormatSaveOptionsWidget::profiles() const
{
  QSettings mySettings;
  return mySettings.value( mProvider + "/driverOptions/" + mFormat.toLower() + "/profiles", "" ).toString().trimmed().split( " ", QString::SkipEmptyParts );
}

void QgsRasterFormatSaveOptionsWidget::swapOptionsUI( int newIndex )
{
  // set new page
  int oldIndex;
  if ( newIndex == -1 )
  {
    oldIndex = mOptionsStackedWidget->currentIndex();
    newIndex = ( oldIndex + 1 ) % 2;
  }
  else
  {
    oldIndex = ( newIndex + 1 ) % 2;
  }

  // resize pages to minimum - this works well with gdaltools merge ui, but not raster save as...
  mOptionsStackedWidget->setCurrentIndex( newIndex );
  mOptionsStackedWidget->widget( newIndex )->setSizePolicy(
    QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
  mOptionsStackedWidget->widget( oldIndex )->setSizePolicy(
    QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored ) );
  layout()->activate();

  updateOptions();
}

// map options label left mouse click to optionsToggle()
bool QgsRasterFormatSaveOptionsWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::MouseButtonPress )
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
    if ( mouseEvent && ( mouseEvent->button() == Qt::RightButton ) )
    {
      QMenu* menu = 0;
      QString text;
      if ( mOptionsStackedWidget->currentIndex() == 0 )
        text = tr( "Use simple interface" );
      else
        text = tr( "Use table interface" );
      if ( obj->objectName() == "mOptionsLineEdit" )
      {
        menu = mOptionsLineEdit->createStandardContextMenu();
        menu->addSeparator();
      }
      else
        menu = new QMenu( this );
      QAction* action = new QAction( text, menu );
      menu->addAction( action );
      connect( action, SIGNAL( triggered() ), this, SLOT( swapOptionsUI() ) );
      menu->exec( mouseEvent->globalPos() );
      delete menu;
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter( obj, event );
}

