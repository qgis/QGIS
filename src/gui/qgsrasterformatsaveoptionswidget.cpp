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

#include "qgsdialog.h"
#include "qgsgdalutils.h"
#include "qgslogger.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"

#include <QContextMenuEvent>
#include <QFileInfo>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextEdit>

#include "moc_qgsrasterformatsaveoptionswidget.cpp"

QMap<QString, QStringList> QgsRasterFormatSaveOptionsWidget::sBuiltinProfiles;

static const QString PYRAMID_JPEG_YCBCR_COMPRESSION( u"JPEG_QUALITY_OVERVIEW=75 COMPRESS_OVERVIEW=JPEG PHOTOMETRIC_OVERVIEW=YCBCR INTERLEAVE_OVERVIEW=PIXEL"_s );
static const QString PYRAMID_JPEG_COMPRESSION( u"JPEG_QUALITY_OVERVIEW=75 COMPRESS_OVERVIEW=JPEG INTERLEAVE_OVERVIEW=PIXEL"_s );

QgsRasterFormatSaveOptionsWidget::QgsRasterFormatSaveOptionsWidget( QWidget *parent, const QString &format, QgsRasterFormatSaveOptionsWidget::Type type, const QString &provider )
  : QWidget( parent )
  , mFormat( format )
  , mProvider( provider )
{
  setupUi( this );

  // Set the table minimum size to fit at least 4 rows
  mOptionsTable->setMinimumSize( 200, mOptionsTable->verticalHeader()->defaultSectionSize() * 4 + mOptionsTable->horizontalHeader()->height() + 2 );

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
    sBuiltinProfiles[u"z_adefault"_s] = ( QStringList() << QString() << tr( "Default" ) << QString() );

    // these GTiff profiles are based on Tim's benchmarks at
    // http://linfiniti.com/2011/05/gdal-efficiency-of-various-compression-algorithms/
    // big: no compression | medium: reasonable size/speed tradeoff | small: smallest size
    sBuiltinProfiles[u"z_gtiff_1big"_s] = ( QStringList() << u"GTiff"_s << tr( "No Compression" ) << u"COMPRESS=NONE BIGTIFF=IF_NEEDED"_s );
    sBuiltinProfiles[u"z_gtiff_2medium"_s] = ( QStringList() << u"GTiff"_s << tr( "Low Compression" ) << u"COMPRESS=PACKBITS"_s );
    sBuiltinProfiles[u"z_gtiff_3small"_s] = ( QStringList() << u"GTiff"_s << tr( "High Compression" ) << u"COMPRESS=DEFLATE PREDICTOR=2 ZLEVEL=9"_s );
    sBuiltinProfiles[u"z_gtiff_4jpeg"_s] = ( QStringList() << u"GTiff"_s << tr( "JPEG Compression" ) << u"COMPRESS=JPEG JPEG_QUALITY=75"_s );

    // overview compression schemes for GTiff format, see
    // http://www.gdal.org/gdaladdo.html and http://www.gdal.org/frmt_gtiff.html
    // TODO - should we offer GDAL_TIFF_OVR_BLOCKSIZE option here or in QgsRasterPyramidsOptionsWidget ?
    sBuiltinProfiles[u"z__pyramids_gtiff_1big"_s] = ( QStringList() << u"_pyramids"_s << tr( "No Compression" ) << u"COMPRESS_OVERVIEW=NONE BIGTIFF_OVERVIEW=IF_NEEDED"_s );
    sBuiltinProfiles[u"z__pyramids_gtiff_2medium"_s] = ( QStringList() << u"_pyramids"_s << tr( "Low Compression" ) << u"COMPRESS_OVERVIEW=PACKBITS"_s );
    sBuiltinProfiles[u"z__pyramids_gtiff_3small"_s] = ( QStringList() << u"_pyramids"_s << tr( "High Compression" ) << u"COMPRESS_OVERVIEW=DEFLATE PREDICTOR_OVERVIEW=2 ZLEVEL=9"_s ); // how to set zlevel?
    sBuiltinProfiles[u"z__pyramids_gtiff_4jpeg"_s] = ( QStringList() << u"_pyramids"_s << tr( "JPEG Compression" ) << PYRAMID_JPEG_YCBCR_COMPRESSION );
  }

  connect( mProfileComboBox, &QComboBox::currentTextChanged, this, &QgsRasterFormatSaveOptionsWidget::updateOptions );
  connect( mOptionsTable, &QTableWidget::cellChanged, this, &QgsRasterFormatSaveOptionsWidget::optionsTableChanged );
  connect( mOptionsHelpButton, &QAbstractButton::clicked, this, &QgsRasterFormatSaveOptionsWidget::helpOptions );
  connect( mOptionsValidateButton, &QAbstractButton::clicked, this, [this] { validateOptions(); } );

  // Install an eventFilter to customize the default QLineEdit contextMenu with an added swapOptionsUI action
  mOptionsLineEdit->installEventFilter( this );

  // Use a Custom Context menu for the widget to swap between modes (table / lineedit)
  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, &QWidget::customContextMenuRequested, this, [this]( QPoint pos ) {
    QMenu menu( this );
    QString text;
    if ( mTableWidget->isVisible() )
      text = tr( "Use Simple Interface" );
    else
      text = tr( "Use Table Interface" );
    QAction *swapAction = menu.addAction( text );
    connect( swapAction, &QAction::triggered, this, [this]() { swapOptionsUI( -1 ); } );
    menu.exec( this->mapToGlobal( pos ) );
  } );


  updateControls();
  updateProfiles();

  QgsDebugMsgLevel( u"done"_s, 3 );
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
  const QList<QWidget *> widgets = this->findChildren<QWidget *>();
  if ( ( type == Table ) || ( type == LineEdit ) )
  {
    // hide all controls, except stacked widget
    const auto constWidgets = widgets;
    for ( QWidget *widget : constWidgets )
      widget->setVisible( false );
    mOptionsWidget->setVisible( true );

    // show relevant page
    if ( type == Table )
      swapOptionsUI( 0 );
    else
      swapOptionsUI( 1 );
  }
  else
  {
    // show all widgets, except profile buttons (unless Full)
    const auto constWidgets = widgets;
    for ( QWidget *widget : constWidgets )
      widget->setVisible( true );
    if ( type != Full )
      mProfileButtons->setVisible( false );

    // show elevant page
    if ( type == ProfileLineEdit )
      swapOptionsUI( 1 );
    else
      swapOptionsUI( 0 );
  }
}

