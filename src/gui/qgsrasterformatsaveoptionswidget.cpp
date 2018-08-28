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
#include "qgsrasterlayer.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgssettings.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QMouseEvent>
#include <QMenu>


QMap< QString, QStringList > QgsRasterFormatSaveOptionsWidget::sBuiltinProfiles;

static const QString PYRAMID_JPEG_YCBCR_COMPRESSION( QStringLiteral( "JPEG_QUALITY_OVERVIEW=75 COMPRESS_OVERVIEW=JPEG PHOTOMETRIC_OVERVIEW=YCBCR INTERLEAVE_OVERVIEW=PIXEL" ) );
static const QString PYRAMID_JPEG_COMPRESSION( QStringLiteral( "JPEG_QUALITY_OVERVIEW=75 COMPRESS_OVERVIEW=JPEG INTERLEAVE_OVERVIEW=PIXEL" ) );

QgsRasterFormatSaveOptionsWidget::QgsRasterFormatSaveOptionsWidget( QWidget *parent, const QString &format,
    QgsRasterFormatSaveOptionsWidget::Type type, const QString &provider )
  : QWidget( parent )
  , mFormat( format )
  , mProvider( provider )
{
  setupUi( this );
  connect( mProfileNewButton, &QPushButton::clicked, this, &QgsRasterFormatSaveOptionsWidget::mProfileNewButton_clicked );
  connect( mProfileDeleteButton, &QPushButton::clicked, this, &QgsRasterFormatSaveOptionsWidget::mProfileDeleteButton_clicked );
  connect( mProfileResetButton, &QPushButton::clicked, this, &QgsRasterFormatSaveOptionsWidget::mProfileResetButton_clicked );
  connect( mOptionsAddButton, &QPushButton::clicked, this, &QgsRasterFormatSaveOptionsWidget::mOptionsAddButton_clicked );
  connect( mOptionsDeleteButton, &QPushButton::clicked, this, &QgsRasterFormatSaveOptionsWidget::mOptionsDeleteButton_clicked );
  connect( mOptionsLineEdit, &QLineEdit::editingFinished, this, &QgsRasterFormatSaveOptionsWidget::mOptionsLineEdit_editingFinished );

  setType( type );

  if ( sBuiltinProfiles.isEmpty() )
  {
    // key=profileKey values=format,profileName,options
    sBuiltinProfiles[ QStringLiteral( "z_adefault" )] = ( QStringList() << QString() << tr( "Default" ) << QString() );

    // these GTiff profiles are based on Tim's benchmarks at
    // http://linfiniti.com/2011/05/gdal-efficiency-of-various-compression-algorithms/
    // big: no compression | medium: reasonable size/speed tradeoff | small: smallest size
    sBuiltinProfiles[ QStringLiteral( "z_gtiff_1big" )] =
      ( QStringList() << QStringLiteral( "GTiff" ) << tr( "No compression" )
        << QStringLiteral( "COMPRESS=NONE BIGTIFF=IF_NEEDED" ) );
    sBuiltinProfiles[ QStringLiteral( "z_gtiff_2medium" )] =
      ( QStringList() << QStringLiteral( "GTiff" ) << tr( "Low compression" )
        << QStringLiteral( "COMPRESS=PACKBITS" ) );
    sBuiltinProfiles[ QStringLiteral( "z_gtiff_3small" )] =
      ( QStringList() << QStringLiteral( "GTiff" ) << tr( "High compression" )
        << QStringLiteral( "COMPRESS=DEFLATE PREDICTOR=2 ZLEVEL=9" ) );
    sBuiltinProfiles[ QStringLiteral( "z_gtiff_4jpeg" )] =
      ( QStringList() << QStringLiteral( "GTiff" ) << tr( "JPEG compression" )
        << QStringLiteral( "COMPRESS=JPEG JPEG_QUALITY=75" ) );

    // overview compression schemes for GTiff format, see
    // http://www.gdal.org/gdaladdo.html and http://www.gdal.org/frmt_gtiff.html
    // TODO - should we offer GDAL_TIFF_OVR_BLOCKSIZE option here or in QgsRasterPyramidsOptionsWidget ?
    sBuiltinProfiles[ QStringLiteral( "z__pyramids_gtiff_1big" )] =
      ( QStringList() << QStringLiteral( "_pyramids" ) << tr( "No compression" )
        << QStringLiteral( "COMPRESS_OVERVIEW=NONE BIGTIFF_OVERVIEW=IF_NEEDED" ) );
    sBuiltinProfiles[ QStringLiteral( "z__pyramids_gtiff_2medium" )] =
      ( QStringList() << QStringLiteral( "_pyramids" ) << tr( "Low compression" )
        << QStringLiteral( "COMPRESS_OVERVIEW=PACKBITS" ) );
    sBuiltinProfiles[ QStringLiteral( "z__pyramids_gtiff_3small" )] =
      ( QStringList() << QStringLiteral( "_pyramids" ) << tr( "High compression" )
        << QStringLiteral( "COMPRESS_OVERVIEW=DEFLATE PREDICTOR_OVERVIEW=2 ZLEVEL=9" ) ); // how to set zlevel?
    sBuiltinProfiles[ QStringLiteral( "z__pyramids_gtiff_4jpeg" )] =
      ( QStringList() << QStringLiteral( "_pyramids" ) << tr( "JPEG compression" )
        << PYRAMID_JPEG_YCBCR_COMPRESSION );
  }

  connect( mProfileComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ),
           this, &QgsRasterFormatSaveOptionsWidget::updateOptions );
  connect( mOptionsTable, &QTableWidget::cellChanged, this, &QgsRasterFormatSaveOptionsWidget::optionsTableChanged );
  connect( mOptionsHelpButton, &QAbstractButton::clicked, this, &QgsRasterFormatSaveOptionsWidget::helpOptions );
  connect( mOptionsValidateButton, &QAbstractButton::clicked, this, [ = ] { validateOptions(); } );

  // create eventFilter to map right click to swapOptionsUI()
  // mOptionsLabel->installEventFilter( this );
  mOptionsLineEdit->installEventFilter( this );
  mOptionsStackedWidget->installEventFilter( this );

  updateControls();
  updateProfiles();

  QgsDebugMsg( "done" );
}

