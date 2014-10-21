/***************************************************************************
                         qgscomposerpicturewidget.cpp
                         ----------------------------
    begin                : August 13, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerpicturewidget.h"
#include "qgsapplication.h"
#include "qgscomposermap.h"
#include "qgscomposerpicture.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposition.h"
#include "qgsexpressionbuilderdialog.h"
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QSettings>
#include <QSvgRenderer>

QgsComposerPictureWidget::QgsComposerPictureWidget( QgsComposerPicture* picture ): QgsComposerItemBaseWidget( 0, picture ), mPicture( picture ), mPreviewsLoaded( false )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, picture );
  mainLayout->addWidget( itemPropertiesWidget );

  setGuiElementValues();
  mPreviewsLoadingLabel->hide();

  mPreviewListWidget->setIconSize( QSize( 30, 30 ) );

  // mSearchDirectoriesGroupBox is a QgsCollapsibleGroupBoxBasic, so its collapsed state should not be saved/restored
  mSearchDirectoriesGroupBox->setCollapsed( true );
  // setup connection for loading previews on first expansion of group box
  connect( mSearchDirectoriesGroupBox, SIGNAL( collapsedStateChanged( bool ) ), this, SLOT( loadPicturePreviews( bool ) ) );

  connect( mPicture, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
  connect( mPicture, SIGNAL( pictureRotationChanged( double ) ), this, SLOT( setPicRotationSpinValue( double ) ) );

  QgsAtlasComposition* atlas = atlasComposition();
  if ( atlas )
  {
    // repopulate data defined buttons if atlas layer changes
    connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
             this, SLOT( populateDataDefinedButtons() ) );
    connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( populateDataDefinedButtons() ) );
  }

  //connections for data defined buttons
  connect( mSourceDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mSourceDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mSourceDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mPictureLineEdit, SLOT( setDisabled( bool ) ) );
}

QgsComposerPictureWidget::~QgsComposerPictureWidget()
{

}

void QgsComposerPictureWidget::on_mPictureBrowseButton_clicked()
{
  QSettings s;
  QString openDir;
  QString lineEditText = mPictureLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( "/UI/lastComposerPictureDir", "" ).toString();
  }

  //show file dialog
  QString filePath = QFileDialog::getOpenFileName( 0, tr( "Select svg or image file" ), openDir );
  if ( filePath.isEmpty() )
  {
    return;
  }

  //check if file exists
  QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() || !fileInfo.isReadable() )
  {
    QMessageBox::critical( 0, "Invalid file", "Error, file does not exist or is not readable" );
    return;
  }

  s.setValue( "/UI/lastComposerPictureDir", fileInfo.absolutePath() );

  mPictureLineEdit->blockSignals( true );
  mPictureLineEdit->setText( filePath );
  mPictureLineEdit->blockSignals( false );

  //pass file path to QgsComposerPicture
  if ( mPicture )
  {
    mPicture->beginCommand( tr( "Picture changed" ) );
    mPicture->setPicturePath( filePath );
    mPicture->update();
    mPicture->endCommand();
  }
}

void QgsComposerPictureWidget::on_mPictureLineEdit_editingFinished()
{
  if ( mPicture )
  {
    QString filePath = mPictureLineEdit->text();

    //check if file exists
    QFileInfo fileInfo( filePath );

    mPicture->beginCommand( tr( "Picture changed" ) );
    mPicture->setPicturePath( filePath );
    mPicture->update();
    mPicture->endCommand();
  }
}

void QgsComposerPictureWidget::on_mPictureRotationSpinBox_valueChanged( double d )
{
  if ( mPicture )
  {
    mPicture->beginCommand( tr( "Picture rotation changed" ), QgsComposerMergeCommand::ComposerPictureRotation );
    mPicture->setPictureRotation( d );
    mPicture->endCommand();
  }
}

void QgsComposerPictureWidget::on_mPreviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
{
  Q_UNUSED( previous );
  if ( !mPicture || !current )
  {
    return;
  }

  QString absoluteFilePath = current->data( Qt::UserRole ).toString();
  mPicture->beginCommand( tr( "Picture changed" ) );
  mPicture->setPicturePath( absoluteFilePath );
  mPictureLineEdit->setText( absoluteFilePath );
  mPicture->update();
  mPicture->endCommand();
}

void QgsComposerPictureWidget::on_mAddDirectoryButton_clicked()
{
  //let user select a directory
  QString directory = QFileDialog::getExistingDirectory( 0, tr( "Select new preview directory" ) );
  if ( directory.isNull() )
  {
    return; //dialog canceled by user
  }

  //add entry to mSearchDirectoriesComboBox
  mSearchDirectoriesComboBox->addItem( directory );

  //and add icons to the preview
  addDirectoryToPreview( directory );

  //update the image directory list in the settings
  QSettings s;
  QStringList userDirList = s.value( "/Composer/PictureWidgetDirectories" ).toStringList();
  if ( !userDirList.contains( directory ) )
  {
    userDirList.append( directory );
  }
  s.setValue( "/Composer/PictureWidgetDirectories", userDirList );
}

void QgsComposerPictureWidget::on_mRemoveDirectoryButton_clicked()
{
  QString directoryToRemove = mSearchDirectoriesComboBox->currentText();
  if ( directoryToRemove.isEmpty() )
  {
    return;
  }
  mSearchDirectoriesComboBox->removeItem( mSearchDirectoriesComboBox->currentIndex() );

  //remove entries from back to front (to have the indices of existing items constant)
  for ( int i = ( mPreviewListWidget->count() - 1 ); i >= 0; --i )
  {
    QListWidgetItem* currentItem = mPreviewListWidget->item( i );
    if ( currentItem && currentItem->data( Qt::UserRole ).toString().startsWith( directoryToRemove ) )
    {
      delete( mPreviewListWidget->takeItem( i ) );
    }
  }

  //update the image directory list in the settings
  QSettings s;
  QStringList userDirList = s.value( "/Composer/PictureWidgetDirectories" ).toStringList();
  userDirList.removeOne( directoryToRemove );
  s.setValue( "/Composer/PictureWidgetDirectories", userDirList );
}

void QgsComposerPictureWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Picture resize mode changed" ) );
  mPicture->setResizeMode(( QgsComposerPicture::ResizeMode )index );
  mPicture->endCommand();

  //disable picture rotation for non-zoom modes
  mRotationGroupBox->setEnabled( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
                                 mPicture->resizeMode() == QgsComposerPicture::ZoomResizeFrame );

  //disable anchor point control for certain zoom modes
  if ( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
       mPicture->resizeMode() == QgsComposerPicture::Clip )
  {
    mAnchorPointComboBox->setEnabled( true );
  }
  else
  {
    mAnchorPointComboBox->setEnabled( false );
  }
}

void QgsComposerPictureWidget::on_mAnchorPointComboBox_currentIndexChanged( int index )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Picture placement changed" ) );
  mPicture->setPictureAnchor(( QgsComposerItem::ItemPositionMode )index );
  mPicture->endCommand();
}

void QgsComposerPictureWidget::on_mRotationFromComposerMapCheckBox_stateChanged( int state )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Rotation synchronisation toggled" ) );
  if ( state == Qt::Unchecked )
  {
    mPicture->setRotationMap( -1 );
    mPictureRotationSpinBox->setEnabled( true );
    mComposerMapComboBox->setEnabled( false );
    mPicture->setPictureRotation( mPictureRotationSpinBox->value() );
  }
  else
  {
    int currentItemIndex = mComposerMapComboBox->currentIndex();
    if ( currentItemIndex == -1 )
    {
      return;
    }
    int composerId = mComposerMapComboBox->itemData( currentItemIndex, Qt::UserRole ).toInt();

    mPicture->setRotationMap( composerId );
    mPictureRotationSpinBox->setEnabled( false );
    mComposerMapComboBox->setEnabled( true );
  }
  mPicture->endCommand();
}

void QgsComposerPictureWidget::on_mComposerMapComboBox_activated( const QString & text )
{
  if ( !mPicture || text.isEmpty() || !mPicture->useRotationMap() )
  {
    return;
  }

  //get composition
  const QgsComposition* composition = mPicture->composition();
  if ( !composition )
  {
    return;
  }

  //extract id
  int id;
  bool conversionOk;
  QStringList textSplit = text.split( " " );
  if ( textSplit.size() < 1 )
  {
    return;
  }

  QString idString = textSplit.at( textSplit.size() - 1 );
  id = idString.toInt( &conversionOk );

  if ( !conversionOk )
  {
    return;
  }

  const QgsComposerMap* composerMap = composition->getComposerMapById( id );
  if ( !composerMap )
  {
    return;
  }
  mPicture->beginCommand( tr( "Rotation map changed" ) );
  mPicture->setRotationMap( id );
  mPicture->update();
  mPicture->endCommand();
}

void QgsComposerPictureWidget::refreshMapComboBox()
{
  mComposerMapComboBox->blockSignals( true );
  //save the current entry in case it is still present after refresh
  QString saveCurrentComboText = mComposerMapComboBox->currentText();

  mComposerMapComboBox->clear();

  if ( mPicture )
  {
    //insert available maps into mMapComboBox
    const QgsComposition* composition = mPicture->composition();
    if ( composition )
    {
      QList<const QgsComposerMap*> availableMaps = composition->composerMapItems();
      QList<const QgsComposerMap*>::const_iterator mapItemIt = availableMaps.constBegin();
      for ( ; mapItemIt != availableMaps.constEnd(); ++mapItemIt )
      {
        mComposerMapComboBox->addItem( tr( "Map %1" ).arg(( *mapItemIt )->id() ), ( *mapItemIt )->id() );
      }
    }
  }

  if ( !saveCurrentComboText.isEmpty() )
  {
    if ( mComposerMapComboBox->findText( saveCurrentComboText ) == -1 )
    {
      //the former entry is no longer present. Inform the scalebar about the changed composer map
      on_mComposerMapComboBox_activated( mComposerMapComboBox->currentText() );
    }
    else
    {
      //the former entry is still present. Make it the current entry again
      mComposerMapComboBox->setCurrentIndex( mComposerMapComboBox->findText( saveCurrentComboText ) );
    }
  }
  mComposerMapComboBox->blockSignals( false );
}

void QgsComposerPictureWidget::setPicRotationSpinValue( double r )
{
  mPictureRotationSpinBox->blockSignals( true );
  mPictureRotationSpinBox->setValue( r );
  mPictureRotationSpinBox->blockSignals( false );
}

void QgsComposerPictureWidget::setGuiElementValues()
{
  //set initial gui values
  if ( mPicture )
  {
    mPictureRotationSpinBox->blockSignals( true );
    mPictureLineEdit->blockSignals( true );
    mComposerMapComboBox->blockSignals( true );
    mRotationFromComposerMapCheckBox->blockSignals( true );
    mResizeModeComboBox->blockSignals( true );
    mAnchorPointComboBox->blockSignals( true );

    mPictureLineEdit->setText( mPicture->picturePath() );
    mPictureRotationSpinBox->setValue( mPicture->pictureRotation() );

    refreshMapComboBox();

    if ( mPicture->useRotationMap() )
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Checked );
      mPictureRotationSpinBox->setEnabled( false );
      mComposerMapComboBox->setEnabled( true );
      QString mapText = tr( "Map %1" ).arg( mPicture->rotationMap() );
      int itemId = mComposerMapComboBox->findText( mapText );
      if ( itemId >= 0 )
      {
        mComposerMapComboBox->setCurrentIndex( itemId );
      }
    }
    else
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Unchecked );
      mPictureRotationSpinBox->setEnabled( true );
      mComposerMapComboBox->setEnabled( false );
    }

    mResizeModeComboBox->setCurrentIndex(( int )mPicture->resizeMode() );
    //disable picture rotation for non-zoom modes
    mRotationGroupBox->setEnabled( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
                                   mPicture->resizeMode() == QgsComposerPicture::ZoomResizeFrame );

    mAnchorPointComboBox->setCurrentIndex(( int )mPicture->pictureAnchor() );
    //disable anchor point control for certain zoom modes
    if ( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
         mPicture->resizeMode() == QgsComposerPicture::Clip )
    {
      mAnchorPointComboBox->setEnabled( true );
    }
    else
    {
      mAnchorPointComboBox->setEnabled( false );
    }

    mRotationFromComposerMapCheckBox->blockSignals( false );
    mPictureRotationSpinBox->blockSignals( false );
    mPictureLineEdit->blockSignals( false );
    mComposerMapComboBox->blockSignals( false );
    mResizeModeComboBox->blockSignals( false );
    mAnchorPointComboBox->blockSignals( false );

    populateDataDefinedButtons();
  }
}

int QgsComposerPictureWidget::addDirectoryToPreview( const QString& path )
{
  //go through all files of a directory
  QDir directory( path );
  if ( !directory.exists() || !directory.isReadable() )
  {
    return 1; //error
  }

  QFileInfoList fileList = directory.entryInfoList( QDir::Files );
  QFileInfoList::const_iterator fileIt = fileList.constBegin();

  QProgressDialog progress( "Adding Icons...", "Abort", 0, fileList.size() - 1, this );
  //cancel button does not seem to work properly with modal dialog
  //progress.setWindowModality(Qt::WindowModal);

  int counter = 0;
  for ( ; fileIt != fileList.constEnd(); ++fileIt )
  {

    progress.setLabelText( tr( "Creating icon for file %1" ).arg( fileIt->fileName() ) );
    progress.setValue( counter );
    QCoreApplication::processEvents();
    if ( progress.wasCanceled() )
    {
      break;
    }
    QString filePath = fileIt->absoluteFilePath();

    //test if file is svg or pixel format
    bool fileIsPixel = false;
    bool fileIsSvg = testSvgFile( filePath );
    if ( !fileIsSvg )
    {
      fileIsPixel = testImageFile( filePath );
    }

    //exclude files that are not svg or image
    if ( !fileIsSvg && !fileIsPixel )
    {
      ++counter; continue;
    }

    QListWidgetItem * listItem = new QListWidgetItem( mPreviewListWidget );
    listItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    if ( fileIsSvg )
    {
      QIcon icon( filePath );
      listItem->setIcon( icon );
    }
    else if ( fileIsPixel ) //for pixel formats: create icon from scaled pixmap
    {
      QPixmap iconPixmap( filePath );
      if ( iconPixmap.isNull() )
      {
        ++counter; continue; //unknown file format or other problem
      }
      //set pixmap hardcoded to 30/30, same as icon size for mPreviewListWidget
      QPixmap scaledPixmap( iconPixmap.scaled( QSize( 30, 30 ), Qt::KeepAspectRatio ) );
      QIcon icon( scaledPixmap );
      listItem->setIcon( icon );
    }
    else
    {
      ++counter; continue;
    }

    listItem->setText( "" );
    //store the absolute icon file path as user data
    listItem->setData( Qt::UserRole, fileIt->absoluteFilePath() );
    ++counter;
  }

  return 0;
}

void QgsComposerPictureWidget::addStandardDirectoriesToPreview()
{
  mPreviewListWidget->clear();

  //list all directories in $prefix/share/qgis/svg
  QStringList svgPaths = QgsApplication::svgPaths();
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir svgDirectory( svgPaths[i] );
    if ( !svgDirectory.exists() || !svgDirectory.isReadable() )
    {
      continue;
    }

    //add directory itself
    mSearchDirectoriesComboBox->addItem( svgDirectory.absolutePath() );
    addDirectoryToPreview( svgDirectory.absolutePath() );

    //and also subdirectories
    QFileInfoList directoryList = svgDirectory.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
    QFileInfoList::const_iterator dirIt = directoryList.constBegin();
    for ( ; dirIt != directoryList.constEnd(); ++dirIt )
    {
      if ( addDirectoryToPreview( dirIt->absoluteFilePath() ) == 0 )
      {
        mSearchDirectoriesComboBox->addItem( dirIt->absoluteFilePath() );
      }
    }
  }

  //include additional user-defined directories for images
  QSettings s;
  QStringList userDirList = s.value( "/Composer/PictureWidgetDirectories" ).toStringList();
  QStringList::const_iterator userDirIt = userDirList.constBegin();
  for ( ; userDirIt != userDirList.constEnd(); ++userDirIt )
  {
    addDirectoryToPreview( *userDirIt );
    mSearchDirectoriesComboBox->addItem( *userDirIt );
  }

  mPreviewsLoaded = true;
}

bool QgsComposerPictureWidget::testSvgFile( const QString& filename ) const
{
  //QSvgRenderer crashes with some (non-svg) xml documents.
  //So at least we try to sort out the ones with different suffixes
  if ( !filename.endsWith( ".svg" ) )
  {
    return false;
  }

  QSvgRenderer svgRenderer( filename );
  return svgRenderer.isValid();
}

bool QgsComposerPictureWidget::testImageFile( const QString& filename ) const
{
  QString formatName = QString( QImageReader::imageFormat( filename ) );
  return !formatName.isEmpty(); //file is in a supported pixel format
}

void QgsComposerPictureWidget::loadPicturePreviews( bool collapsed )
{
  if ( mPreviewsLoaded )
  {
    return;
  }

  if ( !collapsed ) // load the previews only on first parent group box expansion
  {
    mPreviewListWidget->hide();
    mPreviewsLoadingLabel->show();
    addStandardDirectoriesToPreview();
    mPreviewsLoadingLabel->hide();
    mPreviewListWidget->show();
  }
}

void QgsComposerPictureWidget::showEvent( QShowEvent * event )
{
  Q_UNUSED( event );
  refreshMapComboBox();
}

void QgsComposerPictureWidget::resizeEvent( QResizeEvent * event )
{
  Q_UNUSED( event );
  mSearchDirectoriesComboBox->setMinimumWidth( mPreviewListWidget->sizeHint().width() );
}

QgsComposerObject::DataDefinedProperty QgsComposerPictureWidget::ddPropertyForWidget( QgsDataDefinedButton *widget )
{
  if ( widget == mSourceDDBtn )
  {
    return QgsComposerObject::PictureSource;
  }

  return QgsComposerObject::NoProperty;
}

void QgsComposerPictureWidget::populateDataDefinedButtons()
{
  QgsVectorLayer* vl = atlasCoverageLayer();

  //block signals from data defined buttons
  mSourceDDBtn->blockSignals( true );

  //initialise buttons to use atlas coverage layer
  mSourceDDBtn->init( vl, mPicture->dataDefinedProperty( QgsComposerObject::PictureSource ),
                      QgsDataDefinedButton::AnyType, QgsDataDefinedButton::anyStringDesc() );

  //initial state of controls - disable related controls when dd buttons are active
  mPictureLineEdit->setEnabled( !mSourceDDBtn->isActive() );

  //unblock signals from data defined buttons
  mSourceDDBtn->blockSignals( false );
}