QString QgsRasterFormatSaveOptionsWidget::pseudoFormat() const
{
  return mPyramids ? u"_pyramids"_s : mFormat;
}

void QgsRasterFormatSaveOptionsWidget::updateProfiles()
{
  // build profiles list = user + builtin(last)
  const QString format = pseudoFormat();
  QStringList profileKeys = profiles();
  QMapIterator<QString, QStringList> it( sBuiltinProfiles );
  while ( it.hasNext() )
  {
    it.next();
    const QString profileKey = it.key();
    if ( !profileKeys.contains( profileKey ) && !it.value().isEmpty() )
    {
      // insert key if is for all formats or this format (GTiff)
      if ( it.value()[0].isEmpty() || it.value()[0] == format )
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
  const auto constProfileKeys = profileKeys;
  for ( const QString &profileKey : constProfileKeys )
  {
    QString profileName, profileOptions;
    profileOptions = creationOptions( profileKey );
    if ( sBuiltinProfiles.contains( profileKey ) )
    {
      profileName = sBuiltinProfiles[profileKey][1];
      if ( profileOptions.isEmpty() )
        profileOptions = sBuiltinProfiles[profileKey][2];
    }
    else
    {
      profileName = profileKey;
    }
    mOptionsMap[profileKey] = profileOptions;
    mProfileComboBox->addItem( profileName, profileKey );
  }

  // update UI
  mProfileComboBox->blockSignals( false );
  // mProfileComboBox->setCurrentIndex( 0 );
  const QgsSettings mySettings;
  mProfileComboBox->setCurrentIndex( mProfileComboBox->findData( mySettings.value(
    mProvider + "/driverOptions/" + format.toLower() + "/defaultProfile",
    "z_adefault"
  ) ) );
  updateOptions();
}

void QgsRasterFormatSaveOptionsWidget::updateOptions()
{
  mBlockOptionUpdates++;
  QString myOptions = mOptionsMap.value( currentProfileKey() );
  QStringList myOptionsList = myOptions.trimmed().split( ' ', Qt::SkipEmptyParts );

  // If the default JPEG compression profile was selected, remove PHOTOMETRIC_OVERVIEW=YCBCR
  // if the raster is not RGB. Otherwise this is bound to fail afterwards.
  if ( mRasterLayer && mRasterLayer->bandCount() != 3 && myOptions == PYRAMID_JPEG_YCBCR_COMPRESSION )
  {
    myOptions = PYRAMID_JPEG_COMPRESSION;
  }

  if ( mTableWidget->isVisible() )
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

  mBlockOptionUpdates--;
  emit optionsChanged();
}

void QgsRasterFormatSaveOptionsWidget::apply()
{
  setCreationOptions();
}

void QgsRasterFormatSaveOptionsWidget::helpOptions()
{
  QString message;

  if ( mProvider == "gdal"_L1 && !mFormat.isEmpty() && !mPyramids )
  {
    message = QgsGdalUtils::helpCreationOptionsFormat( mFormat );
    if ( message.isEmpty() )
      message = tr( "Cannot get create options for driver %1" ).arg( mFormat );
  }
  else if ( mProvider == "gdal"_L1 && mPyramids )
  {
    message = tr( "For details on pyramids options please see the following pages" );
    message += "\n\nhttps://gdal.org/programs/gdaladdo.html\n\nhttps://gdal.org/drivers/raster/gtiff.html"_L1;
  }
  else
    message = tr( "No help available" );

  // show simple non-modal dialog - should we make the basic xml prettier?
  QgsDialog *dlg = new QgsDialog( this );
  dlg->setWindowTitle( tr( "Create Options for %1" ).arg( mFormat ) );
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
  const QStringList creationOptions = options();
  QString message;

  QgsDebugMsgLevel( u"layer: [%1] file: [%2] format: [%3]"_s.arg( mRasterLayer ? mRasterLayer->id() : "none", mRasterFileName, mFormat ), 2 );
  // if no rasterLayer is defined, but we have a raster fileName, then create a temp. rasterLayer to validate options
  // ideally we should keep it for future access, but this is trickier
  QgsRasterLayer *rasterLayer = mRasterLayer;
  bool tmpLayer = false;
  if ( !( mRasterLayer && rasterLayer->dataProvider() ) && !mRasterFileName.isNull() )
  {
    tmpLayer = true;
    QgsRasterLayer::LayerOptions options;
    options.skipCrsValidation = true;
    rasterLayer = new QgsRasterLayer( mRasterFileName, QFileInfo( mRasterFileName ).baseName(), u"gdal"_s, options );
  }

  if ( mProvider == "gdal"_L1 && mPyramids )
  {
    if ( rasterLayer && rasterLayer->dataProvider() )
    {
      QgsDebugMsgLevel( u"calling validate pyramids on layer's data provider"_s, 2 );
      message = rasterLayer->dataProvider()->validatePyramidsConfigOptions( mPyramidsFormat, creationOptions, mFormat );
    }
    else
    {
      message = tr( "cannot validate pyramid options" );
    }
  }
  else if ( !creationOptions.isEmpty() && mProvider == "gdal"_L1 && !mFormat.isEmpty() )
  {
    if ( rasterLayer && rasterLayer->dataProvider() )
    {
      QgsDebugMsgLevel( u"calling validate on layer's data provider"_s, 2 );
      message = rasterLayer->dataProvider()->validateCreationOptions( creationOptions, mFormat );
    }
    else
    {
      // get validateCreationOptionsFormat() function ptr for provider
      message = QgsGdalUtils::validateCreationOptionsFormat( creationOptions, mFormat );
    }
  }
  else if ( !creationOptions.isEmpty() )
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
  if ( mBlockOptionUpdates )
    return;

  QTableWidgetItem *key, *value;
  QString options;
  for ( int i = 0; i < mOptionsTable->rowCount(); i++ )
  {
    key = mOptionsTable->item( i, 0 );
    if ( !key || key->text().isEmpty() )
      continue;
    value = mOptionsTable->item( i, 1 );
    if ( !value || value->text().isEmpty() )
      continue;
    options += key->text() + '=' + value->text() + ' ';
  }
  options = options.trimmed();
  mOptionsMap[currentProfileKey()] = options;
  mOptionsLineEdit->setText( options );
  mOptionsLineEdit->setCursorPosition( 0 );
}

void QgsRasterFormatSaveOptionsWidget::mOptionsLineEdit_editingFinished()
{
  mOptionsMap[currentProfileKey()] = mOptionsLineEdit->text().trimmed();
}

void QgsRasterFormatSaveOptionsWidget::mProfileNewButton_clicked()
{
  QString profileName = QInputDialog::getText( this, QString(), tr( "Profile name:" ) );
  if ( !profileName.isEmpty() )
  {
    profileName = profileName.trimmed();
    mOptionsMap[profileName] = QString();
    mProfileComboBox->addItem( profileName, profileName );
    mProfileComboBox->setCurrentIndex( mProfileComboBox->count() - 1 );
  }
}

void QgsRasterFormatSaveOptionsWidget::mProfileDeleteButton_clicked()
{
  const int index = mProfileComboBox->currentIndex();
  const QString profileKey = currentProfileKey();
  if ( index != -1 && !sBuiltinProfiles.contains( profileKey ) )
  {
    mOptionsMap.remove( profileKey );
    mProfileComboBox->removeItem( index );
  }
}

void QgsRasterFormatSaveOptionsWidget::mProfileResetButton_clicked()
{
  const QString profileKey = currentProfileKey();
  if ( sBuiltinProfiles.contains( profileKey ) )
  {
    mOptionsMap[profileKey] = sBuiltinProfiles[profileKey][2];
  }
  else
  {
    mOptionsMap[profileKey] = QString();
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
  const int newRow = mOptionsTable->rowCount() - 1;
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
  return mOptionsMap.value( currentProfileKey() ).trimmed().split( ' ', Qt::SkipEmptyParts );
}

QString QgsRasterFormatSaveOptionsWidget::creationOptions( const QString &profileName ) const
{
  const QgsSettings mySettings;
  return mySettings.value( settingsKey( profileName ), "" ).toString();
}

void QgsRasterFormatSaveOptionsWidget::deleteCreationOptions( const QString &profileName )
{
  QgsSettings mySettings;
  mySettings.remove( settingsKey( profileName ) );
}

void QgsRasterFormatSaveOptionsWidget::setCreationOptions()
{
  QgsSettings mySettings;
  QStringList myProfiles;
  QMap<QString, QString>::const_iterator i = mOptionsMap.constBegin();
  while ( i != mOptionsMap.constEnd() )
  {
    setCreationOptions( i.key(), i.value() );
    myProfiles << i.key();
    ++i;
  }
  mySettings.setValue( mProvider + "/driverOptions/" + pseudoFormat().toLower() + "/profiles", myProfiles );
  mySettings.setValue( mProvider + "/driverOptions/" + pseudoFormat().toLower() + "/defaultProfile", currentProfileKey().trimmed() );
}

void QgsRasterFormatSaveOptionsWidget::setCreationOptions( const QString &profileName, const QString &options )
{
  QgsSettings mySettings;
  mySettings.setValue( settingsKey( profileName ), options.trimmed() );
}

void QgsRasterFormatSaveOptionsWidget::setCreationOptions( const QString &profileName, const QStringList &options )
{
  setCreationOptions( profileName, options.join( ' '_L1 ) );
}

QStringList QgsRasterFormatSaveOptionsWidget::profiles() const
{
  const QgsSettings mySettings;
  return mySettings.value( mProvider + "/driverOptions/" + pseudoFormat().toLower() + "/profiles", "" ).toStringList();
}

void QgsRasterFormatSaveOptionsWidget::swapOptionsUI( int newIndex )
{
  // If newIndex == -1, toggle option mode
  // If newIndex == 0, set option mode to Table
  // If newIndex == 1, set option to lineEdit
  bool lineEditMode = mOptionsLineEdit->isVisible();
  mOptionsLineEdit->setVisible( ( newIndex == -1 && !lineEditMode ) || newIndex == 1 );
  mTableWidget->setVisible( ( newIndex == -1 && lineEditMode ) || newIndex == 0 );
  updateOptions();
}

void QgsRasterFormatSaveOptionsWidget::updateControls()
{
  const bool valid = mProvider == "gdal"_L1 && !mFormat.isEmpty();
  mOptionsValidateButton->setEnabled( valid );
  mOptionsHelpButton->setEnabled( valid );
}

// map options label left mouse click to optionsToggle()
bool QgsRasterFormatSaveOptionsWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::ContextMenu )
  {
    QContextMenuEvent *contextEvent = static_cast<QContextMenuEvent *>( event );
    QMenu *menu = nullptr;
    menu = mOptionsLineEdit->createStandardContextMenu();
    menu->addSeparator();
    QAction *action = new QAction( tr( "Use Table Interface" ), menu );
    menu->addAction( action );
    connect( action, &QAction::triggered, this, [this] { swapOptionsUI( 0 ); } );
    menu->exec( contextEvent->globalPos() );
    delete menu;
    return true;
  }
  // standard event processing
  return QObject::eventFilter( obj, event );
}

void QgsRasterFormatSaveOptionsWidget::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )
  updateOptions();
  mOptionsTable->horizontalHeader()->resizeSection( 0, mOptionsTable->width() - 115 );
  QgsDebugMsgLevel( u"done"_s, 3 );
}

void QgsRasterFormatSaveOptionsWidget::setOptions( const QString &options )
{
  mBlockOptionUpdates++;
  mOptionsTable->clearContents();

  const QStringList optionsList = options.trimmed().split( ' ', Qt::SkipEmptyParts );
  for ( const QString &opt : optionsList )
  {
    const int rowCount = mOptionsTable->rowCount();
    mOptionsTable->insertRow( rowCount );

    const QStringList values = opt.split( '=' );
    if ( values.count() == 2 )
    {
      QTableWidgetItem *nameItem = new QTableWidgetItem( values.at( 0 ) );
      mOptionsTable->setItem( rowCount, 0, nameItem );
      QTableWidgetItem *valueItem = new QTableWidgetItem( values.at( 1 ) );
      mOptionsTable->setItem( rowCount, 1, valueItem );
    }
  }

  // reset to no profile index, otherwise we are changing the definition of whichever profile
  // is currently selected...
  mProfileComboBox->setCurrentIndex( 0 );

  mOptionsMap[currentProfileKey()] = options.trimmed();
  mOptionsLineEdit->setText( options.trimmed() );
  mOptionsLineEdit->setCursorPosition( 0 );

  mBlockOptionUpdates--;
}