void QgsRasterFormatSaveOptionsWidget::setFormat( const QString &format )
{
  mFormat = format;
  updateControls();
  updateProfiles();
}

void QgsRasterFormatSaveOptionsWidget::setProvider( const QString &provider )
{
  mProvider = provider;
  updateControls();
}

// show/hide widgets - we need this function if widget is used in creator
void QgsRasterFormatSaveOptionsWidget::setType( QgsRasterFormatSaveOptionsWidget::Type type )
{
  QList< QWidget * > widgets = this->findChildren<QWidget *>();
  if ( ( type == Table ) || ( type == LineEdit ) )
  {
    // hide all controls, except stacked widget
    Q_FOREACH ( QWidget *widget, widgets )
      widget->setVisible( false );
    mOptionsStackedWidget->setVisible( true );
    Q_FOREACH ( QWidget *widget, mOptionsStackedWidget->findChildren<QWidget *>() )
      widget->setVisible( true );

    // show relevant page
    if ( type == Table )
      swapOptionsUI( 0 );
    else if ( type == LineEdit )
      swapOptionsUI( 1 );
  }
  else
  {
    // show all widgets, except profile buttons (unless Full)
    Q_FOREACH ( QWidget *widget, widgets )
      widget->setVisible( true );
    if ( type != Full )
      mProfileButtons->setVisible( false );

    // show elevant page
    if ( type == ProfileLineEdit )
      swapOptionsUI( 1 );
  }
}

QString QgsRasterFormatSaveOptionsWidget::pseudoFormat() const
{
  return mPyramids ? QStringLiteral( "_pyramids" ) : mFormat;
}

void QgsRasterFormatSaveOptionsWidget::updateProfiles()
{
  // build profiles list = user + builtin(last)
  QString format = pseudoFormat();
  QStringList profileKeys = profiles();
  QMapIterator<QString, QStringList> it( sBuiltinProfiles );
  while ( it.hasNext() )
  {
    it.next();
    QString profileKey = it.key();
    if ( ! profileKeys.contains( profileKey ) && !it.value().isEmpty() )
    {
      // insert key if is for all formats or this format (GTiff)
      if ( it.value()[0].isEmpty() ||  it.value()[0] == format )
      {
        profileKeys.insert( 0, profileKey );
      }
    }
  }
  std::sort( profileKeys.begin(), profileKeys.end() );

  // populate mOptionsMap and mProfileComboBox
  mOptionsMap.clear();
  mProfileComboBox->blockSignals( true );
  mProfileComboBox->clear();
  Q_FOREACH ( const QString &profileKey, profileKeys )
  {
    QString profileName, profileOptions;
    profileOptions = createOptions( profileKey );
    if ( sBuiltinProfiles.contains( profileKey ) )
    {
      profileName = sBuiltinProfiles[ profileKey ][ 1 ];
      if ( profileOptions.isEmpty() )
        profileOptions = sBuiltinProfiles[ profileKey ][ 2 ];
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
  // mProfileComboBox->setCurrentIndex( 0 );
  QgsSettings mySettings;
  mProfileComboBox->setCurrentIndex( mProfileComboBox->findData( mySettings.value(
                                       mProvider + "/driverOptions/" + format.toLower() + "/defaultProfile",
                                       "z_adefault" ) ) );
  updateOptions();
}

void QgsRasterFormatSaveOptionsWidget::updateOptions()
{
  QString myOptions = mOptionsMap.value( currentProfileKey() );
  QStringList myOptionsList = myOptions.trimmed().split( ' ', QString::SkipEmptyParts );

  // If the default JPEG compression profile was selected, remove PHOTOMETRIC_OVERVIEW=YCBCR
  // if the raster is not RGB. Otherwise this is bound to fail afterwards.
  if ( mRasterLayer && mRasterLayer->bandCount() != 3 &&
       myOptions == PYRAMID_JPEG_YCBCR_COMPRESSION )
  {
    myOptions = PYRAMID_JPEG_COMPRESSION;
  }

  if ( mOptionsStackedWidget->currentIndex() == 0 )
  {
    mOptionsTable->setRowCount( 0 );
    for ( int i = 0; i < myOptionsList.count(); i++ )
    {
      QStringList key_value = myOptionsList[i].split( '=' );
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
    mOptionsLineEdit->setCursorPosition( 0 );
  }

  emit optionsChanged();
}

void QgsRasterFormatSaveOptionsWidget::apply()
{
  setCreateOptions();
}

// typedefs for gdal provider function pointers
typedef QString validateCreationOptionsFormat_t( const QStringList &createOptions, QString format );
typedef QString helpCreationOptionsFormat_t( QString format );

void QgsRasterFormatSaveOptionsWidget::helpOptions()
{
  QString message;

  if ( mProvider == QLatin1String( "gdal" ) && !mFormat.isEmpty() && ! mPyramids )
  {
    // get helpCreationOptionsFormat() function ptr for provider
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( mProvider ) );
    if ( library )
    {
      helpCreationOptionsFormat_t *helpCreationOptionsFormat =
        ( helpCreationOptionsFormat_t * ) cast_to_fptr( library->resolve( "helpCreationOptionsFormat" ) );
      if ( helpCreationOptionsFormat )
      {
        message = helpCreationOptionsFormat( mFormat );
      }
      else
      {
        message = library->fileName() + " does not have helpCreationOptionsFormat";
      }
    }
    else
      message = QStringLiteral( "cannot load provider library %1" ).arg( mProvider );


    if ( message.isEmpty() )
      message = tr( "Cannot get create options for driver %1" ).arg( mFormat );
  }
  else if ( mProvider == QLatin1String( "gdal" ) && mPyramids )
  {
    message = tr( "For details on pyramids options please see the following pages" );
    message += QLatin1String( "\n\nhttp://www.gdal.org/gdaladdo.html\n\nhttp://www.gdal.org/frmt_gtiff.html" );
  }
  else
    message = tr( "No help available" );

  // show simple non-modal dialog - should we make the basic xml prettier?
  QgsDialog *dlg = new QgsDialog( this );
  QTextEdit *textEdit = new QTextEdit( dlg );
  textEdit->setReadOnly( true );
  // message = tr( "Create Options:\n\n%1" ).arg( message );
  textEdit->setText( message );
  dlg->layout()->addWidget( textEdit );
  dlg->resize( 600, 400 );
#ifdef Q_OS_MAC
  dlg->exec(); //modal
#else
  dlg->show(); //non modal
#endif
}

QString QgsRasterFormatSaveOptionsWidget::validateOptions( bool gui, bool reportOK )
{
  QStringList createOptions = options();
  QString message;

  QgsDebugMsg( QString( "layer: [%1] file: [%2] format: [%3]" ).arg( mRasterLayer ? mRasterLayer->id() : "none", mRasterFileName, mFormat ) );
  // if no rasterLayer is defined, but we have a raster fileName, then create a temp. rasterLayer to validate options
  // ideally we should keep it for future access, but this is trickier
  QgsRasterLayer *rasterLayer = mRasterLayer;
  bool tmpLayer = false;
  if ( !( mRasterLayer && rasterLayer->dataProvider() ) && ! mRasterFileName.isNull() )
  {
    // temporarily override /Projections/defaultBehavior to avoid dialog prompt
    // this is taken from qgsbrowserdockwidget.cpp
    // TODO - integrate this into qgis core
    QgsSettings settings;
    QString defaultProjectionOption = settings.value( QStringLiteral( "Projections/defaultBehavior" ), "prompt" ).toString();
    if ( settings.value( QStringLiteral( "Projections/defaultBehavior" ), "prompt" ).toString() == QLatin1String( "prompt" ) )
    {
      settings.setValue( QStringLiteral( "Projections/defaultBehavior" ), "useProject" );
    }
    tmpLayer = true;
    rasterLayer = new QgsRasterLayer( mRasterFileName, QFileInfo( mRasterFileName ).baseName(), QStringLiteral( "gdal" ) );
    // restore /Projections/defaultBehavior
    if ( defaultProjectionOption == QLatin1String( "prompt" ) )
    {
      settings.setValue( QStringLiteral( "Projections/defaultBehavior" ), defaultProjectionOption );
    }
  }

  if ( mProvider == QLatin1String( "gdal" ) && mPyramids )
  {
    if ( rasterLayer && rasterLayer->dataProvider() )
    {
      QgsDebugMsg( "calling validate pyramids on layer's data provider" );
      message = rasterLayer->dataProvider()->validatePyramidsConfigOptions( mPyramidsFormat, createOptions, mFormat );
    }
    else
    {
      message = tr( "cannot validate pyramid options" );
    }
  }
  else if ( !createOptions.isEmpty() && mProvider == QLatin1String( "gdal" ) && !mFormat.isEmpty() )
  {
    if ( rasterLayer && rasterLayer->dataProvider() )
    {
      QgsDebugMsg( "calling validate on layer's data provider" );
      message = rasterLayer->dataProvider()->validateCreationOptions( createOptions, mFormat );
    }
    else
    {
      // get validateCreationOptionsFormat() function ptr for provider
      std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( mProvider ) );
      if ( library )
      {
        validateCreationOptionsFormat_t *validateCreationOptionsFormat =
          ( validateCreationOptionsFormat_t * ) cast_to_fptr( library->resolve( "validateCreationOptionsFormat" ) );
        if ( validateCreationOptionsFormat )
        {
          message = validateCreationOptionsFormat( createOptions, mFormat );
        }
        else
        {
          message = library->fileName() + " does not have validateCreationOptionsFormat";
        }
      }
      else
        message = QStringLiteral( "cannot load provider library %1" ).arg( mProvider );
    }
  }
  else if ( ! createOptions.isEmpty() )
  {
    QMessageBox::information( this, QString(), tr( "Cannot validate creation options." ), QMessageBox::Close );
    if ( tmpLayer )
      delete rasterLayer;
    return QString();
  }

  if ( gui )
  {
    if ( message.isNull() )
    {
      if ( reportOK )
        QMessageBox::information( this, QString(), tr( "Valid" ), QMessageBox::Close );
    }
    else
    {
      QMessageBox::warning( this, QString(), tr( "Invalid %1:\n\n%2\n\nClick on help button to get valid creation options for this format." ).arg( mPyramids ? tr( "pyramid creation option" ) : tr( "creation option" ), message ), QMessageBox::Close );
    }
  }

  if ( tmpLayer )
    delete rasterLayer;

  return message;
}

void QgsRasterFormatSaveOptionsWidget::optionsTableChanged()
{
  QTableWidgetItem *key, *value;
  QString options;
  for ( int i = 0; i < mOptionsTable->rowCount(); i++ )
  {
    key = mOptionsTable->item( i, 0 );
    if ( ! key  || key->text().isEmpty() )
      continue;
    value = mOptionsTable->item( i, 1 );
    if ( ! value  || value->text().isEmpty() )
      continue;
    options += key->text() + '=' + value->text() + ' ';
  }
  options = options.trimmed();
  mOptionsMap[ currentProfileKey()] = options;
  mOptionsLineEdit->setText( options );
  mOptionsLineEdit->setCursorPosition( 0 );
}

void QgsRasterFormatSaveOptionsWidget::mOptionsLineEdit_editingFinished()
{
  mOptionsMap[ currentProfileKey()] = mOptionsLineEdit->text().trimmed();
}

void QgsRasterFormatSaveOptionsWidget::mProfileNewButton_clicked()
{
  QString profileName = QInputDialog::getText( this, QString(), tr( "Profile name:" ) );
  if ( ! profileName.isEmpty() )
  {
    profileName = profileName.trimmed();
    mOptionsMap[ profileName ] = QString();
    mProfileComboBox->addItem( profileName, profileName );
    mProfileComboBox->setCurrentIndex( mProfileComboBox->count() - 1 );
  }
}

void QgsRasterFormatSaveOptionsWidget::mProfileDeleteButton_clicked()
{
  int index = mProfileComboBox->currentIndex();
  QString profileKey = currentProfileKey();
  if ( index != -1 && ! sBuiltinProfiles.contains( profileKey ) )
  {
    mOptionsMap.remove( profileKey );
    mProfileComboBox->removeItem( index );
  }
}

void QgsRasterFormatSaveOptionsWidget::mProfileResetButton_clicked()
{
  QString profileKey = currentProfileKey();
  if ( sBuiltinProfiles.contains( profileKey ) )
  {
    mOptionsMap[ profileKey ] = sBuiltinProfiles[ profileKey ][ 2 ];
  }
  else
  {
    mOptionsMap[ profileKey ] = QString();
  }
  mOptionsLineEdit->setText( mOptionsMap.value( currentProfileKey() ) );
  mOptionsLineEdit->setCursorPosition( 0 );
  updateOptions();
}

void QgsRasterFormatSaveOptionsWidget::optionsTableEnableDeleteButton()
{
  mOptionsDeleteButton->setEnabled( mOptionsTable->currentRow() >= 0 );
}

void QgsRasterFormatSaveOptionsWidget::mOptionsAddButton_clicked()
{
  mOptionsTable->insertRow( mOptionsTable->rowCount() );
  // select the added row
  int newRow = mOptionsTable->rowCount() - 1;
  QTableWidgetItem *item = new QTableWidgetItem();
  mOptionsTable->setItem( newRow, 0, item );
  mOptionsTable->setCurrentItem( item );
}

void QgsRasterFormatSaveOptionsWidget::mOptionsDeleteButton_clicked()
{
  if ( mOptionsTable->currentRow() >= 0 )
  {
    mOptionsTable->removeRow( mOptionsTable->currentRow() );
    // select the previous row or the next one if there is no previous row
    QTableWidgetItem *item = mOptionsTable->item( mOptionsTable->currentRow(), 0 );
    mOptionsTable->setCurrentItem( item );
    optionsTableChanged();
  }
}

QString QgsRasterFormatSaveOptionsWidget::settingsKey( QString profileName ) const
{
  if ( !profileName.isEmpty() )
    profileName = "/profile_" + profileName;
  else
    profileName = "/profile_default" + profileName;
  return mProvider + "/driverOptions/" + pseudoFormat().toLower() + profileName + "/create";
}

QString QgsRasterFormatSaveOptionsWidget::currentProfileKey() const
{
  return mProfileComboBox->currentData().toString();
}

QStringList QgsRasterFormatSaveOptionsWidget::options() const
{
  return mOptionsMap.value( currentProfileKey() ).trimmed().split( ' ', QString::SkipEmptyParts );
}

QString QgsRasterFormatSaveOptionsWidget::createOptions( const QString &profileName ) const
{
  QgsSettings mySettings;
  return mySettings.value( settingsKey( profileName ), "" ).toString();
}

void QgsRasterFormatSaveOptionsWidget::deleteCreateOptions( const QString &profileName )
{
  QgsSettings mySettings;
  mySettings.remove( settingsKey( profileName ) );
}

void QgsRasterFormatSaveOptionsWidget::setCreateOptions()
{
  QgsSettings mySettings;
  QStringList myProfiles;
  QMap< QString, QString >::const_iterator i = mOptionsMap.constBegin();
  while ( i != mOptionsMap.constEnd() )
  {
    setCreateOptions( i.key(), i.value() );
    myProfiles << i.key();
    ++i;
  }
  mySettings.setValue( mProvider + "/driverOptions/" + pseudoFormat().toLower() + "/profiles",
                       myProfiles );
  mySettings.setValue( mProvider + "/driverOptions/" + pseudoFormat().toLower() + "/defaultProfile",
                       currentProfileKey().trimmed() );
}

void QgsRasterFormatSaveOptionsWidget::setCreateOptions( const QString &profileName, const QString &options )
{
  QgsSettings mySettings;
  mySettings.setValue( settingsKey( profileName ), options.trimmed() );
}

void QgsRasterFormatSaveOptionsWidget::setCreateOptions( const QString &profileName, const QStringList &list )
{
  setCreateOptions( profileName, list.join( QStringLiteral( " " ) ) );
}

QStringList QgsRasterFormatSaveOptionsWidget::profiles() const
{
  QgsSettings mySettings;
  return mySettings.value( mProvider + "/driverOptions/" + pseudoFormat().toLower() + "/profiles", "" ).toStringList();
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

void QgsRasterFormatSaveOptionsWidget::updateControls()
{
  bool valid = mProvider == QLatin1String( "gdal" ) && !mFormat.isEmpty();
  mOptionsValidateButton->setEnabled( valid );
  mOptionsHelpButton->setEnabled( valid );
}

// map options label left mouse click to optionsToggle()
bool QgsRasterFormatSaveOptionsWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::MouseButtonPress )
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
    if ( mouseEvent && ( mouseEvent->button() == Qt::RightButton ) )
    {
      QMenu *menu = nullptr;
      QString text;
      if ( mOptionsStackedWidget->currentIndex() == 0 )
        text = tr( "Use simple interface" );
      else
        text = tr( "Use table interface" );
      if ( obj->objectName() == QLatin1String( "mOptionsLineEdit" ) )
      {
        menu = mOptionsLineEdit->createStandardContextMenu();
        menu->addSeparator();
      }
      else
        menu = new QMenu( this );
      QAction *action = new QAction( text, menu );
      menu->addAction( action );
      connect( action, &QAction::triggered, this, &QgsRasterFormatSaveOptionsWidget::swapOptionsUI );
      menu->exec( mouseEvent->globalPos() );
      delete menu;
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter( obj, event );
}

void QgsRasterFormatSaveOptionsWidget::showEvent( QShowEvent *event )
{
  Q_UNUSED( event );
  mOptionsTable->horizontalHeader()->resizeSection( 0, mOptionsTable->width() - 115 );
  QgsDebugMsg( "done" );
}

void QgsRasterFormatSaveOptionsWidget::setOptions( const QString &options )
{
  mOptionsTable->blockSignals( true );
  mOptionsTable->clearContents();

  QStringList values;
  QStringList optionsList = options.trimmed().split( ' ', QString::SkipEmptyParts );
  Q_FOREACH ( const QString &opt, optionsList )
  {
    int rowCount = mOptionsTable->rowCount();
    mOptionsTable->insertRow( rowCount );

    values = opt.split( '=' );
    if ( values.count() == 2 )
    {
      QTableWidgetItem *nameItem = new QTableWidgetItem( values.at( 0 ) );
      mOptionsTable->setItem( rowCount, 0, nameItem );
      QTableWidgetItem *valueItem = new QTableWidgetItem( values.at( 1 ) );
      mOptionsTable->setItem( rowCount, 0, valueItem );
    }
  }

  mOptionsMap[ currentProfileKey()] = options.trimmed();
  mOptionsLineEdit->setText( options.trimmed() );
  mOptionsLineEdit->setCursorPosition( 0 );

  mOptionsTable->blockSignals( false );
}
